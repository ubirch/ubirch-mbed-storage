/**
 ******************************************************************************
 * @file    FlashStorage.cpp
 * @author  Waldemar Gruenwald
 * @version V1.0.0
 * @date    23 October 2017
 * @brief   flash-storage class implementation
 *
 * @update 	V1.0.1
 * 			08 August 2017
 * 			error handling and debug
 *
 * @update  V1.0.2
 *          18 January 2018
 *          reduced memory footprint and added no softdevice functionality
 ******************************************************************************
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
 *  For more information about the flash-storage,
 *  see documentation for mbed fstorage
 */
#include "FlashStorage.h"
#include <nrf_soc.h>
#include <BLE.h>
#include <nrf52_bitfields.h>
#include "NRF52FlashStorage.h"

extern "C" {
#include <softdevice_handler.h>
}


#define PRINTF(...)
//#define PRINTF printf

/*
* flag for the callback, used to determine, when event handling is finished
* so the rest of the program can continue
*/
static volatile uint8_t fs_callback_flag;

inline static void fs_evt_handler(fs_evt_t const *const evt, fs_ret_t result) {
    (void) evt;
    if (result != FS_SUCCESS) {
        PRINTF("    fstorage event handler ERROR   \r\n");
    } else {
        fs_callback_flag = 0;
    }
};

/*
 * set the configuration
 */
FS_REGISTER_CFG(fs_config_t fs_config) =
        {
                .p_start_addr = 0,                                  // DUMMY
                .p_end_addr = (const uint32_t *) PAGE_SIZE_WORDS,   // DUMMY
                .callback  = fs_evt_handler,                        // Function for event callbacks.
                .num_pages = STORAGE_PAGES,                         // Number of physical flash pages required.
                .priority  = 0xFE                                   // Priority for flash usage.
        };


// adapted from an example found here:
// https://devzone.nordicsemi.com/question/54763/sd_flash_write-implementation-without-softdevice/
fs_ret_t NRF52FlashStorage::nosd_erase_page(const fs_config_t *p_config,
                                            const uint32_t *page_address,
                                            uint32_t num_pages) {
    if (page_address == NULL) {
        return FS_ERR_NULL_ARG;
    }

    // Check that the page is aligned to a page boundary.
    if (((uint32_t) page_address % NRF_FICR->CODEPAGESIZE) != 0) {
        return FS_ERR_UNALIGNED_ADDR;
    }

    // Check that the operation doesn't go outside the client's memory boundaries.
    if ((page_address < p_config->p_start_addr) ||
        (page_address + (PAGE_SIZE_WORDS * num_pages) > p_config->p_end_addr)) {
        return FS_ERR_INVALID_ADDR;
    }

    if (num_pages == 0) {
        return FS_ERR_INVALID_ARG;
    }

    for (uint8_t i = 0; i < num_pages; i++) {
        // Turn on flash erase enable and wait until the NVMC is ready:
        NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Een << NVMC_CONFIG_WEN_Pos);

        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
            // Do nothing.
        }

        PRINTF("NOSD erase(0x%08x)\r\n", (unsigned int) page_address);
        NRF_NVMC->ERASEPAGE = (uint32_t) page_address;

        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
            // Do nothing.
        }

        // Turn off flash erase enable and wait until the NVMC is ready:
        NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
            // Do nothing.
        }
        page_address += NRF_FICR->CODEPAGESIZE / sizeof(uint32_t *);

    }
    return FS_SUCCESS;
}


