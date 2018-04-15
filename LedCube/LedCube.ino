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

/**
 * This program was created from a mix of several sources for the inspiration
 * on how to implement the system. I especially followed:
 *   http://www.instructables.com/id/4x4x4-LED-Cube-Arduino-Uno/ for the 
 * construction of the hardware.
 * 
 * The main idea is to keep two memory frame (3d) buffer. In order to refresh
 * those we use a handler triggered through a timer, set for a period of 
 * ``FRAME_PERIOD''. Every period the whole cube needs to be redrawn, but as 
 * we have multiplexing, this means that each layer has only a fourth of the 
 * frame period to be turned on for the maximum brightness.  In order to 
 * provide multiple brightness levels, a PWM-like signal is manually 
 * generated, per led, per layer, by turning it on for a percentage of the 
 * layer time only.
 */

/* Either 1 (Enabled) or 0 (disabled) */
#define DEBUG 0 
#if DEBUG
#define DEBUG_SETUP 1
#define DEBUG_LOOP 1
#endif

#include "FrameBuffer.h"


/******************************* ANIMATIONS ********************************/

/* Taken from PWMallPins.pde by Paul Badger, 2007;
 * formatted by Seth Wolf, 2015 */
// Preset 256 values of a binary conversions of a sine wave 
unsigned char sinewave[] = {
  0x80,0x83,0x86,0x89,0x8c,0x8f,0x92,0x95,0x98,0x9c,0x9f,0xa2,0xa5,0xa8,0xab,0xae,
  0xb0,0xb3,0xb6,0xb9,0xbc,0xbf,0xc1,0xc4,0xc7,0xc9,0xcc,0xce,0xd1,0xd3,0xd5,0xd8,
  0xda,0xdc,0xde,0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xed,0xef,0xf0,0xf2,0xf3,0xf5,
  0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfc,0xfd,0xfe,0xfe,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0xfe,0xfd,0xfc,0xfc,0xfb,0xfa,0xf9,0xf8,0xf7,
  0xf6,0xf5,0xf3,0xf2,0xf0,0xef,0xed,0xec,0xea,0xe8,0xe6,0xe4,0xe2,0xe0,0xde,0xdc,
  0xda,0xd8,0xd5,0xd3,0xd1,0xce,0xcc,0xc9,0xc7,0xc4,0xc1,0xbf,0xbc,0xb9,0xb6,0xb3,
  0xb0,0xae,0xab,0xa8,0xa5,0xa2,0x9f,0x9c,0x98,0x95,0x92,0x8f,0x8c,0x89,0x86,0x83,
  0x80,0x7c,0x79,0x76,0x73,0x70,0x6d,0x6a,0x67,0x63,0x60,0x5d,0x5a,0x57,0x54,0x51,
  0x4f,0x4c,0x49,0x46,0x43,0x40,0x3e,0x3b,0x38,0x36,0x33,0x31,0x2e,0x2c,0x2a,0x27,
  0x25,0x23,0x21,0x1f,0x1d,0x1b,0x19,0x17,0x15,0x13,0x12,0x10,0x0f,0x0d,0x0c,0x0a,
  0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x03,0x02,0x01,0x01,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x03,0x03,0x04,0x05,0x06,0x07,0x08,
  0x09,0x0a,0x0c,0x0d,0x0f,0x10,0x12,0x13,0x15,0x17,0x19,0x1b,0x1d,0x1f,0x21,0x23,
  0x25,0x27,0x2a,0x2c,0x2e,0x31,0x33,0x36,0x38,0x3b,0x3e,0x40,0x43,0x46,0x49,0x4c,
  0x4f,0x51,0x54,0x57,0x5a,0x5d,0x60,0x63,0x67,0x6a,0x6d,0x70,0x73,0x76,0x79,0x7c
};

/**
 * Increase by steps the luminosity, then dim by steps the whole cube.
 * 
 * Mainly useful as a test of the led cube.
 */
void levels(void) {
  for(int i = 0; i < BRIGHTNESS_MAX; i++) {
    FrameBufferSet(i);
    FrameBufferSwitch();
    delay(100);
  }

  for(int i = BRIGHTNESS_MAX; i >= 0; i--) {
    FrameBufferSet(i);
    FrameBufferSwitch();
    delay(100);
  }
}

/**
 * Increase by steps the luminosity, then dim by steps the whole cube.
 * 
 * Mainly useful as a test of the led cube.
 */
