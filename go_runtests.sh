#! /bin/sh
TESTS='tests-storage*'
mbed new .
mbed target NRF52_DK
mbed toolchain GCC_ARM
mbed test --compile -n "$TESTS" --app-config TESTS/settings.json
mbedgt -n "$TESTS" --plain --report-junit=testresult.xml --report-memory-metrics-csv=testmem.csv