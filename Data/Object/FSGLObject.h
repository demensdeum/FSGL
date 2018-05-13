/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FSGLObject.h
 * Author: demensdeum
 *
 * Created on July 29, 2017, 11:45 AM
 */

#ifndef FSGLOBJECT_H
#define FSGLOBJECT_H

#include "../Model/FSGLModel.h"
#include "../Vector/FSGLVector.h"

#include <glm/glm.hpp>

#include "../Serializable/FSGLSerializable.h"

#include "../Matrix/FSGLMatrix.h"

#include <memory>

using namespace std;

class FSGLObject: public FSGLSerializable {
public:
    FSGLObject(shared_ptr<FSGLModel> model);
    FSGLObject() : FSGLObject(shared_ptr<FSGLModel>()) {};
    virtual ~FSGLObject();
    
    int id;
    
    shared_ptr<FSGLModel> model;    
    
    shared_ptr<FSGLVector> scaleVector;
    shared_ptr<FSGLVector> rotationVector;
    shared_ptr<FSGLVector> positionVector;   
  
    virtual glm::mat4 matrix();
    
    virtual shared_ptr<string> serializeIntoString();
    virtual shared_ptr<FSGLSerializable> deserializeFromString(shared_ptr<string> serializedData);

    virtual shared_ptr<FSGLSerializable> deserializeFromFile(shared_ptr<string> path);
    
    void updateAnimationTransformations();
    
    void applyAnimation(shared_ptr<string> animationName, double animationOffset);
    
    void postRenderUpdate();
    
private:
    
	void resetTransformationMatrix();

    void incrementAnimation();
    
};

#endif /* FSGLOBJECT_H */

