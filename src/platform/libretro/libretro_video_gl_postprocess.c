/* Copyright (c) 2013-2026 mGBA contributors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "libretro_video_gl_postprocess.h"

#include <string.h>

#if defined(COLOR_16_BIT) && defined(COLOR_5_6_5)

enum {
	M_LIBRETRO_VIDEO_GL_HISTORY_MAX = 4,
	M_LIBRETRO_VIDEO_GL_FAST_TEXTURES = 2,
	M_LIBRETRO_VIDEO_GL_COLOR_LUT_UNIT = 5,
};

struct mLibretroVideoGLPostProcessor {
	retro_log_printf_t logCallback;
	GLuint program;
	GLuint vao;
	GLint currentLocation;
	GLint historyLocations[M_LIBRETRO_VIDEO_GL_HISTORY_MAX];
	GLint colorLutLocation;
	GLint blendModeLocation;
	GLint colorCorrectionLocation;
	GLuint colorCorrectionTexture;
	unsigned uploadedColorCorrectionType;
	unsigned failedColorCorrectionType;
	GLuint historyTextures[M_LIBRETRO_VIDEO_GL_HISTORY_MAX];
	GLuint historyFramebuffers[M_LIBRETRO_VIDEO_GL_HISTORY_MAX];
	GLuint fastTextures[M_LIBRETRO_VIDEO_GL_FAST_TEXTURES];
	GLuint fastFramebuffers[M_LIBRETRO_VIDEO_GL_FAST_TEXTURES];
	unsigned width;
	unsigned height;
	enum mLibretroVideoGLFrameBlend blendMode;
	unsigned historyCount;
	unsigned fastReadIndex;
	unsigned failedWidth;
	unsigned failedHeight;
	enum mLibretroVideoGLFrameBlend failedBlendMode;
	bool programReady;
	bool resourcesReady;
	bool resourceFailure;
	bool failureLogged;
	bool colorLutFailure;
	bool colorLutFailureLogged;
};

static struct mLibretroVideoGLPostProcessor s_post;

static void _resetUniformLocations(void) {
	s_post.currentLocation = -1;
	memset(s_post.historyLocations, 0xFF, sizeof(s_post.historyLocations));
	s_post.colorLutLocation = -1;
	s_post.blendModeLocation = -1;
	s_post.colorCorrectionLocation = -1;
}

void mLibretroVideoGLPostProcessInit(retro_log_printf_t logCallback) {
	memset(&s_post, 0, sizeof(s_post));
	s_post.logCallback = logCallback;
	s_post.blendMode = mLIBRETRO_VIDEO_GL_FRAME_BLEND_NONE;
	s_post.failedBlendMode = mLIBRETRO_VIDEO_GL_FRAME_BLEND_NONE;
	_resetUniformLocations();
}

/* The shader blendMode values intentionally mirror mLibretroVideoGLFrameBlend. */
static const char* const _postVertexShader =
	"#version 330 core\n"
	"out vec2 texCoord;\n"
	"void main() {\n"
	"\tvec2 position = vec2(float((gl_VertexID << 1) & 2), float(gl_VertexID & 2));\n"
	"\ttexCoord = position;\n"
	"\tgl_Position = vec4(position * 2.0 - 1.0, 0.0, 1.0);\n"
	"}\n";

