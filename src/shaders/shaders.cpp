
#include <mutex>
#include <spdlog/spdlog.h>
#include "shaders.h"
#include "../controller.h"

Shader::Shader(const char *vertexPath, const char *fragmentPath) : Shader(vertexPath, fragmentPath, nullptr) {
}

Shader::Shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath) {
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // if geometry shader path is present, also load a geometry shader
        if (geometryPath != nullptr) {
            gShaderFile.open(geometryPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::ifstream::failure &e) {
        spdlog::error("shader file not read successfully {} {}", e.code().value(), e.code().message());
    }
    compileShaders(geometryPath, vertexCode, fragmentCode, geometryCode);

}

void Shader::compileShaders(const char *geometryPath, const std::string &vertexCode, const std::string &fragmentCode, const std::string &geometryCode) {
    controller::executeOpenGL([&](){
        const char *vShaderCode = vertexCode.c_str();
        const char *fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, nullptr);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, nullptr);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // if geometry shader is given, compile geometry shader
        unsigned int geometry;
        if (geometryPath != nullptr) {
            const char *gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, nullptr);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }
        // shader Program
        id = glCreateProgram();
        glAttachShader(id, vertex);
        glAttachShader(id, fragment);
        if (geometryPath != nullptr)
            glAttachShader(id, geometry);
        glLinkProgram(id);
        checkCompileErrors(id, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometryPath != nullptr) {
            glDeleteShader(geometry);
        }
    });
}

void Shader::use() const {
    glUseProgram(id);
}

void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int) value);
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const {
    glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::setVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const {
    glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const {
    glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const {
    glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileErrors(GLuint shader, const std::string &type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            spdlog::error("{} shader compilation error: \n{}", type, infoLog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            spdlog::error("{} shader program linking error: \n{}", type, infoLog);
        }
    }
}

Shader::~Shader() {
    controller::executeOpenGL([this](){
        glDeleteProgram(id);
    });
}

namespace shaders {
    namespace {
        std::map<shader_id_t, std::unique_ptr<Shader>> allShaders;
    }
    const Shader &get(shader_id_t id) {
        return *allShaders[id];
    }

    void initialize() {
        allShaders[TRIANGLE] = std::make_unique<Shader>("src/shaders/triangle_shader.vsh", "src/shaders/triangle_shader.fsh");
        allShaders[TEXTURED_TRIANGLE] = std::make_unique<Shader>("src/shaders/textured_triangle_shader.vsh", "src/shaders/textured_triangle_shader.fsh");
        allShaders[LINE] = std::make_unique<Shader>("src/shaders/line_shader.vsh", "src/shaders/line_shader.fsh");
        allShaders[OPTIONAL_LINE] = std::make_unique<Shader>("src/shaders/optional_line_shader.vsh", "src/shaders/line_shader.fsh", "src/shaders/optional_line_shader.gsh");
        allShaders[OVERLAY] = std::make_unique<Shader>("src/shaders/overlay_shader.vsh", "src/shaders/simple_color_forwarding_shader.fsh");
        allShaders[TRIANGLE_SELECTION] = std::make_unique<Shader>("src/shaders/triangle_selection_shader.vsh", "src/shaders/simple_color_forwarding_shader.fsh");
        allShaders[TEXTURED_TRIANGLE_SELECTION] = std::make_unique<Shader>("src/shaders/textured_triangle_selection_shader.vsh", "src/shaders/simple_color_forwarding_shader.fsh");

        spdlog::debug("shader IDs: "
                      "TRIANGLE={} "
                      "TEXTURED_TRIANGLE={} "
                      "LINE={} "
                      "OPTIONAL_LINE={} "
                      "OVERLAY={} "
                      "TRIANGLE_SELECTION={} "
                      "TEXTURED_TRIANGLE_SELECTION={}",
                      allShaders[TRIANGLE]->id,
                      allShaders[TEXTURED_TRIANGLE]->id,
                      allShaders[LINE]->id,
                      allShaders[OPTIONAL_LINE]->id,
                      allShaders[OVERLAY]->id,
                      allShaders[TRIANGLE_SELECTION]->id,
                      allShaders[TEXTURED_TRIANGLE_SELECTION]->id
                      );
    }

    void cleanup() {
        allShaders.clear();
    }
}
