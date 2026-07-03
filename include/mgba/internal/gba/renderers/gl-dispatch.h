/* Copyright (c) 2013-2026 mGBA contributors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* Minimal OpenGL 3.3 dispatch declarations for platforms that own the
 * context and resolve entry points dynamically. */
#ifndef MGBA_INTERNAL_GBA_RENDERERS_GL_DISPATCH_H
#define MGBA_INTERNAL_GBA_RENDERERS_GL_DISPATCH_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef double GLdouble;
typedef char GLchar;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

#if defined(_WIN32) && (defined(_M_IX86) || defined(__i386__))
#if defined(_MSC_VER)
#define M_GBA_GL_APIENTRY __stdcall
#else
#define M_GBA_GL_APIENTRY __attribute__((stdcall))
#endif
#else
#define M_GBA_GL_APIENTRY
#endif

#define GL_FALSE                         0
#define GL_TRUE                          1
#define GL_NONE                          0
#define GL_BYTE                          0x1400
#define GL_UNSIGNED_BYTE                 0x1401
#define GL_UNSIGNED_SHORT                0x1403
#define GL_INT                           0x1404
#define GL_FLOAT                         0x1406
#define GL_UNSIGNED_SHORT_5_6_5          0x8363
#define GL_UNSIGNED_INT_24_8             0x84FA
#define GL_TRIANGLES                     0x0004
#define GL_TRIANGLE_FAN                  0x0006
#define GL_EQUAL                         0x0202
#define GL_LESS                          0x0201
#define GL_ALWAYS                        0x0207
#define GL_FRONT_AND_BACK                0x0408
#define GL_KEEP                          0x1E00
#define GL_REPLACE                       0x1E01
#define GL_FILL                          0x1B02
#define GL_COLOR_BUFFER_BIT              0x00004000
#define GL_DEPTH_BUFFER_BIT              0x00000100
#define GL_STENCIL_BUFFER_BIT            0x00000400
#define GL_TEXTURE_2D                    0x0DE1
#define GL_TEXTURE_MAG_FILTER            0x2800
#define GL_TEXTURE_MIN_FILTER            0x2801
#define GL_TEXTURE_WRAP_S                0x2802
#define GL_TEXTURE_WRAP_T                0x2803
#define GL_NEAREST                       0x2600
#define GL_CLAMP_TO_EDGE                 0x812F
#define GL_ARRAY_BUFFER                  0x8892
#define GL_PIXEL_PACK_BUFFER             0x88EB
#define GL_PIXEL_UNPACK_BUFFER           0x88EC
#define GL_STATIC_DRAW                   0x88E4
#define GL_SCISSOR_TEST                  0x0C11
#define GL_DEPTH_TEST                    0x0B71
#define GL_STENCIL_TEST                  0x0B90
#define GL_BLEND                         0x0BE2
#define GL_CULL_FACE                     0x0B44
#define GL_DITHER                        0x0BD0
#define GL_COLOR_LOGIC_OP                0x0BF2
#define GL_RASTERIZER_DISCARD            0x8C89
#define GL_FRAMEBUFFER_SRGB              0x8DB9
#define GL_PACK_SWAP_BYTES               0x0D00
#define GL_PACK_LSB_FIRST                0x0D01
#define GL_PACK_ROW_LENGTH               0x0D02
#define GL_PACK_SKIP_ROWS                0x0D03
#define GL_PACK_SKIP_PIXELS              0x0D04
#define GL_PACK_ALIGNMENT                0x0D05
#define GL_PACK_SKIP_IMAGES              0x806B
#define GL_PACK_IMAGE_HEIGHT             0x806C
#define GL_UNPACK_SWAP_BYTES             0x0CF0
#define GL_UNPACK_LSB_FIRST              0x0CF1
#define GL_UNPACK_ROW_LENGTH             0x0CF2
#define GL_UNPACK_SKIP_ROWS              0x0CF3
#define GL_UNPACK_SKIP_PIXELS            0x0CF4
#define GL_UNPACK_ALIGNMENT              0x0CF5
#define GL_UNPACK_SKIP_IMAGES            0x806D
#define GL_UNPACK_IMAGE_HEIGHT           0x806E
#define GL_RGB                           0x1907
#define GL_RGBA                          0x1908
#define GL_RGB565                        0x8D62
#define GL_RGB16F                        0x881B
#define GL_COLOR                         0x1800
#define GL_VERSION                       0x1F02
#define GL_TEXTURE0                      0x84C0
#define GL_TEXTURE1                      0x84C1
#define GL_RED_INTEGER                   0x8D94
#define GL_RGBA_INTEGER                  0x8D99
#define GL_R16UI                         0x8234
#define GL_RGBA8I                        0x8D8E
#define GL_DEPTH_STENCIL                 0x84F9
#define GL_DEPTH24_STENCIL8              0x88F0
#define GL_FRAMEBUFFER                   0x8D40
#define GL_READ_FRAMEBUFFER              0x8CA8
#define GL_DRAW_FRAMEBUFFER              0x8CA9
#define GL_FRAMEBUFFER_COMPLETE          0x8CD5
#define GL_COLOR_ATTACHMENT0             0x8CE0
#define GL_COLOR_ATTACHMENT1             0x8CE1
#define GL_COLOR_ATTACHMENT2             0x8CE2
#define GL_DEPTH_STENCIL_ATTACHMENT      0x821A
#define GL_FRAGMENT_SHADER               0x8B30
#define GL_VERTEX_SHADER                 0x8B31
#define GL_COMPILE_STATUS                0x8B81
#define GL_LINK_STATUS                   0x8B82

