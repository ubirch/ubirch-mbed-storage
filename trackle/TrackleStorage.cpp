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
    readData(measOffset + address * sizeof(Entry), (unsigned char *) &entry, sizeof(Entry));
    return entry == EMPTY;
}

bool TrackleStorage::storeMeas(Entry &entry) {
    bool retval = false;
    // write data and advance
    PRINTF("E.ts : %08X E.t : %04X addr : %06X \r\n", entry.timestamp, entry.temperature,
           (measCounter * sizeof(Entry)));
//    if(!storageFullMeas()){
    if (writeData(measOffset + measCounter * sizeof(Entry), (const unsigned char *) &entry, sizeof(Entry))) {
        measCounter++;
        retval = true;
    } else {
        retval = false;
    }
//    }
    return retval;
}

Entry TrackleStorage::loadMeas(int address) {
    Entry entry = {};
    if (readData(measOffset + address * sizeof(Entry), (unsigned char *) &entry, sizeof(Entry))) {
        return entry;
    }
    return EMPTY;
}


bool TrackleStorage::eraseMeas(int page, int numPages) {
    /**
     * only delete the measurements, not the keys and not the settings
     */
    int measPageStart = pageOffsetMeas + page;
    if (erasePage(static_cast<uint8_t>(measPageStart),
                  (uint8_t) min(numPages, (STORAGE_PAGES - (int) pageOffsetMeas)))) {
        measCounter = 0;
        return true;
    }

    return false;
}

Settings TrackleStorage::loadSettings() {
    Settings settings = {};
    if (readData(settingsOffset, (unsigned char *) (&settings), sizeof(settings))) {
        return settings;
    }
    return EMPTYSETTINGS;
}


uint32_t TrackleStorage::scanSettings() {
    uint32_t location = 0xFFFF;
    uint32_t tempMagicNum[1];

    /**
     * scan from start of the page, to see, if there are settings stored.
     */
    for (int i = 0; i < PAGE_SIZE_BYTES; i += sizeof(uint32_t)) {
        PRINTF("SCANNING \r\n");
        if (readData(settingsOffset + i, (unsigned char *) tempMagicNum, sizeof(uint32_t))) {
            if (tempMagicNum[0] == SETTING_MAGIC_NUMBER_VALID) {
                PRINTF(" settings valid = %d\r\n", i);
                location = (uint32_t) i;
                break;
            }
        }
    }
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
        // check if the new settings can be stored
        if (writeData(settingsOffset + settingsLoc, (const unsigned char *) &settings, settings.length)) {
            retval = true;
        } else { PRINTF("!#! NEW SETTINGS NOT STORED\r\n"); }
    } // get the length of the valid settings
    else if (readData(settingsOffset + settingsLoc, settingsHeader, sizeof(settingsHeader))) {
        uint16_t settingsLength = (uint16_t) settingsHeader[4] << 8 + settingsHeader[5];
        PRINTF("settings header = ");
        for (int i = 0; i < sizeof(settingsHeader); ++i) {
            PRINTF(" %02x", settingsHeader[i]);
        }
        PRINTF("\r\n");
        PRINTF("old settings length = %d\r\n", settingsLength);
        // check if the remaining space is sufficient for new settings
        if ((settingsLoc + settingsLength + settings.length) <= PAGE_SIZE_BYTES) {
            uint32_t settingsFreeAfter;
            // check if the space afterwards is free
            if (readData(settingsOffset + settingsLoc + settingsLength, (unsigned char *) &settingsFreeAfter,
                         sizeof(settingsFreeAfter))) {
                if (settingsFreeAfter == 0xFFFFFFFF) {
                    // check if the new settings can be stored
                    if (writeData(settingsOffset + settingsLoc + settingsLength, (const unsigned char *) &settings,
                                  settings.length)) {
                        if (writeData(settingsOffset + settingsLoc + 3, invalid, sizeof(invalid))) {
                            retval = true;
                        } else { PRINTF("!#! OLD SETTINGS NOT INVALIDATED\r\n"); }
                    } else { PRINTF("!#! NEW SETTINGS NOT STORED\r\n"); }

                } else { PRINTF("!#! SPACE NOT EMPTY \r\n"); }
            } else retval = false;
        } else {
            //not enough space for new settings
            PRINTF("NOT ENOUGH SPACE FOR NEW SETTINGS\r\n");
        }
    } else retval = false;
//    /**
//     * store the settings
//     */
//    if (!writeData(settingsOffset, (const unsigned char *) &settings, settings.length)) {
//        retval = false;
//    }
    return retval;
}


bool TrackleStorage::eraseSettings() {
    return erasePage(pageOffsetSettings, 1);
}

bool TrackleStorage::replaceSettings(Settings settings) {
    bool retval = true;
    Settings tempSettings = loadSettings();
    /**
     * overwrite the setting, which are not 0
     */
    if (settings.magicNumber) { tempSettings.magicNumber = settings.magicNumber; }
    if (settings.length) { tempSettings.length = settings.length; }
    if (settings.param1) { tempSettings.param1 = settings.param1; }
    if (settings.param2) { tempSettings.param2 = settings.param2; }
    if (settings.param3) { tempSettings.param3 = settings.param3; }
    if (settings.param4) { tempSettings.param4 = settings.param4; }
    if (settings.param5) { tempSettings.param5 = settings.param5; }
    if (settings.param6) { tempSettings.param6 = settings.param6; }
    if (settings.param7) { tempSettings.param7 = settings.param7; }
    if (settings.param8) { tempSettings.param8 = settings.param8; }
    if (settings.param9) { tempSettings.param9 = settings.param9; }
    if (settings.param10) { tempSettings.param10 = settings.param10; }
    /**
     * store the settings
     */
    if (eraseSettings()) {
        if (storeSettings(tempSettings)) {
            PRINTF("replacing successful\r\n");
        } else { retval = false; }
    } else { retval = false; }
    return retval;
}

bool TrackleStorage::storeKeys() {

    return false;
}

bool TrackleStorage::loadKeys() {
    return false;
}

bool TrackleStorage::eraseKeys() {
    return false;
}

bool TrackleStorage::replaceKeys() {
    return false;
}

//bool TrackleStorage::storageFullMeas() {
//    bool retval = false;
//    uint32_t biggestMeasAddress = (STORAGE_PAGES - 1) * PAGE_SIZE_BYTES;
//    uint32_t currentAddress = measOffset + ((measCounter + 1) * sizeof(Entry));
//    if (currentAddress >= biggestMeasAddress){
//        retval = true;
//        PRINTF("###\r\nstorage full \r\n###\r\n");
//    }
//    return retval;
//}


