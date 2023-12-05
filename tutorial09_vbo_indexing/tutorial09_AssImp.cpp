/*
  Author: Kevin D Patino Sosa
  Class: ECE6122-A
  Last Date Modified: 10/10/2023
  Description: Lab2
*/

// Include standard headers
#include <iostream> // Include the necessary header for std::cout
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/vboindexer.hpp>

#include "final_project.hpp"

int main(void)
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Tutorial 09 - Loading with AssImp", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
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
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

    // Get a handle for our buffers
    GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
    GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
    GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");

    // Load the textures
    GLuint Texture = loadDDS("uvmap.DDS");
    GLuint TextureWolf = loadBMP_custom("uvtemplate.bmp");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
    GLuint TextureIDWolf = glGetUniformLocation(programID, "myTextureSamplerWolf");

    // Read our .obj file
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    bool res = loadAssImp("suzanne.obj", indices, indexed_vertices, indexed_uvs, indexed_normals);

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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;
    float step = 1.85f;
    float high = 1.5f;
    glm::vec3 boxMin(-8.0f, -8.0, 0);    // Assuming -1.0f as the minimum z value for the box
    glm::vec3 boxMax(8.0f, 8.0f, 16.0f); // Assuming 1.0f as the maximum z value for the box
    float objRadius = 1.0f;
    RenderObject renderObjects[4] = {RenderObject(glm::vec3(0, step, high), objRadius, boxMin, boxMax),
                                     RenderObject(glm::vec3(step, 0, high), objRadius, boxMin, boxMax),
                                     RenderObject(glm::vec3(0, -step, high), objRadius, boxMin, boxMax),
                                     RenderObject(glm::vec3(-step, 0, high), objRadius, boxMin, boxMax)};

    float modelRotations[4] = {0.0f, 90.0f, 180.0f, -90.0f};

    float move_step = 0.05f;

    // Calculate the position and size of the green plane
    float greenPlaneSize = 16.0f;     // Adjust the size as needed
    float greenPlaneYPosition = 0.0f; // Place it at the ground level

    // Define vertices, UVs, and normals for the green plane
    std::vector<glm::vec3> planeVertices = {
        glm::vec3(-0.5f * greenPlaneSize, greenPlaneYPosition, -0.5f * greenPlaneSize),
        glm::vec3(0.5f * greenPlaneSize, greenPlaneYPosition, -0.5f * greenPlaneSize),
        glm::vec3(0.5f * greenPlaneSize, greenPlaneYPosition, 0.5f * greenPlaneSize),

        glm::vec3(-0.5f * greenPlaneSize, greenPlaneYPosition, -0.5f * greenPlaneSize),
        glm::vec3(0.5f * greenPlaneSize, greenPlaneYPosition, 0.5f * greenPlaneSize),
        glm::vec3(-0.5f * greenPlaneSize, greenPlaneYPosition, 0.5f * greenPlaneSize),
    };

    std::vector<glm::vec2> planeUVs = {
        glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f),

        glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f),
    };

    std::vector<glm::vec3> planeNormals = {
        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),

        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
    };

    // Create buffer objects for the green plane
    GLuint planeVertexBuffer;
    glGenBuffers(1, &planeVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, planeVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, planeVertices.size() * sizeof(glm::vec3), &planeVertices[0], GL_STATIC_DRAW);

    GLuint planeUVBuffer;
    glGenBuffers(1, &planeUVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, planeUVBuffer);
    glBufferData(GL_ARRAY_BUFFER, planeUVs.size() * sizeof(glm::vec2), &planeUVs[0], GL_STATIC_DRAW);

    GLuint planeNormalBuffer;
    glGenBuffers(1, &planeNormalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, planeNormalBuffer);
    glBufferData(GL_ARRAY_BUFFER, planeNormals.size() * sizeof(glm::vec3), &planeNormals[0], GL_STATIC_DRAW);

    bool lightEnabled = false;
    bool textureControl = true;
    static double lastToggleTime = 0.0;
    const double toggleDelay = 0.2; // Adjust the delay as needed

    bool init_move_flag = false;
    do
    {
        textureControl = true;
        GLuint lightEnabledID = glGetUniformLocation(programID, "lightEnabled");
        glUniform1i(lightEnabledID, lightEnabled ? 1 : 0);

        GLuint textureControlID = glGetUniformLocation(programID, "textureControl");
        glUniform1i(textureControlID, textureControl ? 1 : 0);
        // Measure speed
        double currentTime = glfwGetTime();
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && (currentTime - lastToggleTime) >= toggleDelay)
        {
            // Toggle the lighting state when the 'L' key is pressed after a delay
            lightEnabled = !lightEnabled;
            lastToggleTime = currentTime; // Update the last toggle time
        }
        nbFrames++;
        if (currentTime - lastTime >= 1.0)
        { // If last prinf() was more than 1sec ago
            // printf and reset
            printf("%f ms/frame\n", 1000.0 / double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glDisable(GL_CULL_FACE);

        glUseProgram(programID);
        computeMatricesFromInputs();

        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        {
            init_move_flag = true;
            std::cout << "[INFO] Movement Activated" << std::endl;
        }
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();

        textureControl = false;
        glUniform1i(textureControlID, textureControl ? 1 : 0);
        glm::mat4 planeModelMatrix = glm::mat4(1.0);
        glm::mat4 planeMVP = ProjectionMatrix * ViewMatrix * planeModelMatrix;

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &planeMVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &planeModelMatrix[0][0]);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureWolf);
        glUniform1i(TextureIDWolf, 1);

        // Set the green color (you may want to create a new shader for the green plane)
        glUniform3f(LightID, 0.0f, 1.0f, 0.0f); // Set the light color to green

        glEnableVertexAttribArray(vertexPosition_modelspaceID);
        glEnableVertexAttribArray(vertexUVID);
        glEnableVertexAttribArray(vertexNormal_modelspaceID);

        glBindBuffer(GL_ARRAY_BUFFER, planeVertexBuffer);
        glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glBindBuffer(GL_ARRAY_BUFFER, planeUVBuffer);
        glVertexAttribPointer(vertexUVID, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glBindBuffer(GL_ARRAY_BUFFER, planeNormalBuffer);
        glVertexAttribPointer(vertexNormal_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void *)0);

        glEnable(GL_CULL_FACE);

        textureControl = true;
        glUniform1i(textureControlID, textureControl ? 1 : 0);

