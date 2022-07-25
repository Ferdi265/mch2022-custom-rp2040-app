/*
 * This example code is in the Public Domain (or CC0 licensed, at your option.)
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

// This file contains a simple Hello World app which you can base you own
// native Badge apps on.

#include "main.h"
#include "rp2040_custom.h"

static pax_buf_t buf;
xQueueHandle buttonQueue;

#include <esp_log.h>
static const char *TAG = "custom-rp2040-app";

// Exits the app, returning to the launcher.
void exit_to_launcher() {
    REG_WRITE(RTC_CNTL_STORE0_REG, 0);
    esp_restart();
}

void app_main() {
    ESP_LOGI(TAG, "Starting RP2040 Custom Firmware App");

    // Initialize the screen, the I2C and the SPI busses.
    bsp_init();

    // Initialize the RP2040 (responsible for buttons, etc).
    bsp_rp2040_init();

    // This queue is used to receive button presses.
    buttonQueue = get_rp2040()->queue;

    // Initialize graphics for the screen.
    pax_buf_init(&buf, NULL, 320, 240, PAX_BUF_16_565RGB);

    // Initialize NVS.
    nvs_flash_init();

    // Initialize WiFi. This doesn't connect to Wifi yet.
    wifi_init();

    rp2040_custom_run(get_rp2040(), &buf, get_ili9341());

    pax_center_text(&buf, 0xFF000000, pax_font_saira_regular, 18, buf.width / 2, buf.height - 24, "Press Home to exit");

    while (1) {
        rp2040_input_message_t message;
        xQueueReceive(buttonQueue, &message, portMAX_DELAY);

        if (message.input == RP2040_INPUT_BUTTON_HOME && message.state) {
            rp2040_reboot_to_stock(get_rp2040());
        }
    }
}
