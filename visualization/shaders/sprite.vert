#version 300 es

in vec4 aPos;
uniform vec2 uResolution;
uniform mat4 uPMat;
uniform mat4 uVMMat;

void main() {
	vec4 eyePos = uVMMat * aPos;
	vec4 projVoxel = uPMat * vec4(0.1, 0.0, eyePos.zw);

	gl_Position = uPMat * eyePos;
	gl_PointSize = 0.5 * uResolution.x * projVoxel.x / projVoxel.w;
}
