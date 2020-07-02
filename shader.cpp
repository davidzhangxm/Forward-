#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#include "shader.h"

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Check to make sure the file exists and you passed in the right filepath!\n", vertex_file_path);
		printf("The current working directory is:");
#ifdef _WIN32
		system("CD");
#else
		system("pwd");
#endif
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}
	else {
		printf("Successfully compiled vertex shader!\n");
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}
	else {
		printf("Successfully compiled fragment shader!\n");
	}


	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}
	
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

Program::Program(const char* comp_file_path)
{
	id = LoadShaders(comp_file_path);
}
Program::Program(const char *vertex_file_path, const char *fragment_file_path, const char *geometry_file_path)
{
	id = LoadShaders(vertex_file_path, fragment_file_path, geometry_file_path);
}


Program::Program(const char* vertex_file_path, const char* frag_file_path)
{
	id = LoadShaders(vertex_file_path, frag_file_path);
}

GLuint Program::LoadSingleShader(const char * shaderFilePath, ShaderType type)
{
	// Create a shader id.
	GLuint shaderID = 0;
	if (type == vertex)
		shaderID = glCreateShader(GL_VERTEX_SHADER);
	else if (type == fragment)
		shaderID = glCreateShader(GL_FRAGMENT_SHADER);
	else if (type == geometry)
		shaderID = glCreateShader(GL_GEOMETRY_SHADER);
	else if (type == compute)
		shaderID = glCreateShader(GL_COMPUTE_SHADER);

	// Try to read shader codes from the shader file.
	std::string shaderCode;
	std::ifstream shaderStream(shaderFilePath, std::ios::in);
	if (shaderStream.is_open())
	{
		std::string Line = "";
		while (getline(shaderStream, Line))
			shaderCode += "\n" + Line;
		shaderStream.close();
	}
	else
	{
		std::cerr << "Impossible to open " << shaderFilePath << ". "
			<< "Check to make sure the file exists and you passed in the "
			<< "right filepath!"
			<< std::endl;
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Shader.
	std::cerr << "Compiling shader: " << shaderFilePath << std::endl;
	char const * sourcePointer = shaderCode.c_str();
	glShaderSource(shaderID, 1, &sourcePointer, NULL);
	glCompileShader(shaderID);

	// Check Shader.
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> shaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(shaderID, InfoLogLength, NULL, shaderErrorMessage.data());
		std::string msg(shaderErrorMessage.begin(), shaderErrorMessage.end());
		std::cerr << msg << std::endl;
		return 0;
	}
	else
	{
		if (type == vertex)
			printf("Successfully compiled vertex shader!\n");
		else if (type == fragment)
			printf("Successfully compiled fragment shader!\n");
		else if (type == geometry)
			printf("Sucessfully compiled geometry shader\n");
		else if (type == compute)
			printf("Successfully compiled compute shader\n");
	}

	return shaderID;
}

GLuint Program::LoadShaders(const char * vertexFilePath, const char * fragmentFilePath)
{
	// Create the vertex shader and fragment shader.
	GLuint vertexShaderID = LoadSingleShader(vertexFilePath, vertex);
	GLuint fragmentShaderID = LoadSingleShader(fragmentFilePath, fragment);

	// Check both shaders.
	if (vertexShaderID == 0 || fragmentShaderID == 0) return 0;

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Link the program.
	printf("Linking program\n");
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	// Check the program.
	glGetProgramiv(programID, GL_LINK_STATUS, &Result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(programID, InfoLogLength, NULL, ProgramErrorMessage.data());
		std::string msg(ProgramErrorMessage.begin(), ProgramErrorMessage.end());
		std::cerr << msg << std::endl;
		glDeleteProgram(programID);
		return 0;
	}
	else
	{
		printf("Successfully linked program!\n\n");
	}

	// Detach and delete the shaders as they are no longer needed.
	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragmentShaderID);
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return programID;
}
GLuint Program::LoadShaders(const char* comp_file_path)
{
	GLuint computeShaderID = LoadSingleShader(comp_file_path, compute);
	if (computeShaderID == 0)
		return 0;

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Link the program.
	printf("Linking program\n");
	GLuint programID = glCreateProgram();
	glAttachShader(programID, computeShaderID);
	glLinkProgram(programID);

	// Check the program.
	glGetProgramiv(programID, GL_LINK_STATUS, &Result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(programID, InfoLogLength, NULL, ProgramErrorMessage.data());
		std::string msg(ProgramErrorMessage.begin(), ProgramErrorMessage.end());
		std::cerr << msg << std::endl;
		glDeleteProgram(programID);
		return 0;
	}
	else
	{
		printf("Successfully linked program!\n\n");
	}

	// Detach and delete the shaders as they are no longer needed.
	glDetachShader(programID, computeShaderID);
	glDeleteShader(computeShaderID);

	return programID;

}
GLuint Program::LoadShaders(const char *vertex_file_path, const char *fragment_file_path, const char *geometry_file_path)
{
	// Create the vertex shader and fragment shader.
	GLuint vertexShaderID = LoadSingleShader(vertex_file_path, vertex);
	GLuint fragmentShaderID = LoadSingleShader(fragment_file_path, fragment);
	GLuint geometryShaderID = LoadSingleShader(geometry_file_path, geometry);

	// Check both shaders.
	if (vertexShaderID == 0 || fragmentShaderID == 0 || geometryShaderID == 0) return 0;

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Link the program.
	printf("Linking program\n");
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glAttachShader(programID, geometryShaderID);
	glLinkProgram(programID);

	// Check the program.
	glGetProgramiv(programID, GL_LINK_STATUS, &Result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(programID, InfoLogLength, NULL, ProgramErrorMessage.data());
		std::string msg(ProgramErrorMessage.begin(), ProgramErrorMessage.end());
		std::cerr << msg << std::endl;
		glDeleteProgram(programID);
		return 0;
	}
	else
	{
		printf("Successfully linked program!\n\n");
	}

	// Detach and delete the shaders as they are no longer needed.
	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragmentShaderID);
	glDetachShader(programID, geometryShaderID);
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
	glDeleteShader(geometryShaderID);

	return programID;
}

void Program::use()
{
	glUseProgram(id);
}
void Program::unuse()
{
    glUseProgram(0);
}

void Program::setBool(const char *name, bool value) const
{
	glUniform1i(glGetUniformLocation(id, name), (int)value);
}

void Program::setInt(const char *name, int value) const
{
	glUniform1i(glGetUniformLocation(id, name), value);
}
void Program::setFloat(const char *name, float value) const
{
	glUniform1f(glGetUniformLocation(id, name), value);
}
void Program::setVec3(const char *name, glm::vec3 value) const
{
	glUniform3fv(glGetUniformLocation(id, name), 1, &value[0]);
}
void Program::setMat4(const char *name, glm::mat4 value) const
{
	glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, &value[0][0]);
}
void Program::setInt2(const char* name, glm::ivec2 value) const
{
	glUniform2iv(glGetUniformLocation(id, name), 1, &value[0]);
}