//
// Created by Grant on 2019-08-05.
//

#pragma once

#define STB_IMAGE_IMPLEMENTATION

#include <STB/stb_image.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <string>

#include <unistd.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

std::unordered_map<std::string, GLuint> SHADER_TYPES = {
        {"fragment-shader", GL_FRAGMENT_SHADER},
        {"vertex-shader",   GL_VERTEX_SHADER}
};

std::unordered_map<GLenum, GLsizei> SIZES = {
        {GL_FLOAT,        sizeof(GLfloat)},
        {GL_UNSIGNED_INT, sizeof(GLuint)}
};

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

    void draw(GLenum mode) const;

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
    std::string fp;
    unsigned char *localBuf;
    int width, height, bits;

    explicit Texture(const std::string &path, GLenum type, GLint lod, GLint border);

    void genMipmaps();

    void setRenderHints(std::unordered_map<GLenum, GLint> hints);

    void destroy();

    void bind(GLuint slot = 0) const;

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

void flushGLErrors();
