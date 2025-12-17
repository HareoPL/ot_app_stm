/**
 * @file ot_app_srp_client.h
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief 
 * @version 0.1
 * @date 06-09-2025
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
#ifndef OT_APP_SRP_CLIENT_H_
#define OT_APP_SRP_CLIENT_H_

#include "hro_utils.h"
#include "openthread/instance.h"


#define OTAPP_DNS_LEASE_TASK_DELAY  300  // in secounds = 5m
#define OTAPP_DNS_LEASE_TIME        7200 // in secounds = 2h
#define OTAPP_DNS_LEASE_GUARD       (4 * OTAPP_DNS_LEASE_TASK_DELAY) // 20 min before end the time lease
#define OTAPP_DNS_M_KEY_LEASE_TIME  86400


/**
 * @brief todo
 * 
 * @param instance 
 */
void otapp_srpClientUpdateHostAddress(otInstance *instance);

/**
 * @brief todo
 * 
 */
void otapp_srpInit();

#endif  /* OT_APP_SRP_CLIENT_H_ */
