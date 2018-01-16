/*!
 * @file
 * @brief TODO: ${FILE}
 *
 * ...
 *
 * @author Matthias L. Jugel
 * @date   2017-10-29
 *
 * @copyright &copy; 2017 ubirch GmbH (https://ubirch.com)
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
#include <algorithm>
//#include <edebug.h>
#include "TrackleStorage.h"

bool operator==(const Entry &lhs, const Entry &rhs) {
    return lhs.timestamp == rhs.timestamp && lhs.temperature == rhs.temperature;
}

bool TrackleStorage::init() {
    bool initOk = NRF52FlashStorage::init();
    PRINTF("Flash Start Address : %08X\r\n", NRF52FlashStorage::getStartAddress());
    PRINTF("Flash  End  Address : %08X\r\n", NRF52FlashStorage::getEndAddress());
    // do an initial storage survey and update current address
    while (!isEmptyMeas(measCounter)) measCounter++;
    PRINTF("storage seek: %08d [%08x]\r\n", measCounter, measCounter * (unsigned int) sizeof(Entry));
    return initOk;
}

int TrackleStorage::sizeMeas() {
    return measCounter;
}

bool TrackleStorage::isEmptyMeas(int address) {
    Entry entry = {};
    readData(measAddressOffset + address * sizeof(Entry), (unsigned char *) &entry, sizeof(Entry));
    return entry == EMPTY;
}

bool TrackleStorage::storeMeas(Entry &entry) {
    bool retval = false;
    // write data and advance
    PRINTF("E.ts : %08X E.t : %04X addr : %06X \r\n", entry.timestamp, entry.temperature,
           (measCounter * sizeof(Entry)));
//    if(!storageFullMeas()){
    if (writeData(measAddressOffset + measCounter * sizeof(Entry), (const unsigned char *) &entry, sizeof(Entry))) {
        measCounter++;
        retval = true;
    } else {
        retval = false;
    }
//    }
    return retval;
}

Entry TrackleStorage::loadMeas(int counter) {
    Entry entry = {};
    if (readData(measAddressOffset + counter * sizeof(Entry), (unsigned char *) &entry, sizeof(Entry))) {
        return entry;
    }
    return EMPTY;
}


bool TrackleStorage::eraseMeas(int page, int numPages) {
    /**
     * only delete the measurements, not the keys and not the settings
     */
    int measPageStart = measPageOffset + page;
    if (erasePage(static_cast<uint8_t>(measPageStart),
                  (uint8_t) min(numPages, (STORAGE_PAGES - (int) measPageOffset)))) {
        measCounter = 0;
        return true;
    }

    return false;
}

Settings TrackleStorage::loadSettings() {
    Settings settings = {};
    uint32_t location = scanSettings();
    if (location != 0xFFFF) {
        if (readData(settingsAddressOffset + settingsCurrentLocation, (unsigned char *) (&settings),
                     sizeof(settings))) {
            return settings;
        } else {}
    } else {}
    return EMPTYSETTINGS;
}


uint32_t TrackleStorage::scanSettings() {
    uint32_t location = 0xFFFF;
    uint32_t tempMagicNum[1];

    /**
     * scan from start of the page, to see, if there are settings stored.
     */
    for (int i = settingsCurrentLocation; i < PAGE_SIZE_BYTES; i += sizeof(uint32_t)) {
        PRINTF("SCANNING \r\n");
        if (readData(settingsAddressOffset + i, (unsigned char *) tempMagicNum, sizeof(uint32_t))) {
            if (tempMagicNum[0] == SETTING_MAGIC_NUMBER_VALID) {
                PRINTF(" settings valid = %d\r\n", i);
                location = (uint32_t) i;
                settingsCurrentLocation = (uint32_t) i;
                break;
            } else {}
        } else {}
    } //end for
    if (location == 0xFFFF) {
        for (int i = 0; i < settingsCurrentLocation; i += sizeof(uint32_t)) {
            PRINTF("SCANNING \r\n");
            if (readData(settingsAddressOffset + i, (unsigned char *) tempMagicNum, sizeof(uint32_t))) {
                if (tempMagicNum[0] == SETTING_MAGIC_NUMBER_VALID) {
                    PRINTF(" settings valid = %d\r\n", i);
                    location = (uint32_t) i;
                    settingsCurrentLocation = (uint32_t) i;
                    break;
                } else {}
            } else {}
        } //end for
    } else {};
    return location;
}

bool TrackleStorage::storeSettings(Settings &settings) {
    bool retval = false;
    const unsigned char invalid[1] = {0x00};
    unsigned char settingsHeader[6];
    // get the actual location of the valid settings
    uint32_t settingsLoc = scanSettings();
    if (settingsLoc == 0xFFFF) {
        PRINTF("!#! NO VALID SETTINGS\r\n");
        settingsLoc = 0;
        if (eraseSettings()) {
            // check if the new settings can be stored
            if (writeData(settingsAddressOffset + settingsLoc, (const unsigned char *) &settings, settings.length)) {
                retval = true;
            } else { PRINTF("!#! NEW SETTINGS NOT STORED\r\n"); }
        } else { PRINTF("!#! NEW SETTINGS NOT STORED\r\n"); }
    } // get the length of the valid settings
    else if (readData(settingsAddressOffset + settingsLoc, settingsHeader, sizeof(settingsHeader))) {
        uint16_t settingsLength = (uint16_t) (settingsHeader[5] << 8) + (uint16_t) (settingsHeader[4]);
        // check if the remaining space is sufficient for new settings
        if ((settingsLoc + settingsLength + settings.length) <= PAGE_SIZE_BYTES) {
            // check if the new settings can be stored
            if (writeData(settingsAddressOffset + settingsLoc + settingsLength, (const unsigned char *) &settings,
                          settings.length)) {
                if (writeData(settingsAddressOffset + settingsLoc, invalid, sizeof(invalid))) {
                    settingsCurrentLocation = settingsLoc + settingsLength;
                    retval = true;
                } else { PRINTF("!#! OLD SETTINGS NOT INVALIDATED\r\n"); }
            } else { PRINTF("!#! NEW SETTINGS NOT STORED\r\n"); }
        } else {
            //not enough space for new settings
            PRINTF("NOT ENOUGH SPACE FOR NEW SETTINGS\r\n");
            if (eraseSettings()) {
                settingsLoc = 0;
                if (writeData(settingsAddressOffset + settingsLoc, (const unsigned char *) &settings,
                              settings.length)) {
                    retval = true;
                } else { PRINTF("!#! NEW SETTINGS NOT STORED\r\n"); }
            } else {}
        }
    } else {}
    return retval;
}


bool TrackleStorage::eraseSettings() {
    return erasePage(settingsPageOffset, 1);
}



bool TrackleStorage::storeKeys() {

    return false;
}

CryptoKey TrackleStorage::loadKeys() {
    CryptoKey key = {};

    return key;
}

bool TrackleStorage::eraseKeys() {
    return false;
}

bool TrackleStorage::replaceKeys() {
    return false;
}



