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
#include <cutils/properties.h>
#include "zlib.h"

#include "ByteArray.h"
#include "ISystem.h"
#include "ImsStrLib.h"
#include "OsUtil.h"
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_IPL__;

PUBLIC
OsUtil::OsUtil() :
        m_nBuildType(BUILD_TYPE_UNKNOWN),
        m_bImsDebugOn(IMS_FALSE),
        m_bHidePrivacyLog(IMS_FALSE),
        m_strChipsetVendor(AString::ConstNull())
{
}

PUBLIC
void OsUtil::SetDebugOn(IN IMS_BOOL bDebugOn)
{
    if (bDebugOn != m_bImsDebugOn)
    {
        IMS_TRACE_D("System :: ImsDebugOn (%s >> %s)", _TRACE_B_(m_bImsDebugOn),
                _TRACE_B_(bDebugOn), 0);

        m_bImsDebugOn = bDebugOn;
    }
}

PUBLIC VIRTUAL const AString& OsUtil::GetChipsetVendor() const
{
    return m_strChipsetVendor;
}

PUBLIC VIRTUAL IMS_BOOL OsUtil::IsDebugMode() const
{
    if (m_bImsDebugOn)
    {
        return IMS_TRUE;
    }

    return !IsUserMode();
}

PUBLIC VIRTUAL IMS_BOOL OsUtil::IsServerInfoHiddenInLog() const
{
    if (m_bImsDebugOn)
    {
        return IMS_FALSE;
    }

    if (m_bHidePrivacyLog)
    {
        return IMS_TRUE;
    }

    return IsUserMode();
}

PUBLIC VIRTUAL IMS_BOOL OsUtil::IsUserMode() const
{
    if (m_nBuildType == BUILD_TYPE_UNKNOWN)
    {
        IMS_CHAR acValue[PROPERTY_VALUE_MAX] = {
                0,
        };

        // Build type : "ro.build.type"
        property_get("ro.build.type", acValue, "userdebug");

        if (IMS_StrICmp(acValue, "user") == 0)
        {
            m_nBuildType = BUILD_TYPE_USER;
        }
        else
        {
            m_nBuildType = BUILD_TYPE_DEBUG;
        }
    }
    return m_nBuildType == BUILD_TYPE_USER;
}

PUBLIC GLOBAL OsUtil* OsUtil::GetInstance()
{
    static OsUtil* s_pUtil = IMS_NULL;

    if (s_pUtil == IMS_NULL)
    {
        s_pUtil = new OsUtil();
    }

    return s_pUtil;
}

PRIVATE VIRTUAL void OsUtil::GetUuid(IN IMS_SINT32 nVersion, OUT AString& strUuid,
        IN const AString& strName /* = AString::ConstNull()*/)
{
    ISystem* piSystem = PlatformContext::GetInstance()->GetSystem();
    piSystem->GetUuid(nVersion, strName, strUuid);
}

PRIVATE VIRTUAL AString OsUtil::Get(IN const AString& strName)
{
    IMS_CHAR acValue[PROPERTY_VALUE_MAX] = {
            0,
    };

    property_get(strName.GetStr(), acValue, "");

    return AString(acValue);
}

