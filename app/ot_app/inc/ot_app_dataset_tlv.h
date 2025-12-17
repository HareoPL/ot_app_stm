/**
 * @file ot_app_dataset_tlv.h
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief 
 * @version 0.1
 * @date 31-07-2025
 * 
 * @copyright The MIT License (MIT) Copyright (c) 2025 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 * 
 */
#ifndef OT_APP_DATASET_TLV_H_
#define OT_APP_DATASET_TLV_H_

#include "stdint.h"
#include "openthread/dataset.h"

const otOperationalDatasetTlvs otapp_dataset_tlv = {
    .mTlvs = {
        0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x13, 0x4a,
        0x03, 0x00, 0x00, 0x10, 0x35, 0x06, 0x00, 0x04, 0x00, 0x1f, 0xff, 0xe0, 0x02, 0x08, 0x1c, 0xc8,
        0x1a, 0x73, 0xcd, 0x4c, 0x61, 0xfa, 0x07, 0x08, 0xfd, 0xef, 0x86, 0x2f, 0xed, 0x61, 0x7a, 0x32,
        0x05, 0x10, 0x29, 0x47, 0x54, 0xd4, 0x8c, 0x17, 0x3d, 0x2c, 0x01, 0x65, 0x34, 0x8f, 0xc7, 0x4e,
        0xb9, 0x9f, 0x03, 0x0f, 0x4f, 0x70, 0x65, 0x6e, 0x54, 0x68, 0x72, 0x65, 0x61, 0x64, 0x2d, 0x62,
        0x37, 0x66, 0x33, 0x01, 0x02, 0xb7, 0xf3, 0x04, 0x10, 0xef, 0x6c, 0x20, 0x54, 0xdc, 0x92, 0x26,
        0xdc, 0x78, 0xe7, 0x6f, 0x57, 0x54, 0x9f, 0xc1, 0xbd, 0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8
    },
    .mLength = 111,
};

#endif  /* OT_APP_DATASET_TLV_H_ */
