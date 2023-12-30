#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "glsystem.h"

GLShader::GLShader(GLenum type, const char* code) {
    mShaderId = glCreateShader(type);

    //printf("Compiling shader...\n");
    glShaderSource(mShaderId, 1, &code, nullptr);
	glCompileShader(mShaderId);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Check Shader
	glGetShaderiv(mShaderId, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(mShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(mShaderId, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}
}

GLShader::~GLShader() {
    glDeleteShader(mShaderId);
}

GLProgram::GLProgram(GLShader* vs, GLShader* ps, GLShader* gs) {
    GLint Result = GL_FALSE;
	int InfoLogLength;

	// Link the program
	//printf("Linking program\n");

	mProgramId = glCreateProgram();
	glAttachShader(mProgramId, vs->getResourceId());
	glAttachShader(mProgramId, ps->getResourceId());
	if (gs) {
        glAttachShader(mProgramId, gs->getResourceId());
	}
	glLinkProgram(mProgramId);

	// Check the program
	glGetProgramiv(mProgramId, GL_LINK_STATUS, &Result);
	glGetProgramiv(mProgramId, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(mProgramId, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(mProgramId, vs->getResourceId());
	glDetachShader(mProgramId, ps->getResourceId());
	if (gs) {
        glDetachShader(mProgramId, gs->getResourceId());
	}
}

GLProgram::~GLProgram() {
    glDeleteProgram(mProgramId);
}

void GLProgram::setValueInt(const char* name, int i) {
    glUniform1i(glGetUniformLocation(mProgramId, name), i);
}

void GLProgram::setValueFloat3(const char* name, float x, float y, float z) {
    glUniform3f(glGetUniformLocation(mProgramId, name), x, y, z);
}

void GLProgram::setValueMatrix4fv(const char* name, float* v) {
    glUniformMatrix4fv(glGetUniformLocation(mProgramId, name), 1, GL_FALSE, v);
}







