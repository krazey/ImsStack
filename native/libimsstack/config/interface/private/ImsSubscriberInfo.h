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
#ifndef IMS_SUBSCRIBER_INFO_H_
#define IMS_SUBSCRIBER_INFO_H_

#include "IImsSubscriberInfo.h"

class ImsSubscriberInfo : public IImsSubscriberInfo
{
public:
    ImsSubscriberInfo();
    inline virtual ~ImsSubscriberInfo() {}

    ImsSubscriberInfo(IN const ImsSubscriberInfo&) = delete;
    ImsSubscriberInfo& operator=(IN const ImsSubscriberInfo&) = delete;

public:
    // IImsSubscriberInfo interface
    inline const Credential& GetCredential() const override { return m_objCredential; }
    inline const AString& GetHomeDomainName() const override { return m_strHomeDomainName; }
    inline IMS_SINT32 GetIndexOfPrimaryPublicUserId() const override
    {
        return m_nRefIndexOfPrimaryImpu;
    }
    inline const AString& GetPhoneContext() const override
    {
        return (m_strPhoneContext.GetLength() == 0) ? m_strHomeDomainName : m_strPhoneContext;
    }
    inline const AString& GetPrivateUserId() const override { return m_strPrivateUserId; }
    const AString& GetPublicUserId(
            IN IMS_SINT32 nImpuType = IImsSubscriberInfo::IMPU_REF_INDEX) const override;
    inline const AStringArray& GetPublicUserIds() const override { return m_objPublicUserIds; }

    inline const AString& GetScscfAddress() const
    {
        return (m_strScscfAddress.GetLength() == 0) ? m_strHomeDomainName : m_strScscfAddress;
    }
    inline IMS_BOOL IsAuthRealmLenient() const { return m_bIsAuthRealmLenient; }

private:
    friend class SubscriberConfig;

    // Main information which locates on ISIM
    AString m_strHomeDomainName;
    AString m_strPrivateUserId;
    // Reference index (zero-based) of primary IMPU among public user identities
    IMS_SINT32 m_nRefIndexOfPrimaryImpu;
    AString m_strPrimaryImpuSipUri;
    AString m_strPrimaryImpuTelUri;
    AStringArray m_objPublicUserIds;
    AString m_strPhoneContext;

    // Credential information
    Credential m_objCredential;
    IMS_BOOL m_bIsAuthRealmLenient;

    // Server information: S-CSCF (Registrar)
    AString m_strScscfAddress;
};

#endif
