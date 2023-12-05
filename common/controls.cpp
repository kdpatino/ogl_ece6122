/*
  Author: Kevin D Patino Sosa
  Class: ECE6122-A
  Last Date Modified: 12/5/2023
  Description: Final Project
*/

// Include GLFW
#include <glfw3.h>
extern GLFWwindow *window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp.
                           // This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix()
{
    return ViewMatrix;
}
glm::mat4 getProjectionMatrix()
{
    return ProjectionMatrix;
}

float initialFoV = 45.0f;

glm::vec3 position = glm::vec3(0, 0, 5);
glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);

float horizontalAngle = 3.14f;
float verticalAngle = 0.0f;
glm::vec3 direction(sin(verticalAngle) * cos(horizontalAngle), sin(verticalAngle),
                    cos(verticalAngle) * cos(horizontalAngle));

glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

/**
 * Updates camera vectors based on the given position and target.
 *
 * This function calculates and updates the direction, right, and up vectors
 * for a camera based on the specified position and target vectors. The resulting
 * vectors are designed to maintain a proper orientation for the camera to look
 * at the specified target from the given position.
 *
 * @param position - The position vector of the camera.
 * @param target - The target vector the camera should be looking at.
 * @param direction - A reference to the direction vector of the camera, which will be updated.
 * @param up - A reference to the up vector of the camera, which will be updated.
 */
void updateCameraVectors(glm::vec3 &position, glm::vec3 &target, glm::vec3 &direction, glm::vec3 &up)
{
    direction = glm::normalize(target - position);
    glm::vec3 right = glm::normalize(glm::cross(direction, up));
    up = glm::cross(right, direction);
}
/**
 * Rotates the camera radially around its target point.
 *
 * This function rotates the camera around its target point by the specified angle
 * in radians. It updates the camera's direction, up vectors, and position accordingly.
 *
 * @param angle - The angle in radians by which to rotate the camera.
 * @param position - The position vector of the camera.
 * @param direction - A reference to the direction vector of the camera, which will be updated.
 * @param up - A reference to the up vector of the camera, which will be updated.
 * @param target - The target vector at which the camera is looking.
 */
void rotateCameraRadially(float angle, glm::vec3 &position, glm::vec3 &direction, glm::vec3 &up, glm::vec3 &target)
{
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::cross(direction, up));
    direction = glm::mat3(rotationMatrix) * direction;
    up = glm::mat3(rotationMatrix) * up;
    position += up * angle;
}

/**
 * Updates the camera's position based on a specified angle.
 *
 * This function updates the camera's position vector by moving it along the right
 * vector (perpendicular to the direction and up vectors) by a given angle in radians.
 * The camera's direction vector is also updated to maintain the proper orientation.
 *
 * @param angle - The angle in radians by which to update the camera's position.
 * @param position - A reference to the position vector of the camera, which will be updated.
 * @param direction - A reference to the direction vector of the camera, which will be updated.
 * @param up - A reference to the up vector of the camera.
 */
void updateCameraPosition(float angle, glm::vec3 &position, glm::vec3 &direction, glm::vec3 &up)
{
    //glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, up);
    //direction = glm::mat3(rotationMatrix) * direction;
    glm::vec3 right = glm::cross(direction, up);
    position += right * angle;
}

/**
 * Moves the camera radially along its direction vector.
 *
 * This function moves the camera along its direction vector by a specified distance.
 * It updates the camera's position accordingly.
 *
 * @param distance - The distance by which to move the camera along its direction vector.
 * @param position - A reference to the position vector of the camera, which will be updated.
 * @param direction - A reference to the direction vector of the camera.
 */
void moveCameraRadially(float distance, glm::vec3 &position, glm::vec3 &direction)
{
    position += direction * distance;
}

void computeMatricesFromInputs()
{

    // glfwGetTime is called only once, the first time this function is called
    static double lastTime = glfwGetTime();

    // Compute time difference between current and last frame
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        updateCameraPosition(0.05f, position, direction, up);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        updateCameraPosition(-0.05f, position, direction, up);
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        moveCameraRadially(-0.05f, position, direction);
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        moveCameraRadially(0.05f, position, direction);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        rotateCameraRadially(-0.05f, position, direction, up, target); // Rotate radially upward
    }

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
        rotateCameraRadially(0.05f, position, direction, up, target); // Rotate radially downward
    }
    updateCameraVectors(position, target, direction, up);
    // Up vector
    float FoV = initialFoV; // - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a
                            // bit too complicated for this beginner's tutorial, so it's disabled instead.

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    ViewMatrix = glm::lookAt(position,             // Camera is here
                             position + direction, // and looks here : at the same position, plus "direction"
                             up                    // Head is up (set to 0,-1,0 to look upside-down)
    );

    // For the next frame, the "last time" will be "now"
    lastTime = currentTime;
}