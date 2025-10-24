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
#include <utils/String16.h>

#ifdef __IMS_DEBUG__
#include "AStringBuffer.h"
#endif
#include "ImsMessageDef.h"
#include "OsParcel.h"
#include "PlatformContext.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "device/OsCarrierConfig.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_IPL__;

using namespace android;
using namespace android::os;

#ifdef __IMS_DEBUG__
static AString osCarrierConfig_GetStringFromIntVector(IN const ImsVector<IMS_SINT32>& objIntVector)
{
    AStringBuffer objValue(32);

    for (IMS_UINT32 i = 0; i < objIntVector.GetSize(); ++i)
    {
        objValue.Append(objIntVector.GetAt(i));
        objValue.Append(',');
    }

    if (!objValue.IsEmpty())
    {
        objValue.Chop(1);
    }

    return static_cast<const AStringBuffer&>(objValue).GetString();
}

static AString osCarrierConfig_GetStringFromStringVector(IN const ImsVector<AString>& objStrVector)
{
    AStringBuffer objValue(128);

    for (IMS_UINT32 i = 0; i < objStrVector.GetSize(); ++i)
    {
        objValue.Append(objStrVector.GetAt(i));
        objValue.Append(',');
    }

    if (!objValue.IsEmpty())
    {
        objValue.Chop(1);
    }

    return static_cast<const AStringBuffer&>(objValue).GetString();
}
#endif

PUBLIC
OsCarrierConfig::OsCarrierConfig(IN IMS_SINT32 nSlotId) :
        ImsCarrierConfig(nSlotId),
        m_piOwnerThread(IMS_NULL),
        m_bIsBundle(IMS_FALSE)
{
    m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();

    PlatformContext::GetInstance()->GetSystem()->AddListener(
            SystemConstants::CATEGORY_CONFIG, this, GetSlotId());
}

PUBLIC VIRTUAL OsCarrierConfig::~OsCarrierConfig()
{
    if (!m_bIsBundle)
    {
        PlatformContext::GetInstance()->GetSystem()->RemoveListener(
                SystemConstants::CATEGORY_CONFIG, this, GetSlotId());
    }
}

PRIVATE
OsCarrierConfig::OsCarrierConfig(
        IN IMS_SINT32 nSlotId, IN IMS_BOOL bIsBundle, IN const PersistableBundle& objConfig) :
        ImsCarrierConfig(nSlotId),
        m_bIsBundle(bIsBundle),
        m_objConfig(objConfig)
{
    if (!m_bIsBundle)
    {
        PlatformContext::GetInstance()->GetSystem()->AddListener(
                SystemConstants::CATEGORY_CONFIG, this, GetSlotId());
    }
}

PUBLIC VIRTUAL void OsCarrierConfig::System_NotifyEvent(
        IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    IMS_TRACE_D(
            "OsCarrierConfig :: event=%d, wp=%" PFLS_u ", lp=%" PFLS_u, nEvent, nWParam, nLParam);

    switch (nEvent)
    {
        case IMS_SYSTEM_CONFIGURATION_CHANGED:
            if (m_piOwnerThread != IMS_NULL)
            {
                m_piOwnerThread->PostMessageI(IMS_MSG_CONFIGURATION, GetSlotId(), 0);
            }
            break;
        default:
            IMS_TRACE_D("Invalid event", 0, 0, 0);
            break;
    }
}

PUBLIC VIRTUAL void OsCarrierConfig::DispatchServiceMessage(
        IN IMS_UINTP /*nWparam*/, IN IMS_UINTP /*nLparam*/)
{
    OsParcel objConfig;

    if (PlatformContext::GetInstance()->GetSystem()->GetCarrierConfig(GetSlotId(), objConfig))
    {
        PersistableBundle objNewConfig;
        status_t status;

        if ((status = objNewConfig.readFromParcel(&(objConfig.GetParcel()))) != NO_ERROR)
        {
            IMS_TRACE_E(status, "Getting a carrier-config failed", 0, 0, 0);
            return;
        }

        if (m_objConfig != objNewConfig)
        {
            IMS_TRACE_I("carrier-config is changed: slotId=%d", GetSlotId(), 0, 0);

            m_objConfig = objNewConfig;

            NotifyConfigChanged();
        }
        else
        {
            IMS_TRACE_I("carrier-config is not changed: slotId=%d", GetSlotId(), 0, 0);
        }
    }
}

