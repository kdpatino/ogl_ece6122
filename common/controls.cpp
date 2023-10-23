// Include GLFW
#include <glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

float initialFoV = 45.0f;

glm::vec3 position = glm::vec3( 0, 0, 5 );
glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);

float horizontalAngle = 3.14f;
float verticalAngle = 0.0f;
glm::vec3 direction(
		sin(verticalAngle) * cos(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

glm::vec3 up = glm::vec3(0.0f,1.0f,0.0f);

void updateCameraVectors(glm::vec3& position, glm::vec3& target, glm::vec3& direction, glm::vec3& up) {
    direction = glm::normalize(target - position);
    glm::vec3 right = glm::normalize(glm::cross(direction, up));
    up = glm::cross(right, direction);
}

void rotateCameraRadially(float angle, glm::vec3& position, glm::vec3& direction, glm::vec3& up, glm::vec3& target) {
    glm::vec3 currentPosition = position;

    // Calculate new position in spherical coordinates
    float x = currentPosition.x * cos(angle);
    float y = currentPosition.y * cos(angle) - currentPosition.z * sin(angle);
    float z = currentPosition.x * sin(angle) + currentPosition.y * sin(angle) + currentPosition.z * cos(angle);

    position = glm::vec3(x, y, z);
}

void updateCameraPosition(float angle, glm::vec3& position, glm::vec3& direction) {
    glm::vec3 currentPosition = position;
    float x = currentPosition.x * cos(angle) - currentPosition.z * sin(angle);
    float z = currentPosition.x * sin(angle) + currentPosition.z * cos(angle);
    position = glm::vec3(x, currentPosition.y, z);
}

void moveCameraRadially(float distance, glm::vec3& position, glm::vec3& direction) {
    position += direction * distance;
}


void computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

 	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		updateCameraPosition(0.01f, position, direction);

    }
 	
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		updateCameraPosition(-0.01f, position, direction);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        moveCameraRadially(-0.1f, position, direction); 
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        moveCameraRadially(0.1f, position, direction); 
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        rotateCameraRadially(0.01f, position, direction,up,target);  // Rotate radially upward
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        rotateCameraRadially(-0.01f, position, direction,up,target);  // Rotate radially downward
    }
	updateCameraVectors(position,target,direction,up);
	// Up vector
	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
								position,           // Camera is here
								position+direction, // and looks here : at the same position, plus "direction"
								up                  // Head is up (set to 0,-1,0 to look upside-down)
						   );

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}