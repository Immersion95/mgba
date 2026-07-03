/* Copyright (c) 2013-2026 mGBA contributors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef M_LIBRETRO_VIDEO_GL_POSTPROCESS_H
#define M_LIBRETRO_VIDEO_GL_POSTPROCESS_H

#include "libretro_gl.h"
#include "libretro_video_gl.h"

void mLibretroVideoGLPostProcessInit(retro_log_printf_t logCallback);
bool mLibretroVideoGLPostProcessCreate(void);
void mLibretroVideoGLPostProcessDestroy(bool contextValid);

bool mLibretroVideoGLPostProcessApply(GLuint targetFramebuffer,
	GLuint sourceFramebuffer, GLuint sourceTexture,
	unsigned width, unsigned height,
	const struct mLibretroVideoGLPostProcessing* settings);

#endif
