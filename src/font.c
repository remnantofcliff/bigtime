// #ifndef BT_FONT_H
// #define BT_FONT_H
//
// #include "data.h"
// #include "stb_truetype.h"
// #include <SDL3/SDL_log.h>
// #include <stdckdint.h>
//
// struct bt_font {
//   stbtt_fontinfo info;
// };
//
// #endif
//
// bool bt_font_init(struct bt_font font[static 1]) {
//   unsigned char const *data = nullptr;
//   if (!stbtt_InitFont(&font->info, data,
//                       stbtt_GetFontOffsetForIndex(data, 0))) {
//     return false;
//   }
//
//   for (int i = U'\0'; i < (int)U'\x10FFFF'; ++i) {
//     int glyph_index = stbtt_FindGlyphIndex(&font->info, i);
//     if (glyph_index == 0) {
//       // Special case
//     }
//
//     stbtt_vertex **vertices = SDL_calloc(U'\x10FFFF' - '\0', sizeof(*vertices));
//     size_t total_verts = 0;
//     for (int j = 0; j < 0; ++j) {
//       int num_verts =
//           stbtt_GetGlyphShape(&font->info, glyph_index, &vertices[j]);
//       if (ckd_add(&total_verts, total_verts, num_verts)) {
//         SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Too many vertices in font");
//         return false;
//       }
//     }
//
//     struct bt_font_curve *curves = SDL_calloc();
//
//     // for (int j = 0; j < num_verts; ++j) {
//     //   switch (vertices[i].type) {
//     //   case STBTT_vmove:
//     //     break;
//     //   case STBTT_vline:
//     //     break;
//     //   case STBTT_vcurve:
//     //     break;
//     //   case STBTT_vcubic:
//     //     break;
//     //   default:
//     //     SDL_LogError(
//     //         SDL_LOG_CATEGORY_ERROR,
//     //         "Got unknown vertex type when going over font glyph vertices");
//     //     return false;
//     //   }
//   }
//   stbtt_FreeShape(&font->info, vertices);
// }
//
// return true;
// }
