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
#include "config/UceConfig.h"

#include "CarrierConfig.h"
#include "ICarrierConfig.h"
#include "ServiceConfig.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("CONF");

PUBLIC
UceConfig::UceConfig() :
        m_objAssetMap(IMSMap<IMS_SINT32, UceAssetItems*>())
{
    IMS_TRACE_D("UCE_M : UceConfig - static = %" PFLS_u, sizeof(UceConfig), 0, 0);
}

PUBLIC VIRTUAL UceConfig::~UceConfig()
{
    m_objAssetMap.Clear();
    IMS_TRACE_D("UCE_F : UceConfig - static = %" PFLS_u, sizeof(UceConfig), 0, 0);
}

PUBLIC GLOBAL UceConfig* UceConfig::GetInstance()
{
    static UceConfig* pUceConfig = IMS_NULL;
    //---------------------------------------------------------------------------------------------
    if (pUceConfig == IMS_NULL)
    {
        pUceConfig = new UceConfig();
    }
    return pUceConfig;
}
/*

Remarks

*/
PUBLIC
void UceConfig::Init(IN IMS_SINT32 nSimSlot)
{
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(nSimSlot);
    piCc->AddListener(this);

    Update(piCc, nSimSlot);
}

AString UceConfig::GetAStringValue(IN KEY_UCE_STRING eKey, IN IMS_SINT32 nSimSlot)
{
    AString strRet = AString::ConstEmpty();
    if (m_objAssetMap.GetIndexOfKey(nSimSlot) < 0)
    {
        return strRet;
    }

    UceAssetItems* objUceAssetItems = m_objAssetMap.GetValue(nSimSlot);
    switch (eKey)
    {
        case KEY_RLS_URI:
            strRet = objUceAssetItems->m_strRlsUri;
            break;
    }
    IMS_TRACE_D("GetAStringValue:simSlot[%d], RLS_URI, value[%d]", nSimSlot, strRet.GetStr(), 0);
    return strRet;
}

IMS_UINT32 UceConfig::GetIntValue(IN KEY_UCE_INT eKey, IN IMS_SINT32 nSimSlot)
{
    IMS_UINT32 nRet = 0;

    if (m_objAssetMap.GetIndexOfKey(nSimSlot) < 0)
    {
        return nRet;
    }

    UceAssetItems* objUceAssetItems = m_objAssetMap.GetValue(nSimSlot);
    switch (eKey)
    {
        case KEY_EXPIRE_VALUE_PUBLISH:
            nRet = objUceAssetItems->m_nExpireValuePublish;
            break;
        case KEY_EXTENDED_EXPIRE_VALUE_PUBLISH:
            nRet = objUceAssetItems->m_nExtendedExpireValuePublish;
            break;
        case KEY_PUBLISH_REFRESH_RATIO:
            nRet = objUceAssetItems->m_nPublishRefreshRatio;
            break;
        case KEY_EXPIRE_VALUE_LIST_SUBSCRIBE:
            nRet = objUceAssetItems->m_nExpireValueListSubscribe;
            break;
        case KEY_ANONYMOUS_FETCH_METHOD_INT:
            nRet = objUceAssetItems->m_nAnonymousFetchMethod;
            break;
        case KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_MAX_COUNT:
            nRet = objUceAssetItems->m_nImmediatelyRetryPublishResponseMaxCount;
            break;
        case KEY_RETRY_PUBLISH_RESPONSE_MAX_COUNT:
            nRet = objUceAssetItems->m_nRetryPublishResponseMaxCount;
            break;
        case KEY_RETRY_PUBLISH_RESPONSE_TIME_SEC:
            nRet = objUceAssetItems->m_nRetryPublishResponseTimeSec;
            break;
        case KEY_VARIABLE_RETRY_PUBLISH_RESPONSE_MAX_COUNT:
            nRet = objUceAssetItems->m_nVariableRetryPublishResponseMaxCount;
            break;
    }
    IMS_TRACE_D("GetIntValue:simSlot[%d], key[%s], value[%d]", nSimSlot, GetKeyString(eKey), nRet);
    return nRet;
}

