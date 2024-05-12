#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 1280, HEIGHT = 720;

// The MAIN function, from here we start the application and run the game loop
int main()
{
    std::cout << "Starting GLFW context, OpenGL 3.1" << std::endl;
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "glskeleton", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);

    float ver1[3] = { -0.5f, -0.5f, 1 };
    float ver2[3] = { 0.5, -0.5f, 1 };
    float ver3[3] = { 0, 0.5f, 1 };
    int t = 0;
    float rotmat[] =
    {
        cos(t), -sin(t), 0,
        sin(t), cos(t), 0,
        0, 0, 1
    };

    double before = glfwGetTime();
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        double now = glfwGetTime();
        double delta = now - before;
        before = now;
        rotmat[0] = cos(delta);
        rotmat[1] = -sin(delta);
        rotmat[3] = sin(delta);
        rotmat[4] = rotmat[0];
        ver1[0] = rotmat[0] * ver1[0] + rotmat[1] * ver1[1];
        ver1[1] = rotmat[3] * ver1[0] + rotmat[4] * ver1[1];
        ver2[0] = rotmat[0] * ver2[0] + rotmat[1] * ver2[1];
        ver2[1] = rotmat[3] * ver2[0] + rotmat[4] * ver2[1];
        ver3[0] = rotmat[0] * ver3[0] + rotmat[1] * ver3[1];
        ver3[1] = rotmat[3] * ver3[0] + rotmat[4] * ver3[1];

        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        //glLoadIdentity();
        //glRotatef(delta, 0.0f, 0.0f, 1.0f);
        //std::cout << "delta = " << delta << std::endl;
        glBegin(GL_TRIANGLES);
        double variation = cos(now);
        glColor3f(variation, 0.3, 0.3);
        glVertex3f(ver1[0], ver1[1], 0);
        glVertex3f(ver2[0], ver2[1], 0);
        glVertex3f(ver3[0], ver3[1], 0);
        glEnd();


        //glMultMatrixd();

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Terminates GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}
