# Flash Storage

> **⚠ WIP: not all tests are working yet!**

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
+------------------+---------------+---------------------------+---------------------------------------------+--------+--------+--------+--------------------+
| target           | platform_name | test suite                | test case                                   | passed | failed | result | elapsed_time (sec) |
+------------------+---------------+---------------------------+---------------------------------------------+--------+--------+--------+--------------------+
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic | Storage test storage write 1.5 words-0      | 0      | 2      | FAIL   | 0.38               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic | Storage test storage write 2 words-0        | 1      | 0      | OK     | 0.2                |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic | Storage test storage write 3 byte-0         | 0      | 4      | FAIL   | 0.48               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic | Storage test storage write buffer-0         | 1      | 0      | OK     | 0.31               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic | Storage test storage write byte-0           | 0      | 2      | FAIL   | 0.29               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic | Storage test storage write existing fails-0 | 0      | 1      | FAIL   | 0.43               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic | Storage test storage write half word-0      | 0      | 3      | FAIL   | 0.4                |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic | Storage test storage write non-aligned-0    | 1      | 0      | OK     | 0.2                |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-storage-nrf52-basic | Storage test storage write word-0           | 1      | 0      | OK     | 0.2                |
+------------------+---------------+---------------------------+---------------------------------------------+--------+--------+--------+--------------------+
```

## TODO

- fix tests and library
- add automated tests on dev kit hardware
- link test results here


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