#define M_GBA_GL_FUNCTIONS(F) \
	F(void, glActiveTexture, (GLenum texture)) \
	F(void, glAttachShader, (GLuint program, GLuint shader)) \
	F(void, glBindBuffer, (GLenum target, GLuint buffer)) \
	F(void, glBindSampler, (GLuint unit, GLuint sampler)) \
	F(void, glBindFramebuffer, (GLenum target, GLuint framebuffer)) \
	F(void, glBindTexture, (GLenum target, GLuint texture)) \
	F(void, glBindVertexArray, (GLuint array)) \
	F(void, glBlitFramebuffer, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, \
		GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
	F(void, glBufferData, (GLenum target, GLsizeiptr size, const void* data, GLenum usage)) \
	F(GLenum, glCheckFramebufferStatus, (GLenum target)) \
	F(void, glClear, (GLbitfield mask)) \
	F(void, glClearBufferiv, (GLenum buffer, GLint drawbuffer, const GLint* value)) \
	F(void, glClearColor, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
	F(void, glClearDepth, (GLdouble depth)) \
	F(void, glClearStencil, (GLint s)) \
	F(void, glColorMask, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)) \
	F(void, glCompileShader, (GLuint shader)) \
	F(GLuint, glCreateProgram, (void)) \
	F(GLuint, glCreateShader, (GLenum type)) \
	F(void, glDeleteBuffers, (GLsizei n, const GLuint* buffers)) \
	F(void, glDeleteFramebuffers, (GLsizei n, const GLuint* framebuffers)) \
	F(void, glDeleteProgram, (GLuint program)) \
	F(void, glDeleteShader, (GLuint shader)) \
	F(void, glDeleteTextures, (GLsizei n, const GLuint* textures)) \
	F(void, glDeleteVertexArrays, (GLsizei n, const GLuint* arrays)) \
	F(void, glDepthFunc, (GLenum func)) \
	F(void, glDepthMask, (GLboolean flag)) \
	F(void, glDepthRange, (GLdouble nearValue, GLdouble farValue)) \
	F(void, glDisable, (GLenum cap)) \
	F(void, glDrawArrays, (GLenum mode, GLint first, GLsizei count)) \
	F(void, glDrawBuffers, (GLsizei n, const GLenum* bufs)) \
	F(void, glEnable, (GLenum cap)) \
	F(void, glEnableVertexAttribArray, (GLuint index)) \
	F(void, glFinish, (void)) \
	F(void, glFramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
	F(void, glGenBuffers, (GLsizei n, GLuint* buffers)) \
	F(void, glGenFramebuffers, (GLsizei n, GLuint* framebuffers)) \
	F(void, glGenTextures, (GLsizei n, GLuint* textures)) \
	F(void, glGenVertexArrays, (GLsizei n, GLuint* arrays)) \
	F(GLint, glGetAttribLocation, (GLuint program, const GLchar* name)) \
	F(void, glGetProgramiv, (GLuint program, GLenum pname, GLint* params)) \
	F(void, glGetProgramInfoLog, (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)) \
	F(void, glGetShaderiv, (GLuint shader, GLenum pname, GLint* params)) \
	F(void, glGetShaderInfoLog, (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)) \
	F(const GLubyte*, glGetString, (GLenum name)) \
	F(GLint, glGetUniformLocation, (GLuint program, const GLchar* name)) \
	F(void, glLinkProgram, (GLuint program)) \
	F(void, glPixelStorei, (GLenum pname, GLint param)) \
	F(void, glPolygonMode, (GLenum face, GLenum mode)) \
	F(void, glReadPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* data)) \
	F(void, glScissor, (GLint x, GLint y, GLsizei width, GLsizei height)) \
	F(void, glShaderSource, (GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)) \
	F(void, glStencilFunc, (GLenum func, GLint ref, GLuint mask)) \
	F(void, glStencilMask, (GLuint mask)) \
	F(void, glStencilOp, (GLenum sfail, GLenum dpfail, GLenum dppass)) \
	F(void, glTexImage2D, (GLenum target, GLint level, GLint internalformat, GLsizei width, \
		GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)) \
	F(void, glTexParameteri, (GLenum target, GLenum pname, GLint param)) \
	F(void, glTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, \
		GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)) \
	F(void, glUniform1i, (GLint location, GLint v0)) \
	F(void, glUniform1iv, (GLint location, GLsizei count, const GLint* value)) \
	F(void, glUniform2i, (GLint location, GLint v0, GLint v1)) \
	F(void, glUniform3f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
	F(void, glUniform3i, (GLint location, GLint v0, GLint v1, GLint v2)) \
	F(void, glUniform4i, (GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
	F(void, glUniform4iv, (GLint location, GLsizei count, const GLint* value)) \
	F(void, glUniformMatrix2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)) \
	F(void, glUseProgram, (GLuint program)) \
	F(void, glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, \
		GLsizei stride, const void* pointer)) \
	F(void, glViewport, (GLint x, GLint y, GLsizei width, GLsizei height))

#define M_GBA_GL_DECLARE(ret, name, args) \
	typedef ret (M_GBA_GL_APIENTRY *mGBA##name##Proc) args; \
	extern mGBA##name##Proc mGBA##name;
M_GBA_GL_FUNCTIONS(M_GBA_GL_DECLARE)
#undef M_GBA_GL_DECLARE

#define glActiveTexture mGBAglActiveTexture
#define glAttachShader mGBAglAttachShader
#define glBindBuffer mGBAglBindBuffer
#define glBindSampler mGBAglBindSampler
#define glBindFramebuffer mGBAglBindFramebuffer
#define glBindTexture mGBAglBindTexture
#define glBindVertexArray mGBAglBindVertexArray
#define glBlitFramebuffer mGBAglBlitFramebuffer
#define glBufferData mGBAglBufferData
#define glCheckFramebufferStatus mGBAglCheckFramebufferStatus
#define glClear mGBAglClear
#define glClearBufferiv mGBAglClearBufferiv
#define glClearColor mGBAglClearColor
#define glClearDepth mGBAglClearDepth
#define glClearStencil mGBAglClearStencil
#define glColorMask mGBAglColorMask
#define glCompileShader mGBAglCompileShader
#define glCreateProgram mGBAglCreateProgram
#define glCreateShader mGBAglCreateShader
#define glDeleteBuffers mGBAglDeleteBuffers
#define glDeleteFramebuffers mGBAglDeleteFramebuffers
#define glDeleteProgram mGBAglDeleteProgram
#define glDeleteShader mGBAglDeleteShader
#define glDeleteTextures mGBAglDeleteTextures
#define glDeleteVertexArrays mGBAglDeleteVertexArrays
#define glDepthFunc mGBAglDepthFunc
#define glDepthMask mGBAglDepthMask
#define glDepthRange mGBAglDepthRange
#define glDisable mGBAglDisable
#define glDrawArrays mGBAglDrawArrays
#define glDrawBuffers mGBAglDrawBuffers
#define glEnable mGBAglEnable
#define glEnableVertexAttribArray mGBAglEnableVertexAttribArray
#define glFinish mGBAglFinish
#define glFramebufferTexture2D mGBAglFramebufferTexture2D
#define glGenBuffers mGBAglGenBuffers
#define glGenFramebuffers mGBAglGenFramebuffers
#define glGenTextures mGBAglGenTextures
#define glGenVertexArrays mGBAglGenVertexArrays
#define glGetAttribLocation mGBAglGetAttribLocation
#define glGetProgramiv mGBAglGetProgramiv
#define glGetProgramInfoLog mGBAglGetProgramInfoLog
#define glGetShaderiv mGBAglGetShaderiv
#define glGetShaderInfoLog mGBAglGetShaderInfoLog
#define glGetString mGBAglGetString
#define glGetUniformLocation mGBAglGetUniformLocation
#define glLinkProgram mGBAglLinkProgram
#define glPixelStorei mGBAglPixelStorei
#define glPolygonMode mGBAglPolygonMode
#define glReadPixels mGBAglReadPixels
#define glScissor mGBAglScissor
#define glShaderSource mGBAglShaderSource
#define glStencilFunc mGBAglStencilFunc
#define glStencilMask mGBAglStencilMask
#define glStencilOp mGBAglStencilOp
#define glTexImage2D mGBAglTexImage2D
#define glTexParameteri mGBAglTexParameteri
#define glTexSubImage2D mGBAglTexSubImage2D
#define glUniform1i mGBAglUniform1i
#define glUniform1iv mGBAglUniform1iv
#define glUniform2i mGBAglUniform2i
#define glUniform3f mGBAglUniform3f
#define glUniform3i mGBAglUniform3i
#define glUniform4i mGBAglUniform4i
#define glUniform4iv mGBAglUniform4iv
#define glUniformMatrix2fv mGBAglUniformMatrix2fv
#define glUseProgram mGBAglUseProgram
#define glVertexAttribPointer mGBAglVertexAttribPointer
#define glViewport mGBAglViewport

#undef M_GBA_GL_APIENTRY

#ifdef __cplusplus
}
#endif

#endif
