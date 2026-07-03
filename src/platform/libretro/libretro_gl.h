/* Copyright (c) 2013-2026 mGBA contributors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef M_LIBRETRO_GL_H
#define M_LIBRETRO_GL_H

#include <stdbool.h>

#include <mgba/internal/gba/renderers/gl-dispatch.h>

#include "libretro.h"

bool mLibretroGLLoad(retro_hw_get_proc_address_t getProcAddress);
void mLibretroGLUnload(void);
const char* mLibretroGLMissingSymbol(void);
void mLibretroGLResetState(void);

#endif
