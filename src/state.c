#include "state.h"
#include "helpers.h"
#include "math.h"
#include <stddef.h>

// clang-format off
static const unsigned char bt_vertex_spirv[] = {
#embed <shader.vert.spv>
};
static const unsigned char bt_fragment_spirv[] = {
#embed <shader.frag.spv>
};
// clang-format on

struct bt_vertex_data {
  float pos[3];
  float uv[2];
};

constexpr struct bt_vertex_data bt_vertex_data_array[4] = {
    {
        .pos =
            {
                -1.0,
                1.0,
                0.0,
            },
        .uv =
            {
                0.0,
                0.0,
            },
    },
    {
        .pos =
            {
                1.0,
                1.0,
                0.0,
            },
        .uv =
            {
                1.0,
                0.0,
            },
    },
    {
        .pos =
            {
                -1.0,
                -1.0,
                0.0,
            },
        .uv =
            {
                0.0,
                1.0,
            },
    },
    {
        .pos =
            {
                1.0,
                -1.0,
                0.0,
            },
        .uv =
            {
                1.0,
                1.0,
            },
    },
};

constexpr uint32_t bt_index_data_array[] = {
    0, 1, 2, 1, 3, 2,
};

constexpr SDL_GPUIndexedIndirectDrawCommand bt_draw_data_array[] = {
    {

        .num_indices = 6,
        .num_instances = 1,
        .first_index = 0,
        .vertex_offset = 0,
        .first_instance = 0,
    },
};

static bool bt_create_buffers(struct bt_state state[static 1]) {
  state->transfer_buffer = SDL_CreateGPUTransferBuffer(
      state->gpu,
      &(SDL_GPUTransferBufferCreateInfo){
          .size = sizeof(bt_vertex_data_array) + sizeof(bt_index_data_array) +
                  sizeof(bt_draw_data_array),
      });
  if (state->transfer_buffer == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create transfer buffer");
    return false;
  }

  state->vertex_buffer =
      SDL_CreateGPUBuffer(state->gpu, &(SDL_GPUBufferCreateInfo){
                                          .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                                          .size = sizeof(bt_vertex_data_array),
                                      });
  if (state->vertex_buffer == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create vertex buffer");
    return false;
  }

  state->index_buffer =
      SDL_CreateGPUBuffer(state->gpu, &(SDL_GPUBufferCreateInfo){
                                          .usage = SDL_GPU_BUFFERUSAGE_INDEX,
                                          .size = sizeof(bt_index_data_array),
                                      });

  if (state->index_buffer == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create index buffer");
    return false;
  }

  state->draw_buffer =
      SDL_CreateGPUBuffer(state->gpu, &(SDL_GPUBufferCreateInfo){
                                          .usage = SDL_GPU_BUFFERUSAGE_INDIRECT,
                                          .size = sizeof(bt_draw_data_array),
                                      });
  if (state->draw_buffer == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create draw buffer");
    return false;
  }

  return true;
}

static bool bt_initialize_transfer_buffer(struct bt_state state[static 1]) {
  unsigned char *p =
      SDL_MapGPUTransferBuffer(state->gpu, state->transfer_buffer, true);
  if (p == nullptr) {
    BT_LOG_SDL_FAIL("Failed to map transfer buffer");
    return false;
  }

  SDL_memcpy(p, bt_vertex_data_array, sizeof(bt_vertex_data_array));
  p += sizeof(bt_vertex_data_array);
  SDL_memcpy(p, bt_index_data_array, sizeof(bt_index_data_array));
  p += sizeof(bt_index_data_array);
  SDL_memcpy(p, bt_draw_data_array, sizeof(bt_draw_data_array));

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

  SDL_UploadToGPUBuffer(copy_pass,
                        &(SDL_GPUTransferBufferLocation){
                            .transfer_buffer = state->transfer_buffer,
                        },
                        &(SDL_GPUBufferRegion){
                            .buffer = state->vertex_buffer,
                            .offset = 0,
                            .size = sizeof(bt_vertex_data_array),
                        },
                        true);
  SDL_UploadToGPUBuffer(copy_pass,
                        &(SDL_GPUTransferBufferLocation){
                            .transfer_buffer = state->transfer_buffer,
                            .offset = sizeof(bt_vertex_data_array),
                        },
                        &(SDL_GPUBufferRegion){
                            .buffer = state->index_buffer,
                            .offset = 0,
                            .size = sizeof(bt_index_data_array),
                        },
                        true);
  SDL_UploadToGPUBuffer(
      copy_pass,
      &(SDL_GPUTransferBufferLocation){
          .transfer_buffer = state->transfer_buffer,
          .offset = sizeof(bt_vertex_data_array) + sizeof(bt_index_data_array),
      },
      &(SDL_GPUBufferRegion){
          .buffer = state->draw_buffer,
          .offset = 0,
          .size = sizeof(bt_draw_data_array),
      },
      true);

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
  state->vertex_shader =
      SDL_CreateGPUShader(state->gpu, &(SDL_GPUShaderCreateInfo){
                                          .code_size = sizeof(bt_vertex_spirv),
                                          .code = bt_vertex_spirv,
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
                      .code_size = sizeof(bt_fragment_spirv),
                      .code = bt_fragment_spirv,
                      .entrypoint = "main",
                      .format = SDL_GPU_SHADERFORMAT_SPIRV,
                      .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
                  });
  if (state->fragment_shader == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create fragment shader");
    return false;
  }

  return true;
}

