#include "renderer.cpp"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfwErrCallback(int error, const char *description) {
    std::cerr << "[GLFW Error [" << error << "]]: " << description << std::endl;
}


int main(void) {
//    std::uniform_real_distribution<double> unif(0, 1);
//    std::default_random_engine randEngine;

    glfwSetErrorCallback(glfwErrCallback);

    /* Initialize the library */
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW! Stopping..." << std::endl;
        return 1;
    }

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow *window = glfwCreateWindow(960, 540, "Hello World", NULL, NULL);

    if (!window) {
        std::cerr << "Failed to create GLFW window! (window=" << window << ")" << std::endl;
        std::cerr << "Stopping!" << std::endl;
        glfwTerminate();
        return 1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
//    glfwSwapInterval(1);

    glewExperimental = GL_TRUE; // Needed to use VAOs
    {
        GLenum glewState = glewInit();
        if (glewState != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW! (glewInit() returned " << glewState << ")" << std::endl;
            return 1;
        }
    }


    std::cout << "Successfully initialized OpenGL (with GLFW, GLEW, GLM, IMGUI, and STB) version "
              << glGetString(GL_VERSION) << std::endl;

    /* Loop until the user closes the window */
    float x = 0.5f;
    float y = 0.0f;
    float pos[] = {
            -x, x, x, 0, 0, // 0
            x, x, x, 1, 0, // 1
            x, -x, x, 1, 1, //2
            -x, -x, x, 0, 1, // 3

            -x, x, -x, 0, 0, // 4
            x, x, -x, 1, 0,// 5
            x, -x, -x, 1, 1, // 6
            -x, -x, -x, 0, 1, // 7

            x, -x, x, 0, 1, // 8
            x, x, x, 0, 0, // 9
            x, x, x, 1, 1, // 10
            -x, x, x, 0, 1, // 11
            -x, x, x, 1, 0, // 12
            -x, -x, x, 1, 1, // 13

            -x, -x, x, 0, 0, // 14
            x, -x, x, 1, 0, // 15

    };

    unsigned int index[] = {
            0, 1, 2, 3,
            7, 6, 5, 4,

            9, 5, 6, 8, // (x, x; 0, 0), (x, x; 1, 0), (x, -x; 1, 1), (x, -x; 0, 1)
            4, 5, 10, 11, // (-x, x, -x);(0, 0), (x, x, -x);(1, 0), (x, x, x);(1, 1), (-x, x, x);(0, 1)
            4, 12, 13, 7, // (-x, x, x);(1, 0), (-x, -x, x);(1, 1);
            14, 15, 6, 7 // (-x, -x, x);(0, 0), (x, -x, x);(1, 0)
    };

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_ADD);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    VertexArray va;
    va.bind();

    VBLayout layout;
    layout.addAttribute(3, GL_FLOAT, false); // Positions
    layout.addAttribute(2, GL_FLOAT, false); // Texture coords

    VertexBuffer buf(sizeof(pos), pos, GL_STATIC_DRAW);
    buf.bind();

    buf.setLayout(layout, va);

//    glEnableVertexAttribArray(0);
    // set the attribute that can be accessed at index 0 by the shader
    // to be the vertex position that contains 2 GL_FLOATs,
    // is already normalized so we dont have to normalize it again (GL_FALSE),
    // and the count of the entire vertex (including other attribs) in bytes is (sizeof(float) * 2),
    //  The position attribute begins 0 bytes into each vert.
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (const void *) 0);

    IndexBuffer ibo(24, index, GL_STATIC_DRAW);
    ibo.bind();

    glm::mat4 projectionMat;

    glm::mat4 viewMat;

    glm::mat4 modelMat = glm::mat4(1.0f); // Model is at origin. No need for further transformations.

    glm::mat4 mvp = projectionMat * viewMat * modelMat;

    ShaderProgram sp("./res/shaders/default");
    sp.bind();

    ImVec4 tint = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    sp.setUniform4f("u_Tint", 0, 0, 0, 0);
    sp.setUniform4f("u_Mult", 1, 1, 1, 1);

    sp.setUniform1i("u_Texture", 0);
    sp.setUniformMat4f("u_MVP", mvp);

    stbi_set_flip_vertically_on_load(1); // Loading PNGs requires this or else they're upside-down :(
    Texture tex("./res/textures/tex0.png", GL_TEXTURE_2D, 0, 0);
    tex.setRenderHints({{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
                        {GL_TEXTURE_MAG_FILTER, GL_NEAREST}}); // Don't blur the textures!
    tex.genMipmaps();

    Texture tex2("./res/textures/tex1.png", GL_TEXTURE_2D, 0, 0);
    tex2.setRenderHints({{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
                         {GL_TEXTURE_MAG_FILTER, GL_NEAREST}});
    tex2.genMipmaps();

    tex.bind(0);

    Camera player(glm::vec3(0, 0, 0), glm::vec2(0, 0), window);

    float speed = 3;
    float sensitivity = 3;

    double lastFrame = glfwGetTime();

    float deltaTime;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    bool demo = false;

    float fov = 70;

//    io.Fonts->AddFontDefault();

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        deltaTime = float(now - lastFrame);
        lastFrame = now;

        flushGLErrors();

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            player.move(speed, 0, deltaTime);
        }
// Move backward
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            player.move(-speed, 0, deltaTime);
        }
// Strafe right
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            player.move(0, speed, deltaTime);
        }
// Strafe left
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            player.move(0, -speed, deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            player.look(glm::vec2(0, sensitivity), deltaTime);
        }
// Move backward
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            player.look(glm::vec2(0, -sensitivity), deltaTime);
        }
// Strafe right
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            player.look(glm::vec2(-sensitivity, 0), deltaTime);
        }
// Strafe left
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            player.look(glm::vec2(sensitivity, 0), deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            player.position += glm::vec3(0, deltaTime * speed, 0);
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            player.position -= glm::vec3(0, deltaTime * speed, 0);
        }


        projectionMat = player.getProjection(fov);
        viewMat = player.getView();

        sp.setUniform4f("u_Tint", tint.x, tint.y, tint.z, tint.w);


        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (demo) {
            ImGui::ShowDemoWindow(&demo);
        }

        {
            ImGui::Begin("Hello, world!");
            ImGui::ColorEdit3("Tint", (float *) &tint);
            ImGui::SliderFloat("FOV", &fov, 10, 100);
            ImGui::Checkbox("Show Demo", &demo);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            ImGui::End();
        }


        ImGui::Render();

        glClearColor(0.25f, 0.25f, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_CULL_FACE);
        modelMat = glm::translate(glm::mat4(1.0), player.position);
        modelMat *= glm::scale(glm::mat4(1.0), glm::vec3(50, 50, 50));
        mvp = projectionMat * viewMat * modelMat;
        tex.bind(0);
        sp.setUniformMat4f("u_MVP", mvp);
        ibo.draw(GL_QUADS);

        glEnable(GL_CULL_FACE);
        modelMat = glm::mat4(1.0);
        mvp = projectionMat * viewMat * modelMat;
        tex2.bind(0);
        sp.setUniformMat4f("u_MVP", mvp);
        ibo.draw(GL_QUADS);

        va.unbind();
        buf.unbind();
        ibo.unbind();
        sp.unbind();

        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        sp.bind();
        ibo.bind();
        va.bind();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    sp.destroy();
    va.destroy();
    buf.destroy();
    ibo.destroy();
    tex.destroy();

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    flushGLErrors();
    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "App stopped without errors." << std::endl;
    return 0;
}
