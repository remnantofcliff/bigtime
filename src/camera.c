#include "camera.h"
#include "time.h"

constexpr float movement_speed = 1.0f * bt_dt;
constexpr float kbd_dir_speed = 1.0f * bt_dt;

#include <SDL3/SDL_log.h>

void bt_camera_update(struct bt_camera camera[static 1],
                      struct bt_input const input[static 1]) {
  struct bt_vec2 movement = {};

  movement.arr[0] +=
      (float)input->kbd.moving_right - (float)input->kbd.moving_left;
  movement.arr[1] +=
      (float)input->kbd.moving_forwards - (float)input->kbd.moving_backwards;

  movement = bt_vec2_normalize_or_zero(movement);
  movement = bt_vec2_mulf(movement, movement_speed);

  camera->eye.arr[0] += movement.arr[0];
  camera->eye.arr[2] += movement.arr[1];

  camera->yaw += ((float)input->kbd.view_moving_left -
                  (float)input->kbd.view_moving_right) *
                 kbd_dir_speed;
  camera->pitch +=
      ((float)input->kbd.view_moving_up - (float)input->kbd.view_moving_down) *
      kbd_dir_speed;

  constexpr float pitch_clamp_value = bt_pi * 0.5f - bt_pi / 100.0f;
  camera->pitch =
      SDL_clamp(camera->pitch, -pitch_clamp_value, pitch_clamp_value);
  camera->yaw = SDL_fmodf(camera->yaw, 2.0f * bt_pi);
  float sin_yaw = SDL_sinf(camera->yaw);
  float cos_yaw = SDL_cosf(camera->yaw);
  float sin_pitch = SDL_sinf(camera->pitch);
  float cos_pitch = SDL_cosf(camera->pitch);
  camera->dir.arr[0] = cos_pitch * sin_yaw;
  camera->dir.arr[1] = sin_pitch;
  camera->dir.arr[2] = -cos_pitch * cos_yaw;
  camera->dir = bt_vec3_normalize(camera->dir);
}
