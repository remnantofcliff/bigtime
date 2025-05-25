#include "data.h"

// clang-format off
static unsigned char const bt_glyph2d_vertex_spirv_bytes[] = {
#embed <glyph_2d.vert.spv>
};
static unsigned char const bt_glyph3d_vertex_spirv_bytes[] = {
#embed <glyph_3d.vert.spv>
};
static unsigned char const bt_glyph_fragment_spirv_bytes[] = {
#embed <glyph.frag.spv>
};
static alignas(alignof(struct bt_font_curve)) unsigned char const bt_font_curve_bytes[] = {
#embed <glyph_buffer.data>
};
static alignas(alignof(struct bt_font_curve_info)) unsigned char const bt_font_curve_info_bytes[] = {
#embed <info_buffer.data>
};
static alignas(alignof(struct bt_font_metrics)) unsigned char const bt_font_metrics_bytes[] = {
#embed <metrics_buffer.data>
};

uint32_t const *const bt_glyph2d_vertex_spirv = (uint32_t const *)bt_glyph2d_vertex_spirv_bytes;
uint32_t const *const bt_glyph3d_vertex_spirv = (uint32_t const *)bt_glyph3d_vertex_spirv_bytes;
uint32_t const *const bt_glyph_fragment_spirv = (uint32_t const *)bt_glyph_fragment_spirv_bytes;
struct bt_font_curve const *const bt_font_curves = (struct bt_font_curve const *)bt_font_curve_bytes;
struct bt_font_curve_info const *const bt_font_curve_infos = (struct bt_font_curve_info const *)bt_font_curve_info_bytes;
struct bt_font_metrics const *const bt_font_metrics = (struct bt_font_metrics const *)bt_font_metrics_bytes;

uint32_t const bt_glyph2d_vertex_spirv_byte_size = sizeof(bt_glyph2d_vertex_spirv_bytes);
uint32_t const bt_glyph3d_vertex_spirv_byte_size = sizeof(bt_glyph3d_vertex_spirv_bytes);
uint32_t const bt_glyph_fragment_spirv_byte_size = sizeof(bt_glyph_fragment_spirv_bytes);
uint32_t const bt_font_curves_byte_size = sizeof(bt_font_curve_bytes);
uint32_t const bt_font_curve_infos_byte_size = sizeof(bt_font_curve_info_bytes);
uint32_t const bt_font_metrics_byte_size = sizeof(bt_font_metrics_bytes);

uint32_t const bt_glyph2d_vertex_spirv_len = bt_glyph2d_vertex_spirv_byte_size / sizeof(*bt_glyph2d_vertex_spirv);
uint32_t const bt_glyph3d_vertex_spirv_len = bt_glyph3d_vertex_spirv_byte_size / sizeof(*bt_glyph3d_vertex_spirv);
uint32_t const bt_glyph_fragment_spirv_len = bt_glyph_fragment_spirv_byte_size / sizeof(*bt_glyph_fragment_spirv);
uint32_t const bt_font_curves_len = bt_font_curves_byte_size / sizeof(*bt_font_curves);
uint32_t const bt_font_curve_infos_len = bt_font_curve_infos_byte_size / sizeof(*bt_font_curve_infos);
uint32_t const bt_font_metrics_len = bt_font_metrics_byte_size / sizeof(*bt_font_metrics);

