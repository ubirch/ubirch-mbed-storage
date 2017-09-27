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
# possibly fix mbed-os.lib!
mbed target NRF52_DK
mbed toolchain GCC_ARM
mbed test -n tests-storage-nrf52*
```

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


