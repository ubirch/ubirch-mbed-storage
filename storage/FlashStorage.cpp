/**
 ******************************************************************************
 * @file    key-storage.cpp
 * @author  Waldemar Gruenwald
 * @version V1.0.0
 * @date    20 July 2017
 * @brief   key-storage class implementation
 *
 * @update 	V1.0.1
 * 			08 August 2017
 * 			error handling and debug
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
 *  For more information about the key-storage,
 *  see documentation for mbed fstorage
 */

#include "FlashStorage.h"
#include <nrf_soc.h>
#include <cstdio>

//#define PRINTF(...)
#define PRINTF printf

// TODO, maybe separate the storage into two different storages, for secure and not secure data

/*
 * flag for the callback, used to determine, when event handling is finished
 * so the rest of the program can continue
 */
static uint8_t fs_callback_flag;


static void fs_evt_handler(fs_evt_t const *const evt, fs_ret_t result) {
    if (result != FS_SUCCESS) {
        PRINTF("    fstorage event handler ERROR   \n\r");
    } else {
        fs_callback_flag = 0;
    }
}

/*
 * set the configuration
 */
FS_REGISTER_CFG(fs_config_t fs_config) =
{
.p_start_addr = 0,                // DUMMY
.p_end_addr = (const uint32_t *) PAGE_SIZE_WORDS,    // DUMMY
.callback  = fs_evt_handler, // Function for event callbacks.
.num_pages = NUM_PAGES,      // Number of physical flash pages required.
.priority  = 0xFF            // Priority for flash usage.
};


bool ks_init(void) {
    /*
     * initialize the storage and check for success
     */
    fs_ret_t ret = fs_init();
    if (ret != FS_SUCCESS) {
        PRINTF("    fstorage INITIALIZATION ERROR    \n\r");
        return false;
    } else {
        PRINTF("    fstorage INITIALIZATION successful    \n\r");
        return true;
    }
}


bool ks_read_data(uint32_t p_location, unsigned char *buffer, uint16_t length8) {
    if (buffer == NULL || length8 == 0) {
        return false;
    }
    uint16_t length32 = length8 >> 2;
    uint32_t buf32[length32];
    PRINTF("Data read from flash address 0x%X: ", ((uint32_t) fs_config.p_start_addr) + p_location);
    for (uint16_t i = 0; i < length32; i++) {
        buf32[i] = *(fs_config.p_start_addr + p_location + i);
        PRINTF("%X ", buf32[i]);
    }
    PRINTF("\r\n");
    return ks_conv32to8(buf32, buffer, length8);
}


bool ks_erase_page(void) {
    // Erase one page (page 0).
    PRINTF("Erasing a flash page at address 0x%X\r\n", (uint32_t) fs_config.p_start_addr);
    fs_callback_flag = 1;
    fs_ret_t ret = fs_erase(&fs_config, fs_config.p_start_addr, 1);
    if (ret != FS_SUCCESS) {
        PRINTF("    fstorage ERASE ERROR    \n\r");
        return false;
    } else {
        PRINTF("    fstorage ERASE successful    \n\r");
    }
    while (fs_callback_flag == 1) sd_app_evt_wait()/* do nothing */;
    return true;
}


bool ks_write_data(uint32_t p_location, const unsigned char *buffer, uint16_t length8) {
    if (buffer == NULL || length8 == 0) {
        return false;
    }

    // TODO fix for buffer lenght < 4

    //Write the buffer into flash
    uint16_t length32 = length8 >> 2;
    uint32_t buf32[length32];
    // TODO check if copying here is absolutely necessary
    if (!ks_conv8to32(buffer, buf32, length8)) {
        return false;            // ERROR
    }

    PRINTF("Writing data 0x%X to address 0x%X\r\n", buf32[0], ((uint32_t) fs_config.p_start_addr + p_location));
    fs_callback_flag = 1;
    fs_ret_t ret = fs_store(&fs_config, (fs_config.p_start_addr + p_location), buf32,
                            (length32));      //Write data to memory address 0x0003F000. Check it with command: nrfjprog --memrd 0x0003F000 --n 16
    if (ret != FS_SUCCESS) {
        PRINTF("    fstorage WRITE A ERROR    \n\r");
        return false;
    } else {
        PRINTF("    fstorage WRITE A successful    \n\r");
    }
    while (fs_callback_flag == 1) /* do nothing */;
    return true;
}


bool ks_conv8to32(const unsigned char *d8, uint32_t *d32, uint16_t length8) {
    if (d8 == NULL || d32 == NULL || length8 == 0) {
        return false;
    }

    for (int i = 0; i < (length8 >> 2); ++i) {
        d32[i] = (uint32_t) ((d8[(i << 2) + 3] << 24) | (d8[(i << 2) + 2] << 16) | (d8[(i << 2) + 1] << 8) |
                             (d8[(i << 2)]));
    }
    return true;
}

// TODO pad the end with 0 align to 4 Bytes

bool ks_conv32to8(const uint32_t *d32, unsigned char *d8, uint16_t length8) {
    if (d8 == NULL || d32 == NULL || length8 == 0) {
        return false;
    }
    uint32_t temp;                            // temporary data storage
    for (uint16_t j = 0; j < (length8 >> 2); ++j) {
        temp = d32[j];
        for (int i = 0; i < 4; ++i) {
            d8[(j << 2) + i] = (unsigned char) (temp & 0xff);
            temp >>= 8;
        }
    }
    return true;
}

// TODO pad the end with 0 alagn to 4 Bytes
