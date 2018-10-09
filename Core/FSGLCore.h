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

#ifndef FSGLCORE_H
#define FSGLCORE_H

#include <string>
#include <memory>
#include <map>

struct SDL_Window;
class FSGLObject;
class FSGLCamera;

using namespace std;

namespace FSGL {
	class Renderer;
};

using namespace FSGL;

class FSGLCore {
public:
    FSGLCore();
    virtual ~FSGLCore();
    
    SDL_Window* initialize();
    
	//Create, Remove
    void addObject(shared_ptr<FSGLObject> object);
    void removeObject(shared_ptr<FSGLObject> object);
    void removeAllObjects();

    shared_ptr<FSGLObject> getObjectWithID(string id);
    
    void render();
    void stop();
    
    shared_ptr<FSGLCamera> camera();

private:
	map<string, shared_ptr<FSGLObject>> idObjectMap;
	shared_ptr<Renderer> renderer;

};

#endif /* FSEOGLCORE_H */