static const char* const _postFragmentShader =
	"#version 330 core\n"
	"in vec2 texCoord;\n"
	"layout(location = 0) out vec4 fragColor;\n"
	"uniform sampler2D currentTex;\n"
	"uniform sampler2D history1;\n"
	"uniform sampler2D history2;\n"
	"uniform sampler2D history3;\n"
	"uniform sampler2D history4;\n"
	"uniform sampler2D colorLut;\n"
	"uniform int blendMode;\n"
	"uniform int colorCorrection;\n"
	"ivec3 mixColor5(ivec3 current, ivec3 previous) {\n"
	"\tivec3 total = current + previous;\n"
	"\treturn ivec3((total.r + 1) / 2, total.g / 2, (total.b + 1) / 2);\n"
	"}\n"
	"vec3 displayColor(ivec3 color5) {\n"
	"\treturn vec3(float(color5.r) / 31.0, float(color5.g * 2) / 63.0,\n"
	"\t\tfloat(color5.b) / 31.0);\n"
	"}\n"
	"vec3 mixDisplayColor(ivec3 current, ivec3 previous) {\n"
	"\tivec3 total = current + previous;\n"
	"\treturn vec3(float((total.r + 1) / 2) / 31.0, float(total.g) / 63.0,\n"
	"\t\tfloat((total.b + 1) / 2) / 31.0);\n"
	"}\n"
	"vec3 correctColor(ivec3 color5) {\n"
	"\tint index = (color5.r << 11) | (color5.g << 6) | color5.b;\n"
	"\treturn texelFetch(colorLut, ivec2(index & 255, index >> 8), 0).rgb;\n"
	"}\n"
	"void main() {\n"
	"\tvec3 currentRaw = clamp(texture(currentTex, texCoord).rgb, 0.0, 1.0);\n"
	"\tvec3 historyRaw1 = clamp(texture(history1, texCoord).rgb, 0.0, 1.0);\n"
	"\tivec3 current5 = ivec3(floor(currentRaw * 31.0 + 0.5));\n"
	"\tivec3 previous1 = ivec3(floor(historyRaw1 * 31.0 + 0.5));\n"
	"\tivec3 previous2 = ivec3(floor(clamp(texture(history2, texCoord).rgb, 0.0, 1.0) * 31.0 + 0.5));\n"
	"\tivec3 previous3 = ivec3(floor(clamp(texture(history3, texCoord).rgb, 0.0, 1.0) * 31.0 + 0.5));\n"
	"\tivec3 previous4 = ivec3(floor(clamp(texture(history4, texCoord).rgb, 0.0, 1.0) * 31.0 + 0.5));\n"
	"\tivec3 correctionColor5 = current5;\n"
	"\tvec3 outputColor = displayColor(current5);\n"
	"\tif (blendMode == 1) {\n"
	"\t\tcorrectionColor5 = mixColor5(current5, previous1);\n"
	"\t\toutputColor = mixDisplayColor(current5, previous1);\n"
	"\t} else if (blendMode == 2) {\n"
	"\t\tbool alternating = (all(equal(current5, previous2)) || all(equal(previous1, previous3))) &&\n"
	"\t\t\t!all(equal(current5, previous1)) && !all(equal(current5, previous3)) &&\n"
	"\t\t\t!all(equal(previous1, previous2));\n"
	"\t\tif (alternating) {\n"
	"\t\t\tcorrectionColor5 = mixColor5(current5, previous1);\n"
	"\t\t\toutputColor = mixDisplayColor(current5, previous1);\n"
	"\t\t}\n"
	"\t} else if (blendMode == 3) {\n"
	"\t\tvec3 ghost = vec3(current5);\n"
	"\t\tghost += (vec3(previous1) - ghost) * 0.333;\n"
	"\t\tghost += (vec3(previous2) - ghost) * 0.110889;\n"
	"\t\tghost += (vec3(previous3) - ghost) * 0.036926037;\n"
	"\t\tghost += (vec3(previous4) - ghost) * 0.012296970321;\n"
	"\t\tcorrectionColor5 = ivec3(floor(ghost + 0.5));\n"
	"\t\toutputColor = displayColor(correctionColor5);\n"
	"\t} else if (blendMode == 4) {\n"
	"\t\toutputColor = (vec3(current5) + historyRaw1 * 31.0) * (0.5 / 31.0);\n"
	"\t}\n"
	"\tif (colorCorrection != 0) {\n"
	"\t\toutputColor = correctColor(correctionColor5);\n"
	"\t}\n"
	"\tfragColor = vec4(clamp(outputColor, 0.0, 1.0), 1.0);\n"
	"}\n";


static GLuint _compileShader(GLenum type, const char* source) {
	GLuint shader = glCreateShader(type);
	GLint status = GL_FALSE;
	GLchar log[2048] = {0};

	if (!shader) {
		return 0;
	}
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE) {
		return shader;
	}

	glGetShaderInfoLog(shader, sizeof(log), NULL, log);
	if (s_post.logCallback) {
		s_post.logCallback(RETRO_LOG_ERROR,
			"OpenGL post-processing shader compilation failed: %s\n", log);
	}
	glDeleteShader(shader);
	return 0;
}

