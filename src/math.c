#include "math.h"
#include <SDL3/SDL_stdinc.h>
#include <math.h>

float bt_recip(float x) { return 1.0f / x; }

struct bt_vec3 bt_cross_product(struct bt_vec3 a, struct bt_vec3 b) {
  return (struct bt_vec3){{
      a.arr[1] * b.arr[2] - a.arr[2] * b.arr[1],
      a.arr[2] * b.arr[0] - a.arr[0] * b.arr[2],
      a.arr[0] * b.arr[1] - a.arr[1] * b.arr[0],
  }};
}

void bt_look_to(struct bt_mat4 out[restrict static 1],
                struct bt_vec3 const eye[static 1],
                struct bt_vec3 const dir[static 1],
                struct bt_vec3 const up[static 1]) {
  struct bt_vec3 f = bt_vec3_negate(*dir);
  struct bt_vec3 s = bt_vec3_normalize(bt_cross_product(f, *up));
  struct bt_vec3 u = bt_cross_product(s, f);
  out->arr[0] = s.arr[0];
  out->arr[1] = u.arr[0];
  out->arr[2] = -f.arr[0];
  out->arr[3] = 0.0f;
  out->arr[4] = s.arr[1];
  out->arr[5] = u.arr[1];
  out->arr[6] = -f.arr[1];
  out->arr[7] = 0.0f;
  out->arr[8] = s.arr[2];
  out->arr[9] = u.arr[2];
  out->arr[10] = -f.arr[2];
  out->arr[11] = 0.0f;
  out->arr[12] = -bt_vec3_dot(*eye, s);
  out->arr[13] = -bt_vec3_dot(*eye, u);
  out->arr[14] = bt_vec3_dot(*eye, f);
  out->arr[15] = 1.0f;
}

void bt_perspective(struct bt_mat4 out[static 1], float fovy,
                    float aspect_ratio, float near, float far) {
  float sin_fov = SDL_sinf(0.5f * fovy);
  float cos_fov = SDL_cosf(0.5f * fovy);
  float h = cos_fov / sin_fov;
  float w = h / aspect_ratio;
  float r = far / (far - near);
  out->arr[0] = w;
  out->arr[1] = 0.0f;
  out->arr[2] = 0.0f;
  out->arr[3] = 0.0f;
  out->arr[4] = 0.0f;
  out->arr[5] = h;
  out->arr[6] = 0.0f;
  out->arr[7] = 0.0f;
  out->arr[8] = 0.0f;
  out->arr[9] = 0.0f;
  out->arr[10] = r;
  out->arr[11] = 1.0f;
  out->arr[12] = 0.0f;
  out->arr[13] = 0.0f;
  out->arr[14] = -r * near;
  out->arr[15] = 0.0f;
}

float bt_vec2_length(struct bt_vec2 a) {
  return SDL_sqrtf(bt_vec2_sum(bt_vec2_mul(a, a)));
}

struct bt_vec2 bt_vec2_mul(struct bt_vec2 a, struct bt_vec2 b) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); ++i) {
    result.arr[i] = a.arr[i] * b.arr[i];
  }
  return result;
}

struct bt_vec2 bt_vec2_mulf(struct bt_vec2 a, float b) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); ++i) {
    result.arr[i] = a.arr[i] * b;
  }
  return result;
}

struct bt_vec2 bt_vec2_normalize(struct bt_vec2 a) {
  return bt_vec2_mulf(a, bt_recip(bt_vec2_length(a)));
}

struct bt_vec2 bt_vec2_normalize_or_zero(struct bt_vec2 a) {
  float recip_len = bt_recip(bt_vec2_length(a));
  if (!isfinite(recip_len) && recip_len > 0.0f) {
    return (typeof(a)){};
  }

  return bt_vec2_mulf(a, recip_len);
}

float bt_vec2_sum(struct bt_vec2 a) {
  float sum = 0.0f;
  for (size_t i = 0; i < SDL_arraysize(a.arr); ++i) {
    sum += a.arr[i];
  }
  return sum;
}

struct bt_vec3 bt_vec3_add(struct bt_vec3 a, struct bt_vec3 b) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); ++i) {
    result.arr[i] = a.arr[i] + b.arr[i];
  }
  return result;
}

