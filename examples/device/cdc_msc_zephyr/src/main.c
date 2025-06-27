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

// Increase stack size when debug log is enabled
#define USBD_STACK_SIZE    (4096) * (CFG_TUSB_DEBUG ? 2 : 1)
#define CDC_STACK_SIZE     (2048)

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTOTYPES
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

static K_THREAD_STACK_DEFINE(usb_device_stack, USBD_STACK_SIZE);
static struct k_thread usb_device_taskdef;

static K_THREAD_STACK_DEFINE(cdc_stack, CDC_STACK_SIZE);
static struct k_thread cdc_taskdef;

static struct k_timer blinky_tm;

void led_blinky_cb(struct k_timer *timer);
static void usb_device_task(void *p1, void *p2, void *p3);
void cdc_task(void *p1, void *p2, void *p3);

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
  k_thread_create(&cdc_taskdef, cdc_stack, CDC_STACK_SIZE,
                  cdc_task, NULL, NULL, NULL,
                  K_HIGHEST_APPLICATION_THREAD_PRIO-2, 0, K_NO_WAIT);
  k_thread_name_set(&cdc_taskdef, "cdc");

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

    // following code only run if tud_task() process at least 1 event
    tud_cdc_write_flush();
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
// USB CDC
//--------------------------------------------------------------------+
void cdc_task(void *p1, void *p2, void *p3) {
  (void) p1, (void) p2, (void) p3;

  // RTOS forever loop
  while (1) {
    // connected() check for DTR bit
    // Most but not all terminal client set this when making connection
    // if ( tud_cdc_connected() )
    {
      // There are data available
      while (tud_cdc_available()) {
        uint8_t buf[64];

        // read and echo back
        uint32_t count = tud_cdc_read(buf, sizeof(buf));
        (void) count;

        // Echo back
        // Note: Skip echo by commenting out write() and write_flush()
        // for throughput test e.g
        //    $ dd if=/dev/zero of=/dev/ttyACM0 count=10000
        tud_cdc_write(buf, count);
      }

      tud_cdc_write_flush();
    }

    // TODO: check if this delay is essential to allow idle task to run and reset watchdog
    k_msleep(1);
  }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  (void) itf;
  (void) rts;

  // TODO set some indicator
  if (dtr) {
    // Terminal connected
  } else {
    // Terminal disconnected
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf) {
  (void) itf;
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
