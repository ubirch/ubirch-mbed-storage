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
#ifndef UBIRCH_FLASH_STORAGE_H
#define UBIRCH_FLASH_STORAGE_H

#include <cstdio>

extern "C" {
#include <nrf_fstorage.h>
}

/**
 * A flash storage abstraction.
 */
class FlashStorage {

public:

    /*!
     * @brief   Constructor
     */
     FlashStorage() {};

    virtual /*!
     * @brief   Destructor
     */
     ~FlashStorage() {};

    /*!
     * Initialize the key storage
     * to enable a secure data storage of the size, described in the configuration
     *
     * @note	the initialization only works, if the soft device and the event
     * 			handler are enabled
     *
     * @return 	int 	true, if initialization successful, else false
     */
    virtual bool init() = 0;

    /*!
     * Read data from the key storage
     *
     * @param p_location	location (pointer) inside the configured data space (32 Bit)
     * @param *buffer		pointer to the buffer, where the data will be filled in (8 Bit
     * @param length8 		length of data elements to read (8 Bit)
     *
     * @return int 			true, if reading successful, else false
     */
    virtual bool readData(uint32_t p_location, unsigned char *buffer, uint16_t length8) = 0;

    /*!
     * Erase a page in the key storage
     *
     * @return int 			true, if erasing succeeded, else false
     */
    virtual bool erasePage(uint8_t page, uint8_t numPages) = 0;

    /*!
     * Write data to the key storage
     *
     * @param p_location 	location (pointer) inside the configured data space (32 Bit)
     * @param *buffer		pointer to the buffer with the data (8 Bit)
     * @param length8 		length of data elements to write (8 Bit)
     *
     * @return int 			true, if writing successful, else false
     */
    virtual bool writeData(uint32_t p_location, const unsigned char *buffer, uint16_t length8) = 0;

    /*!
     * Convert 32 Bit array into 8 bit array.
     *
     * @param *d32			pointer to 32 Bit data array (input)
     * @param *d8 			pointer to 8 Bit data array (output)
     * @param length8 		length of 8 Bit array
     *
     * @return int			true, if successful, else false
     */
    bool conv32to8(const uint32_t *d32, unsigned char *d8, uint16_t length8);

    /*!
     * Convert 8 Bit array into 32 Bit array
     *
     * @param *d8 			pointer to 8 Bit data array (input)
     * @param *d32			pointer to 32 Bit data array (output)
     * @param length8		length of the 8 bit array
     *
     * @return int			true if successful, else false
     */
    bool conv8to32(const unsigned char *d8, uint32_t *d32, uint16_t length8);


public:
    /*!
     * Get the start address of the storage.
     *
     * @return  start address (32 Bit)
     */
    virtual uint32_t getStartAddress() = 0;

    /*!
     * Get the end address of the storage.
     *
     * @return end address (32 Bit)
     */
    virtual uint32_t getEndAddress() = 0;

};

#endif //UBIRCH_FLASH_STORAGE_H
