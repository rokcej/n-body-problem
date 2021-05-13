export function createShader(gl, type, source) {
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

export function createProgram(gl, vertexShader, fragmentShader) {
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

export function getUniforms(gl, prog) {
	let uniforms = {};
	const numUniforms = gl.getProgramParameter(prog, gl.ACTIVE_UNIFORMS);
	for (let i = 0; i < numUniforms; ++i) {
		const info = gl.getActiveUniform(prog, i);
		uniforms[info.name] = gl.getUniformLocation(prog, info.name);
	}
	return uniforms;
}

export function getAttributes(gl, prog) {
	let attribs = {};
	const numAttribs = gl.getProgramParameter(prog, gl.ACTIVE_ATTRIBUTES);
	for (let i = 0; i < numAttribs; ++i) {
		const info = gl.getActiveAttrib(prog, i);
		attribs[info.name] = gl.getAttribLocation(prog, info.name);
	}
	return attribs;
}
