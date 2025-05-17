#ifndef BT_TIME_H
#define BT_TIME_H

#include <stdint.h>

constexpr uint64_t bt_updates_per_sec = 100;
constexpr float bt_dt = 1.0f / (float)bt_updates_per_sec;
constexpr uint64_t bt_time_between_updates = 1'000'000'000 / bt_updates_per_sec;

struct bt_time {
  uint64_t current_time;
  uint64_t last_time;
  uint64_t accumulator;
};

void bt_time_init(struct bt_time time[static 1]);
void bt_time_start_loop(struct bt_time time[static 1]);
bool bt_time_should_update(struct bt_time time[static 1]);
void bt_time_update(struct bt_time time[static 1]);
void bt_time_end_loop(struct bt_time time[static 1]);

#endif
