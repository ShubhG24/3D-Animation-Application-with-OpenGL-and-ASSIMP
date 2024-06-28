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
glm:: vec3 ambient_shader;

// Initial position : on +Z
glm::vec3 position = glm::vec3( 0,15,0 ); 
// Initial horizontal angle : toward -Z

// Initial Field of View
float initialFoV = 45.0f;
glm::vec3 origin = glm::vec3(0.0f,0.0f,0.0f);

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.00005;

// Toggle b/w different lightings.
bool togglelighting = true;
bool toggleMovement = false;
bool toggleTexture = false;

// To check whether the "L" key was pressed.
bool lKeyWasPressed = false;
bool gKeyWasPressed = false;
bool tKeyWasPressed = false;

// Initializing values.
float radius = glm::length(position);
float theta = 3.14f;
float phi = 0.0f;
// Increment/Decrement values.
float deltaradius = 0.01;
float deltatheta = glm::radians(0.2f);
float deltaphi = glm::radians(0.2f);

void computeMatricesFromInputs(){

	// Vertical and Horizontal angles.
	theta = acos(position.z/radius);
	phi = atan(position.y/position.x);

	// Clamping the value to avoid errors.
	radius = glm::clamp(radius,0.0f,30.0f);
	// Zoom in
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS)
	{
	  	radius -= deltaradius;
	}
	// Zoom out
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS)
	{
	  	radius += deltaradius;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS)
	{
		phi -= deltaphi;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS)
	{
	  	phi += deltaphi;
	}	
	theta = glm::clamp(theta,0.0f,3.14f);
	// Strafe up
	if (glfwGetKey( window, GLFW_KEY_U ) == GLFW_PRESS)
	{
	  	theta -= deltatheta;
	}
	// Strafe down
	if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS)
	{
	  	theta += deltatheta;
	}

	// Toggle lighting
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) 
	{
		if (!lKeyWasPressed) 
		{
			togglelighting = !togglelighting;
			lKeyWasPressed = true;
		}
	} 
	else 
	{
	  	lKeyWasPressed = false;
	}

	// Toggle movement
	if (glfwGetKey( window, GLFW_KEY_G ) == GLFW_PRESS)
	{
	  	if (!gKeyWasPressed) 
		{
			toggleMovement = !toggleMovement;
			gKeyWasPressed = true;
		}
	}
	else 
	{
	  	gKeyWasPressed = false;
	}

	// Toggle texture
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) 
	{
		if (!tKeyWasPressed) 
		{
			toggleTexture = !toggleTexture;
			tKeyWasPressed = true;
		}
	} 
	else 
	{
	  	tKeyWasPressed = false;
	}

	// Update position values.
	position.x = radius * sin(theta) * cos(phi);
	position.y = radius * sin(theta) * sin(phi);
	position.z = radius * cos(theta);
	

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
				       position,           	// Camera is here.
				       origin, 				// and looks here : at the origin.
				       glm::vec3(0,0,1)     // Head is up (set to 0,-1,0 to look upside-down)
						   	);
	    
}
