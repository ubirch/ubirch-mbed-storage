/*
 * nRF52 flash storage library tests.
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
#include "FlashStorage.h"
#include <BLE.h>
#include <nrf52_bitfields.h>

#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"

using namespace utest::v1;

void TestTrue() {
    TEST_ASSERT_TRUE_MESSAGE(true, "this is just to make it work");
}

void TestStorageWriteSubsequentBytes() {
    uint32_t location = 0x00;
    const uint8_t writeData[16] = {0xA1, 0xB2, 0xC3, 0xD4,
                                   0xE5, 0xF6, 0x07, 0x18,
                                   0x29, 0x3A, 0x4B, 0x5C,
                                   0x6D, 0x7E, 0x8F, 0x90};
    uint8_t readData[16] = {0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00};
    int index = 0;
    uint16_t number = 1;

    for(index = 0; index < 8; index ++){
        for( number = 1; number < 16 - index; number++){

            printf(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"
                           "testing loc = [0x%X],  index = [%u], number = [%u]\r\n", location, index, number);
            TEST_ASSERT_TRUE_MESSAGE(ks_write_data((uint32_t)(location + index), (const unsigned char *) &writeData[index], number),
                                     "failed to write to storage");
            TEST_ASSERT_TRUE_MESSAGE(ks_read_data(location, (unsigned char *) readData, sizeof(readData)),
                                     "failed to read from storage");
            TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(&writeData[index], &readData[index], number/* sizeof(writeData)*/,
                                                 "data read does not match written data");
            location += number;
        }
    }
}

utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
    greentea_case_failure_abort_handler(source, reason);
    return STATUS_CONTINUE;
}

Case cases[] = {
Case("Storage test true-0", TestTrue, greentea_failure_handler),
Case("Storage test storage write subsequent bytes-0", TestStorageWriteSubsequentBytes, greentea_failure_handler),
};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
    static bool initialized = false;
    if (!initialized) {
        TEST_ASSERT_TRUE_MESSAGE(ks_init(), "failed to initialze storage");
        initialized = true;
    }
    TEST_ASSERT_TRUE_MESSAGE(ks_erase_page(), "failed to erase page");
    GREENTEA_SETUP(150, "default_auto");
    return greentea_test_setup_handler(number_of_cases);
}

void startTests(BLE::InitializationCompleteCallbackContext *params) {
    Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);
    Harness::run(specification);
}

static Thread bleEventThread(osPriorityNormal, 24000);
static EventQueue bleEventQueue(/* event count */ 16 * EVENTS_EVENT_SIZE);

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext *context) {
    bleEventQueue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

int main() {
    wait_ms(100);

    // set the storage address (exclude bootloader area)
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
    NRF_UICR->NRFFW[0] = 0x7A000;
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}

    // enable external clock mode
    DigitalOut extClkPin(P0_27, 0);

    extClkPin = 0;
    wait_ms(100);
    extClkPin = 1;
    wait_ms(100);

    /* Mark the HF clock as not started */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    /* Try to start the HF clock */
    NRF_CLOCK->TASKS_HFCLKSTART = 1;

    //Make sure HFCLK is on
    //TODO use a while loop and reset module if it fails to init HFCLK
    for (int i = 0; i < 5; i++) {
        if (NRF_CLOCK->EVENTS_HFCLKSTARTED) {
            break;
        }
        wait(1);
    }

    // configure BLE to ensure events are handled
    bleEventThread.start(callback(&bleEventQueue, &EventQueue::dispatch_forever));

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(startTests);
}