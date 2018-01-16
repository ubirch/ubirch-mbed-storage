# Flash Storage

An mbed OS library to store data in non-volatile memory. Requires some
setup to ensure space is available or alternatively, you know your
free areas.

Wraps the chip and mbed specific implementations in an easy to use
library.

To add this library to your project:

```bash
mbed add https://github.com/ubirch/ubirch-mbed-nrf52-storage
```

## Nordic nRF52832

These functions allow the handling of the non-volatile data storage in
the flash memory of the MCU.

## Testing

```bash
mbed new .
mbed target NRF52_DK
mbed toolchain GCC_ARM
mbed test -n tests-storage-nrf52*
```

Current Status:

```
+------------------+---------------+-----------------------------------+--------+--------------------+-------------+
| target           | platform_name | test suite                        | result | elapsed_time (sec) | copy_method |
+------------------+---------------+-----------------------------------+--------+--------------------+-------------+
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced      | OK     | 23.73              | default     |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced-nosd | OK     | 25.62              | default     |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic         | OK     | 21.35              | default     |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic-nosd    | OK     | 21.82              | default     |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-nosd          | OK     | 18.18              | default     |
+------------------+---------------+-----------------------------------+--------+--------------------+-------------+
mbedgt: test suite results: 5 OK
mbedgt: test case report:
+------------------+---------------+-----------------------------------+------------------------------------------------------------+--------+--------+--------+--------------------+
| target           | platform_name | test suite                        | test case                                                  | passed | failed | result | elapsed_time (sec) |
+------------------+---------------+-----------------------------------+------------------------------------------------------------+--------+--------+--------+--------------------+
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced      | Storage test storage                                       | 2      | 0      | OK     | 0.06               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced      | Storage test storage erase pages                           | 1      | 0      | OK     | 0.65               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced      | Storage test storage write big buffer                      | 1      | 0      | OK     | 0.08               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced      | Storage test storage write buffer over page boarder        | 1      | 0      | OK     | 0.09               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced      | Storage test storage write byte above end address          | 1      | 0      | OK     | 0.14               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced      | Storage test storage write over the upper bound            | 1      | 0      | OK     | 0.08               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced      | Storage test storage write subsequent bytes                | 1      | 0      | OK     | 2.19               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced-nosd | Storage [noSD] test storage                                | 2      | 0      | OK     | 0.05               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced-nosd | Storage [noSD] test storage erase pages                    | 1      | 0      | OK     | 0.62               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced-nosd | Storage [noSD] test storage write big buffer               | 1      | 0      | OK     | 0.09               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced-nosd | Storage [noSD] test storage write buffer over page boarder | 1      | 0      | OK     | 0.09               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced-nosd | Storage [noSD] test storage write byte above end address   | 1      | 0      | OK     | 0.14               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced-nosd | Storage [noSD] test storage write over the upper bound     | 1      | 0      | OK     | 0.09               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-advanced-nosd | Storage [noSD] test storage write subsequent bytes         | 1      | 0      | OK     | 2.2                |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic         | Storage [SD] test storage write 1.5 words                  | 1      | 0      | OK     | 0.08               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic         | Storage [SD] test storage write 2 words                    | 1      | 0      | OK     | 0.07               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic         | Storage [SD] test storage write 3 byte                     | 1      | 0      | OK     | 0.07               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic         | Storage [SD] test storage write buffer                     | 1      | 0      | OK     | 0.08               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic         | Storage [SD] test storage write byte                       | 1      | 0      | OK     | 0.07               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic         | Storage [SD] test storage write existing fails             | 1      | 0      | OK     | 0.08               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic         | Storage [SD] test storage write half word                  | 1      | 0      | OK     | 0.07               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic         | Storage [SD] test storage write non-aligned                | 1      | 0      | OK     | 0.06               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic         | Storage [SD] test storage write word                       | 1      | 0      | OK     | 0.06               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic-nosd    | Storage [noSD] test storage write 1.5 words                | 1      | 0      | OK     | 0.08               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic-nosd    | Storage [noSD] test storage write 2 words                  | 1      | 0      | OK     | 0.07               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic-nosd    | Storage [noSD] test storage write 3 byte                   | 1      | 0      | OK     | 0.06               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic-nosd    | Storage [noSD] test storage write buffer                   | 1      | 0      | OK     | 0.08               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic-nosd    | Storage [noSD] test storage write byte                     | 1      | 0      | OK     | 0.08               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic-nosd    | Storage [noSD] test storage write existing fails           | 1      | 0      | OK     | 0.07               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic-nosd    | Storage [noSD] test storage write half word                | 1      | 0      | OK     | 0.07               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic-nosd    | Storage [noSD] test storage write non-aligned              | 1      | 0      | OK     | 0.07               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic-nosd    | Storage [noSD] test storage write word                     | 1      | 0      | OK     | 0.06               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-nosd          | Storage [nosd] test write page                             | 1      | 0      | OK     | 0.16               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-nosd          | Storage [nosd] test write simple                           | 1      | 0      | OK     | 0.08               |
+------------------+---------------+-----------------------------------+------------------------------------------------------------+--------+--------+--------+--------------------+
mbedgt: test case results: 34 OK
```

## TODO

- add automated tests on dev kit hardware


# License

This library is available under the [Apache License](LICENSE)

```
Copyright 2017 ubirch GmbH

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
````


