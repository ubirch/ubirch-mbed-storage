/*!
 * @file
 * @brief BasicFlashStorageTests.h
 *
 * Basic Flash Storage Test Functions.
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
#ifndef UBIRCH_MBED_NRF52_STORAGE_BASICFLASHSTORAGETESTS_H
#define UBIRCH_MBED_NRF52_STORAGE_BASICFLASHSTORAGETESTS_H

#include <unity/unity.h>

#ifndef NUM_PAGES
#define NUM_PAGES   1
#endif

void TestStorageWriteWord() {
    NRF52FlashStorage flashStorage;
    const uint32_t writeData = 0xA1B2C3D4;
    uint32_t readData = 0x000000;

    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData (0x00, (const unsigned char *) &writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(0x00, (unsigned char *) &readData, sizeof(writeData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(writeData, readData, "data read does not match written data");
}

void TestStorageWriteMultipleWords() {
    NRF52FlashStorage flashStorage;
    const uint8_t writeData[8] = {0xA1, 0xB2, 0xC3, 0xD4,
                                  0xE5, 0xF6, 0x07, 0x18};
    uint8_t readData[8] = {0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00};

    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData(0x81, (const unsigned char *) writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(0x81, (unsigned char *) readData, sizeof(writeData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readData, sizeof(writeData),
                                         "data read does not match written data");
}

void TestStorageWriteSingleByte() {
    NRF52FlashStorage flashStorage;
    const uint8_t writeData = 0x2C;
    uint8_t readData = 0x00;

    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData(0x13, (const unsigned char *) &writeData, 1),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(0x13, (unsigned char *) &readData, 1),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(writeData, readData, "data read does not match written data");
}

void TestStorageWriteHalfWord() {
    NRF52FlashStorage flashStorage;
    const uint8_t writeData[4] = {0xA1, 0xB2, 0xC3, 0xD4};
    const uint8_t readDataExpected[4] = {0xA1, 0xB2, 0x00, 0x00};
    uint8_t readData[4] = {0x00, 0x00, 0x00, 0x00};

    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData(0x24, (const unsigned char *) writeData, 2),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(0x24, (unsigned char *) readData, 2),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(readData, readDataExpected, sizeof(readDataExpected),
                                         "data read does not match written data");
}

void TestStorageWriteThreeBytes() {
    NRF52FlashStorage flashStorage;
    const uint8_t writeData[3] = {0xB4, 0xC3, 0xD1};
    uint8_t readData[3] = {0x00, 0x00, 0x00};

    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData(0x35, (const unsigned char *) writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(0x35, (unsigned char *) readData, sizeof(writeData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readData, sizeof(writeData),
                                         "data read does not match written data");
}

void TestStorageWriteWordAndHalfWord() {
    NRF52FlashStorage flashStorage;
    const uint8_t writeData[8] = {0xA1, 0xB2, 0xC3, 0xD4,
                                  0xE5, 0xF6, 0x07, 0x18};
    const uint8_t readDataExpected[8] = {0xA1, 0xB2, 0xC3, 0xD4,
                                         0xE5, 0xF6, 0x00, 0x00};
    uint8_t readData[8] = {0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00};

    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData(0x46, (const unsigned char *) writeData, 6),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(0x46, (unsigned char *) readData, 6),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(readData, readDataExpected, sizeof(readDataExpected),
                                         "data read does not match written data");
}

void TestStorageWriteBuffer() {
    NRF52FlashStorage flashStorage;
    uint8_t writeData[12 * 4];
    uint8_t readData[12 * 4];

//    sd_rand_application_vector_get(writeData, sizeof(writeData));
    memset(writeData, 0xAD, 12 * 4);
    memset(readData, 0, sizeof(readData));


    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData(0x158, (const unsigned char *) writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(0x158, (unsigned char *) readData, sizeof(writeData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(writeData, readData, sizeof(writeData),
                                         "data read does not match written data");

}

void TestStorageWriteFailOnUsedFlash() {
    NRF52FlashStorage flashStorage;

    const uint32_t writeData = 0xA1B2C3D4;
    const uint32_t writeData2 = 0x4D3C2B1A;
    uint32_t readData = 0x000000;

    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData(0x64, (const unsigned char *) &writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(0x64, (unsigned char *) &readData, sizeof(writeData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(writeData, readData, "data read does not match written data");

    // try to write again, the read should then still be the original writeData value
    TEST_ASSERT_TRUE_MESSAGE(!flashStorage.writeData(0x64, (const unsigned char *) &writeData2, sizeof(writeData2)),
                             "write operation failed");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(0x64, (unsigned char *) &readData, sizeof(writeData2)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(writeData, readData, "data read does not match written data");

    // apparently, the 1 bits not yet zero will be zeroes by the second write
}

void TestStorageWriteNonAligned() {
    NRF52FlashStorage flashStorage;
    const uint32_t writeData = 0xA1B2C3D4;
    uint32_t readData = 0x000000;

    TEST_ASSERT_TRUE_MESSAGE(flashStorage.writeData(0x75, (const unsigned char *) &writeData, sizeof(writeData)),
                             "failed to write to storage");
    TEST_ASSERT_TRUE_MESSAGE(flashStorage.readData(0x75, (unsigned char *) &readData, sizeof(writeData)),
                             "failed to read from storage");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(writeData, readData, "data read does not match written data");

}

#endif //UBIRCH_MBED_NRF52_STORAGE_BASICFLASHSTORAGETESTS_H
