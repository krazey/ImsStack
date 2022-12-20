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
#include "ImsMd5.h"
#include "ImsSha1.h"
#include "ImsUuid.h"
#include "IpAddress.h"
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "ServiceUtil.h"

struct Uuid
{
    IMS_UINT32 nTimeLow;
    IMS_UINT16 nTimeMid;
    IMS_UINT16 nTimeHiAndVersion;
    IMS_UINT8 nClockSeqHiAndReserved;
    IMS_UINT8 nClockSeqLow;
    IMS_BYTE abyNode[6];
};

// 6ba7b817-9dad-11d1-80b4-00c04fd430c8
static const Uuid NAMESPACE_IMS = {
        0x6ba7b817, 0x9dad, 0x11d1, 0x80, 0xb4, {0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8}
};

PUBLIC GLOBAL AString ImsUuid::GetUuid(IN IMS_SINT32 nVersion /*= VERSION_4*/,
        IN const AString& strName /*= AString::ConstNull()*/)
{
    // UUID = time-low "-" time-mid "-" time-hi-and-version "-"
    // clock-seq-hi-and-reserved clock-seq-low "-" node
    // 00000000-0000-0000-0000-000000000000
    AStringBuffer objUuid(64);

    switch (nVersion)
    {
        case ImsUuid::VERSION_1:
        {
            // TODO: need to be adapted.
            // objUUID = UtilService::GetUtilService()->GetSystemUtil()->GetUuid();
            IMS_UINT32 nMicroSecs = IMS_SYS_GetTimeInMicroSeconds();
            AString strRandom;
            strRandom.SetNumber(nMicroSecs);
            GetUuidv4(strRandom, objUuid);
            break;
        }
        case ImsUuid::VERSION_3:
            GetUuidv3(strName, objUuid);
            break;
        case ImsUuid::VERSION_4:
            GetUuidv4(strName, objUuid);
            break;
        case ImsUuid::VERSION_5:
            GetUuidv5(strName, objUuid);
            break;
        default:
            break;
    }

    return static_cast<const AStringBuffer&>(objUuid).GetString();
}

PRIVATE GLOBAL void ImsUuid::GetUuidv3(IN const AString& strName, OUT AStringBuffer& objUuidStr)
{
    Uuid objNsId = NAMESPACE_IMS;

    // Put the namespace id in the network byte order
    objNsId.nTimeLow = IpAddress::HToNL(objNsId.nTimeLow);
    objNsId.nTimeMid = IpAddress::HToNS(objNsId.nTimeMid);
    objNsId.nTimeHiAndVersion = IpAddress::HToNS(objNsId.nTimeHiAndVersion);

    // MD5 hash
    ImsMd5Context objMd5;
    IMS_UCHAR uacHash[IMS_MD5_DIGEST_SIZE];

    ImsMd5_Initialize(&objMd5);
    ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(&objNsId), sizeof(Uuid), &objMd5);
    ImsMd5_Update(
            reinterpret_cast<const IMS_UCHAR*>(strName.GetStr()), strName.GetLength(), &objMd5);
    ImsMd5_Finalize(&objMd5, uacHash);

    // Convert UUID to the host byte order
    Uuid objUuid;

    IMS_MEM_Memcpy(&objUuid, uacHash, sizeof(Uuid));

    objUuid.nTimeLow = IpAddress::NToHL(objUuid.nTimeLow);
    objUuid.nTimeMid = IpAddress::NToHS(objUuid.nTimeMid);
    objUuid.nTimeHiAndVersion = IpAddress::NToHS(objUuid.nTimeHiAndVersion);

    // Set the variant & version bits
    objUuid.nTimeHiAndVersion &= 0x0FFF;
    objUuid.nTimeHiAndVersion |= (3 << 12);
    objUuid.nClockSeqHiAndReserved &= 0x3F;
    objUuid.nClockSeqHiAndReserved |= 0x80;

    objUuidStr.Sprintf("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", objUuid.nTimeLow,
            objUuid.nTimeMid, objUuid.nTimeHiAndVersion, objUuid.nClockSeqHiAndReserved,
            objUuid.nClockSeqLow, objUuid.abyNode[0], objUuid.abyNode[1], objUuid.abyNode[2],
            objUuid.abyNode[3], objUuid.abyNode[4], objUuid.abyNode[5]);
}