IMS_BOOL UceConfig::GetBoolValue(IN KEY_UCE_BOOL eKey, IN IMS_SINT32 nSimSlot)
{
    IMS_BOOL bRet = IMS_FALSE;

    if (m_objAssetMap.GetIndexOfKey(nSimSlot) < 0)
    {
        return bRet;
    }

    UceAssetItems* objUceAssetItems = m_objAssetMap.GetValue(nSimSlot);
    switch (eKey)
    {
        case KEY_SUBSCRIBE_INDEPENDENT_OF_PUBLISH:
            bRet = objUceAssetItems->m_bSubscribeIndepedentOfPublish;
            break;
        case KEY_ENCODE_PUBLISH_BODY:
            bRet = objUceAssetItems->m_bEncodePublishBody;
            break;
        case KEY_ENCODE_SUBSCRIBE_BODY:
            bRet = objUceAssetItems->m_bEncodeSubscribeBody;
            break;
        case KEY_SUPPORT_OPTIONS:
            bRet = objUceAssetItems->m_bSupportOptions;
            break;
        case KEY_USE_CONTACT_HEADER_IN_PUBLISH:
            bRet = objUceAssetItems->m_bUseContactHeaderInPublish;
            break;
        case KEY_USE_CONTACT_HEADER_IN_SUBSCRIBE:
            bRet = objUceAssetItems->m_bUseContactHeaderInSubscribe;
            break;
        case KEY_ADD_VIDEO_TAG_CONTACT_HEADER_IN_PUBLISH:
            bRet = objUceAssetItems->m_bAddVideoTagContactHeaderInPublish;
            break;
    }
    IMS_TRACE_D("GetBoolValue:simSlot[%d], key[%s], value[%d]", nSimSlot, GetKeyString(eKey), bRet);
    return bRet;
}

IMSVector<IMS_SINT32> UceConfig::GetExponentialRetryPublishRespTimeArray(IN IMS_SINT32 nSimSlot)
{
    if (m_objAssetMap.GetIndexOfKey(nSimSlot) < 0)
    {
        return IMSVector<IMS_SINT32>();
    }
    IMSVector<IMS_SINT32> objExponentialRetryPublishResponseTimeSec =
            m_objAssetMap.GetValue(nSimSlot)->m_objVariableRetryPublishResponseTimeSec;
    return objExponentialRetryPublishResponseTimeSec;
}

IMS_UINT32 UceConfig::GetPublishRetryType(IN IMS_SINT32 nResponseCode, IN IMS_SINT32 nSimSlot)
{
    UCE_RETRY_TYPE nRet = NONE;

    if (m_objAssetMap.GetIndexOfKey(nSimSlot) < 0)
    {
        return nRet;
    }
    IMS_TRACE_D("GetPublishRetryType:simSlot[%d], respoonseCode[%d]", nSimSlot, nResponseCode, 0);

    UceAssetItems* objUceAssetItems = m_objAssetMap.GetValue(nSimSlot);
    IMSVector<IMS_SINT32> temporary = objUceAssetItems->m_objImmediatelyRetryPublishResponse;
    if (!temporary.IsEmpty())
    {
        for (IMS_UINT32 index = 0; index < temporary.GetSize(); index++)
        {
            if (nResponseCode == temporary.GetAt(index))
            {
                IMS_TRACE_D("GetRetryType:IMMEDIATELY", 0, 0, 0);
                return IMMEDIATELY;
            }
        }
    }

    temporary = objUceAssetItems->m_objRetryPublishResponse;
    if (!temporary.IsEmpty())
    {
        for (IMS_UINT32 index = 0; index < temporary.GetSize(); index++)
        {
            if (nResponseCode == temporary.GetAt(index))
            {
                IMS_TRACE_D("GetPublishRetryType:RETRY", 0, 0, 0);
                return RETRY;
            }
        }
    }

    temporary = objUceAssetItems->m_objVariableRetryPublishResponse;
    if (!temporary.IsEmpty())
    {
        for (IMS_UINT32 index = 0; index < temporary.GetSize(); index++)
        {
            if (nResponseCode == temporary.GetAt(index))
            {
                IMS_TRACE_D("GetPublishRetryType:EXPONENTIAL", 0, 0, 0);
                return EXPONENTIAL;
            }
        }
    }
    return nRet;
}

IMS_BOOL UceConfig::IsImsRegistrationRequired(
        IN IMS_BOOL isPublish, IN IMS_SINT32 nResponseCode, IN IMS_SINT32 nSimSlot)
{
    if (m_objAssetMap.GetIndexOfKey(nSimSlot) < 0)
    {
        return IMS_FALSE;
    }
    UceAssetItems* objUceAssetItems = m_objAssetMap.GetValue(nSimSlot);

    IMSVector<IMS_SINT32> temporary;
    if (isPublish)
    {
        temporary = objUceAssetItems->m_objReAttemptRegistrationPublishResponse;
    }
    else
    {
        temporary = objUceAssetItems->m_objReAttemptRegistrationSubscribeResponse;
    }

    if (!temporary.IsEmpty())
    {
        for (IMS_UINT32 index = 0; index < temporary.GetSize(); index++)
        {
            if (nResponseCode == temporary.GetAt(index))
            {
                IMS_TRACE_D("IsImsRegistrationRequired:matched", 0, 0, 0);
                return IMS_TRUE;
            }
        }
    }
    return IMS_FALSE;
}

