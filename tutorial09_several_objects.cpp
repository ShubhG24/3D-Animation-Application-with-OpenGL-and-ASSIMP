// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <random>
// Include GLEW
#include <GL/glew.h>
#include <cstdlib>
// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>



// Atomic flag to signal thread termination
//std::atomic<bool> terminateThreads(false);

// Function to update model matrix
void updateModelMatrix(glm::mat4& modelMatrix)
{
	//location updation logic//
	double lastTime;
	double currentTime = glfwGetTime();
	double deltaTime = currentTime - lastTime;
	lastTime =currentTime;
   
	float speed = 0.5f;

    float  float_dist = speed*deltaTime;
   
   	modelMatrix = glm::translate(modelMatrix,glm::vec3((float)(rand()% 5), (float)(rand()% 5), (float)(rand()% 5)));

    modelMatrix = glm::rotate(modelMatrix,float_dist* glm::radians(180.0f),glm::vec3(0.0f, 1.0f, 0.0f));
	     
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
}

int main( void )
{

	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}
    
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 09 - Rendering several models", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
    
	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");



	// Get a handle for our buffers
	GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
	GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
	GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");

	// Load the texture
	GLuint Texture = loadDDS("uvmap.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	std::thread thread1;

	do{
	  
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0/double(nbFrames));
			nbFrames = 0;
			lastTime =currentTime;
		}
	  
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();


	    glUseProgram(programID);
		GLuint toggleLightingLoc = glGetUniformLocation(programID, "togglelighting");
		glUniform1i(toggleLightingLoc, togglelighting ? 1 : 0);	

		GLuint toggleMovementLoc = glGetUniformLocation(programID, "toggleMovement");
		glUniform1i(toggleMovementLoc, toggleMovement ? 1 : 0);

		GLuint toggleTextureLoc = glGetUniformLocation(programID, "toggleTexture");
		glUniform1i(toggleTextureLoc, toggleTexture ? 1 : 0);

		GLuint justgreenloc = glGetUniformLocation(programID,"JustGreen");

		srand(time(NULL));
       
	        
		float randomFloat = (float)(rand()%(5-1+1));

		GLuint lightIntensityLoc = glGetUniformLocation(programID, "LightIntensity");
		glUniform1f(lightIntensityLoc, randomFloat);

		// Use our shader
		glUseProgram(programID);
		glUniform1i(justgreenloc,0);
		glm::vec3 lightPos = glm::vec3(4,4,4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

		
		glm::mat4 ModelMatrix1 = glm::mat4(1.0);
		// Translating on z-axis, to adjust suzanne chin on rectangle.
		ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(0.0f, 0.0f, 0.99f));
		// Translating and rotating to make 4 heads touch ear to ear.
		ModelMatrix1 = glm::translate(ModelMatrix1, glm::vec3(1.85f, 0.0f, 0.0f));
		ModelMatrix1 = glm::rotate(ModelMatrix1, glm::radians(90.0f),glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix1 = glm::rotate(ModelMatrix1, glm::radians(90.0f),glm::vec3(1.0f, 0.0f, 0.0f));
		glUniform3f(LightID, lightPos.x = 1.85f, lightPos.y = 0.0f, lightPos.z = 0.0f);

		

		if(toggleMovement == 1)
		{
		    
			
			srand(time(NULL));

			double currentTime = glfwGetTime();
			double deltaTime = currentTime - lastTime;
			double lastTime =currentTime;
			float  speed = 0.5f;

			float float_dist = speed*deltaTime;
			double maxX = 5.0f;
			double minX = -5.0f;
			double maxY = 5.0f;
			double minY = 0.0f;
			
			glm::vec3 randm_movt = glm::vec3((float)(rand()% 5),(float)(rand()% 5),(float)(rand()% 5));
			
		
			if(minX<randm_movt.x<maxX && minY<randm_movt.y<maxY )
			{
				randm_movt.x += (float_dist+1.0);
				randm_movt.y +=(float_dist+1.0);
				randm_movt.z +=(float_dist+1.0);
				
				ModelMatrix1 = glm::translate(ModelMatrix1,glm::vec3(randm_movt.x,randm_movt.y,randm_movt.z));


				ModelMatrix1 = glm::rotate(ModelMatrix1,float_dist* glm::radians(180.0f),glm::vec3(0.0f, 1.0f, 0.0f));
			}
			else
			{
				randm_movt.x = -randm_movt.x;
				randm_movt.y = -randm_movt.y;
				randm_movt.z = -randm_movt.z;
				
				ModelMatrix1 = glm::translate(ModelMatrix1,glm::vec3(randm_movt.x,randm_movt.y,randm_movt.z));


				ModelMatrix1 = glm::rotate(ModelMatrix1,float_dist* glm::radians(180.0f),glm::vec3(0.0f, 1.0f, 0.0f));
			}
	
   	    
		}

		
		glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);


		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(vertexPosition_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			vertexPosition_modelspaceID, // The attribute we want to configure
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(vertexUVID);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			vertexUVID,                       // The attribute we want to configure
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(vertexNormal_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			vertexNormal_modelspaceID,        // The attribute we want to configure
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			indices.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
		);
		



		////// End of rendering of the first object //////
		////// Start of the rendering of the second object //////

		// In our very specific case, the 2 objects use the same shader.
		// So it's useless to re-bind the "programID" shader, since it's already the current one.
		//glUseProgram(programID);
		
		// Similarly : don't re-set the light position and camera matrix in programID,
		// it's still valid !
		// *** You would have to do it if you used another shader ! ***
		//glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		//glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

		
		// Again : this is already done, but this only works because we use the same shader.
		//// Bind our texture in Texture Unit 0
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, Texture);
		//// Set our "myTextureSampler" sampler to user Texture Unit 0
		//glUniform1i(TextureID, 0);
		
		
		// BUT the Model matrix is different (and the MVP too)
		// Setting justgreen variable to 0, since only rectangle needs to be green.
		glUniform1i(justgreenloc,0);

		glm::mat4 ModelMatrix2 = glm::mat4(1.0);
		ModelMatrix2 = glm::translate(ModelMatrix2, glm::vec3(0.0f, 0.0f, 0.99f));
		ModelMatrix2 = glm::translate(ModelMatrix2, glm::vec3(-1.85f, 0.0f, 0.0f));
		ModelMatrix2 = glm::rotate(ModelMatrix2, glm::radians(270.0f),glm::vec3(0.0f, 0.0f, 1.0f));
		glUniform3f(LightID, lightPos.x =1.85f, lightPos.y = 0.0f, lightPos.z = 0.99f);

		ModelMatrix2 = glm::rotate(ModelMatrix2, glm::radians(90.0f),glm::vec3(1.0f, 0.0f, 0.0f));
	        

		if(toggleMovement == 1)
		{
			double currentTime = glfwGetTime();
			double deltaTime = currentTime - lastTime;
			double lastTime =currentTime;

			float speed = 0.5f;

			float  float_dist = speed*deltaTime;

			ModelMatrix2 = glm::translate(ModelMatrix2,glm::vec3((float)(rand()% 5), (float)(rand()% 5), (float)(rand()% 5)));

			ModelMatrix2 = glm::rotate(ModelMatrix2,float_dist* glm::radians(180.0f),glm::vec3(0.0f, 1.0f, 0.0f));
		}

		glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrix2;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix2[0][0]);


		// The rest is exactly the same as the first object
		
		// 1rst attribute buffer : vertices
		//glEnableVertexAttribArray(vertexPosition_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		//glEnableVertexAttribArray(vertexUVID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		//glEnableVertexAttribArray(vertexNormal_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);


		////// End of rendering of the second object //////


		////// End of rendering of the second object //////
		////// Start of the rendering of the third object //////

		// In our very specific case, the 2 objects use the same shader.
		// So it's useless to re-bind the "programID" shader, since it's already the current one.
		//glUseProgram(programID);
		
		// Similarly : don't re-set the light position and camera matrix in programID,
		// it's still valid !
		// *** You would have to do it if you used another shader ! ***
		//glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		//glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

		
		// Again : this is already done, but this only works because we use the same shader.
		//// Bind our texture in Texture Unit 0
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, Texture);
		//// Set our "myTextureSampler" sampler to user Texture Unit 0
		//glUniform1i(TextureID, 0);
		
		
		// BUT the Model matrix is different (and the MVP too)
		glUniform1i(justgreenloc,0);

	        glm::mat4 ModelMatrix3 = glm::mat4(1.0);
		ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(0.0f, 0.0f, 0.99f));
		ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(0.0f, 1.85f, 0.0f));
		ModelMatrix3 = glm::rotate(ModelMatrix3, glm::radians(180.0f),glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix3 = glm::rotate(ModelMatrix3, glm::radians(90.0f),glm::vec3(1.0f, 0.0f, 0.0f));
		glUniform3f(LightID, lightPos.x = 0.0f, lightPos.y = 1.85f, lightPos.z = 0.99f);

		if(toggleMovement == 1)
		{
			double currentTime = glfwGetTime();
			double deltaTime = currentTime - lastTime;
			double lastTime =currentTime;
		
			float speed = 0.5f;

			float  float_dist = speed*deltaTime;
		
			ModelMatrix3 = glm::translate(ModelMatrix3,glm::vec3((float)(rand()% 5), (float)(rand()% 5), (float)(rand()% 5)));

			ModelMatrix3 = glm::rotate(ModelMatrix3,float_dist* glm::radians(180.0f),glm::vec3(0.0f, 1.0f, 0.0f));
		}
		glm::mat4 MVP3 = ProjectionMatrix * ViewMatrix * ModelMatrix3;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP3[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix3[0][0]);


		// The rest is exactly the same as the first object
		
		// 1rst attribute buffer : vertices
		//glEnableVertexAttribArray(vertexPosition_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		//glEnableVertexAttribArray(vertexUVID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		//glEnableVertexAttribArray(vertexNormal_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);


		////// End of rendering of the third object //////
        ////// Start of the rendering of the fourth object //////

		// In our very specific case, the 2 objects use the same shader.
		// So it's useless to re-bind the "programID" shader, since it's already the current one.
		//glUseProgram(programID);
		
		// Similarly : don't re-set the light position and camera matrix in programID,
		// it's still valid !
		// *** You would have to do it if you used another shader ! ***
		//glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		//glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

		
		// Again : this is already done, but this only works because we use the same shader.
		//// Bind our texture in Texture Unit 0
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, Texture);
		//// Set our "myTextureSampler" sampler to user Texture Unit 0
		//glUniform1i(TextureID, 0);
		
 		glUniform1i(justgreenloc,0);

		
		// BUT the Model matrix is different (and the MVP too)
		 glm::mat4 ModelMatrix4 = glm::mat4(1.0);
		 ModelMatrix4 = glm::translate(ModelMatrix4, glm::vec3(0.0f, 0.0f, 0.99f));
		 ModelMatrix4 = glm::translate(ModelMatrix4, glm::vec3(0.0f, -1.85f, 0.0f));
		 ModelMatrix4 = glm::rotate(ModelMatrix4, glm::radians(0.0f),glm::vec3(0.0f, 0.0f, 1.0f));
		 ModelMatrix4 = glm::rotate(ModelMatrix4, glm::radians(90.0f),glm::vec3(1.0f, 0.0f, 0.0f));

        if(toggleMovement == 1)
		{
		    double currentTime = glfwGetTime();
		    double deltaTime = currentTime - lastTime;
		    double lastTime =currentTime;
		
			float speed = 0.5f;

			float  float_dist = speed*deltaTime;
		
			ModelMatrix4 = glm::translate(ModelMatrix4,glm::vec3((float)(rand()% 5), (float)(rand()% 5), (float)(rand()% 5)));

			ModelMatrix4 = glm::rotate(ModelMatrix4,float_dist* glm::radians(180.0f),glm::vec3(0.0f, 1.0f, 0.0f));
		}

		glUniform3f(LightID, lightPos.x = 0.0f, lightPos.y = -1.85f, lightPos.z = 0.99f);

		glm::mat4 MVP4 = ProjectionMatrix * ViewMatrix * ModelMatrix4;


		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP4[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix4[0][0]);


		// The rest is exactly the same as the first object
		
		// 1rst attribute buffer : vertices
		//glEnableVertexAttribArray(vertexPosition_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		//glEnableVertexAttribArray(vertexUVID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		//glEnableVertexAttribArray(vertexNormal_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);
		/// end of 4th suzanne.
		//
 		glUniform1i(justgreenloc,0);

		
		// BUT the Model matrix is different (and the MVP too)
		glm::mat4 ModelMatrix5 = glm::mat4(1.0);
		ModelMatrix5 = glm::scale(ModelMatrix5, glm::vec3(0.5f, 0.5f, 0.5f)); 
		ModelMatrix5 = glm::translate(ModelMatrix5, glm::vec3(0.0f, 0.0f, 0.99f));
		ModelMatrix5 = glm::translate(ModelMatrix5, glm::vec3(-10.0f, 3.5f, 0.0f));
		ModelMatrix5 = glm::translate(ModelMatrix5, glm::vec3(0.0f, 8.0f, 4.0f));

		ModelMatrix5 = glm::rotate(ModelMatrix5, glm::radians(0.0f),glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix5 = glm::rotate(ModelMatrix5, glm::radians(90.0f),glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 MVP5 = ProjectionMatrix * ViewMatrix * ModelMatrix5;
		//glUniform3f(LightID, lightPos.x = -10.0f, lightPos.y = 11.5f, lightPos.z = 4.99f);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP5[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix5[0][0]);


		// The rest is exactly the same as the first object
		
		// 1rst attribute buffer : vertices
		//glEnableVertexAttribArray(vertexPosition_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		//glEnableVertexAttribArray(vertexUVID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		//glEnableVertexAttribArray(vertexNormal_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);


		///// end of 5th object//////
		//// start of 6th suzzanne head//////
		glUniform1i(justgreenloc,0);

		
		// BUT the Model matrix is different (and the MVP too)
		glm::mat4 ModelMatrix6 = glm::mat4(1.0);
		ModelMatrix6 = glm::scale(ModelMatrix6, glm::vec3(0.5f, 0.5f, 0.5f)); 
		ModelMatrix6 = glm::translate(ModelMatrix6, glm::vec3(0.0f, 0.0f, 0.99f));
		ModelMatrix6 = glm::translate(ModelMatrix6, glm::vec3(10.0f, 3.5f, 0.0f));
		ModelMatrix6 = glm::translate(ModelMatrix6, glm::vec3(0.0f, 8.0f, 4.0f));

		ModelMatrix6 = glm::rotate(ModelMatrix6, glm::radians(0.0f),glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix6 = glm::rotate(ModelMatrix6, glm::radians(90.0f),glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 MVP6 = ProjectionMatrix * ViewMatrix * ModelMatrix6;
		//glUniform3f(LightID, lightPos.x = -8.0f, lightPos.y = -1.0f, lightPos.z = 4.99f);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP6[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix6[0][0]);


		// The rest is exactly the same as the first object
		
		// 1rst attribute buffer : vertices
		//glEnableVertexAttribArray(vertexPosition_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		//glEnableVertexAttribArray(vertexUVID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		//glEnableVertexAttribArray(vertexNormal_modelspaceID); // Already enabled
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);

		/// end of 6th suzzanne//////

		
		///// Start of rendering the floor //////
		glUseProgram(programID);
		// Setting JustGreen to 1 as we need the rectangle to be green.
		glUniform1i(justgreenloc, 1);

		glm::mat4 ModelMatrixRect = glm::mat4(1.0);
		ModelMatrixRect = glm::scale(ModelMatrixRect, glm::vec3(100.0f, 100.0f, 0.0f)); // Scale the rectangle
		ModelMatrixRect = glm::translate(ModelMatrixRect, glm::vec3(0.0f,0.0f,0.0f)); 
		glm::mat4 MVPRect = ProjectionMatrix * ViewMatrix * ModelMatrixRect;

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPRect[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrixRect[0][0]);

		if(toggleTexture == 1)
		{
		glUniform1i(justgreenloc, 0);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);
		}
		glBegin(GL_TRIANGLES);
    
		// Triangle 1 
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f); 
		glTexCoord2f(1.0f, 0.0f);
 
		glVertex3f( 1.0f, -1.0f, 0.0f); 
		glTexCoord2f(0.0f, 1.0f);
 
		glVertex3f(-1.0f,  1.0f, 0.0f);  
    
		// Triangle 2 
		glTexCoord2f(0.0f, 1.0f);

		glVertex3f(-1.0f,  1.0f, 0.0f); 
		glTexCoord2f(1.0f, 0.0f);
 
		glVertex3f( 1.0f, -1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f);
  
		glVertex3f( 1.0f,  1.0f, 0.0f);  
    
		glEnd();

		
   		////// End of rendering of the floor //////
        
        
		////start of painting_rect1/////
    
		glUseProgram(programID);

		glUniform1i(justgreenloc, 1);

		glm::mat4 ModelMatrixRect1 = glm::mat4(1.0);
		//ModelMatrixRect1 = glm::scale(ModelMatrixRect1, glm::vec3(3.0f, 3.0f, 0.0f)); // Scale the rectangle
		ModelMatrixRect1 = glm::translate(ModelMatrixRect1, glm::vec3(-5.0f,6.2f,2.5f)); 
		ModelMatrixRect1 = glm::rotate(ModelMatrixRect1, glm::radians(90.0f),glm::vec3(1.0f,0.0f,0.0f));
		glm::mat4 MVPRect1 = ProjectionMatrix * ViewMatrix * ModelMatrixRect1;

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPRect1[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrixRect1[0][0]);  


		double minX = -1.0f;
		double maxX = 1.0f;
		double minY = -1.0f;
		double maxY = 1.0f;
		// Draw the floor with texture
		glBegin(GL_TRIANGLES);
		// Triangle 1
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(minX, minY, 0.0f);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(maxX, minY, 0.0f);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(minX, maxY, 0.0f);

		// Triangle 2
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(minX, maxY, 0.0f);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(maxX, minY, 0.0f);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(maxX, maxY, 0.0f);
		glEnd();

		///end of painting_rect_1//////


		////start of painting_rect2/////

		glUseProgram(programID);

		glUniform1i(justgreenloc, 1);
		glm::mat4 ModelMatrixRect2 = glm::mat4(1.0);
		//ModelMatrixRect2 = glm::scale(ModelMatrixRect2, glm::vec3(3.0f, 3.0f, 0.0f)); // Scale the rectangle
		ModelMatrixRect2 = glm::translate(ModelMatrixRect2, glm::vec3(5.0f,6.2f,2.5f)); 
		ModelMatrixRect2 = glm::rotate(ModelMatrixRect2, glm::radians(90.0f),glm::vec3(1.0f,0.0f,0.0f));
		glm::mat4 MVPRect2 = ProjectionMatrix * ViewMatrix * ModelMatrixRect2;

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPRect2[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrixRect2[0][0]);   

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glUniform1i(TextureID, 0);
		

		minX = -1.0f;
		maxX = 1.0f;
		minY = -1.0f;
		maxY = 1.0f;
		glBegin(GL_TRIANGLES);
		// Triangle 1
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(minX, minY, 0.0f);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(maxX, minY, 0.0f);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(minX, maxY, 0.0f);

		// Triangle 2
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(minX, maxY, 0.0f);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(maxX, minY, 0.0f);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(maxX, maxY, 0.0f);
		glEnd();

		///// end of painting_rect_2/////

		glDisableVertexAttribArray(vertexPosition_modelspaceID);
		glDisableVertexAttribArray(vertexUVID);
		glDisableVertexAttribArray(vertexNormal_modelspaceID);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);


	thread1.join();
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

