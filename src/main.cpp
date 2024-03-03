#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadTexture(const char *path);

unsigned int loadCubemap(vector<std::string> faces);

void renderQuad();

void renderCube(Shader shader, glm::vec3 center, float a);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float heightScale = 0.1f;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 backpackPosition = glm::vec3(0.0f);
    float backpackScale = 1.0f;
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 10.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

const float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
        0.5f, -0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,-1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,-1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,-1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

        0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
        0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f
};

float skyboxVertices[] = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
};

float cubes[][3] = {
        {-18.0f, -5.2f, 0.0f},
        {-18.0f, -7.2f, 0.0f},
        {-18.0f, -9.2f, 0.0f},
        {-18.0f, -11.2f, 0.0f},
        {-18.0f, -3.2f, 0.0f},
        {-18.0f, -1.2f, 0.0f},
        {-18.0f, 0.8f, 0.0f},
        {-18.0f, 2.8f, 0.0f},
        {-18.0f, 4.8f, 0.0f},
        {-18.0f, 6.8f, 0.0f},
        {-18.0f, 8.8f, 0.0f},
        {-18.0f, 10.8f, 0.0f},
        {-16.0f, -5.2f, 0.0f},
        {-16.0f, -7.2f, 0.0f},
        {-16.0f, -9.2f, 0.0f},
        {-16.0f, -11.2f, 0.0f},
        {-14.0f, -5.2f, 0.0f},
        {-14.0f, -7.2f, 0.0f},
        {-14.0f, -9.2f, 0.0f},
        {-14.0f, -11.2f, 0.0f},
        {-12.0f, -5.2f, 0.0f},
        {-12.0f, -7.2f, 0.0f},
        {-12.0f, -9.2f, 0.0f},
        {-12.0f, -11.2f, 0.0f},
        {-10.0f, -5.2f, 0.0f},
        {-10.0f, -7.2f, 0.0f},
        {-10.0f, -9.2f, 0.0f},
        {-10.0f, -11.2f, 0.0f},
        {-8.0f, -5.2f, 0.0f},
        {-8.0f, -7.2f, 0.0f},
        {-8.0f, -9.2f, 0.0f},
        {-8.0f, -11.2f, 0.0f},
        {-6.0f, -5.2f, 0.0f},
        {-6.0f, -7.2f, 0.0f},
        {-6.0f, -9.2f, 0.0f},
        {-6.0f, -11.2f, 0.0f},
        {-4.0f, -5.2f, 0.0f},
        {-4.0f, -7.2f, 0.0f},
        {-4.0f, -9.2f, 0.0f},
        {-4.0f, -11.2f, 0.0f},
        {-2.0f, -5.2f, 0.0f},
        {-2.0f, -7.2f, 0.0f},
        {-2.0f, -9.2f, 0.0f},
        {-2.0f, -11.2f, 0.0f},
        {0.0f, -5.2f, 0.0f},
        {0.0f, -7.2f, 0.0f},
        {0.0f, -9.2f, 0.0f},
        {0.0f, -11.2f, 0.0f},
        {2.0f, -5.2f, 0.0f},
        {2.0f, -7.2f, 0.0f},
        {2.0f, -9.2f, 0.0f},
        {2.0f, -11.2f, 0.0f},
        {4.0f, -5.2f, 0.0f},
        {4.0f, -7.2f, 0.0f},
        {4.0f, -9.2f, 0.0f},
        {4.0f, -11.2f, 0.0f},
        {6.0f, -5.2f, 0.0f},
        {6.0f, -7.2f, 0.0f},
        {6.0f, -9.2f, 0.0f},
        {6.0f, -11.2f, 0.0f},
        {14.0f, -5.2f, 0.0f},
        {16.0f, -5.2f, 0.0f},
        {18.0f, -5.2f, 0.0f},
        {20.0f, -5.2f, 0.0f},
        {22.0f, -5.2f, 0.0f},
        {24.0f, -5.2f, 0.0f},
        {26.0f, -5.2f, 0.0f},
        {28.0f, -5.2f, 0.0f},
        {30.0f, -5.2f, 0.0f},
        {32.0f, -5.2f, 0.0f},
        {34.0f, -5.2f, 0.0f},
        {36.0f, -5.2f, 0.0f},
        {42.0f, -5.2f, 0.0f},
        {44.0f, -5.2f, 0.0f},
        {46.0f, -5.2f, 0.0f},
        {48.0f, -5.2f, 0.0f},
        {50.0f, -5.2f, 0.0f},
        {52.0f, -5.2f, 0.0f},
        {54.0f, -5.2f, 0.0f},
        {56.0f, -5.2f, 0.0f},
        {58.0f, -5.2f, 0.0f},
        {60.0f, -5.2f, 0.0f},
        {62.0f, -5.2f, 0.0f},
        {64.0f, -5.2f, 0.0f},
        {68.0f, -5.2f, 0.0f},
        {68.0f, -7.2f, 0.0f},
        {68.0f, -9.2f, 0.0f},
        {68.0f, -11.2f, 0.0f},
        {68.0f, -3.2f, 0.0f},
        {68.0f, -1.2f, 0.0f},
        {68.0f, 0.8f, 0.0f},
        {68.0f, 2.8f, 0.0f},
        {68.0f, 4.8f, 0.0f},
        {68.0f, 6.8f, 0.0f},
        {68.0f, 8.8f, 0.0f},
        {68.0f, 10.8f, 0.0f},
        {68.0f, 12.8f, 0.0f},
        {20.0f, 0.8f, 0.0f},
        {22.0f, 0.8f, 0.0f},
        {26.0f, 0.8f, 0.0f},
        {28.0f, 0.8f, 0.0f},
        {50.0f, 0.8f, 0.0f},
        {52.0f, 0.8f, 0.0f},
        {56.0f, 0.8f, 0.0f},
        {58.0f, 0.8f, 0.0f},
};

