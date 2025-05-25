#version 460

layout(std140, set = 1, binding = 0) uniform readonly uniforms {
    mat4x4 u_proj_view;
    float u_aspect_ratio;
};

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_scale;
layout(location = 2) in vec4 in_rotation;
layout(location = 3) in vec3 in_translation;
layout(location = 4) in uint in_char;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec3 out_color;
layout(location = 2) flat out uint out_char;

mat3 quat_to_mat3(vec4 quat) {
    float x = quat.x;
    float y = quat.y;
    float z = quat.z;
    float w = quat.w;

    float x2 = x + x;
    float y2 = y + y;
    float z2 = z + z;

    float xx = x * x2;
    float yy = y * y2;
    float zz = z * z2;
    float xy = x * y2;
    float xz = x * z2;
    float yz = y * z2;
    float wx = w * x2;
    float wy = w * y2;
    float wz = w * z2;

    return mat3(vec3(1.0 - (yy + zz), xy + wz, xz - wy), vec3(xy - wz, 1.0 - (xx + zz), yz + wx), vec3(xz + wy, yz - wx, 1.0 - (xx + yy)));
}

void main() {
    vec2 pos;
    vec2 uv;
    vec3 scale = in_scale;
    vec4 rotation = in_rotation;
    vec3 translation = in_translation;
    switch (gl_VertexIndex % 4) {
        case 0:
        pos = vec2(-0.5f, 0.5f);
        uv = vec2(0.0f, 0.0f);
        break;
        case 1:
        pos = vec2(0.5f, 0.5f);
        uv = vec2(1.0f, 0.0f);
        break;
        case 2:
        pos = vec2(-0.5f, -0.5f);
        uv = vec2(0.0f, 1.0f);
        break;
        case 3:
        pos = vec2(0.5f, -0.5f);
        uv = vec2(1.0f, 1.0f);
        break;
    }
    out_uv = uv;
    out_color = in_color;
    out_char = in_char;
    mat3 rot_mat = quat_to_mat3(rotation);
    mat4 model = mat4(vec4(rot_mat[0] * scale.x, 0.0), vec4(rot_mat[1] * scale.y, 0.0), vec4(rot_mat[2] * scale.z, 0.0), vec4(translation, 1.0));
    gl_Position = u_proj_view * model * vec4(pos, 0.0, 1.0);
}
