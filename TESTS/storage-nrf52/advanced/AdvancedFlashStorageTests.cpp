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
#include <BLE.h>
#include <nrf52_bitfields.h>
#include <NRF52FlashStorage.h>

#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"

using namespace utest::v1;

static Thread bleEventThread(osPriorityNormal, 24000);
static EventQueue bleEventQueue(/* event count */ 16 * EVENTS_EVENT_SIZE);

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext *context) {
    bleEventQueue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

void continueTests(BLE::InitializationCompleteCallbackContext *params);

int initSd() {
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    return ble.init(continueTests);
}

int deinitSd() {

    if (BLE::Instance().hasInitialized()) {
        BLE::Instance().shutdown();
    }
    return 0;
}

struct __attribute__ ((packed)) Entry {
    time_t timestamp;       //!< the timestamp of the measurement
    uint16_t temperature;   //!< the temperature in degrees celsius
};

bool operator==(const Entry &lhs, const Entry &rhs) {
    return lhs.timestamp == rhs.timestamp && lhs.temperature == rhs.temperature;
}

//NRF52FlashStorage flashStorage;

void TestStorageWriteSubsequentBytes() {
    NRF52FlashStorage flashStorage;
    uint32_t location = 0x00;
    const uint8_t writeData[16] = {0xA1, 0xB2, 0xC3, 0xD4,
                                   0xE5, 0xF6, 0x07, 0x18,
                                   0x29, 0x3A, 0x4B, 0x5C,
                                   0x6D, 0x7E, 0x8F, 0x90};
    uint8_t readData[16] = {0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00};

    for (int index = 0; index < 1; index++) {
        for (uint16_t number = 1; number < 16 - index; number++) {

            printf(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"
                           "testing loc = [0x%X],  index = [%u], number = [%u]\r\n", location, index, number);
            TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData((uint32_t)(location + index), (const unsigned char *) &writeData[index], number),
                                     "failed to write to storage");
            TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(location, (unsigned char *) readData, sizeof(readData)),
                                     "failed to read from storage");
            TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(&writeData[index], &readData[index], number/* sizeof(writeData)*/,
                                                 "data read does not match written data");
            location += number;
        }
    }
}

void TestStorageWriteAboveEndAddress(){
    NRF52FlashStorage flashStorage;
    uint32_t location;
    const uint8_t writeByte = 0xEA;
    uint8_t readByte = 0x00;

    // get the end location of the storage
    location = flashStorage.getEndAddress() - flashStorage.getStartAddress();

    printf("#####\r\n start:(%X) end:(%X) loc:(%X) \r\n#####\r\n", flashStorage.getStartAddress(), flashStorage.getEndAddress(), location);
    TEST_ASSERT_TRUE_MESSAGE(!flashStorage.writeData(location, (const unsigned char *)(&writeByte), sizeof(writeByte)),
                             "failed to not write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(location, (unsigned char *) (&readByte), sizeof(readByte)),
                             "failed to read from storage");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(writeByte, readByte, "data read does match written data");
}

/*!
 * @note    This test fails, if only one page is reserved
 */
void TestStorageWriteOverPageBoarder(){
    NRF52FlashStorage flashStorage;
    uint32_t location = 0x1000 - 0x08;
    const uint8_t writeData[16] = {0xA1, 0xB2, 0xC3, 0xD4,
                                   0xE5, 0xF6, 0x07, 0x18,
                                   0x29, 0x3A, 0x4B, 0x5C,
                                   0x6D, 0x7E, 0x8F, 0x90};
    uint8_t readData[16] = {0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData((uint32_t)(location), (const unsigned char *) writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(location, (unsigned char *) readData, sizeof(readData)),
                             "failed to read from storage");

    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readData, sizeof(writeData)/* sizeof(writeData)*/,
                                         "data read does not match written data");
}
/*!
 * @note this test fails if the number of pages < 3
 */
