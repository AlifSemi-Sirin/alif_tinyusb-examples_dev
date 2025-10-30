/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

#define USBD_STACK_SIZE    (4096) * (CFG_TUSB_DEBUG ? 2 : 1)
#define VIDEO_STACK_SIZE   (2048)

static K_THREAD_STACK_DEFINE(usb_device_stack, USBD_STACK_SIZE);
static struct k_thread usb_device_taskdef;

static K_THREAD_STACK_DEFINE(video_stack, VIDEO_STACK_SIZE);
static struct k_thread video_taskdef;

static struct k_timer blinky_tm;

void led_blinky_cb(struct k_timer *timer);
static void usb_device_task(void *p1, void *p2, void *p3);
void video_task(void *p1, void *p2, void *p3);


//--------------------------------------------------------------------+
// Main
//--------------------------------------------------------------------+
int main(void) {
  board_init();

  // Create task for tinyusb
  k_thread_create(&usb_device_taskdef, usb_device_stack, USBD_STACK_SIZE,
                  usb_device_task, NULL, NULL, NULL,
                  K_HIGHEST_APPLICATION_THREAD_PRIO-1, 0, K_NO_WAIT);
  k_thread_name_set(&usb_device_taskdef, "usbd");

  // Create task for cdc
  k_thread_create(&video_taskdef, video_stack, VIDEO_STACK_SIZE,
                  video_task, NULL, NULL, NULL,
                  K_HIGHEST_APPLICATION_THREAD_PRIO-2, 0, K_NO_WAIT);
  k_thread_name_set(&video_taskdef, "video");

  // Create blinky timer
  k_timer_init(&blinky_tm, led_blinky_cb, NULL);
  k_timer_start(&blinky_tm, K_MSEC(BLINK_NOT_MOUNTED), K_MSEC(BLINK_NOT_MOUNTED));

  return 0;
}

