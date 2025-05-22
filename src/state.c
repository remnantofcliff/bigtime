#include "state.h"
#include "data.h"
#include "event_queue.h"
#include "helpers.h"
#include "math.h"
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_version.h>
#include <stddef.h>

struct bt_vertex_data {
  float color[3];
};

constexpr struct bt_vertex_data bt_vertex_data_array[] = {
    {
        .color = {0.25f, 0.5f, 1.0f},
    },
    {
        .color = {0.5f, 1.0f, 0.2f},
    },
    {
        .color = {1.0f, 0.2f, 0.4f},
    },
    {
        .color = {0.2f, 0.4f, 0.8f},
    },
};
constexpr uint16_t bt_index_data_array[] = {0, 1, 2, 1, 3, 2};
constexpr SDL_GPUIndexedIndirectDrawCommand bt_draw_data_array[] = {
    {

        .num_indices = 6,
        .num_instances = SDL_arraysize((struct bt_state){}.instance_data),
        .first_index = 0,
        .vertex_offset = 0,
        .first_instance = 0,
    },
};

constexpr SDL_GPUBufferUsageFlags bt_gpu_buffer_flags[] = {
    [bt_gpu_buffer_font_curve] = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
    [bt_gpu_buffer_font_curve_info] = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
    [bt_gpu_buffer_vertex] = SDL_GPU_BUFFERUSAGE_VERTEX,
    [bt_gpu_buffer_instance] = SDL_GPU_BUFFERUSAGE_VERTEX,
    [bt_gpu_buffer_index] = SDL_GPU_BUFFERUSAGE_INDEX,
    [bt_gpu_buffer_draw] = SDL_GPU_BUFFERUSAGE_INDIRECT,
};
static char const *const bt_gpu_buffer_names[] = {
    [bt_gpu_buffer_font_curve] = "font curve buffer",
    [bt_gpu_buffer_font_curve_info] = "font curve info buffer",
    [bt_gpu_buffer_vertex] = "vertex buffer",
    [bt_gpu_buffer_instance] = "instance buffer",
    [bt_gpu_buffer_index] = "index buffer",
    [bt_gpu_buffer_draw] = "draw buffer",
};

static bool bt_create_buffers(struct bt_state state[static 1]) {
  state->buffer_sizes[bt_gpu_buffer_font_curve] = bt_font_curves_byte_size;
  state->buffer_sizes[bt_gpu_buffer_font_curve_info] =
      bt_font_curve_infos_byte_size;
  state->buffer_sizes[bt_gpu_buffer_vertex] = sizeof(bt_vertex_data_array);
  state->buffer_sizes[bt_gpu_buffer_instance] = sizeof(state->instance_data);
  state->buffer_sizes[bt_gpu_buffer_index] = sizeof(bt_index_data_array);
  state->buffer_sizes[bt_gpu_buffer_draw] = sizeof(bt_draw_data_array);

  state->transfer_buffer_offsets[0] = 0;
  for (enum bt_gpu_buffer i = 1; i < bt_gpu_buffer_count; ++i) {
    state->transfer_buffer_offsets[i] =
        state->transfer_buffer_offsets[i - 1] + state->buffer_sizes[i - 1];
  }

  uint32_t transfer_buffer_size = 0;
  for (enum bt_gpu_buffer i = 0; i < bt_gpu_buffer_count; ++i) {
    transfer_buffer_size += state->buffer_sizes[i];
  }
  state->transfer_buffer = SDL_CreateGPUTransferBuffer(
      state->gpu, &(SDL_GPUTransferBufferCreateInfo){
                      .size = transfer_buffer_size,
                  });
  if (state->transfer_buffer == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create transfer buffer");
    return false;
  }

  for (enum bt_gpu_buffer i = 0; i < bt_gpu_buffer_count; ++i) {
    state->buffers[i] =
        SDL_CreateGPUBuffer(state->gpu, &(SDL_GPUBufferCreateInfo){
                                            .usage = bt_gpu_buffer_flags[i],
                                            .size = state->buffer_sizes[i],
                                        });
    if (state->buffers[i] == nullptr) {
      BT_LOG_SDL_FAIL("Failed to create %s", bt_gpu_buffer_names[i]);
      return false;
    }
  }

  return true;
}

