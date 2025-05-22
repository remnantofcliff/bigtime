#ifndef BT_STATE_H
#define BT_STATE_H

#include "fps_timer.h"
#include "game.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>
#include <stddef.h>

enum bt_gpu_buffer {
  bt_gpu_buffer_font_curve = 0,
  bt_gpu_buffer_font_curve_info,
  bt_gpu_buffer_vertex,
  bt_gpu_buffer_glyph2d_instance,
  bt_gpu_buffer_glyph3d_instance,
  bt_gpu_buffer_index,
  bt_gpu_buffer_draw,
  /*
   * Number of buffers
   */
  bt_gpu_buffer_count,
};

enum bt_shader {
  bt_shader_glyph2d_vert = 0,
  bt_shader_glyph3d_vert,
  bt_shader_glyph_frag,
  /*
   * Number of shaders
   */
  bt_shader_count,
};

enum bt_render_pipeline {
  bt_render_pipeline_glyph2d = 0,
  bt_render_pipeline_glyph3d,
  /*
   * Number of render pipelines
   */
  bt_render_pipeline_count,
};

struct bt_glyph2d_instance_data {
  float scale[2];
  float rotation;
  float translation[2];
  uint32_t c;
};

struct bt_glyph3d_instance_data {
  float scale[3];
  float rotation[4];
  float translation[3];
  uint32_t c;
};

struct bt_state {
  SDL_Window *window;
  SDL_GPUDevice *gpu;
  SDL_GPUShader *shaders[3];
  SDL_GPUTransferBuffer *transfer_buffer;
  SDL_GPUBuffer *buffers[bt_gpu_buffer_count];
  uint32_t buffer_sizes[bt_gpu_buffer_count];
  uint32_t transfer_buffer_offsets[bt_gpu_buffer_count];
  SDL_GPUGraphicsPipeline *render_pipelines[bt_render_pipeline_count];
  struct bt_fps_timer fps_timer;
  struct bt_glyph2d_instance_data glyph2d_instance_data[16];
  struct bt_glyph3d_instance_data glyph3d_instance_data[16];
  struct bt_game game;
  uint32_t width;
  uint32_t height;
};

bool bt_state_init(struct bt_state state[static 1]);
void bt_state_deinit(struct bt_state state[static 1]);

void bt_state_handle_keyevent(struct bt_state state[static 1],
                              SDL_KeyboardEvent const event[static 1]);
void bt_state_handle_mouse_motion_event(
    struct bt_state state[static 1],
    SDL_MouseMotionEvent const event[static 1]);
bool bt_state_render(struct bt_state state[static 1]);

#endif
