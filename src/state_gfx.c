#include "data.h"
#include "logging.h"
#include "state_private.h"
#include <uchar.h>

constexpr uint32_t bt_glyph2d_draw_offset = 0 * sizeof(*bt_draw_data_array);
constexpr uint32_t bt_glyph3d_draw_offset = 1 * sizeof(*bt_draw_data_array);

static void extrapolate_render_infos(struct bt_render_info info[static 1],
                                     struct bt_render_data out[static 1]) {
  out->camera_dir =
      bt_vec3_lerp(info->previous_state.camera_dir,
                   info->current_state.camera_dir, info->blend_factor);
  out->camera_pos =
      bt_vec3_lerp(info->previous_state.camera_pos,
                   info->current_state.camera_pos, info->blend_factor);
}

static bool
bt_state_copy_data_to_transfer_buffer(struct bt_state state[static 1],
                                      uint32_t offset, size_t data_size,
                                      unsigned char data[static data_size]) {
  unsigned char *p =
      SDL_MapGPUTransferBuffer(state->gpu, state->transfer_buffer, true);
  if (!p) {
    BT_LOG_SDL_FAIL("Failed to map transfer buffer");
    return false;
  }

  p += offset;

  SDL_memcpy(p, data, data_size);
  SDL_UnmapGPUTransferBuffer(state->gpu, state->transfer_buffer);

  return true;
}

static void bt_state_update_glyph2d_instance_data(
    struct bt_state state[static 1],
    uint32_t const chars[static bt_glyph2d_max_instances]) {
  float advance = 0.0f;
  for (size_t i = 0; i < bt_glyph2d_max_instances; i += 1) {
    struct bt_font_metrics const *metrics = &bt_font_metrics[chars[i]];
    struct bt_glyph2d_instance_data *instance_data =
        &state->glyph2d_instance_data[i];
    float scale = 0.1f;
    instance_data->scale[0] = scale;
    instance_data->scale[1] = scale;
    instance_data->rotation = 0.0f;
    instance_data->translation[0] =
        advance - 1.0f +
        (0.5f * scale) * ((float)state->height / (float)state->width);
    instance_data->translation[1] = 0.0f + 1.0f - 0.5f * scale;
    instance_data->c = chars[i];
    advance += metrics->advance * scale;
  }
}
static void bt_state_update_glyph3d_instance_data(
    struct bt_state state[static 1],
    uint32_t const chars[static bt_glyph3d_max_instances]) {
  float advance = 0.0f;
  float scale = 1.0f;
  for (size_t i = 0; i < bt_glyph3d_max_instances; i += 1) {
    struct bt_font_metrics const *metrics = &bt_font_metrics[chars[i]];
    struct bt_glyph3d_instance_data *instance_data =
        &state->glyph3d_instance_data[i];
    instance_data->scale[0] = scale;
    instance_data->scale[1] = scale;
    instance_data->scale[2] = 1.0f;
    instance_data->rotation[0] = 0.0f;
    instance_data->rotation[1] = 0.0f;
    instance_data->rotation[2] = 0.0f;
    instance_data->rotation[3] = 1.0f;
    instance_data->translation[0] = advance * scale;
    instance_data->translation[1] = 0.0f;
    instance_data->translation[2] = 0.0f;
    instance_data->c = chars[i];
    advance += metrics->advance;
  }
}

struct bt_uniforms {
  alignas(16) struct bt_mat4 proj_view;
  float aspect_ratio;
};

static void bt_state_get_uniform_data(struct bt_state state[static 1],
                                      struct bt_uniforms out[static 1]) {
  struct bt_render_info info = {};
  bt_game_get_render_info(&state->game, &info);

  struct bt_render_data extrapolated = {};
  extrapolate_render_infos(&info, &extrapolated);

  struct bt_mat4 view = {};
  bt_look_to(&view, &extrapolated.camera_pos, &extrapolated.camera_dir,
             &(struct bt_vec3){
                 .y = 1.0f,
             });

  out->aspect_ratio = (float)state->width / (float)state->height;
  struct bt_mat4 proj = {};
  bt_perspective(&proj, bt_pi * 0.25f, out->aspect_ratio, 0.1f, 100.0f);
  bt_mat4_mul(&proj, &view, &out->proj_view);
}

