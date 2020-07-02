//
//  Mesh.hpp
//  TA167-assignment3
//
//  Created by David Zhang on 6/29/20.
//  Copyright © 2020 Xinming Zhang. All rights reserved.
//

#ifndef Mesh_hpp
#define Mesh_hpp

#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>

#include "shader.h"

using namespace std;
using namespace glm;

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 textureCoord;
    vec3 tangent;
    vec3 bitangent;
};

struct Texture {
    GLuint id;
    string type;
    aiString path;
};

class Mesh {
public:
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;
    GLuint VAO;
    
    Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures);
    
    void draw(Program &shader);
private:
    GLuint VBO, EBO;
    
    void setupMesh();
};

#endif /* Mesh_hpp */
