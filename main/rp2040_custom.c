#include "rp2040_custom.h"
#include "pax_codecs.h"
#include "rp2040bl.h"
#include "esp32/rom/crc.h"
#include "string.h"

#define RP2040_XIP_BASE 0x10000000
#define RP2040_BOOTLOADER_SIZE (64 * 1024UL)
#define RP2040_FIRMWARE_SIZE (64 * 1024UL)
#define RP2040_FIRMWARE_OFFSET RP2040_BOOTLOADER_SIZE
#define RP2040_CUSTOM_OFFSET (RP2040_BOOTLOADER_SIZE + RP2040_FIRMWARE_SIZE)
#define RP2040_FIRMWARE_ADDR (RP2040_XIP_BASE + RP2040_FIRMWARE_OFFSET)
#define RP2040_CUSTOM_ADDR (RP2040_XIP_BASE + RP2040_CUSTOM_OFFSET)

extern const uint8_t rpi_logo_png_start[] asm("_binary_rpi_logo_png_start");
extern const uint8_t rpi_logo_png_end[] asm("_binary_rpi_logo_png_end");

extern const uint8_t rp2040_custom_bin_start[] asm("_binary_rp2040_custom_bin_start");
extern const uint8_t rp2040_custom_bin_end[] asm("_binary_rp2040_custom_bin_end");

static void display_rp2040_custom_base(pax_buf_t* pax_buffer, ILI9341* ili9341) {
    pax_noclip(pax_buffer);
    const pax_font_t* tfont = pax_font_saira_condensed;
    pax_background(pax_buffer, 0xFFFFFF);

    pax_insert_png_buf(pax_buffer, rpi_logo_png_start, rpi_logo_png_end - rpi_logo_png_start, 10, 80, 0);
    pax_center_text(pax_buffer, 0xFF000000, tfont, 24, pax_buffer->width / 2, 40, "Launching Custom RP2040 Firmware");
}

static void display_rp2040_custom_log(pax_buf_t* pax_buffer, ILI9341* ili9341, const char* text1, const char* text2) {
    display_rp2040_custom_base(pax_buffer, ili9341);
    const pax_font_t* font = pax_font_saira_regular;

    pax_center_text(pax_buffer, 0xFF000000, font, 18, (pax_buffer->width - 105) / 2 + 105, pax_buffer->height / 2 + 4, text1);
    if (text2) pax_center_text(pax_buffer, 0xFF000000, font, 16, (pax_buffer->width - 105) / 2 + 105, pax_buffer->height / 2 + 28, text2);
    ili9341_write(ili9341, pax_buffer->buf);
}

static void display_rp2040_custom_error(pax_buf_t* pax_buffer, ILI9341* ili9341, const char* text1, const char* text2) {
    display_rp2040_custom_base(pax_buffer, ili9341);
    const pax_font_t* font = pax_font_saira_regular;

    pax_center_text(pax_buffer, 0xFFFF0000, font, 18, (pax_buffer->width - 105) / 2 + 105, pax_buffer->height / 2 + 4, text1);
    if (text2) pax_center_text(pax_buffer, 0xFFFF0000, font, 18, (pax_buffer->width - 105) / 2 + 105, pax_buffer->height / 2 + 28, text2);
    ili9341_write(ili9341, pax_buffer->buf);
}