fs_ret_t NRF52FlashStorage::nosd_store(const fs_config_t *p_config,
                                       uint32_t *p_dest,
                                       uint32_t *p_src,
                                       uint32_t size) {
    if ((p_src == NULL) || (p_dest == NULL)) {
        return FS_ERR_NULL_ARG;
    }

    // Check that both pointers are word aligned.
    if (((uint32_t) p_src & 0x03) ||
        ((uint32_t) p_dest & 0x03)) {
        return FS_ERR_UNALIGNED_ADDR;
    }

    // Check that the operation doesn't go outside the client's memory boundaries.
    if ((p_config->p_start_addr > p_dest) ||
        (p_config->p_end_addr < (p_dest + size))) {
        return FS_ERR_INVALID_ADDR;
    }

    if (size == 0) {
        return FS_ERR_INVALID_ARG;
    }

    PRINTF("NOSD STORE 0x%08x (%d words)\r\n", (unsigned int) p_dest, size);
    for (uint32_t i = 0; i < size; i++) {
        // Turn on flash write enable and wait until the NVMC is ready:
        NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos);

        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
            // Do nothing.
        }

        *p_dest = *p_src;
        p_src++;

        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
            // Do nothing.
        }

        // Turn off flash write enable and wait until the NVMC is ready:
        NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
            // Do nothing.
        }

        p_dest++;
    }

    return FS_SUCCESS;
}


bool NRF52FlashStorage::init() {
    /*
     * initialize the storage and check for success
     */
    fs_ret_t ret = fs_init();
    if (ret != FS_SUCCESS) {
        PRINTF("    fstorage INITIALIZATION ERROR    \r\n");
        return false;
    } else {
        PRINTF("    fstorage INITIALIZATION successful    \r\n");
        return true;
    }
}


bool NRF52FlashStorage::readData(uint32_t p_location,
                                 unsigned char *buffer,
                                 uint16_t length8) {
    if (buffer == NULL || length8 == 0) {
        return false;
    }

    // determine the real location alligned to 32bit (4Byte) values
    uint8_t preLength = (uint8_t) (p_location % 4);
    uint32_t locationReal = p_location - preLength;
    // determine the required length, considering the preLength
    uint16_t lengthReal = (uint16_t) (length8 + preLength);
    if (lengthReal % 4) {
        lengthReal += 4 - (lengthReal % 4);
    }

    uint16_t length32 = lengthReal >> 2;
    uint32_t buf32[length32];
    PRINTF("Data read from flash address 0x%X (%d words)\r\n",
           (uint32_t) (fs_config.p_start_addr) + locationReal,
           length32);
    for (uint16_t i = 0; i < length32; i++) {
        buf32[i] = *(fs_config.p_start_addr + (locationReal >> 2) + i);
//        PRINTF("(%u)[0x%X] = %X", i, ((uint32_t) fs_config.p_start_addr + (locationReal >> 2) + i), buf32[i]);
    }
//    PRINTF("\r\n");

    // create a new buffer, to fill all the data and convert the data into it
    unsigned char bufferReal[lengthReal];
    if (!conv32to8(buf32, bufferReal, lengthReal)) {
        return false;            // ERROR
    }

    // shift the data to the right position
    for (int j = 0; j < length8; j++) {
        buffer[j] = bufferReal[j + preLength];
    }
    return true;
}


