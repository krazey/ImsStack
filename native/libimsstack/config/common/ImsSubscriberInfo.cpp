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
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "private/ImsSubscriberInfo.h"

__IMS_TRACE_TAG_CONF__;

PUBLIC
ImsSubscriberInfo::ImsSubscriberInfo() :
        m_strHomeDomainName(AString::ConstNull()),
        m_strPrivateUserId(AString::ConstNull()),
        m_nRefIndexOfPrimaryImpu(0)  // First one as a primary IMPU
        ,
        m_strPrimaryImpuSipUri(AString::ConstNull()),
        m_strPrimaryImpuTelUri(AString::ConstNull()),
        m_strPhoneContext(AString::ConstNull()),
        m_bIsAuthRealmLenient(IMS_FALSE),
        m_strScscfAddress(AString::ConstNull())
{
}

PUBLIC VIRTUAL const AString& ImsSubscriberInfo::GetPublicUserId(
        IN IMS_SINT32 nImpuType /*= IImsSubscriberInfo::IMPU_REF_INDEX*/) const
{
    if (nImpuType == IImsSubscriberInfo::IMPU_SIP)
    {
        return m_strPrimaryImpuSipUri;
    }
    else if (nImpuType == IImsSubscriberInfo::IMPU_TEL)
    {
        return m_strPrimaryImpuTelUri;
    }
    else
    {
        if ((m_nRefIndexOfPrimaryImpu >= 0) &&
                (m_nRefIndexOfPrimaryImpu < m_objPublicUserIds.GetCount()))
        {
            return m_objPublicUserIds.GetElementAt(m_nRefIndexOfPrimaryImpu);
        }

        return m_strPrimaryImpuSipUri;
    }
}
