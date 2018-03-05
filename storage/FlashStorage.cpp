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

bool FlashStorage::conv8to32(const unsigned char *d8, uint32_t *d32, uint16_t length8){
    if (d8 == NULL || d32 == NULL || length8 == 0) {
        return false;
    }

    for (int i = 0; i < (length8 >> 2); ++i) {
        d32[i] = (uint32_t) ((d8[(i << 2) + 3] << 24) | (d8[(i << 2) + 2] << 16) | (d8[(i << 2) + 1] << 8) |
                             (d8[(i << 2)]));
    }
    return true;
}


bool FlashStorage::conv32to8(const uint32_t *d32, unsigned char *d8, uint16_t length8){
    if (d8 == NULL || d32 == NULL || length8 == 0) {
        return false;
    }
    uint32_t temp;                            // temporary data storage
    for (uint16_t j = 0; j < (length8 >> 2); ++j) {
        temp = d32[j];
        for (int i = 0; i < 4; ++i) {
            d8[(j << 2) + i] = (unsigned char) (temp & 0xFF);
            temp >>= 8;
        }
    }
    return true;
}
