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
#include <Arduino.h>
#include <MsTimer2.h>

#include "FrameBuffer.h"

/* All the code assumes dimensions are below 127, as signed chars are used to
 * store positions and indices. */
#define MAX_X 4
#define MAX_Y 4
#define MAX_Z 4

#define BRIGHTNESS_LEVELS 16
#define BRIGHTNESS_MAX 256

// Positions (x,y) to pin, same for all layers.
static char position2pin[MAX_X][MAX_Y] = {
  {13, 12, 11, 10},
  { 9,  8,  7,  6},
  { 5,  4,  3,  2},
  { 1,  0, A5, A4}
};

// z-coordinate position to pin.
static char layer[MAX_Z] = { A0, A1, A2, A3 };

static unsigned char font[][4][4] = {
  // a
  {
    {0,0,0,0},  
    {1,1,1,0},  
    {1,0,1,0},  
    {1,1,1,1},  
  },
  // b
  {
    {1,0,0,0},  
    {1,1,1,0},  
    {1,0,1,0},  
    {1,1,1,0},  
  },
  // c
  {
    {0,0,0,0},  
    {1,1,1,0},  
    {1,0,0,0},  
    {1,1,1,0},  
  },
  // d
  {
    {0,0,1,0},  
    {1,1,1,0},  
    {1,0,1,0},  
    {1,1,1,0},  
  },
  // e
  {
    {1,1,1,0},  
    {1,1,1,0},  
    {1,0,0,0},  
    {1,1,1,0},  
  },
  // f
  {
    {1,1,1,0},  
    {1,0,0,0},  
    {1,1,0,0},  
    {1,0,0,0},  
  },
  // g
  {
    {1,1,1,1},  
    {1,0,0,0},  
    {1,0,1,1},  
    {1,1,1,1},  
  },
  // h
  {
    {1,0,0,0},  
    {1,0,0,0},  
    {1,1,1,0},  
    {1,0,1,0},  
  },
  // i
  {
    {0,1,0,0},  
    {0,0,0,0},  
    {0,1,0,0},  
    {0,1,0,0},  
  },
  // j
  {
    {0,0,1,0},  
    {0,0,1,0},  
    {1,0,1,0},  
    {1,1,1,0},  
  },
  // k
  {
    {1,0,0,0},  
    {1,0,1,0},  
    {1,1,0,0},  
    {1,0,1,0},  
  },
  // l
  {
    {0,1,0,0},  
    {0,1,0,0},  
    {0,1,0,0},  
    {0,1,1,0},  
  },
  // M
  {
    {1,0,0,1},  
    {1,1,1,1},  
    {1,0,0,1},  
    {1,0,0,1},  
  },
  // n
  {
    {0,0,0,0},  
    {1,0,1,1},  
    {0,1,0,1},  
    {0,1,0,1},  
  },
  // o
  {
    {0,0,0,0},  
    {1,1,1,0},  
    {1,0,1,0},  
    {1,1,1,0},  
  },
  // p
  {
    {1,1,1,0},  
    {1,0,1,0},  
    {1,1,1,0},  
    {1,0,0,0},  
  },
  // q
  {
    {1,1,1,0},  
    {1,0,1,0},  
    {1,1,1,0},  
    {0,0,1,0},  
  },
  // r
  {
    {1,1,1,0},  
    {1,0,1,0},  
    {1,1,1,0},  
    {1,0,0,1},  
  },
  // s
  {
    {1,1,1,0},  
    {0,1,0,0},  
    {0,0,1,0},  
    {1,1,1,0},  
  },
  // T
  {
    {1,1,1,0},  
    {0,1,0,0},  
    {0,1,0,0},  
    {0,1,0,0},  
  },
  // u
  {
    {0,0,0,0},  
    {1,0,1,0},  
    {1,0,1,0},  
    {1,1,1,0},  
  },
  // v
  {
    {0,0,0,0},  
    {1,0,1,0},  
    {1,0,1,0},  
    {0,1,0,0},  
  },
  // Ww
  {
    {1,0,0,1},  
    {1,0,0,1},  
    {1,1,1,1},  
    {1,0,0,1},  
  },
  // x 
  {
    {0,0,0,0},  
    {1,0,1,0},  
    {1,1,1,0},  
    {1,0,1,0},  
  },
  // y 
  {
    {0,0,0,0},  
    {1,0,1,0},  
    {0,1,0,0},  
    {0,1,0,0},  
  },
  // z
  {
    {1,1,1,1},  
    {0,0,1,0},  
    {0,1,0,0},  
    {1,1,1,1},  
  }

};

#define FRAMEBUFFER_POINTERS 1 /* Either 1 (Enabled) or 0 (disabled) */
#if FRAMEBUFFER_POINTERS
struct Frame frames[2];

struct FrameBuffer {
  unsigned char intensity = 0;
  Frame *front = &frames[0];
  Frame *back = &frames[1];
} FrameBuffer;
#else
struct FrameBuffer {
  unsigned char visible = 0;
  unsigned char intensity = 0;
  Frame frames[2];
} FrameBuffer;
#endif

void FrameBufferSwitch(void) {
#if FRAMEBUFFER_POINTERS
  Frame * t = FrameBuffer.back;
  FrameBuffer.back = FrameBuffer.front;
  FrameBuffer.front = t;
#else
  FrameBuffer.visible = (FrameBuffer.visible + 1) & 0x1;
#endif
}

