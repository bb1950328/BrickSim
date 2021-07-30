#pragma once

#include <glad/glad.h>
#include <cstddef>

namespace bricksim::graphics::opengl_native_or_replacement {
    void initialize();

    void drawElementsInstancedBaseInstance(GLenum mode,
                                           GLsizei count,
                                           GLenum type,
                                           const void* indices,
                                           GLsizei instancecount,
                                           GLuint baseinstance,
                                           GLuint instanceBufferId,
                                           GLsizeiptr totalInstanceBufferSize,
                                           size_t instanceSize);
    void drawArraysInstancedBaseInstance(GLenum mode,
                                         GLint first,
                                         GLsizei count,
                                         GLsizei instancecount,
                                         GLuint baseinstance,
                                         GLuint instanceBufferId,
                                         GLsizeiptr totalInstanceBufferSize,
                                         size_t instanceSize);
}