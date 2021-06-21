#version 300 es

in vec4 aPos;
uniform mat4 uPVMMat;
uniform mat4 uVMMat;

void main() {
	gl_Position = uPVMMat * aPos;
}