PRIVATE GLOBAL void ImsUuid::GetUuidv4(IN const AString& strRandom, OUT AStringBuffer& objUuidStr)
{
    Uuid objNsId = NAMESPACE_IMS;

    // Put the namespace id in the network byte order
    objNsId.nTimeLow = IpAddress::HToNL(objNsId.nTimeLow);
    objNsId.nTimeMid = IpAddress::HToNS(objNsId.nTimeMid);
    objNsId.nTimeHiAndVersion = IpAddress::HToNS(objNsId.nTimeHiAndVersion);

    // SHA1 hash
    ImsSha1Context objSha1;
    IMS_UCHAR uacHash[IMS_SHA1_HASH_SIZE];

    ImsSha1_Initialize(&objSha1);
    ImsSha1_Update(reinterpret_cast<const IMS_UCHAR*>(&objNsId), sizeof(Uuid), &objSha1);
    ImsSha1_Update(reinterpret_cast<const IMS_UCHAR*>(strRandom.GetStr()), strRandom.GetLength(),
            &objSha1);
    ImsSha1_Finalize(&objSha1, uacHash);

    // Convert UUID to the host byte order
    Uuid objUuid;

    IMS_MEM_Memcpy(&objUuid, uacHash, sizeof(Uuid));

    objUuid.nTimeLow = IpAddress::NToHL(objUuid.nTimeLow);
    objUuid.nTimeMid = IpAddress::NToHS(objUuid.nTimeMid);
    objUuid.nTimeHiAndVersion = IpAddress::NToHS(objUuid.nTimeHiAndVersion);

    // Set the variant & version bits
    objUuid.nTimeHiAndVersion &= 0x0FFF;
    objUuid.nTimeHiAndVersion |= (4 << 12);
    objUuid.nClockSeqHiAndReserved &= 0x3F;
    objUuid.nClockSeqHiAndReserved |= 0x80;

    objUuidStr.Sprintf("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", objUuid.nTimeLow,
            objUuid.nTimeMid, objUuid.nTimeHiAndVersion, objUuid.nClockSeqHiAndReserved,
            objUuid.nClockSeqLow, objUuid.abyNode[0], objUuid.abyNode[1], objUuid.abyNode[2],
            objUuid.abyNode[3], objUuid.abyNode[4], objUuid.abyNode[5]);
}

PRIVATE GLOBAL void ImsUuid::GetUuidv5(IN const AString& strName, OUT AStringBuffer& objUuidStr)
{
    Uuid objNsId = NAMESPACE_IMS;

    // Put the namespace id in the network byte order
    objNsId.nTimeLow = IpAddress::HToNL(objNsId.nTimeLow);
    objNsId.nTimeMid = IpAddress::HToNS(objNsId.nTimeMid);
    objNsId.nTimeHiAndVersion = IpAddress::HToNS(objNsId.nTimeHiAndVersion);

    // SHA1 hash
    ImsSha1Context objSha1;
    IMS_UCHAR uacHash[IMS_SHA1_HASH_SIZE];

    ImsSha1_Initialize(&objSha1);
    ImsSha1_Update(reinterpret_cast<const IMS_UCHAR*>(&objNsId), sizeof(Uuid), &objSha1);
    ImsSha1_Update(
            reinterpret_cast<const IMS_UCHAR*>(strName.GetStr()), strName.GetLength(), &objSha1);
    ImsSha1_Finalize(&objSha1, uacHash);

    // Convert UUID to the host byte order
    Uuid objUuid;

    IMS_MEM_Memcpy(&objUuid, uacHash, sizeof(Uuid));

    objUuid.nTimeLow = IpAddress::NToHL(objUuid.nTimeLow);
    objUuid.nTimeMid = IpAddress::NToHS(objUuid.nTimeMid);
    objUuid.nTimeHiAndVersion = IpAddress::NToHS(objUuid.nTimeHiAndVersion);

    // Set the variant & version bits
    objUuid.nTimeHiAndVersion &= 0x0FFF;
    objUuid.nTimeHiAndVersion |= (5 << 12);
    objUuid.nClockSeqHiAndReserved &= 0x3F;
    objUuid.nClockSeqHiAndReserved |= 0x80;

    objUuidStr.Sprintf("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", objUuid.nTimeLow,
            objUuid.nTimeMid, objUuid.nTimeHiAndVersion, objUuid.nClockSeqHiAndReserved,
            objUuid.nClockSeqLow, objUuid.abyNode[0], objUuid.abyNode[1], objUuid.abyNode[2],
            objUuid.abyNode[3], objUuid.abyNode[4], objUuid.abyNode[5]);
}
