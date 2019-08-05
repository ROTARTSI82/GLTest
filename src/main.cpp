#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
//#include <random>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

using namespace std;

unordered_map<string, unsigned int> SHADER_TYPES = {
        {"fragment-shader", GL_FRAGMENT_SHADER},
        {"vertex-shader",   GL_VERTEX_SHADER}
};

static void flushGLErrors() {
    GLenum err = glGetError();
    while (err != GL_NO_ERROR) {
        cerr << "[OpenGL ERROR]: 0x" << hex << err << endl;
        err = glGetError();
    }
}

static unsigned int compileShader(const string &type, const string &src, const string &fullpath) {
    unsigned int shadertype;
    if (SHADER_TYPES.find(type) != SHADER_TYPES.end()) {
        shadertype = SHADER_TYPES[type];
    } else {
        cerr << "Invalid shader type: " << type << " // Skipping..." << endl;
        return 0;
    }

    unsigned int shader = glCreateShader(shadertype);
    const char *rsrc = src.c_str();

    glShaderSource(shader, 1, &rsrc, nullptr);
    glCompileShader(shader);

    int result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        cerr << "Error compiling " << type << " from " << fullpath << endl;
        int len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        char msg[len];
        glGetShaderInfoLog(shader, len, &len, msg);
        cerr << msg << endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

//static unsigned int setShaders(const string &vertSrc, const string &fragSrc) {
//    unsigned int program = glCreateProgram();
//    unsigned int vertShader = compileShader(GL_VERTEX_SHADER, vertSrc);
//    unsigned int fragShader = compileShader(GL_FRAGMENT_SHADER, fragSrc);
//
//    glAttachShader(program, vertShader);
//    glAttachShader(program, fragShader);
//
//    glLinkProgram(program);
//    glValidateProgram(program);
//
//    glDeleteShader(fragShader);
//    glDeleteShader(vertShader);
//
//    return program;
//}

static unsigned int setShaderProfile(const string &path) {
    string profilePath = path + "/shaders.meta";
    ifstream metadat(profilePath);

    string line;
    unsigned int prgm = glCreateProgram();
    vector<unsigned int> shaders;

    if (!metadat.is_open()) {
        cerr << "Failed to load shaders: cannot read " << profilePath << endl;
        return 0;
    }

    while (getline(metadat, line)) {
        unsigned long indx = line.find(": ");
        if (line[0] == '#' ||
            indx == string::npos) { // comments. Skip lines that don't have colons or contain the # character.
            continue;
        }
        string shaderStrType = line.substr(0, indx);
        string filename = line.substr(indx + 2);
        string fullPath = path + "/" + filename;

        string shaderSrc;
        string shaderln;

        ifstream shaderStream(fullPath);

        if (!shaderStream.is_open()) {
            cerr << "Failed to load " << shaderStrType << ": cannot read " << fullPath << " // Skipping.." << endl;
            continue;
        }
        while (getline(shaderStream, shaderln)) {
            shaderSrc.append(shaderln + "\n");
        }

        // Shader compilation code.
        unsigned int shader = compileShader(shaderStrType, shaderSrc, fullPath);
        if (shader != 0) {
            glAttachShader(prgm, shader);
            shaders.push_back(shader);
            cout << "Successfully loaded " << shaderStrType << " from " << fullPath << endl;
        }

    }
    glLinkProgram(prgm);
    glValidateProgram(prgm);

    for (unsigned int s : shaders) {
        glDeleteShader(s);
    }

    cout << "Successfully loaded shader program from " << profilePath << endl;

    return prgm;

}

int main(void) {
//    std::uniform_real_distribution<double> unif(0, 1);
//    std::default_random_engine randEngine;

    GLFWwindow *window;

    /* Initialize the library */
    if (!glfwInit()) {
        cerr << "Failed to initialize GLFW! Stopping..." << endl;
        return 1;
    }

    glfwWindowHint(GLFW_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_VERSION_MINOR, 1);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

    if (!window) {
        cerr << "Failed to create GLFW window! (window=" << window << ")" << endl;
        cerr << "Stopping!" << endl;
        glfwTerminate();
        return 1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE; // Needed to use VAOs
    GLenum glewState = glewInit();
    if (glewState != GLEW_OK) {
        cerr << "Failed to initialize GLEW! (glewInit() returned " << glewState << ")" << endl;
        return 1;
    }

    cout << "Successfully initialized OpenGL (with GLFW & GLEW) version " << glGetString(GL_VERSION) << endl;

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

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    // set the attribute that can be accessed at index 0 by the shader
    // to be the vertex position that contains 2 GL_FLOATs,
    // is already normalized so we dont have to normalize it again (GL_FALSE),
    // and the size of the entire vertex (including other attribs) in bytes is (sizeof(float) * 2),
    //  The position attribute begins 0 bytes into each vert.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (const void *) 0);

    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

    unsigned int lineIBO;
    glGenBuffers(1, &lineIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(lineIndex), lineIndex, GL_STATIC_DRAW);

    unsigned int pgm = setShaderProfile("./res/shaders/default");
    if (pgm != 0) {
        glUseProgram(pgm);
    }

    int tint = glGetUniformLocation(pgm, "u_Tint");
    int mult = glGetUniformLocation(pgm, "u_Mult");

    glUniform4f(tint, 1, 1, 1, 1);
    glUniform4f(mult, -1, -1, -1, -1);

    bool wireframe = false;

    if (!wireframe) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
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
            glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
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

    glfwTerminate();
    cout << "App stopped without errors." << endl;
    return 0;
}
