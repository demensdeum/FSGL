/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FSEOGLCore.h
 * Author: demensdeum
 *
 * Created on July 8, 2017, 10:10 AM
 */

#ifndef FSEOGLCORE_H
#define FSEOGLCORE_H

#include <SDL2/SDL.h>

#include <memory>

#include <FSGL/Data/Model/FSGLModel.h>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <FSGL/Data/Object/FSGLObject.h>
#include <FSGL/Renderer/Renderer.h>

#include <set>

using namespace std;
using namespace FSGL;

namespace FSGL {

class OGLRenderer: public Renderer {
public:    

	OGLRenderer() : Renderer() { };
	virtual ~OGLRenderer();

    SDL_Window* initialize();
    
	//Create, Remove
    void addObject(shared_ptr<FSGLObject> object);
    void removeObject(shared_ptr<FSGLObject> object);

    shared_ptr<FSGLObject> getObjectWithID(string id);
    
    void render();
    void stop();
    
private:

	void removeObjectAtIndex(shared_ptr<FSGLObject> object, int index);
    void renderObject(shared_ptr<FSGLObject> object);
    
    GLint common_get_shader_program(const char *vertex_shader_source, const char *fragment_shader_source);

    SDL_GLContext context;
    GLuint shader_program;
    
    GLuint vao, vbo, indexBuffer;
   
    SDL_Window *window;
    
    vector<shared_ptr<FSGLObject>> objects2D;
    vector<shared_ptr<FSGLObject>> objects;
};

};

#endif /* FSEOGLCORE_H */