void TestStorageWriteBigBuffer(){
    NRF52FlashStorage flashStorage;
    uint16_t length = 0x200;
    uint32_t location = 0x2000 - (length >> 1);
    uint8_t writeData[length];
    uint8_t readData[length];

    for (int i = 0; i < length; i++) {
        writeData[i] =(uint8_t)(i & 0xFF);
    }
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData((uint32_t)(location), (const unsigned char *) writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(location, (unsigned char *) readData, sizeof(readData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readData, sizeof(writeData)/* sizeof(writeData)*/,
                                         "data read does not match written data");
}

void TestStorageErasePages(){
    NRF52FlashStorage flashStorage;
    uint32_t location = 0;
    uint8_t writeByte = 0x5A;
    uint8_t readByte1 = 0x00;
    uint8_t readByte2 = 0x00;
    uint8_t emptyData = 0xFF;
    uint32_t randomAddr = 0;

    for (int numPages = 0; numPages < STORAGE_PAGES; numPages++) {
        randomAddr = (rand() & 0x0FFF);
        printf(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"
                       "testing erase page = [%X],  address = [0x%X]\r\n", numPages, (location + randomAddr));
        // first write one byte and read it to check if data is in the storage
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData((location + randomAddr), (const unsigned char *) (&writeByte), sizeof(writeByte)),
                                 "failed to write to storage");
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData((location + randomAddr), (unsigned char *) (&readByte1), sizeof(readByte1)),
                                 "failed to read from storage");
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(writeByte, readByte1, "data read does not match written data");

        // now erase the page and check if the page is completely empty (0xFF)
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.erasePage((uint8_t)(numPages)), "page not erased");
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData((location + randomAddr), (unsigned char *) (&readByte2), sizeof(readByte1)),
                                 "failed to read from storage");
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(readByte2, emptyData, "data read does not match written data");
        location += 0x1000;
    }
}

void TestStorageWriteOverUpperBound(){
    NRF52FlashStorage flashStorage;
    uint16_t length = 0x20;
    uint32_t location = STORAGE_PAGES * 0x1000 - (length >> 1);
    uint8_t writeData[length];
    uint8_t readData[length];

    for (int i = 0; i < length; i++) {
        writeData[i] =(uint8_t)(i & 0xFF);
    }
    TEST_ASSERT_TRUE_MESSAGE(!flashStorage.writeData((uint32_t)(location), (const unsigned char *) writeData, sizeof(writeData)),
                             "failed to recognize the upper bound of storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(location, (unsigned char *) readData, sizeof(readData)),
                             "failed to read from storage");
    for (int j = 0; j < length; j++) {
        TEST_ASSERT_NOT_EQUAL_MESSAGE(writeData[j], readData[j],
                                      "data was written over the upper bound");
    }
}

control_t TestStorageDisabledSd(const size_t n) {
    NRF52FlashStorage flashStorage;

    deinitSd();

    uint32_t location = 0x3000 - 0x40 + 16 * n;
    const uint8_t writeData[16] = {0xA1, 0xB2, 0xC3, 0xD4,
                                   0xE5, 0xF6, 0x07, 0x18,
                                   0x29, 0x3A, 0x4B, 0x5C,
                                   0x6D, 0x7E, 0x8F, 0x90};
    uint8_t readData[16] = {0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_TRUE_MESSAGE(
            flashStorage.writeData((uint32_t) (location), (const unsigned char *) writeData, sizeof(writeData)),
            "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(location, (unsigned char *) readData, sizeof(readData)),
                             "failed to read from storage");

    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readData, sizeof(writeData)/* sizeof(writeData)*/,
                                         "data read does not match written data");

    initSd();
    return n < 2 ? CaseRepeatAll : CaseNext;
}

