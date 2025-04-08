# ESP32 LVGL

Board: `ESP32-C6-LCD-1.47`

Display: ` 172(H)RGB x320(V) `

waveshare [URL](https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47)

## Playgroud for testing LVGL

LVGL Start [DOC](https://github.com/lvgl/lvgl/blob/4a506542dd3fbcd8d0f39cd12bde542589b57081/docs/get-started/quick-overview.md) 

# Making things work:

Install:

- `idf.py add-dependency "lvgl/lvgl^9.2.2"`

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
3. Next: ???

# Emulator

Using VS code:

- https://github.com/lvgl/lv_port_pc_vscode


## Refference

Project example
- https://github.com/Omegaki113r/lvgl_demo/tree/main

## DOCs

Espressif
- https://docs.lvgl.io/master/details/integration/chip/espressif.html

LVGL:
- https://github.com/lvgl/lvgl/blob/4a506542dd3fbcd8d0f39cd12bde542589b57081/docs/get-started/quick-overview.md

Other:
- https://github.com/Omegaki113r/lvgl_demo/tree/main/components/lvgl
