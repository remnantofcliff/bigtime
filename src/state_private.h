#ifndef STATE_PRIVATE_H
#define STATE_PRIVATE_H

#include "state.h"
#include <SDL3/SDL_gpu.h>

constexpr size_t bt_glyph2d_max_instances = 512;
constexpr size_t bt_glyph3d_max_instances = 256;

constexpr SDL_GPUIndexedIndirectDrawCommand bt_draw_data_array[] = {
    {
        .num_indices = 6,
        .num_instances = bt_glyph2d_max_instances,
        .first_index = 0,
        .vertex_offset = 0,
        .first_instance = 0,
    },
    {
        .num_indices = 6,
        .num_instances = bt_glyph3d_max_instances,
        .first_index = 0,
        .vertex_offset = 0,
        .first_instance = 0,
    },
};

constexpr SDL_GPUTextureFormat bt_depth_format =
    SDL_GPU_TEXTUREFORMAT_D16_UNORM;

SDL_GPUTexture *bt_create_depth_texture(struct bt_state state[static 1]);

#endif
