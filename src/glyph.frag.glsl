#version 460 core

struct bt_font_curve {
    vec2 p0;
    vec2 p1;
    vec2 p2;
};

struct bt_font_curve_info {
    uint start;
    uint end;
};

layout(std140, set = 2, binding = 0) readonly buffer bt_font_curves {
    bt_font_curve curves[];
};

layout(std140, set = 2, binding = 1) readonly buffer bt_font_curve_infos {
    bt_font_curve_info curve_infos[];
};

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec3 in_color;
layout(location = 2) flat in uint in_char;
layout(location = 0) out vec4 out_color;

float compute_coverage(float inverse_diameter, vec2 p0, vec2 p1, vec2 p2) {
    vec2 a = p0 - 2.0 * p1 + p2;
    vec2 b = p0 - p1;
    vec2 c = p0;

    float t0;
    float t1;

    if (abs(a.y) >= 1e-5) {
        float radicand = b.y * b.y - a.y * c.y;
        if (radicand <= 0)
            return 0.0;

        float s = sqrt(radicand);
        t0 = (b.y - s) / a.y;
        t1 = (b.y + s) / a.y;
    } else {
        float t = p0.y / (p0.y - p2.y);
        if (p0.y < p2.y) {
            t0 = -1.0;
            t1 = t;
        } else {
            t0 = t;
            t1 = -1.0;
        }
    }

    float alpha = 0.0;

    if (t0 >= 0.0 && t0 < 1.0) {
        float x = (a.x * t0 - 2.0 * b.x) * t0 + c.x;
        alpha += clamp(x * inverse_diameter + 0.5, 0, 1);
    }

    if (t1 >= 0.0 && t1 < 1.0) {
        float x = (a.x * t1 - 2.0 * b.x) * t1 + c.x;
        alpha -= clamp(x * inverse_diameter + 0.5, 0, 1);
    }

    return alpha;
}

void main() {
    vec2 uv = in_uv;
    vec3 color = in_color;
    uint char = in_char;

    float alpha = 0.0;
    float inverse_diameter = 1.0 / fwidth(uv).x;
    bt_font_curve_info info = curve_infos[char];
    for (uint i = info.start; i < info.end; i += 1) {
        bt_font_curve curve = curves[i];
        vec2 uv = uv;
        vec2 p0 = curve.p0 - uv;
        vec2 p1 = curve.p1 - uv;
        vec2 p2 = curve.p2 - uv;
        float min_y = min(min(p0.y, p1.y), p2.y);
        float max_y = max(max(p0.y, p1.y), p2.y);
        if (min_y > 0.0 || max_y < 0.0) {
            continue;
        }
        alpha += compute_coverage(inverse_diameter, p0, p1, p2);
    }

    // Discard so that the transparent pixels of the quad don't overlap with
    // other quads and cause depth issues
    if (alpha <= 0.0)
        discard;

    out_color = vec4(color, min(alpha, 1.0));
}
