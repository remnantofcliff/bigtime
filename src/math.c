#include "math.h"
#include <SDL3/SDL_stdinc.h>
#include <math.h>

float bt_recip(float x) { return 1.0f / x; }

struct bt_vec3 bt_cross_product(struct bt_vec3 a, struct bt_vec3 b) {
  return (struct bt_vec3){
      .x = a.y * b.z - a.z * b.y,
      .y = a.z * b.x - a.x * b.z,
      .z = a.x * b.y - a.y * b.x,
  };
}

void bt_look_to(struct bt_mat4 out[restrict static 1],
                struct bt_vec3 const eye[static 1],
                struct bt_vec3 const dir[static 1],
                struct bt_vec3 const up[static 1]) {
  struct bt_vec3 f = bt_vec3_negate(*dir);
  struct bt_vec3 s = bt_vec3_normalize(bt_cross_product(f, *up));
  struct bt_vec3 u = bt_cross_product(s, f);
  out->arr[0] = s.x;
  out->arr[1] = u.x;
  out->arr[2] = -f.x;
  out->arr[3] = 0.0f;
  out->arr[4] = s.y;
  out->arr[5] = u.y;
  out->arr[6] = -f.y;
  out->arr[7] = 0.0f;
  out->arr[8] = s.z;
  out->arr[9] = u.z;
  out->arr[10] = -f.z;
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

struct bt_vec2 bt_vec2_add(struct bt_vec2 a, struct bt_vec2 b) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); i += 1) {
    result.arr[i] = a.arr[i] + b.arr[i];
  }
  return result;
}

float bt_vec2_length(struct bt_vec2 a) {
  return SDL_sqrtf(bt_vec2_sum(bt_vec2_mul(a, a)));
}

struct bt_vec2 bt_vec2_mul(struct bt_vec2 a, struct bt_vec2 b) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); i += 1) {
    result.arr[i] = a.arr[i] * b.arr[i];
  }
  return result;
}

struct bt_vec2 bt_vec2_mulf(struct bt_vec2 a, float b) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); i += 1) {
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
  for (size_t i = 0; i < SDL_arraysize(a.arr); i += 1) {
    sum += a.arr[i];
  }
  return sum;
}

struct bt_vec3 bt_vec3_add(struct bt_vec3 a, struct bt_vec3 b) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); i += 1) {
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

struct bt_vec3 bt_vec3_lerp(struct bt_vec3 from, struct bt_vec3 to, float t) {
  typeof(from) result = {};
  for (size_t i = 0; i < SDL_arraysize(from.arr); i += 1) {
    result.arr[i] = (to.arr[i] - from.arr[i]) * t + from.arr[i];
  }
  return result;
}

struct bt_vec3 bt_vec3_mul(struct bt_vec3 a, struct bt_vec3 b) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); i += 1) {
    result.arr[i] = a.arr[i] * b.arr[i];
  }
  return result;
}

struct bt_vec3 bt_vec3_mulf(struct bt_vec3 a, float b) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); i += 1) {
    result.arr[i] = a.arr[i] * b;
  }
  return result;
}

struct bt_vec3 bt_vec3_negate(struct bt_vec3 a) {
  typeof(a) result = {};
  for (size_t i = 0; i < SDL_arraysize(a.arr); i += 1) {
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
  for (size_t i = 0; i < SDL_arraysize(a.arr); i += 1) {
    sum += a.arr[i];
  }
  return sum;
}

struct bt_vec4 bt_vec4_add(struct bt_vec4 a, struct bt_vec4 b) {
  typeof(a) result = {};
  for (size_t i = 0; i < 4; i += 1) {
    result.arr[i] = a.arr[i] + b.arr[i];
  }
  return result;
}

struct bt_vec4 bt_vec4_mul(struct bt_vec4 a, struct bt_vec4 b) {
  typeof(a) result = {};
  for (size_t i = 0; i < 4; i += 1) {
    result.arr[i] = a.arr[i] * b.arr[i];
  }
  return result;
}

struct bt_vec4 bt_vec4_splat(float x) {
  return (struct bt_vec4){.arr = {x, x, x, x}};
}

void bt_mat4_into_columns(struct bt_mat4 const in[static 1],
                          struct bt_vec4 out[restrict static 4]) {
  for (size_t i = 0; i < 4; i += 1) {
    for (size_t j = 0; j < 4; j += 1) {
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

  for (size_t i = 0; i < 4; i += 1) {
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
  for (size_t i = 0; i < 4; i += 1) {
    bt_mat4_mul_vec4(in1, &in2_cols[i], &res[i]);
  }

  for (size_t i = 0; i < 4; i += 1) {
    for (size_t j = 0; j < 4; j += 1) {
      out->arr[i * 4 + j] = res[i].arr[j];
    }
  }
}