void UceConfig::toString(IN IMS_SINT32 nSimSlot)
{
    if (m_objAssetMap.GetIndexOfKey(nSimSlot) < 0)
    {
        return;
    }
    IMS_TRACE_D("toString:slotId[%d]", nSimSlot, 0, 0);
    UceAssetItems* objUceAssetItems = m_objAssetMap.GetValue(nSimSlot);
    IMS_TRACE_D("ExpireValuePublish[%d], ExtendedExpireValuePublish[%d], PublishRefreshRatio[%d]",
            objUceAssetItems->m_nExpireValuePublish,
            objUceAssetItems->m_nExtendedExpireValuePublish,
            objUceAssetItems->m_nPublishRefreshRatio);
    IMS_TRACE_D("ExpireValueListSubscribe[%d], RlsUri[%s], SubscribeIndepedentOfPublish[%d]",
            objUceAssetItems->m_nExpireValueListSubscribe, objUceAssetItems->m_strRlsUri.GetStr(),
            objUceAssetItems->m_bSubscribeIndepedentOfPublish);
    IMS_TRACE_D("AnonymousFetchMethod[%d], EncodePublishBody[%d], EncodeSubscribeBody[%d]",
            objUceAssetItems->m_nAnonymousFetchMethod, objUceAssetItems->m_bEncodePublishBody,
            objUceAssetItems->m_bEncodeSubscribeBody);
    IMS_TRACE_D("SupportOptions[%d]", objUceAssetItems->m_bSupportOptions, 0, 0);
    IMS_TRACE_D("UseContactHeaderInPublish[%d], UseContactHeaderInSubscribe[%d], "
                "AddVideoTagContactHeaderInPublish[%d]",
            objUceAssetItems->m_bUseContactHeaderInPublish,
            objUceAssetItems->m_bUseContactHeaderInSubscribe,
            objUceAssetItems->m_bAddVideoTagContactHeaderInPublish);

    IMS_TRACE_D("ImmediatelyRetryPublishResponse size[%d]",
            objUceAssetItems->m_objImmediatelyRetryPublishResponse.GetSize(), 0, 0);
    for (IMS_UINT32 i = 0; i < objUceAssetItems->m_objImmediatelyRetryPublishResponse.GetSize();
            i++)
    {
        IMS_TRACE_D("ImmediatelyRetryPublishResponse index[%d], value[%d]", i,
                objUceAssetItems->m_objImmediatelyRetryPublishResponse.GetValueAt(i), 0);
    }
    IMS_TRACE_D("ImmediatelyRetryPublishResponseMaxCount[%d]",
            objUceAssetItems->m_nImmediatelyRetryPublishResponseMaxCount, 0, 0);

    IMS_TRACE_D("RetryPublishResponse size[%d]",
            objUceAssetItems->m_objRetryPublishResponse.GetSize(), 0, 0);
    for (IMS_UINT32 i = 0; i < objUceAssetItems->m_objRetryPublishResponse.GetSize(); i++)
    {
        IMS_TRACE_D("RetryPublishResponse index[%d], value[%d]", i,
                objUceAssetItems->m_objRetryPublishResponse.GetValueAt(i), 0);
    }
    IMS_TRACE_D("RetryPublishResponseMaxCount[%d], RetryPublishResponseTimeSec[%d]",
            objUceAssetItems->m_nRetryPublishResponseMaxCount,
            objUceAssetItems->m_nRetryPublishResponseTimeSec, 0);

    IMS_TRACE_D("VariableRetryPublishResponse size[%d]",
            objUceAssetItems->m_objVariableRetryPublishResponse.GetSize(), 0, 0);
    for (IMS_UINT32 i = 0; i < objUceAssetItems->m_objVariableRetryPublishResponse.GetSize(); i++)
    {
        IMS_TRACE_D("VariableRetryPublishResponse index[%d], value[%d]", i,
                objUceAssetItems->m_objVariableRetryPublishResponse.GetValueAt(i), 0);
    }
    IMS_TRACE_D("VariableRetryPublishResponseMaxCount[%d]",
            objUceAssetItems->m_nVariableRetryPublishResponseMaxCount, 0, 0);
    IMS_TRACE_D("VariableRetryPublishResponseTimeSec size[%d]",
            objUceAssetItems->m_objVariableRetryPublishResponseTimeSec.GetSize(), 0, 0);
    for (IMS_UINT32 i = 0; i < objUceAssetItems->m_objVariableRetryPublishResponseTimeSec.GetSize();
            i++)
    {
        IMS_TRACE_D("VariableRetryPublishResponseTimeSec index[%d], value[%d]", i,
                objUceAssetItems->m_objVariableRetryPublishResponseTimeSec.GetValueAt(i), 0);
    }
    for (IMS_UINT32 i = 0;
            i < objUceAssetItems->m_objReAttemptRegistrationPublishResponse.GetSize(); i++)
    {
        IMS_TRACE_D("ReAttemptRegistrationPublishResponse index[%d], value[%d]", i,
                objUceAssetItems->m_objReAttemptRegistrationPublishResponse.GetValueAt(i), 0);
    }
    for (IMS_UINT32 i = 0;
            i < objUceAssetItems->m_objReAttemptRegistrationSubscribeResponse.GetSize(); i++)
    {
        IMS_TRACE_D("ReAttemptRegistrationSubscribeResponse index[%d], value[%d]", i,
                objUceAssetItems->m_objReAttemptRegistrationSubscribeResponse.GetValueAt(i), 0);
    }
}

