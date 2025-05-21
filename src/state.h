#ifndef BT_STATE_H
#define BT_STATE_H

#include "fps_timer.h"
#include "game.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

enum bt_gpu_buffer {
  bt_gpu_buffer_font_curve,
  bt_gpu_buffer_font_curve_info,
  bt_gpu_buffer_vertex,
  bt_gpu_buffer_instance,
  bt_gpu_buffer_index,
  bt_gpu_buffer_draw,
  /*
   * Number of buffers
   */
  bt_gpu_buffer_count,
};

struct bt_instance_data {
  float scale[3];
  float rotation[4];
  float translation[3];
  uint32_t c;
};
#include <limits.h>
struct bt_state {
  SDL_Window *window;
  SDL_GPUDevice *gpu;
  SDL_GPUShader *vertex_shader;
  SDL_GPUShader *fragment_shader;
  SDL_GPUTransferBuffer *transfer_buffer;
  SDL_GPUBuffer *buffers[bt_gpu_buffer_count];
  uint32_t buffer_sizes[bt_gpu_buffer_count];
  SDL_GPUGraphicsPipeline *graphics_pipeline;
  struct bt_fps_timer fps_timer;
  struct bt_instance_data instance_data[16];
  struct bt_game game;
};

bool bt_state_init(struct bt_state state[static 1]);
void bt_state_deinit(struct bt_state state[static 1]);

void bt_state_handle_keyevent(struct bt_state state[static 1],
                              SDL_KeyboardEvent const event[static 1]);
bool bt_state_render(struct bt_state state[static 1]);
void bt_state_update(struct bt_state state[static 1]);

#endif
