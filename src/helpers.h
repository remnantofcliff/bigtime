#ifndef BT_HELPERS_H
#define BT_HELPERS_H

#include <SDL3/SDL_log.h>

#define BT_LOG_SDL_FAIL(fmt, ...)                                              \
  SDL_LogError(SDL_LOG_CATEGORY_ERROR, fmt "; got error: %s",                  \
               __VA_ARGS__ __VA_OPT__(, ) SDL_GetError())

#endif
