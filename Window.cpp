#include "Window.h"
#include <vector>
#include <numeric>
#include <typeinfo>

/* 
 * Declare your variables below. Unnamed namespace is used here to avoid 
 * declaring global or static variables.
 */
namespace
{
	int Width, Height;
	std::string windowTitle("Forward+ Project");
	const glm::ivec2 SCREEN_SIZE(1080, 720);

	glm::vec3 eye(0, 0, 5); // Camera position.
	glm::vec3 center(0, 0, 0); // The point we are looking at.
	glm::vec3 up(0, 1, 0); // The up direction of the camera.

	float near = 0.1f;
	float far = 300.0f;

    Camera camera(vec3(-40.0f, 10.0f, 0.0f));
	glm::mat4 view; // View matrix, defined by eye, center and up.
	glm::mat4 projection; // Projection matrix.

    GLuint depthMapFBO;
    GLuint depthMap;

    Model sponzaModel;

    // tile property
    // X and Y work group dimension variables for compute shader
    GLuint workGroupsX = 0;
    GLuint workGroupsY = 0;

    // Used for storage buffer objects to hold light data and visible light indicies data
    GLuint lightBuffer = 0;
    GLuint visibleLightIndicesBuffer = 0;

	// lights
	int NUM_LIGHTS = 1024;
	// Constants for light animations
	const glm::vec3 LIGHT_MIN_BOUNDS = glm::vec3(-135.0f, -20.0f, -60.0f);
	const glm::vec3 LIGHT_MAX_BOUNDS = glm::vec3(135.0f, 170.0f, 60.0f);
	const float LIGHT_DELTA_TIME = -0.6f;
	const float LIGHT_RADIUS = 30.0f;

	// loop property
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	bool firstMouse = true;
	float lastX = 400.0f;
	float lastY = 300.0f;


	struct PointLight {
		vec4 color;
		vec4 position;
		vec4 paddingAndRadius;
	};

	struct VisibleIndex {
		int index;
	};

	// program
	GLuint quadVAO = 0;
	GLuint quadVBO;
	Program depthShader;
	Program depthRenderShader;
	Program lightCullingShader;
	Program finalShader;
};

void drawQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

vec3 RandomPosition(uniform_real_distribution<> dis, mt19937 gen)
{
	vec3 position = vec3(0.0);
	for (int i = 0; i < 3; ++i)
	{
		float minB = LIGHT_MIN_BOUNDS[i];
		float maxB = LIGHT_MAX_BOUNDS[i];

		position[i] = (float)dis(gen) * (maxB - minB) + minB;
	}
	return position;
}

void SetupLights()
{
	if (lightBuffer == 0)
	{
		return;
	}
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> dis(0, 1);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	PointLight* pointLights = (PointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

	for (int i = 0; i < NUM_LIGHTS; ++i)
	{
		PointLight& light = pointLights[i];
		light.position = vec4(RandomPosition(dis, gen), 1.0f);
		light.color = vec4(1.0f + dis(gen), 1.0f + dis(gen), 1.0f + dis(gen), 1.0f);
		light.paddingAndRadius = vec4(0.0f, 0.0f, 0.0f, LIGHT_RADIUS);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

bool Window::initializeProgram()
{
    depthShader = Program(R"(shaders\depth_vert.glsl)", R"(shaders\depth_frag.glsl)");
	depthRenderShader = Program(R"(shaders\depthRender_vert.glsl)", R"(shaders\depthRender_frag.glsl)");
	lightCullingShader = Program(R"(shaders\light_culling_comp.glsl)");
	finalShader = Program(R"(shaders\final_shading_vert.glsl)", R"(shaders\final_shading_frag.glsl)");

	return true;
}

bool Window::initializeObjects()
{
    // depth prepass obj

    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    GLfloat boarderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, boarderColor);
    
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
	// model
    sponzaModel = Model(R"(model\sponza.obj)");

	initScene();
    
	return true;
}
void Window::initScene()
{
    // tile size 16 x 16
    workGroupsX = (Width + (Width % 16)) / 16;
    workGroupsY = (Height + (Height % 16)) / 16;
    auto numberOfTiles = workGroupsX * workGroupsY;
    
    // Generate lightbuffer
    glGenBuffers(1, &lightBuffer);
    glGenBuffers(1, &visibleLightIndicesBuffer);
    
    // Bind light Buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_LIGHTS * sizeof(PointLight), 0, GL_DYNAMIC_DRAW);

	// Bind visible light indices buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleLightIndicesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleIndex) * 1024, 0, GL_STATIC_DRAW);

	// setup lights
	SetupLights();
}

void Window::cleanUp()
{
	// Deallcoate the objects.

}



GLFWwindow* Window::createWindow(int width, int height)
{
    Width = width;
    Height = height;
	// Initialize GLFW.
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return NULL;
	}

	// 4x antialiasing.
	glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// Apple implements its own version of OpenGL and requires special treatments
	// to make it uses modern OpenGL.

	// Ensure that minimum OpenGL version is 4.4
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	// Enable forward compatibility and allow a modern OpenGL context
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


	// Create the GLFW window.
	GLFWwindow* window = glfwCreateWindow(width, height, windowTitle.c_str(), NULL, NULL);

	// Check if the window could not be created.
	if (!window)
	{
		std::cerr << "Failed to open GLFW window." << std::endl;
		glfwTerminate();
		return NULL;
	}

	// Make the context of the window.
	glfwMakeContextCurrent(window);

#ifndef __APPLE__
	// On Windows and Linux, we need GLEW to provide modern OpenGL functionality.

	// Initialize GLEW.
	if (glewInit())
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return NULL;
	}
#endif

	// Set swap interval to 1.
	glfwSwapInterval(0);


	return window;
}

