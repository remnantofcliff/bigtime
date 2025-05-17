#ifndef BT_INPUT_H
#define BT_INPUT_H

struct bt_input {
  bool moving_forwards : 1;
  bool moving_backwards : 1;
  bool moving_left : 1;
  bool moving_right : 1;
};

#endif
