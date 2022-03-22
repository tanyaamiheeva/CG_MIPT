#include <stdio.h>
#include <windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

GLFWwindow *window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include <common/shader.hpp>

int main(void) {
    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Tutorial 04 - Colored Cube", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr,
                "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    glm::mat4 Model = glm::mat4(1.0f);

    static const GLfloat g_vertex_buffer_data[] = {
            0, 1.0f, 0,
            -0.866f, -0.5f, 0,
            0, 0, 2.0f,

            -0.866f, -0.5f, 0,
            0.866f, -0.5f, 0,
            0, 0, 2.0f,

            0.866f, -0.5f, 0,
            0, 1.0f, 0,
            0, 0, 2.0f,

            0, 1.0f, 0,
            -0.866f, -0.5f, 0,
            0, 0, -2.0f,

            -0.866f, -0.5f, 0,
            0.866f, -0.5f, 0,
            0, 0, -2.0f,

            0.866f, -0.5f, 0,
            0, 1.0f, 0,
            0, 0, -2.0f,
    };

    static const GLfloat g_color_buffer_data[] = {
            0.1f, 0.2f, 0.03f,
            0, 0.5f, 0.06f,
            0.4f, 0.8f, 0.09f,

            0.1f, 0.2f, 0.03f,
            0, 0.5f, 0.06f,
            0.5f, 0.8f, 0.09f,

            0.1f, 0.2f, 0.03f,
            0.1f, 0, 0.06f,
            0.6f, 0.8f, 0.09f,

            0.1f, 0.2f, 0.03f,
            0.9f, 0.5f, 0,
            0.7f, 0.8f, 0.09f,

            0.1f, 0.2f, 0.03f,
            0.1f, 0.5f, 0.06f,
            0.8f, 0, 0.09f,

            0.1f, 0.2f, 0,
            0.4f, 0.5f, 0.06f,
            0.9f, 0.8f, 0.09f,
    };

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    GLuint colorbuffer;
    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

    double step = 0.025;
    double camera_angle = 10;

    do {
        Sleep(15);
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 View = glm::lookAt(
                glm::vec3(2 * cos(camera_angle), 1, -6 * sin(camera_angle)), // Camera is rotating, in World Space
                glm::vec3(0.5 * sin(camera_angle), 0, 0.5 * cos(camera_angle)),
                glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
        );
        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void *) 0            // array buffer offset
        );

        // 2nd attribute buffer : colors
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glVertexAttribPointer(
                1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
                3,                                // size
                GL_FLOAT,                         // type
                GL_FALSE,                         // normalized?
                0,                                // stride
                (void *) 0                          // array buffer offset
        );

        glUseProgram(programID);

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // Draw the triangles
//        printf("Length : %d\n", sizeof(g_vertex_buffer_data));
        glDrawArrays(GL_TRIANGLES, 0, 6 * 3);
//        g_vertex_buffer_data
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        camera_angle += step;
    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