// USB Device Driver task
// This top level thread process all usb events and invoke callbacks
static void usb_device_task(void *p1, void *p2, void *p3) {
  (void) p1, (void) p2, (void) p3;

  // init device stack on configured roothub port
  // This should be called after scheduler/kernel is started.
  // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
  tusb_rhport_init_t dev_init = {
    .role = TUSB_ROLE_DEVICE,
    .speed = TUSB_SPEED_AUTO
  };
  tusb_init(BOARD_TUD_RHPORT, &dev_init);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  // RTOS forever loop
  while (1) {
    // put this thread to waiting state until there is new events
    tud_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
  k_timer_start(&blinky_tm, K_MSEC(BLINK_MOUNTED), K_MSEC(BLINK_MOUNTED));
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
  k_timer_start(&blinky_tm, K_MSEC(BLINK_NOT_MOUNTED), K_MSEC(BLINK_NOT_MOUNTED));
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
  k_timer_start(&blinky_tm, K_MSEC(BLINK_SUSPENDED), K_MSEC(BLINK_SUSPENDED));
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  if (tud_mounted())
  {
    k_timer_start(&blinky_tm, K_MSEC(BLINK_MOUNTED), K_MSEC(BLINK_MOUNTED));
  }
  else
  {
    k_timer_start(&blinky_tm, K_MSEC(BLINK_NOT_MOUNTED), K_MSEC(BLINK_NOT_MOUNTED));
  }
}

//--------------------------------------------------------------------+
// USB Video
//--------------------------------------------------------------------+
static unsigned frame_num = 0;

#ifdef CFG_EXAMPLE_VIDEO_READONLY
// For mcus that does not have enough SRAM for frame buffer, we use fixed frame data.
// To further reduce the size, we use MJPEG format instead of YUY2.
#include "images.h"

#if !defined(CFG_EXAMPLE_VIDEO_DISABLE_MJPEG)
static struct {
  uint32_t       size;
  uint8_t const *buffer;
} const frames[] = {
  {color_bar_0_jpg_len, color_bar_0_jpg},
  {color_bar_1_jpg_len, color_bar_1_jpg},
  {color_bar_2_jpg_len, color_bar_2_jpg},
  {color_bar_3_jpg_len, color_bar_3_jpg},
  {color_bar_4_jpg_len, color_bar_4_jpg},
  {color_bar_5_jpg_len, color_bar_5_jpg},
  {color_bar_6_jpg_len, color_bar_6_jpg},
  {color_bar_7_jpg_len, color_bar_7_jpg},
};
#endif

#else

// YUY2 frame buffer
static uint8_t frame_buffer[FRAME_WIDTH * FRAME_HEIGHT * 16 / 8];

static void fill_color_bar(uint8_t* buffer, unsigned start_position) {
  /* EBU color bars: https://stackoverflow.com/questions/6939422 */
  static uint8_t const bar_color[8][4] = {
      /*  Y,   U,   Y,   V */
      { 235, 128, 235, 128}, /* 100% White */
      { 219,  16, 219, 138}, /* Yellow */
      { 188, 154, 188,  16}, /* Cyan */
      { 173,  42, 173,  26}, /* Green */
      {  78, 214,  78, 230}, /* Magenta */
      {  63, 102,  63, 240}, /* Red */
      {  32, 240,  32, 118}, /* Blue */
      {  16, 128,  16, 128}, /* Black */
  };
  uint8_t* p;

  /* Generate the 1st line */
  uint8_t* end = &buffer[FRAME_WIDTH * 2];
  unsigned idx = (FRAME_WIDTH / 2 - 1) - (start_position % (FRAME_WIDTH / 2));
  p = &buffer[idx * 4];
  for (unsigned i = 0; i < 8; ++i) {
    for (int j = 0; j < FRAME_WIDTH / (2 * 8); ++j) {
      memcpy(p, &bar_color[i], 4);
      p += 4;
      if (end <= p) {
        p = buffer;
      }
    }
  }

  /* Duplicate the 1st line to the others */
  p = &buffer[FRAME_WIDTH * 2];
  for (unsigned i = 1; i < FRAME_HEIGHT; ++i) {
    memcpy(p, buffer, FRAME_WIDTH * 2);
    p += FRAME_WIDTH * 2;
  }
}

#endif

static void video_send_frame(void) {
  if (!tud_video_n_streaming(0, 0)) {
    frame_num = 0;
    return;
  }

#ifdef CFG_EXAMPLE_VIDEO_READONLY
  #if defined(CFG_EXAMPLE_VIDEO_DISABLE_MJPEG)
  tud_video_n_frame_xfer(0, 0, (void*)(uintptr_t)&frame_buffer[(frame_num % (FRAME_WIDTH / 2)) * 4],
                         FRAME_WIDTH * FRAME_HEIGHT * 16/8);
  #else
  tud_video_n_frame_xfer(0, 0, (void*)(uintptr_t)frames[frame_num % 8].buffer, frames[frame_num % 8].size);
  #endif
#else
  fill_color_bar(frame_buffer, frame_num);
  tud_video_n_frame_xfer(0, 0, (void*) frame_buffer, FRAME_WIDTH * FRAME_HEIGHT * 16 / 8);
#endif
}


void video_task(void *p1, void *p2, void *p3) {
  (void) p1;
  (void) p2;
  (void) p3;

  while (1) {
    video_send_frame();

    k_msleep(1);
  }
}

void tud_video_frame_xfer_complete_cb(uint_fast8_t ctl_idx, uint_fast8_t stm_idx) {
  (void) ctl_idx;
  (void) stm_idx;
  /* flip buffer */
  ++frame_num;
}

int tud_video_commit_cb(uint_fast8_t ctl_idx, uint_fast8_t stm_idx,
                        video_probe_and_commit_control_t const* parameters) {
  (void) ctl_idx;
  (void) stm_idx;
  /* convert unit to ms from 100 ns */
  return VIDEO_ERROR_NONE;
}

//--------------------------------------------------------------------+
// BLINKING TIMER
//--------------------------------------------------------------------+
void led_blinky_cb(struct k_timer *timer)
{
  (void) timer;
  static bool led_state = false;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
