#include "Renderer.h"
#include <FSGL/Data/Camera/FSGLCamera.h>

using namespace FSGL;

Renderer::Renderer() {
	camera = make_shared<FSGLCamera>(); 
}

Renderer::~Renderer() {

}