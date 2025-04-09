# ESP32 LVGL

Board: `ESP32-C6-LCD-1.47`
waveshare [URL](https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47)

Connect VS Code COM port as following: `ESP32-C6 chip via ... USB-JTAG`

Initial setup:

```text
CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
CONFIG_ESPTOOLPY_FLASHSIZE="4MB"
```

Use partitions from example:

```text
# Name,     Type, SubType, Offset,   Size, Flags
# Note: if you have increased the bootloader size, make sure to update the offsets to avoid overlap,,,,
nvs,        data, nvs,      0x9000,  0x6000,
factory,0,0,        0x10000, 2M,
flash_test, data, fat,      ,        528K,

```

# Display

Model: `ST7789` 
- `LCD type 	TFT `
- `172(H)RGB x320(V)`

```text
LCD Pin 	ESP32C6
MOSI        GPIO6
SCLK        GPIO7
LCD_CS      GPIO14
LCD_DC      GPIO15
LCD_RST     GPIO21
LCD_BL      GPIO22 
```

Driver: [components/esp_lcd/src/esp_lcd_panel_st7789.c](https://github.com/espressif/esp-idf/blob/a25e7ab59ed197817d4a78e139220b2707481f67/components/esp_lcd/src/esp_lcd_panel_st7789.c)

# RGB

```text
RGB_Control 	GPIO8 
```

# SD Card

```text
TF Card     ESP32C6
MISO        GPIO5
MOSI        GPIO6
SCLK        GPIO7
CS          GPIO4
SD_D1       NC
SD_D2       NC 
```


## Playgroud for testing LVGL

LVGL Start [DOC](https://github.com/lvgl/lvgl/blob/4a506542dd3fbcd8d0f39cd12bde542589b57081/docs/get-started/quick-overview.md) 

# Making things work:

## Example way

Requre older LVGL 8, not suitable now

Use example dir hierarchy and files from:
- https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47
- File `ESP32-S3-LCD-1.47-Demo`

Just use as project example.
Cannot be used with latest version 9+

[Log](doc/log/lvgl_9_is_not_usable_with_example.log)

## Other people help way

Read this [forum](https://forum.lvgl.io/t/gestures-are-slow-perceiving-only-detecting-one-of-5-10-tries/18515)

It has a great example of everything without a hustle.

For deep understanding it can be usefull to read the `complex way` later.

## Complex way

Continue to study:

Install:

Usual 1st cmd OR better local (to be able to configure lvgl for once)

- `idf.py add-dependency "lvgl/lvgl^9.2.2"`
- `git submodule add https://github.com/lvgl/lvgl.git components/lvgl`

Into the ignored folder (not to add the full other repo in my repo): 

- `git submodule add -f https://github.com/lvgl/lvgl.git components/lvgl`

IMPORTANT: Switch branch to a released last: `9.2.2`

- `cd .\components\lvgl\`
- `git checkout v9.2.2`

### Optional

Optional, not need LED for LCD to work

- `idf.py add-dependency "espressif/led_strip^3.0.1"`

Other module help driver for display, probably (not actually, try to use lvgl pure, as it already has a driver for our display and board):

- `idf.py add-dependency "espressif/esp_lvgl_port^2.5.0"`

### Setup

#### Configuration

According to the [doc](https://docs.lvgl.io/master/details/integration/adding-lvgl-to-your-project/configuration.html#lv-conf-h)

1. Copy `lv_conf_template.h` 
   1. to `components/lv_conf.h`
      1. Should be a path: 
         1. `{PROJECT_ROOT}/components/lvgl/..`
         2. `{PROJECT_ROOT}/components/lv_conf.h`
   2. Open and change the following to 1: `#if 0` (/* Set this to "1" to enable content */)
   3. Define `LV_COLOR_DEPTH 16` for LCD: `Display: ST7789`
   4. Define `LV_USE_ST7789 1` for LCD: `Display: ST7789`
   5. Optional: disable examples: `LV_BUILD_EXAMPLES 0`
   6. Optional: disable demo: `LV_USE_DEMO_WIDGETS 0 `

2. Optional: config LVGL at `menuconfig`
   1. `idf.py menuconfig` -> `LVGL configuration` (most bottom) - >
        ```text
        Color depth: 24
        ```
3. Try build the project first time.
   1. Do not flash, just builds.
   2. Everything should be fine.

#### Connecting LVGL

According to the [DOC](https://docs.lvgl.io/master/details/integration/adding-lvgl-to-your-project/connecting_lvgl.html#initializing-lvgl)

Help [example](https://forum.lvgl.io/t/gestures-are-slow-perceiving-only-detecting-one-of-5-10-tries/18515/86)

1. Modify `main.c` adding `#include <lvgl.h>` and init at the `app_main`: `lv_init();`
2. Init drivers for our display: `ST7789`
   1. Doc [read](https://docs.lvgl.io/master/details/integration/driver/display/st7789.html)
   2. Check this too: [DOC](https://github.com/lvgl/lvgl/blob/release/v9.2/docs/porting/display.rst#id2)

5. Using 

6. Create a dir for LVGL 'Squareline studio' export: `{PROJECT_ROOT}/components/screen/`
   1. It's not used yet

# Emulator

Using VS code:

- https://github.com/lvgl/lv_port_pc_vscode


## Refference

Project example
- https://github.com/Omegaki113r/lvgl_demo/tree/main
- https://github.com/VolosR/waveshareBoards/tree/main/C6example

## DOCs

Waveshare:
- https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47

LVGL Espressif
- https://docs.lvgl.io/master/details/integration/chip/espressif.html

LVGL:
- https://github.com/lvgl/lvgl/blob/4a506542dd3fbcd8d0f39cd12bde542589b57081/docs/get-started/quick-overview.md

Espressif:
- https://github.com/espressif/esp-bsp/tree/master/components/esp_lvgl_port

Other:
- https://github.com/Omegaki113r/lvgl_demo/tree/main/components/lvgl


## Components

- https://components.espressif.com/components/lvgl/lvgl/versions/9.2.2
- https://components.espressif.com/components/espressif/esp_lvgl_port/versions/2.5.0


## ESP Home

- https://esphome.io/cookbook/lvgl