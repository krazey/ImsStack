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
#ifndef __SIP_P_ASSERTED_SERVICE_HEADER_H__
#define __SIP_P_ASSERTED_SERVICE_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipPAssertedServiceHeader : public SipHeaderBase
{
public:
    SipPAssertedServiceHeader();
    SipPAssertedServiceHeader(const SipPAssertedServiceHeader& objHeader);

    SIP_BOOL Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen) override;

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

private:
    ~SipPAssertedServiceHeader() override;
};
#endif  //__SIP_P_ASSERTED_SERVICE_HEADER_H__
