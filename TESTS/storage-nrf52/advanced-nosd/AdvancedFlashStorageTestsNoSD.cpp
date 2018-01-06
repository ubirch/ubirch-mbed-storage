/*
 * nRF52 flash storage library tests.
 *
 * @author Matthias L. Jugel, Waldemar Grünwald
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

#include "utest/utest.h"
#include "greentea-client/test_env.h"

#include "../AdvancedFlashStorageTests.h"

using namespace utest::v1;

utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) { // NOLINT
    greentea_case_failure_abort_handler(source, reason);
    return STATUS_CONTINUE;
}


utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
    GREENTEA_SETUP(150, "default_auto");
    return greentea_test_setup_handler(number_of_cases);
}

Case cases[] = {
        Case("Storage [noSD] test storage",
             TestStorage, greentea_failure_handler),
        Case("Storage [noSD] test storage write subsequent bytes",
             TestStorageWriteSubsequentBytes, greentea_failure_handler),
        Case("Storage [noSD] test storage write byte above end address",
             TestStorageWriteAboveEndAddress, greentea_failure_handler),
        Case("Storage [noSD] test storage write buffer over page boarder",
             TestStorageWriteOverPageBoarder, greentea_failure_handler),
        Case("Storage [noSD] test storage write big buffer",
             TestStorageWriteBigBuffer, greentea_failure_handler),
        Case("Storage [noSD] test storage erase pages",
             TestStorageErasePages, greentea_failure_handler),
        Case("Storage [noSD] test storage write over the upper bound",
             TestStorageWriteOverUpperBound, greentea_failure_handler),

};

int main() {
    // set the storage address (exclude bootloader area)
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
    NRF_UICR->NRFFW[0] = 0x7A000;
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}

    NRF52FlashStorage flashStorage;
    static bool initialized = false;
    if (!initialized) {
        flashStorage.init();
        initialized = true;
    }
    flashStorage.erasePage(0, NUM_PAGES);

    Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);
    Harness::run(specification);
}
