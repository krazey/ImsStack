/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __SIP_PF_MEMORY_H__
#define __SIP_PF_MEMORY_H__

#include "sip_pf_datatypes.h"

SIP_VOID SipPf_Memset(SIP_VOID* pvMem_block, SIP_UCHAR cChars, SIP_SIZE_T nSize);

SIP_VOID SipPf_Memcpy(SIP_VOID* pvMem_Dest, const SIP_VOID* pvMem_Source, SIP_SIZE_T nSize);

SIP_UINT32 SipPf_Memcmp(const SIP_VOID* pvMem1, const SIP_VOID* pvMem2, SIP_SIZE_T nSize);

#endif /*__SIP_PF_MEMORY_H__*/
