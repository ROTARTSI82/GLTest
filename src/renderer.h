//
// Created by Grant on 2019-08-05.
//
#pragma once

#ifndef GRANT_RENDERER_H_DEFINED
#define GRANT_RENDERER_H_DEFINED

#define STB_IMAGE_IMPLEMENTATION

#include <STB/stb_image.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <utility>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <string>

#include <stdio.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_glfw.h"

#include "imgui/imgui.cpp"
#include "imgui/imgui_impl_glfw.cpp"
#include "imgui/imgui_impl_opengl2.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/imgui_demo.cpp"


#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif


std::unordered_map<std::string, GLuint> SHADER_TYPES = {
        {"fragment-shader", GL_FRAGMENT_SHADER},
        {"vertex-shader",   GL_VERTEX_SHADER},
        {"geometry-shader", GL_GEOMETRY_SHADER}
};

std::unordered_map<GLenum, GLsizei> SIZES = {
        {GL_FLOAT,        sizeof(GLfloat)},
        {GL_UNSIGNED_INT, sizeof(GLuint)}
};

glm::mat4 IDENTITY_MAT4 = glm::mat4(1.0f);

double pi = atan(1) * 4;

struct VBAttribute {
public:
    GLint count;
    GLenum type;
    GLboolean normalized;
    GLsizei pointer;
};

class ShaderProgram {
public:

    std::unordered_map<std::string, GLint> uniforms = {};
    GLuint id;

    explicit ShaderProgram(const std::string &path);

    void bind() const;

    void unbind() const;

    GLint getUniformLoc(const std::string &name);

    void setUniform1i(const std::string &name, int v);

    void setUniform4f(const std::string &name, float f0, float f1, float f2, float f3);

    void setUniformMat4f(const std::string &name, glm::mat4 &mat4, GLboolean transpose = GL_FALSE);

    GLuint compileShader(const std::string &type, const std::string &src, const std::string &fullpath);

    void destroy();
};

class IndexBuffer {
public:
    GLuint id;
    GLsizei count;

    IndexBuffer(GLsizei count, const GLuint *data, GLenum usage);

    void bind() const;

    void unbind() const;

    void destroy();
};

class VBLayout {
public:
    std::vector<VBAttribute> attribs;
    GLsizei stride = 0;

    VBLayout() = default;

    void addAttribute(GLint count, GLenum type, GLboolean normalized);
};

class VertexArray {
public:
    GLuint id;

    VertexArray();

    void bind() const;

    void unbind() const;

    void destroy();
};

class Texture {
public:
    GLenum textureType;
    GLuint id;
    GLuint slot;
    std::string fp;
    unsigned char *localBuf;
    int width, height, bits;

    explicit Texture(const std::string &path, GLenum type, GLint lod, GLint border);

    void genMipmaps();

    void setRenderHints(std::unordered_map<GLenum, GLint> hints);

    void destroy();

    void bind(GLuint slot = 0);

    void unbind() const;
};

class VertexBuffer {
public:
    GLuint id;

    VertexBuffer(GLsizeiptr size, const GLvoid *data, GLenum usage);

    void bind() const;

    void unbind() const;

    void setLayout(VBLayout layout, VertexArray vertArr);

    void destroy();
};

class Camera {
public:
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 forward;

    glm::vec3 lookDirection;
    glm::vec3 position;
    glm::vec2 lookAngle;
    GLFWwindow *win;

    Camera(glm::vec3 pos, glm::vec2 look, GLFWwindow *window);

    glm::mat4 getProjection(float fov, float near = 0.1f, float far = 100.0f);

    glm::mat4 getView();

    void look(glm::vec2 amount, float deltaTime);

    void move(float forward, float right, float deltaTime);
};

struct Model {
public:
    IndexBuffer *ibo;
    VertexArray *vao;
    GLenum drawMode;

    // This is just for destroyin'
    VertexBuffer *vbo;
};

struct GameObject {
public:
    Model *model;
    ShaderProgram *shader;
    Texture *texture;

    glm::mat4 transforms = glm::mat4(1.0f);
};

class Renderer {
public:
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::mat4(1.0f);
    GLFWwindow *window;
    std::unordered_map<std::string, GameObject *> gameObjects;

    bool init(const char *title, int x, int y, GLFWmonitor *monitor = nullptr, GLFWwindow *share = nullptr);

    void quit();

    void clear(GLclampf r, GLclampf g, GLclampf b, GLclampf a);

    void addGameObject(const std::string &name, GameObject *obj);

    void drawObject(const std::string &obj);

    void drawImGui();

    void flip();

};

void flushGLErrors();

void glfwErrCallback(int error, const char *description);

#endif