static bool _uniformLocationsValid(void) {
	unsigned i;

	if (s_post.currentLocation < 0 || s_post.colorLutLocation < 0 ||
			s_post.blendModeLocation < 0 || s_post.colorCorrectionLocation < 0) {
		return false;
	}
	for (i = 0; i < M_LIBRETRO_VIDEO_GL_HISTORY_MAX; ++i) {
		if (s_post.historyLocations[i] < 0) {
			return false;
		}
	}
	return true;
}

bool mLibretroVideoGLPostProcessCreate(void) {
	GLuint vertexShader;
	GLuint fragmentShader;
	GLint status = GL_FALSE;
	GLchar log[2048] = {0};
	unsigned i;

	if (s_post.programReady) {
		return true;
	}

	vertexShader = _compileShader(GL_VERTEX_SHADER, _postVertexShader);
	if (!vertexShader) {
		return false;
	}
	fragmentShader = _compileShader(GL_FRAGMENT_SHADER, _postFragmentShader);
	if (!fragmentShader) {
		glDeleteShader(vertexShader);
		return false;
	}

	s_post.program = glCreateProgram();
	if (!s_post.program) {
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return false;
	}
	glAttachShader(s_post.program, vertexShader);
	glAttachShader(s_post.program, fragmentShader);
	glLinkProgram(s_post.program);
	glGetProgramiv(s_post.program, GL_LINK_STATUS, &status);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	if (status != GL_TRUE) {
		glGetProgramInfoLog(s_post.program, sizeof(log), NULL, log);
		if (s_post.logCallback) {
			s_post.logCallback(RETRO_LOG_ERROR,
				"OpenGL post-processing shader link failed: %s\n", log);
		}
		glDeleteProgram(s_post.program);
		s_post.program = 0;
		return false;
	}

	glGenVertexArrays(1, &s_post.vao);
	if (!s_post.vao) {
		glDeleteProgram(s_post.program);
		s_post.program = 0;
		return false;
	}

	s_post.currentLocation = glGetUniformLocation(s_post.program, "currentTex");
	s_post.historyLocations[0] = glGetUniformLocation(s_post.program, "history1");
	s_post.historyLocations[1] = glGetUniformLocation(s_post.program, "history2");
	s_post.historyLocations[2] = glGetUniformLocation(s_post.program, "history3");
	s_post.historyLocations[3] = glGetUniformLocation(s_post.program, "history4");
	s_post.colorLutLocation = glGetUniformLocation(s_post.program, "colorLut");
	s_post.blendModeLocation = glGetUniformLocation(s_post.program, "blendMode");
	s_post.colorCorrectionLocation = glGetUniformLocation(s_post.program, "colorCorrection");
	if (!_uniformLocationsValid()) {
		if (s_post.logCallback) {
			s_post.logCallback(RETRO_LOG_ERROR,
				"OpenGL post-processing shader is missing a required uniform.\n");
		}
		glDeleteVertexArrays(1, &s_post.vao);
		glDeleteProgram(s_post.program);
		s_post.vao = 0;
		s_post.program = 0;
		_resetUniformLocations();
		return false;
	}

	glUseProgram(s_post.program);
	glUniform1i(s_post.currentLocation, 0);
	for (i = 0; i < M_LIBRETRO_VIDEO_GL_HISTORY_MAX; ++i) {
		glUniform1i(s_post.historyLocations[i], (GLint) i + 1);
	}
	glUniform1i(s_post.colorLutLocation, M_LIBRETRO_VIDEO_GL_COLOR_LUT_UNIT);
	glUseProgram(0);
	s_post.programReady = true;
	return true;
}

