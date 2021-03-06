#include "FSGLCore.h"
#include <FSGL/Data/Camera/FSGLCamera.h>
#include <FSGL/Renderer/OGLNewAgeRenderer/OGLNewAgeRenderer.h>

FSGLCore::FSGLCore() {

	renderer = make_shared<OGLNewAgeRenderer>();
	
}

FSGLCore::~FSGLCore() {

}

shared_ptr<FSGLCamera> FSGLCore::camera() {
	return renderer->camera;
}

SDL_Window* FSGLCore::initialize(shared_ptr<FSEGTIOGenericSystemParams> params) {
	return renderer->initialize(params);
};
    
void FSGLCore::addObject(shared_ptr<FSGLObject> object) {

	auto id = object->id;
	idObjectMap[id] = object;

	renderer->addObject(object);
}

void FSGLCore::removeObject(shared_ptr<FSGLObject> object) {

	auto id = object->id;
	idObjectMap.erase(id);

	renderer->removeObject(object);
}

void FSGLCore::removeAllObjects() {

	for (auto iterator : idObjectMap) {
		renderer->removeObject(iterator.second);
	}

	idObjectMap.clear();
}

shared_ptr<FSGLObject> FSGLCore::getObjectWithID(string id) {
	return idObjectMap[id];
}
    
void FSGLCore::render() {
	renderer->render();
}

void FSGLCore::stop() {
	renderer->stop();
}