#include "gamepad.h"

#include <bsp/board.h>
#include <tusb.h>
#include <ws2812.h>

#include "usb_descriptors.h"

static gamepad_state_t gamepad_state = {
    .mount_state = STATE_NOT_MOUNTED,
    .hid_report = {
        .x = 0,
        .y = 0,
        .z = 0,
        .rz = 0,
        .rx = 0,
        .ry = 0,
        .hat = 0,
        .buttons = 0,
    },
    .dirty = true,
};

static void buttons_init();
static void buttons_update();

static void indicator_init();
static void indicator_update();

static void hid_handle_suspended_state();
static void hid_send_report(uint8_t report_id);
static void hid_update();

void gamepad_init() {
  board_init();
  buttons_init();
  indicator_init();
  tusb_init();
}

void gamepad_update() {
  tud_task();
  buttons_update();
  indicator_update();
  hid_update();
}

void tud_mount_cb() {
  gamepad_state.mount_state = STATE_MOUNTED;
}

void tud_umount_cb() {
  gamepad_state.mount_state = STATE_NOT_MOUNTED;
}

void tud_suspend_cb(bool remote_wakeup_en) {
  gamepad_state.mount_state = remote_wakeup_en ? STATE_SUSPENDED_BUT_RESUMABLE : STATE_SUSPENDED;
}

void tud_resume_cb() {
  gamepad_state.mount_state = STATE_MOUNTED;
}

static void hid_handle_suspended_state() {
  if (gamepad_state.mount_state != STATE_SUSPENDED_BUT_RESUMABLE) return;

  if (gamepad_state.hid_report.buttons != 0) tud_remote_wakeup();
}

static void hid_send_report(uint8_t report_id) {
  if (report_id != REPORT_ID_GAMEPAD)
    return;

  if (!tud_hid_ready())
    return;

  if (gamepad_state.dirty)
    tud_hid_report(REPORT_ID_GAMEPAD, &gamepad_state.hid_report, sizeof(gamepad_state.hid_report));
  gamepad_state.dirty = false;
}

static void hid_update() {
  // Poll every 10ms
  const uint32_t interval_ms = 1;
  static uint32_t start_ms = 0;

  if (to_ms_since_boot(get_absolute_time()) - start_ms < interval_ms)
    return;  // not enough time
  start_ms += interval_ms;

  if (tud_suspended()) {
    hid_handle_suspended_state();
    return;
  }

  hid_send_report(REPORT_ID_GAMEPAD);
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len) {
  // printf("report complete %u %u\n", instance, len);
  (void)instance;
  (void)len;

  uint8_t next_report_id = report[0] + 1;

  if (next_report_id < REPORT_ID_COUNT) {
    hid_send_report(next_report_id);
  }
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
  // printf("get report %u %u %u %u\n", instance, report_id, report_type, reqlen);
  return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
  // printf("set report %u %u %u %u\n", instance, report_id, report_type, bufsize);
}

// #define ENV_PROD

static ws2812_t ws2812 = {
    .pio = pio0,
    .sm = 0,
#ifdef ENV_PROD
    .pin = 16,
#else
    .pin = 23,
#endif
    .freq = WS2812_TYP_FREQ,
    .rgbw = false,
    .type = WS2812_TYPE_WS2812C,
};

static void indicator_init() {
  ws2812_init(&ws2812);

  ws2812_put_pixel(&ws2812, 0x000000);
  ws2812_wait_reset(&ws2812);
}

static void indicator_update() {
  switch (gamepad_state.mount_state) {
    case STATE_MOUNTED:
      ws2812_put_pixel(&ws2812, 0x070000);
      break;

    case STATE_NOT_MOUNTED:
      ws2812_put_pixel(&ws2812, 0x000700);
      break;

    case STATE_SUSPENDED:
      ws2812_put_pixel(&ws2812, 0x000007);
      break;

    case STATE_SUSPENDED_BUT_RESUMABLE:
      ws2812_put_pixel(&ws2812, 0x070700);
      break;
  }

  ws2812_wait_reset(&ws2812);
}

static uint const clk_pin = 2;
static uint const latch_pin = 3;
static uint const data_pin = 4;
static uint32_t buttons_codes[] = {
  GAMEPAD_BUTTON_0,       // B, button 0
  GAMEPAD_BUTTON_3,       // Y, button 2
  GAMEPAD_BUTTON_10,      // SELECT, button 4
  GAMEPAD_BUTTON_11,      // START, button 6
  GAMEPAD_BUTTON_1,       // A, button 1
  GAMEPAD_BUTTON_4,       // X, button 3
  GAMEPAD_BUTTON_6,       // L, button 9
  GAMEPAD_BUTTON_7,       // R, button 10
};
static uint8_t hat_map[] = {
  GAMEPAD_HAT_CENTERED,
  GAMEPAD_HAT_UP,
  GAMEPAD_HAT_DOWN,
  GAMEPAD_HAT_CENTERED,
  GAMEPAD_HAT_LEFT,
  GAMEPAD_HAT_UP_LEFT,
  GAMEPAD_HAT_DOWN_LEFT,
  GAMEPAD_HAT_CENTERED,
  GAMEPAD_HAT_RIGHT,
  GAMEPAD_HAT_UP_RIGHT,
  GAMEPAD_HAT_DOWN_RIGHT,
  GAMEPAD_HAT_CENTERED,
  GAMEPAD_HAT_CENTERED,
  GAMEPAD_HAT_CENTERED,
};

static void buttons_init() {
  gpio_init(clk_pin);
  gpio_set_dir(clk_pin, GPIO_OUT);
  gpio_set_pulls(clk_pin, false, true);

  gpio_init(latch_pin);
  gpio_set_dir(latch_pin, GPIO_OUT);
  gpio_set_pulls(latch_pin, false, true);

  gpio_init(data_pin);
  gpio_set_dir(data_pin, GPIO_IN);
  gpio_set_pulls(data_pin, true, false);
}

static void buttons_update() {
  uint32_t prev_buttons = gamepad_state.hid_report.buttons;
  uint8_t prev_hat = gamepad_state.hid_report.hat;
  gamepad_state.hid_report.buttons = 0;

  gpio_put(latch_pin, true);
  sleep_us(12);
  gpio_put(latch_pin, false);

  for (int i = 0; i < 4; i++) {
    sleep_us(6);
    if (gpio_get(data_pin) == false) gamepad_state.hid_report.buttons |= buttons_codes[i];

    gpio_put(clk_pin, true);
    sleep_us(6);
    gpio_put(clk_pin, false);
    sleep_us(6);
  }

  uint8_t hat = 0;

  for (int i = 0; i < 4; i++) {
    sleep_us(6);
    if (gpio_get(data_pin) == false) hat |= 1 << i;

    gpio_put(clk_pin, true);
    sleep_us(6);
    gpio_put(clk_pin, false);
    sleep_us(6);
  }

  gamepad_state.hid_report.hat = hat_map[hat];

  for (int i = 4; i < 8; i++) {
    sleep_us(6);
    if (gpio_get(data_pin) == false) gamepad_state.hid_report.buttons |= buttons_codes[i];

    gpio_put(clk_pin, true);
    sleep_us(6);
    gpio_put(clk_pin, false);
    sleep_us(6);
  }

  if (prev_buttons != gamepad_state.hid_report.buttons || prev_hat != gamepad_state.hid_report.buttons) gamepad_state.dirty = true;
}
