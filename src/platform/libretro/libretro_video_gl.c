/* Copyright (c) 2013-2026 mGBA contributors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "libretro_video_gl.h"

#include <mgba/core/core.h>
#include <mgba/gba/core.h>

#include <stdlib.h>
#include <string.h>

#include "libretro_gl.h"
#include "libretro_video_gl_postprocess.h"

struct mLibretroVideoGLState {
	retro_environment_t environment;
	retro_log_printf_t logCallback;
	struct mCore* core;
	struct retro_hw_render_callback callback;
	GLuint outputTexture;
	GLuint outputFramebuffer;
	unsigned scale;
	bool requested;
	bool dispatchLoaded;
	bool contextReady;
	bool rendererEnabled;
};

static struct mLibretroVideoGLState s_video;

static void _contextReset(void);
static void _contextDestroy(void);

static unsigned _readScale(void) {
	struct retro_variable variable = {
		.key = "mgba_opengl_scale",
		.value = NULL,
	};
	long scale;

	if (!s_video.environment ||
			!s_video.environment(RETRO_ENVIRONMENT_GET_VARIABLE, &variable) ||
			!variable.value) {
		return 1;
	}

	scale = strtol(variable.value, NULL, 10);
	return scale >= 1 && scale <= 16 ? (unsigned) scale : 1;
}

static void _releaseContext(bool contextValid) {
	mLibretroVideoGLPostProcessDestroy(contextValid && s_video.dispatchLoaded);

	if (s_video.core && s_video.rendererEnabled &&
			s_video.core->platform(s_video.core) == mPLATFORM_GBA) {
		if (contextValid) {
			mCoreConfigSetIntValue(&s_video.core->config, "hwaccelVideo", 0);
			s_video.core->reloadConfigOption(s_video.core, "hwaccelVideo", NULL);
		} else {
			GBACoreInvalidateVideoGLContext(s_video.core);
		}
		s_video.core->setVideoGLTex(s_video.core, (unsigned) -1);
	}

	if (contextValid && s_video.dispatchLoaded) {
		if (s_video.outputFramebuffer) {
			glDeleteFramebuffers(1, &s_video.outputFramebuffer);
		}
		if (s_video.outputTexture) {
			glDeleteTextures(1, &s_video.outputTexture);
		}
	}

	s_video.outputFramebuffer = 0;
	s_video.outputTexture = 0;
	s_video.contextReady = false;
	s_video.rendererEnabled = false;
	if (s_video.dispatchLoaded) {
		mLibretroGLUnload();
		s_video.dispatchLoaded = false;
	}
}

static void _contextReset(void) {
	GLuint frontendFramebuffer;
	const GLubyte* version;

	if (!s_video.requested || !s_video.core ||
			s_video.core->platform(s_video.core) != mPLATFORM_GBA) {
		return;
	}

	/* A frontend may replace a context without delivering context_destroy.
	 * Old object names must then be abandoned, not deleted in the new context. */
	if (s_video.contextReady || s_video.rendererEnabled || s_video.dispatchLoaded) {
		_releaseContext(false);
	}

	if (!mLibretroGLLoad(s_video.callback.get_proc_address)) {
		if (s_video.logCallback) {
			s_video.logCallback(RETRO_LOG_ERROR,
				"OpenGL high-resolution renderer unavailable: missing GL entry point '%s'. "
				"Restart content with the software renderer.\n",
				mLibretroGLMissingSymbol() ? mLibretroGLMissingSymbol() : "unknown");
		}
		return;
	}
	s_video.dispatchLoaded = true;

	if (!s_video.callback.get_current_framebuffer) {
		if (s_video.logCallback) {
			s_video.logCallback(RETRO_LOG_ERROR,
				"OpenGL high-resolution renderer unavailable: frontend did not provide get_current_framebuffer.\n");
		}
		_releaseContext(false);
		return;
	}

	mLibretroGLResetState();
	glGenTextures(1, &s_video.outputTexture);
	if (!s_video.outputTexture) {
		if (s_video.logCallback) {
			s_video.logCallback(RETRO_LOG_ERROR,
				"OpenGL high-resolution renderer could not create its output texture.\n");
		}
		_releaseContext(true);
		return;
	}

	s_video.core->setVideoGLTex(s_video.core, s_video.outputTexture);
	mCoreConfigSetIntValue(&s_video.core->config, "videoScale", (int) s_video.scale);
	mCoreConfigSetIntValue(&s_video.core->config, "hwaccelVideo", 1);
	s_video.core->reloadConfigOption(s_video.core, "hwaccelVideo", NULL);
	s_video.rendererEnabled = true;

	glGenFramebuffers(1, &s_video.outputFramebuffer);
	if (!s_video.outputFramebuffer) {
		if (s_video.logCallback) {
			s_video.logCallback(RETRO_LOG_ERROR,
				"OpenGL high-resolution renderer could not create its output framebuffer.\n");
		}
		_releaseContext(true);
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, s_video.outputFramebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, s_video.outputTexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		if (s_video.logCallback) {
			s_video.logCallback(RETRO_LOG_ERROR,
				"OpenGL high-resolution renderer produced an incomplete output framebuffer.\n");
		}
		_releaseContext(true);
		return;
	}

	if (!mLibretroVideoGLPostProcessCreate() && s_video.logCallback) {
		s_video.logCallback(RETRO_LOG_WARN,
			"OpenGL color correction and interframe blending are unavailable; "
			"high-resolution rendering will continue without post-processing.\n");
	}

	s_video.contextReady = true;
	frontendFramebuffer = (GLuint) s_video.callback.get_current_framebuffer();
	mLibretroGLResetState();
	glBindFramebuffer(GL_FRAMEBUFFER, frontendFramebuffer);

	version = glGetString(GL_VERSION);
	if (s_video.logCallback) {
		s_video.logCallback(RETRO_LOG_INFO,
			"mGBA OpenGL high-resolution renderer enabled at %ux (%s).\n",
			s_video.scale, version ? (const char*) version : "unknown GL version");
	}
}

