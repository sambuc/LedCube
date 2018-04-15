#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
/*
 * Copyright (c) 2018, Lionel Adrien Sambuc
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, 
 *     this list of conditions and the following disclaimer.
 * 
 *  2. Redistributions in binary form must reproduce the above copyright 
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its 
 *     contributors may be used to endorse or promote products derived from 
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Either 1 (Enabled) or 0 (disabled) */
#define DEBUG 0
#if DEBUG
#define DEBUG_FB_REFRESH 1
#endif

/* All the code assumes dimensions are below 127, as signed chars are used to
 * store positions and indices. */
#define MAX_X 4
#define MAX_Y 4
#define MAX_Z 4

#define BRIGHTNESS_LEVELS 16
#define BRIGHTNESS_MAX 256

struct Frame {
  unsigned char data[MAX_X][MAX_Y][MAX_Z];
};

extern struct FrameBuffer FrameBuffer;

#define FRAME_SIZE() sizeof(struct Frame)

#if BRIGHTNESS_LEVELS
#define BRIGHTNESS_INCREMENT (BRIGHTNESS_MAX / BRIGHTNESS_LEVELS)
#define FRAME_PERIOD (40000 / BRIGHTNESS_LEVELS) /* [us] */
#else
#define FRAME_PERIOD (40000) /* [us] */
#endif

/**
 * Switch the back and front buffers.
 * 
 */
void FrameBufferSwitch(void);

/**
 * Read the brightness ``val'' of the led at position (x,y,z), in the front
 * buffer.
 */
void FrameBufferReadFront(char x, char y, char z, unsigned char *val);

/**
 * Read the brightness ``val'' of the led at position (x,y,z), in the back
 * buffer.
 */
void FrameBufferReadBack(char x, char y, char z, unsigned char *val);

/**
 * Copy the value from (x2, y2, z2) in the front buffer, to (x1,y1,z1) in
 * the back buffer.
 */
void FrameBufferCopy(char x1, char y1, char z1, char x2, char y2, char z2);

/**
 * Write the brightness ``val'' to the led at position (x,y,z), in the back
 * buffer.
 *
 * In order to make the value visible, a call to FrameBufferSwitch is needed,
 * once the drawing of the frame is complete.
 */
void FrameBufferWrite(char x, char y, char z, unsigned char val);

/**
 * Write the brightness ``val'' to the whole back buffer.
 *
 * In order to make the value visible, a call to FrameBufferSwitch is needed,
 * once the drawing of the frame is complete.
 */
void FrameBufferSet(unsigned char val);

/**
 * Blank the Back Buffer.
 *
 * In order to make the value visible, a call to FrameBufferSwitch is needed,
 * once the drawing of the frame is complete.
 */
void FrameBufferBlank();

/**
 * Print the string, a letter at a time, showing each for delayPerLetter time,
 * with the associated brightness.
 *
 * This has minimal checks, only lowercase font, space and zero to terminate
 * the string.
 */
void FrameBufferWriteStr(char * str, const short delayPerLetter, 
        const unsigned char brightness);

/**
 * Print all symbols available, one after the other.
 */
void printAllSymbols(void);

/** 
 * Draw the front frame on the ``screen''.
 */
void FrameBufferRefresh(void);

/** 
 * Initialize the framebuffer, and start the timer to refresh the ``screen''.
 */
void FrameBufferInit(void);

#endif /* FRAMEBUFFER_H */
