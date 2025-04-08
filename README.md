# ESP32 LVGL

Board: `ESP32-C6-LCD-1.47`

Display: ` 172(H)RGB x320(V) `

waveshare [URL](https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47)

## Playgroud for testing LVGL

LVGL Start [DOC](https://github.com/lvgl/lvgl/blob/4a506542dd3fbcd8d0f39cd12bde542589b57081/docs/get-started/quick-overview.md) 

# Making things work:

Install:

`idf.py add-dependency "lvgl/lvgl^9.2.2"`

OR better local

`git submodule add https://github.com/lvgl/lvgl.git components/lvgl`

Or into the irnored folder: `git submodule add -f https://github.com/lvgl/lvgl.git components/lvgl`

`idf.py add-dependency "espressif/esp_lvgl_port^2.5.0"`

# Setup

`idf.py menuconfig` -> `LVGL configuration` (most bottom) - >

```text
Color depth: 24
```

## Refference

Project example
- https://github.com/Omegaki113r/lvgl_demo/tree/main

## DOCs

Espressif
- https://docs.lvgl.io/master/details/integration/chip/espressif.html
- 