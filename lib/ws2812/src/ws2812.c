#include "ws2812.h"

#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "pico/time.h"

void ws2812_init(ws2812_t const *ws2812) {
  uint offset = pio_add_program(ws2812->pio, &ws2812_program);
  pio_gpio_init(ws2812->pio, ws2812->pin);
  pio_sm_set_consecutive_pindirs(ws2812->pio, ws2812->sm, ws2812->pin, 1, true);

  pio_sm_config c = ws2812_program_get_default_config(offset);
  sm_config_set_sideset_pins(&c, ws2812->pin);
  sm_config_set_out_shift(&c, false, true, ws2812->rgbw ? 32 : 24);
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

  int cycles_per_bit = ws2812_T1 + ws2812_T2 + ws2812_T3;
  float div = clock_get_hz(clk_sys) / (ws2812->freq * cycles_per_bit);
  sm_config_set_clkdiv(&c, div);

  pio_sm_init(ws2812->pio, ws2812->sm, offset, &c);
  pio_sm_set_enabled(ws2812->pio, ws2812->sm, true);
}

void ws2812_put_pixel(ws2812_t const *ws2812, uint32_t const pixel) {
  pio_sm_put_blocking(ws2812->pio, ws2812->sm, ws2812->rgbw ? pixel : pixel << 8);
}

void ws2812_wait_reset(ws2812_t const *ws2812) {
  switch (ws2812->type) {
    case WS2812_TYPE_WS2812:
    case WS2812_TYPE_WS2812B:
      sleep_us(50);
      break;

    case WS2812_TYPE_WS2812C:
      sleep_us(280);
      break;
  }
}
