# MCH2022 Custom RP2040 App

This repository contains an example app that loads a new custom RP2040 firmware
alongside the stock firmware. When you reboot your badge, the original firmware
will be booted again.

## License

The source code contained in the main folder of this example is public domain / CC0 licensed, use it as you please.

Source code included as submodules is licensed separately, please check the
following table for details.

| Submodule                   | License                           | Author                                                 |
|-----------------------------|-----------------------------------|--------------------------------------------------------|
| esp-idf                     | Apache License 2.0                | Espressif Systems (Shanghai) CO LTD                    |
| components/appfs            | THE BEER-WARE LICENSE Revision 42 | Jeroen Domburg <jeroen@spritesmods.com>                |
| components/bus-i2c          | MIT                               | Nicolai Electronics                                    |
| components/i2c-bno055       | MIT                               | Nicolai Electronics                                    |
| components/mch2022-rp2040   | MIT                               | Renze Nicolai                                          |
| components/pax-graphics     | MIT                               | Julian Scheffers                                       |
| components/pax-keyboard     | MIT                               | Julian Scheffers                                       |
| components/sdcard           | MIT                               | Nicolai Electronics                                    |
| components/spi-ice40        | MIT                               | Nicolai Electronics                                    |
| components/spi-ili9341      | MIT                               | Nicolai Electronics                                    |
| components/ws2812           | Unlicense / Public domain         | None                                                   |


Source the `update_components.sh` to update all the submodules to their
corresponding tips.
