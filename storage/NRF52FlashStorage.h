/**
 ******************************************************************************
 * @file    FlashStorage.cpp
 * @author  Waldemar Gruenwald
 * @version V1.0.0
 * @date    20 July 2017
 * @brief   key-storage class implementation
 *
 * @update 	V1.0.1
 * 			08 August 2017
 * 			error handling and debug
 ******************************************************************************
 * @attention
 *
 * Copyright 2016 ubirch GmbH (https://ubirch.com)
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

/**
 *  For more information about the key-storage,
 *  see documentation for mbed fstorage
 */

#ifndef UBIRCH_MBED_NRF52_STORAGE_NRF52FLASHSTORAGE_H
#define UBIRCH_MBED_NRF52_STORAGE_NRF52FLASHSTORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fstorage.h>
#include <blecommon.h>
#include "FlashStorage.h"

#define NUM_PAGES 4

#if defined (NRF52)
#define PAGE_SIZE_WORDS 1024
#endif


/** Interface for secure flash storage access via key storage
 *
 * @code
 * #include "key-storage.h"
 *
 * TODO write example code , convert to C++ class!
 *
 *
 * @endcode
 */

class NRF52FlashStorage : public FlashStorage {

public:

    /*!
     * @brief   Constructor
     */
    NRF52FlashStorage() { activatedSdStatus = false; };

    /*!
     * @brief   Destructor
     */
    ~NRF52FlashStorage() {};
    /*!
     * Initialize the key storage
     * to enable a secure data storage of the size, described in the configuration
     *
     * @note	the initialization only works, if the soft device and the event
     * 			handler are enabled
     *
     * @return 	int 	true, if initialization successful, else false
     */
    bool init();

    /*!
     * Read data from the key storage
     *
     * @param p_location	location (pointer) inside the configured data space (32 Bit)
     * @param buffer		pointer to the buffer, where the data will be filled in (8 Bit
     * @param length8 		length of data elements to read (8 Bit)
     *
     * @return int 			true, if reading successful, else false
     */
    bool readData(uint32_t p_location, unsigned char *buffer, uint16_t length8);

    /*!
     * Erase a page in the key storage
     *
     * @return int 			true, if erasing succeeded, else false
     */
    bool erasePage(uint8_t page = 0, uint8_t numPages = 1);

    /*!
     * Write data to the key storage
     *
     * @param p_location 	location (pointer) inside the configured data space (32 Bit)
     * @param buffer		pointer to the buffer with the data (8 Bit)
     * @param length8 		length of data elements to write (8 Bit)
     *
     * @return 			    true, if writing successful, else false
     */
    bool writeData(uint32_t p_location, const unsigned char *buffer, uint16_t length8);

    /*!
     * Get the start address of the storage.
     *
     * @return  start address
     */
    uint32_t getStartAddress();

    /*!
     * Get the end address of the storage.
     *
     * @return  end address
     */
    uint32_t getEndAddress();

protected:

    /*!
     * Initialize the Softdevice, if it is not initialized yet.
     * Calling this function if Softdevice is enabled will not reinitialize the Softdevice, but return.
     *
     * @return  BLE_ERROR_NONE, if successful
     */
    ble_error_t initBleSd();

    /*!
     * Deinitialize the Softdevice, if previously it was explicitly initialized by initBleSd(). If not,
     * this function will return without doing anything.
     */
    void deinitBleSd();

    /*!
     * Status of explicit Softdevice activation for Storage purposes.
     */
    bool activatedSdStatus = false;


};
#ifdef __cplusplus
}
#endif

#endif //UBIRCH_MBED_NRF52_STORAGE_NRF52FLASHSTORAGE_H
