#ifndef FSGL_RENDERER_H_
#define FSGL_RENDERER_H_

namespace FSGL {

class Renderer {

public:
	virtual void addObject(shared_ptr<FSGLObject> object) = 0;
	virtual void removeObject(shared_ptr<FSGLObject> object) = 0;

	virtual void renderObject(shared_ptr<FSGLObject> object) = 0;

}
}

#endif