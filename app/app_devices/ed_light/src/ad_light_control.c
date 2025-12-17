/**
 * @file ad_light_control.c
 * @author Jan Łukaszewicz (pldevluk@gmail.com)
 * @brief 
 * @version 0.1
 * @date 13-11-2025
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
#include "ad_light_control.h"
#include "ws2812b_fx.h"

#define AD_LIGHT_CTR_COLOR 3 // RGB

#define AD_LIGHT_CTR_MAX_DIM    (255)
#define AD_LIGHT_CTR_BRIGHTNESS_FORM(rgb, dim) (((rgb)*(dim)) / AD_LIGHT_CTR_MAX_DIM)

#define AD_LIGHT_ID_R 0
#define AD_LIGHT_ID_G 1
#define AD_LIGHT_ID_B 2

static ot_app_devDrv_t *drv;

typedef struct {
    uint8_t RGB[AD_LIGHT_CTR_COLOR]; 
    uint8_t RGB_dim[AD_LIGHT_CTR_COLOR]; 
    uint32_t dim;
}ad_light_ctr_t;

static ad_light_ctr_t ctr;

static uint8_t *ad_light_ctr_rgbScaling(uint32_t dimValue)
{
    for (uint8_t i = 0; i < AD_LIGHT_CTR_COLOR; i++)
    {
        ctr.RGB_dim[i] = AD_LIGHT_CTR_BRIGHTNESS_FORM(ctr. RGB[i], dimValue);
    }
    return ctr.RGB_dim;
}

static void ad_light_ctr_dimSave(uint32_t dimValue)
{
    ctr.dim = dimValue;
}

static void ad_light_ctr_colorSave(uint8_t r, uint8_t g, uint8_t b)
{
    ctr.RGB[AD_LIGHT_ID_R] = r;
    ctr.RGB[AD_LIGHT_ID_G] = g;
    ctr.RGB[AD_LIGHT_ID_B] = b;
}

void ad_light_ctr_onOff(uint32_t ledState)
{
    if(ledState)
    {
        if(ctr.dim == 0)
        {
            WS2812BFX_ForceAllColor(0, ctr.RGB[AD_LIGHT_ID_R], ctr.RGB[AD_LIGHT_ID_G], ctr.RGB[AD_LIGHT_ID_B]);
        }
        else
        {
            WS2812BFX_ForceAllColor(0, ctr.RGB_dim[AD_LIGHT_ID_R], ctr.RGB_dim[AD_LIGHT_ID_G], ctr.RGB_dim[AD_LIGHT_ID_B]);
        }
    }else
    {
        WS2812BFX_ForceAllColor(0, 0, 0, 0);
    }
}

void ad_light_ctr_dimSet(uint32_t dimValue)
{
    uint8_t *rgb;

    ad_light_ctr_dimSave(dimValue);  
    rgb = ad_light_ctr_rgbScaling(dimValue);  

    WS2812BFX_ForceAllColor(0, rgb[AD_LIGHT_ID_R], rgb[AD_LIGHT_ID_G], rgb[AD_LIGHT_ID_B]);
}


void ad_light_ctr_colorSet(uint8_t r, uint8_t g, uint8_t b)
{   
    ad_light_ctr_colorSave(r, g, b);

    if(ctr.dim != 0)
    {
        ad_light_ctr_dimSet(ctr.dim);
    }
    else
    {
        WS2812BFX_ForceAllColor(0, r, g, b);
    }       
}

void ad_light_ctr_init(ot_app_devDrv_t *devDrv)
{
    if(devDrv == NULL) return;
    drv = devDrv;

    // todo load latest settings
    // color, dim, 
    ad_light_ctr_colorSave(0, 0, 5);
}