PUBLIC VIRTUAL void OsCarrierConfig::LoadConfig()
{
    IMS_TRACE_D("LoadConfig: slotId=%d", GetSlotId(), 0, 0);

    OsParcel objConfig;

    if (PlatformContext::GetInstance()->GetSystem()->GetCarrierConfig(GetSlotId(), objConfig))
    {
        PersistableBundle objNewConfig;
        status_t status;

        if ((status = objNewConfig.readFromParcel(&(objConfig.GetParcel()))) != NO_ERROR)
        {
            IMS_TRACE_E(status, "Getting a carrier-config failed", 0, 0, 0);
            return;
        }

        m_objConfig = objNewConfig;

        // For debug only.
#ifdef __IMS_DEBUG__
        DisplayCarrierConfig();
        DisplaySpecificConfigs();
#endif

        IMS_TRACE_D("LoadConfig: carrier-config loaded", 0, 0, 0);
    }
}

PUBLIC VIRTUAL IMS_BOOL OsCarrierConfig::GetBoolean(
        IN const IMS_CHAR* pszKey, IN IMS_BOOL bDefaultValue /*= IMS_FALSE*/) const
{
    const String16 strKey(pszKey);
    bool out = false;

    if (!m_objConfig.getBoolean(strKey, &out))
    {
        return bDefaultValue;
    }

    return out ? IMS_TRUE : IMS_FALSE;
}

PUBLIC VIRTUAL IMS_SINT32 OsCarrierConfig::GetInt(
        IN const IMS_CHAR* pszKey, IN IMS_SINT32 nDefaultValue /*= -1*/) const
{
    const String16 strKey(pszKey);
    int32_t out = -1;

    if (!m_objConfig.getInt(strKey, &out))
    {
        return nDefaultValue;
    }

    return out;
}

PUBLIC VIRTUAL IMS_SLONG OsCarrierConfig::GetLong(
        IN const IMS_CHAR* pszKey, IN IMS_SLONG nDefaultValue /*= -1L*/) const
{
    const String16 strKey(pszKey);
    int64_t out = -1L;

    if (!m_objConfig.getLong(strKey, &out))
    {
        return nDefaultValue;
    }

    return out;
}

PUBLIC VIRTUAL AString OsCarrierConfig::GetString(IN const IMS_CHAR* pszKey,
        IN const AString& strDefaultValue /*= AString::ConstNull()*/) const
{
    const String16 strKey(pszKey);
    String16 out;

    if (!m_objConfig.getString(strKey, &out))
    {
        return strDefaultValue;
    }

    String8 str8(out);

    return AString(str8.c_str());
}

PUBLIC VIRTUAL ImsVector<IMS_BOOL> OsCarrierConfig::GetBooleanArray(
        IN const IMS_CHAR* pszKey, OUT IMS_BOOL& bKeyExists /* = ByRef<IMS_BOOL>(IMS_TRUE)*/) const
{
    const String16 strKey(pszKey);
    std::vector<bool> out;

    if (!m_objConfig.getBooleanVector(strKey, &out))
    {
        bKeyExists = IMS_FALSE;
        return ImsVector<IMS_BOOL>();
    }

    bKeyExists = IMS_TRUE;
    ImsVector<IMS_BOOL> objBooleanArray;

    for (IMS_UINT32 i = 0; i < out.size(); ++i)
    {
        objBooleanArray.Push(out.at(i));
    }

    return objBooleanArray;
}

