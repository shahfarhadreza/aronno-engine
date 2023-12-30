#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "glsystem.h"

GLVertexBuffer::GLVertexBuffer(const Vertex* data, uint32_t count) : mVertexCount(count) {
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	const uint32_t nSize = sizeof(Vertex);
	int nOffset = 0;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, nSize, 0);
	nOffset += 3;

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, nSize, (void*)(nOffset * sizeof(float)));
	nOffset += 3;

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, nSize, (void*)(nOffset * sizeof(float)));
	nOffset += 3;

	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, nSize, (void*)(nOffset * sizeof(float)));
	nOffset += 2;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

GLVertexBuffer::GLVertexBuffer(const AnimatedVertex* data, uint32_t count) : mVertexCount(count) {
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(AnimatedVertex), data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);

	glEnableVertexAttribArray(6);
	glEnableVertexAttribArray(7);

	const uint32_t nSize = sizeof(AnimatedVertex);
	int nOffset = 0;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, nSize, 0);
	nOffset += 3;

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, nSize, (void*)(nOffset * sizeof(float)));
	nOffset += 3;

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, nSize, (void*)(nOffset * sizeof(float)));
	nOffset += 3;

	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, nSize, (void*)(nOffset * sizeof(float)));
	nOffset += 2;

	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, nSize, (void*)(nOffset * sizeof(float)));
	nOffset += 4;

	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, nSize, (void*)(nOffset * sizeof(float)));
	nOffset += 4;

	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, nSize, (void*)(nOffset * sizeof(float)));
	nOffset += 4;

	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, nSize, (void*)(nOffset * sizeof(float)));
	nOffset += 4;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

GLVertexBuffer::~GLVertexBuffer() {
    glDeleteBuffers(1, &mVBO);
    glDeleteVertexArrays(1, &mVAO);
}

void GLVertexBuffer::updateData(const Vertex* data, uint32_t count) {
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(Vertex), data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    mVertexCount = count;
}

GLIndexBuffer::GLIndexBuffer(const uint32_t* data, uint32_t count) : mCount(count) {

    glGenBuffers(1, &mBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

GLIndexBuffer::~GLIndexBuffer() {
    glDeleteBuffers(1, &mBufferId);
}

GLConstantBuffer::GLConstantBuffer(uint32_t sizeinBytes) : mSize(sizeinBytes) {

    glGenBuffers(1, &mBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, mBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeinBytes, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

GLConstantBuffer::~GLConstantBuffer() {
    glDeleteBuffers(1, &mBufferId);
}

void GLConstantBuffer::updateData(void* data) {
    glBindBuffer(GL_UNIFORM_BUFFER, mBufferId);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, mSize, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}




