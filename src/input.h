#ifndef BT_INPUT_H
#define BT_INPUT_H

#include "math.h"

struct bt_input {
  struct {
    bool moving_forwards : 1;
    bool moving_backwards : 1;
    bool moving_left : 1;
    bool moving_right : 1;
    bool view_moving_up : 1;
    bool view_moving_down : 1;
    bool view_moving_left : 1;
    bool view_moving_right : 1;
  } kbd;
  struct {
    struct bt_vec2 diff;
  } mouse;
};

#endif
