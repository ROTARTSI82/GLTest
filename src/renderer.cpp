//
// Created by Grant on 2019-08-05.
//

#include "renderer.h"

void flushGLErrors() {
    GLenum err = glGetError();
    while (err != GL_NO_ERROR) {
        std::cerr << "[OpenGL ERROR]: 0x" << std::hex << err << std::endl;
        err = glGetError();
    }
}


VertexBuffer::VertexBuffer(GLsizeiptr size, const GLvoid *data, GLenum usage) {
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

void VertexBuffer::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, id);
}

void VertexBuffer::unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

IndexBuffer::IndexBuffer(GLsizei count, const GLuint *data, GLenum usage) {
    glGenBuffers(1, &id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), data, usage);

    this->count = count;
}

void IndexBuffer::draw(GLenum mode) const {
    bind();
    glDrawElements(mode, count, GL_UNSIGNED_INT, nullptr);
}

void IndexBuffer::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void IndexBuffer::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


GLuint compileShader(const std::string &type, const std::string &src, const std::string &fullpath) {
    GLenum shadertype;
    if (SHADER_TYPES.find(type) != SHADER_TYPES.end()) {
        shadertype = SHADER_TYPES[type];
    } else {
        std::cerr << "Invalid shader type: " << type << " // Skipping..." << std::endl;
        return 0;
    }

    GLuint shader = glCreateShader(shadertype);
    const char *rsrc = src.c_str();

    glShaderSource(shader, 1, &rsrc, nullptr);
    glCompileShader(shader);

    GLint result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        std::cerr << "Error compiling " << type << " from " << fullpath << std::endl;
        GLint len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        char msg[len];
        glGetShaderInfoLog(shader, len, &len, msg);
        std::cerr << msg << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint setShaderProfile(const std::string &path) {
    std::string profilePath = path + "/shaders.meta";
    std::ifstream metadat(profilePath);

    std::string line;
    GLuint prgm = glCreateProgram();
    std::vector<GLuint> shaders;

    if (!metadat.is_open()) {
        std::cerr << "Failed to load shaders: cannot read " << profilePath << std::endl;
        return 0;
    }

    while (getline(metadat, line)) {
        int indx = line.find(": ");
        if (line[0] == '#' ||
            indx == std::string::npos) { // comments. Skip lines that don't have colons or contain the # character.
            continue;
        }
        std::string shaderStrType = line.substr(0, indx);
        std::string filename = line.substr(indx + 2);
        std::string fullPath = path + "/" + filename;

        std::string shaderSrc;
        std::string shaderln;

        std::ifstream shaderStream(fullPath);

        if (!shaderStream.is_open()) {
            std::cerr << "Failed to load " << shaderStrType << ": cannot read " << fullPath << " // Skipping.."
                      << std::endl;
            continue;
        }
        while (getline(shaderStream, shaderln)) {
            shaderSrc.append(shaderln + "\n");
        }

        // Shader compilation code.
        GLuint shader = compileShader(shaderStrType, shaderSrc, fullPath);
        if (shader != 0) {
            glAttachShader(prgm, shader);
            shaders.push_back(shader);
            std::cout << "Successfully loaded " << shaderStrType << " from " << fullPath << std::endl;
        }

    }
    glLinkProgram(prgm);
    glValidateProgram(prgm);

    for (GLuint s : shaders) {
        glDeleteShader(s);
    }

    std::cout << "Successfully loaded shader program from " << profilePath << std::endl;

    return prgm;
}

VertexArray::VertexArray() {
    glGenVertexArrays(1, &id);
    glBindVertexArray(id);
}

void VertexArray::bind() const {
    glBindVertexArray(id);
}

void VertexArray::unbind() const {
    glBindVertexArray(0);
}

void VBLayout::addAttribute(GLint count, GLenum type, GLboolean normalized) {
    attribs.push_back({count, type, normalized, stride});
    stride += SIZES[type] * count;
}

void VertexBuffer::setLayout(VBLayout layout, VertexArray vertArr) {
    vertArr.bind();

    for (int i = 0; i < layout.attribs.size(); i++) {
        VBAttribute attrib = layout.attribs.at(i);
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, attrib.count, attrib.type, attrib.normalized, layout.stride,
                              (const void *) attrib.pointer);
    }
}