bool NRF52FlashStorage::erasePage(uint8_t page, uint8_t numPages) {
    // Erase one page (page 0).
    PRINTF("flash erase 0x%X\r\n",
           (uint32_t) (fs_config.p_start_addr + (PAGE_SIZE_WORDS * page)));

    fs_ret_t ret;
#ifdef NRF52
    if (softdevice_handler_isEnabled()) {
        fs_callback_flag = 1;
        ret = fs_erase(&fs_config, fs_config.p_start_addr + (PAGE_SIZE_WORDS * page), numPages);
#elif NRF52840_XXAA
	if (softdevice_handler_is_enabled()) {
		fs_callback_flag = 1;
		ret = fs_erase(&fs_config, fs_config.p_start_addr + (PAGE_SIZE_WORDS * page), numPages, NULL);
#endif
        while (fs_callback_flag == 1) /* do nothing */ ;
    } else {
        ret = nosd_erase_page(&fs_config,
                              fs_config.p_start_addr + (PAGE_SIZE_WORDS * page),
                              numPages);
    }

    if (ret != FS_SUCCESS) {
        PRINTF("    fstorage ERASE ERROR    \r\n");
        return false;
    } else {
        PRINTF("    fstorage ERASE successful    \r\n");
    }

    return ret == FS_SUCCESS;
}


bool NRF52FlashStorage::writeData(uint32_t p_location,
                                  const unsigned char *buffer,
                                  uint16_t length8) {
    if (buffer == NULL || length8 == 0) {
        PRINTF("ERROR NULL  \r\n");
        return false;
    }

    // determine the real location aligned to 32bit (4Byte) values
    uint8_t preLength = (uint8_t) (p_location % 4);
    uint32_t locationReal = p_location - preLength;
    // determine the required length, considering the preLength
    uint16_t lengthReal = (uint16_t) (length8 + preLength);
    if (lengthReal % 4) {
        lengthReal += 4 - (lengthReal % 4);
    }

    PRINTF("write start=0x%08x, address=0x%08x (offset=%08x, real=0x%08x)\r\n",
           (uint32_t) fs_config.p_start_addr,
           ((uint32_t) fs_config.p_start_addr) + locationReal,
           p_location,
           locationReal);

    // check, if there is already data in the buffer
    unsigned char buffer8[1];
    for (int index = 0; index < length8; index++) {
        this->readData((p_location + index), buffer8, sizeof(uint8_t));
        if (buffer8[0] != 0xFF) {
            PRINTF("ERROR FLASH NOT EMPTY \r\n");
            return false;
        }
    }

    unsigned char bufferReal[lengthReal];
    // shift the data in the buffer to the right location
    // and fill the remaining bytes with 0xFF to not overwrite existing data in the memory
    for (int j = 0; j < preLength; j++) {
        bufferReal[j] = 0xFF;
    }
    for (int k = 0; k < length8; k++) {
        bufferReal[k + preLength] = buffer[k];
    }
    for (int l = (preLength + length8); l < lengthReal; l++) {
        bufferReal[l] = 0xFF;
    }

    // convert the data into 32 bit array
    uint16_t length32 = lengthReal >> 2;
    uint32_t buf32[length32];
    if (!this->conv8to32(bufferReal, buf32, lengthReal)) {
        PRINTF("ERROR CONVERSION \r\n");
        return false;            // ERROR
    }

    fs_ret_t ret;
#ifdef NRF52
    if (softdevice_handler_isEnabled()) {

        fs_callback_flag = 1;
        ret = fs_store(&fs_config,
                       (fs_config.p_start_addr + (locationReal >> 2)),
                       buf32,
                       length32);      //Write data to memory address 0x0003F000. Check it with command: nrfjprog --memrd 0x0003F000 --n 16
#elif NRF52840_XXAA
	if (softdevice_handler_is_enabled()) {

		fs_callback_flag = 1;
		ret = fs_store(&fs_config, (fs_config.p_start_addr + (locationReal >> 2)), buf32,
		               length32, NULL);      //Write data to memory address 0x0003F000. Check it with command: nrfjprog --memrd 0x0003F000 --n 16
#endif
        while (fs_callback_flag == 1) /* do nothing */;
    } else {
        ret = nosd_store(&fs_config,
                         (uint32_t *) (fs_config.p_start_addr + (locationReal >> 2)),
                         buf32,
                         length32);
    }

    if (ret != FS_SUCCESS) {
        PRINTF("    fstorage WRITE ERROR    \r\n");
        return false;
    } else {
        PRINTF("    fstorage WRITE successful    \r\n");
    }

    return ret == FS_SUCCESS;
}

uint32_t NRF52FlashStorage::getStartAddress() {
    return (uint32_t) (fs_config.p_start_addr);
}

uint32_t NRF52FlashStorage::getEndAddress() {
    return (uint32_t) (fs_config.p_end_addr);
}

