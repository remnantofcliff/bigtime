#ifndef BT_STATE_H
#define BT_STATE_H

#include "game.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

struct bt_state {
  SDL_Window *window;
  SDL_GPUDevice *gpu;
  SDL_GPUShader *vertex_shader;
  SDL_GPUShader *fragment_shader;
  SDL_GPUTransferBuffer *transfer_buffer;
  SDL_GPUBuffer *vertex_buffer;
  SDL_GPUBuffer *index_buffer;
  SDL_GPUBuffer *draw_buffer;
  SDL_GPUGraphicsPipeline *graphics_pipeline;
  struct bt_game game;
};

bool bt_state_init(struct bt_state state[static 1]);
void bt_state_deinit(struct bt_state state[static 1]);

void bt_state_handle_keyevent(struct bt_state state[static 1],
                              SDL_KeyboardEvent const event[static 1]);
bool bt_state_render(struct bt_state state[static 1]);
void bt_state_update(struct bt_state state[static 1]);

#endif
