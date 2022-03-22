#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() ) {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint( GLFW_SAMPLES, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE ); // To make MacOS happy; should not be needed
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE ); //We don't want the old OpenGL

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 512, "Triangles", NULL, NULL );
    if( window == NULL ) {
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent( window );

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if( glewInit() != GLEW_OK ) {
        fprintf( stderr, "Failed to initialize GLEW\n" );
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );

    // Dark blue background
    glClearColor( 0.0f, 0.0f, 0.4f, 0.0f );

    // Enable blending
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    GLuint VertexArrayID;
    glGenVertexArrays( 1, &VertexArrayID );
    glBindVertexArray( VertexArrayID );

    // Create and compile our GLSL program from the shaders
    GLuint programIDs[] = {
            LoadShaders( "Transform.vertexshader", "FragmentShader1.fragmentshader" ),
            LoadShaders( "Transform.vertexshader", "FragmentShader2.fragmentshader" ),
    };

    // Get a handle for our "MVP" uniform
    GLint MatrixIDs[] = {
            glGetUniformLocation( programIDs[0], "MVP" ),
            glGetUniformLocation( programIDs[1], "MVP" ),
    };

    glm::mat4 Projection = glm::perspective( 45.05f, 2.0f / 1.0f, 0.1f, 100.0f );
    glm::mat4 Model = glm::mat4( 1.0f );

    static const GLfloat g_vertex_buffer_data[] = {
            1.5f,  0.4f, 0.0f,
            -0.3f,  0.1f, 0.0f,
            0.3f, -0.7f, 0.0f,
            1.7f, -0.6f, 0.0f,
            0.3f,  -0.5f, 0.0f,
            0.9f, 0.7f, 0.0f,
    };

    GLuint vertexbuffer;
    glGenBuffers( 1, &vertexbuffer );
    glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof( g_vertex_buffer_data ), g_vertex_buffer_data, GL_STATIC_DRAW );

    double step = 0.05;
    double angle = 10;
    do {
        Sleep( 20 );
        // Clear the screen
        glClear( GL_COLOR_BUFFER_BIT );

        // Camera matrix
        glm::mat4 View = glm::lookAt(
                glm::vec3( 2 * sin( angle ), 1, 3 * cos( angle ) ), // Camera is rotating, in World Space
                glm::vec3( 0.5 * cos(angle), 0, sin(angle) ),
                glm::vec3( 0, 1, 0 )  // Head is up (set to 0,-1,0 to look upside-down)
        );

        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glVertexAttribPointer(
                0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*) 0            // array buffer offset
        );

        for (int i = 0; i < 2; i++) {
            glUseProgram( programIDs[i] );
            glUniformMatrix4fv( MatrixIDs[i], 1, GL_FALSE, &MVP[0][0] );
            glDrawArrays( GL_TRIANGLES, i * 3, 3 );
        }

        glDisableVertexAttribArray( 0 );

        glfwSwapBuffers( window );
        glfwPollEvents();

        angle += step;
    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose( window ) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers( 1, &vertexbuffer );
    glDeleteProgram( programIDs[0] );
    glDeleteProgram( programIDs[1] );
    glDeleteVertexArrays( 1, &VertexArrayID );

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}