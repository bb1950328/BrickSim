#include "opengl_native_or_replacement.h"
#include <vector>
#include <spdlog/spdlog.h>

namespace bricksim::graphics::opengl_native_or_replacement {

    void drawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLuint baseinstance,
                                           GLuint instanceBufferId, GLsizeiptr totalInstanceBufferSize, size_t instanceSize) {
        if (glDrawElementsInstancedBaseInstance) {
            glDrawElementsInstancedBaseInstance(mode, count, type, indices, instancecount, baseinstance);
        } else if (baseinstance == 0) {
            glDrawElementsInstanced(mode, count, type, indices, instancecount);
        } else {
            //I know that this is a very inefficient solution, but most graphics cards support glDrawElementsInstancedBaseInstance anyways
            //and it will only be called when the same mesh is in multiple layers
            std::vector<uint8_t> originalData;
            originalData.resize(totalInstanceBufferSize);
            glBindBuffer(GL_ARRAY_BUFFER, instanceBufferId);
            glGetBufferSubData(GL_ARRAY_BUFFER, 0, totalInstanceBufferSize, &originalData[0]);
            glBindBuffer(GL_ARRAY_BUFFER, instanceBufferId);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(instancecount * instanceSize), &originalData[baseinstance * instanceSize], GL_STATIC_DRAW);

            glDrawElementsInstanced(mode, count, type, indices, instancecount);

            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(originalData.size()), &originalData[0], GL_STATIC_DRAW);
        }
    }

    void drawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance,
                                           GLuint instanceBufferId, GLsizeiptr totalInstanceBufferSize, size_t instanceSize) {
        if (glDrawArraysInstancedBaseInstance) {
            glDrawArraysInstancedBaseInstance(mode, first, count, instancecount, baseinstance);
        } else if (baseinstance == 0) {
            glDrawArraysInstanced(mode, first, count, instancecount);
        } else {
            //I know that this is a very inefficient solution, but most graphics cards support glDrawElementsInstancedBaseInstance anyways
            //and it will only be called when the same mesh is in multiple layers
            std::vector<uint8_t> originalData;
            originalData.resize(totalInstanceBufferSize);
            glBindBuffer(GL_ARRAY_BUFFER, instanceBufferId);
            glGetBufferSubData(GL_ARRAY_BUFFER, 0, totalInstanceBufferSize, &originalData[0]);
            glBindBuffer(GL_ARRAY_BUFFER, instanceBufferId);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(instancecount * instanceSize), &originalData[baseinstance * instanceSize], GL_STATIC_DRAW);

            glDrawArraysInstanced(mode, first, count, instancecount);

            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(originalData.size()), &originalData[0], GL_STATIC_DRAW);
        }
    }

    void initialize() {
        if (!glDrawElementsInstancedBaseInstance) {
            spdlog::warn("GL_ARB_base_instance extension not supported by GPU");
        }
    }
}
