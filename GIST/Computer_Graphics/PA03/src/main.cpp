#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <tinyobjloader/tiny_obj_loader.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>



// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Window dimensions
const GLuint WIDTH = 1280, HEIGHT = 720;

//dragging
bool isDragging = false;
glm::vec2 lastMousePos;
glm::vec3 prevFrame;
glm::mat4 matView = glm::lookAt(glm::vec3(0, 0, 5),
    glm::vec3(0, 0, 0),
    glm::vec3(0, 1, 0));

//dolly
bool dollyIN = false;
bool dollyOUT = false;

//zoom
float fov = 60.0f;
float aspect = WIDTH / HEIGHT;
bool zoomIN = false;
bool zoomOUT = false;

glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest)
{
    start = glm::normalize(start);
    dest = glm::normalize(dest);
    float cosTheta = dot(start, dest);
    glm::vec3 rotationAxis;
    if (cosTheta < -1 + 0.001f) {
        // special case when vectors in opposite directions:
        // there is no "ideal" rotation axis
        // So guess one; any will do as long as it's perpendicular to start
        rotationAxis = cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
        if (glm::length2(rotationAxis) <
            0.01) // bad luck, they were parallel, try again!
            rotationAxis = cross(glm::vec3(1.0f, 0.0f, 0.0f), start);
        rotationAxis = normalize(rotationAxis);
        return glm::angleAxis(glm::radians(180.0f), rotationAxis);
    }

    rotationAxis = cross(start, dest);
    float s = sqrt((1 + cosTheta) * 2);
    float invs = 1 / s;

    return glm::quat(s * 0.5f, rotationAxis.x * invs, rotationAxis.y * invs, rotationAxis.z * invs);
}

glm::vec3 projectCursor(float x, float y)
{
    // Convert screen coordinates to NDC
    float nx = (2.0f * x) / WIDTH - 1.0f;
    float ny = 1.0f - (2.0f * y) / HEIGHT;

    // Compute z coordinate on the sphere
    float r_squared = nx * nx + ny * ny;
    float z = 0.0f;
    if (r_squared <= 1.0f) {
        z = sqrt(1.0f - r_squared);
    }
    else {
        z = 1.0f / sqrt(r_squared);
    }

    return glm::vec3(nx, ny, z);
}

