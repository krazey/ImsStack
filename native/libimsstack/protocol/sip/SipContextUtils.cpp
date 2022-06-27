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
#include "SipContextUtils.h"

static SipContextUtils *gpUtil = SIP_NULL;

SipContextUtils* SipContextUtils::GetInstance()
{
    if (gpUtil == SIP_NULL)
    {
        gpUtil = new SipContextUtils();
    }

    return gpUtil;
}

SIP_VOID SipContextUtils::Destruct()
{
    if (gpUtil != SIP_NULL)
    {
        delete gpUtil;
        gpUtil = SIP_NULL;
    }
}

SipContextUtils::SipContextUtils()
{
}

SipContextUtils::~SipContextUtils()
{
}

SipTxnContext* SipContextUtils::Sip_CreateTxnContext()
{
    return new SipTxnContext();
}

void SipContextUtils::Sip_DestroyTxnContext(IN SipTxnContext* pContext)
{
    // Destroy SipTxnContext
    if (pContext != SIP_NULL)
    {
        delete pContext;
        pContext = SIP_NULL;
    }
}
