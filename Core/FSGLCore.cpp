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

#include "FSGLCore.h"

#include "../Data/Model/FSGLModel.h"

#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <thread>

#include <fstream>
#include <iostream>
#include <set>

using namespace std;

static const auto FSGLCoreScreenWidth = 1024;
static const auto FSGLCoreScreenHeight = 576;

static const GLchar* vertexShaderSource =
        "#version 100\n"
        "uniform mat4 projectionMatrix;\n"
        "uniform mat4 viewMatrix;\n"
        "uniform mat4 modelMatrix;\n"

        "attribute vec4 vertex;\n"
        "attribute vec2 uvIn;\n"

        "attribute vec4 animationTransformRowOne;\n"
        "attribute vec4 animationTransformRowTwo;\n"
        "attribute vec4 animationTransformRowThree;\n"
        "attribute vec4 animationTransformRowFour;\n"

        "varying vec2 uvOut;\n"
        "void main() {\n"
        "   mat4 animationTransform = mat4(animationTransformRowOne, animationTransformRowTwo,animationTransformRowThree, animationTransformRowFour);\n"
        "   vec4 vertexPosition = vertex * animationTransform;\n"
        "   gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPosition;\n"
        "   uvOut = uvIn;\n"
        "}\n";

static const GLchar* fragmentShaderSource =
        "#version 100\n"
	  "precision mediump int;\n"
	  "precision mediump float;\n"
	  "precision lowp sampler2D;\n"
	  "precision lowp samplerCube;\n"
        "varying lowp vec2 uvOut;\n"
        "uniform sampler2D texture;\n"
	  "uniform float brightness;\n"
        "void main() {\n"
	 "vec4 color = texture2D(texture, uvOut);\n"
	 "color.r = brightness * color.r;\n"
	 "color.g = brightness * color.g;\n"
	 "color.b = brightness * color.b;\n"
        "   gl_FragColor = color;"
        "}\n";

GLint FSGLCore::common_get_shader_program(const char *vertex_shader_source, const char *fragment_shader_source) {

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
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

FSGLCore::FSGLCore() {

    camera = make_shared<FSGLCamera>();
    
}

void FSGLCore::removeObject(shared_ptr<FSGLObject> object) {

	    for (size_t i = 0; i < objects.size(); i++) {
	
		if (object.get() == objects[i].get()) {

			objects.erase(objects.begin() + i);

			return;
		}
		}
}

void FSGLCore::removeAllObjects() {
    
    for (size_t i = 0; i < objects.size(); i++) {
        
        objects.pop_back();
        
        i--;
        
    }
   
}

static void  FSGL_openGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{ 
	if (type != GL_DEBUG_TYPE_OTHER)
	{
		cout << "OpenGL: "<< message << endl;
	}
}

SDL_Window* FSGLCore::initialize() {

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,16);
    
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
            "Death Mask - Prototype",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            FSGLCoreScreenWidth,
            FSGLCoreScreenHeight,
            SDL_WINDOW_OPENGL
            );

    if (window == NULL) {

	printf("Could not create window: %s\n", SDL_GetError());
	exit(1);
        
    } 
    
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );        
    
    context = SDL_GL_CreateContext(window);

    if (context == NULL) {
     
        printf("SDL_Init failed: %s\n", SDL_GetError());
        
    }
    
    GLenum err = glewInit();
    
    if (GLEW_OK != err) {
        
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        
    }
    
    SDL_GL_MakeCurrent(window, context);
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, FSGLCoreScreenWidth, FSGLCoreScreenHeight);

    shader_program = common_get_shader_program(vertexShaderSource, fragmentShaderSource);
    
    glUseProgram(shader_program);
    
    GLint projectionMatrixUniform;    
    
    glm::mat4 projectionMatrix = glm::perspective(45.0f, float(float(FSGLCoreScreenWidth) / float(FSGLCoreScreenHeight)), 0.001f, 200.0f);
    projectionMatrixUniform = glGetUniformLocation(shader_program, "projectionMatrix");
    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));    
 
	return window;

}

void FSGLCore::addObject(shared_ptr<FSGLObject> object) {

    objects.push_back(object);

}

void FSGLCore::render() {

    // clear

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepthf(1.0f);

    for (unsigned int i = 0; i < objects.size(); i++) {

        auto object = objects[i];

	if (object.get() == nullptr) {

		cout << "FSGLCore cannot render empty object" << endl;

		exit(1);

	}

        auto objectID = object->id;
        
        if (renderIDs.size() < 1 || renderIDs.find(objectID) != renderIDs.end()) {
        
            renderObject(object);
            
        }

    }

    SDL_GL_SwapWindow(window);
}

void FSGLCore::addRenderID(string id) {
    
    renderIDs.insert(id);
    
}

void FSGLCore::cleanRenderIDs() {
    
    renderIDs.clear();
    
}

void FSGLCore::stop() {

}

shared_ptr<FSGLObject> FSGLCore::getObjectWithID(string id) {
    
    for (size_t i = 0; i < objects.size(); i++) {
        
        auto object = objects[i];
        
        if (object->id == id) {
            
            return object;
            
        }
        
    }
    
    return shared_ptr<FSGLObject>();
}

void FSGLCore::renderObject(shared_ptr<FSGLObject> object) {
    
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

            throw logic_error("FSGLCore: cannot load material");

	}

        auto surface = material->surface;

        if (surface == NULL) {

            cout << "FSGLCore: cannot load texture " << material->texturePath->c_str() << endl;
	
		throw logic_error("FSGLCore: cannot load texture");

        }

        auto palleteMode = GL_RGB;

        GLuint textureBinding;
        glGenTextures(1, &textureBinding);
        glBindTexture(GL_TEXTURE_2D, textureBinding);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &indexBuffer);

        object->postRenderUpdate();
    }

}

FSGLCore::~FSGLCore() {
}

