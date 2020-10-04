#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>
#include "../common/ogl.h"
#include "yuv.h"

static const char * source_vert_yuv =
"uniform mat4 u_matrix;\n"
"attribute vec4 a_position;\n"
"attribute vec2 a_texcoord;\n"
#ifdef __arm__
"varying mediump vec2 v_texcoord;\n"
#else
"varying vec2 v_texcoord;\n"
#endif
"\n"
"void main() {\n"
"    gl_Position = u_matrix * a_position;\n"
"    v_texcoord = a_texcoord;\n"
"}\n"
;

static const char * source_frag_yuv =
#ifdef __arm__
"varying mediump vec2 v_texcoord;\n"
#else
"varying vec2 v_texcoord;\n"
#endif
"uniform sampler2D tex_y;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"\n"
"const vec3 offset = vec3(-0.0625, -0.5, -0.5);\n"
"const vec3 rcoeff = vec3(1.164, 0.000, 1.596);\n"
"const vec3 gcoeff = vec3(1.164, -0.391, -0.813);\n"
"const vec3 bcoeff = vec3(1.164, 2.018, 0.000);\n"
"\n"
"void main() {\n"
"    vec3 yuv;\n"
"    yuv.x = texture2D(tex_y, v_texcoord).r;\n"
"    yuv.y = texture2D(tex_u, v_texcoord).r;\n"
"    yuv.z = texture2D(tex_v, v_texcoord).r;\n"
"    yuv += offset;\n"
"    float r = dot(yuv, rcoeff);\n"
"    float g = dot(yuv, gcoeff);\n"
"    float b = dot(yuv, bcoeff);\n"
"    gl_FragColor = vec4(r, g, b, 1);\n"
"}\n"
;

static GLuint program;
static struct {
	GLint mtx;
	GLint pos;
	GLint txc;
	GLint tex_y;
	GLint tex_u;
	GLint tex_v;
} location;

int yuv_init() {
	int status = GL_FALSE;

	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &source_vert_yuv, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint len = 0;
		glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &len);
		char * log = (char*)malloc(len);
		glGetShaderInfoLog(vertex, len, 0, log);
		printf("Vertex shader log: %s\n", log);
		free(log);
		return -1;
	}

	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &source_frag_yuv, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint len = 0;
		glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &len);
		char * log = (char*)malloc(len);
		glGetShaderInfoLog(fragment, len, 0, log);
		printf("Fragment shader log: %s\n", log);
		free(log);
		return -1;
	}

	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint len = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		char * log = (char*)malloc(len);
		glGetProgramInfoLog(program, len, 0, log);
		printf("Program log: %s\n", log);
		free(log);
		return -1;
	}

	location.mtx = glGetUniformLocation(program, "u_matrix");
	location.pos = glGetAttribLocation(program, "a_position");
	location.txc = glGetAttribLocation(program, "a_texcoord");
	location.tex_y = glGetUniformLocation(program, "tex_y");
	location.tex_u = glGetUniformLocation(program, "tex_u");
	location.tex_v = glGetUniformLocation(program, "tex_v");

	return 0;
}

void yuv_bind() {
	glUseProgram(program);
}

int yuv_get_uniform(const char * name) {
	return glGetUniformLocation(program, name);
}

int yuv_get_attrib(const char * name) {
	return glGetAttribLocation(program, name);
}

void yuv_textures(int tex_y, int tex_u, int tex_v) {
	//glUniformMatrix4fv(loc_m, 1, GL_FALSE, &matrix[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_y);
	glUniform1i(location.tex_y, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_u);
	glUniform1i(location.tex_u, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex_v);
	glUniform1i(location.tex_v, 2);

	glActiveTexture(GL_TEXTURE0);
}

void yuv_unbind() {
	glUseProgram(0);
}