void TestStorageErasePagesDisabledSd() {
    NRF52FlashStorage flashStorage;
    uint32_t location = 0;
    uint8_t writeByte = 0x5A;
    uint8_t readByte1 = 0x00;
    uint8_t readByte2 = 0x00;
    uint8_t emptyData = 0xFF;
    uint32_t randomAddr = 0;

    deinitSd();

    for (int numPages = 0; numPages < STORAGE_PAGES; numPages++) {
        randomAddr = (rand() & 0x0FFF);
        printf(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"
                       "testing erase page = [%X],  address = [0x%X]\r\n", numPages, (location + randomAddr));
        // erase the page and check if the page is completely empty (0xFF)
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.erasePage((uint8_t) (numPages)), "page not erased");
        // write one byte and read it to check if data is in the storage
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData((location + randomAddr), (const unsigned char *) (&writeByte),
                                                        sizeof(writeByte)),
                                 "failed to write to storage");
        TEST_ASSERT_TRUE_MESSAGE(
                flashStorage.readData((location + randomAddr), (unsigned char *) (&readByte1), sizeof(readByte1)),
                "failed to read from storage");
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(writeByte, readByte1, "data read does not match written data");

        // now erase the page and check if the page is completely empty (0xFF)
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.erasePage((uint8_t) (numPages)), "page not erased");
        TEST_ASSERT_TRUE_MESSAGE(
                flashStorage.readData((location + randomAddr), (unsigned char *) (&readByte2), sizeof(readByte1)),
                "failed to read from storage");
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(readByte2, emptyData, "data read does not match written data");
        location += 0x1000;
    }

    initSd();
}

void TestStorageStructWriteRead() {
    NRF52FlashStorage flashStorage;

    uint16_t address = STORAGE_PAGES * 1024 - 99;

    Entry EArray;
    EArray.temperature = 3655;
    EArray.timestamp = 0x12345678;
//    EArray[1]->temperature = 2100;
//    EArray[1]->timestamp = 0x87654321;

    Entry EArrayRead;

    // write one byte and read it to check if data is in the storage
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData((address), (const unsigned char *) (&EArray),
                                                    sizeof(EArray)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(
            flashStorage.readData((address), (unsigned char *) (&EArrayRead), sizeof(EArrayRead)),
            "failed to read from storage");
    TEST_ASSERT_TRUE_MESSAGE(EArray == EArrayRead,
                             "data read does not match written data");

}




utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
    greentea_case_failure_abort_handler(source, reason);
    return STATUS_CONTINUE;
}

Case cases[] = {
        Case("Storage test storage erase pages-0", TestStorageErasePages, greentea_failure_handler),
        Case("Storage test storage write subsequent bytes-0", TestStorageWriteSubsequentBytes,
             greentea_failure_handler),
        Case("Storage test storage write byte above end address-0", TestStorageWriteAboveEndAddress,
             greentea_failure_handler),
        Case("Storage test storage write buffer over page boarder-0", TestStorageWriteOverPageBoarder,
             greentea_failure_handler),
        Case("Storage test storage write big buffer-0", TestStorageWriteBigBuffer, greentea_failure_handler),
        Case("Storage test storage write without SD-0", TestStorageDisabledSd, greentea_failure_handler),
        Case("Storage test storage write over the upper bound-0", TestStorageWriteOverUpperBound,
             greentea_failure_handler),
        Case("Storage test storage erase pages without SD-0", TestStorageErasePagesDisabledSd,
             greentea_failure_handler),
        Case("Storage test storage write read Struct-0", TestStorageStructWriteRead,
             greentea_failure_handler),


};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
    NRF52FlashStorage flashStorage;
    static bool initialized = false;
    if (!initialized) {
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.init(), "failed to initialize storage");
        initialized = true;
    }
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.erasePage(0, STORAGE_PAGES), "failed to erase pages");
    GREENTEA_SETUP(150, "default_auto");
    return greentea_test_setup_handler(number_of_cases);
}

void startTests(BLE::InitializationCompleteCallbackContext *params) {
    Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);
    Harness::run(specification);
}

void continueTests(BLE::InitializationCompleteCallbackContext *params) {
    Harness::validate_callback();
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
