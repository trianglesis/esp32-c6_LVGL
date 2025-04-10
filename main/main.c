#include "esp_log.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "esp_check.h"
#include "esp_err.h"
#include "lvgl.h"

// pictures
// #include "ui/ui.h"

/*
This file was compiled from multiple sources and help topics:

Actual HW setup is loaded from the example of the board itself. But be aware, that the example is LVGL ver 8 only!
https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47

How to actually setup display and all helper functions:
https://forum.lvgl.io/t/gestures-are-slow-perceiving-only-detecting-one-of-5-10-tries/18515/59

Rotation: 
https://forum.lvgl.io/t/gestures-are-slow-perceiving-only-detecting-one-of-5-10-tries/18515/60

Rotated display have a "dead zone" fix:
https://forum.lvgl.io/t/gestures-are-slow-perceiving-only-detecting-one-of-5-10-tries/18515/86?u=trianglesis

NOTE: There is no touch support, so the code for this functionality is not here.

Other docs to consider:
https://docs.lvgl.io/master/API/display/lv_display.html#_CPPv422lv_display_set_defaultP12lv_display_t
https://github.com/lvgl/lvgl/blob/release/v9.2/docs/porting/display.rst#id2
*/

static const char *TAG = "playground";

/* LCD size */
#define DISP_HOR_RES        172
#define DISP_VER_RES        320

/* LCD settings */
#define DISP_DRAW_BUFF_HEIGHT 50

/* LCD pins */
// From example https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47
#define DISP_SPI_NUM         SPI2_HOST

#define LCD_PIXEL_CLOCK_HZ     (12 * 1000 * 1000)

#define DISP_GPIO_SCLK       GPIO_NUM_7 // GPIO_NUM_7
#define DISP_GPIO_MOSI       GPIO_NUM_6 // GPIO_NUM_6
#define DISP_GPIO_RST        GPIO_NUM_21 // GPIO_NUM_21
#define DISP_GPIO_DC         GPIO_NUM_15 // GPIO_NUM_15
#define DISP_GPIO_CS         GPIO_NUM_14 // GPIO_NUM_14
#define DISP_GPIO_BL         GPIO_NUM_22  // GPIO_NUM_22

// Bit number used to represent command and parameter
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

// Rotate 90deg and compensate buffer change
#define Offset_X 0 // 0 IF NOT ROTATED 270deg
#define Offset_Y 34  // 34 IF ROTATED 270deg

#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_HS_CH0_GPIO       EXAMPLE_PIN_NUM_BK_LIGHT
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_TEST_DUTY         (4000)
#define LEDC_ResolutionRatio   LEDC_TIMER_13_BIT
#define LEDC_MAX_Duty          ((1 << LEDC_ResolutionRatio) - 1)

#define BUFFER_SIZE            (DISP_VER_RES * DISP_HOR_RES * 2)

static esp_lcd_panel_handle_t panel_handle = NULL;
static esp_lcd_panel_io_handle_t io_handle = NULL;
static lv_display_t *display = NULL;

static void* buf1 = NULL;
static void* buf2 = NULL;


// Reuse from example
// Backlight
static ledc_channel_config_t ledc_channel;

void BK_Init(void) {
    
    ESP_LOGI(TAG, "Turn off LCD backlight");
    
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << DISP_GPIO_BL
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    
    // 配置LEDC
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_LS_MODE,
        .timer_num = LEDC_HS_TIMER,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel.channel    = LEDC_HS_CH0_CHANNEL;
    ledc_channel.duty       = 0;
    ledc_channel.gpio_num   = DISP_GPIO_BL;
    ledc_channel.speed_mode = LEDC_LS_MODE;
    ledc_channel.timer_sel  = LEDC_HS_TIMER;
    ledc_channel_config(&ledc_channel);
    ledc_fade_func_install(0);
}

void BK_Light(uint8_t Light) {   
    ESP_LOGI(TAG, "Set LCD backlight");

    if(Light > 100) Light = 100;
    uint16_t Duty = LEDC_MAX_Duty-(81*(100-Light));
    if(Light == 0) Duty = 0;
    // 设置PWM占空比
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, Duty);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
}

// this gets called when the DMA transfer of the buffer data has completed
static bool notify_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_display_t *disp_driver = (lv_display_t *)user_ctx;
    lv_display_flush_ready(disp_driver);
    return false;
}

// Added offset for ROTATED diaplay!
static void flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    // Rotated
    // https://forum.lvgl.io/t/gestures-are-slow-perceiving-only-detecting-one-of-5-10-tries/18515/86
    int x1 = area->x1 + Offset_X;
    int x2 = area->x2 + Offset_X;
    int y1 = area->y1 + Offset_Y;
    int y2 = area->y2 + Offset_Y;
    
    // uncomment the following line if the colors are wrong
    lv_draw_sw_rgb565_swap(px_map, (x2 + 1 - x1) * (y2 + 1 - y1)); // I have tried with and without this

    esp_lcd_panel_draw_bitmap((esp_lcd_panel_handle_t)lv_display_get_user_data(disp), x1, y1, x2 + 1, y2 + 1, px_map);
}

