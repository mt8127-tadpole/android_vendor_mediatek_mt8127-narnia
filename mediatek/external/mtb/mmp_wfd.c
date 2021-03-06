/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */



#ifdef MTK_MMPROFILE_SUPPORT
#define LOG_TAG "MediaTekTraceBridge"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <cutils/xlog.h>
#include <linux/mmprofile.h>
#include "constants.h"
#include "mtb_tracer.h"
#include "mmp_wfd.h"
#include "mmp_hash.h"

//#define THREAD_MAX 10

MMP_Event MMP_WFD_ROOT;
MMP_Event MMP_WFD_MESSAGE;
MMP_Event MMP_WFD_ADATA;
MMP_Event MMP_WFD_VDATA;
MMP_Event MMP_WFD_SPECIAL;

static struct mmp_event_info mmp_wfd_hashTable[MMP_EVENT_HASHTABLE_LENGTH];


int32_t  mmp_wfd_init(tracer_handle_t self)
{
    SXLOGD("Enter function %s", __FUNCTION__);
    MMP_WFD_ROOT = MMProfileRegisterEvent(MMP_RootEvent, "WFD");
    MMProfileEnableEvent(MMP_WFD_ROOT, 1); 

    MMP_WFD_MESSAGE = MMProfileRegisterEvent(MMP_WFD_ROOT, "message");
    MMProfileEnableEvent(MMP_WFD_MESSAGE,1); 

    MMP_WFD_ADATA = MMProfileRegisterEvent(MMP_WFD_ROOT, "audiodata");
    MMProfileEnableEvent(MMP_WFD_ADATA, 1); 
    
    MMP_WFD_VDATA = MMProfileRegisterEvent(MMP_WFD_ROOT, "videodata");
    MMProfileEnableEvent(MMP_WFD_VDATA, 1); 
    
    MMP_WFD_SPECIAL = MMProfileRegisterEvent(MMP_WFD_ROOT, "special");
    MMProfileEnableEvent(MMP_WFD_SPECIAL, 1); 
    
    return 0;

}

int32_t mmp_wfd_begin(tracer_handle_t self, const char *name, uint32_t pid) {
    MMP_Event event;
    struct mmp_event_info *info;
    
    if ((info = mmp_hash_search(name, mmp_wfd_hashTable)) == NULL) {
        event = MMProfileRegisterEvent(MMP_WFD_MESSAGE, name);
        MMProfileEnableEvent(event, 1); 
        mmp_hash_insert(name, event, mmp_wfd_hashTable);
    }
    else {
        event = info->id;
    }
    MMProfileLogEx(event, MMProfileFlagStart, pid, 0);
    return 0;
}

int32_t mmp_wfd_end(tracer_handle_t self, const char* name, uint32_t pid) {
    MMP_Event event;
    struct mmp_event_info *info;
    
    if ((info = mmp_hash_search(name, mmp_wfd_hashTable)) != NULL) {
        MMProfileLogEx(info->id, MMProfileFlagEnd, pid, 0);
    }
    return 0;
}

int32_t mmp_wfd_oneshot(tracer_handle_t self, uint32_t type, const char *name, uint32_t pid) {

    MMP_Event event;

    SXLOGV("Enter function %s, type = %d, name = %s", __FUNCTION__, type, name);
    switch(type) {
        case ATRACE_ONESHOT_ADATA:
            event = MMP_WFD_ADATA;
            break;
        case ATRACE_ONESHOT_VDATA:
            event = MMP_WFD_VDATA;
            break;
        case ATRACE_ONESHOT_SPECIAL:
        { 
            const char *p = name;
            char event_name[32];
            int i = 0;
            while (*p != '\0') {
                if (*p == ',' || i > 14) 
                    break;
                event_name[i] = *p++;
                i++;
            }
            event_name[i] = '\0';
            
            struct mmp_event_info *info;
            if ((info = mmp_hash_search(event_name, mmp_wfd_hashTable)) == NULL) {
                event = MMProfileRegisterEvent(MMP_WFD_SPECIAL, event_name);
                MMProfileEnableEvent(event, 1); 
                mmp_hash_insert(event_name, event,mmp_wfd_hashTable);
            }
            else {
                event = info->id;
            }
            break;
        }
        default:
            SXLOGE("Not Support ATRACE ONE SHOT type: %d", type);
            return -1;
    }

    MMProfileLogMetaStringEx(event, MMProfileFlagPulse, (unsigned int)(*self)->tag, 0, name);
    return 0;
}

#endif