static Frame *FrameBufferGetFront(void) {
#if FRAMEBUFFER_POINTERS
  return FrameBuffer.front;
#else
  return &FrameBuffer.frames[FrameBuffer.visible];
#endif
}

static Frame *FrameBufferGetBack(void) {
#if FRAMEBUFFER_POINTERS
  return FrameBuffer.back;
#else
  return &FrameBuffer.frames[(FrameBuffer.visible + 1) & 0x1];
#endif
}

void FrameBufferReadFront(unsigned char x, unsigned char y, unsigned char z,
        unsigned char *val) {
  *val = FrameBufferGetFront()->data[x][y][z];
}

void FrameBufferReadBack(unsigned char x, unsigned char y, unsigned char z,
        unsigned char *val) {
  *val = FrameBufferGetBack()->data[x][y][z];
}

void FrameBufferCopy(unsigned char x1, unsigned char y1, unsigned char z1,
        unsigned char x2, unsigned char y2, unsigned char z2) {
  FrameBufferGetBack()->data[x1][y1][z1] =
    FrameBufferGetFront()->data[x2][y2][z2];
}

void FrameBufferWrite(unsigned char x, unsigned char y, unsigned char z,
        unsigned char val) {
  FrameBufferGetBack()->data[x][y][z] = val;
}

void FrameBufferSet(unsigned char val) {
  memset(FrameBufferGetBack(), val, FRAME_SIZE());
}

void FrameBufferBlank(void) {
  FrameBufferSet(0);
}

static void letter(unsigned char l, unsigned char brightness) {
  memset(FrameBufferGetBack(), 0, FRAME_SIZE());
  for (unsigned char x = 0; x < MAX_X; x++) {
    for (unsigned char y = 0; y < MAX_Y; y++) {
      FrameBufferGetBack()->data[3][y][3-x] =
        font[l - 'a'][x][y] * brightness;
    }
  }
  FrameBufferSwitch();
  delay(200);
}

void printAllSymbols(void) {
  for(unsigned char c = 0; c < sizeof(font)/sizeof(font[0]); c++) {
    letter(c+'a', 127);
    delay(500);
  }
}

void FrameBufferWriteStr(char const * str, const short delayPerLetter, 
        const unsigned char brightness) {
  char const *c = str;
  while(*c != 0) {
    if (*c == ' ') {
      memset(FrameBufferGetBack(), 0, FRAME_SIZE());
      FrameBufferSwitch();
    } else {
      letter(*c, brightness);
    }

    delay(delayPerLetter);

    // Make sure there is a white space between font
    memset(FrameBufferGetBack(), 0, FRAME_SIZE());
    FrameBufferSwitch();
    delay(delayPerLetter/4);

    c++;
  }
}


static void FrameBufferRefresh(void) {
#if DEBUG_FB_REFRESH
  static long int timeStartRefresh = micros();
  static long int timeLastRefresh = 0;
#endif

  Frame *f = FrameBufferGetFront();
#if BRIGHTNESS_INCREMENT
  FrameBuffer.intensity = 
    (FrameBuffer.intensity + BRIGHTNESS_INCREMENT) % BRIGHTNESS_MAX;
#else
  FrameBuffer.intensity = 128;
#endif

  for (unsigned char z = 0; z < MAX_Z; z++) {
    // 1. Set up the layer leds to be turned on / off
    for (unsigned char x = 0; x < MAX_X; x++) {
      for (unsigned char y = 0; y < MAX_Y; y++) {
        if (f->data[x][y][z] > FrameBuffer.intensity) {
          digitalWrite(position2pin[x][y], HIGH);
        } else {
          digitalWrite(position2pin[x][y], LOW);
        }
      }
    }
    
    // 2. Turn the power on on the layer
    digitalWrite(layer[z], LOW);

    // Let it display
#if BRIGHTNESS_LEVELS
    delayMicroseconds(FRAME_PERIOD / (MAX_Z * BRIGHTNESS_LEVELS));
#else
    delayMicroseconds(FRAME_PERIOD / MAX_Z);
#endif

    // 3. Turn the power off on the layer
    digitalWrite(layer[z], HIGH);
  }

#if DEBUG_FB_REFRESH
  Serial.print("Time Since last refresh: ");
  Serial.println((timeStartRefresh - timeLastRefresh), DEC);

  timeLastRefresh = micros();

  Serial.print("Period Refresh Time: ");
  Serial.println((timeLastRefresh - timeStartRefresh), DEC);
#endif
}

void FrameBufferInit(void) {
  // Setting rows to ouput
  for (unsigned char x = 0; x < MAX_X; x++) {
    for (unsigned char y = 0; y < MAX_Y; y++) {
      pinMode(position2pin[x][y], OUTPUT);
      digitalWrite(position2pin[x][y], LOW);
    }
  }

  // Setting layers to output
  for (unsigned char z = 0; z < MAX_Z; z++) {
    pinMode(layer[z], OUTPUT);
    digitalWrite(layer[z], HIGH);
  }

  // Initialize FrameBuffers
  memset(FrameBufferGetFront(), 0, FRAME_SIZE());
  memset(FrameBufferGetBack(), 0, FRAME_SIZE());

#if BRIGHTNESS_LEVELS
  MsTimer2::set(FRAME_PERIOD / (1000 * BRIGHTNESS_LEVELS), 
    FrameBufferRefresh);
#else
  MsTimer2::set(FRAME_PERIOD / (1000), FrameBufferRefresh);
#endif
  MsTimer2::start();
}
