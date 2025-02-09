/*
 * Copyright (c) 2009-2021 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __TOP_APP_CFG_H__
#define __TOP_APP_CFG_H__

/*============================ INCLUDES ======================================*/
#include <assert.h>

//#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
//#   error This demo requires C11. 
//#endif
#if !defined(__GNUC__)
#   error This demo requires GNU extensions
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/

/*! \note FPB Size
 *!         8 * 8     128B
 *!        16 * 16    512B
 *!        20 * 20    800B
 *!        24 * 24   1152B
 *!        32 * 32      2K
 *!       480 * 1     960B  
 *!       480 * 2    1920B
 *!       480 * 4    3840B
 *!       480 * 8    7680B
 */
#define PFB_BLOCK_WIDTH                     GLCD_WIDTH
#define PFB_BLOCK_HEIGHT                    GLCD_HEIGHT

#define BUSY_WHEEL_SPIN_SPEED   100

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/


#endif
