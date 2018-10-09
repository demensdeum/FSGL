	/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FSEOGLCore.cpp
 * Author: demensdeum
 * 
 * Created on July 8, 2017, 10:10 AM
 */

#include "OGLNewAgeRenderer.h"

#include <FSGL/Data/Model/FSGLModel.h>
#include <FSGL/Data/Camera/FSGLCamera.h>

#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <thread>

#include <fstream>
#include <iostream>
#include <set>

#include <json/json.hpp>

#include "shaders.h"

#ifdef __APPLE__
#define FSGL_LEGACY_OPENGL 1
#endif

using namespace std;
using namespace FSGL;

static const auto OGLNewAgeRendererScreenWidth = 1024;
static const auto OGLNewAgeRendererScreenHeight = 576;

GLint OGLNewAgeRenderer::common_get_shader_program(const char *vertex_shader_source, const char *fragment_shader_source) {

    GLchar infoLog[512];
    GLint fragment_shader;
    GLint shader_program;
    GLint success;
    GLint vertex_shader;

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << endl << infoLog << endl;
	throw logic_error("Can't compile shader program");
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << endl << infoLog << endl;
	throw logic_error("Can't compile shader program");
    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED" << endl << infoLog << endl;
	throw logic_error("Can't link shader program");
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

OGLNewAgeRenderer::~OGLNewAgeRenderer() {

}

void OGLNewAgeRenderer::removeObject(shared_ptr<FSGLObject> object) {

	if (object->flag2D) {
	    for (size_t i = 0; i < objects2D.size(); i++) {
	
		if (object.get() == objects2D[i].get()) {

			removeObjectAtIndex(object, i);

			return;
		}
		}
	}
	else {
	    for (size_t i = 0; i < objects.size(); i++) {
	
		if (object.get() == objects[i].get()) {

			removeObjectAtIndex(object, i);

			return;
		}
		}
	}
}

void OGLNewAgeRenderer::removeObjectAtIndex(shared_ptr<FSGLObject> object, int index) {

	if (object->flag2D) {
		objects2D.erase(objects2D.begin() + index);
	}
	else {
		objects.erase(objects.begin() + index);
	}

}

#ifndef FSGL_LEGACY_OPENGL

static void  FSGL_openGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{ 
	if (type != GL_DEBUG_TYPE_OTHER)
	{
		cout << "OpenGL: "<< message << endl;
	}
}

#endif

SDL_Window* OGLNewAgeRenderer::initialize() {

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,16);
    
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
            "Death Mask - 0.1 Alpha",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            OGLNewAgeRendererScreenWidth,
            OGLNewAgeRendererScreenHeight,
            SDL_WINDOW_OPENGL
            );

    if (window == NULL) {

	printf("Could not create window: %s\n", SDL_GetError());
	exit(1);
        
    } 
    
#ifndef __EMSCRIPTEN__
#ifndef FSGL_LEGACY_OPENGL
    auto majorVersion = 4;
    auto minorVersion = 2;
#else
    auto majorVersion = 2;
    auto minorVersion = 0;
#endif

    
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, majorVersion);
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, minorVersion);
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );        
#endif
    
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

    context = SDL_GL_CreateContext(window);

    if (context == NULL) {
     
        printf("SDL_Init failed: %s\n", SDL_GetError());
        
    }
    
    GLenum err = glewInit();
    
    if (GLEW_OK != err) {
        
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        
    }
    
    SDL_GL_MakeCurrent(window, context);

	glEnable(GL_MULTISAMPLE);
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

#ifndef FSGL_LEGACY_OPENGL
	glEnable(GL_DEBUG_OUTPUT);
	if (glGetError() != GL_NO_ERROR) {
		throw runtime_error("Can't enable debug output");
	}

        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	if (glGetError() != GL_NO_ERROR) {
		throw runtime_error("Can't enable debug output synchronous");
	}

        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        if (glGetError() != GL_NO_ERROR) {
		throw runtime_error("Can't enable debug message control");
	}

	glDebugMessageCallback(FSGL_openGLDebugCallback, NULL);
#endif

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, OGLNewAgeRendererScreenWidth, OGLNewAgeRendererScreenHeight);

    shader_program = common_get_shader_program(vertexShaderSource, fragmentShaderSource);
    
    glUseProgram(shader_program);
    
    GLint projectionMatrixUniform;    
    
    glm::mat4 projectionMatrix = glm::perspective(45.0f, float(float(OGLNewAgeRendererScreenWidth) / float(OGLNewAgeRendererScreenHeight)), 0.001f, 200.0f);
    projectionMatrixUniform = glGetUniformLocation(shader_program, "projectionMatrix");
    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));    
 
	return window;

}

void OGLNewAgeRenderer::addObject(shared_ptr<FSGLObject> object) {
	if (object->flag2D) {
		objects2D.push_back(object);
	}
	else {
		objects.push_back(object);
	}
}

void OGLNewAgeRenderer::render() {

    // clear

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepthf(1.0f);

    for (unsigned int i = 0; i < objects.size(); i++) {

        auto object = objects[i];

	if (object.get() == nullptr) {

		cout << "OGLNewAgeRenderer cannot render empty object" << endl;

		exit(1);

	}

	renderObject(object);

    }

	// 2D On Screen Rendering

    glClear(GL_DEPTH_BUFFER_BIT);
    glClearDepthf(1.0f);

    for (unsigned int i = 0; i < objects2D.size(); i++) {
        auto object = objects2D[i];
	if (object.get() == nullptr) {
		cout << "OGLNewAgeRenderer cannot render empty object" << endl;
		exit(1);
	}
   
	renderObject(object);            
        
    }

    SDL_GL_SwapWindow(window);
}

