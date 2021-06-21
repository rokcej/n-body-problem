#version 300 es

in vec4 aPosSize;

uniform vec2 uResolution;
uniform mat4 uPMat;
uniform mat4 uVMMat;

void main() {
	vec3 pos = aPosSize.xyz;
	float size = aPosSize.w;
	vec4 eyePos = uVMMat * vec4(pos, 1.0);
	vec4 projVoxel = uPMat * vec4(0.1, 0.0, eyePos.zw);

	gl_Position = uPMat * eyePos;
	gl_PointSize = 0.5 * size * uResolution.x * projVoxel.x / projVoxel.w;
}
