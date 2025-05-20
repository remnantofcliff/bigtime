/*
 * All matrices are column major
 *
 */
#ifndef BT_MATH_H
#define BT_MATH_H

struct bt_vec2 {
  float arr[2];
};

struct bt_vec3 {
  float arr[3];
};

struct bt_vec4 {
  float arr[4];
};

/*
 * A 4x4 matrix with column-major order
 */
struct bt_mat4 {
  float arr[16];
};

constexpr float bt_pi = 3.1415927410125732421875;
constexpr struct bt_mat4 bt_mat4_identity = {
    {
        [0] = 1.0f,
        [5] = 1.0f,
        [10] = 1.0f,
        [15] = 1.0f,
    },
};

float bt_recip(float x);
struct bt_vec3 bt_cross_product(struct bt_vec3 a, struct bt_vec3 b);
/*
 * Computes a left-handed view matrix that looks from `eye` to direc  `dir`.
 * `dir` must be normalized
 */
void bt_look_to(struct bt_mat4 out[restrict static 1],
                struct bt_vec3 const eye[static 1],
                struct bt_vec3 const dir[static 1],
                struct bt_vec3 const up[static 1]);
/*
 * Computes a left-handed projection matrix using the provided values
 */
void bt_perspective(struct bt_mat4 out[static 1], float fovy,
                    float aspect_ratio, float near, float far);

float bt_vec2_length(struct bt_vec2 a);
struct bt_vec2 bt_vec2_mul(struct bt_vec2 a, struct bt_vec2 b);
struct bt_vec2 bt_vec2_mulf(struct bt_vec2 a, float b);
/*
 * Length needs to be > 0.0f or you will get garbage results
 */
struct bt_vec2 bt_vec2_normalize(struct bt_vec2 a);
struct bt_vec2 bt_vec2_normalize_or_zero(struct bt_vec2 a);
float bt_vec2_sum(struct bt_vec2 a);

struct bt_vec3 bt_vec3_add(struct bt_vec3 a, struct bt_vec3 b);
float bt_vec3_dot(struct bt_vec3 a, struct bt_vec3 b);
float bt_vec3_length(struct bt_vec3 a);
struct bt_vec3 bt_vec3_lerp(struct bt_vec3 a, struct bt_vec3 b, float t);
struct bt_vec3 bt_vec3_mul(struct bt_vec3 a, struct bt_vec3 b);
struct bt_vec3 bt_vec3_mulf(struct bt_vec3 a, float b);
struct bt_vec3 bt_vec3_negate(struct bt_vec3 a);
/*
 * Length needs to be > 0.0f or you will get garbage results
 */
struct bt_vec3 bt_vec3_normalize(struct bt_vec3 a);
struct bt_vec3 bt_vec3_normalize_or_zero(struct bt_vec3 a);
float bt_vec3_sum(struct bt_vec3 a);

struct bt_vec4 bt_vec4_add(struct bt_vec4 a, struct bt_vec4 b);
struct bt_vec4 bt_vec4_mul(struct bt_vec4 a, struct bt_vec4 b);
struct bt_vec4 bt_vec4_splat(float x);

void bt_mat4_into_columns(struct bt_mat4 const in[static 1],
                          struct bt_vec4 out[restrict static 4]);
void bt_mat4_mul_vec4(struct bt_mat4 const in1[static 1],
                      struct bt_vec4 const in2[static 1],
                      struct bt_vec4 out[restrict static 1]);
void bt_mat4_mul(struct bt_mat4 const in1[static 1],
                 struct bt_mat4 const in2[static 1],
                 struct bt_mat4 out[restrict static 1]);

#endif
