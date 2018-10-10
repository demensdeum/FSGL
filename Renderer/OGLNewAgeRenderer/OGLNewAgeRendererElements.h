#ifndef FSGLOGLNEWAGERENDERERELEMENTS_H_
#define FSGLOGLNEWAGERENDERERELEMENTS_H_

#include <GL/glew.h>
#include <memory>

using namespace std;

class FSGLMesh;

namespace FSGL {

class OGLNewAgeRendererElements {

public:
	OGLNewAgeRendererElements(shared_ptr<FSGLMesh> mesh);
	~OGLNewAgeRendererElements();
	GLuint vao, vbo, indexBuffer;
	GLsizei    indicesCount = 0;
	GLsizeiptr verticesBufferSize, indicesBufferSize;
	GLuint textureBinding = 0;

	void performPreRender();

private:
	shared_ptr<FSGLMesh> mesh;

	void bind();
	void fillTexture();

};

};

#endif