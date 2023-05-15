#ifndef __BASENODE_H__
#define __BASENODE_H__

#include "../common/vgl.h"
#include "../common/vmath.h"

class BaseNode
{
public:
    // Object buffers
    GLuint VAO;
    GLuint PosBuff;
    GLuint PosPtr;
    GLint PosCoords;
    GLint NumVertices;
    
	// Shader references
	GLuint Shader;
	GLuint ProjMatPtr;
	GLuint CamMatPtr;
	GLuint ModMatPtr;

	// Object properties
    vmath::mat4 BaseTransform;
	vmath::mat4 ModelTransform;
	BaseNode* Sibling;
	BaseNode* Child;


    BaseNode() {
        BaseTransform = vmath::mat4().identity();
        ModelTransform = vmath::mat4().identity();
        Shader = 0;
        Child = NULL;
        Sibling = NULL;
    }

    void set_shader(GLuint shader, GLuint pPtr, GLuint cPtr, GLuint mPtr) {
        Shader = shader;
        ProjMatPtr = pPtr;
        CamMatPtr = cPtr;
        ModMatPtr = mPtr;
    }

    virtual void draw(vmath::mat4 proj, vmath::mat4 cam, vmath::mat4 trans) = 0;

    void set_base_transform(vmath::mat4 transform) {
        BaseTransform = transform;
    }

    void set_update_transform(vmath::mat4 transform) {
    	ModelTransform = transform;
    };

    void attach_nodes(BaseNode* child, BaseNode* sibling) {
        Child = child;
        Sibling = sibling;
    }
};

#endif /* __BASENODE_H__ */