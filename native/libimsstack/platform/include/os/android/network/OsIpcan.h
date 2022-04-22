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
#ifndef OS_IPCAN_H_
#define OS_IPCAN_H_

#include "AStringArray.h"
#include "IIpcan.h"

class OsIpcan
    : public IIPCAN
{
public:
    OsIpcan();
    virtual ~OsIpcan();

    OsIpcan(IN const OsIpcan&) = delete;
    OsIpcan& operator=(IN const OsIpcan&) = delete;

protected:
    // IIPCAN class
    virtual void GetAccessInfo(IN IMS_SINT32 nSlotId, IN_OUT AccessNetworkInfo& objAni);
    virtual void GetAccessInfoForWiFi(OUT AccessNetworkInfo& objAni);
    virtual void GetLastAccessInfo(IN IMS_SINT32 nSlotId, OUT AccessNetworkInfo& objAni,
            OUT AString& strTimestamp, OUT AString& strCellInfoAge);
    virtual void GetLastAccessInfoForWiFi(OUT AccessNetworkInfo& objAni,
            OUT AString& strTimestamp, OUT AString& strCellInfoAge);
    virtual IMS_SINT32 GetNetworkType(IN IMS_SINT32 nSlotId);

private:
    static AccessNetworkInfo CreateAccessNetworkInfo(IN IMS_SINT32 nNetworkType,
            IN const AStringArray& objCellIdentities);
};

#endif