PUBLIC VIRTUAL void UceConfig::CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId)
{
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(nSlotId);
    Update(piCc, nSlotId);
}

PRIVATE
const IMS_CHAR* UceConfig::GetKeyString(IN KEY_UCE_BOOL eKey)
{
    static const char* pszBoolKey[] = {
            "KEY_SUBSCRIBE_INDEPENDENT_OF_PUBLISH",
            "KEY_ENCODE_PUBLISH_BODY",
            "KEY_ENCODE_SUBSCRIBE_BODY",
            "KEY_SUPPORT_OPTIONS",
            "KEY_USE_CONTACT_HEADER_IN_PUBLISH",
            "KEY_USE_CONTACT_HEADER_IN_SUBSCRIBE",
            "KEY_ADD_VIDEO_TAG_CONTACT_HEADER_IN_PUBLISH",
            "ERROR",
    };
    if (sizeof(pszBoolKey) / sizeof(char*) <= eKey)
    {
        IMS_TRACE_E(0, "GetKeyString(bool):key Error", 0, 0, 0);
        return pszBoolKey[sizeof(pszBoolKey) / sizeof(char*) - 1];
    }
    return pszBoolKey[eKey];
}

const IMS_CHAR* UceConfig::GetKeyString(IN KEY_UCE_INT eKey)
{
    static const char* pszIntKey[] = {
            "KEY_EXPIRE_VALUE_PUBLISH",
            "KEY_EXTENDED_EXPIRE_VALUE_PUBLISH",
            "KEY_PUBLISH_REFRESH_RATIO",
            "KEY_EXPIRE_VALUE_LIST_SUBSCRIBE",
            "KEY_ANONYMOUS_FETCH_METHOD_INT",
            "KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_MAX_COUNT",
            "KEY_RETRY_PUBLISH_RESPONSE_MAX_COUNT",
            "KEY_RETRY_PUBLISH_RESPONSE_TIME_SEC",
            "KEY_VARIABLE_RETRY_PUBLISH_RESPONSE_MAX_COUNT",
            "ERROR",
    };
    if (sizeof(pszIntKey) / sizeof(char*) <= eKey)
    {
        IMS_TRACE_E(0, "GetKeyString(int):key Error", 0, 0, 0);
        return pszIntKey[sizeof(pszIntKey) / sizeof(char*) - 1];
    }
    return pszIntKey[eKey];
}