float bt_vec3_dot(struct bt_vec3 a, struct bt_vec3 b) {
  return bt_vec3_sum(bt_vec3_mul(a, b));
}

float bt_vec3_length(struct bt_vec3 a) {
  return SDL_sqrtf(bt_vec3_sum(bt_vec3_mul(a, a)));
}

struct bt_vec3 bt_vec3_lerp(struct bt_vec3 a, struct bt_vec3 b, float t) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); ++i) {
    result.arr[i] = (b.arr[i] - a.arr[i]) * t + a.arr[i];
  }
  return result;
}

struct bt_vec3 bt_vec3_mul(struct bt_vec3 a, struct bt_vec3 b) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); ++i) {
    result.arr[i] = a.arr[i] * b.arr[i];
  }
  return result;
}

struct bt_vec3 bt_vec3_mulf(struct bt_vec3 a, float b) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); ++i) {
    result.arr[i] = a.arr[i] * b;
  }
  return result;
}

struct bt_vec3 bt_vec3_negate(struct bt_vec3 a) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); ++i) {
    result.arr[i] = -a.arr[i];
  }
  return result;
}

struct bt_vec3 bt_vec3_normalize(struct bt_vec3 a) {
  return bt_vec3_mulf(a, bt_recip(bt_vec3_length(a)));
}

struct bt_vec3 bt_vec3_normalize_or_zero(struct bt_vec3 a) {
  float recip_len = bt_recip(bt_vec3_length(a));
  if (!isfinite(recip_len) && recip_len > 0.0f) {
    return (typeof(a)){};
  }

  return bt_vec3_mulf(a, recip_len);
}

float bt_vec3_sum(struct bt_vec3 a) {
  float sum = 0.0f;
  for (size_t i = 0; i < SDL_arraysize(a.arr); ++i) {
    sum += a.arr[i];
  }
  return sum;
}

struct bt_vec4 bt_vec4_add(struct bt_vec4 a, struct bt_vec4 b) {
  typeof(a) result = {};
  for (size_t i = 0; i < 4; ++i) {
    result.arr[i] = a.arr[i] + b.arr[i];
  }
  return result;
}

struct bt_vec4 bt_vec4_mul(struct bt_vec4 a, struct bt_vec4 b) {
  typeof(a) result = {};
  for (size_t i = 0; i < 4; ++i) {
    result.arr[i] = a.arr[i] * b.arr[i];
  }
  return result;
}

struct bt_vec4 bt_vec4_splat(float x) {
  return (struct bt_vec4){{
      x,
      x,
      x,
      x,
  }};
}

void bt_mat4_into_columns(struct bt_mat4 const in[static 1],
                          struct bt_vec4 out[restrict static 4]) {
  for (size_t i = 0; i < 4; ++i) {
    for (size_t j = 0; j < 4; ++j) {
      out[i].arr[j] = in->arr[j + i * 4];
    }
  }
}

void bt_mat4_mul_vec4(struct bt_mat4 const in1[static 1],
                      struct bt_vec4 const in2[static 1],
                      struct bt_vec4 out[restrict static 1]) {
  SDL_zerop(out);

  struct bt_vec4 in1_cols[4] = {};
  bt_mat4_into_columns(in1, in1_cols);

  for (size_t i = 0; i < 4; ++i) {
    *out =
        bt_vec4_add(*out, bt_vec4_mul(in1_cols[i], bt_vec4_splat(in2->arr[i])));
  }
}

void bt_mat4_mul(struct bt_mat4 const in1[static 1],
                 struct bt_mat4 const in2[static 1],
                 struct bt_mat4 out[restrict static 1]) {
  struct bt_vec4 in2_cols[4] = {};
  bt_mat4_into_columns(in2, in2_cols);

  struct bt_vec4 res[4] = {};
  for (size_t i = 0; i < 4; ++i) {
    bt_mat4_mul_vec4(in1, &in2_cols[i], &res[i]);
  }

  for (size_t i = 0; i < 4; ++i) {
    for (size_t j = 0; j < 4; ++j) {
      out->arr[i * 4 + j] = res[i].arr[j];
    }
  }
}
