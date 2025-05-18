#version 450 core

layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_color;
layout (location = 0) out vec4 out_color;

void main() {
  out_color = vec4(in_color, 1.0);
}
