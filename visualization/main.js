"use strict";

// Libraries
import * as WEBGL from "./webgl.js"
const GLM = glMatrix;


class App {
	constructor(canvas) {
		// Create global reference
		window.app = this;
		// Create WebGL2 context
		this.canvas = canvas;
		this.gl = canvas.getContext("webgl2");
		if (!this.gl) {
			console.log("WebGL2 not supported");
			return;
		}

		// Attributes
		this.fps = {
			value: 0,
			count: 0,
			lastUpdated: 0,
			span: document.getElementById("fps")
		}
		this.camera = {
			r: 2.0, // Radius
			fov: deg2rad(90.0), // Field of view
			rot: GLM.vec2.fromValues(0.5 * Math.PI, 0.5 * Math.PI),
			aspect: this.canvas.width / this.canvas.height,
			projMat: GLM.mat4.create()
		}
		this.mouse = {
			dragSpeed: 0.005, // Dragging sensitivity
			scrollSpeed: 1.05, // Scrolling sensitivity
			xPrev: 0,
			yPrev: 0,
			dragging: false
		}

		// Setup
		this.resize();
		this.initEvents();

		// Read shader source code
		readFiles(["shaders/sprite.vert", "shaders/sprite.frag"]).then(contents => {
			// Compile program
			const [vsSource, fsSource] = contents;
			const vs = WEBGL.createShader(this.gl, this.gl.VERTEX_SHADER, vsSource);
			const fs = WEBGL.createShader(this.gl, this.gl.FRAGMENT_SHADER, fsSource);

			this.prog = WEBGL.createProgram(this.gl, vs, fs);
			this.uniforms = WEBGL.getUniforms(this.gl, this.prog);
			this.attribs = WEBGL.getAttributes(this.gl, this.prog);

			// Initialize scene geometry
			this.initData();

			// Start animation
			window.requestAnimationFrame(() => { this.update(); });
		});
	}

	initEvents() {
		this.canvas.addEventListener("mousedown", (event) => {
			this.mouse.xPrev = event.clientX;
			this.mouse.yPrev = event.clientY;
			this.mouse.dragging = true;
		});
		this.canvas.addEventListener("mouseup", (event) => {
			this.mouse.dragging = false;
		});
		this.canvas.addEventListener("mousemove", (event) => {
			if (this.mouse.dragging) {
				let x = event.clientX, y = event.clientY;
				let dx = x - this.mouse.xPrev;
				let dy = y - this.mouse.yPrev;
				this.mouse.xPrev = x;
				this.mouse.yPrev = y;

				this.camera.rot[0] += dx * this.mouse.dragSpeed;
				this.camera.rot[1] -= dy * this.mouse.dragSpeed;

				if (this.camera.rot[1] > Math.PI)
					this.camera.rot[1] = Math.PI
				else if (this.camera.rot[1] < 0)
					this.camera.rot[1] = 0;
			}
		});
		this.canvas.addEventListener("wheel", (event) => {
			if (event.deltaY > 0)
				this.camera.r *= this.mouse.scrollSpeed;
			else if (event.deltaY < 0)
				this.camera.r /= this.mouse.scrollSpeed;
		});
		window.addEventListener("resize", () => {
			this.resize();
		});
	}

	initData() {
		let vbo = this.gl.createBuffer();
		this.gl.bindBuffer(this.gl.ARRAY_BUFFER, vbo);

		var positions = [
			-0.5, -0.5, -0.5,
			-0.5, -0.5, +0.5,
			-0.5, +0.5, -0.5,
			-0.5, +0.5, +0.5,
			+0.5, -0.5, -0.5,
			+0.5, -0.5, +0.5,
			+0.5, +0.5, -0.5,
			+0.5, +0.5, +0.5,
		];
		this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(positions), this.gl.STATIC_DRAW);

		this.vao = this.gl.createVertexArray();
		this.gl.bindVertexArray(this.vao);
		this.gl.enableVertexAttribArray(this.attribs["aPos"]);
		this.gl.vertexAttribPointer(this.attribs["aPos"], 3, this.gl.FLOAT, false, 0, 0);
	}

	update() {
		// Time
		let t = performance.now() * 0.001;
		// FPS
		++this.fps.count;
		if (t - this.fps.lastUpdated >= 1.0) {
			this.fps.value = this.fps.count / (t - this.fps.lastUpdated);
			this.fps.span.innerHTML = Math.round(this.fps.value);
			this.fps.count = 0;
			this.fps.lastUpdated = t;
		}

		// Draw
		this.gl.clearColor(0.0, 0.0, 0.0, 1.0);
		this.gl.clear(this.gl.COLOR_BUFFER_BIT);
	
		this.gl.useProgram(this.prog)
		this.gl.bindVertexArray(this.vao);

		let viewMat = GLM.mat4.create();
		GLM.mat4.lookAt(viewMat, GLM.vec3.fromValues(
				this.camera.r * Math.cos(this.camera.rot[0]) * Math.sin(this.camera.rot[1]),
				this.camera.r * Math.cos(this.camera.rot[1]),
				this.camera.r * Math.sin(this.camera.rot[0]) * Math.sin(this.camera.rot[1])
			), GLM.vec3.fromValues(0, 0, 0), GLM.vec3.fromValues(0, 1, 0)
		);

		this.gl.uniformMatrix4fv(this.uniforms["uPMat"], false, this.camera.projMat);
		this.gl.uniformMatrix4fv(this.uniforms["uVMMat"], false, viewMat); // modelMat = I --> viewModelMat == viewMat
		this.gl.uniform2f(this.uniforms["uResolution"], this.canvas.width, this.canvas.height);
	
		this.gl.drawArrays(this.gl.POINTS, 0, 8);
		
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


function readFiles(urls) {
	let contents = []
	for (let url of urls)
		contents.push(fetch(url).then(res => res.text()));
	return Promise.all(contents);
}

function deg2rad(angle) {
	return angle * Math.PI / 180.0;
}


document.addEventListener("DOMContentLoaded", () => {
	const canvas = document.getElementById("canvas");
	const app = new App(canvas);
});