static bool bt_state_update_fps(struct bt_state state[static 1]) {
  struct bt_fps_report report = {};
  bt_fps_timer_increment_fps(&state->fps_timer, &report);
  if (report.did_update) {
    uint32_t chars2d[bt_glyph2d_max_instances] = {};
    char temp[SDL_arraysize(chars2d)] = {};
    SDL_snprintf(temp, SDL_arraysize(temp), "FPS: %" PRIu64, report.fps);

    char *temp_p = temp;
    char *temp_end = temp + SDL_arraysize(temp);
    uint32_t *out_p = chars2d;
    mbstate_t mbstate = {};
    while (true) {
      size_t n =
          mbrtoc32(out_p, temp_p,
                   (size_t)((uintptr_t)temp_end - (uintptr_t)temp_p), &mbstate);
      if (n == 0) {
        break;
      }
      if (n == (size_t)-1 || n == (size_t)-2) {
        break;
      }
      temp_p += n;
      out_p += 1;
    }
    bt_state_update_glyph2d_instance_data(state, chars2d);

    if (!bt_state_copy_data_to_transfer_buffer(
            state,
            state->transfer_buffer_offsets[bt_gpu_buffer_glyph2d_instance],
            bt_glyph2d_max_instances * sizeof(*state->glyph2d_instance_data),
            (unsigned char *)state->glyph2d_instance_data)) {
      return false;
    }

    uint32_t chars3d[bt_glyph3d_max_instances] = U"3D TEXT TEST:D";
    bt_state_update_glyph3d_instance_data(state, chars3d);
    if (!bt_state_copy_data_to_transfer_buffer(
            state,
            state->transfer_buffer_offsets[bt_gpu_buffer_glyph3d_instance],
            bt_glyph3d_max_instances * sizeof(*state->glyph3d_instance_data),
            (unsigned char *)state->glyph3d_instance_data)) {
      return false;
    }
  }

  return true;
}

static void
bt_state_copy_transform_buffer_to_buffer(struct bt_state state[static 1],
                                         SDL_GPUCopyPass *copy_pass,
                                         enum bt_gpu_buffer buffer) {
  SDL_UploadToGPUBuffer(copy_pass,
                        &(SDL_GPUTransferBufferLocation){
                            .transfer_buffer = state->transfer_buffer,
                            .offset = state->transfer_buffer_offsets[buffer],
                        },
                        &(SDL_GPUBufferRegion){
                            .buffer = state->buffers[buffer],
                            .size = state->buffer_sizes[buffer],
                        },
                        true);
}

static void bt_state_render_text2d(struct bt_state state[static 1],
                                   SDL_GPURenderPass *render_pass) {
  SDL_BindGPUGraphicsPipeline(
      render_pass, state->render_pipelines[bt_render_pipeline_glyph2d]);
  SDL_BindGPUVertexBuffers(
      render_pass, 0,
      (SDL_GPUBufferBinding[]){
          {
              .buffer = state->buffers[bt_gpu_buffer_vertex],
              .offset = 0,
          },
          {
              .buffer = state->buffers[bt_gpu_buffer_glyph2d_instance],
              .offset = 0,
          },
      },
      2);
  SDL_BindGPUIndexBuffer(render_pass,
                         &(SDL_GPUBufferBinding){
                             .buffer = state->buffers[bt_gpu_buffer_index],
                             .offset = 0,
                         },
                         SDL_GPU_INDEXELEMENTSIZE_16BIT);
  SDL_BindGPUFragmentStorageBuffers(
      render_pass, 0, &state->buffers[bt_gpu_buffer_font_curve], 2);
  SDL_DrawGPUIndexedPrimitivesIndirect(render_pass,
                                       state->buffers[bt_gpu_buffer_draw],
                                       bt_glyph2d_draw_offset, 1);
}

static void bt_state_render_text3d(struct bt_state state[static 1],
                                   SDL_GPURenderPass *render_pass) {
  SDL_BindGPUGraphicsPipeline(
      render_pass, state->render_pipelines[bt_render_pipeline_glyph3d]);
  SDL_BindGPUVertexBuffers(
      render_pass, 0,
      (SDL_GPUBufferBinding[]){
          {
              .buffer = state->buffers[bt_gpu_buffer_vertex],
              .offset = 0,
          },
          {
              .buffer = state->buffers[bt_gpu_buffer_glyph3d_instance],
              .offset = 0,
          },
      },
      2);
  SDL_BindGPUIndexBuffer(render_pass,
                         &(SDL_GPUBufferBinding){
                             .buffer = state->buffers[bt_gpu_buffer_index],
                             .offset = 0,
                         },
                         SDL_GPU_INDEXELEMENTSIZE_16BIT);
  SDL_BindGPUFragmentStorageBuffers(
      render_pass, 0, &state->buffers[bt_gpu_buffer_font_curve], 2);
  SDL_DrawGPUIndexedPrimitivesIndirect(render_pass,
                                       state->buffers[bt_gpu_buffer_draw],
                                       bt_glyph3d_draw_offset, 1);
}