static bool bt_initialize_transfer_buffer(struct bt_state state[static 1]) {
  void const *const data[] = {
      [bt_gpu_buffer_font_curve] = bt_font_curves,
      [bt_gpu_buffer_font_curve_info] = bt_font_curve_infos,
      [bt_gpu_buffer_vertex] = bt_vertex_data_array,
      [bt_gpu_buffer_instance] = state->instance_data,
      [bt_gpu_buffer_index] = bt_index_data_array,
      [bt_gpu_buffer_draw] = bt_draw_data_array,
  };
  static_assert(SDL_arraysize(data) == bt_gpu_buffer_count);

  unsigned char *p =
      SDL_MapGPUTransferBuffer(state->gpu, state->transfer_buffer, true);
  if (p == nullptr) {
    BT_LOG_SDL_FAIL("Failed to map transfer buffer");
    return false;
  }

  for (enum bt_gpu_buffer i = 0; i < bt_gpu_buffer_count; ++i) {
    SDL_memcpy(p, data[i], state->buffer_sizes[i]);
    p += state->buffer_sizes[i];
  }

  SDL_UnmapGPUTransferBuffer(state->gpu, state->transfer_buffer);

  return true;
}

static bool bt_initialize_buffers(struct bt_state state[static 1]) {
  if (!bt_initialize_transfer_buffer(state)) {
    return false;
  }

  SDL_GPUCommandBuffer *const command_buffer =
      SDL_AcquireGPUCommandBuffer(state->gpu);
  if (command_buffer == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create command buffer");
    return false;
  }

  SDL_GPUCopyPass *const copy_pass = SDL_BeginGPUCopyPass(command_buffer);

  uint32_t current_offset = 0;
  for (enum bt_gpu_buffer i = 0; i < bt_gpu_buffer_count; ++i) {
    SDL_UploadToGPUBuffer(copy_pass,
                          &(SDL_GPUTransferBufferLocation){
                              .transfer_buffer = state->transfer_buffer,
                              .offset = current_offset,
                          },
                          &(SDL_GPUBufferRegion){
                              .buffer = state->buffers[i],
                              .size = state->buffer_sizes[i],
                          },
                          true);
    current_offset += state->buffer_sizes[i];
  }

  SDL_EndGPUCopyPass(copy_pass);

  SDL_GPUFence *const fence =
      SDL_SubmitGPUCommandBufferAndAcquireFence(command_buffer);
  if (fence == nullptr) {
    BT_LOG_SDL_FAIL("Failed to submit command buffer");
    return false;
  }

  if (!SDL_WaitForGPUFences(state->gpu, true, &fence, 1)) {
    BT_LOG_SDL_FAIL("Failed to wait for fence");
    return false;
  }
  SDL_ReleaseGPUFence(state->gpu, fence);

  return true;
}

static bool bt_create_shaders(struct bt_state state[static 1]) {
  state->vertex_shader = SDL_CreateGPUShader(
      state->gpu, &(SDL_GPUShaderCreateInfo){
                      .code_size = bt_vertex_shader_spirv_byte_size,
                      .code = (uint8_t const *)bt_vertex_shader_spirv,
                      .entrypoint = "main",
                      .format = SDL_GPU_SHADERFORMAT_SPIRV,
                      .stage = SDL_GPU_SHADERSTAGE_VERTEX,
                      .num_uniform_buffers = 1,
                  });
  if (state->vertex_shader == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create vertex shader");
    return false;
  }

  state->fragment_shader = SDL_CreateGPUShader(
      state->gpu, &(SDL_GPUShaderCreateInfo){
                      .code_size = bt_fragment_shader_spirv_byte_size,
                      .code = (uint8_t const *)bt_fragment_shader_spirv,
                      .entrypoint = "main",
                      .format = SDL_GPU_SHADERFORMAT_SPIRV,
                      .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
                      .num_storage_buffers = 2,
                  });
  if (state->fragment_shader == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create fragment shader");
    return false;
  }

  return true;
}

