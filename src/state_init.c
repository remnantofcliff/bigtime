#include "data.h"
#include "helpers.h"
#include "state_private.h"
#include <SDL3/SDL_gpu.h>
#include <stddef.h>

struct bt_vertex_data {
  float color[3];
};

constexpr struct bt_vertex_data bt_vertex_data_array[] = {
    {
        .color = {1.0f, 1.0f, 1.0f},
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

static bool bt_create_buffers(struct bt_state state[static 1]) {
  constexpr SDL_GPUBufferUsageFlags bt_gpu_buffer_flags[] = {
      [bt_gpu_buffer_font_curve] = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
      [bt_gpu_buffer_font_curve_info] =
          SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
      [bt_gpu_buffer_vertex] = SDL_GPU_BUFFERUSAGE_VERTEX,
      [bt_gpu_buffer_glyph2d_instance] = SDL_GPU_BUFFERUSAGE_VERTEX,
      [bt_gpu_buffer_glyph3d_instance] = SDL_GPU_BUFFERUSAGE_VERTEX,
      [bt_gpu_buffer_index] = SDL_GPU_BUFFERUSAGE_INDEX,
      [bt_gpu_buffer_draw] = SDL_GPU_BUFFERUSAGE_INDIRECT,
  };
  static char const *const bt_gpu_buffer_names[] = {
      [bt_gpu_buffer_font_curve] = "font curve buffer",
      [bt_gpu_buffer_font_curve_info] = "font curve info buffer",
      [bt_gpu_buffer_vertex] = "vertex buffer",
      [bt_gpu_buffer_glyph2d_instance] = "glyph2d instance buffer",
      [bt_gpu_buffer_glyph3d_instance] = "glyph3d instance buffer",
      [bt_gpu_buffer_index] = "index buffer",
      [bt_gpu_buffer_draw] = "draw buffer",
  };

  state->buffer_sizes[bt_gpu_buffer_font_curve] = bt_font_curves_byte_size;
  state->buffer_sizes[bt_gpu_buffer_font_curve_info] =
      bt_font_curve_infos_byte_size;
  state->buffer_sizes[bt_gpu_buffer_vertex] = sizeof(bt_vertex_data_array);
  state->buffer_sizes[bt_gpu_buffer_glyph2d_instance] =
      bt_glyph2d_max_instances * sizeof(*state->glyph2d_instance_data);
  state->buffer_sizes[bt_gpu_buffer_glyph3d_instance] =
      bt_glyph3d_max_instances * sizeof(*state->glyph3d_instance_data);
  state->buffer_sizes[bt_gpu_buffer_index] = sizeof(bt_index_data_array);
  state->buffer_sizes[bt_gpu_buffer_draw] = sizeof(bt_draw_data_array);

  state->transfer_buffer_offsets[0] = 0;
  for (enum bt_gpu_buffer i = 1; i < bt_gpu_buffer_count; i += 1) {
    state->transfer_buffer_offsets[i] =
        state->transfer_buffer_offsets[i - 1] + state->buffer_sizes[i - 1];
  }

  uint32_t transfer_buffer_size = 0;
  for (enum bt_gpu_buffer i = 0; i < bt_gpu_buffer_count; i += 1) {
    transfer_buffer_size += state->buffer_sizes[i];
  }
  state->transfer_buffer = SDL_CreateGPUTransferBuffer(
      state->gpu, &(SDL_GPUTransferBufferCreateInfo){
                      .size = transfer_buffer_size,
                  });
  if (!state->transfer_buffer) {
    BT_LOG_SDL_FAIL("Failed to create transfer buffer");
    return false;
  }

  for (enum bt_gpu_buffer i = 0; i < bt_gpu_buffer_count; i += 1) {
    state->buffers[i] =
        SDL_CreateGPUBuffer(state->gpu, &(SDL_GPUBufferCreateInfo){
                                            .usage = bt_gpu_buffer_flags[i],
                                            .size = state->buffer_sizes[i],
                                        });
    if (!state->buffers[i]) {
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
      [bt_gpu_buffer_glyph2d_instance] = state->glyph2d_instance_data,
      [bt_gpu_buffer_glyph3d_instance] = state->glyph3d_instance_data,
      [bt_gpu_buffer_index] = bt_index_data_array,
      [bt_gpu_buffer_draw] = bt_draw_data_array,
  };

  unsigned char *p =
      SDL_MapGPUTransferBuffer(state->gpu, state->transfer_buffer, true);
  if (!p) {
    BT_LOG_SDL_FAIL("Failed to map transfer buffer");
    return false;
  }

  for (enum bt_gpu_buffer i = 0; i < bt_gpu_buffer_count; i += 1) {
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
  if (!command_buffer) {
    BT_LOG_SDL_FAIL("Failed to create command buffer");
    return false;
  }

  SDL_GPUCopyPass *const copy_pass = SDL_BeginGPUCopyPass(command_buffer);

  uint32_t current_offset = 0;
  for (enum bt_gpu_buffer i = 0; i < bt_gpu_buffer_count; i += 1) {
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
  if (!fence) {
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
  size_t const shader_source_sizes[] = {
      [bt_shader_glyph2d_vert] = bt_glyph2d_vertex_spirv_byte_size,
      [bt_shader_glyph3d_vert] = bt_glyph3d_vertex_spirv_byte_size,
      [bt_shader_glyph_frag] = bt_glyph_fragment_spirv_byte_size,
  };
  uint8_t const *const shader_sources[] = {
      [bt_shader_glyph2d_vert] = (uint8_t const *)bt_glyph2d_vertex_spirv,
      [bt_shader_glyph3d_vert] = (uint8_t const *)bt_glyph3d_vertex_spirv,
      [bt_shader_glyph_frag] = (uint8_t const *)bt_glyph_fragment_spirv,
  };
  SDL_GPUShaderStage const shader_stages[] = {
      [bt_shader_glyph2d_vert] = SDL_GPU_SHADERSTAGE_VERTEX,
      [bt_shader_glyph3d_vert] = SDL_GPU_SHADERSTAGE_VERTEX,
      [bt_shader_glyph_frag] = SDL_GPU_SHADERSTAGE_FRAGMENT,
  };
  constexpr uint32_t sampler_counts[] = {
      [bt_shader_glyph2d_vert] = 0,
      [bt_shader_glyph3d_vert] = 0,
      [bt_shader_glyph_frag] = 0,
  };
  constexpr uint32_t storage_texture_counts[] = {
      [bt_shader_glyph2d_vert] = 0,
      [bt_shader_glyph3d_vert] = 0,
      [bt_shader_glyph_frag] = 0,
  };
  constexpr uint32_t storage_buffer_counts[] = {
      [bt_shader_glyph2d_vert] = 0,
      [bt_shader_glyph3d_vert] = 0,
      [bt_shader_glyph_frag] = 2,
  };
  constexpr uint32_t uniform_counts[] = {
      [bt_shader_glyph2d_vert] = 2,
      [bt_shader_glyph3d_vert] = 1,
      [bt_shader_glyph_frag] = 0,
  };

  for (enum bt_shader i = 0; i < bt_shader_count; i += 1) {
    state->shaders[i] = SDL_CreateGPUShader(
        state->gpu, &(SDL_GPUShaderCreateInfo){
                        .code_size = shader_source_sizes[i],
                        .code = shader_sources[i],
                        .entrypoint = "main",
                        .format = SDL_GPU_SHADERFORMAT_SPIRV,
                        .stage = shader_stages[i],
                        .num_samplers = sampler_counts[i],
                        .num_storage_textures = storage_texture_counts[i],
                        .num_storage_buffers = storage_buffer_counts[i],
                        .num_uniform_buffers = uniform_counts[i],
                    });
    if (!state->shaders[i]) {
      BT_LOG_SDL_FAIL("Failed to create vertex shader");
      return false;
    }
  }

  return true;
}

static bool bt_create_render_pipelines(struct bt_state state[static 1]) {
  constexpr enum bt_shader vertex_shaders[] = {
      [bt_render_pipeline_glyph2d] = bt_shader_glyph2d_vert,
      [bt_render_pipeline_glyph3d] = bt_shader_glyph3d_vert,
  };

  constexpr enum bt_shader fragment_shaders[] = {
      [bt_render_pipeline_glyph2d] = bt_shader_glyph_frag,
      [bt_render_pipeline_glyph3d] = bt_shader_glyph_frag,
  };

  constexpr SDL_GPUVertexBufferDescription
      glyph2d_vertex_buffer_descriptions[] = {
          {
              .slot = 0,
              .pitch = sizeof(struct bt_vertex_data),
              .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
          },
          {
              .slot = 1,
              .pitch = sizeof(struct bt_glyph2d_instance_data),
              .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
          },
      };

  constexpr SDL_GPUVertexBufferDescription
      glyph3d_vertex_buffer_descriptions[] = {
          {
              .slot = 0,
              .pitch = sizeof(struct bt_vertex_data),
              .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
          },
          {
              .slot = 1,
              .pitch = sizeof(struct bt_glyph3d_instance_data),
              .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
          },
      };

  SDL_GPUVertexBufferDescription const *const vertex_buffer_descriptions[] = {
      [bt_render_pipeline_glyph2d] = glyph2d_vertex_buffer_descriptions,
      [bt_render_pipeline_glyph3d] = glyph3d_vertex_buffer_descriptions,
  };

  constexpr uint32_t vertex_buffer_description_counts[] = {
      [bt_render_pipeline_glyph2d] =
          SDL_arraysize(glyph2d_vertex_buffer_descriptions),
      [bt_render_pipeline_glyph3d] =
          SDL_arraysize(glyph3d_vertex_buffer_descriptions),
  };

  constexpr SDL_GPUVertexAttribute glyph2d_vertex_attributes[] = {
      {
          .location = 0,
          .buffer_slot = 0,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
          .offset = offsetof(struct bt_vertex_data, color),
      },
      {
          .location = 1,
          .buffer_slot = 1,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
          .offset = offsetof(typeof(*state->glyph2d_instance_data), scale),
      },
      {
          .location = 2,
          .buffer_slot = 1,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
          .offset = offsetof(typeof(*state->glyph2d_instance_data), rotation),
      },
      {
          .location = 3,
          .buffer_slot = 1,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
          .offset =
              offsetof(typeof(*state->glyph2d_instance_data), translation),
      },
      {
          .location = 4,
          .buffer_slot = 1,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_UINT,
          .offset = offsetof(typeof(*state->glyph2d_instance_data), c),
      },
  };
  constexpr SDL_GPUVertexAttribute glyph3d_vertex_attributes[] = {
      {
          .location = 0,
          .buffer_slot = 0,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
          .offset = offsetof(struct bt_vertex_data, color),
      },
      {
          .location = 1,
          .buffer_slot = 1,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
          .offset = offsetof(typeof(*state->glyph3d_instance_data), scale),
      },
      {
          .location = 2,
          .buffer_slot = 1,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
          .offset = offsetof(typeof(*state->glyph3d_instance_data), rotation),
      },
      {
          .location = 3,
          .buffer_slot = 1,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
          .offset =
              offsetof(typeof(*state->glyph3d_instance_data), translation),
      },
      {
          .location = 4,
          .buffer_slot = 1,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_UINT,
          .offset = offsetof(typeof(*state->glyph3d_instance_data), c),
      },
  };

  SDL_GPUVertexAttribute const *const vertex_attributes[] = {
      [bt_render_pipeline_glyph2d] = glyph2d_vertex_attributes,
      [bt_render_pipeline_glyph3d] = glyph3d_vertex_attributes,
  };

  constexpr uint32_t vertex_attribute_counts[] = {
      [bt_render_pipeline_glyph2d] = SDL_arraysize(glyph2d_vertex_attributes),
      [bt_render_pipeline_glyph3d] = SDL_arraysize(glyph3d_vertex_attributes),
  };

  constexpr bool enable_depths[] = {
      [bt_render_pipeline_glyph2d] = false,
      [bt_render_pipeline_glyph3d] = true,
  };

  SDL_GPUTextureFormat format =
      SDL_GetGPUSwapchainTextureFormat(state->gpu, state->window);
  for (enum bt_render_pipeline i = 0; i < bt_render_pipeline_count; i += 1) {
    SDL_GPUGraphicsPipelineCreateInfo create_info = {
        .vertex_shader = state->shaders[vertex_shaders[i]],
        .fragment_shader = state->shaders[fragment_shaders[i]],
        .vertex_input_state =
            {
                .vertex_buffer_descriptions = vertex_buffer_descriptions[i],
                .num_vertex_buffers = vertex_buffer_description_counts[i],
                .vertex_attributes = vertex_attributes[i],
                .num_vertex_attributes = vertex_attribute_counts[i],
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
                            .format = format,
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
    };
    if (enable_depths[i]) {
      create_info.depth_stencil_state = (SDL_GPUDepthStencilState){
          .compare_op = SDL_GPU_COMPAREOP_LESS,
          .write_mask = 0xFF,
          .enable_depth_test = true,
          .enable_depth_write = true,
      };
      create_info.target_info.depth_stencil_format = bt_depth_format;
      create_info.target_info.has_depth_stencil_target = true;
    }
    state->render_pipelines[i] =
        SDL_CreateGPUGraphicsPipeline(state->gpu, &create_info);
    if (!state->render_pipelines[i]) {
      BT_LOG_SDL_FAIL("Failed to create render pipeline");
      return false;
    }
  }

  return true;
}

SDL_GPUTexture *bt_create_depth_texture(struct bt_state state[static 1]) {
  SDL_GPUTexture *texture = SDL_CreateGPUTexture(
      state->gpu, &(SDL_GPUTextureCreateInfo){
                      .type = SDL_GPU_TEXTURETYPE_2D,
                      .format = bt_depth_format,
                      .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
                      .width = state->width,
                      .height = state->height,
                      .layer_count_or_depth = 1,
                      .num_levels = 1,
                      .sample_count = SDL_GPU_SAMPLECOUNT_1,
                  });
  if (!texture) {
    BT_LOG_SDL_FAIL("Failed to create depth texture");
  }

  return texture;
}

bool bt_state_init(struct bt_state state[static 1]) {
  SDL_zerop(state);

  state->glyph2d_instance_data = SDL_calloc(
      bt_glyph2d_max_instances, sizeof(*state->glyph2d_instance_data));
  state->glyph3d_instance_data = SDL_calloc(
      bt_glyph3d_max_instances, sizeof(*state->glyph3d_instance_data));
  if (!(state->glyph2d_instance_data && state->glyph3d_instance_data)) {
    BT_LOG_SDL_FAIL("Failed to allocate instance data");
    return false;
  }
  state->width = 800;
  state->height = 800;

  state->window = SDL_CreateWindow("bigtime", (int)state->width,
                                   (int)state->height, SDL_WINDOW_RESIZABLE);
  if (!state->window) {
    BT_LOG_SDL_FAIL("Failed to create SDL window");
    return false;
  }

  if (!SDL_SetWindowRelativeMouseMode(state->window, true)) {
    BT_LOG_SDL_FAIL("Failed to enable relative mouse mode");
  }

  state->gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
  if (!state->window) {
    BT_LOG_SDL_FAIL("Failed to create SDL GPUDevice");
    return false;
  }

  if (!SDL_ClaimWindowForGPUDevice(state->gpu, state->window)) {
    BT_LOG_SDL_FAIL("Failed to claim SDL window for GPUDevice");
    return false;
  }

  state->depth_texture = bt_create_depth_texture(state);
  if (!state->depth_texture) {
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

  if (!bt_create_render_pipelines(state)) {
    return false;
  }

  if (!bt_game_run(&state->game)) {
    return false;
  }

  return true;
}

void bt_state_deinit(struct bt_state state[static 1]) {
  bt_game_stop(&state->game);

  SDL_WaitForGPUIdle(state->gpu);
  for (enum bt_render_pipeline i = 0; i < bt_render_pipeline_count; i += 1) {
    if (state->render_pipelines[i]) {
      SDL_ReleaseGPUGraphicsPipeline(state->gpu, state->render_pipelines[i]);
    }
  }
  for (enum bt_shader i = 0; i < bt_shader_count; i += 1) {
    if (state->shaders[i]) {
      SDL_ReleaseGPUShader(state->gpu, state->shaders[i]);
    }
  }
  for (enum bt_gpu_buffer i = 0; i < bt_gpu_buffer_count; i += 1) {
    if (state->buffers[i]) {
      SDL_ReleaseGPUBuffer(state->gpu, state->buffers[i]);
    }
  }
  if (state->depth_texture) {
    SDL_ReleaseGPUTexture(state->gpu, state->depth_texture);
  }
  if (state->transfer_buffer) {
    SDL_ReleaseGPUTransferBuffer(state->gpu, state->transfer_buffer);
  }
  if (state->gpu) {
    SDL_DestroyGPUDevice(state->gpu);
  }
  if (state->window) {
    SDL_DestroyWindow(state->window);
  }
  if (state->glyph2d_instance_data) {
    SDL_free(state->glyph2d_instance_data);
  }
  if (state->glyph3d_instance_data) {
    SDL_free(state->glyph3d_instance_data);
  }
  SDL_memset(state, 0, sizeof(*state));
}
