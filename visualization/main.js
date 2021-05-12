"use strict";

const GLM = glMatrix;

var vertexShaderSource = `#version 300 es

// an attribute is an input (in) to a vertex shader.
// It will receive data from a buffer
in vec4 a_position;
uniform mat4 uPVM;


// all shaders have a main function
void main() {

	// gl_Position is a special variable a vertex shader
	// is responsible for setting
	gl_Position = uPVM * a_position;
}
`;

var fragmentShaderSource = `#version 300 es

// fragment shaders don't have a default precision so we need
// to pick one. highp is a good default. It means "high precision"
precision highp float;

// we need to declare an output for the fragment shader
out vec4 outColor;

void main() {
	// Just set the output to a constant redish-purple
	outColor = vec4(1, 0, 0.5, 1);
}
`;

function createShader(gl, type, source) {
	var shader = gl.createShader(type);
	gl.shaderSource(shader, source);
	gl.compileShader(shader);
	var success = gl.getShaderParameter(shader, gl.COMPILE_STATUS);
	if (success) {
		return shader;
	}

	console.log(gl.getShaderInfoLog(shader));  // eslint-disable-line
	gl.deleteShader(shader);
	return undefined;
}

function createProgram(gl, vertexShader, fragmentShader) {
	var program = gl.createProgram();
	gl.attachShader(program, vertexShader);
	gl.attachShader(program, fragmentShader);
	gl.linkProgram(program);
	var success = gl.getProgramParameter(program, gl.LINK_STATUS);
	if (success) {
		return program;
	}

	console.log(gl.getProgramInfoLog(program));  // eslint-disable-line
	gl.deleteProgram(program);
	return undefined;
}

class App {
	constructor(canvas) {
		window.app = this;

		this.canvas = canvas;
		this.gl = canvas.getContext("webgl2");
		if (!this.gl) {
			console.log("WebGL2 not supported");
			return;
		}

		let vertexShader = createShader(this.gl, this.gl.VERTEX_SHADER, vertexShaderSource);
		let fragmentShader = createShader(this.gl, this.gl.FRAGMENT_SHADER, fragmentShaderSource);

		this.program = createProgram(this.gl, vertexShader, fragmentShader);

		let posLocation = this.gl.getAttribLocation(this.program, "a_position");

		let posBuffer = this.gl.createBuffer();
		this.gl.bindBuffer(this.gl.ARRAY_BUFFER, posBuffer);

		var positions = [
			0, 0, 0,
			0, 0.5, 0,
			0.7, 0, 0,
			0.7, 0.5, 0,
			0, 0.5, 0,
			0.7, 0,	0
		];
		this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(positions), this.gl.STATIC_DRAW);

		this.vao = this.gl.createVertexArray();
		this.gl.bindVertexArray(this.vao);
		this.gl.enableVertexAttribArray(posLocation);

		this.gl.vertexAttribPointer(posLocation, 3, this.gl.FLOAT, false, 0, 0);

		this.gl.viewport(0, 0, this.gl.canvas.width, this.gl.canvas.height);


		this.camera = {
			rot: GLM.vec2.fromValues(0.5 * Math.PI, 0.5 * Math.PI),
			r: 1,
			aspect: 16 / 9,
			fov: Math.PI / 2,
			projMat: GLM.mat4.create()
		}
		this.mouse = {
			x: 0, y: 0,
			dragging: false
		}
		this.canvas.addEventListener("mousedown", (event) => {
			this.mouse.x = event.clientX;
			this.mouse.y = event.clientY;
			this.mouse.dragging = true;
		});
		this.canvas.addEventListener("mouseup", (event) => {
			this.mouse.dragging = false;
		});
		this.canvas.addEventListener("mousemove", (event) => {
			if (this.mouse.dragging) {
				let x = event.clientX, y = event.clientY;
				let dx = x - this.mouse.x;
				let dy = y - this.mouse.y;
				this.mouse.x = event.clientX;
				this.mouse.y = event.clientY;

				this.camera.rot[0] += dx * 0.005;
				this.camera.rot[1] -= dy * 0.005;

				if (this.camera.rot[1] > Math.PI)
					this.camera.rot[1] = Math.PI
				else if (this.camera.rot[1] < 0)
					this.camera.rot[1] = 0;
			}
		});
		this.canvas.addEventListener("wheel", (event) => {
			const factor = 1.05;
			if (event.deltaY < 0) {
				this.camera.r /= factor;
			} else if (event.deltaY > 0) {
				this.camera.r *= factor;
			}
		});
		this.resize();
		window.addEventListener("resize", () => { this.resize(); });

		window.requestAnimationFrame(() => { this.update(); });
	}

	update() {
		this.gl.clearColor(0, 0, 0, 255);
		this.gl.clear(this.gl.COLOR_BUFFER_BIT);
	
		this.gl.useProgram(this.program)
		this.gl.bindVertexArray(this.vao);
	
		let t = performance.now()/1000;

		// TODO don't compute every frame
		//const project = GLM.mat4.create();
    	//GLM.mat4.perspective(project, this.camera.fov, this.camera.aspect, 0.1, 1000);
		const project = this.camera.projMat;


		const view = GLM.mat4.create();
		GLM.mat4.lookAt(view, GLM.vec3.fromValues(
				this.camera.r * Math.cos(this.camera.rot[0])*Math.sin(this.camera.rot[1]),
				this.camera.r * Math.cos(this.camera.rot[1]),
				this.camera.r * Math.sin(this.camera.rot[0])*Math.sin(this.camera.rot[1])
			), GLM.vec3.fromValues(0, 0, 0), GLM.vec3.fromValues(0, 1, 0)
		);

		const trans = GLM.mat4.create();
		GLM.mat4.fromTranslation(trans, GLM.vec3.fromValues(0,Math.sin(t)*0.5, 0));

		const pvm = GLM.mat4.create();
		GLM.mat4.mul(pvm, project, view);
		GLM.mat4.mul(pvm, pvm, trans);

		let pvmLocation = this.gl.getUniformLocation(this.program, "uPVM");
		this.gl.uniformMatrix4fv(pvmLocation, false, pvm)
	
		this.gl.drawArrays(this.gl.TRIANGLES, 0, 6);
		
		window.requestAnimationFrame(() => { this.update(); });
	}

	resize() {
		this.canvas.width  = window.innerWidth;
		this.canvas.height = window.innerHeight;

		this.camera.aspect = this.canvas.width / this.canvas.height;
		this.gl.viewport(0, 0, this.canvas.width, this.canvas.height);

		GLM.mat4.perspective(this.camera.projMat, this.camera.fov, this.camera.aspect, 0.1, 1000);
	}

}

document.addEventListener("DOMContentLoaded", () => {
	const canvas = document.getElementById("canvas");
	const app = new App(canvas);

});
