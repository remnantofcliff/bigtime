#include "camera.h"
#include "time.h"

constexpr float movement_speed = 1.0f * bt_dt;

void bt_camera_update(struct bt_camera camera[static 1],
                      struct bt_input const input[static 1]) {
  struct bt_vec2 movement = {};

  movement.arr[0] += ((float)input->moving_right - (float)input->moving_left);
  movement.arr[1] +=
      ((float)input->moving_forwards - (float)input->moving_backwards);

  movement = bt_vec2_normalize_or_zero(movement);
  movement = bt_vec2_mulf(movement, movement_speed);

  camera->eye.arr[0] += movement.arr[0];
  camera->eye.arr[2] += movement.arr[1];
}