static void _destroyResources(bool contextValid) {
	if (contextValid) {
		glDeleteTextures(M_LIBRETRO_VIDEO_GL_HISTORY_MAX, s_post.historyTextures);
		glDeleteFramebuffers(M_LIBRETRO_VIDEO_GL_HISTORY_MAX, s_post.historyFramebuffers);
		glDeleteTextures(M_LIBRETRO_VIDEO_GL_FAST_TEXTURES, s_post.fastTextures);
		glDeleteFramebuffers(M_LIBRETRO_VIDEO_GL_FAST_TEXTURES, s_post.fastFramebuffers);
	}
	memset(s_post.historyTextures, 0, sizeof(s_post.historyTextures));
	memset(s_post.historyFramebuffers, 0, sizeof(s_post.historyFramebuffers));
	memset(s_post.fastTextures, 0, sizeof(s_post.fastTextures));
	memset(s_post.fastFramebuffers, 0, sizeof(s_post.fastFramebuffers));
	s_post.width = 0;
	s_post.height = 0;
	s_post.blendMode = mLIBRETRO_VIDEO_GL_FRAME_BLEND_NONE;
	s_post.historyCount = 0;
	s_post.fastReadIndex = 0;
	s_post.resourcesReady = false;
}

void mLibretroVideoGLPostProcessDestroy(bool contextValid) {
	retro_log_printf_t logCallback = s_post.logCallback;

	_destroyResources(contextValid);
	if (contextValid) {
		if (s_post.colorCorrectionTexture) {
			glDeleteTextures(1, &s_post.colorCorrectionTexture);
		}
		if (s_post.vao) {
			glDeleteVertexArrays(1, &s_post.vao);
		}
		if (s_post.program) {
			glDeleteProgram(s_post.program);
		}
	}
	memset(&s_post, 0, sizeof(s_post));
	s_post.logCallback = logCallback;
	s_post.blendMode = mLIBRETRO_VIDEO_GL_FRAME_BLEND_NONE;
	s_post.failedBlendMode = mLIBRETRO_VIDEO_GL_FRAME_BLEND_NONE;
	_resetUniformLocations();
}

static bool _createTexture(GLuint* texture, GLuint* framebuffer,
		GLenum internalFormat, GLenum type, unsigned width, unsigned height,
		float clearValue) {
	GLenum drawBuffer = GL_COLOR_ATTACHMENT0;

	*texture = 0;
	*framebuffer = 0;
	glGenTextures(1, texture);
	if (!*texture) {
		return false;
	}
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, (GLint) internalFormat,
		(GLsizei) width, (GLsizei) height, 0, GL_RGB, type, NULL);

	glGenFramebuffers(1, framebuffer);
	if (!*framebuffer) {
		glDeleteTextures(1, texture);
		*texture = 0;
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, *texture, 0);
	glDrawBuffers(1, &drawBuffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		glDeleteFramebuffers(1, framebuffer);
		glDeleteTextures(1, texture);
		*framebuffer = 0;
		*texture = 0;
		return false;
	}

	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	glClearColor(clearValue, clearValue, clearValue, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	return true;
}

static unsigned _historyCount(enum mLibretroVideoGLFrameBlend blendMode) {
	switch (blendMode) {
	case mLIBRETRO_VIDEO_GL_FRAME_BLEND_MIX:
		return 1;
	case mLIBRETRO_VIDEO_GL_FRAME_BLEND_MIX_SMART:
		return 3;
	case mLIBRETRO_VIDEO_GL_FRAME_BLEND_LCD_GHOSTING:
		return 4;
	default:
		return 0;
	}
}

static bool _sameFailedConfiguration(unsigned width, unsigned height,
		enum mLibretroVideoGLFrameBlend blendMode) {
	return s_post.resourceFailure && s_post.failedWidth == width &&
		s_post.failedHeight == height && s_post.failedBlendMode == blendMode;
}

