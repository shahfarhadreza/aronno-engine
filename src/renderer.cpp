#include "stdafx.h"
#include "engine.h"
#include "renderer.h"
#include "mesh.h"

QuadBufferIndexed::QuadBufferIndexed()
    : mVertexBuffer(nullptr), mIndexBuffer(nullptr) {

}

bool QuadBufferIndexed::create(Renderer* rnd) {
    uint32_t vCount = sizeof(quadVertices) / sizeof(Vertex);
    mVertexBuffer = rnd->createGPUVertexBuffer(quadVertices, vCount);

    uint32_t iCount = sizeof(quadIndices) / sizeof(uint32_t);
    mIndexBuffer = rnd->createGPUIndexBuffer(quadIndices, iCount);
    return true;
}

Renderer::~Renderer() {
    for(auto it = mQuadBufferIndexedList.begin(); it!= mQuadBufferIndexedList.end();++it) {
        delete *it;
    }
    mQuadBufferIndexedList.clear();
}

QuadBufferIndexed* Renderer::createQuadBufferIndexed() {
    QuadBufferIndexed* buf = new QuadBufferIndexed();
    buf->create(this);
    mQuadBufferIndexedList.push_back(buf);
    return buf;
}

void Renderer::bindQuadBuffer(QuadBufferIndexed* qb) {
    bindResource(qb->getVertexBuffer());
    bindResource(qb->getIndexBuffer());
}







