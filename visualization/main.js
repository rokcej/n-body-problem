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
		this.numSteps = null;
		this.numBodies = null;
		this.step = 0;
		this.drawOrbits = true;
		this.relativeSize = 0.8;
		this.absoluteSize = 1.5;

		// Setup
		this.resize();
		this.initEvents();

		// Custom dataset
		const urlParams = new URLSearchParams(window.location.search);
		let dataset = urlParams.get("data");
		if (!dataset)
			dataset = "output.txt"

		// Read shader source code and data
		readFiles([
				"shaders/sprite.vert", "shaders/sprite.frag",
				"shaders/line.vert", "shaders/line.frag",
				"../data/" + dataset
			]).then(contents => {
			const [vsSource, fsSource, vsLineSrc, fsLineSrc, dataText] = contents;

			// Compile program
			const vs = WEBGL.createShader(this.gl, this.gl.VERTEX_SHADER, vsSource);
			const fs = WEBGL.createShader(this.gl, this.gl.FRAGMENT_SHADER, fsSource);

			this.prog = WEBGL.createProgram(this.gl, vs, fs);
			this.uniforms = WEBGL.getUniforms(this.gl, this.prog);
			this.attribs = WEBGL.getAttributes(this.gl, this.prog);

			// Compile line program
			const vsLine = WEBGL.createShader(this.gl, this.gl.VERTEX_SHADER, vsLineSrc);
			const fsLine = WEBGL.createShader(this.gl, this.gl.FRAGMENT_SHADER, fsLineSrc);

			this.progLine = WEBGL.createProgram(this.gl, vsLine, fsLine);
			this.uniformsLine = WEBGL.getUniforms(this.gl, this.progLine);
			this.attribsLine = WEBGL.getAttributes(this.gl, this.progLine);

			// Initialize scene geometry
			this.initData(dataText);

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

				// Constrain horizontal rotation to [0, 2*PI)
				this.camera.rot[0] = (this.camera.rot[0] + 2.0 * Math.PI) % (2.0 * Math.PI);
				// Constrain vertical rotation to (0, PI)
				const eps = 0.001; // Small offset to prevent lookAt errors
				if (this.camera.rot[1] > Math.PI - eps)
					this.camera.rot[1] = Math.PI - eps
				else if (this.camera.rot[1] < eps)
					this.camera.rot[1] = eps;
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

	initData(dataText) {
		let lines = dataText.split("\n");
		this.numBodies = parseInt(lines[0]);
		this.numSteps = parseInt(lines[1]);

		console.log(this.numSteps);
		console.log(this.numBodies);
		console.log(lines);

		let maxPos = 0;
		let avgPos = 0
		let maxMass = 0;

		let positions = new Float32Array(this.numBodies * this.numSteps * 4);
		let orbits = new Float32Array(this.numSteps * this.numBodies * 3)
		for (let b = 0; b < this.numBodies; ++b) {
			for (let s = 0; s < this.numSteps; ++s) {
				let idx = b * this.numSteps + s;
				let idxRev = s * this.numBodies + b;
				let [x, y, z] = lines[2 + this.numBodies + idx].split(" ").map(x => parseFloat(x)); // Position
				let m = parseFloat(lines[2 + b]); // Mass
				positions[idxRev * 4 + 0] = x;
				positions[idxRev * 4 + 1] = y;
				positions[idxRev * 4 + 2] = z;
				positions[idxRev * 4 + 3] = m;

				orbits[idx * 3 + 0] = x;
				orbits[idx * 3 + 1] = y;
				orbits[idx * 3 + 2] = z;

				maxPos = Math.max(maxPos, Math.abs(x), Math.abs(y), Math.abs(z));
				avgPos += (Math.abs(x) + Math.abs(y) + Math.abs(z)) / 3.0;
				maxMass = Math.max(maxMass, m);
			}
		}
		avgPos /= this.numBodies * this.numSteps;

		// Normalize data
		for (let i = 0; i < positions.length; ++i) {
			if (i % 4 == 3) // Mass
				positions[i] *= 1.0 / maxMass;
			else // Position
				positions[i] *= 1.0 / avgPos;
		}
		for (let i = 0; i < orbits.length; ++i)
			orbits[i] *= 1.0 / avgPos;


		let vbo = this.gl.createBuffer();
		this.gl.bindBuffer(this.gl.ARRAY_BUFFER, vbo);
		this.gl.bufferData(this.gl.ARRAY_BUFFER, positions, this.gl.STATIC_DRAW);

		this.vao = this.gl.createVertexArray();
		this.gl.bindVertexArray(this.vao);
		this.gl.enableVertexAttribArray(this.attribs["aPosSize"]);
		this.gl.vertexAttribPointer(this.attribs["aPosSize"], 4, this.gl.FLOAT, false, 0, 0);

		let vboOrbits = this.gl.createBuffer();
		this.gl.bindBuffer(this.gl.ARRAY_BUFFER, vboOrbits);
		this.gl.bufferData(this.gl.ARRAY_BUFFER, orbits, this.gl.STATIC_DRAW);

		this.vaoOrbits = this.gl.createVertexArray();
		this.gl.bindVertexArray(this.vaoOrbits);
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

		// Uniforms
		let viewMat = GLM.mat4.create();
		GLM.mat4.lookAt(viewMat, GLM.vec3.fromValues(
				this.camera.r * Math.cos(this.camera.rot[0]) * Math.sin(this.camera.rot[1]),
				this.camera.r * Math.cos(this.camera.rot[1]),
				this.camera.r * Math.sin(this.camera.rot[0]) * Math.sin(this.camera.rot[1])
			), GLM.vec3.fromValues(0, 0, 0), GLM.vec3.fromValues(0, 1, 0)
		);
		let pvmMat = GLM.mat4.create();
		GLM.mat4.multiply(pvmMat, this.camera.projMat, viewMat)

		// Draw
		// this.gl.enable(this.gl.DEPTH_TEST);
		this.gl.enable(this.gl.BLEND);
		this.gl.blendFunc(this.gl.SRC_ALPHA, this.gl.ONE_MINUS_SRC_ALPHA);

		this.gl.clearColor(0.0, 0.0, 0.0, 1.0);
		this.gl.clear(this.gl.COLOR_BUFFER_BIT);

		// Orbits
		if (this.drawOrbits) {
			this.gl.useProgram(this.progLine)
			this.gl.bindVertexArray(this.vaoOrbits);

			this.gl.uniformMatrix4fv(this.uniformsLine["uPVMMat"], false, pvmMat);
		
			for (let b = 0; b < this.numBodies; ++b)
				this.gl.drawArrays(this.gl.LINE_STRIP, b * this.numSteps, this.step);
		}

		// Bodies		
		this.gl.useProgram(this.prog)
		this.gl.bindVertexArray(this.vao);

		this.gl.uniformMatrix4fv(this.uniforms["uPMat"], false, this.camera.projMat);
		this.gl.uniformMatrix4fv(this.uniforms["uVMMat"], false, viewMat); // modelMat = I --> viewModelMat == viewMat
		this.gl.uniform2f(this.uniforms["uResolution"], this.canvas.width, this.canvas.height);
		this.gl.uniform1f(this.uniforms["uRelSize"], this.relativeSize);
		this.gl.uniform1f(this.uniforms["uAbsSize"], this.absoluteSize);
	
		this.gl.drawArrays(this.gl.POINTS, this.step * this.numBodies, this.numBodies);


		this.step = (this.step + 1) % this.numSteps;
		
		window.requestAnimationFrame(() => { this.update(); });
	}

	resize() {
		this.canvas.width  = window.innerWidth;
		this.canvas.height = window.innerHeight;

		this.camera.aspect = this.canvas.width / this.canvas.height;
		this.gl.viewport(0, 0, this.canvas.width, this.canvas.height);

		GLM.mat4.perspective(this.camera.projMat, this.camera.fov, this.camera.aspect, 0.1, 10000);
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
