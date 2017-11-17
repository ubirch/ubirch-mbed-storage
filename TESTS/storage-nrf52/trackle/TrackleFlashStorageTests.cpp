/*
 * nRF52 flash storage library tests.
 *
 * @author Waldemar Grünwald
 * @date 2017-11-15
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
#include <TrackleStorage.h>

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

// public trackleStorage Class

class publicTrackleStorage : public TrackleStorage {
public:
    /*!
     * Constructor for inherited object.
     */
    publicTrackleStorage() : TrackleStorage() {}

public:
    void setMeasCounter(unsigned int measCounter) {
        publicTrackleStorage::measCounter = measCounter;
    }

    unsigned int getMeasOffset() const {
        return measOffset;
    }

    unsigned int getSettingsOffset() const {
        return settingsOffset;
    }

protected:
    using TrackleStorage::measCounter;

    using TrackleStorage::measOffset;

    using TrackleStorage::pageOffsetMeas;

    using TrackleStorage::settingsOffset;

    using TrackleStorage::pageOffsetSettings;
};
//NRF52FlashStorage flashStorage;

void TestMeasurementStorageFull() {
    publicTrackleStorage storage;
    storage.init();

    TEST_ASSERT_TRUE_MESSAGE(storage.eraseMeas(0, STORAGE_PAGES), " failed to erase measurement storage");
    Entry measEntry = {};

    uint32_t spaceMeas = (STORAGE_PAGES * PAGE_SIZE_BYTES) - storage.getMeasOffset();
    PRINTF("spaceMeas = %d \r\n", spaceMeas);

    storage.setMeasCounter((spaceMeas / sizeof(Entry)) - 3);
    PRINTF("counter = %d \r\n", storage.sizeMeas());

    for (int i = 0; i < 5; i++) {
        measEntry.timestamp = 0x12345000 + i;
        measEntry.temperature = 0x7000 + i;

        if ((((storage.sizeMeas() + 1) * sizeof(Entry)) + storage.getMeasOffset()) <=
            (storage.getEndAddress() - storage.getStartAddress())) {
            PRINTF("STORAGE NOT FULL %d < %d\r\n",
                   (((storage.sizeMeas() + 1) * sizeof(Entry)) + storage.getMeasOffset()),
                   (storage.getEndAddress() - storage.getStartAddress()));
            TEST_ASSERT_TRUE_MESSAGE(storage.storeMeas(measEntry), "failed to write measurement");
        } else {
            PRINTF("STORAGE FULL %d > %d\r\n",
                   (((storage.sizeMeas() + 1) * sizeof(Entry)) + storage.getMeasOffset()),
                   (storage.getEndAddress() - storage.getStartAddress()));
            TEST_ASSERT_TRUE_MESSAGE(!storage.storeMeas(measEntry), "failed to identify full storage");
        }
    }
}

control_t TestSettingsWriteStorage(const size_t n) {
    publicTrackleStorage storage;
    storage.init();

//    TEST_ASSERT_TRUE_MESSAGE(storage.eraseSettings(), " failed to erase settings storage");

    Settings settings = {};
    settings.magicNumber = SETTING_MAGIC_NUMBER_VALID;
    settings.length = sizeof(settings);
    settings.param1 = 0x01010101;
    settings.param2 = 0x02020202;
    settings.param3 = 0x03030303;
    settings.param4 = 0x04040404;
    settings.param5 = 0x05050505;
    settings.param6 = 0x06060606;
    settings.param7 = 0x07070707;
    settings.param8 = 0x08080808;
    settings.param9 = 0x09090909;
    settings.param10 = 0xA0A0A0A0;

    TEST_ASSERT_TRUE_MESSAGE(storage.storeSettings(settings), " failed to store the settings");
    return n < 3 ? CaseRepeatAll : CaseNext;
}


utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
    greentea_case_failure_abort_handler(source, reason);
    return STATUS_CONTINUE;
}

Case cases[] = {
//        Case("Trackle Storage test mesurements full-0", TestMeasurementStorageFull, greentea_failure_handler),
        Case("Trackle Storage test settings write new-0", TestSettingsWriteStorage, greentea_failure_handler),

};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
    NRF52FlashStorage flashStorage;
    static bool initialized = false;
    if (!initialized) {
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.init(), "failed to initialize storage");
        initialized = true;
    }
//    TEST_ASSERT_TRUE_MESSAGE(flashStorage.erasePage(0, STORAGE_PAGES), "failed to erase pages");
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
