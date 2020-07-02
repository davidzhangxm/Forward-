//
//  Model.hpp
//  TA167-assignment3
//
//  Created by David Zhang on 6/29/20.
//  Copyright © 2020 Xinming Zhang. All rights reserved.
//

#ifndef Model_hpp
#define Model_hpp

#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include "Mesh.hpp"


using namespace std;

GLint TextureFromFile(const char* path, string directory, bool gamma = false);

class Model{
public:
    vector<Texture> texturesLoaded;
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;
    
    // Takes a file path to 3D model
    Model() {}
    Model(const string& path, bool gamma = false): gammaCorrection(gamma)
    {
        LoadModel(path);
    }
    
    void draw(Program shader);
private:
    void LoadModel(string path);
    
    void ProcessNode(aiNode* node, const aiScene* scene);
    
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
    
    vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
};

#endif /* Model_hpp */
