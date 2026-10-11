# BootselDash
The firmware makes the Bootsel button act as the spacebar which allows you to play Geometry Dash.

The firmware has been tested with a raspberry pi pico, with the RP2040 processor. It mostly likey wont work with other processors

## Compilation (for linux)
# Install dependancies
```bash
sudo apt update
sudo apt install git cmake ninja-build gcc-arm-none-eabi libnewlib-arm-none-eabi
```

# Clone the repo
```bash
git clone https://github.com/1869-bli/BootselDash.git
cd BootselDash
```
# Download the Pico SDK
```bash
git clone --depth 1 --recurse-submodules https://github.com/raspberrypi/pico-sdk.git ~/pico-sdk
```

# Build
```bash
cmake -S . -B build -G Ninja -DPICO_SDK_PATH="$HOME/pico-sdk" -DPICO_BOARD=pico
cmake --build build
```
