#include "OGLNewAgeRendererElements.h"
#include <FSGL/Data/Mesh/FSGLMesh.h>

using namespace FSGL;

OGLNewAgeRendererElements::OGLNewAgeRendererElements(shared_ptr<FSGLMesh> mesh) {

	this->mesh = mesh;

	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &indexBuffer);

        auto vertices = mesh->glVertices;
        auto indices = mesh->glIndices;

        verticesBufferSize = mesh->glVerticesBufferSize;
        indicesBufferSize = mesh->glIndicesBufferSize;
        indicesCount = mesh->glIndicesCount;

	 bind();

        glBufferData(GL_ARRAY_BUFFER, verticesBufferSize, mesh->glVertices, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesBufferSize, mesh->glIndices, GL_STATIC_DRAW);
}

void OGLNewAgeRendererElements::bind() {

        glBindVertexArray(vao);        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);	

}

OGLNewAgeRendererElements::~OGLNewAgeRendererElements() {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &indexBuffer);
}