#include <stdio.h>

static void bt_state_update_instance_data(
    struct bt_state state[static 1],
    uint32_t const chars[static SDL_arraysize(state->instance_data)]) {
  float advance = 0.0f;
  for (size_t i = 0; i < SDL_arraysize(state->instance_data); ++i) {
    struct bt_font_metrics const *metrics = &bt_font_metrics[chars[i]];
    state->instance_data[i].scale[0] = 1.0f;
    state->instance_data[i].scale[1] = 1.0f;
    state->instance_data[i].scale[2] = 1.0f;
    state->instance_data[i].translation[0] = advance;
    state->instance_data[i].translation[1] = 0.0f;
    state->instance_data[i].c = chars[i];
    advance += metrics->advance;
  }
}

bool bt_state_init(struct bt_state state[static 1]) {
  SDL_zerop(state);

  state->window = SDL_CreateWindow("bigtime", 800, 600, SDL_WINDOW_RESIZABLE);
  if (state->window == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create SDL window");
    return false;
  }

  if (!SDL_SetWindowRelativeMouseMode(state->window, true)) {
    BT_LOG_SDL_FAIL("Failed to enable relative mouse mode");
  }

  // SDL_SetRelativeMouseTransform();

  state->gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
  if (state->window == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create SDL GPUDevice");
    return false;
  }

  if (!SDL_ClaimWindowForGPUDevice(state->gpu, state->window)) {
    BT_LOG_SDL_FAIL("Failed to claim SDL window for GPUDevice");
    return false;
  }

  if (!bt_create_buffers(state)) {
    return false;
  }

  if (!bt_initialize_buffers(state)) {
    return false;
  }

  if (!bt_create_shaders(state)) {
    return false;
  }

  state->graphics_pipeline = SDL_CreateGPUGraphicsPipeline(
      state->gpu,
      &(SDL_GPUGraphicsPipelineCreateInfo){
          .vertex_shader = state->vertex_shader,
          .fragment_shader = state->fragment_shader,
          .vertex_input_state =
              {
                  .vertex_buffer_descriptions =
                      (SDL_GPUVertexBufferDescription[]){
                          {
                              .slot = 0,
                              .pitch = sizeof(*bt_vertex_data_array),
                              .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                          },
                          {
                              .slot = 1,
                              .pitch = sizeof(*state->instance_data),
                              .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
                          },
                      },
                  .num_vertex_buffers = 2,
                  .vertex_attributes =
                      (SDL_GPUVertexAttribute[]){
                          {
                              .location = 0,
                              .buffer_slot = 0,
                              .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                              .offset = 0,
                          },
                          {
                              .location = 1,
                              .buffer_slot = 1,
                              .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                              .offset = offsetof(typeof(*state->instance_data),
                                                 scale),
                          },
                          {
                              .location = 2,
                              .buffer_slot = 1,
                              .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                              .offset = offsetof(typeof(*state->instance_data),
                                                 rotation),
                          },
                          {
                              .location = 3,
                              .buffer_slot = 1,
                              .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                              .offset = offsetof(typeof(*state->instance_data),
                                                 translation),
                          },
                          {
                              .location = 4,
                              .buffer_slot = 1,
                              .format = SDL_GPU_VERTEXELEMENTFORMAT_UINT,
                              .offset =
                                  offsetof(typeof(*state->instance_data), c),
                          },
                      },
                  .num_vertex_attributes = 5,
              },
          .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
          .rasterizer_state =
              {
                  .fill_mode = SDL_GPU_FILLMODE_FILL,
                  .cull_mode = SDL_GPU_CULLMODE_BACK,
                  .front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
              },
          .target_info =
              {
                  .color_target_descriptions =
                      (SDL_GPUColorTargetDescription[]){
                          {
                              .format = SDL_GetGPUSwapchainTextureFormat(
                                  state->gpu, state->window),
                              .blend_state =
                                  {
                                      .src_color_blendfactor =
                                          SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                                      .dst_color_blendfactor =
                                          SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                                      .color_blend_op = SDL_GPU_BLENDOP_ADD,
                                      .src_alpha_blendfactor =
                                          SDL_GPU_BLENDFACTOR_ONE,
                                      .dst_alpha_blendfactor =
                                          SDL_GPU_BLENDFACTOR_ZERO,
                                      .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
                                      .enable_blend = true,
                                  },
                          },
                      },
                  .num_color_targets = 1,
              },
      });

  if (state->graphics_pipeline == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create graphics pipeline");
    return false;
  }

  if (!bt_game_run(&state->game)) {
    return false;
  }

  return true;
}