void testfreq(void) {
  FrameBufferSet(255);
  FrameBufferSwitch();
  delay(200);

  FrameBufferSet(1);
  FrameBufferSwitch();
  delay(200);
}

/**
 * Blinks the whole cube on and off with a period `period`.
 */
void blinkWholeCube(int period /* [ms] */) {
  FrameBufferSet(255);
  FrameBufferSwitch();
  delay(period >> 1);
  
  FrameBufferBlank();
  FrameBufferSwitch();
  delay(period >> 1);
}

/**
 * Fade in and out following a sine curve with a period `period`.
 */
void sine(int period /* [ms] */) {
  for(short int i = 0; i < BRIGHTNESS_MAX; i++) {
    FrameBufferSet(sinewave[i]);
    FrameBufferSwitch();
    delay(period / BRIGHTNESS_MAX);
  }
}

/**
 * Fade in and out following a sine curve with a period `period`, but only a 
 * small 2x2x2 cube in the center.
 */
void sine2(int period /* [ms] */) {
  for(short int i = 0; i < BRIGHTNESS_MAX; i++) {
    FrameBufferBlank();

    for (char x = 1; x < 3; x++) {
      for (char y = 1; y < 3; y++) {
        for (char z = 1; z < 3; z++) {
          FrameBufferWrite(x, y, z, sinewave[i]);
        }
      }
    }

    FrameBufferSwitch();
    delay(period / BRIGHTNESS_MAX);
  }
}

/**
 * Fade in and out following a sine curve with a period `period` the entire
 * cube, but with the center 2x2x2 cube dephased by 180°.
 */
void sine3(int period /* [ms] */) {
  for(int i = 0; i < 256; i++) {
    FrameBufferBlank();;
    for (char x = 1; x < 3; x++) {
      for (char y = 1; y < 3; y++) {
        for (char z = 1; z < 3; z++) {
          FrameBufferWrite(x, y, z, sinewave[i]);
        }
      }
    }

    for (char x = 0; x < MAX_X; x++) {
      for (char y = 0; y < MAX_Y; y++) {
        for (char z = 0; z < MAX_Z; z++) {
          if ( ((x == 0) || (x == 3))
             ||((y == 0) || (y == 3))
             ||((z == 0) || (z == 3))) {
                FrameBufferWrite(x, y, z, sinewave[(i+128)%256]);
          }
        }
      }
    }

    FrameBufferSwitch();
    delay(period / BRIGHTNESS_MAX);
  }
}

/**
 * Random rain drpos fall to the bottom of the cube
 */
void randomRain() {
  unsigned char val;
  for (char a = MAX_Z; a > 0; a--) {
    // animation of 4 steps, requiring computing 4 full frames
    for (char x = 0; x < MAX_X; x++) {
      for (char y = 0; y < MAX_Y; y++) {
          FrameBufferWrite(x, y, 3,
            (random(0, 4) == 0) ? 1 : random(0, BRIGHTNESS_MAX));
          FrameBufferCopy(x, y, 2, x, y, 3);
          FrameBufferCopy(x, y, 1, x, y, 2);
          FrameBufferCopy(x, y, 0, x, y, 1);
      }
    }

    FrameBufferSwitch();
    delay(500);
  }
}

/********************************** BODY ***********************************/

void setup()
{
#if DEBUG_SETUP
  static long timeStartSetup = micros();
#endif

  // Seeding random for random pattern
  randomSeed(analogRead(10));

#if DEBUG
  Serial.begin(9600);
  Serial.println("\nSetup DONE");
#endif

  FrameBufferInit();

#if DEBUG_SETUP
  Serial.print("Setup Time: ");
  Serial.println((micros() - timeStartSetup), DEC);
#endif
}

void loop() {
  static char a = -1;

#if DEBUG_LOOP
  static long timeStartLoop = micros();
#endif

  a = ((a+1) % 9);
  for (int j = 0; j < 10; j++) {
    switch(a) {
      case 0: sine2(2000); break;
      case 1: randomRain(); break;
      case 2: sine3(2000); break;
      case 3: blinkWholeCube(1000); break;
      case 4: sine(2000); break;
      case 5: printAllSymbols(); break;
      case 6: FrameBufferWriteStr("hello world", 250, 255);
              delay(1000);
              break;
      case 7: testfreq(); break;
      case 8: levels(); break;
      default: break;
    }
  }


#if DEBUG_LOOP
  Serial.print("Loop Time: ");
  Serial.println((micros() - timeStartLoop), DEC);
#endif
}

