/*!
 * @file
 * @brief AdvancedFlashStorageTests
 *
 * Advanced Flash Storage Test Functions
 *
 * @author Matthias L. Jugel
 * @date   2018-01-06
 *
 * @copyright &copy; 2018 ubirch GmbH (https://ubirch.com)
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
#ifndef UBIRCH_MBED_NRF52_STORAGE_ADVANCEDFLASHSTORAGETESTS_H
#define UBIRCH_MBED_NRF52_STORAGE_ADVANCEDFLASHSTORAGETESTS_H

#ifndef NUM_PAGES
#define NUM_PAGES   1
#endif

#include <utest/utest.h>
#include <unity/unity.h>
#include <NRF52FlashStorage.h>

using namespace utest::v1;

control_t TestStorage(const size_t n) {
    NRF52FlashStorage flashStorage;

    uint32_t location = (uint32_t) (0x3000 - 0x40 + 16 * n);
    const uint8_t writeData[16] = {0xA1, 0xB2, 0xC3, 0xD4,
                                   0xE5, 0xF6, 0x07, 0x18,
                                   0x29, 0x3A, 0x4B, 0x5C,
                                   0x6D, 0x7E, 0x8F, 0x90};
    uint8_t readData[16] = {0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData((uint32_t) (location),
                                                    (const unsigned char *) writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(location, (unsigned char *) readData, sizeof(readData)),
                             "failed to read from storage");

    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readData, sizeof(writeData)/* sizeof(writeData)*/,
                                         "data read does not match written data");

    return n < 2 ? CaseRepeatAll : CaseNext;
}

void TestStorageErasePages() {
    NRF52FlashStorage flashStorage;
    uint32_t location = 0;
    uint8_t writeByte = 0x5A;
    uint8_t readByte1 = 0x00;
    uint8_t readByte2 = 0x00;
    uint8_t emptyData = 0xFF;
    uint32_t randomAddr = 0;

    for (int numPages = 0; numPages < NUM_PAGES; numPages++) {
        randomAddr = (uint32_t) (random() & 0x0FFF);
        printf(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"
                       "testing erase page = [%X],  address = [0x%X]\r\n", numPages, (location + randomAddr));
        // first write one byte and read it to check if data is in the storage
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData((location + randomAddr), (const unsigned char *) (&writeByte),
                                                        sizeof(writeByte)),
                                 "failed to write to storage");
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData((location + randomAddr),
                                                       (unsigned char *) (&readByte1), sizeof(readByte1)),
                                 "failed to read from storage");
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(writeByte, readByte1, "data read does not match written data");

        // now erase the page and check if the page is completely empty (0xFF)
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.erasePage((uint8_t) (numPages), 1), "page not erased");
        TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData((location + randomAddr),
                                                       (unsigned char *) (&readByte2), sizeof(readByte1)),
                                 "failed to read from storage");
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(emptyData, readByte2, "data read does not match written data");
        location += 0x1000;
    }
}

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
            TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData((uint32_t) (location + index),
                                                            (const unsigned char *) &writeData[index], number),
                                     "failed to write to storage");
            TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(location, (unsigned char *) readData, sizeof(readData)),
                                     "failed to read from storage");
            TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(&writeData[index], &readData[index], number/* sizeof(writeData)*/,
                                                 "data read does not match written data");
            location += number;
        }
    }
}

void TestStorageWriteAboveEndAddress() {
    NRF52FlashStorage flashStorage;
    uint32_t location;
    const uint8_t writeByte = 0xEA;
    uint8_t readByte = 0x00;

    // get the end location of the storage
    location = flashStorage.getEndAddress() - flashStorage.getStartAddress();

    printf("#####\r\n start:(%X) end:(%X) loc:(%X) \r\n#####\r\n", flashStorage.getStartAddress(),
           flashStorage.getEndAddress(), location);
    TEST_ASSERT_TRUE_MESSAGE(!flashStorage.writeData(location, (const unsigned char *) (&writeByte), sizeof(writeByte)),
                             "failed to not write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(location, (unsigned char *) (&readByte), sizeof(readByte)),
                             "failed to read from storage");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(writeByte, readByte, "data read does match written data");
}

/*!
 * @note    This test fails, if only one page is reserved
 */
void TestStorageWriteOverPageBoarder() {
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
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData((uint32_t) (location),
                                                    (const unsigned char *) writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(location, (unsigned char *) readData, sizeof(readData)),
                             "failed to read from storage");

    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readData, sizeof(writeData)/* sizeof(writeData)*/,
                                         "data read does not match written data");
}


void TestStorageWriteOverUpperBound() {
    NRF52FlashStorage flashStorage;
    uint16_t length = 0x20;
    uint32_t location = (uint32_t) NUM_PAGES * 0x1000 - (length >> 1);
    uint8_t writeData[length];
    uint8_t readData[length];

    for (int i = 0; i < length; i++) {
        writeData[i] = (uint8_t) (i & 0xFF);
    }
    TEST_ASSERT_TRUE_MESSAGE(
            !flashStorage.writeData((uint32_t) (location), (const unsigned char *) writeData, sizeof(writeData)),
            "failed to recognize the upper bound of storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(location, (unsigned char *) readData, sizeof(readData)),
                             "failed to read from storage");
    for (int j = 0; j < length; j++) {
        TEST_ASSERT_NOT_EQUAL_MESSAGE(writeData[j], readData[j],
                                      "data was written over the upper bound");
    }
}

/*!
 * @note this test fails if the number of pages < 3
 */
void TestStorageWriteBigBuffer() {
    NRF52FlashStorage flashStorage;
    uint16_t length = 0x200;
    uint32_t location = (uint32_t) 0x2000 - (length >> 1);
    uint8_t writeData[length];
    uint8_t readData[length];

    for (int i = 0; i < length; i++) {
        writeData[i] = (uint8_t) (i & 0xFF);
    }
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData((uint32_t) (location),
                                                    (const unsigned char *) writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(location, (unsigned char *) readData, sizeof(readData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readData, sizeof(writeData)/* sizeof(writeData)*/,
                                         "data read does not match written data");
}

#endif //UBIRCH_MBED_NRF52_STORAGE_ADVANCEDFLASHSTORAGETESTS_H
