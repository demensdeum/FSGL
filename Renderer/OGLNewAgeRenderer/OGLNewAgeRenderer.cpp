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
#include <FlameSteelEngineGameToolkit/IO/IOSystems/FSEGTIOGenericSystemParams.h>

#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <thread>

#include <fstream>
#include <iostream>
#include <set>

#include <json/json.hpp>
#include "OGLNewAgeRendererElements.h"

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
        //cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << endl << infoLog << endl;
	throw logic_error("Can't compile shader program");
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        //cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << endl << infoLog << endl;
	throw logic_error("Can't compile shader program");
    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        //cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED" << endl << infoLog << endl;
	throw logic_error("Can't link shader program");
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

OGLNewAgeRenderer::~OGLNewAgeRenderer() {

}

void OGLNewAgeRenderer::cleanObjectCache(shared_ptr<FSGLObject> object) {

	auto model = object->model;
	auto size = model->meshes.size();
	for (unsigned int meshIndex = 0; meshIndex < size; meshIndex++) {
		auto mesh = model->meshes[meshIndex];
		mapMeshElementsMap.erase(mesh);
	}
	mapObjectElementsMap.erase(object);

}

void OGLNewAgeRenderer::removeObject(shared_ptr<FSGLObject> object) {

	cleanObjectCache(object);

	auto layer = object->layer;
	
	if (layerToObjectsMap.find(layer) == layerToObjectsMap.end()) {
		throw runtime_error("Can't remove object, because layer does not exist in layer map");
	}

	auto objects = layerToObjectsMap[layer];

	for (size_t i = 0; i < objects.size(); i++) {

		auto iteratedObject = objects[i];

		if (object->id == iteratedObject->id) {

			objects.erase(objects.begin() + i);

			layerToObjectsMap[layer] = objects;

			break;

		}
	}

	if (objects.size() < 1) {

		layerToObjectsMap.erase(layer);

		cout << "Layer removed" << endl;

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

SDL_Window* OGLNewAgeRenderer::initialize(shared_ptr<FSEGTIOGenericSystemParams> params) {

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,16);
    
    SDL_Init(SDL_INIT_VIDEO);

	if (params.get() == nullptr) {
		throw logic_error("Can't initialize renderer - params is null");
	}

	if (params->title.get() == nullptr) {
		throw logic_error("Can't initialize renderer - title is null in params");
	}

	auto title = params->title->c_str();
	auto width = params->width;
	auto height = params->height;

    window = SDL_CreateWindow(
            title,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width,
            height,
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

	auto model = object->model;
	auto size = model->meshes.size();

	for (unsigned int meshIndex = 0; meshIndex < size; meshIndex++) {

		auto mesh = model->meshes[meshIndex];
		auto elements = make_shared<OGLNewAgeRendererElements>(mesh);	

		mapMeshElementsMap[mesh] = elements;
		mapObjectElementsMap[object] = elements;
	}

	auto layer = object->layer;

	if (layerToObjectsMap.find(layer) == layerToObjectsMap.end()) {
		auto newLayer = vector<shared_ptr<FSGLObject>>();
		layerToObjectsMap[layer] =  newLayer;
	}

	auto objectsLayer = layerToObjectsMap[layer];
	objectsLayer.push_back(object);

	layerToObjectsMap[layer] = objectsLayer;

}

void OGLNewAgeRenderer::render() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepthf(1.0f);

	for (auto pair : layerToObjectsMap) {

		auto objects = pair.second;

		glClear(GL_DEPTH_BUFFER_BIT);
		glClearDepthf(1.0f);

		for (auto object : objects) {
			if (object.get() == nullptr) {

				throw runtime_error("can't render null object");

			}

				renderObject(object);
		}
	}

    SDL_GL_SwapWindow(window);
}

void OGLNewAgeRenderer::stop() {

}

shared_ptr<FSGLObject> OGLNewAgeRenderer::getObjectWithID(string id) {
    
	if (idToObjectMap.find(id) == idToObjectMap.end()) {

		throw runtime_error("Id not found in id to object map, it was added? it was removed?");

	}

    return idToObjectMap[id];
}

void OGLNewAgeRenderer::renderObject(shared_ptr<FSGLObject> object) {
    
    auto model = object->model;

    for (unsigned int meshIndex = 0; meshIndex < model->meshes.size(); meshIndex++) {

        auto mesh = model->meshes[meshIndex];

        mesh->updateGlAnimationTransformation();

        GLint vertexSlot = glGetAttribLocation(shader_program, "vertex");

	auto elements = mapMeshElementsMap[mesh];

        glVertexAttribPointer(vertexSlot, 3, GL_FLOAT, GL_FALSE, FSGLMesh::glVertexSize, 0);
        glEnableVertexAttribArray(vertexSlot);

        auto material = mesh->material;

	if (material == NULL) {

            throw logic_error("OGLNewAgeRenderer: cannot load material");

	}

        auto surface = material->surface;

        if (surface == NULL) {

            //cout << "OGLNewAgeRenderer: cannot load texture " << material->texturePath->c_str() << endl;
	
		throw logic_error("OGLNewAgeRenderer: cannot load texture");

        }

        glActiveTexture(GL_TEXTURE0);

        GLint textureSlot = glGetUniformLocation(shader_program, "texture");
        glUniform1i(textureSlot, 0);

        GLint brightnessSlot = glGetUniformLocation(shader_program, "brightness");
        glUniform1f(brightnessSlot, object->brightness);

        GLint uvSlot = glGetAttribLocation(shader_program, "uvIn");
        glVertexAttribPointer(uvSlot, 2, GL_FLOAT, GL_FALSE, FSGLMesh::glVertexSize, (GLvoid*) (sizeof (GLfloat) * 3));
        glEnableVertexAttribArray(uvSlot);   
        
        GLint modelMatrixUniform;
        GLint viewMatrixUniform;

        auto modelMatrix = object->matrix();

        modelMatrixUniform = glGetUniformLocation(shader_program, "modelMatrix");
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        auto viewMatrix = camera->matrix();

        viewMatrixUniform = glGetUniformLocation(shader_program, "viewMatrix");
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	  elements->performPreRender();

        glDrawElements(GL_TRIANGLES, elements->indicesCount, GL_UNSIGNED_SHORT, 0);

        object->postRenderUpdate();
    }

}
