set dotenv-load

default: build upload

build:
  @mkdir -p build
  @cd build && cmake ..
  @cd build && make

upload:
  openocd -f interface/cmsis-dap.cfg -c "adapter speed 5000" -f target/rp2040.cfg -c "program build/gamepad.elf verify reset exit"

