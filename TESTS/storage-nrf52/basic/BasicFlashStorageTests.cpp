/*
 * SI7050 Sensor library tests.
 *
 * @author Waldemar Grünwald
 * @date 2017-07-25
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
#include "rtos.h"
#include "FlashStorage.h"
#include <BLE.h>
#include <nrf52_bitfields.h>

#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"

using namespace utest::v1;

void TestStorageWriteWord() {
    const uint32_t writeData = 0xA1B2C3D4;
    uint32_t readData = 0x000000;

    TEST_ASSERT_TRUE_MESSAGE(ks_write_data(0x00, (const unsigned char *) &writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(ks_read_data(0x00, (unsigned char *) &readData, sizeof(writeData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(writeData, readData, "data read does not match written data");
}

void TestStorageWriteMultipleWords() {
    const uint8_t writeData[8] = {0xA1, 0xB2, 0xC3, 0xD4,
                                  0xE5, 0xF6, 0x07, 0x18};
    uint8_t readData[8] = {0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00};

    TEST_ASSERT_TRUE_MESSAGE(ks_write_data(0x01, (const unsigned char *) writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(ks_read_data(0x01, (unsigned char *) readData, sizeof(writeData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readData, sizeof(writeData),
                                         "data read does not match written data");
}

void TestStorageWriteSingleByte() {
    const uint8_t writeData = 0x2C;
    uint8_t readData = 0x00;

    TEST_ASSERT_TRUE_MESSAGE(ks_write_data(0x03, (const unsigned char *) &writeData, 1),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(ks_read_data(0x03, (unsigned char *) &readData, 1),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(writeData, readData, "data read does not match written data");
}

void TestStorageWriteHalfWord() {
    const uint8_t writeData[4] = {0xA1, 0xB2, 0xC3, 0xD4};
    const uint8_t readDataExpected[4] = {0xA1, 0xB2, 0x00, 0x00};
    uint8_t readData[4] = {0x00, 0x00, 0x00, 0x00};

    TEST_ASSERT_TRUE_MESSAGE(ks_write_data(0x04, (const unsigned char *) writeData, 2),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(ks_read_data(0x04, (unsigned char *) readData, 2),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readDataExpected, sizeof(readDataExpected),
                                         "data read does not match written data");
}

void TestStorageWriteThreeBytes() {
    const uint8_t writeData[3] = {0xB4, 0xC3, 0xD1};
    uint8_t readData[3] = {0x00, 0x00, 0x00};

    TEST_ASSERT_TRUE_MESSAGE(ks_write_data(0x05, (const unsigned char *) writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(ks_read_data(0x05, (unsigned char *) readData, sizeof(writeData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readData, sizeof(writeData),
                                         "data read does not match written data");
}

void TestStorageWriteWordAndHalfWord() {
    const uint8_t writeData[8] = {0xA1, 0xB2, 0xC3, 0xD4,
                                  0xE5, 0xF6, 0x07, 0x18};
    const uint8_t readDataExpected[8] = {0xA1, 0xB2, 0xC3, 0xD4,
                                         0xE5, 0xF6, 0x00, 0x00};
    uint8_t readData[8] = {0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00};

    TEST_ASSERT_TRUE_MESSAGE(ks_write_data(0x06, (const unsigned char *) writeData, 6),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(ks_read_data(0x06, (unsigned char *) readData, 6),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readDataExpected, sizeof(readDataExpected),
                                         "data read does not match written data");
}

void TestStorageWriteBuffer() {
    uint8_t writeData[12 * 4];
    uint8_t readData[12 * 4];

//    sd_rand_application_vector_get(writeData, sizeof(writeData));
    memset(writeData, 0xAD, 12 * 4);
    memset(readData, 0, sizeof(readData));


    TEST_ASSERT_TRUE_MESSAGE(ks_write_data(0x08, (const unsigned char *) writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(ks_read_data(0x08, (unsigned char *) readData, sizeof(writeData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readData, sizeof(writeData),
                                         "data read does not match written data");

}

void TestStorageWriteFailOnUsedFlash() {
    const uint32_t writeData = 0xA1B2C3D4;
    const uint32_t writeData2 = 0x4D3C2B1A;
    uint32_t readData = 0x000000;

    TEST_ASSERT_TRUE_MESSAGE(ks_write_data(0x14, (const unsigned char *) &writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(ks_read_data(0x14, (unsigned char *) &readData, sizeof(writeData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(writeData, readData, "data read does not match written data");

    // try to write again, the read should then still be the original writeData value
    TEST_ASSERT_TRUE_MESSAGE(ks_write_data(0x14, (const unsigned char *) &writeData2, sizeof(writeData2)),
                             "write operation failed");
    TEST_ASSERT_TRUE_MESSAGE(ks_read_data(0x14, (unsigned char *) &readData, sizeof(writeData2)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(writeData, readData, "data read does not match written data");

    // TODO: apparently, the 1 bis not yet zero will be zeroes by the second write
}

void TestStorageWriteNonAligned() {
    const uint32_t writeData = 0xA1B2C3D4;
    uint32_t readData = 0x000000;

    // TODO: the lib automatically aligned the address on 4 byte (address * 4)!!
    TEST_ASSERT_MESSAGE(false, "the lib auto-alignes, so this test is not possible!")
    TEST_ASSERT_TRUE_MESSAGE(ks_write_data(0x15, (const unsigned char *) &writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(ks_read_data(0x15, (unsigned char *) &readData, sizeof(writeData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(writeData, readData, "data read does not match written data");

}

utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
    greentea_case_failure_abort_handler(source, reason);
    return STATUS_CONTINUE;
}

Case cases[] = {
Case("Storage test storage write byte-0", TestStorageWriteSingleByte, greentea_failure_handler),
Case("Storage test storage write half word-0", TestStorageWriteHalfWord, greentea_failure_handler),
Case("Storage test storage write 3 byte-0", TestStorageWriteThreeBytes, greentea_failure_handler),
Case("Storage test storage write word-0", TestStorageWriteWord, greentea_failure_handler),
Case("Storage test storage write 2 words-0", TestStorageWriteMultipleWords, greentea_failure_handler),
Case("Storage test storage write 1.5 words-0", TestStorageWriteWordAndHalfWord, greentea_failure_handler),
Case("Storage test storage write buffer-0", TestStorageWriteBuffer, greentea_failure_handler),
Case("Storage test storage write existing fails-0", TestStorageWriteFailOnUsedFlash, greentea_failure_handler),
Case("Storage test storage write non-aligned-0", TestStorageWriteNonAligned, greentea_failure_handler),
};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
    static bool initialized = false;
    if (!initialized) {
        TEST_ASSERT_TRUE_MESSAGE(ks_init(), "failed to initialze storage");
        TEST_ASSERT_TRUE_MESSAGE(ks_erase_page(), "failed to erase page");
        initialized = true;
    }
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