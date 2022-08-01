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
#ifndef __UCE_CONFIG_H_
#define __UCE_CONFIG_H_

#include "AString.h"
#include "ICarrierConfigListener.h"
#include "ImsMap.h"
#include "UceAssetItems.h"

class ICarrierConfig;
class UceConfig : public ICarrierConfigListener
{
public:
    UceConfig();
    virtual ~UceConfig();

    static UceConfig* GetInstance();

    enum KEY_UCE_BOOL
    {
        KEY_SUBSCRIBE_INDEPENDENT_OF_PUBLISH,
        KEY_ENCODE_PUBLISH_BODY,
        KEY_ENCODE_SUBSCRIBE_BODY,
        KEY_USE_CONTACT_HEADER_IN_PUBLISH,
        KEY_USE_CONTACT_HEADER_IN_SUBSCRIBE,
        KEY_ADD_VIDEO_TAG_CONTACT_HEADER_IN_PUBLISH,
    };

    enum KEY_UCE_INT
    {
        KEY_EXPIRE_VALUE_PUBLISH,
        KEY_EXTENDED_EXPIRE_VALUE_PUBLISH,
        KEY_PUBLISH_REFRESH_RATIO,
        KEY_EXPIRE_VALUE_LIST_SUBSCRIBE,
        KEY_ANONYMOUS_FETCH_METHOD_INT,
        KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE_MAX_COUNT,
        KEY_RETRY_PUBLISH_RESPONSE_MAX_COUNT,
        KEY_RETRY_PUBLISH_RESPONSE_TIME_SEC,
        KEY_VARIABLE_RETRY_PUBLISH_RESPONSE_MAX_COUNT,
    };

    enum KEY_UCE_STRING
    {
        KEY_RLS_URI,
    };

    enum KEY_UCE_INT_ARRAY
    {
        KEY_IMMEDIATELY_RETRY_PUBLISH_RESPONSE,
        KEY_RETRY_PUBLISH_RESPONSE,
        KEY_VARIABLE_RETRY_PUBLISH_RESPONSE,
        KEY_VARIABLE_RETRY_PUBLISH_RESPONSE_TIME_SEC,
        KEY_REATTEMPT_REGISTRATION_PUBLISH_RESPONSE,
        KEY_REATTEMPT_REGISTRATION_SUBSCRIBE_RESPONSE,
    };

    enum UCE_RETRY_TYPE
    {
        NONE = 0,
        IMMEDIATELY,
        RETRY,
        EXPONENTIAL,
    };

public:
    void Init(IN IMS_SINT32 nSimSlot = 0);
    AString GetAStringValue(IN KEY_UCE_STRING eKey, IN IMS_SINT32 nSimSlot = 0);
    IMS_UINT32 GetIntValue(IN KEY_UCE_INT eKey, IN IMS_SINT32 nSimSlot = 0);
    IMS_BOOL GetBoolValue(IN KEY_UCE_BOOL eKey, IN IMS_SINT32 nSimSlot = 0);
    IMSVector<IMS_SINT32> GetExponentialRetryPublishRespTimeArray(IN IMS_SINT32 nSimSlot = 0);
    IMS_UINT32 GetPublishRetryType(IN IMS_SINT32 nResponseCode, IN IMS_SINT32 nSimSlot = 0);
    IMS_BOOL IsImsRegistrationRequired(
            IN IMS_BOOL isPublish, IN IMS_SINT32 nResponseCode, IN IMS_SINT32 nSimSlot = 0);
    void toString(IN IMS_SINT32 nSimSlot);

protected:
    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;

private:
    const IMS_CHAR* GetKeyString(IN KEY_UCE_BOOL eKey);
    const IMS_CHAR* GetKeyString(IN KEY_UCE_INT eKey);
    const IMS_CHAR* GetKeyString(IN KEY_UCE_STRING eKey);
    void Update(IN ICarrierConfig* piCc, IN IMS_SINT32 nSimSlot = 0);

private:
    IMSMap<IMS_SINT32, UceAssetItems*> m_objAssetMap;
};

#endif  // __UCE_CONFIG_H_