PUBLIC VIRTUAL ImsVector<IMS_SINT32> OsCarrierConfig::GetIntArray(
        IN const IMS_CHAR* pszKey, OUT IMS_BOOL& bKeyExists /* = ByRef<IMS_BOOL>(IMS_TRUE)*/) const
{
    const String16 strKey(pszKey);
    std::vector<int32_t> out;

    if (!m_objConfig.getIntVector(strKey, &out))
    {
        bKeyExists = IMS_FALSE;
        return ImsVector<IMS_SINT32>();
    }

    bKeyExists = IMS_TRUE;
    ImsVector<IMS_SINT32> objIntArray;

    for (IMS_UINT32 i = 0; i < out.size(); ++i)
    {
        objIntArray.Push(out.at(i));
    }

    return objIntArray;
}

PUBLIC VIRTUAL ImsVector<IMS_SLONG> OsCarrierConfig::GetLongArray(
        IN const IMS_CHAR* pszKey, OUT IMS_BOOL& bKeyExists /* = ByRef<IMS_BOOL>(IMS_TRUE)*/) const
{
    const String16 strKey(pszKey);
    std::vector<int64_t> out;

    if (!m_objConfig.getLongVector(strKey, &out))
    {
        bKeyExists = IMS_FALSE;
        return ImsVector<IMS_SLONG>();
    }

    bKeyExists = IMS_TRUE;
    ImsVector<IMS_SLONG> objLongArray;

    for (IMS_UINT32 i = 0; i < out.size(); ++i)
    {
        objLongArray.Push(out.at(i));
    }

    return objLongArray;
}

PUBLIC VIRTUAL ImsVector<AString> OsCarrierConfig::GetStringArray(
        IN const IMS_CHAR* pszKey, OUT IMS_BOOL& bKeyExists /* = ByRef<IMS_BOOL>(IMS_TRUE)*/) const
{
    const String16 strKey(pszKey);
    std::vector<String16> out;

    if (!m_objConfig.getStringVector(strKey, &out))
    {
        bKeyExists = IMS_FALSE;
        return ImsVector<AString>();
    }

    bKeyExists = IMS_TRUE;
    ImsVector<AString> objStrArray;

    for (IMS_UINT32 i = 0; i < out.size(); ++i)
    {
        const String16& str16 = out.at(i);
        String8 str8(str16);
        objStrArray.Push(AString(str8.c_str()));
    }

    return objStrArray;
}

PUBLIC VIRTUAL ICarrierConfig* OsCarrierConfig::GetBundle(IN const IMS_CHAR* pszKey) const
{
    const String16 strKey(pszKey);
    PersistableBundle out;

    if (!m_objConfig.getPersistableBundle(strKey, &out))
    {
        return IMS_NULL;
    }

    return new OsCarrierConfig(GetSlotId(), IMS_TRUE, out);
}

PUBLIC VIRTUAL void OsCarrierConfig::ReleaseBundle()
{
    if (m_bIsBundle)
    {
        delete this;
    }
}

PUBLIC VIRTUAL void OsCarrierConfig::AddListener(IN ICarrierConfigListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        const ICarrierConfigListener* piTmpListener = m_objListeners.GetAt(i);

        if (piListener == piTmpListener)
        {
            return;
        }
    }

    m_objListeners.Append(piListener);
}

PUBLIC VIRTUAL void OsCarrierConfig::RemoveListener(IN ICarrierConfigListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        const ICarrierConfigListener* piTmpListener = m_objListeners.GetAt(i);

        if (piListener == piTmpListener)
        {
            m_objListeners.RemoveAt(i);
            return;
        }
    }
}

PRIVATE
void OsCarrierConfig::NotifyConfigChanged()
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        ICarrierConfigListener* piListener = m_objListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->CarrierConfig_NotifyConfigChanged(GetSlotId());
        }
    }
}

