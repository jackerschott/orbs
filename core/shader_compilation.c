#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include "shader_compilation.h"

const char* prog_names[] = {
	"render_cluster",
	"evolve_cluster",
	"gen_elliptic_cluster_pos",
	"gen_elliptic_cluster_colors",
	"render_firmament",
	"gen_uniform_samples",
	"gen_gaussian_samples",
};
int n_shaders[] = { 2, 1, 1, 1, 2, 1, 1 };
GLuint shader_types[][MAX_SHADERS_PER_PROG] = {
	{ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER },
	{ GL_COMPUTE_SHADER },
	{ GL_COMPUTE_SHADER },
	{ GL_COMPUTE_SHADER },
	{ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER },
	{ GL_COMPUTE_SHADER },
	{ GL_COMPUTE_SHADER },
};
const char *shadersrcs[][MAX_SHADERS_PER_PROG] = {
	{ CLUSTER_VERT_SRC, CLUSTER_FRAG_SRC },
	{ EVOLVE_CLUSTER_COMP_SRC },
	{ GEN_ELLIPTIC_CLUSTER_POS_COMP_SRC },
	{ GEN_ELLIPTIC_CLUSTER_COLORS_COMP_SRC },
	{ FIRMAMENT_VERT_SRC, FIRMAMENT_FRAG_SRC },
	{ GEN_UNIFORM_SAMPLES_COMP_SRC },
	{ GEN_GAUSSIAN_SAMPLES_COMP_SRC },
};
const char* shader_names[][MAX_SHADERS_PER_PROG] = {
	{ "cluster_vert", "cluster_frag" },
	{ "evolve_cluster_comp" },
	{ "gen_elliptic_cluster_pos_comp" },
	{ "gen_elliptic_cluster_colors_comp" },
	{ "firmament_vert", "firmament_frag" },
	{ "gen_uniform_samples" },
	{ "gen_gaussian_samples" },
};

void compile_shaders(int _n_shaders, GLenum *_shader_types,
		const GLchar *const *_shadersrcs, GLuint *shaders)
{
	for (int i = 0; i < _n_shaders; ++i) {
		GLuint shader = glCreateShader(_shader_types[i]);
		glShaderSource(shader, 1, &_shadersrcs[i], NULL);

		glCompileShader(shader);
		shaders[i] = shader;
	}
}
void link_program(int n_shaders, GLuint shaders[], GLuint *_prog)
{
	GLuint prog = glCreateProgram();

	for (int i = 0; i < n_shaders; ++i) {
		glAttachShader(prog, shaders[i]);
	}

	glLinkProgram(prog);
	glValidateProgram(prog);
	
	*_prog = prog;
}

int check_shader_compile_errors(int _n_shaders,
		GLuint *shaders, int *successes)
{
	int retval = 0;
	int success;
	for (int i = 0; i < _n_shaders; ++i) {
		glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &success);
		successes[i] = success;
		if (!success && retval == 0) {
			retval = 1;
		}
	}
	return retval;
}
int check_program_link_error(GLuint prog)
{
	int success;
	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	return !success;
}
int check_program_validate_error(GLuint prog)
{
	int success;
	glGetProgramiv(prog, GL_VALIDATE_STATUS, &success);
	return !success;
}

void handle_shader_compile_errors(int _n_shaders, GLuint *shaders,
		const char **shader_names, int *successes)
{
	for (int i = 0; i < _n_shaders; ++i) {
		if (successes[i])
			continue;

		GLint log_length;
		glGetShaderiv(shaders[i], GL_INFO_LOG_LENGTH, &log_length);

		GLchar *log = malloc(log_length);
		glGetShaderInfoLog(shaders[i], log_length, NULL, log);

		fprintf(stderr,"Internal error: Shader %s failed to compile:\n",
				shader_names[i]);
		fprintf(stderr, "%s\n", log);
	}
}
void handle_program_link_error(GLuint prog, const char *prog_name)
{
	GLint log_length;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &log_length);

	GLchar *log = malloc(log_length);
	glGetProgramInfoLog(prog, log_length, NULL, log);

	fprintf(stderr, "Internal error: Program %s failed to link:\n",
			prog_name);
	fprintf(stderr, "%s\n", log);
}
void handle_program_validate_error(GLuint prog, const char *prog_name)
{
	GLint log_length;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &log_length);

	GLchar *log = malloc(log_length);
	glGetProgramInfoLog(prog, log_length, NULL, log);

	fprintf(stderr, "Internal error: Program %s could not be validated:\n",
			prog_name);
	fprintf(stderr, "%s\n", log);
}

void free_shaders(GLuint prog, int _n_shaders, GLuint *_shaders)
{
	for (int i = 0; i <_n_shaders; ++i) {
		glDetachShader(prog, _shaders[i]);
		glDeleteShader(_shaders[i]);
	}
}
void free_programs(int n_progs, GLuint *progs)
{
	for (int i = 1; i < n_progs; ++i) {
		glDeleteProgram(progs[i]);
	}
}

int compile_gl_programs(union gl_programs* progs)
{
	GLuint shaders[N_PROGS][MAX_SHADERS_PER_PROG];
	for (int i = 0; i < N_PROGS; ++i) {
		compile_shaders(n_shaders[i], shader_types[i],
				shadersrcs[i], shaders[i]);
		link_program(n_shaders[i], shaders[i],
				&progs->wrap[i]);
	}

	int retval = 0;
	for (int i = 0; i < N_PROGS; ++i) {
		int successes[n_shaders[i]];
		if (check_shader_compile_errors(n_shaders[i],
				shaders[i], successes))
		{
			handle_shader_compile_errors(n_shaders[i],
				shaders[i], shader_names[i], successes);
			retval = 1;
		}
		free_shaders(progs->wrap[i], n_shaders[i], shaders[i]);

		if (check_program_link_error(progs->wrap[i])) {
			handle_program_link_error(progs->wrap[i],
					prog_names[i]);
			retval = 1;
		}
		
		if (check_program_validate_error(progs->wrap[i])) {
			handle_program_validate_error(progs->wrap[i],
					prog_names[i]);
			retval = 1;
		}
	}

	if (retval != 0)
		free_programs(N_PROGS, progs->wrap);
	return retval;
}

void free_gl_programs(union gl_programs* progs)
{
	free_programs(N_PROGS, progs->wrap);
}

