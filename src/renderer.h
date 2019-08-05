//
// Created by Grant on 2019-08-05.
//

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <string>

std::unordered_map<std::string, GLuint> SHADER_TYPES = {
        {"fragment-shader", GL_FRAGMENT_SHADER},
        {"vertex-shader",   GL_VERTEX_SHADER}
};

void flushGLErrors();

class VertexBuffer {
public:
    GLuint id;

    VertexBuffer(GLsizeiptr size, const GLvoid *data, GLenum usage);

    void bind() const;

    void unbind() const;
};

class IndexBuffer {
public:
    GLuint id;
    GLsizei count;

    IndexBuffer(GLsizei count, const GLuint *data, GLenum usage);

    void bind() const;

    void unbind() const;

    void draw(GLenum mode) const;
};

unsigned int setShaderProfile(const std::string &path);

unsigned int compileShader(const std::string &type, const std::string &src, const std::string &fullpath);