// Draw axes function
void drawAxes() 
{
    GLfloat sx[3] = { -100.0f, 0.0f, 0.0f };
    GLfloat ex[3] = { 100.0f, 0.0f, 0.0f };
    GLfloat sz[3] = { 0.0f, 0.0f, -100.0f };
    GLfloat ez[3] = { 0.0f, 0.0f, 100.0f };
    // Draw X axis (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3fv(sx);
    glVertex3fv(ex);
    glEnd();

    // Draw Z axis (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3fv(sz);
    glVertex3fv(ez);
    glEnd();
}

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
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    glEnable(GL_LIGHTING);
    float pointLight[] = { 0.0, 10.0, 0.0, 1.0 };
    float La[] = { 0.1, 0.1, 0.1, 1.0 };
    float Ld[] = { 1.0, 1.0, 1.0, 1.0 };
    float Ls[] = { 1.0, 1.0, 1.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, pointLight);
    glLightfv(GL_LIGHT0, GL_AMBIENT, La);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, Ld);
    glLightfv(GL_LIGHT0, GL_SPECULAR, Ls);
    glEnable(GL_LIGHT0);

    float directionalLight[] = { 0.0, -10.0, 0.0, 0.0 };
    glLightfv(GL_LIGHT1, GL_POSITION, directionalLight);
    glLightfv(GL_LIGHT1, GL_AMBIENT, La);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, Ld);
    glLightfv(GL_LIGHT1, GL_SPECULAR, Ls);
    glEnable(GL_LIGHT1);
    glEnable(GL_DEPTH_TEST);



    // Define the viewport dimensions
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
    int g_bunnyIDC = glGenLists(1);
    glNewList(g_bunnyIDC, GL_COMPILE);

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            if (shapes[s].mesh.material_ids[f] < materials.size())
            {
                // per-face material
                tinyobj::material_t material = materials[shapes[s].mesh.material_ids[f]];

                glMaterialfv(GL_FRONT, GL_AMBIENT, material.ambient);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, material.diffuse);
                glMaterialfv(GL_FRONT, GL_SPECULAR, material.specular);
                glMaterialfv(GL_FRONT, GL_SHININESS, &material.shininess);
            }


            // Loop over vertices in the face.
            glBegin(GL_TRIANGLES);
            for (size_t v = 0; v < fv; v++)
            {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0)
                {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    glNormal3f(nx, ny, nz);
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0)
                {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    glTexCoord2f(tx, ty);
                }
                // Optional: vertex colors
                // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
                glVertex3f(vx, vy, vz);
            }
            glEnd();
            index_offset += fv;


        }
    }

    glEndList();

    glm::mat4 matModel = glm::identity<glm::mat4>(); //4x4 identity matrix

    glm::mat4 matProj = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        glm::vec3 cameraDirection = glm::vec3(matView[0][2], matView[1][2], matView[2][2]);
        if (dollyIN)
        {
            matView = glm::translate(matView, 0.5f * cameraDirection);
            //std::cout << "approching" << std::endl;
        }
        if (dollyOUT)
        {
            matView = glm::translate(matView, -0.5f * cameraDirection);
            //std::cout << "get out" << std::endl;
        }

        if (zoomIN)
        {
            if (fov > 35.0f)
            {
                fov--;
                matProj = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
            }
        }
        if (zoomOUT)
        {
            if (fov < 130.0f)
            {
                fov++;
                matProj = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
            }
        }
        // Render
        // Clear the colorbuffer
        glEnable(GL_LIGHTING);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // set projection matrix for this frame
        
        glMatrixMode(GL_PROJECTION); // set projection matrix
        glLoadMatrixf(glm::value_ptr(matProj));

        glm::mat4 modelView = matView * matModel;
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(glm::value_ptr(modelView));

        glCallList(g_bunnyIDC);

        dollyIN = dollyOUT = false;

        glDisable(GL_LIGHTING);
        drawAxes();
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
    //std::cout << "lights" << std::endl;
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_1)
            glEnable(GL_LIGHT0);
        if (key == GLFW_KEY_2)
            glDisable(GL_LIGHT0);
        if (key == GLFW_KEY_3)
            glEnable(GL_LIGHT1);
        if (key == GLFW_KEY_4)
            glDisable(GL_LIGHT1);

        if (key == GLFW_KEY_Q)
            zoomIN = true;
        if (key == GLFW_KEY_W)
            zoomOUT = true;

      //  std::cout << "endlights" << std::endl;
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_Q)
            zoomIN = false;
        if (key == GLFW_KEY_W)
            zoomOUT = false;
    }
}

// Mouse callback function
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (isDragging) 
    {
        // Calculate rotation based on mouse movement
        // Implement your trackball rotation calculation here
        //glm::vec3 rotation = 
        glm::vec3 v1 = projectCursor(lastMousePos.x, lastMousePos.y);
        glm::vec3 v2 = projectCursor(xpos, ypos);
        matView *= glm::toMat4(RotationBetweenVectors(v1, v2));
        //std::cout << "working " << v1.x << " " << v2.x << std::endl;
        
    }
    lastMousePos = glm::vec2(xpos, ypos);
}

// Mouse button callback function
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    //std::cout << "begin" << std::endl;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        isDragging = true;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        lastMousePos = glm::vec2(xpos, ypos);
      //  std::cout << "begin" << std::endl;
    }

    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
       // std::cout << "end" << std::endl;
        isDragging = false;
    }

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset > 0)
        dollyIN = true;
    else
        dollyOUT = true;
}