static esp_err_t lvgl_init(void)
{
    lv_init();

    display = lv_display_create(DISP_HOR_RES, DISP_VER_RES);
    
    buf1 = heap_caps_calloc(1, BUFFER_SIZE, MALLOC_CAP_INTERNAL |  MALLOC_CAP_DMA);
    buf2 = heap_caps_calloc(1, BUFFER_SIZE, MALLOC_CAP_INTERNAL |  MALLOC_CAP_DMA);

    lv_display_set_buffers(display, buf1, buf2, BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_display_set_user_data(display, panel_handle);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);

    lv_display_set_flush_cb(display, flush_cb);

     const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = notify_flush_ready,
    };
    /* Register done callback */
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, display), TAG, "esp_lcd_panel_io_register_event_callbacks error"); // I have tried to use 
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel_handle), TAG, "esp_lcd_panel_init error");

    lv_display_set_resolution(display, DISP_HOR_RES, DISP_VER_RES);
    lv_display_set_physical_resolution(display, DISP_HOR_RES, DISP_VER_RES);

    /* Landscape orientation:
    270deg = USB on the left side - landscape orientation
    270deg = USB on the right side - landscape orientation
        
    90 
    lv_display_set_rotation(display, LV_DISPLAY_ROTATION_90);
    esp_lcd_panel_mirror(panel_handle, true, false);
    esp_lcd_panel_swap_xy(panel_handle, true);

    180
    lv_display_set_rotation(display, LV_DISPLAY_ROTATION_180);
    esp_lcd_panel_mirror(panel_handle, true, true);
    esp_lcd_panel_swap_xy(panel_handle, false);

    270
    lv_display_set_rotation(display, LV_DISPLAY_ROTATION_270);
    esp_lcd_panel_mirror(panel_handle, false, true);
    esp_lcd_panel_swap_xy(panel_handle, true);

    https://forum.lvgl.io/t/gestures-are-slow-perceiving-only-detecting-one-of-5-10-tries/18515/60
    */
    
    lv_display_set_rotation(display, LV_DISPLAY_ROTATION_270);
    esp_lcd_panel_mirror(panel_handle, false, true);
    esp_lcd_panel_swap_xy(panel_handle, true);

    // Set this display as defaulkt for UI use
    lv_display_set_default(display);

    return ESP_OK;
}

static esp_err_t display_init(void) {

    ESP_LOGI(TAG, "Turn on LCD backlight first, to see display content early!");
    BK_Init();  // Back light
    BK_Light(100);

    // LCD initialization - enable from example
    ESP_LOGD(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = { 
        .sclk_io_num = DISP_GPIO_SCLK,
        .mosi_io_num = DISP_GPIO_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = BUFFER_SIZE, // DISP_HOR_RES * DISP_DRAW_BUFF_HEIGHT * sizeof(uint16_t);
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");

    //  - repeat after example
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = DISP_GPIO_DC,
        .cs_gpio_num = DISP_GPIO_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = notify_flush_ready,
        .user_ctx = &display,
    };

    // Attach the LCD to the SPI bus - repeat after example
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &io_handle), TAG, "SPI init failed");

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = DISP_GPIO_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .flags = { .reset_active_high = 0 }  // Not in the example
    };
    
    ESP_LOGI(TAG, "Install ST7789T panel driver");
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle), TAG, "Display init failed");

    // Reset the display
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    // Initialize LCD panel
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    // Turn on the screen
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    return ESP_OK;
}

static void lvgl_tick_increment(void *arg)
{
    // Tell LVGL how many milliseconds have elapsed
    lv_tick_inc(2);
}

static esp_err_t lvgl_tick_init(void)
{
    esp_timer_handle_t  tick_timer;

    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_tick_increment,
        .name = "LVGL tick",
    };
    ESP_RETURN_ON_ERROR(esp_timer_create(&lvgl_tick_timer_args, &tick_timer), TAG, "Creating LVGL timer filed!");
    return esp_timer_start_periodic(tick_timer, 2 * 1000); // 2 ms
}

static void lvgl_task(void *arg) {

    vTaskDelay(pdMS_TO_TICKS(10));

    esp_log_level_set("lcd_panel", ESP_LOG_VERBOSE);
    esp_log_level_set("lcd_panel.st7789", ESP_LOG_VERBOSE);
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

    esp_err_t ret = display_init();

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ST7789 failed to initilize");
        while (1);
    }
    
    ret = lvgl_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LVGL Display failed to initialize");
        while (1);
    }

    ret = lvgl_tick_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Timer failed to initialize");
        while (1);
    }

    // Create a simple label
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam euismod egestas augue at semper. Etiam ut erat vestibulum, volutpat lectus a, laoreet lorem.");
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
    lv_obj_set_width(label, DISP_VER_RES);  /*Set smaller width to make the lines wrap*/
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    long curtime = esp_timer_get_time()/1000;
    int counter = 0;

    // Handle LVGL tasks
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_task_handler();

        if (esp_timer_get_time()/1000 - curtime > 1000) {
            curtime = esp_timer_get_time()/1000;

            char textlabel[20];
            sprintf(textlabel, "This is counter: %u\n", counter);
            printf(textlabel);
            lv_label_set_text(label, textlabel);
            lv_obj_set_width(label, DISP_VER_RES);  /*Set smaller width to make the lines wrap*/
            lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

            // ESP_LOGW(TAG, "Calling SquareLine LVGL UI objects once at init. Sleep 5 sec.");
            // vTaskDelay(pdMS_TO_TICKS(15000));
            // lv_label_set_text(ui_Label1, "START");
            // ESP_LOGW(TAG, "Called SquareLine LVGL UI objects once at init. Sleep 5 sec.");
            // vTaskDelay(pdMS_TO_TICKS(25000));

            counter++;
        }
    }
}

void app_main() {

    vTaskDelay(pdMS_TO_TICKS(1000));

    TaskHandle_t taskHandle = NULL;
    BaseType_t res = xTaskCreatePinnedToCore(lvgl_task, "LVGL task", 8192, NULL, 4, &taskHandle, 0); // stack, params, prio, handle, core

    while(true) {

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// By https://forum.lvgl.io/t/gestures-are-slow-perceiving-only-detecting-one-of-5-10-tries/18515/62