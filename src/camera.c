#include "camera.h"

void bt_camera_update(struct bt_camera camera[static 1],
                      struct bt_input const input[static 1]) {
  camera->eye.arr[2] +=
      (float)input->moving_forwards - (float)input->moving_backwards;
  camera->eye.arr[0] += (float)input->moving_right - (float)input->moving_left;
}
