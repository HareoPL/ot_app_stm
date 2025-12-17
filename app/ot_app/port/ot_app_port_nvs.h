/**
 * @file ot_app_port_nvs.h
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief 
 * @version 0.1
 * @date 24-10-2025
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

#ifndef OT_APP_NVS_H_
#define OT_APP_NVS_H_

#include "stdint.h"

#define OT_APP_NVS_OK               (-1)
#define OT_APP_NVS_ERROR            (-2)
#define OT_APP_NVS_IS               (-3)
#define OT_APP_NVS_IS_NOT           (-4)
#define OT_APP_NVS_IS_NO_SPACE      (-5)

/**
 * @brief save data into NVS partition
 * 
 * @param inData        [in] ptr to data
 * @param keyId         [in] it is id for saved data. It will be necessary for update or read data
 * @return int8_t       OT_APP_NVS_ERROR or OT_APP_NVS_OK
 */
int8_t ot_app_nvs_saveString(const char *inData, const uint8_t keyId);

/**
 * @brief read data from NVS partition
 * 
 * @param outData        [out] ptr to data
 * @param outDataSize    [out] ptr to the size of the data that was read
 * @param keyId          [in] it is id for saved data. It will be necessary for update or read data
 * @return int8_t        OT_APP_NVS_ERROR or OT_APP_NVS_OK
 */
int8_t ot_app_nvs_readString(char *outBuff, uint8_t outBuffSize, const uint8_t keyId);

int8_t ot_app_nvs_init(void);

void ot_app_nvs_test();


#endif  /* OT_APP_NVS_H_ */
