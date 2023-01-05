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

#ifndef _UCE_OPTIONS_H_
#define _UCE_OPTIONS_H_

#include "AString.h"
#include "ICapabilitiesListener.h"

class ICoreService;
class ISipMessage;
class ICapabilities;
class IUceJniThread;

class UceOptions : public ICapabilitiesListener
{
public:
    explicit UceOptions(IN const AString& strManagerName, IN ICoreService* piCoreService,
            IN ICapabilities* piCapabilities, IN IMS_UINT32 nKey, IN IMS_BOOL isSendingRequest,
            IN IMS_SINT32 nSimSlot = 0);
    virtual ~UceOptions();

public:
    IMS_BOOL SendOptionsRequest(IN AString strRemoteURI, IN IMS_UINT32 ownCapabilities);
    IMS_BOOL SendOptionsResponse(
            IN IMS_UINT32 nResponse, IN const AString& reason, IN IMS_UINT32 ownCapabilities);
    void AoSDisconnected();
    static IMS_UINT32 GetCapability(IMSList<AString> objContactList);
    static void SetIARIFeatureTag(IN IMS_UINT32 capabilities, OUT AString& strIARITag);
    static void SetICSIFeatureTag(IN IMS_UINT32 capabilities, OUT AString& strICSITag);
    static void SetNoTypeFeatureTag(IN IMS_UINT32 capabilities, OUT AString& strTag);

protected:
    // ICapabilitiesListener
    virtual void CapabilityQueryDelivered(IN ICapabilities* piCapabilities) override;
    virtual void CapabilityQueryDeliveryFailed(IN ICapabilities* piCapabilities) override;

private:
    void SetContactHeader(IN IMS_UINT32 capabilities, ISipMessage* piSIPMessage) const;
    void SendOptionsResponseInd(
            IN IMS_SINT32 nResponseCode, IN AString reason, IN IMS_UINT32 capabilities);
    void SendOptionsCommandError(IN IMS_UINT32 code);
    void OptionsTerminated();
    void DestroyCapabilities();
    IUceJniThread* GetJniThread();

public:
    enum
    {
        FEATURE_TAG_DP = (0x00000001),                        // iari
        FEATURE_TAG_IPCALL_VOICE = (0x00000002),              // icsi
        FEATURE_TAG_IPCALL_VIDEO = (0x00000004),              // no type (video)
        FEATURE_TAG_PAGER_MESSAGING = (0x00000008),           // icsi
        FEATURE_TAG_LARGE_MESSAGING = (0x00000010),           // icsi
        FEATURE_TAG_CPM_CHAT = (0x00000020),                  // icsi
        FEATURE_TAG_SIMPLE_IM = (0x00000040),                 // iari
        FEATURE_TAG_STORE_FORWARD_GROUP_CHAT = (0x00000080),  // iari
        FEATURE_TAG_FT_THUMBNAIL = (0x00000100),              // iari
        FEATURE_TAG_FT_STORE_FORWARD = (0x00000200),          // iari
        FEATURE_TAG_FT_HTTP = (0x00000400),                   // iari
        FEATURE_TAG_GEOLOCATION_PUSH = (0x00000800),          // iari
        FEATURE_TAG_FT = (0x00001000),                        // iari
        FEATURE_TAG_SHARED_MAP = (0x00002000),                // icsi
        FEATURE_TAG_SHARED_SKETCH = (0x00004000),             // icsi
        FEATURE_TAG_CALL_COMPOSER = (0x00008000),             // icsi
        FEATURE_TAG_CALL_COMPOSER_MMTEL = (0x00010000),       // no type
        FEATURE_TAG_POST_CALL = (0x00020000),                 // icsi
        FEATURE_TAG_FT_SMS = (0x00040000),                    // iari
        FEATURE_TAG_GEOLOCATION_SMS = (0x00080000),           // iari
        FEATURE_TAG_CHATBOT_SESSION = (0x00100000),           // iari
        FEATURE_TAG_CHATBOT_SA = (0x00200000),                // iari
        FEATURE_TAG_IS_BOT = (0x00400000),                    // no type
        FEATURE_TAG_CHATBOT_VERSION_V1 = (0x00800000),        // no type
        FEATURE_TAG_CHATBOT_VERSION_V2 = (0x01000000),        // no type
    };

protected:
    IMS_UINT32 m_nKey;
    IMS_BOOL m_bIsSendingRequest;

private:
    AString m_strManagerName;
    ICoreService* m_piCoreService;
    ICapabilities* m_piCapabilities;
    IMS_SINT32 m_nSimSlot;
    AString m_strMSISDN;
};

#endif  // _UCE_OPTIONS_H_