PRIVATE VIRTUAL IMS_BOOL OsUtil::Set(IN const AString& strName, IN const AString& strValue)
{
    property_set(strName.GetStr(), strValue.GetStr());

    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL OsUtil::Compress(IN const ByteArray& objData, OUT ByteArray& objCompData)
{
#define MAX_COMPRESS_SIZE 32767
#define GZIP_MAX_WBITS    (16 + MAX_WBITS)

    z_stream stream;
    IMS_SINT32 nError;
    IMS_BYTE abyCompData[MAX_COMPRESS_SIZE];

    // input
    stream.next_in = const_cast<IMS_BYTE*>(objData.GetData());
    stream.avail_in = objData.GetLength();

    // for internal state
    stream.zalloc = IMS_NULL;
    stream.zfree = IMS_NULL;
    stream.opaque = 0;

    // level : Z_BEST_SPEED, Z_BEST_COMPRESSION, Z_DEFAULT_COMPRESSION
    if ((nError = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, GZIP_MAX_WBITS,
                 MAX_MEM_LEVEL, Z_FILTERED)) != Z_OK)
    {
        IMS_TRACE_E(0, "ZLib :: deflateInit (%d, %s)", nError,
                (stream.msg != IMS_NULL) ? stream.msg : "-", 0);
        return IMS_FALSE;
    }

    IMS_SINT32 nIndex = 0;

    for (;;)
    {
        // output
        stream.next_out = abyCompData;
        stream.avail_out = MAX_COMPRESS_SIZE;

        nError = deflate(&stream, Z_FINISH);

        if (nError == Z_STREAM_END)
            break;

        if (nError == Z_OK)
        {
            objCompData.Append(abyCompData, MAX_COMPRESS_SIZE);

            ++nIndex;
            IMS_TRACE_D("ZLib :: intermediate compressed data (index=%d)", nIndex, 0, 0);
        }
        else
        {
            IMS_TRACE_E(0, "ZLib :: intermediate compressed data (index=%d) >> error (%d)", nIndex,
                    nError, 0);
            break;
        }
    }

    if (objCompData.GetLength() < static_cast<IMS_SINT32>(stream.total_out))
    {
        IMS_TRACE_D("ZLib :: last compressed data (comp=%d, total=%d, avail_out=%d)",
                objCompData.GetLength(), stream.total_out, stream.avail_out);
        // Append the last compressed data
        objCompData.Append(abyCompData, MAX_COMPRESS_SIZE - stream.avail_out);
    }

    if ((nError = deflateEnd(&stream)) != Z_OK)
    {
        IMS_TRACE_E(0, "ZLib :: deflateEnd (%d, %s)", nError,
                (stream.msg != IMS_NULL) ? stream.msg : "-", 0);
    }

    IMS_TRACE_D(
            "ZLib :: compressed data (%d >> %d)", objData.GetLength(), objCompData.GetLength(), 0);

    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL OsUtil::Uncompress(IN const ByteArray& objCompData, OUT ByteArray& objData)
{
#define MAX_UNCOMPRESS_SIZE 32767
#define GZIP_MAX_WBITS      (16 + MAX_WBITS)

    z_stream stream;
    IMS_SINT32 nError;
    IMS_BYTE abyUncompData[MAX_UNCOMPRESS_SIZE];

    // input
    stream.next_in = const_cast<IMS_BYTE*>(objCompData.GetData());
    stream.avail_in = objCompData.GetLength();

    // for internal state
    stream.zalloc = IMS_NULL;
    stream.zfree = IMS_NULL;
    stream.opaque = 0;

    // Check if the file format is gzip or not (zlib)
    IMS_BOOL bGZIP = IMS_FALSE;

    if (objCompData.GetLength() >= 2)
    {
        // ID1 (0x1f) & ID2 (0x8b) -> gzip
        const IMS_BYTE* pbyData = objCompData.GetData();

        if ((pbyData[0] == 0x1f) && (pbyData[1] == 0x8b))
        {
            bGZIP = IMS_TRUE;
        }
    }

    if (bGZIP)
    {
        nError = inflateInit2(&stream, GZIP_MAX_WBITS);
    }
    else
    {
        nError = inflateInit(&stream);
    }

    if (nError != Z_OK)
    {
        IMS_TRACE_E(0, "ZLib :: inflateInit (gzip:%d, %d, %s)", bGZIP, nError,
                (stream.msg != IMS_NULL) ? stream.msg : "-");
        return IMS_FALSE;
    }

    IMS_SINT32 nIndex = 0;

    for (;;)
    {
        // output
        stream.next_out = abyUncompData;
        stream.avail_out = MAX_UNCOMPRESS_SIZE;

        nError = inflate(&stream, Z_NO_FLUSH);

        if (nError == Z_STREAM_END)
            break;

        if (nError == Z_OK)
        {
            ++nIndex;
            IMS_TRACE_D("ZLib :: intermediate uncompressed data "
                        "(index=%d, total=%d, avail_out=%d)",
                    nIndex, stream.total_out, stream.avail_out);

            objData.Append(abyUncompData, MAX_UNCOMPRESS_SIZE - stream.avail_out);

            if (stream.avail_out != 0)
            {
                // No more data to be unzipped
                break;
            }
        }
        else
        {
            IMS_TRACE_E(0,
                    "ZLib :: intermediate uncompressed data (index=%d) >> "
                    "error (%d, %s)",
                    nIndex, nError, (stream.msg != IMS_NULL) ? stream.msg : "-");
            break;
        }
    }

    if (objData.GetLength() < static_cast<IMS_SINT32>(stream.total_out))
    {
        IMS_TRACE_D("ZLib :: last uncompressed data (uncomp=%d, total=%d, avail_out=%d)",
                objData.GetLength(), stream.total_out, stream.avail_out);
        // Append the last uncompressed data
        objData.Append(abyUncompData, MAX_UNCOMPRESS_SIZE - stream.avail_out);
    }

    if ((nError = inflateEnd(&stream)) != Z_OK)
    {
        IMS_TRACE_E(0, "ZLib :: inflateEnd (%d, %s)", nError,
                (stream.msg != IMS_NULL) ? stream.msg : "-", 0);
    }

    IMS_TRACE_D("ZLib :: uncompressed data (%d >> %d)", objCompData.GetLength(),
            objData.GetLength(), 0);

    return IMS_TRUE;
}
