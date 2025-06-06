#ifndef BT_CAMERA_H
#define BT_CAMERA_H

#include "input.h"

struct bt_camera {
  struct bt_vec3 eye;
  struct bt_vec3 dir;
  float yaw;
  float pitch;
};

constexpr struct bt_camera bt_default_camera = {
    .eye.z = -2.0f,
    .yaw = bt_pi,
};

void bt_camera_update(struct bt_camera camera[static 1],
                      struct bt_input const input[static 1]);

#endif
