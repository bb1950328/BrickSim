#include "shaders.h"
#include "../constant_data/resources.h"
#include "../controller.h"
#include "palanteer.h"
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
            std::stringstream vShaderStream;
            std::stringstream fShaderStream;
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
        controller::executeOpenGL([this, &vertexCode, &fragmentCode, &hasGeometry, &geometryCode]() {
            linkProgram(compileShader(vertexCode, GL_VERTEX_SHADER, "VERTEX"),
                        compileShader(fragmentCode, GL_FRAGMENT_SHADER, "FRAGMENT"),
                        hasGeometry,
                        hasGeometry ? compileShader(geometryCode, GL_GEOMETRY_SHADER, "GEOMETRY") : 0);
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
        std::string infoLog;
        infoLog.resize(1024);
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, static_cast<GLsizei>(infoLog.capacity()), nullptr, infoLog.data());
                spdlog::error("{} shader compilation error: \n{}", type, infoLog);
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, static_cast<GLsizei>(infoLog.capacity()), nullptr, infoLog.data());
                spdlog::error("{} shader program linking error: \n{}", type, infoLog);
            }
        }
    }

    Shader::~Shader() {
        controller::executeOpenGL([this]() {
            glDeleteProgram(id);
        });
    }

    int Shader::compileShader(std::string_view code, int type, const char* const typeName) {
        int shaderId = glCreateShader(type);
        const char* string = code.data();
        const auto length = static_cast<GLint>(code.size());
        glShaderSource(shaderId, 1, &string, &length);
        glCompileShader(shaderId);
        checkCompileErrors(shaderId, typeName);
        return shaderId;
    }

    Shader::Shader(std::string_view vertexCode, std::string_view fragmentCode, std::optional<std::string_view> geometryCode) {
        controller::executeOpenGL([&vertexCode, &fragmentCode, &geometryCode, this]() {
            bool hasGeometry = geometryCode.has_value();
            linkProgram(compileShader(vertexCode, GL_VERTEX_SHADER, "VERTEX"),
                        compileShader(fragmentCode, GL_FRAGMENT_SHADER, "FRAGMENT"),
                        hasGeometry,
                        hasGeometry ? compileShader(geometryCode.value(), GL_GEOMETRY_SHADER, "GEOMETRY") : 0);
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
            plScope("shaders::initialize");
            if (is_directory(std::filesystem::path("./resources/shaders"))) {
                spdlog::info("loading shaders from source files");
                allShaders[shader_id_t::TRIANGLE] = std::make_unique<Shader>("resources/shaders/triangle_shader.vsh", "resources/shaders/triangle_shader.fsh");
                allShaders[shader_id_t::TEXTURED_TRIANGLE] = std::make_unique<Shader>("resources/shaders/textured_triangle_shader.vsh", "resources/shaders/textured_triangle_shader.fsh");
                allShaders[shader_id_t::LINE] = std::make_unique<Shader>("resources/shaders/line_shader.vsh", "resources/shaders/line_shader.fsh");
                allShaders[shader_id_t::OPTIONAL_LINE] = std::make_unique<Shader>("resources/shaders/optional_line_shader.vsh", "resources/shaders/line_shader.fsh", "resources/shaders/optional_line_shader.gsh");
                allShaders[shader_id_t::OVERLAY] = std::make_unique<Shader>("resources/shaders/overlay_shader.vsh", "resources/shaders/simple_color_forwarding_shader.fsh");
                allShaders[shader_id_t::TRIANGLE_SELECTION] = std::make_unique<Shader>("resources/shaders/triangle_selection_shader.vsh", "resources/shaders/simple_color_forwarding_shader.fsh");
                allShaders[shader_id_t::TEXTURED_TRIANGLE_SELECTION] = std::make_unique<Shader>("resources/shaders/textured_triangle_selection_shader.vsh", "resources/shaders/simple_color_forwarding_shader.fsh");
            } else {
                spdlog::info("loading shaders from embedded data");

                auto createShaderVF = [](const auto& vertexShader, const auto& fragmentShader) {
                    plScope("createShaderVF");
                    return std::make_unique<Shader>(std::string_view(reinterpret_cast<const char*>(vertexShader.data()), vertexShader.size()),
                                                    std::string_view(reinterpret_cast<const char*>(fragmentShader.data()), fragmentShader.size()));
                };
                auto createShaderVFG = [](const auto& vertexShader, const auto& fragmentShader, const auto& geometryShader) {
                    plScope("createShaderVFG");
                    return std::make_unique<Shader>(std::string_view(reinterpret_cast<const char*>(vertexShader.data()), vertexShader.size()),
                                                    std::string_view(reinterpret_cast<const char*>(fragmentShader.data()), fragmentShader.size()),
                                                    std::string_view(reinterpret_cast<const char*>(geometryShader.data()), geometryShader.size()));
                };

                allShaders[shader_id_t::TRIANGLE] = createShaderVF(resources::shaders::triangle_shader_vsh, resources::shaders::triangle_shader_fsh);
                allShaders[shader_id_t::TEXTURED_TRIANGLE] = createShaderVF(resources::shaders::textured_triangle_shader_vsh, resources::shaders::textured_triangle_shader_fsh);
                allShaders[shader_id_t::LINE] = createShaderVF(resources::shaders::line_shader_vsh, resources::shaders::line_shader_fsh);
                allShaders[shader_id_t::OPTIONAL_LINE] = createShaderVFG(resources::shaders::optional_line_shader_vsh, resources::shaders::line_shader_fsh, resources::shaders::optional_line_shader_gsh);
                allShaders[shader_id_t::OVERLAY] = createShaderVF(resources::shaders::overlay_shader_vsh, resources::shaders::simple_color_forwarding_shader_fsh);
                allShaders[shader_id_t::TRIANGLE_SELECTION] = createShaderVF(resources::shaders::triangle_selection_shader_vsh, resources::shaders::simple_color_forwarding_shader_fsh);
                allShaders[shader_id_t::TEXTURED_TRIANGLE_SELECTION] = createShaderVF(resources::shaders::textured_triangle_selection_shader_vsh, resources::shaders::simple_color_forwarding_shader_fsh);
            }
            spdlog::debug("shader IDs: "
                          "TRIANGLE={} "
                          "TEXTURED_TRIANGLE={} "
                          "LINE={} "
                          "OPTIONAL_LINE={} "
                          "OVERLAY={} "
                          "TRIANGLE_SELECTION={} "
                          "TEXTURED_TRIANGLE_SELECTION={}",
                          allShaders[shader_id_t::TRIANGLE]->id,
                          allShaders[shader_id_t::TEXTURED_TRIANGLE]->id,
                          allShaders[shader_id_t::LINE]->id,
                          allShaders[shader_id_t::OPTIONAL_LINE]->id,
                          allShaders[shader_id_t::OVERLAY]->id,
                          allShaders[shader_id_t::TRIANGLE_SELECTION]->id,
                          allShaders[shader_id_t::TEXTURED_TRIANGLE_SELECTION]->id);
        }

        void cleanup() {
            allShaders.clear();
        }
    }
}
