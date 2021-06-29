#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

namespace bricksim::graphics {
    class Shader {
    public:
        unsigned int id;
        Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
        Shader(const char* vertexCodeBegin, const char* vertexCodeEnd,
               const char* fragmentCodeBegin, const char* fragmentCodeEnd,
               const char* geometryCodeBegin = nullptr, const char* geometryCodeEnd = nullptr);

        void use() const;

        // utility uniform functions
        void setBool(const std::string& name, bool value) const;
        void setInt(const std::string& name, int value) const;
        void setFloat(const std::string& name, float value) const;
        void setVec2(const std::string& name, const glm::vec2& value) const;
        void setVec2(const std::string& name, float x, float y) const;
        void setVec3(const std::string& name, const glm::vec3& value) const;
        void setVec3(const std::string& name, float x, float y, float z) const;
        void setVec4(const std::string& name, const glm::vec4& value) const;
        void setVec4(const std::string& name, float x, float y, float z, float w) const;
        void setMat2(const std::string& name, const glm::mat2& mat) const;
        void setMat3(const std::string& name, const glm::mat3& mat) const;
        void setMat4(const std::string& name, const glm::mat4& mat) const;

        virtual ~Shader();
        Shader& operator=(const Shader&) = delete;
        Shader(const Shader&) = delete;

    private:
        // utility function for checking shader compilation/linking errors.
        static void checkCompileErrors(GLuint shader, const std::string& type);
        void linkProgram(unsigned int vertex, unsigned int fragment, bool hasGeometry, unsigned int geometry);
        int compileShader(const char* const code, int* length, int type, const char* const typeName);
    };

    namespace shaders {
        enum shader_id_t {
            TRIANGLE,
            TEXTURED_TRIANGLE,
            LINE,
            OPTIONAL_LINE,
            OVERLAY,
            TRIANGLE_SELECTION,
            TEXTURED_TRIANGLE_SELECTION
        };

        const Shader& get(shader_id_t id);

        void initialize();
        void cleanup();
    }
}
