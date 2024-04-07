#include "gamepad.h"

int main() {
  gamepad_init();

  while (1) {
    gamepad_update();
  }
}