void bt_state_deinit(struct bt_state state[static 1]) {
  bt_game_stop(&state->game);

  if (state->graphics_pipeline != nullptr) {
    SDL_ReleaseGPUGraphicsPipeline(state->gpu, state->graphics_pipeline);
  }
  if (state->fragment_shader != nullptr) {
    SDL_ReleaseGPUShader(state->gpu, state->fragment_shader);
  }
  if (state->vertex_shader != nullptr) {
    SDL_ReleaseGPUShader(state->gpu, state->vertex_shader);
  }
  for (enum bt_gpu_buffer i = 0; i < bt_gpu_buffer_count; ++i) {
    if (state->buffers[i] != nullptr) {
      SDL_ReleaseGPUBuffer(state->gpu, state->buffers[i]);
    }
  }
  if (state->transfer_buffer != nullptr) {
    SDL_ReleaseGPUTransferBuffer(state->gpu, state->transfer_buffer);
  }
  if (state->gpu != nullptr) {
    SDL_DestroyGPUDevice(state->gpu);
  }
  if (state->window != nullptr) {
    SDL_DestroyWindow(state->window);
  }
  SDL_memset(state, 0, sizeof(*state));
}

void bt_state_handle_keyevent(struct bt_state state[static 1],
                              SDL_KeyboardEvent const event[static 1]) {
  enum bt_key key = {};
  bool key_used = false;
  switch (event->scancode) {
  case SDL_SCANCODE_A:
    key = bt_key_a;
    key_used = true;
    break;
  case SDL_SCANCODE_D:
    key = bt_key_d;
    key_used = true;
    break;
  case SDL_SCANCODE_S:
    key = bt_key_s;
    key_used = true;
    break;
  case SDL_SCANCODE_W:
    key = bt_key_w;
    key_used = true;
    break;
  case SDL_SCANCODE_RIGHT:
    key = bt_key_right;
    key_used = true;
    break;
  case SDL_SCANCODE_LEFT:
    key = bt_key_left;
    key_used = true;
    break;
  case SDL_SCANCODE_DOWN:
    key = bt_key_down;
    key_used = true;
    break;
  case SDL_SCANCODE_UP:
    key = bt_key_up;
    key_used = true;
    break;
  default:
    break;
  }

  if (!key_used) {
    return;
  }

  bt_event_queue_add(&state->game.event_queue, &(struct bt_event){
                                                   .type = bt_event_type_key,
                                                   .key =
                                                       {
                                                           .code = key,
                                                           .down = event->down,
                                                       },
                                               });
}

void bt_state_handle_mouse_motion_event(
    struct bt_state state[static 1],
    SDL_MouseMotionEvent const event[static 1]) {
  bt_event_queue_add(&state->game.event_queue,
                     &(struct bt_event){
                         .type = bt_event_type_mouse_motion,
                         .mouse_motion =
                             {
                                 .diff =
                                     {
                                         .x = event->xrel,
                                         .y = event->yrel,
                                     },
                             },
                     });
}

static void extrapolate_render_infos(struct bt_render_info info[static 1],
                                     struct bt_render_data out[static 1]) {
  out->camera_dir =
      bt_vec3_lerp(info->previous_state.camera_dir,
                   info->current_state.camera_dir, info->blend_factor);
  out->camera_pos =
      bt_vec3_lerp(info->previous_state.camera_pos,
                   info->current_state.camera_pos, info->blend_factor);
}

