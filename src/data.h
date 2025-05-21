#ifndef BT_DATA_H
#define BT_DATA_H

#include <stdint.h>

struct bt_font_curve {
  alignas(16) float p0[2];
  float p1[2];
  float p2[2];
};

struct bt_font_curve_info {
  alignas(16) uint32_t start;
  uint32_t count;
};

struct bt_font_metrics {
  float advance;
};

extern uint32_t const *const bt_vertex_shader_spirv;
extern uint32_t const *const bt_fragment_shader_spirv;
extern struct bt_font_curve const *const bt_font_curves;
extern struct bt_font_curve_info const *const bt_font_curve_infos;
extern struct bt_font_metrics const *const bt_font_metrics;

extern uint32_t const bt_vertex_shader_spirv_byte_size;
extern uint32_t const bt_fragment_shader_spirv_byte_size;
extern uint32_t const bt_font_curves_byte_size;
extern uint32_t const bt_font_curve_infos_byte_size;
extern uint32_t const bt_font_metrics_byte_size;

extern uint32_t const bt_vertex_shader_spirv_len;
extern uint32_t const bt_fragment_shader_spirv_len;
extern uint32_t const bt_font_curves_len;
extern uint32_t const bt_font_curve_infos_len;
extern uint32_t const bt_font_metrics_len;

#endif
