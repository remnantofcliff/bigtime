#version 450

layout (std140, set = 1, binding = 0) uniform readonly uniforms {
  mat4x4 u_projView;
};

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_color;

layout (location = 0) out vec2 out_uv;
layout (location = 1) out vec3 out_color;

void main() {
  out_uv = in_uv;
  out_color = in_color;
  gl_Position = u_projView * vec4(in_pos, 1.0);
}
