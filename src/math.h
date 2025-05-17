/*
 * All matrices are column major
 *
 */
#ifndef BT_MATH_H
#define BT_MATH_H

struct bt_vec3 {
  float arr[3];
};

struct bt_vec4 {
  float arr[4];
};

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

struct bt_vec3 bt_cross_product(struct bt_vec3 a, struct bt_vec3 b);

// Normal operations

struct bt_vec3 bt_vec3_divf(struct bt_vec3 a, float b);
float bt_vec3_dot(struct bt_vec3 a, struct bt_vec3 b);
float bt_vec3_length(struct bt_vec3 x);
struct bt_vec3 bt_vec3_lerp(struct bt_vec3 a, struct bt_vec3 b, float t);
struct bt_vec3 bt_vec3_negate(struct bt_vec3 x);
struct bt_vec3 bt_vec3_normalize(struct bt_vec3 x);
struct bt_vec4 bt_vec4_add_vec4(struct bt_vec4 a, struct bt_vec4 b);
struct bt_vec4 bt_vec4_mul_vec4(struct bt_vec4 a, struct bt_vec4 b);
struct bt_vec4 bt_vec4_splat(float x);
void bt_mat4_into_columns(struct bt_mat4 const in[static 1],
                          struct bt_vec4 out[restrict static 4]);
void bt_mat4_mul_vec4(struct bt_mat4 const in1[static 1],
                      struct bt_vec4 const in2[static 1],
                      struct bt_vec4 out[restrict static 1]);
void bt_mat4_mul_mat4(struct bt_mat4 const in1[static 1],
                      struct bt_mat4 const in2[static 1],
                      struct bt_mat4 out[restrict static 1]);

// Special operations

/*
 * Left handed
 */
void bt_look_to(struct bt_mat4 out[restrict static 1],
                struct bt_vec3 const eye[static 1],
                struct bt_vec3 const dir[static 1],
                struct bt_vec3 const up[static 1]);
/*
 * Left handed
 */
void bt_perspective(struct bt_mat4 out[static 1], float fovy,
                    float aspect_ratio, float near, float far);

#endif
