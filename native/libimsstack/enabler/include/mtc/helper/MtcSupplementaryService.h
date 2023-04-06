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

#include "ImsMap.h"
#include "MtcDef.h"

class ISession;
class IMessage;
class IMtcCallContext;
class ISipHeader;
class SipAddress;
class MtcConfigurationProxy;

class MtcSupplementaryService final
{
public:
    explicit MtcSupplementaryService(IN IMtcCallContext& objContext,
            IN MtcConfigurationProxy& objConfigurationProxy,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices =
                    ImsMap<SuppType, SuppService*>());
    ~MtcSupplementaryService();
    MtcSupplementaryService(const MtcSupplementaryService&) = delete;
    MtcSupplementaryService& operator=(const MtcSupplementaryService&) = delete;

    void UpdateOutgoingServices(IN const ImsMap<SuppType, SuppService*>& objSuppServices);
    void UpdateTip(IN IMessage* piMessage);

    IMS_BOOL UpdateIncomingServices(IN IMessage* piMessage);
    IMS_BOOL UpdateCallerId(IN IMessage* piMessage);
    IMS_BOOL UpdateCnap(IN IMessage* piMessage);
    static IMS_BOOL UpdateCnapEx(IN IMessage* piMessage);
    IMS_BOOL UpdateMmc(IN IMessage* piMessage);
    static IMS_BOOL UpdateGtt(IN IMessage* piMessage);
    IMS_BOOL UpdateCdivCause(IN IMessage* piMessage);
    IMS_BOOL UpdateCdivHistory(IN IMessage* piMessage);
    IMS_BOOL UpdateCw(IN IMessage* piMessage);
    static IMS_BOOL UpdateVm(IN IMessage* piMessage);
    static IMS_BOOL UpdateAnswerHold(IN IMessage* piMessage);
    IMS_BOOL UpdateMcid(IN IMessage* piMessage);
    static IMS_BOOL UpdateDualNumber(IN IMessage* piMessage);
    IMS_BOOL UpdateCallingNumVerification(IN IMessage* piMessage);
    IMS_BOOL UpdateCallComposerElements(IN IMessage* piMessage);
    void Delete(IN SuppType eType);
    void DeleteServices();
    const SuppService* Get(IN SuppType eType);
    const ImsMap<SuppType, SuppService*>& GetServices() const;
    void Add(IN SuppType eSuppType, IN const AString& strValue);
    void Add(IN SuppType eSuppType, IN IMS_SINT32 nValue);
    void Add(IN SuppType eSuppType, IN IMS_BOOL bValue);

private:
    ISipHeader* GetHistoryInfoHeader(IN IMessage* piMessage);
    static IMS_BOOL GetCdivCause(IN const SipAddress* pAddress, OUT IMS_SINT32& nCause);
    static IMS_BOOL GetCdivTarget(IN const SipAddress* pAddress, OUT AString& strTarget);
    static IMS_SINT32 ConvertCdivCause(IN IMS_SINT32 nCause);
    static IMS_SINT32 GetCallingNumVerificationResult(IN const AString& strValue);
    AString GetCnvParameterValue(IN IMessage* piMessage) const;
    OipType GetOipTypeByHeader(
            IN IMessage* piMessage, IN IMS_BOOL bFromHeader, IN IMS_BOOL bDoFallBack);
    void GetCnapByHeader(IN IMessage* piMessage, IN IMS_BOOL bFromHeader, OUT AString& strCnap,
            IN IMS_BOOL bDoFallBack);
    IMS_BOOL IsExist(IN SuppType suppType);

private:
    IMtcCallContext& m_objContext;
    ImsMap<SuppType, SuppService*> m_objSuppService;
    MtcConfigurationProxy& m_objConfigurationProxy;
};
#endif