#ifdef __IMS_DEBUG__
PRIVATE
void OsCarrierConfig::DisplayCarrierConfig()
{
    IMS_TRACE_D("DisplayCarrierConfig: starts", 0, 0, 0);

    std::set<String16>::iterator it;
    std::set<String16> booleanKeys = m_objConfig.getBooleanKeys();

    for (it = booleanKeys.begin(); it != booleanKeys.end(); ++it)
    {
        bool out = false;
        m_objConfig.getBoolean(*it, &out);

        String8 strKey(*it);
        IMS_TRACE_D("carrier-config(bool): %s=%s", strKey.string(), out ? "true" : "false", 0);
    }

    std::set<String16> intKeys = m_objConfig.getIntKeys();

    for (it = intKeys.begin(); it != intKeys.end(); ++it)
    {
        int32_t out = -1;
        m_objConfig.getInt(*it, &out);

        String8 strKey(*it);
        IMS_TRACE_D("carrier-config(int): %s=%d", strKey.string(), out, 0);
    }

    std::set<String16> longKeys = m_objConfig.getLongKeys();

    for (it = longKeys.begin(); it != longKeys.end(); ++it)
    {
        int64_t out = -1L;
        m_objConfig.getLong(*it, &out);

        String8 strKey(*it);
        IMS_TRACE_D("carrier-config(long): %s=%ld", strKey.string(), out, 0);
    }

    std::set<String16> stringKeys = m_objConfig.getStringKeys();

    for (it = stringKeys.begin(); it != stringKeys.end(); ++it)
    {
        String16 out;
        m_objConfig.getString(*it, &out);

        String8 strKey(*it);
        String8 strValue(out);
        IMS_TRACE_D("carrier-config(string): %s=%s", strKey.string(), strValue.string(), 0);
    }

    std::set<String16> intVectorKeys = m_objConfig.getIntVectorKeys();

    for (it = intVectorKeys.begin(); it != intVectorKeys.end(); ++it)
    {
        std::vector<int32_t> out;
        m_objConfig.getIntVector(*it, &out);

        AStringBuffer objValue;

        for (int32_t i = 0; i < out.size(); ++i)
        {
            objValue.Append(out.at(i));
            objValue.Append(',');
        }

        if (!objValue.IsEmpty())
        {
            objValue.Chop(1);
        }

        String8 strKey(*it);

        IMS_TRACE_D("carrier-config(int-array): %s=%s", strKey.string(),
                static_cast<const AStringBuffer&>(objValue).GetString().GetStr(), 0);
    }

    std::set<String16> stringVectorKeys = m_objConfig.getStringVectorKeys();

    for (it = stringVectorKeys.begin(); it != stringVectorKeys.end(); ++it)
    {
        std::vector<String16> out;
        m_objConfig.getStringVector(*it, &out);

        AStringBuffer objValue;

        for (int32_t i = 0; i < out.size(); ++i)
        {
            String8 str8(out.at(i));
            objValue.Append(str8.string());
            objValue.Append(',');
        }

        if (!objValue.IsEmpty())
        {
            objValue.Chop(1);
        }

        String8 strKey(*it);
        const IMS_SINT32 MAX_LEN = 350;

        if (objValue.GetLength() > MAX_LEN)
        {
            const AString& strValue = static_cast<const AStringBuffer&>(objValue).GetString();

            for (int32_t j = 0; j < strValue.GetLength(); j += MAX_LEN)
            {
                AString strSubValue = strValue.GetSubStr(j, MAX_LEN);
                IMS_TRACE_D("carrier-config(string-array): %s=%s", strKey.string(),
                        strSubValue.GetStr(), 0);
            }
        }
        else
        {
            IMS_TRACE_D("carrier-config(string-array): %s=%s", strKey.string(),
                    static_cast<const AStringBuffer&>(objValue).GetString().GetStr(), 0);
        }
    }

    IMS_TRACE_D("DisplayCarrierConfig: ends", 0, 0, 0);
}