bool bt_copy_data_to_transfer_buffer(struct bt_state state[static 1],
                                     uint32_t offset, size_t data_size,
                                     unsigned char data[static data_size]) {
  unsigned char *p =
      SDL_MapGPUTransferBuffer(state->gpu, state->transfer_buffer, true);
  if (p == nullptr) {
    BT_LOG_SDL_FAIL("Failed to map transfer buffer");
    return false;
  }

  p += offset;

  SDL_memcpy(p, data, data_size);
  SDL_UnmapGPUTransferBuffer(state->gpu, state->transfer_buffer);

  return true;
}

bool bt_state_render(struct bt_state state[static 1]) {
  SDL_GPUCommandBuffer *command_buffer =
      SDL_AcquireGPUCommandBuffer(state->gpu);

  SDL_GPUCopyPass *const copy_pass = SDL_BeginGPUCopyPass(command_buffer);

  SDL_UploadToGPUBuffer(
      copy_pass,
      &(SDL_GPUTransferBufferLocation){
          .transfer_buffer = state->transfer_buffer,
          .offset = state->transfer_buffer_offsets[bt_gpu_buffer_instance],
      },
      &(SDL_GPUBufferRegion){
          .buffer = state->buffers[bt_gpu_buffer_instance],
          .size = state->buffer_sizes[bt_gpu_buffer_instance],
      },
      true);

  SDL_EndGPUCopyPass(copy_pass);

  struct bt_render_info info = {};
  bt_game_get_render_info(&state->game, &info);

  struct bt_render_data extrapolated = {};
  extrapolate_render_infos(&info, &extrapolated);

  struct bt_mat4 view = {};
  bt_look_to(&view, &extrapolated.camera_pos, &extrapolated.camera_dir,
             &(struct bt_vec3){
                 .y = 1.0f,
             });

  struct bt_mat4 proj = {};
  bt_perspective(&proj, bt_pi * 0.25f,
                 (float)state->width / (float)state->height, 0.1f, 100.0f);
  struct bt_mat4 proj_view = {};
  bt_mat4_mul(&proj, &view, &proj_view);

  SDL_GPUTexture *texture = nullptr;
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, state->window,
                                             &texture, &state->width,
                                             &state->height)) {
    BT_LOG_SDL_FAIL("Failed to acquire swapchain texture");
    return false;
  }

  if (texture != nullptr) {
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
                                       .load_op = SDL_GPU_LOADOP_CLEAR,
                                       .store_op = SDL_GPU_STOREOP_STORE,
                                   },
                               },
                               1, nullptr);

    SDL_BindGPUGraphicsPipeline(render_pass, state->graphics_pipeline);
    SDL_BindGPUVertexBuffers(
        render_pass, 0,
        (SDL_GPUBufferBinding[]){
            {
                .buffer = state->buffers[bt_gpu_buffer_vertex],
                .offset = 0,
            },
            {
                .buffer = state->buffers[bt_gpu_buffer_instance],
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
    SDL_PushGPUVertexUniformData(command_buffer, 1, proj_view.arr,
                                 sizeof(proj_view));
    SDL_DrawGPUIndexedPrimitivesIndirect(
        render_pass, state->buffers[bt_gpu_buffer_draw], 0, 1);
    SDL_EndGPURenderPass(render_pass);
  }

  if (!SDL_SubmitGPUCommandBuffer(command_buffer)) {
    BT_LOG_SDL_FAIL("Failed to submit command buffer");
    return false;
  }

  struct bt_fps_report report = {};
  bt_fps_timer_increment_fps(&state->fps_timer, &report);
  if (report.did_update) {
    uint32_t chars[SDL_arraysize(state->instance_data)] = {
        'F',
        'P',
        'S',
        ':',
        ' ',
        report.fps % 100000 / 10000 + '0',
        report.fps % 10000 / 1000 + '0',
        report.fps % 1000 / 100 + '0',
        report.fps % 100 / 10 + '0',
        report.fps % 10 + '0',
    };

    bt_state_update_instance_data(state, chars);

    if (!bt_copy_data_to_transfer_buffer(
            state, state->transfer_buffer_offsets[bt_gpu_buffer_instance],
            sizeof(state->instance_data),
            (unsigned char *)state->instance_data)) {
      return false;
    }
  }

  return true;
}