float mysteryCubes[][3] = {
        {24.0f,0.8f,0.0f},
        {54.0f,0.8f,0.0f},
};

float cubeSize = 2.0f;

float coins[][3] = {
        {22.0f,2.8f,0.0f},
        {26.0f,2.8f,0.0f},

        {52.0f,2.8f,0.0f},
        {56.0f,2.8f,0.0f},
};

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // build and compile shaders
    // -------------------------
    Shader materialShader("resources/shaders/materialVertexShader.vs","resources/shaders/materialFragmentShader.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");

    // load models
    // -----------
    Model coinModel("resources/objects/mario_coin/Mario_Coin.obj");
    coinModel.SetShaderTextureNamePrefix("material.");

    //create skybox
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    //load textures
    // --------------
    unsigned int cubeDiffuse = loadTexture(FileSystem::getPath("resources/textures/bricks.png").c_str());
    unsigned int cubeSpecular = loadTexture(FileSystem::getPath("resources/textures/bricksSpecular.png").c_str());
    unsigned int cubeNormal = loadTexture(FileSystem::getPath("resources/textures/bricksNormal.png").c_str());
    unsigned int cubeDisp = loadTexture(FileSystem::getPath("resources/textures/bricksDisplacment.png").c_str());
    unsigned int mysteryDiffuse = loadTexture(FileSystem::getPath("resources/textures/mystery.png").c_str());
    unsigned int mysterySpecular = loadTexture(FileSystem::getPath("resources/textures/mystery_specular.png").c_str());
    unsigned int mysteryNormal = loadTexture(FileSystem::getPath("resources/textures/mystery_normal.png").c_str());
    unsigned int mysteryDisp= loadTexture(FileSystem::getPath("resources/textures/mystery_displacment.png").c_str());

    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/skybox/front5.jpg"),
                    FileSystem::getPath("resources/textures/skybox/front5.jpg"),
                    FileSystem::getPath("resources/textures/skybox/top5.jpg"),
                    FileSystem::getPath("resources/textures/skybox/bottom6.jpg"),
                    FileSystem::getPath("resources/textures/skybox/front5.jpg"),
                    FileSystem::getPath("resources/textures/skybox/front5.jpg")
            };
    unsigned int cubemapTexture = loadCubemap(faces);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(0.2, 0.2, 0.2);
    pointLight.diffuse = glm::vec3(0.8, 0.8, 0.8);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;



    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
   while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //NEW CODE
        materialShader.use();
        //pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));
        materialShader.setVec3("pointLight.position", pointLight.position);
        materialShader.setVec3("pointLight.ambient", pointLight.ambient);
        materialShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        materialShader.setVec3("pointLight.specular", pointLight.specular);
        materialShader.setFloat("pointLight.constant", pointLight.constant);
        materialShader.setFloat("pointLight.linear", pointLight.linear);
        materialShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        materialShader.setVec3("viewPos", programState->camera.Position);
        materialShader.setFloat("material.shininess", 32.0f);
        materialShader.setBool("blinn",true);
        materialShader.setFloat("heightScale",heightScale);

        materialShader.setVec3("spotLight.position", glm::vec3(5.0f));
        materialShader.setVec3("spotLight.direction", glm::vec3(-5.0f));
        materialShader.setVec3("spotLight.ambient", glm::vec3(0.1f,0.1f,0.1f));
        materialShader.setVec3("spotLight.diffuse", glm::vec3(0.3f,0.3f,0.3f));
        materialShader.setVec3("spotLight.specular", glm::vec3(0.2f,0.2f,0.2f));
        materialShader.setFloat("spotLight.constant", pointLight.constant);
        materialShader.setFloat("spotLight.linear", pointLight.linear);
        materialShader.setFloat("spotLight.quadratic", pointLight.quadratic);
        materialShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(40.0f)));
        materialShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(45.0f)));

        materialShader.setVec3("dirLight.direction", glm::vec3(0.0f,-1.0f,0.0f));
        materialShader.setVec3("dirLight.ambient", glm::vec3(0.1f,0.1f,0.1f));
        materialShader.setVec3("dirLight.diffuse", glm::vec3(0.5f,0.3f,0.3f));
        materialShader.setVec3("dirLight.specular", glm::vec3(0.2f,0.2f,0.2f));

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                 (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        materialShader.setMat4("projection", projection);
        materialShader.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.20f));

        materialShader.setMat4("model", model);
        materialShader.setInt("material.texture_diffuse", 0);
        materialShader.setInt("material.texture_specular", 1);
        materialShader.setInt("material.texture_normal", 2);
        materialShader.setInt("material.texture_depth", 3);
        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubeDiffuse);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, cubeSpecular);
        // bind normal map
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, cubeNormal);
        // bind displacment map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, cubeDisp);

        for(auto cube:cubes){
            renderCube(materialShader,glm::vec3(cube[0],cube[1],cube[2]),cubeSize);
        }

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mysteryDiffuse);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mysterySpecular);
        // bind normal map
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mysteryNormal);
        // bind displacment map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, mysteryDisp);

        for(auto cube:mysteryCubes){
            renderCube(materialShader,glm::vec3(cube[0],cube[1],cube[2]),cubeSize);
        }

        /**/glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);

        int i=0;
        for(auto coin:coins){
            model = glm::mat4(1.0f);
            model = glm::translate(model,glm::vec3(coin[0],coin[1],coin[2]));
            model = glm::translate(model, glm::vec3(0, (float)glm::cos(glfwGetTime()) / 3.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.1f));
            model = glm::rotate(model, 5.0f * (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
            materialShader.setMat4("model", model);
            coinModel.Draw(materialShader);
            i++;
        }



        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
        ImGui::DragFloat3("Backpack position", (float*)&programState->backpackPosition);
        ImGui::DragFloat("Backpack scale", &programState->backpackScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// renders a 1x1 quad in NDC with manually calculated tangent vectors
// ------------------------------------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        // positions
        glm::vec3 pos1(-1.0f,  1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3( 1.0f, -1.0f, 0.0f);
        glm::vec3 pos4( 1.0f,  1.0f, 0.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


        float quadVertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void renderCube(Shader shader, glm::vec3 center, float a){
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, center);
    model = glm::translate(model,glm::vec3(0,0,a/2));
    model = glm::scale(model,glm::vec3(a/2));
    shader.setMat4("model", model);
    renderQuad();

    model = glm::scale(model,glm::vec3(2/a));
    model = glm::translate(model,glm::vec3(0,0,-a/2));
    model = glm::translate(model,glm::vec3(0,0,-a/2));
    model = glm::rotate(model,glm::radians(180.0f),glm::vec3(0,1.0f,0));
    model = glm::scale(model,glm::vec3(a/2));
    shader.setMat4("model", model);
    renderQuad();

    model = glm::scale(model,glm::vec3(2/a));
    model = glm::rotate(model,glm::radians(180.0f),glm::vec3(0,1.0f,0));
    model = glm::translate(model,glm::vec3(0,0,a/2));
    model = glm::translate(model,glm::vec3(a/2,0,0));
    model = glm::rotate(model,glm::radians(90.0f),glm::vec3(0,1.0f,0));
    model = glm::scale(model,glm::vec3(a/2));

    shader.setMat4("model", model);
    renderQuad();

    model = glm::scale(model,glm::vec3(2/a));
    model = glm::rotate(model,glm::radians(-90.0f),glm::vec3(0,1.0f,0));
    model = glm::translate(model,glm::vec3(-a/2,0,0));
    model = glm::translate(model,glm::vec3(-a/2,0,0));
    model = glm::rotate(model,glm::radians(-90.0f),glm::vec3(0,1.0f,0));
    model = glm::scale(model,glm::vec3(a/2));
    shader.setMat4("model", model);
    renderQuad();

    model = glm::scale(model,glm::vec3(2/a));
    model = glm::rotate(model,glm::radians(90.0f),glm::vec3(0,1.0f,0));
    model = glm::translate(model,glm::vec3(a/2,0,0));
    model = glm::translate(model,glm::vec3(0,a/2,0));
    model = glm::rotate(model,glm::radians(-90.0f),glm::vec3(1.0f,0.0f,0));
    model = glm::scale(model,glm::vec3(a/2));
    shader.setMat4("model", model);
    renderQuad();

    model = glm::scale(model,glm::vec3(2/a));
    model = glm::rotate(model,glm::radians(90.0f),glm::vec3(1.0f,0.0f,0));
    model = glm::translate(model,glm::vec3(0,-a/2,0));
    model = glm::translate(model,glm::vec3(0,-a/2,0));
    model = glm::rotate(model,glm::radians(90.0f),glm::vec3(1.0f,0.0f,0));
    model = glm::scale(model,glm::vec3(a/2));
    shader.setMat4("model", model);
    renderQuad();

}