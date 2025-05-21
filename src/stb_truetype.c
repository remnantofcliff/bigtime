#include <SDL3/SDL_stdinc.h>

double floor(double arg) {
  return SDL_floor(arg);
}
double acos(double arg) {
  return SDL_acos(arg);
}
double sqrt(double arg) {
  return SDL_sqrt(arg);
}
double ceil(double arg) {
  return SDL_ceil(arg);
}
double cos(double arg) {
  return SDL_cos(arg);
}
double pow(double base, double exponent) {
  return SDL_pow(base, exponent);
}
double fmod(double x, double y) {
  return SDL_fmod(x, y);
}

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

