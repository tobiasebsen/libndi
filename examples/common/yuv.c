#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../common/ogl.h"
#include "../common/yuv.h"

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
#ifdef __arm__
"const mediump vec3 offset = vec3(-0.0625, -0.5, -0.5);\n"
"const mediump vec3 rcoeff = vec3(1.164, 0.000, 1.596);\n"
"const mediump vec3 gcoeff = vec3(1.164, -0.391, -0.813);\n"
"const mediump vec3 bcoeff = vec3(1.164, 2.018, 0.000);\n"
#else
"const vec3 offset = vec3(-0.0625, -0.5, -0.5);\n"
"const vec3 rcoeff = vec3(1.164, 0.000, 1.596);\n"
"const vec3 gcoeff = vec3(1.164, -0.391, -0.813);\n"
"const vec3 bcoeff = vec3(1.164, 2.018, 0.000);\n"
#endif
"\n"
"void main() {\n"
#ifdef __arm__
"    mediump vec3 yuv;\n"
#else
"    vec3 yuv;\n"
#endif
"    yuv.x = texture2D(tex_y, v_texcoord).r;\n"
"    yuv.y = texture2D(tex_u, v_texcoord).r;\n"
"    yuv.z = texture2D(tex_v, v_texcoord).r;\n"
"    yuv += offset;\n"
#ifdef __arm__
"    mediump float r = dot(yuv, rcoeff);\n"
"    mediump float g = dot(yuv, gcoeff);\n"
"    mediump float b = dot(yuv, bcoeff);\n"
#else
"    float r = dot(yuv, rcoeff);\n"
"    float g = dot(yuv, gcoeff);\n"
"    float b = dot(yuv, bcoeff);\n"
#endif
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
static GLuint texture[3];
static float matrix[4][4];
static GLfloat vertices[4][3];
static GLfloat tex_coords[] = { 0, 0, 1, 0, 1, 1, 0, 1 };


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
    
    glGenTextures(3, texture);
    
    memset(matrix, 0, sizeof(matrix));
    memset(vertices, 0, sizeof(vertices));

	return 0;
}

void yuv_bind() {
	glUseProgram(program);
}

void yuv_view(float left, float right, float top, float bottom, float near, float far) {
    matrix[0][0] = 2.0f / (right - left);
    matrix[1][1] = 2.0f / (top - bottom);
    matrix[2][2] = -2.0f / (far - near);
    matrix[3][0] = -(right + left) / (right - left);
    matrix[3][1] = -(top + bottom) / (top - bottom);
    matrix[3][2] = -(far + near) / (far - near);
    matrix[3][3] = 1;
}

void yuv_data(int plane, unsigned char * data, int width, int height, int linesize) {
    glActiveTexture(GL_TEXTURE0 + plane);
    glBindTexture(GL_TEXTURE_2D, texture[plane]);
#ifdef GL_UNPACK_ROW_LENGTH
    glPixelStorei(GL_UNPACK_ROW_LENGTH, linesize);
#else
    width = linesize;
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void yuv_draw(float x, float y, float width, float height) {
	yuv_draw_sub(x, y, width, height, 0, 0, 1.f, 1.f);
}

void yuv_draw_sub(float x, float y, float width, float height, float sx, float sy, float sw, float sh) {

	glUniformMatrix4fv(location.mtx, 1, GL_FALSE, &matrix[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glUniform1i(location.tex_y, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glUniform1i(location.tex_u, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glUniform1i(location.tex_v, 2);

	glActiveTexture(GL_TEXTURE0);

	vertices[0][0] = x;
	vertices[0][1] = y;
	vertices[1][0] = x + width;
	vertices[1][1] = y;
	vertices[2][0] = x + width;
	vertices[2][1] = y + height;
	vertices[3][0] = x;
	vertices[3][1] = y + height;

	tex_coords[0][0] = sx;
	tex_coords[0][1] = sy;
	tex_coords[1][0] = sx + sw;
	tex_coords[1][1] = sy;
	tex_coords[2][0] = sx + sw;
	tex_coords[2][1] = sy + sh;
	tex_coords[3][0] = sx;
	tex_coords[3][1] = sy + sh;

	glVertexAttribPointer(location.pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), &vertices[0][0]);
	glEnableVertexAttribArray(location.pos);

	glVertexAttribPointer(location.txc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), tex_coords);
	glEnableVertexAttribArray(location.txc);

	GLushort indices[] = { 0, 3, 1, 2 };
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices);

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}

void yuv_unbind() {
	glUseProgram(0);
}

