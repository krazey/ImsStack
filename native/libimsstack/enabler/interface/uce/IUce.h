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

#ifndef _IUCE_H_
#define _IUCE_H_
#include <stdio.h>
#include <string.h>

#include "AString.h"
#include "ImsList.h"
#include "IUUceService.h"

typedef enum
{
    eUCE_RAT_INVALID = -1,
    eUCE_RAT_GERAN = 0,
    eUCE_RAT_HRPD,
    eUCE_RAT_UTRAN,
    eUCE_RAT_EHRPD,
    eUCE_RAT_LTE,
    eUCE_RAT_LTE_NO_VOPS,
    eUCE_RAT_WIFI,
    eUCE_RAT_NR,
    eUCE_RAT_NR_NO_VOPS
} UCE_NETWORK_ENTYPE;

class IUceTerminatedReason
{
public:
    inline IUceTerminatedReason() {}
    inline ~IUceTerminatedReason() {}

public:
    AString m_strContact;
    AString m_strReason;
};
#endif  //_IUCE_H_
