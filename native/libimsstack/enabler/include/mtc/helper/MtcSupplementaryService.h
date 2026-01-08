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

#ifndef MTC_SUPPLEMENTARY_SERVICE_H_
#define MTC_SUPPLEMENTARY_SERVICE_H_

#include "ImsList.h"
#include "MtcDef.h"
#include "utility/SuppServiceUtils.h"

class ISession;
class IMessage;
class IMtcCallContext;
class ISipHeader;
class SipAddress;
class MtcConfigurationProxy;

class MtcSupplementaryService final
{
public:
    MtcSupplementaryService(
            IN IMtcCallContext& objContext, IN MtcConfigurationProxy& objConfigurationProxy);
    ~MtcSupplementaryService();
    MtcSupplementaryService(const MtcSupplementaryService&) = delete;
    MtcSupplementaryService& operator=(const MtcSupplementaryService&) = delete;

    void UpdateServices(IN const ImsList<SuppService*>& objSuppServices);
    void UpdateServices(IN IMessage* piMessage);

    void UpdateTip(IN IMessage* piMessage);
    void UpdateSessionId(IN const IMessage* piMessage);

    // The visibility of these internal methods is currently public to allow for unit testing.
    void UpdateCallerId(IN IMessage* piMessage);
    void UpdateCnap(IN IMessage* piMessage);
    void UpdateCdiv(IN const IMessage* piMessage);
    void UpdateCw(IN const IMessage* piMessage);
    void UpdateCallingNumberVerification(IN IMessage* piMessage);
    void UpdateCallComposerElements(IN const IMessage* piMessage);

    inline void Add(IN SuppType eSuppType, IN const AString& strValue)
    {
        SuppServiceUtils::Add(m_objSuppServices, static_cast<IMS_SINT32>(eSuppType), strValue);
    }
    inline void Add(IN SuppType eSuppType, IN IMS_SINT32 nValue)
    {
        SuppServiceUtils::Add(m_objSuppServices, static_cast<IMS_SINT32>(eSuppType), nValue);
    }
    inline void Add(IN SuppType eSuppType, IN IMS_BOOL bValue)
    {
        SuppServiceUtils::Add(m_objSuppServices, static_cast<IMS_SINT32>(eSuppType), bValue);
    }
    inline void Delete(IN SuppType eSuppType)
    {
        SuppServiceUtils::Delete(m_objSuppServices, static_cast<IMS_SINT32>(eSuppType));
    }
    inline const SuppService* Get(IN SuppType eSuppType) const
    {
        return SuppServiceUtils::Get(m_objSuppServices, static_cast<IMS_SINT32>(eSuppType));
    }
    inline const ImsList<SuppService*>& GetServices() const { return m_objSuppServices; }

    static void ConvertGlobalNumberToLocalNumber(
            IN const MtcConfigurationProxy& objConfigurationProxy, IN_OUT AString& strNumber);

private:
    ISipHeader* GetHistoryInfoHeader(IN const IMessage* piMessage) const;
    AString GetCnvParameterValue(IN IMessage* piMessage) const;
    OipType GetOipTypeByHeader(
            IN IMessage* piMessage, IN IMS_BOOL bFromHeader, IN IMS_BOOL bDoFallBack) const;
    AString GetCnapByHeader(
            IN IMessage* piMessage, IN IMS_BOOL bFromHeader, IN IMS_BOOL bDoFallBack) const;

    static IMS_SINT32 GetCdivCause(IN const SipAddress* pAddress);
    static AString GetCdivTarget(IN const SipAddress* pAddress);
    static IMS_SINT32 ConvertCdivCause(IN IMS_SINT32 nCause);
    static IMS_SINT32 GetCallingNumberVerificationResult(
            IN const AString& strVerstatParameter, IN const AString& strDisplayName);

    IMtcCallContext& m_objContext;
    MtcConfigurationProxy& m_objConfigurationProxy;
    ImsList<SuppService*> m_objSuppServices;
};

#endif
