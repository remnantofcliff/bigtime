#version 450 core

layout (location = 0) in vec2 in_uv;
layout (location = 0) out vec4 out_color;

void main() {
  out_color = vec4(in_uv, 0.0, 1.0);
}
