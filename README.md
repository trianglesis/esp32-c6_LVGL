# ESP32 LVGL

Board: `ESP32-C6-LCD-1.47`
waveshare [URL](https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47)

Connect VS Code COM port as following: `ESP32-C6 chip via ... USB-JTAG`

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

Just use as project example

## Complex way

Install:

- `idf.py add-dependency "lvgl/lvgl^9.2.2"`
- `idf.py add-dependency "espressif/led_strip^3.0.1"`

OR better local

- `git submodule add https://github.com/lvgl/lvgl.git components/lvgl`

Or into the ignored folder: 

- `git submodule add -f https://github.com/lvgl/lvgl.git components/lvgl`

Other module help driver for display, probably:
- `idf.py add-dependency "espressif/esp_lvgl_port^2.5.0"`

# Setup

1. Copy `lv_conf_template.h` 
   1. to `components/lv_conf.h`, change to 1: `#if 0`
      1. Should be a path: 
         1. `{PROJECT_ROOT}/components/lvgl/..`
         2. `{PROJECT_ROOT}/components/lv_conf.h`
      2. According to [doc](https://github.com/lvgl/lvgl/blob/4a506542dd3fbcd8d0f39cd12bde542589b57081/docs/get-started/quick-overview.md)
      3. I've based my exaple at this dir [repo](https://github.com/Omegaki113r/lvgl_demo/tree/main/components/lvgl)

2. Next config LVGL:
   1. `idf.py menuconfig` -> `LVGL configuration` (most bottom) - >
        ```text
        Color depth: 24
        ```
3. Try build the project first time.
   1. no flashing!

4. Adding drivers for display
   1. As per `LVGL Espressif` doc, drivers can be found at `lvgl_esp32_drivers` package.
      1. *NOTE*: `LVGL doc` points to `IDF` repo, showing this display is natively supported:
         1. https://github.com/espressif/esp-idf/tree/master/components/esp_lcd/src at: `components/esp_lcd/src/esp_lcd_panel_st7789.c` it means we only need to install helper modules for LCD, or we can use it as it is, initializing everything from scratch.
   2. Install helper module now: `idf.py add-dependency "espressif/esp_lvgl_port^2.5.0"`
      1. Or should I?

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