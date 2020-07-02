//
//  Camera.hpp
//  opengl_start
//
//  Created by Xinming Zhang on 10/13/19.
//  Copyright © 2019 Xinming Zhang. All rights reserved.
//

#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <stdio.h>

using namespace glm;

const float YAW = 0.0f;
const float PITCH = 0.0f;
const float SPEED = 500.0f;
const float SENSITIVITY = 0.25f;
const float ZOOM = 45.0f;

enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera
{
public:
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 worldUp;

    // angles
    float yaw;
    float pitch;

    // movement
    float movementSpeed;
    float mouseSensitivity;
    float zoom;

    Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) :
        front(vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
    {
        this->position = position;
        this->worldUp = up;
        this->yaw = yaw;
        this->pitch = pitch;
        update();
    }

    mat4 GetViewMatrix();

    void ProcessKeyboard(CameraMovement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    
private:
    void update();
};

#endif /* Camera_hpp */
