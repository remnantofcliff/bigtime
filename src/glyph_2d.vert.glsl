#version 460

layout(std140, set = 1, binding = 0) uniform readonly uniforms {
    mat4x4 u_proj_view;
    float u_aspect_ratio;
};

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec2 in_scale;
layout(location = 2) in float in_rotation;
layout(location = 3) in vec2 in_translation;
layout(location = 4) in uint in_char;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec3 out_color;
layout(location = 2) flat out uint out_char;

void main() {
    vec2 pos;
    vec2 uv;
    vec2 scale = in_scale;
    float rotation = in_rotation;
    vec2 translation = in_translation;
    switch (gl_VertexIndex) {
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

    float sin_rot = sin(rotation);
    float cos_rot = cos(rotation);
    mat3 model = mat3(
        vec3(vec2(cos_rot, sin_rot) * scale.x, 0.0),
        vec3(vec2(-sin_rot, cos_rot * u_aspect_ratio) * scale.y, 0.0),
        vec3(translation, 0.0)
    );

    gl_Position = vec4(model * vec3(pos, 1.0), 1.0);
}