static bool _configureResources(unsigned width, unsigned height,
		enum mLibretroVideoGLFrameBlend blendMode) {
	unsigned i;
	unsigned historyCount;

	if (s_post.resourcesReady && s_post.width == width &&
			s_post.height == height && s_post.blendMode == blendMode) {
		return true;
	}
	if (_sameFailedConfiguration(width, height, blendMode)) {
		return false;
	}

	_destroyResources(true);
	s_post.resourceFailure = false;
	s_post.width = width;
	s_post.height = height;
	s_post.blendMode = blendMode;
	historyCount = _historyCount(blendMode);
	s_post.historyCount = historyCount;

	for (i = 0; i < historyCount; ++i) {
		if (!_createTexture(&s_post.historyTextures[i],
				&s_post.historyFramebuffers[i], GL_RGB565,
				GL_UNSIGNED_SHORT_5_6_5, width, height, 1.0f)) {
			goto fail;
		}
	}
	if (blendMode == mLIBRETRO_VIDEO_GL_FRAME_BLEND_LCD_GHOSTING_FAST) {
		for (i = 0; i < M_LIBRETRO_VIDEO_GL_FAST_TEXTURES; ++i) {
			if (!_createTexture(&s_post.fastTextures[i],
					&s_post.fastFramebuffers[i], GL_RGB16F,
					GL_FLOAT, width, height, 1.0f / 31.0f)) {
				goto fail;
			}
		}
	}

	s_post.resourcesReady = true;
	return true;

fail:
	_destroyResources(true);
	s_post.resourceFailure = true;
	s_post.failedWidth = width;
	s_post.failedHeight = height;
	s_post.failedBlendMode = blendMode;
	return false;
}

static bool _uploadColorCorrectionLut(const uint16_t* colorLut,
		unsigned colorCorrectionType) {
	if (!colorLut) {
		return false;
	}
	if (s_post.colorCorrectionTexture &&
			s_post.uploadedColorCorrectionType == colorCorrectionType) {
		return true;
	}
	if (s_post.colorLutFailure &&
			s_post.failedColorCorrectionType == colorCorrectionType) {
		return false;
	}

	glActiveTexture(GL_TEXTURE0 + M_LIBRETRO_VIDEO_GL_COLOR_LUT_UNIT);
	if (!s_post.colorCorrectionTexture) {
		glGenTextures(1, &s_post.colorCorrectionTexture);
		if (!s_post.colorCorrectionTexture) {
			s_post.colorLutFailure = true;
			s_post.failedColorCorrectionType = colorCorrectionType;
			return false;
		}
		glBindTexture(GL_TEXTURE_2D, s_post.colorCorrectionTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	} else {
		glBindTexture(GL_TEXTURE_2D, s_post.colorCorrectionTexture);
	}

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB565, 256, 256, 0,
		GL_RGB, GL_UNSIGNED_SHORT_5_6_5, colorLut);
	s_post.uploadedColorCorrectionType = colorCorrectionType;
	s_post.colorLutFailure = false;
	return true;
}

