#ifndef ws2812_h
#define ws2812_h

#include "hardware/pio.h"

#define WS2812_MIN_FREQ 400'000
#define WS2812_TYP_FREQ 800'000
#define WS2812_MAX_FREQ 1'200'000

typedef enum ws2812_type {
    WS2812_TYPE_WS2812,
    WS2812_TYPE_WS2812B,
    WS2812_TYPE_WS2812C,
} ws2812_type_t;

typedef struct ws2812 {
  PIO pio;
  uint sm;
  uint pin;
  float freq;
  bool rgbw;
  ws2812_type_t type;
} ws2812_t;

void ws2812_init(ws2812_t const *ws2812);

void ws2812_put_pixel(ws2812_t const *ws2812, uint32_t const pixel);

void ws2812_wait_reset(ws2812_t const *ws2812);

#endif