#pragma omp parallel for
        for (int i = 0; i < 4; i++)
        {

            if (init_move_flag)
            {
                renderObjects[i].moveRandomly(move_step);
            }
        }

        std::vector<CollisionInfo> collisions;

#pragma omp parallel for
        for (int i = 0; i < 4; i++)
        {
            // Check for collisions with other objects
            for (int j = 0; j < 4; j++)
            {
                if (i != j && renderObjects[i].isCollidingWith(renderObjects[j]))
                {
#pragma omp critical
                    {
                        std::cout << "Collision between objects " << i << " and " << j << std::endl;
                        collisions.push_back({i, j});
                    }
                }
            }
        }
        // Mark objects to avoid handling the same collision twice
        std::vector<bool> handled(4, false);

        // Handle collisions outside the parallel region
        for (const auto &collision : collisions)
        {
            if (!handled[collision.object1Index] && !handled[collision.object2Index])
            {
                std::cout << "Collision between objects " << collision.object1Index << " and " << collision.object2Index
                          << std::endl;
                renderObjects[collision.object1Index].handleCollision(renderObjects[collision.object2Index]);
                renderObjects[collision.object2Index].handleCollision(renderObjects[collision.object1Index]);

                handled[collision.object1Index] = true;
                handled[collision.object2Index] = true;
            }
        }

        for (int i = 0; i < 4; i++)
        {
            // Compute the MVP matrix from keyboard and mouse input
            glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0), renderObjects[i].getFixPosition());
            ModelMatrix = glm::rotate(ModelMatrix, glm::radians(modelRotations[i]), glm::vec3(0.0, 1.0, 0.0));
            glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
            glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

            glm::vec3 lightPos = glm::vec3(4, 4, 4);
            glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

            // Bind our texture in Texture Unit 0
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture);
            // Set our "myTextureSampler" sampler to user Texture Unit 0
            glUniform1i(TextureID, 0);

            // 1rst attribute buffer : vertices
            glEnableVertexAttribArray(vertexPosition_modelspaceID);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glVertexAttribPointer(vertexPosition_modelspaceID, // attribute
                                  3,                           // size
                                  GL_FLOAT,                    // type
                                  GL_FALSE,                    // normalized?
                                  0,                           // stride
                                  (void *)0                    // array buffer offset
            );

            // 2nd attribute buffer : UVs
            glEnableVertexAttribArray(vertexUVID);
            glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
            glVertexAttribPointer(vertexUVID, // attribute
                                  2,          // size
                                  GL_FLOAT,   // type
                                  GL_FALSE,   // normalized?
                                  0,          // stride
                                  (void *)0   // array buffer offset
            );

            // 3rd attribute buffer : normals
            glEnableVertexAttribArray(vertexNormal_modelspaceID);
            glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
            glVertexAttribPointer(vertexNormal_modelspaceID, // attribute
                                  3,                         // size
                                  GL_FLOAT,                  // type
                                  GL_FALSE,                  // normalized?
                                  0,                         // stride
                                  (void *)0                  // array buffer offset
            );

            // Index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

            // Draw the triangles !
            glDrawElements(GL_TRIANGLES,      // mode
                           indices.size(),    // count
                           GL_UNSIGNED_SHORT, // type
                           (void *)0          // element array buffer offset
            );

            glDisableVertexAttribArray(vertexPosition_modelspaceID);
            glDisableVertexAttribArray(vertexUVID);
            glDisableVertexAttribArray(vertexNormal_modelspaceID);
        }
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
