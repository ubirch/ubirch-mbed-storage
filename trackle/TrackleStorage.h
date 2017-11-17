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
#ifndef UBIRCH_TRACKLE_FIRMWARE_TRACKLESTORAGE_H
#define UBIRCH_TRACKLE_FIRMWARE_TRACKLESTORAGE_H

#include <mbed.h>
//#include <NRF52FlashStorage.h>
#include <ctime>
#include "../storage/NRF52FlashStorage.h"

#define MEASUREMENT_OFFSET (PAGE_SIZE_BYTES)
#define SETTINGS_SIZE 64


// magic numbers
#define SETTING_MAGIC_NUMBER_VALID      (uint32_t) 0xC927EDFF
#define SETTINGS_MAGIC_NUMBER_INVALID   (uint32_t) 0xC927ED00

/**
 * The storage description of our measurements.
 */
struct __attribute__ ((packed)) Entry {
    time_t timestamp;       //!< the timestamp of the measurement
    uint16_t temperature;   //!< the temperature in degrees celsius
};

/**
 * Empty Entry declaration
 */
const Entry EMPTY = {(long) 0xFFFFFFFF, 0xFFFF};


struct Settings {
    uint32_t magicNumber;   //!< magic number for version control
    uint16_t length;        //!< length of the settings structure
    uint32_t param1;        //!< some parameter
    uint32_t param2;        //!< some parameter
    uint32_t param3;        //!< some parameter
    uint32_t param4;        //!< some parameter
    uint32_t param5;        //!< some parameter
    uint32_t param6;        //!< some parameter
    uint32_t param7;        //!< some parameter
    uint32_t param8;        //!< some parameter
    uint32_t param9;        //!< some parameter
    uint32_t param10;       //!< some parameter
};

const Settings EMPTYSETTINGS = {
        0xFFFFFFFF,
        0xFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF
};

struct CryptoKey {
    uint32_t magicNumber;   //!< magic number for version control
    uint8_t privateKey[32]; //!< private key array 32 Bytes
    uint8_t publicKey[32];  //!< public key array
    uint32_t validFrom;     //!< date from which the key is valid
    uint32_t validTo;       //!< date upto which the key is valid
    uint8_t activeStatus;   //!< status
    uint8_t signature[64];  //!< signature of the key
};



/**
 * Operator override for ==
 * @param lhs left hand side
 * @param rhs right hand side
 * @return true, if equal, else false
 */
bool operator==(const Entry &lhs, const Entry &rhs);


/**
 * @class TrackleStorage
 *
 * This class is the API for the underlying storage.
 * All functionality of the flash storage for the trackle device have
 *
 */
class TrackleStorage : public NRF52FlashStorage {
public:
    /*!
     * Constructor for inherited object.
     */
    TrackleStorage() : NRF52FlashStorage(), measCounter(0), pageOffsetMeas(2), pageOffsetSettings(1) {
        measOffset = pageOffsetMeas * PAGE_SIZE_BYTES;
        settingsOffset = pageOffsetSettings * PAGE_SIZE_BYTES;
    }

    /*!
     * Initialize the storage and seek to the next free address.
     * @return true, if successful, els false
     */
    bool init();

    /*!
     * Get the current address, or number of stored measurements.
     * @return  address / number of stored elements
     */
    int sizeMeas();

    /*!
     * Store the current measurement together with the timestamp.
     * @param entry Measueremant data, consists of 32bit timestamp and 16bit temperature
     * @return
     */
    bool storeMeas(Entry &entry);

    bool storageFullMeas();

    /*!
     * Load the measured data from given address.
     * @param address / number of stored data
     * @return measurement entry
     */
    Entry loadMeas(int address = 0);

    /*!
     * Check if the given address is empty.
     * @param address / number of the stored data
     * @return true if it is empty, else false
     */
    bool isEmptyMeas(int address);

    /*!
     * Erase pages in the flash.
     * @param page start page to erase
     * @param numPages number of pages to erase
     * @return true, if successful, else false
     */
    bool eraseMeas(int page, int numPages);



    //###############################################
    /*!
     * Load the complete settings from the storage.
     * @return complete struct of settings
     */
    Settings loadSettings();

    /*!
     * Store the complete settings into the storage.
     * @param settings struct of settings
     * @return true, if successful, else false
     */
    bool storeSettings(Settings &settings);

    /*!
     * Erase the stored settings from the storage
     * @return true, if successful, else false
     */
    bool eraseSettings();

    /*!
     * Replace the current settings in the storage
     * @param settings complete struct of settings
     * @return true, if successful, else false
     */
    bool replaceSettings(Settings settings);

    uint32_t scanSettings();


    bool storeKeys();

    bool loadKeys();

    bool eraseKeys();

    bool replaceKeys();


protected:

    unsigned int measCounter;

    unsigned int measOffset;

    unsigned int pageOffsetMeas;

    unsigned int settingsOffset;

    unsigned int pageOffsetSettings;

};

class TrackleKeyHandler : public NRF52FlashStorage {
public:


protected:

    /*!
     * address / number of next free flash space to store data
    */
    unsigned int address;


};


#endif //UBIRCH_TRACKLE_FIRMWARE_TRACKLESTORAGE_H