static void _draw(GLuint targetFramebuffer, bool privateFramebuffer,
		GLuint currentTexture, const GLuint* historyTextures,
		enum mLibretroVideoGLFrameBlend blendMode, bool colorCorrection,
		unsigned width, unsigned height) {
	GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
	unsigned i;

	mLibretroGLResetState();
	glBindFramebuffer(GL_FRAMEBUFFER, targetFramebuffer);
	if (privateFramebuffer) {
		glDrawBuffers(1, &drawBuffer);
	}
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	glUseProgram(s_post.program);
	glBindVertexArray(s_post.vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, currentTexture);
	for (i = 0; i < M_LIBRETRO_VIDEO_GL_HISTORY_MAX; ++i) {
		GLuint historyTexture = historyTextures && historyTextures[i]
			? historyTextures[i] : currentTexture;
		glActiveTexture(GL_TEXTURE0 + i + 1);
		glBindTexture(GL_TEXTURE_2D, historyTexture);
	}
	glActiveTexture(GL_TEXTURE0 + M_LIBRETRO_VIDEO_GL_COLOR_LUT_UNIT);
	glBindTexture(GL_TEXTURE_2D,
		colorCorrection ? s_post.colorCorrectionTexture : currentTexture);
	glUniform1i(s_post.blendModeLocation, (GLint) blendMode);
	glUniform1i(s_post.colorCorrectionLocation, colorCorrection ? 1 : 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

static void _updateHistory(GLuint sourceFramebuffer,
		unsigned width, unsigned height) {
	unsigned i;

	for (i = s_post.historyCount; i > 1; --i) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER,
			s_post.historyFramebuffers[i - 2]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
			s_post.historyFramebuffers[i - 1]);
		glBlitFramebuffer(0, 0, (GLint) width, (GLint) height,
			0, 0, (GLint) width, (GLint) height,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
	if (s_post.historyCount) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFramebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s_post.historyFramebuffers[0]);
		glBlitFramebuffer(0, 0, (GLint) width, (GLint) height,
			0, 0, (GLint) width, (GLint) height,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
}

bool mLibretroVideoGLPostProcessApply(GLuint targetFramebuffer,
		GLuint sourceFramebuffer, GLuint sourceTexture,
		unsigned width, unsigned height,
		const struct mLibretroVideoGLPostProcessing* settings) {
	enum mLibretroVideoGLFrameBlend blendMode;
	bool colorCorrection;
	bool requested;

	if (!settings || !s_post.programReady) {
		return false;
	}

	blendMode = settings->frameBlend;
	if (!settings->colorCorrectionEnabled) {
		s_post.colorLutFailure = false;
		s_post.colorLutFailureLogged = false;
	}
	colorCorrection = settings->colorCorrectionEnabled &&
		_uploadColorCorrectionLut(settings->colorCorrectionLut,
			settings->colorCorrectionType);
	if (settings->colorCorrectionEnabled && !colorCorrection) {
		if (!s_post.colorLutFailureLogged && s_post.logCallback) {
			s_post.logCallback(RETRO_LOG_WARN,
				"OpenGL color-correction LUT could not be uploaded; color correction is disabled for this frame.\n");
		}
		s_post.colorLutFailureLogged = true;
	} else {
		s_post.colorLutFailureLogged = false;
	}
	requested = blendMode != mLIBRETRO_VIDEO_GL_FRAME_BLEND_NONE ||
		colorCorrection;
	if (!requested) {
		if (s_post.resourcesReady) {
			_destroyResources(true);
		}
		s_post.resourceFailure = false;
		s_post.failureLogged = false;
		return false;
	}

	if (!_configureResources(width, height, blendMode)) {
		if (!s_post.failureLogged && s_post.logCallback) {
			s_post.logCallback(RETRO_LOG_WARN,
				"OpenGL post-processing resources could not be created; "
				"displaying the unprocessed high-resolution frame.\n");
		}
		s_post.failureLogged = true;
		return false;
	}
	s_post.failureLogged = false;

	if (blendMode == mLIBRETRO_VIDEO_GL_FRAME_BLEND_LCD_GHOSTING_FAST) {
		unsigned writeIndex = s_post.fastReadIndex ^ 1U;
		GLuint fastHistory[M_LIBRETRO_VIDEO_GL_HISTORY_MAX] = {
			s_post.fastTextures[s_post.fastReadIndex], 0, 0, 0
		};

		_draw(s_post.fastFramebuffers[writeIndex], true, sourceTexture,
			fastHistory, mLIBRETRO_VIDEO_GL_FRAME_BLEND_LCD_GHOSTING_FAST,
			false, width, height);
		_draw(targetFramebuffer, false, s_post.fastTextures[writeIndex],
			NULL, mLIBRETRO_VIDEO_GL_FRAME_BLEND_NONE,
			colorCorrection, width, height);
		s_post.fastReadIndex = writeIndex;
	} else {
		_draw(targetFramebuffer, false, sourceTexture, s_post.historyTextures,
			blendMode, colorCorrection, width, height);
		_updateHistory(sourceFramebuffer, width, height);
	}
	return true;
}

#else

void mLibretroVideoGLPostProcessInit(retro_log_printf_t logCallback) {
	(void) logCallback;
}

bool mLibretroVideoGLPostProcessCreate(void) {
	return false;
}

void mLibretroVideoGLPostProcessDestroy(bool contextValid) {
	(void) contextValid;
}

bool mLibretroVideoGLPostProcessApply(GLuint targetFramebuffer,
		GLuint sourceFramebuffer, GLuint sourceTexture,
		unsigned width, unsigned height,
		const struct mLibretroVideoGLPostProcessing* settings) {
	(void) targetFramebuffer;
	(void) sourceFramebuffer;
	(void) sourceTexture;
	(void) width;
	(void) height;
	(void) settings;
	return false;
}

#endif
