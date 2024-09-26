# Inky frame image manager

An application for upload an image though web server, build with C++ and Svelte js

This project is base on template https://github.com/pimoroni/pico-boilerplate

## Wifi setup
- Using `wifi_config.cmake`:
  1. Clone `wifi_config.cmake.sample` and rename it into `wifi_config.cmake` (`wifi_config.cmake` won't track in this repo)
  2. Replace `<ENTER_YOUR_SSID>` and `<ENTER_YOUR_WIFI_PASSWORD>` with your Wifi SSID and password

- Using `wifi.txt`:
  If you use the provided image directly instead of compile by your own
  1. Create `wifi.txt` in SD card, put it in root directory, file format need to be FAT
  2. Place the wifi SSID and password in following format in two separate line, when the program can't find compile marco `WIFI_SSID` and `WIFI_PASSWORD`, it will try to read the wifi config form SD card in runtime
  
     ```
     <YOUR_SSID>
     <YOUR_WIFI_PASSWORD>
     ```

## Structure

This applicatioin is build using following stack

### Web front-end
- Svelte
- flowbite-svelte: UI library integrated with Svelte,
- cropperjs: Handle image cropping

Refer to web_src/package.json for full dependency

When integrated the front-end source into Pico W web server, it need compile and bundle all the web asset,
than convert into `packed_fs.c` (binary array to store all static content need to be serve,
finally host by internal server

Reference to `create_web_packed_fs.sh` for detail about bundle all web content into `packed_fs.c`

### Pico W
- [mongoose](https://github.com/cesanta/mongoose): Embedded web server for Pico W, for hosting all web content and handle image upload request
- [pimoroni-pico](https://github.com/pimoroni/pimoroni-pico): Offical inky frame driver
- RTOS

## Compile instruction
[WIP]

---

This project is intended as a starting point for working with the Pico SDK and Pimoroni Libraries in C++.

- [Before you start](#before-you-start)
- [Preparing your build environment](#preparing-your-build-environment)
- [Grab the Pimoroni libraries](#grab-the-pimoroni-libraries)
- [Clone this boilerplate](#clone-this-boilerplate)
- [Prepare Visual Studio Code](#prepare-visual-studio-code)
- [Prepare your project](#prepare-your-project)
- [Pick your LICENSE](#pick-your-license)

## Before you start

It's easier if you make a `pico` directory or similar in which you keep the SDK, Pimoroni Libraries and your projects alongside each other. This makes it easier to include libraries.

## Preparing your build environment

Install build requirements:

```bash
sudo apt update
sudo apt install cmake gcc-arm-none-eabi build-essential
```

And the Pico SDK:

```
git clone https://github.com/raspberrypi/pico-sdk
cd pico-sdk
git submodule update --init
export PICO_SDK_PATH=`pwd`
cd ../
```

The `PICO_SDK_PATH` set above will only last the duration of your session.

You should should ensure your `PICO_SDK_PATH` environment variable is set by `~/.profile`:

```
export PICO_SDK_PATH="/path/to/pico-sdk"
```

## Grab the Pimoroni libraries

```
git clone https://github.com/pimoroni/pimoroni-pico
```

## Clone this boilerplate

```
git clone https://github.com/pimoroni/pico-boilerplate
cd pico-boilerplate
```

If you have not or don't want to set `PICO_SDK_PATH` you can edit `.vscode/settings.json` to pass the path directly to CMake.

## Prepare Visual Studio Code

Open VS Code and hit `Ctrl+Shift+P`.

Type `Install` and select `Extensions: Install Extensions`.

Make sure you install:

1. C/C++
2. CMake
3. CMake Tools
4. Cortex-Debug (optional: for debugging via a Picoprobe or Pi GPIO)
5. Markdown All in One (recommended: for preparing your own README.md)

## Prepare your project

Edit `CMakeLists.txt` and follow the instructions, you should make sure you:

1. edit your project name
2. include the libraries you need
2. link the libraries to your project

## Pick your LICENSE

We've included a copy of BSD 3-Clause License to match that used in Raspberry Pi's Pico SDK and Pico Examples. You should review this and check it's appropriate for your project before publishing your code.
