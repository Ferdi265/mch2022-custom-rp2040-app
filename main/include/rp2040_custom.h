#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "rp2040.h"
#include "pax_gfx.h"
#include "ili9341.h"

void rp2040_custom_run(RP2040* rp2040, pax_buf_t* pax_buffer, ILI9341* ili9341);
_Noreturn void rp2040_reboot_to_stock(RP2040* rp2040);
