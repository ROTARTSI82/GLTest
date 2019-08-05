#include "renderer.cpp"

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
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

    if (!window) {
        std::cerr << "Failed to create GLFW window! (window=" << window << ")" << std::endl;
        std::cerr << "Stopping!" << std::endl;
        glfwTerminate();
        return 1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE; // Needed to use VAOs
    GLenum glewState = glewInit();
    if (glewState != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW! (glewInit() returned " << glewState << ")" << std::endl;
        return 1;
    }

    std::cout << "Successfully initialized OpenGL (with GLFW & GLEW) version " << glGetString(GL_VERSION) << std::endl;

    /* Loop until the user closes the window */
    float x = 0.5;
    float pos[] = {
            -x, x, x, // 0
            x, x, x, // 1
            x, -x, x, //2
            -x, -x, x, // 3

            -x, x, -x, // 4
            x, x, -x, // 5
            x, -x, -x, // 6
            -x, -x, -x // 7
    };

    unsigned int index[] = {
            0, 1, 2, 3,
            4, 5, 6, 7,
            1, 5, 6, 2,
            4, 5, 1, 0,
            4, 0, 3, 7,
            3, 2, 6, 7
    };

    unsigned int lineIndex[] = {
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            1, 5, 2, 6, 3, 7, 4, 0
    };

    VertexArray vb;
    vb.bind();

    VBLayout layout;
    layout.addAttribute(3, GL_FLOAT, false);

    VertexBuffer buf(sizeof(pos), pos, GL_STATIC_DRAW);
    buf.bind();

    buf.setLayout(layout, vb);

//    glEnableVertexAttribArray(0);
    // set the attribute that can be accessed at index 0 by the shader
    // to be the vertex position that contains 2 GL_FLOATs,
    // is already normalized so we dont have to normalize it again (GL_FALSE),
    // and the count of the entire vertex (including other attribs) in bytes is (sizeof(float) * 2),
    //  The position attribute begins 0 bytes into each vert.
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (const void *) 0);

    IndexBuffer ibo(24, index, GL_STATIC_DRAW);

    IndexBuffer lineIBO(24, lineIndex, GL_STATIC_DRAW);

    GLuint pgm = setShaderProfile("./res/shaders/default");
    if (pgm != 0) {
        glUseProgram(pgm);
    }

    int tint = glGetUniformLocation(pgm, "u_Tint");
    int mult = glGetUniformLocation(pgm, "u_Mult");

    glUniform4f(tint, 1, 1, 1, 1);
    glUniform4f(mult, -1, -1, -1, -1);

    bool wireframe = false;

    if (!wireframe) {
        ibo.bind();
    }

    float r = 0;
    float inc = 0.0125;

    while (!glfwWindowShouldClose(window)) {
        flushGLErrors();
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        glRotatef(0.3, 0.1, 0.1, 0.1);
        glColor3f(1.0, 1.0, 1.0);

        if (!wireframe) {
            ibo.draw(GL_QUADS);
        } else {
            lineIBO.draw(GL_LINES);
        }


        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        r += inc;

        if (r > 1) {
            inc = -0.0125;
        }
        if (r < 0) {
            inc = 0.0125;
        }
        glUniform4f(tint, r, 0, 0, 1);
    }

    glDeleteProgram(pgm);
    flushGLErrors();
    glfwTerminate();
    std::cout << "App stopped without errors." << std::endl;
    return 0;
}