bool bt_state_render(struct bt_state state[static 1]) {
  struct bt_uniforms uniform_data = {};
  bt_state_get_uniform_data(state, &uniform_data);

  SDL_GPUCommandBuffer *command_buffer =
      SDL_AcquireGPUCommandBuffer(state->gpu);
  if (!command_buffer) {
    BT_LOG_SDL_FAIL("Failed to acquire command buffer");
    return false;
  }

  bool result = true;

  SDL_GPUCopyPass *const copy_pass = SDL_BeginGPUCopyPass(command_buffer);
  bt_state_copy_transform_buffer_to_buffer(state, copy_pass,
                                           bt_gpu_buffer_glyph2d_instance);
  bt_state_copy_transform_buffer_to_buffer(state, copy_pass,
                                           bt_gpu_buffer_glyph3d_instance);
  SDL_EndGPUCopyPass(copy_pass);

  uint32_t width;
  uint32_t height;
  SDL_GPUTexture *texture = nullptr;
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, state->window,
                                             &texture, &width, &height)) {
    BT_LOG_SDL_FAIL("Failed to acquire swapchain texture");
    result = false;
    goto submit;
  }

  if (width != state->width || height != state->height) {
    state->width = width;
    state->height = height;
    SDL_GPUTexture *new_texture = bt_create_depth_texture(state);
    if (new_texture) {
      SDL_WaitForGPUIdle(state->gpu);
      SDL_ReleaseGPUTexture(state->gpu, state->depth_texture);
      state->depth_texture = new_texture;
    }
  }

  if (!texture) {
    result = false;
    goto submit;
  }

  SDL_PushGPUVertexUniformData(command_buffer, 1, &uniform_data,
                               sizeof(uniform_data));

  SDL_GPURenderPass *render_pass =
      SDL_BeginGPURenderPass(command_buffer,
                             (SDL_GPUColorTargetInfo[]){
                                 {
                                     .texture = texture,
                                     .clear_color =
                                         (SDL_FColor){
                                             .r = 0.0f,
                                             .g = 0.0f,
                                             .b = 0.0f,
                                             .a = 1.0f,
                                         },
                                     .load_op = SDL_GPU_LOADOP_DONT_CARE,
                                     .store_op = SDL_GPU_STOREOP_STORE,
                                 },
                             },
                             1,
                             &(SDL_GPUDepthStencilTargetInfo){
                                 .texture = state->depth_texture,
                                 .clear_depth = 1.0f,
                                 .load_op = SDL_GPU_LOADOP_CLEAR,
                                 .store_op = SDL_GPU_STOREOP_STORE,
                                 .stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
                                 .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
                                 .cycle = true,
                                 .clear_stencil = 0.0f,
                             });
  bt_state_render_text3d(state, render_pass);
  SDL_EndGPURenderPass(render_pass);
  render_pass =
      SDL_BeginGPURenderPass(command_buffer,
                             (SDL_GPUColorTargetInfo[]){
                                 {
                                     .texture = texture,
                                     .clear_color =
                                         (SDL_FColor){
                                             .r = 0.0f,
                                             .g = 0.0f,
                                             .b = 0.0f,
                                             .a = 1.0f,
                                         },
                                     .load_op = SDL_GPU_LOADOP_LOAD,
                                     .store_op = SDL_GPU_STOREOP_STORE,
                                 },
                             },
                             1, nullptr);
  bt_state_render_text2d(state, render_pass);
  SDL_EndGPURenderPass(render_pass);

submit:
  if (!SDL_SubmitGPUCommandBuffer(command_buffer)) {
    BT_LOG_SDL_FAIL("Failed to submit command buffer");
    result = false;
  }

  if (result) {
    bt_state_update_fps(state);
  }

  return result;
}