void UceConfig::Update(IN ICarrierConfig* piCc, IN IMS_SINT32 nSimSlot)
{
    UceAssetItems* objUceAssetItems = new UceAssetItems();
    objUceAssetItems->m_nExpireValuePublish =
            piCc->GetInt(CarrierConfig::ImsUce::KEY_EXPIRE_VALUE_PUBLISH_SEC_INT);
    objUceAssetItems->m_nExtendedExpireValuePublish =
            piCc->GetInt(CarrierConfig::ImsUce::KEY_EXTENDED_EXPIRE_VALUE_PUBLISH_SEC_INT);
    objUceAssetItems->m_nPublishRefreshRatio =
            piCc->GetInt(CarrierConfig::ImsUce::KEY_PUBLISH_REFRESH_RATIO_INT);
    objUceAssetItems->m_nExpireValueListSubscribe =
            piCc->GetInt(CarrierConfig::ImsUce::KEY_EXPIRE_VALUE_LIST_SUBSCRIBE_SEC_INT);
    objUceAssetItems->m_strRlsUri = piCc->GetString(CarrierConfig::ImsUce::KEY_RLS_URI_STRING);
    objUceAssetItems->m_bSubscribeIndepedentOfPublish =
            piCc->GetBoolean(CarrierConfig::ImsUce::KEY_SUBSCRIBE_INDEPENDENT_OF_PUBLISH_BOOL);
    objUceAssetItems->m_nAnonymousFetchMethod =
            piCc->GetInt(CarrierConfig::ImsUce::KEY_ANONYMOUS_FETCH_METHOD_INT);
    objUceAssetItems->m_bEncodePublishBody =
            piCc->GetBoolean(CarrierConfig::ImsUce::KEY_ENCODE_PUBLISH_BODY_BOOL);
    objUceAssetItems->m_bEncodeSubscribeBody =
            piCc->GetBoolean(CarrierConfig::ImsUce::KEY_ENCODE_SUBSCRIBE_BODY_BOOL);
    objUceAssetItems->m_bSupportOptions =
            piCc->GetBoolean(CarrierConfig::ImsUce::KEY_SUPPORT_OPTIONS_BOOL);
    objUceAssetItems->m_bUseContactHeaderInPublish =
            piCc->GetBoolean(CarrierConfig::ImsUce::KEY_USE_CONTACT_HEADER_IN_PUBLISH_BOOL);
    objUceAssetItems->m_bUseContactHeaderInSubscribe =
            piCc->GetBoolean(CarrierConfig::ImsUce::KEY_USE_CONTACT_HEADER_IN_SUBSCRIBE_BOOL);
    objUceAssetItems->m_objImmediatelyRetryPublishResponse = piCc->GetIntArray(
            CarrierConfig::ImsUce::KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_INT_ARRAY);
    objUceAssetItems->m_nImmediatelyRetryPublishResponseMaxCount = piCc->GetInt(
            CarrierConfig::ImsUce::KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_MAX_COUNT_INT);
    objUceAssetItems->m_objRetryPublishResponse = piCc->GetIntArray(
            CarrierConfig::ImsUce::KEY_FIXED_TIME_RETRY_PUBLISH_RESPONSE_INT_ARRAY);
    objUceAssetItems->m_nRetryPublishResponseMaxCount = piCc->GetInt(
            CarrierConfig::ImsUce::KEY_FIXED_TIME_RETRY_PUBLISH_RESPONSE_MAX_COUNT_INT);
    objUceAssetItems->m_nRetryPublishResponseTimeSec =
            piCc->GetInt(CarrierConfig::ImsUce::KEY_FIXED_TIME_RETRY_PUBLISH_RESPONSE_TIME_SEC_INT);
    objUceAssetItems->m_objVariableRetryPublishResponse = piCc->GetIntArray(
            CarrierConfig::ImsUce::KEY_VARIABLE_TIME_RETRY_PUBLISH_RESPONSE_INT_ARRAY);
    objUceAssetItems->m_nVariableRetryPublishResponseMaxCount = piCc->GetInt(
            CarrierConfig::ImsUce::KEY_VARIABLE_TIME_RETRY_PUBLISH_RESPONSE_MAX_COUNT_INT);
    objUceAssetItems->m_objVariableRetryPublishResponseTimeSec = piCc->GetIntArray(
            CarrierConfig::ImsUce::KEY_VARIABLE_TIME_RETRY_PUBLISH_RESPONSE_TIME_SEC_INT_ARRAY);
    objUceAssetItems->m_objReAttemptRegistrationPublishResponse = piCc->GetIntArray(
            CarrierConfig::ImsUce::KEY_REATTEMPT_REGISTRATION_PUBLISH_RESPONSE_INT_ARRAY);
    objUceAssetItems->m_objReAttemptRegistrationSubscribeResponse = piCc->GetIntArray(
            CarrierConfig::ImsUce::KEY_REATTEMPT_REGISTRATION_SUBSCRIBE_RESPONSE_INT_ARRAY);

    if (m_objAssetMap.GetIndexOfKey(nSimSlot) >= 0)
    {
        UceAssetItems* objOld = m_objAssetMap.GetValue(nSimSlot);
        if (objOld != IMS_NULL)
        {
            delete objOld;
        }
    }
    m_objAssetMap.SetValue(nSimSlot, objUceAssetItems);
}