void rp2040_custom_run(RP2040* rp2040, pax_buf_t* pax_buffer, ILI9341* ili9341) {
    display_rp2040_custom_log(pax_buffer, ili9341, "Initializing RP2040", "Rebooting to Bootloader");
    rp2040_reboot_to_bootloader(rp2040);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    display_rp2040_custom_log(pax_buffer, ili9341, "Initializing RP2040", "Reading Firmware Version");
    uint8_t fw_version;
    if (rp2040_get_firmware_version(rp2040, &fw_version) != ESP_OK) {
        display_rp2040_custom_error(pax_buffer, ili9341, "Failed to read", "firmware version");
        goto fail;
    } else if (fw_version != 0xFF) {
        display_rp2040_custom_error(pax_buffer, ili9341, "RP2040 not in", "bootloader mode");
        goto fail;
    }

    display_rp2040_custom_log(pax_buffer, ili9341, "Initializing RP2040", "Reading Bootloader Version");
    uint8_t bl_version;
    if (rp2040_get_bootloader_version(rp2040, &bl_version) != ESP_OK) {
        display_rp2040_custom_error(pax_buffer, ili9341, "Failed to read", "bootloader version");
        goto fail;
    } else if (bl_version != 0x02) {
        display_rp2040_custom_error(pax_buffer, ili9341, "Incorrect", "bootloader version");
        goto fail;
    }

    rp2040_bl_install_uart();
    rp2040_bl_sync();

    uint32_t flash_start = 0;
    uint32_t flash_size = 0;
    uint32_t erase_size = 0;
    uint32_t write_size = 0;
    uint32_t max_data_len = 0;
    if (!rp2040_bl_get_info(&flash_start, &flash_size, &erase_size, &write_size, &max_data_len)) {
        display_rp2040_custom_error(pax_buffer, ili9341, "Failed to read", "flash info");
        goto fail;
    }

    uint32_t firmware_size = rp2040_custom_bin_end - rp2040_custom_bin_start;
    if (firmware_size > flash_size - RP2040_CUSTOM_OFFSET) {
        display_rp2040_custom_error(pax_buffer, ili9341, "Custom firmware", "too large");
        goto fail;
    }

    display_rp2040_custom_log(pax_buffer, ili9341, "Checking Firmware", "Calculating CRC");
    uint32_t expected_crc = crc32_le(0, rp2040_custom_bin_start, firmware_size);
    uint32_t actual_crc = 0;
    if (!rp2040_bl_crc(RP2040_CUSTOM_ADDR, firmware_size, &actual_crc)) {
        display_rp2040_custom_error(pax_buffer, ili9341, "Failed to", "calculate CRC");
        goto fail;
    }

    if (expected_crc == actual_crc) {
        display_rp2040_custom_log(pax_buffer, ili9341, "Checking Firmware", "Checksum OK");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    } else {
        display_rp2040_custom_log(pax_buffer, ili9341, "Flashing Firmware", "Erasing...");
        vTaskDelay(500 / portTICK_PERIOD_MS);

        uint32_t firmware_size_erase = firmware_size + erase_size - (firmware_size % erase_size);
        if (!rp2040_bl_erase(RP2040_CUSTOM_ADDR, firmware_size_erase)) {
            display_rp2040_custom_error(pax_buffer, ili9341, "Failed to", "erase flash");
            goto fail;
        }

        uint32_t offset = 0;
        uint8_t * tx_buffer = malloc(write_size);
        uint8_t prev_percent = 0;

        while (offset < firmware_size) {
            uint32_t tx_size = firmware_size - offset;
            if (tx_size > write_size) tx_size = write_size;

            uint8_t percent = offset * 100 / firmware_size;
            if (percent != prev_percent) {
                char percent_str[5];
                snprintf(percent_str, sizeof percent_str, "%hhu%%", percent);
                display_rp2040_custom_log(pax_buffer, ili9341, "Flashing Firmware", percent_str);
            }

            memset(tx_buffer, 0, write_size);
            memcpy(tx_buffer, rp2040_custom_bin_start + offset, tx_size);
            uint32_t block_crc = crc32_le(0, tx_buffer, write_size);
            uint32_t check_crc = 0;
            if (!rp2040_bl_write(RP2040_CUSTOM_ADDR + offset, write_size, tx_buffer, &check_crc)) {
                display_rp2040_custom_log(pax_buffer, ili9341, "Failed to", "flash firmware");
                goto fail;
            } else if (block_crc != check_crc) {
                display_rp2040_custom_log(pax_buffer, ili9341, "Block CRC", "check failed");
                goto fail;
            }

            offset += tx_size;
        }

        free(tx_buffer);
    }

    display_rp2040_custom_log(pax_buffer, ili9341, "Launching Firmware", "Have fun :)");
    rp2040_bl_sync();
    rp2040_bl_go(RP2040_CUSTOM_ADDR);
    rp2040_bl_uninstall_uart();
    return;

fail:
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    rp2040_reboot_to_stock(rp2040);
}

_Noreturn void rp2040_reboot_to_stock(RP2040* rp2040) {
    rp2040_reboot_to_bootloader(rp2040);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    rp2040_bl_install_uart();
    rp2040_bl_sync();
    rp2040_bl_go(RP2040_FIRMWARE_ADDR);
    rp2040_bl_uninstall_uart();
    while (1) vTaskDelay(1000 / portTICK_PERIOD_MS);
}
