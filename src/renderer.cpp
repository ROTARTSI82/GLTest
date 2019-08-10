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

void glfwErrCallback(int error, const char *description) {
    std::cerr << "[GLFW Error [" << error << "]]: " << description << std::endl;
}

bool Renderer::init(const char *title, int x, int y, GLFWmonitor *monitor, GLFWwindow *share) {
    glfwSetErrorCallback(glfwErrCallback);

    /* Initialize the library */
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        std::cerr << "Stopping!" << std::endl;
        glfwTerminate();
        return false;
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(x, y, title, monitor, share);

    if (!window) {
        std::cerr << "Failed to create GLFW window! (window=" << window << ")" << std::endl;
        std::cerr << "Stopping!" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE; // Needed to use VAOs
    {
        GLenum glewState = glewInit();
        if (glewState != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW! (glewInit() returned " << glewState << ")" << std::endl;
            std::cerr << "Stopping!" << std::endl;
            glfwDestroyWindow(window);
            glfwTerminate();
            return false;
        }
    }


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

//    ImGuiIO &io = ImGui::GetIO();
//    (void) io;

    ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_ADD);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    return true;
}

void Renderer::flip() {
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Renderer::quit() {

    for (std::pair<std::string, GameObject *> object : gameObjects) {
        object.second->texture->destroy();
        object.second->shader->destroy();

        object.second->model->ibo->destroy();
        object.second->model->vbo->destroy();
        object.second->model->vao->destroy();
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    flushGLErrors();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Renderer::drawImGui() {

    ImGui::Render();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glBindVertexArray(0);

    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::addGameObject(const std::string &name, GameObject *obj) {
    gameObjects[name] = obj;
}

void Renderer::drawObject(const std::string &name) {
    if (gameObjects.find(name) != gameObjects.end()) {
        GameObject *obj = gameObjects[name];
        obj->texture->bind(0);
        obj->shader->bind();
        obj->model->vao->bind();
        obj->model->ibo->bind();

        glm::mat4 mvp = proj * view * obj->transforms;
        obj->shader->setUniformMat4f("u_MVP", mvp);

        glDrawElements(obj->model->drawMode, obj->model->ibo->count, GL_UNSIGNED_INT, nullptr);
    } else {
        std::cerr << "GameObject '" << name << "' doesn't exist! Skipping draw call..." << std::endl;
    }
}

void Renderer::clear(GLclampf r, GLclampf g, GLclampf b, GLclampf a) {
    flushGLErrors();

    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

VertexBuffer::VertexBuffer(GLsizeiptr size, const GLvoid *data, GLenum usage) : id(0) {
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

/*
 * NOTE: The data type of IndexBuffers is hardcoded to be GLuint (or unsigned int). VertexBuffers can have varying types.
 */
IndexBuffer::IndexBuffer(GLsizei count, const GLuint *data, GLenum usage) : id(0) {
    glGenBuffers(1, &id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), data, usage);

    this->count = count;
}

void IndexBuffer::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void IndexBuffer::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

ShaderProgram::ShaderProgram(const std::string &path) {
    std::string profilePath = path + "/shaders.meta";
    std::ifstream metadat(profilePath);

    std::string line;
    id = glCreateProgram();
    std::vector<GLuint> shaders;

    if (!metadat.is_open()) {
        std::cerr << "Failed to load shaders: cannot read " << profilePath << std::endl;
        id = 0;
        return;
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
            glAttachShader(id, shader);
            shaders.push_back(shader);
            std::cout << "Successfully loaded " << shaderStrType << " from " << fullPath << std::endl;
        }
    }
    glLinkProgram(id);
    glValidateProgram(id);

    for (GLuint s : shaders) {
        glDeleteShader(s);
    }

    std::cout << "Successfully loaded shader program from " << profilePath << std::endl;
}


GLuint ShaderProgram::compileShader(const std::string &type, const std::string &src, const std::string &fullpath) {
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

void ShaderProgram::bind() const {
    glUseProgram(id);
}

void ShaderProgram::unbind() const {
    glUseProgram(0);
}

GLint ShaderProgram::getUniformLoc(const std::string &name) {
    auto loc = uniforms.find(name);
    if (loc != uniforms.end()) {  // It's already cached. No need to access GPU again.
        return loc->second;
    }

    GLint uni = glGetUniformLocation(id, name.c_str());
    if (uni == -1) {
        std::cerr << "[WARNING]: Uniform " << name << " doesn't exist!" << std::endl;
        return -1;
    }
    uniforms[name] = uni;
    return uni;
}

void ShaderProgram::setUniform4f(const std::string &name, float f0, float f1, float f2, float f3) {
    GLint loc = getUniformLoc(name);
    if (loc != -1) {
        glUniform4f(loc, f0, f1, f2, f3);
    }
}

void ShaderProgram::setUniform1i(const std::string &name, int v) {
    GLint loc = getUniformLoc(name);
    if (loc != -1) {
        glUniform1i(loc, v);
    }
}

void ShaderProgram::setUniformMat4f(const std::string &name, glm::mat4 &mat4, GLboolean transpose) {
    GLint loc = getUniformLoc(name);
    if (loc != -1) {
        glUniformMatrix4fv(loc, 1, transpose, &mat4[0][0]);
    }
}

VertexArray::VertexArray() : id(0) {
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

void ShaderProgram::destroy() {
    glDeleteProgram(id);
}

void VertexBuffer::destroy() {
    glDeleteBuffers(1, &id);
}

void IndexBuffer::destroy() {
    glDeleteBuffers(1, &id);
}

void VertexArray::destroy() {
    glDeleteVertexArrays(1, &id);
}

Texture::Texture(const std::string &path, GLenum type, GLint lod, GLint border) : localBuf(nullptr), width(0),
                                                                                  height(0), bits(0), id(0) {
    this->fp = path;
    this->textureType = type;

    localBuf = stbi_load(path.c_str(), &width, &height, &bits, 4); // 4 channels for RGBA

    glGenTextures(1, &id);
    bind();

    setRenderHints({{GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                    {GL_TEXTURE_MAG_FILTER, GL_LINEAR}}); // DEFAULT VALUES
    // {GL_TEXTURE_WARP_S, GL_CLAMP}, {GL_TEXTURE_WARP_T, GL_CLAMP}}); // These for some reason don't work :(

    glTexImage2D(type, lod, GL_RGBA8, width, height, border, GL_RGBA, GL_UNSIGNED_BYTE, localBuf);

    if (localBuf) {
        stbi_image_free(localBuf);
    }
}

void Texture::bind(GLuint texSlot) {
    this->slot = texSlot;
    glActiveTexture(GL_TEXTURE0 + texSlot);
    glBindTexture(textureType, id);
}

void Texture::unbind() const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(textureType, id);
}

void Texture::destroy() {
    glDeleteTextures(1, &id);
}

void Texture::genMipmaps() {
    bind();
    glGenerateMipmap(textureType);
}

void Texture::setRenderHints(std::unordered_map<GLenum, GLint> hints) {
    bind();

    for (std::pair<GLenum, GLint> hint : hints) {
        glTexParameteri(textureType, hint.first, hint.second);
    }
}

Camera::Camera(glm::vec3 pos, glm::vec2 look, GLFWwindow *window) {
    win = window;
    lookAngle = look;
    position = pos;

    lookDirection = glm::vec3(cos(lookAngle.y) * sin(lookAngle.x), sin(lookAngle.y),
                              cos(lookAngle.y) * cos(lookAngle.x));

    right = glm::vec3(sin(lookAngle.x - pi / 2.0f), 0, cos(lookAngle.x - pi / 2.0f));
    forward = glm::vec3(sin(lookAngle.x), 0, cos(lookAngle.x));
    up = glm::cross(right, lookDirection);
}

void Camera::move(float forwardAmount, float rightAmount, float deltaTime) {
    position += forward * forwardAmount * deltaTime;
    position += right * rightAmount * deltaTime;
}

void Camera::look(glm::vec2 amount, float deltaTime) {
    lookAngle += amount * deltaTime;

    if (lookAngle.y > pi / 2) {
        lookAngle.y = pi / 2;
    } else if (lookAngle.y < -pi / 2) {
        lookAngle.y = -pi / 2;
    }

    lookDirection = glm::vec3(cos(lookAngle.y) * sin(lookAngle.x), sin(lookAngle.y),
                              cos(lookAngle.y) * cos(lookAngle.x));

    right = glm::vec3(sin(lookAngle.x - pi / 2.0f), 0, cos(lookAngle.x - pi / 2.0f));
    forward = glm::vec3(sin(lookAngle.x), 0, cos(lookAngle.x));
    up = glm::cross(right, lookDirection);
}

glm::mat4 Camera::getProjection(float fov, float near, float far) {
    int width, height;
    glfwGetWindowSize(win, &width, &height);

    return glm::perspective(glm::radians(fov), (float) width / (float) height, near, far);
}

glm::mat4 Camera::getView() {
    return glm::lookAt(position, position + lookDirection, up);
}