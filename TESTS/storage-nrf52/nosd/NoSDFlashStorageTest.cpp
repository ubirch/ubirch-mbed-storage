/*
 * @file NoSDFlashStorageTest.cpp
 *
 * Test that flash storage works with direct chip erase functions.
 *
 * @author Matthias L. Jugel
 * @date 2017-09-27
 *
 * Copyright 2017 ubirch GmbH (https://ubirch.com)
 *
 * ```
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ```
 */

#include "mbed.h"
#include <nrf52_bitfields.h>
#include <NRF52FlashStorage.h>

#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"

#ifndef NUM_PAGES
#define NUM_PAGES   1
#endif

using namespace utest::v1;


void TestStorageWriteSimple() {
    NRF52FlashStorage flashStorage;
    flashStorage.init();

    TEST_ASSERT_TRUE(flashStorage.erasePage(0, NUM_PAGES));

    const uint32_t expected = 0x12345678;
    uint32_t data = 0;

    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData(0, (unsigned char *) &expected, 4), "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(0, (unsigned char *) &data, 4), "failed to read from storage");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, data, "data read does not match written data");
}

unsigned char expected[4096];

void TestStorageWritePage() {
    NRF52FlashStorage flashStorage;
    flashStorage.init();

    TEST_ASSERT_TRUE(flashStorage.erasePage(0, NUM_PAGES));

    for (int i = 0; i < sizeof(expected); i++) expected[i] = static_cast<uint8_t>(random());
    for (int i = 0; i < sizeof(expected) / 128; i++) {
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData(128 * i, (unsigned char *) expected + 128 * i, 128),
                                 "failed to write to storage");
    }

    uint8_t data[128];
    for (int i = 0; i < sizeof(expected) / 128; i++) {
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(128 * i, data, sizeof(data)),
                                 "failed to read data from storage");
        TEST_ASSERT_EQUAL_HEX8_ARRAY(expected + 128 * i, data, sizeof(data));
    }

}

utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) { // NOLINT
    greentea_case_failure_abort_handler(source, reason);
    return STATUS_CONTINUE;
}

Case cases[] = {
        Case("Storage [nosd] test write simple", TestStorageWriteSimple, greentea_failure_handler),
        Case("Storage [nosd] test write page", TestStorageWritePage, greentea_failure_handler),

};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
    GREENTEA_SETUP(150, "default_auto");
    return greentea_test_setup_handler(number_of_cases);
}

int main() {
    // set the storage address (exclude bootloader area)
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
    NRF_UICR->NRFFW[0] = 0x7A000;
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}

    Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);
    Harness::run(specification);
}