void Window::idleCallback()
{
	// Perform any updates as necessary.

	// UpdateLights();
}


void Window::displayCallback(GLFWwindow* window)
{
    // Clear the color and depth buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	projection = perspective(camera.zoom, float(Width / Height), near, far);
	view = camera.GetViewMatrix();

	glViewport(0, 0, Width, Height);
	mat4 model = mat4(1.0);
	model = scale(model, vec3(0.1f, 0.1f, 0.1f));
	// step 1: depth prepass
	depthShader.use();
	depthShader.setMat4("projection", projection);
	depthShader.setMat4("view", view);
	depthShader.setMat4("model", model);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	sponzaModel.draw(depthShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 

#if defined(DEPTH_RENDER)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	depthRenderShader.use();
	depthRenderShader.setFloat("near", near);
	depthRenderShader.setFloat("far", far);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	drawQuad();
#endif

	// step 2: light culling
	lightCullingShader.use();
	lightCullingShader.setMat4("projection", projection);
	lightCullingShader.setMat4("view", view);
	lightCullingShader.setInt("lightCount", NUM_LIGHTS);
	lightCullingShader.setInt2("screenSize", SCREEN_SIZE);

	glActiveTexture(GL_TEXTURE4);
	lightCullingShader.setInt("depthMap", 4);
	glBindTexture(GL_TEXTURE_2D, depthMap);


	// Bind shader storage buffer objects for the light and indice buffers
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, visibleLightIndicesBuffer);

	glDispatchCompute(workGroupsX, workGroupsY, 1);

	// Unbind the depth map
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, 0);

#if defined(CULLING_CHECK)
	// map indices buffer back
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleLightIndicesBuffer);
	VisibleIndex* visibleBuffer = (VisibleIndex*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
	size_t numberOfTiles = workGroupsX * workGroupsY;
	for (int i = 0; i < numberOfTiles; ++i)
	{
		cout << "Tile " << i << "==============" << endl;
		uint offset = i * 1024;
		for (uint i = 0; i < NUM_LIGHTS && visibleBuffer[offset + i].index != -1; ++i)
		{
			cout << visibleBuffer[offset + i].index << endl;
		}
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif
	// step 3: final shading
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	finalShader.use();
	finalShader.setMat4("model", model);
	finalShader.setInt("numberOfTilesX", workGroupsX);
	finalShader.setMat4("projection", projection);
	finalShader.setMat4("view", view);
	finalShader.setVec3("viewPosition", camera.position);

	sponzaModel.draw(finalShader);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

    // Gets events, including input such as keyboard and mouse or window resizing.
    glfwPollEvents();
    // Swap buffers.
    glfwSwapBuffers(window);
}

void Window::cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (firstMouse) {
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	GLfloat xOffset = (GLfloat)xpos - lastX;
	GLfloat yOffset = lastY - (GLfloat)ypos;

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}

void Window::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{

}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/*
	 * TODO: Modify below to add your key callbacks.
	 */

	 // Check for a key press.
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
            case GLFW_KEY_ESCAPE:
                // Close the window. This causes the program to also terminate.
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
			case GLFW_KEY_W:
				camera.ProcessKeyboard(FORWARD, deltaTime);
				break;
			case GLFW_KEY_A:
				camera.ProcessKeyboard(LEFT, deltaTime);
				break;
			case GLFW_KEY_D:
				camera.ProcessKeyboard(RIGHT, deltaTime);
				break;
			case GLFW_KEY_S:
				camera.ProcessKeyboard(BACKWARD, deltaTime);
				break;
            default:
                break;
		}
	}
}


void Window::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
}