static void _contextDestroy(void) {
	_releaseContext(true);
}

void mLibretroVideoGLInit(retro_environment_t environment, retro_log_printf_t logCallback) {
	memset(&s_video, 0, sizeof(s_video));
	s_video.environment = environment;
	s_video.logCallback = logCallback;
	s_video.scale = 1;
	mLibretroVideoGLPostProcessInit(logCallback);
}

bool mLibretroVideoGLRequest(struct mCore* core) {
	struct retro_variable variable = {
		.key = "mgba_renderer",
		.value = NULL,
	};

	if (s_video.requested || s_video.contextReady || s_video.dispatchLoaded) {
		mLibretroVideoGLDeinit();
	}
	s_video.core = core;

	if (!s_video.environment || !core || core->platform(core) != mPLATFORM_GBA ||
			!s_video.environment(RETRO_ENVIRONMENT_GET_VARIABLE, &variable) ||
			!variable.value || strcmp(variable.value, "opengl") != 0) {
		s_video.core = NULL;
		return false;
	}

	s_video.scale = _readScale();
	memset(&s_video.callback, 0, sizeof(s_video.callback));
	s_video.callback.context_type = RETRO_HW_CONTEXT_OPENGL_CORE;
	s_video.callback.context_reset = _contextReset;
	s_video.callback.context_destroy = _contextDestroy;
	s_video.callback.bottom_left_origin = false;
	s_video.callback.version_major = 3;
	s_video.callback.version_minor = 3;
	s_video.callback.cache_context = false;

	/* Set requested before SET_HW_RENDER: frontends may invoke context_reset
	 * synchronously while processing the environment callback. */
	s_video.requested = true;
	if (!s_video.environment(RETRO_ENVIRONMENT_SET_HW_RENDER, &s_video.callback)) {
		s_video.requested = false;
		s_video.core = NULL;
		if (s_video.logCallback) {
			s_video.logCallback(RETRO_LOG_WARN,
				"OpenGL high-resolution renderer unavailable: frontend rejected an "
				"OpenGL 3.3 core context. Using software rendering.\n");
		}
		return false;
	}
	return true;
}

void mLibretroVideoGLDeinit(void) {
	_releaseContext(true);
	s_video.requested = false;
	s_video.core = NULL;
	s_video.scale = 1;
	memset(&s_video.callback, 0, sizeof(s_video.callback));
}

bool mLibretroVideoGLIsRequested(void) {
	return s_video.requested;
}

bool mLibretroVideoGLGetSize(const struct mCore* core, unsigned* width, unsigned* height) {
	if (!s_video.requested || !core || core->platform(core) != mPLATFORM_GBA ||
			!width || !height) {
		return false;
	}
	core->baseVideoSize(core, width, height);
	*width *= s_video.scale;
	*height *= s_video.scale;
	return true;
}

void mLibretroVideoGLBeginFrame(void) {
	if (s_video.contextReady && s_video.rendererEnabled) {
		mLibretroGLResetState();
	}
}

bool mLibretroVideoGLPresent(unsigned width, unsigned height,
		const struct mLibretroVideoGLPostProcessing* postProcessing) {
	GLuint frontendFramebuffer;

	if (!s_video.contextReady || !s_video.rendererEnabled ||
			!s_video.outputFramebuffer || !s_video.callback.get_current_framebuffer) {
		return false;
	}

	frontendFramebuffer = (GLuint) s_video.callback.get_current_framebuffer();
	if (!mLibretroVideoGLPostProcessApply(frontendFramebuffer,
			s_video.outputFramebuffer, s_video.outputTexture,
			width, height, postProcessing)) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, s_video.outputFramebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frontendFramebuffer);
		glBlitFramebuffer(0, 0, (GLint) width, (GLint) height,
			0, 0, (GLint) width, (GLint) height,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	mLibretroGLResetState();
	glBindFramebuffer(GL_FRAMEBUFFER, frontendFramebuffer);
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	return true;
}
