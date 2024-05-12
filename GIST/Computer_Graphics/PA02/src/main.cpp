#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <tinyobjloader/tiny_obj_loader.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

// Window dimensions
const GLuint WIDTH = 1280, HEIGHT = 720;

bool BackInFront = 0;

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
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    std::string inputfile = "../../assets/bunny.obj";
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str());
    if (!warn.empty()) {
        std::cout << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        exit(1);
    }

    // Loop over shapes

    glViewport(0, 0, WIDTH, HEIGHT);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawBuffer(GL_FRONT);
    int g_bunnyIDC = glGenLists(1);
    glNewList(g_bunnyIDC, GL_COMPILE);
    glBegin(GL_TRIANGLES);

    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                

                // Optional: vertex colors
                tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

                // Define the viewport dimensions
                
                
                glColor3f(red, green, blue);
                glVertex3f(vx, vy, vz);

            }
            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }

    glEnd();
    glEndList();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    int g_bunnyIDNC = glGenLists(1);
    glNewList(g_bunnyIDNC, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];


                // Define the viewport dimensions


                glColor3ub(0, 255, 0);
                glVertex3f(vx, vy, vz);

            }
            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }
    glEnd();
    glEndList();

    /*glDrawBuffer(GL_BACK);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0, 1, 0);
    glVertex3f(vx, vy, vz);
    glEnd();*/
    glm::mat4 matModel1 = glm::identity<glm::mat4>(); //4x4 identity matrix
    glm::mat4 matModel2 = glm::identity<glm::mat4>(); //4x4 identity matrix
    glm::mat4 matView = glm::lookAt(glm::vec3(0, 4, 4),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0));
    glm::mat4 matProj = glm::perspective(glm::radians(60.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);


    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwWaitEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // set projection matrix for this frame
        glMatrixMode(GL_PROJECTION); // set projection matrix
        // use either of following lines to set the value of projection matrix
        glLoadMatrixf(glm::value_ptr(matProj));
        
        glm::mat4 modelView1 = matView * matModel1;
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(glm::value_ptr(modelView1));
        if (!BackInFront)
        {
            glDrawBuffer(GL_FRONT);
            glCallList(g_bunnyIDC);
        }

        glm::mat4 modelView2 = matView * matModel2;
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(glm::value_ptr(modelView2));
        glDrawBuffer(GL_BACK);
        glCallList(g_bunnyIDNC);
        if (BackInFront)
        {
            glDrawBuffer(GL_FRONT);
            glCallList(g_bunnyIDNC);
        }
        glFinish();
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

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT &&
        action == GLFW_PRESS)
    {
        glReadBuffer(GL_BACK);
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        y = HEIGHT - y - 1;

        unsigned char pixel[3];
        glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

        if (pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 0)
            BackInFront = 1;
        else
            BackInFront = 0;
    }
}

