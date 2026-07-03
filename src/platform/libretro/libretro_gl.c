/* Copyright (c) 2013-2026 mGBA contributors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "libretro_gl.h"

#include <string.h>

#define M_GBA_GL_DEFINE(ret, name, args) mGBA##name##Proc mGBA##name = NULL;
M_GBA_GL_FUNCTIONS(M_GBA_GL_DEFINE)
#undef M_GBA_GL_DEFINE

static const char* s_missingSymbol;

bool mLibretroGLLoad(retro_hw_get_proc_address_t getProcAddress) {
	mLibretroGLUnload();
	if (!getProcAddress) {
		s_missingSymbol = "get_proc_address";
		return false;
	}

	s_missingSymbol = NULL;
#define M_GBA_GL_LOAD(ret, name, args) \
	do { \
		retro_proc_address_t proc = getProcAddress(#name); \
		if (!proc || sizeof(mGBA##name) != sizeof(proc)) { \
			s_missingSymbol = #name; \
			mLibretroGLUnload(); \
			return false; \
		} \
		memcpy(&mGBA##name, &proc, sizeof(mGBA##name)); \
	} while (0);
	M_GBA_GL_FUNCTIONS(M_GBA_GL_LOAD)
#undef M_GBA_GL_LOAD
	return true;
}

void mLibretroGLUnload(void) {
#define M_GBA_GL_CLEAR(ret, name, args) mGBA##name = NULL;
	M_GBA_GL_FUNCTIONS(M_GBA_GL_CLEAR)
#undef M_GBA_GL_CLEAR
}

const char* mLibretroGLMissingSymbol(void) {
	return s_missingSymbol;
}

void mLibretroGLResetState(void) {
	unsigned unit;

	glDisable(GL_BLEND);
	glDisable(GL_COLOR_LOGIC_OP);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DITHER);
	glDisable(GL_FRAMEBUFFER_SRGB);
	glDisable(GL_RASTERIZER_DISCARD);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glDepthRange(0.0, 1.0);
	glStencilMask(~0U);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glUseProgram(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	for (unit = 0; unit <= 8; ++unit) {
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindSampler(unit, 0);
	}
	glActiveTexture(GL_TEXTURE0);
	glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_PACK_LSB_FIRST, GL_FALSE);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_PACK_SKIP_IMAGES, 0);
	glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}
