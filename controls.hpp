#ifndef CONTROLS_HPP
#define CONTROLS_HPP

extern bool togglelighting;
extern bool toggleMovement;
extern bool toggleTexture;

void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif
