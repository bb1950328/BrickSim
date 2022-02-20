#include "shaders.h"
#include "../constant_data/resources.h"
#include "../controller.h"
#include <fstream>
#include <spdlog/spdlog.h>
#include <sstream>

namespace bricksim::graphics {

    Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
        const auto hasGeometry = geometryPath != nullptr;
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
            if (hasGeometry) {
                gShaderFile.open(geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        } catch (std::ifstream::failure& e) {
            spdlog::error("shader file not read successfully {} {}", e.code().value(), e.code().message());
        }
        controller::executeOpenGL([&]() {
            linkProgram(compileShader(vertexCode.c_str(), nullptr, GL_VERTEX_SHADER, "VERTEX"),
                        compileShader(fragmentCode.c_str(), nullptr, GL_FRAGMENT_SHADER, "FRAGMENT"),
                        hasGeometry,
                        hasGeometry ? compileShader(geometryCode.c_str(), nullptr, GL_GEOMETRY_SHADER, "GEOMETRY") : 0);
        });
    }

    void Shader::linkProgram(unsigned int vertex, unsigned int fragment, const bool hasGeometry, unsigned int geometry) {
        id = glCreateProgram();
        glAttachShader(id, vertex);
        glAttachShader(id, fragment);
        if (hasGeometry) {
            glAttachShader(id, geometry);
        }
        glLinkProgram(id);
        checkCompileErrors(id, "PROGRAM");

        // delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (hasGeometry) {
            glDeleteShader(geometry);
        }
    }

    void Shader::use() const {
        glUseProgram(id);
    }

    void Shader::setBool(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
    }

    void Shader::setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }

    void Shader::setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(id, name.c_str()), value);
    }

    void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
        glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }

    void Shader::setVec2(const std::string& name, float x, float y) const {
        glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
    }

    void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }

    void Shader::setVec3(const std::string& name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
    }

    void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
        glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }

    void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
    }

    void Shader::setMat2(const std::string& name, const glm::mat2& mat) const {
        glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void Shader::setMat3(const std::string& name, const glm::mat3& mat) const {
        glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void Shader::checkCompileErrors(GLuint shader, const std::string& type) {
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
        controller::executeOpenGL([this]() {
            glDeleteProgram(id);
        });
    }

    int Shader::compileShader(const char* const code, int* length, int type, const char* const typeName) {
        int shaderId = glCreateShader(type);
        glShaderSource(shaderId, 1, &code, length);
        glCompileShader(shaderId);
        checkCompileErrors(shaderId, typeName);
        return shaderId;
    }

    Shader::Shader(const char* vertexCodeBegin, const char* const vertexCodeEnd,
                   const char* fragmentCodeBegin, const char* fragmentCodeEnd,
                   const char* geometryCodeBegin, const char* geometryCodeEnd) {
        controller::executeOpenGL([&]() {
            int vertexLength = vertexCodeEnd - vertexCodeBegin;
            int fragmentLength = fragmentCodeEnd - fragmentCodeBegin;
            bool hasGeometry = geometryCodeBegin != nullptr;
            int geometryLength = geometryCodeEnd - geometryCodeBegin;
            linkProgram(compileShader(vertexCodeBegin, &vertexLength, GL_VERTEX_SHADER, "VERTEX"),
                        compileShader(fragmentCodeBegin, &fragmentLength, GL_FRAGMENT_SHADER, "FRAGMENT"),
                        hasGeometry,
                        hasGeometry ? compileShader(geometryCodeBegin, &geometryLength, GL_GEOMETRY_SHADER, "GEOMETRY") : 0);
        });
    }

    namespace shaders {
        namespace {
            uomap_t<shader_id_t, std::unique_ptr<Shader>> allShaders;
        }

        const Shader& get(shader_id_t id) {
            return *allShaders[id];
        }

        void initialize() {
            if (is_directory(std::filesystem::path("./resources/shaders"))) {
                spdlog::info("loading shaders from source files");
                allShaders[TRIANGLE] = std::make_unique<Shader>("resources/shaders/triangle_shader.vsh", "resources/shaders/triangle_shader.fsh");
                allShaders[TEXTURED_TRIANGLE] = std::make_unique<Shader>("resources/shaders/textured_triangle_shader.vsh", "resources/shaders/textured_triangle_shader.fsh");
                allShaders[LINE] = std::make_unique<Shader>("resources/shaders/line_shader.vsh", "resources/shaders/line_shader.fsh");
                allShaders[OPTIONAL_LINE] = std::make_unique<Shader>("resources/shaders/optional_line_shader.vsh", "resources/shaders/line_shader.fsh", "resources/shaders/optional_line_shader.gsh");
                allShaders[OVERLAY] = std::make_unique<Shader>("resources/shaders/overlay_shader.vsh", "resources/shaders/simple_color_forwarding_shader.fsh");
                allShaders[TRIANGLE_SELECTION] = std::make_unique<Shader>("resources/shaders/triangle_selection_shader.vsh", "resources/shaders/simple_color_forwarding_shader.fsh");
                allShaders[TEXTURED_TRIANGLE_SELECTION] = std::make_unique<Shader>("resources/shaders/textured_triangle_selection_shader.vsh", "resources/shaders/simple_color_forwarding_shader.fsh");
            } else {
                spdlog::info("loading shaders from embedded data");

                auto createShaderVF = [](const auto& vertexShader, const auto& fragmentShader) {
                    return std::make_unique<Shader>((const char*)(& (*vertexShader.begin())), (const char*)(&(*vertexShader.end())),
                                                    (const char*)(&(*fragmentShader.begin())), (const char*)(&(*fragmentShader.end())));
                };
                auto createShaderVFG = [](const auto& vertexShader, const auto& fragmentShader, const auto& geometryShader) {
                    return std::make_unique<Shader>((const char*)(&(*vertexShader.begin())), (const char*)(&(*vertexShader.end())),
                                                    (const char*)(&(*fragmentShader.begin())), (const char*)(&(*fragmentShader.end())),
                                                    (const char*)(&(*geometryShader.begin())), (const char*)(&(*geometryShader.end())));
                };
                
                allShaders[TRIANGLE] = createShaderVF(resources::shaders::triangle_shader_vsh, resources::shaders::triangle_shader_fsh);
                allShaders[TEXTURED_TRIANGLE] = createShaderVF(resources::shaders::textured_triangle_shader_vsh, resources::shaders::textured_triangle_shader_fsh);
                allShaders[LINE] = createShaderVF(resources::shaders::line_shader_vsh, resources::shaders::line_shader_fsh);
                allShaders[OPTIONAL_LINE] = createShaderVFG(resources::shaders::optional_line_shader_vsh, resources::shaders::line_shader_fsh, resources::shaders::optional_line_shader_gsh);
                allShaders[OVERLAY] = createShaderVF(resources::shaders::overlay_shader_vsh, resources::shaders::simple_color_forwarding_shader_fsh);
                allShaders[TRIANGLE_SELECTION] = createShaderVF(resources::shaders::triangle_selection_shader_vsh, resources::shaders::simple_color_forwarding_shader_fsh);
                allShaders[TEXTURED_TRIANGLE_SELECTION] = createShaderVF(resources::shaders::textured_triangle_selection_shader_vsh, resources::shaders::simple_color_forwarding_shader_fsh);
            }
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
                          allShaders[TEXTURED_TRIANGLE_SELECTION]->id);
        }

        void cleanup() {
            allShaders.clear();
        }
    }
}