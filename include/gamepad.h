#ifndef gamepad_h
#define gamepad_h

#include <pico/stdlib.h>
#include <tusb.h>

typedef enum mount_state {
  STATE_MOUNTED = 0,
  STATE_NOT_MOUNTED,
  STATE_SUSPENDED,
  STATE_SUSPENDED_BUT_RESUMABLE,
} mount_state_t;

typedef struct gamepad_state {
  mount_state_t mount_state;
  hid_gamepad_report_t hid_report;
  bool dirty;
} gamepad_state_t;

void gamepad_init();
void gamepad_update();

#endif
