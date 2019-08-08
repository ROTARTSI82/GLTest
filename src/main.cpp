#include "renderer.cpp"

float scrollY = 0;


void scrollCallback(GLFWwindow *win, double x, double y) {
    scrollY = y;
}


int main(void) {
//    std::uniform_real_distribution<double> unif(0, 1);
//    std::default_random_engine randEngine;

    GLFWwindow *window;

    /* Initialize the library */
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW! Stopping..." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_VERSION_MINOR, 1);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(960, 540, "Hello World", NULL, NULL);

    if (!window) {
        std::cerr << "Failed to create GLFW window! (window=" << window << ")" << std::endl;
        std::cerr << "Stopping!" << std::endl;
        glfwTerminate();
        return 1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetScrollCallback(window, scrollCallback);

    glewExperimental = GL_TRUE; // Needed to use VAOs
    GLenum glewState = glewInit();
    if (glewState != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW! (glewInit() returned " << glewState << ")" << std::endl;
        return 1;
    }

    std::cout << "Successfully initialized OpenGL (with GLFW & GLEW) version " << glGetString(GL_VERSION) << std::endl;

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
            4, 5, 6, 7,

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

    // Culling is completely broken. ;(

//    glEnable(GL_CULL_FACE);
//    glCullFace(GL_FRONT_AND_BACK);

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

    // glm::mat4 projectionMat = glm::ortho(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f);

    // 45 deg FOV, 4:3 aspect ratio, display range 0.1 - 100 uints (anything outside range is culled)
    glm::mat4 projectionMat;

    // Transformations for the veiw.
    glm::mat4 viewMat;

    // Transformations for the models
    glm::mat4 modelMat = glm::mat4(1.0f); // Model is at origin. No need for further transformations.

    glm::mat4 mvp = projectionMat * viewMat * modelMat;

    ShaderProgram sp("./res/shaders/default");
    sp.bind();

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
    tex2.bind(1);

    Camera player(glm::vec3(0, 0, 0), glm::vec2(0, 0), window);

    float speed = 3;
    float sensitivity = 3;

    double lastFrame = glfwGetTime();

    unsigned int counter = 0;

    float deltaTime;

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        deltaTime = float(now - lastFrame);
        lastFrame = now;


        flushGLErrors();
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.25f, 0.25f, 1, 1);


        ibo.draw(GL_QUADS);

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

        projectionMat = player.getProjection(45 - 5 * scrollY);
        viewMat = player.getView();

        mvp = projectionMat * viewMat * modelMat;
        sp.setUniformMat4f("u_MVP", mvp);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

//        int winW, winH;
//        glfwGetWindowSize(window, &winW, &winH);
//        mvp = glm::ortho(0.0f, (float) winW, (float) winH, 0.0f, 1.0f, -1.0f) * viewMat * modelMat;
//        sp.setUniformMat4f("u_MVP", mvp);

        counter++;
        if (counter > 120) {
            counter = 0;
        }

        sp.setUniform1i("u_Texture", (counter > 60 ? 0 : 1));
    }

    sp.destroy();
    va.destroy();
    buf.destroy();
    ibo.destroy();
    tex.destroy();

    flushGLErrors();
    glfwTerminate();
    std::cout << "App stopped without errors." << std::endl;
    return 0;
}
