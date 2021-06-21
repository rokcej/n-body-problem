#version 300 es
precision highp float;

out vec4 oColor;

void main() {
	float radius = length(gl_PointCoord * 2.0 - 1.0);
	if (radius > 1.0)
		discard;
	oColor = vec4(1.0, 0.5, 0.2, 1.0);
}