PRIVATE
void OsCarrierConfig::DisplaySpecificConfigs()
{
    IMS_BOOL bSipOverIpSecEnabled = GetBoolean("ims.sip_over_ipsec_enabled_bool");
    IMS_SINT32 nServerPort = GetInt("ims.sip_server_port_number_int");
    AString strUserAgentFormat = GetString("ims.ims_user_agent_string");

    IMS_TRACE_D("carrier-config: ipSecEnabled=%s, serverPort=%d, userAgentFormat=%s",
            _TRACE_B_(bSipOverIpSecEnabled), nServerPort, strUserAgentFormat.GetStr());

    ImsVector<IMS_SINT32> objIntArray = GetIntArray("ims.ipsec_encryption_algorithms_int_array");
    AString strValue = osCarrierConfig_GetStringFromIntVector(objIntArray);

    IMS_TRACE_D(
            "carrier-config: ipsec_encryption_algorithms_int_array=%s", strValue.GetStr(), 0, 0);

    ImsVector<AString> objStrArray = GetStringArray("imsvt.video_codec_image_attr_string_array");
    strValue = osCarrierConfig_GetStringFromStringVector(objStrArray);

    IMS_TRACE_D("carrier-config: video_codec_image_attr_string_array=%s", strValue.GetStr(), 0, 0);

    ICarrierConfig* piCc = GetBundle("imsvoice.audio_codec_capability_payload_types_bundle");

    if (piCc != IMS_NULL)
    {
        objIntArray = piCc->GetIntArray("imsvoice.evs_payload_type_int_array");
        strValue = osCarrierConfig_GetStringFromIntVector(objIntArray);
        IMS_TRACE_D("carrier-config: evs_payload_type_int_array=%s", strValue.GetStr(), 0, 0);

        objIntArray = piCc->GetIntArray("imsvoice.amrwb_payload_type_int_array");
        strValue = osCarrierConfig_GetStringFromIntVector(objIntArray);
        IMS_TRACE_D("carrier-config: amrwb_payload_type_int_array=%s", strValue.GetStr(), 0, 0);

        ICarrierConfig* piAmrWbDesc = GetBundle("imsvoice.amrwb_payload_description_bundle");

        if (piAmrWbDesc != IMS_NULL)
        {
            for (IMS_UINT32 i = 0; i < objIntArray.GetSize(); ++i)
            {
                AString strPayloadNumber;
                strPayloadNumber.SetNumber(objIntArray.GetAt(i));

                ICarrierConfig* piCodecAttr = piAmrWbDesc->GetBundle(strPayloadNumber.GetStr());

                if (piCodecAttr != IMS_NULL)
                {
                    IMS_SINT32 nFormat = piCodecAttr->GetInt(
                            "imsvoice.amr_codec_attribute_payload_format_int", 0);
                    IMS_TRACE_D("carrier-config: AMR(%d) - format=%d", objIntArray.GetAt(i),
                            nFormat, 0);

                    IMS_SINT32 nPeriod = piCodecAttr->GetInt(
                            "imsvoice.codec_attribute_mode_change_period_int", 1);
                    IMS_SINT32 nCapability = piCodecAttr->GetInt(
                            "imsvoice.codec_attribute_mode_change_capability_int", 1);
                    IMS_SINT32 nNeighbor = piCodecAttr->GetInt(
                            "imsvoice.codec_attribute_mode_change_neighbor_int", 0);
                    IMS_TRACE_D("carrier-config: period=%d, capability=%d, neighbor=%d", nPeriod,
                            nCapability, nNeighbor);

                    piCodecAttr->ReleaseBundle();
                }
            }

            piAmrWbDesc->ReleaseBundle();
        }

        objIntArray = piCc->GetIntArray("imsvoice.amrnb_payload_type_int_array");
        strValue = osCarrierConfig_GetStringFromIntVector(objIntArray);
        IMS_TRACE_D("carrier-config: amrnb_payload_type_int_array=%s", strValue.GetStr(), 0, 0);

        objIntArray = piCc->GetIntArray("imsvoice.dtmfwb_payload_type_int_array");
        strValue = osCarrierConfig_GetStringFromIntVector(objIntArray);
        IMS_TRACE_D("carrier-config: dtmfwb_payload_type_int_array=%s", strValue.GetStr(), 0, 0);

        objIntArray = piCc->GetIntArray("imsvoice.dtmfnb_payload_type_int_array");
        strValue = osCarrierConfig_GetStringFromIntVector(objIntArray);
        IMS_TRACE_D("carrier-config: dtmfnb_payload_type_int_array=%s", strValue.GetStr(), 0, 0);

        piCc->ReleaseBundle();
    }
}
#endif