bool bt_state_init(struct bt_state state[static 1]) {
  SDL_zerop(state);

  state->window = SDL_CreateWindow("bigtime", 800, 600, SDL_WINDOW_RESIZABLE);
  if (state->window == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create SDL window");
    return false;
  }

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
                      },
                  .num_vertex_buffers = 1,
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
                              .buffer_slot = 0,
                              .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                              .offset =
                                  offsetof(typeof(*bt_vertex_data_array), uv),
                          },
                      },
                  .num_vertex_attributes = 2,
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
                          },
                      },
                  .num_color_targets = 1,
              },
      });

  if (state->graphics_pipeline == nullptr) {
    BT_LOG_SDL_FAIL("Failed to create graphics pipeline");
    return false;
  }

  bt_game_run(&state->game);

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
  if (state->vertex_buffer != nullptr) {
    SDL_ReleaseGPUBuffer(state->gpu, state->vertex_buffer);
  }
  if (state->index_buffer != nullptr) {
    SDL_ReleaseGPUBuffer(state->gpu, state->index_buffer);
  }
  if (state->draw_buffer != nullptr) {
    SDL_ReleaseGPUBuffer(state->gpu, state->draw_buffer);
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
                              SDL_KeyboardEvent const *event) {
  switch (event->scancode) {
  case SDL_SCANCODE_RIGHT:
    state->game.input.moving_right = event->down;
    break;
  case SDL_SCANCODE_LEFT:
    state->game.input.moving_left = event->down;
    break;
  case SDL_SCANCODE_DOWN:
    state->game.input.moving_backwards = event->down;
    break;
  case SDL_SCANCODE_UP:
    state->game.input.moving_forwards = event->down;
    break;
  default:
    break;
  }
}

bool bt_state_render(struct bt_state state[static 1]) {
  SDL_GPUCommandBuffer *command_buffer =
      SDL_AcquireGPUCommandBuffer(state->gpu);

  SDL_GPUTexture *texture = nullptr;
  uint32_t width = 0;
  uint32_t height = 0;
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, state->window,
                                             &texture, &width, &height)) {
    BT_LOG_SDL_FAIL("Failed to acquire swapchain texture");
    return false;
  }
  struct bt_render_info infos[2] = {};
  bt_game_get_render_info(&state->game, infos);

  struct bt_mat4 proj = {};
  bt_perspective(&proj, bt_pi * 0.25f, (float)width / (float)height, 0.1f,
                 100.0f);

  struct bt_mat4 view = {};
  bt_look_to(&view, &infos[0].camera_pos, &infos[1].camera_dir,
             &(struct bt_vec3){{
                 [1] = 1.0f,
             }});

  struct bt_mat4 proj_view = {};
  bt_mat4_mul_mat4(&proj, &view, &proj_view);

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
    SDL_BindGPUVertexBuffers(render_pass, 0,
                             (SDL_GPUBufferBinding[]){
                                 {
                                     .buffer = state->vertex_buffer,
                                     .offset = 0,
                                 },
                             },
                             1);
    SDL_BindGPUIndexBuffer(render_pass,
                           &(SDL_GPUBufferBinding){
                               .buffer = state->index_buffer,
                               .offset = 0,
                           },
                           SDL_GPU_INDEXELEMENTSIZE_32BIT);
    SDL_PushGPUVertexUniformData(command_buffer, 1, proj_view.arr,
                                 sizeof(proj_view));
    SDL_DrawGPUIndexedPrimitivesIndirect(render_pass, state->draw_buffer, 0, 1);
    SDL_EndGPURenderPass(render_pass);
  }

  if (!SDL_SubmitGPUCommandBuffer(command_buffer)) {
    BT_LOG_SDL_FAIL("Failed to submit command buffer");
    return false;
  }

  return true;
}