void OGLNewAgeRenderer::stop() {

}

shared_ptr<FSGLObject> OGLNewAgeRenderer::getObjectWithID(string id) {
    
    for (size_t i = 0; i < objects.size(); i++) {
        auto object = objects[i];
        if (object->id == id) {
            return object;
        }
    }

	for (size_t i = 0; i < objects2D.size(); i++) {
		auto object = objects2D[i];
		if (object->id == id) {
			return object;
		}
	}
    
    return shared_ptr<FSGLObject>();
}

void OGLNewAgeRenderer::renderObject(shared_ptr<FSGLObject> object) {
    
    auto model = object->model;

    for (unsigned int meshIndex = 0; meshIndex < model->meshes.size(); meshIndex++) {

        auto mesh = model->meshes[meshIndex];

        mesh->updateGlAnimationTransformation();
        
        auto vertices = mesh->glVertices;
        auto indices = mesh->glIndices;

        GLsizeiptr verticesBufferSize = mesh->glVerticesBufferSize;
        GLsizeiptr indicesBufferSize = mesh->glIndicesBufferSize;
        GLsizei    indicesCount = mesh->glIndicesCount;

        GLint vertexSlot = glGetAttribLocation(shader_program, "vertex");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);        
        
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verticesBufferSize, vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesBufferSize, indices, GL_STATIC_DRAW);

        glVertexAttribPointer(vertexSlot, 3, GL_FLOAT, GL_FALSE, FSGLMesh::glVertexSize, 0);
        glEnableVertexAttribArray(vertexSlot);

        auto material = mesh->material;

	if (material == NULL) {

            throw logic_error("OGLNewAgeRenderer: cannot load material");

	}

        auto surface = material->surface;

        if (surface == NULL) {

            cout << "OGLNewAgeRenderer: cannot load texture " << material->texturePath->c_str() << endl;
	
		throw logic_error("OGLNewAgeRenderer: cannot load texture");

        }

        auto palleteMode = GL_RGB;

        GLuint textureBinding;
        glGenTextures(1, &textureBinding);
        glBindTexture(GL_TEXTURE_2D, textureBinding);

	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // oldschool vibe
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, palleteMode, surface->w, surface->h, 0, palleteMode, GL_UNSIGNED_BYTE, surface->pixels);
	  glGenerateMipmap(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);

        GLint textureSlot = glGetUniformLocation(shader_program, "texture");
        glUniform1i(textureSlot, 0);

        GLint brightnessSlot = glGetUniformLocation(shader_program, "brightness");
        glUniform1f(brightnessSlot, object->brightness);

        GLint uvSlot = glGetAttribLocation(shader_program, "uvIn");
        glVertexAttribPointer(uvSlot, 2, GL_FLOAT, GL_FALSE, FSGLMesh::glVertexSize, (GLvoid*) (sizeof (GLfloat) * 3));
        glEnableVertexAttribArray(uvSlot);
        
        GLint animationTransformRowOneSlot = glGetAttribLocation(shader_program, "animationTransformRowOne");
        glVertexAttribPointer(animationTransformRowOneSlot, 4, GL_FLOAT, GL_FALSE, FSGLMesh::glVertexSize, (GLvoid*) (sizeof (GLfloat) * 5));
        glEnableVertexAttribArray(animationTransformRowOneSlot);
        
        GLint animationTransformRowTwoSlot = glGetAttribLocation(shader_program, "animationTransformRowTwo");
        glVertexAttribPointer(animationTransformRowTwoSlot, 4, GL_FLOAT, GL_FALSE, FSGLMesh::glVertexSize, (GLvoid*) (sizeof (GLfloat) * 9));
        glEnableVertexAttribArray(animationTransformRowTwoSlot);
        
        GLint animationTransformRowThreeSlot = glGetAttribLocation(shader_program, "animationTransformRowThree");
        glVertexAttribPointer(animationTransformRowThreeSlot, 4, GL_FLOAT, GL_FALSE, FSGLMesh::glVertexSize, (GLvoid*) (sizeof (GLfloat) * 13));
        glEnableVertexAttribArray(animationTransformRowThreeSlot);        
        
        GLint animationTransformRowFourSlot = glGetAttribLocation(shader_program, "animationTransformRowFour");
        glVertexAttribPointer(animationTransformRowFourSlot, 4, GL_FLOAT, GL_FALSE, FSGLMesh::glVertexSize, (GLvoid*) (sizeof (GLfloat) * 17));
        glEnableVertexAttribArray(animationTransformRowFourSlot);        
        
        GLint modelMatrixUniform;
        GLint viewMatrixUniform;

        auto modelMatrix = object->matrix();

        modelMatrixUniform = glGetUniformLocation(shader_program, "modelMatrix");
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        auto viewMatrix = camera->matrix();

        viewMatrixUniform = glGetUniformLocation(shader_program, "viewMatrix");
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_SHORT, 0);

        glDeleteTextures(1, &textureBinding);

	   glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &indexBuffer);

        object->postRenderUpdate();
    }

}
