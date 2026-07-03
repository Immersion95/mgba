/* Copyright (c) 2013-2026 mGBA contributors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef M_LIBRETRO_VIDEO_GL_H
#define M_LIBRETRO_VIDEO_GL_H

#include <stdbool.h>
#include <stdint.h>

#include "libretro.h"

struct mCore;

enum mLibretroVideoGLFrameBlend {
	mLIBRETRO_VIDEO_GL_FRAME_BLEND_NONE = 0,
	mLIBRETRO_VIDEO_GL_FRAME_BLEND_MIX,
	mLIBRETRO_VIDEO_GL_FRAME_BLEND_MIX_SMART,
	mLIBRETRO_VIDEO_GL_FRAME_BLEND_LCD_GHOSTING,
	mLIBRETRO_VIDEO_GL_FRAME_BLEND_LCD_GHOSTING_FAST,
};

struct mLibretroVideoGLPostProcessing {
	bool colorCorrectionEnabled;
	const uint16_t* colorCorrectionLut;
	unsigned colorCorrectionType;
	enum mLibretroVideoGLFrameBlend frameBlend;
};

void mLibretroVideoGLInit(retro_environment_t environment, retro_log_printf_t logCallback);
bool mLibretroVideoGLRequest(struct mCore* core);
void mLibretroVideoGLDeinit(void);

bool mLibretroVideoGLIsRequested(void);
bool mLibretroVideoGLGetSize(const struct mCore* core, unsigned* width, unsigned* height);

void mLibretroVideoGLBeginFrame(void);
bool mLibretroVideoGLPresent(unsigned width, unsigned height,
	const struct mLibretroVideoGLPostProcessing* postProcessing);

#endif
