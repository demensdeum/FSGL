#ifndef FSGL_RENDERER_H_
#define FSGL_RENDERER_H_

#include <memory>

using namespace std;
class FSGLObject;
class FSGLCamera;

struct SDL_Window;

namespace FSGL {

class Renderer {

public:
	Renderer();
	virtual ~Renderer();
	virtual SDL_Window* initialize() = 0;

	virtual void addObject(shared_ptr<FSGLObject> object) = 0;
	virtual void removeObject(shared_ptr<FSGLObject> object) = 0;

	virtual void render() = 0;

	virtual void stop() = 0;

 	shared_ptr<FSGLCamera> camera;
};
};

#endif