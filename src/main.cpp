#include "renderer.cpp"

int main(void) {

    Renderer rend;

    if (!rend.init("Hello World!", 960, 540)) {
        return 1;
    }

    glFrontFace(GL_CW);

    std::cout << "Successfully initialized OpenGL (with GLFW, GLEW, GLM, IMGUI, and STB) version "
              << glGetString(GL_VERSION) << std::endl;

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

    unsigned int inwardIndx[] = {
            3, 2, 1, 0,
            4, 5, 6, 7,

            8, 6, 5, 9, // (x, x; 0, 0), (x, x; 1, 0), (x, -x; 1, 1), (x, -x; 0, 1)
            11, 10, 5, 4, // (-x, x, -x);(0, 0), (x, x, -x);(1, 0), (x, x, x);(1, 1), (-x, x, x);(0, 1)
            7, 13, 12, 4, // (-x, x, x);(1, 0), (-x, -x, x);(1, 1);
            7, 6, 15, 14 // (-x, -x, x);(0, 0), (x, -x, x);(1, 0)
    };

    VertexArray va;
    va.bind();

    VBLayout layout;
    layout.addAttribute(3, GL_FLOAT, false); // Positions
    layout.addAttribute(2, GL_FLOAT, false); // Texture coords

    VertexBuffer buf(sizeof(pos), pos, GL_STATIC_DRAW);
    buf.bind();

    buf.setLayout(layout, va);

    IndexBuffer ibo(24, index, GL_STATIC_DRAW);
    ibo.bind();

    IndexBuffer inIBO(24, inwardIndx, GL_STATIC_DRAW);

    glm::vec3 skyboxScale = glm::vec3(12, 12, 12);

    ShaderProgram sp("./res/shaders/default");
    sp.bind();

    ImVec4 tint = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    sp.setUniform4f("u_Tint", 0, 0, 0, 0);
    sp.setUniform4f("u_Mult", 1, 1, 1, 1);

    sp.setUniform1i("u_Texture", 0);
    sp.setUniformMat4f("u_MVP", IDENTITY_MAT4);

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

    Camera player(glm::vec3(0, 0, 0), glm::vec2(0, 0), rend.window);

    Model cube = {&ibo, &va, GL_QUADS, &buf};
    Model inCube = {&inIBO, &va, GL_QUADS, &buf};

    GameObject purpur = {&cube, &sp, &tex2};
    GameObject skybox = {&inCube, &sp, &tex};

    rend.addGameObject("skybox", &skybox);
    rend.addGameObject("purpur", &purpur);

    float speed = 3;
    float sensitivity = 3;

    double lastFrame = glfwGetTime();

    float deltaTime;

    bool demo = false;

    float fov = 70;

    while (!glfwWindowShouldClose(rend.window)) {
        double now = glfwGetTime();
        deltaTime = float(now - lastFrame);
        lastFrame = now;

        if (glfwGetKey(rend.window, GLFW_KEY_W) == GLFW_PRESS) {
            player.move(speed, 0, deltaTime);
            skybox.transforms = glm::translate(IDENTITY_MAT4, {0, 0, 0});
            skybox.transforms *= glm::scale(IDENTITY_MAT4, skyboxScale);
        }
// Move backward
        if (glfwGetKey(rend.window, GLFW_KEY_S) == GLFW_PRESS) {
            player.move(-speed, 0, deltaTime);
            skybox.transforms = glm::translate(IDENTITY_MAT4, {0, 0, 0});
            skybox.transforms *= glm::scale(IDENTITY_MAT4, skyboxScale);
        }
// Strafe right
        if (glfwGetKey(rend.window, GLFW_KEY_D) == GLFW_PRESS) {
            player.move(0, speed, deltaTime);
            skybox.transforms = glm::translate(IDENTITY_MAT4, {0, 0, 0});
            skybox.transforms *= glm::scale(IDENTITY_MAT4, skyboxScale);
        }
// Strafe left
        if (glfwGetKey(rend.window, GLFW_KEY_A) == GLFW_PRESS) {
            player.move(0, -speed, deltaTime);
            skybox.transforms = glm::translate(IDENTITY_MAT4, {0, 0, 0});
            skybox.transforms *= glm::scale(IDENTITY_MAT4, skyboxScale);
        }

        if (glfwGetKey(rend.window, GLFW_KEY_UP) == GLFW_PRESS) {
            player.look(glm::vec2(0, sensitivity), deltaTime);
        }
// Move backward
        if (glfwGetKey(rend.window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            player.look(glm::vec2(0, -sensitivity), deltaTime);
        }
// Strafe right
        if (glfwGetKey(rend.window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            player.look(glm::vec2(-sensitivity, 0), deltaTime);
        }
// Strafe left
        if (glfwGetKey(rend.window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            player.look(glm::vec2(sensitivity, 0), deltaTime);
        }

        if (glfwGetKey(rend.window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            player.position.y += (deltaTime * speed);
            skybox.transforms = glm::translate(IDENTITY_MAT4, {0, 0, 0});
            skybox.transforms *= glm::scale(IDENTITY_MAT4, skyboxScale);
        }

        if (glfwGetKey(rend.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            player.position.y += -(deltaTime * speed);
            skybox.transforms = glm::translate(IDENTITY_MAT4, {0, 0, 0});
            skybox.transforms *= glm::scale(IDENTITY_MAT4, skyboxScale);
        }

        rend.clear(0.25f, 0.25f, 1, 1);

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

        rend.proj = player.getProjection(fov);
        rend.view = player.getView();

        sp.bind();
        sp.setUniform4f("u_Tint", tint.x, tint.y, tint.z, tint.w);

        rend.drawObject("skybox");
        rend.drawObject("purpur");

        rend.drawImGui();

        rend.flip();
    }

    rend.quit();

    std::cout << "App stopped without errors." << std::endl;
    return 0;
}
