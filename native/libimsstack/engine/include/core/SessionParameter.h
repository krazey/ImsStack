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
#ifndef SESSION_PARAMETER_H_
#define SESSION_PARAMETER_H_

#include "offeranswer/SdpSessionParameter.h"
#include "offeranswer/SdpMediaGroup.h"
#include "offeranswer/SdpMediaParameter.h"

#include "ISessionParameter.h"

class SessionParameter : public ISessionParameter
{
public:
    SessionParameter();
    SessionParameter(IN const SessionParameter& other);
    ~SessionParameter() override;

public:
    SessionParameter& operator=(IN const SessionParameter& other);

public:
    inline const SdpSessionParameter& GetSessionParameter() const override
    {
        return m_objSessionParam;
    }
    inline IMS_SINT32 GetMediaCount() const override
    {
        return static_cast<IMS_SINT32>(m_objMediaParams.GetSize());
    }
    SdpMediaParameter* GetMediaParameter(IN IMS_UINT32 nMid) const override;

    IMS_BOOL Create(IN const SdpSessionDescription& objSessionDescription,
            IN const ImsList<SdpMediaDescription>& objMediaDescriptions);
    // Get session parameter as non-const
    inline SdpSessionParameter& GetSessionParameterNc() { return m_objSessionParam; }
    SdpMediaParameter* CreateMediaParameter();
    const SdpMediaGroup* GetMediaGroup(IN const AString& strMid) const;
    inline const ImsList<SdpMediaParameter*>& GetMediaParameters() const
    {
        return m_objMediaParams;
    }
    inline const AString& GetRemoteVersion() const { return m_strRemoteVersion; }
    IMS_BOOL IsSameVersion(IN const SessionParameter* pSessionParam) const;
    void RemoveMediaParameter(IN IMS_UINT32 nMid, IN IMS_BOOL bRejectedOrRemoved);
    IMS_BOOL FindGroupStartingWithMediaParameter(
            IN IMS_SINT32 nIndex, OUT ImsList<SdpMediaParameter*>& objGroupMediaParams) const;
    IMS_SINT32 GenerateAnswer(IN const SessionParameter* pOffer,
            OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView);
    IMS_SINT32 GenerateAnswer(IN const SessionParameter* pOffer,
            OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView,
            IN IMS_SINT32 nOptions, IN IMS_BOOL bInitialOffer = IMS_FALSE);
    IMS_SINT32 ProcessAnswer(IN const SessionParameter* pAnswer,
            OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView,
            IN IMS_SINT32 nOptions);
    AString ToSdp() const;
    void UpdateDirection(IN const SessionParameter* pOther);
    inline void UpdateRemoteVersion(IN const AString& strRemoteVersion)
    {
        m_strRemoteVersion = strRemoteVersion;
    }
    inline void IncreaseSessionVersion() { m_objSessionParam.IncreaseSessionVersion(); }
    inline IMS_BOOL IsLastSdpProvidedWithNegotiatedSdp() const
    {
        return m_bLastSdpProvidedWithNegotiatedSdp;
    }
    inline void SetLastSdpProvidedWithNegotiatedSdp(IN IMS_BOOL bLastSdpProvidedWithNegotiatedSdp)
    {
        m_bLastSdpProvidedWithNegotiatedSdp = bLastSdpProvidedWithNegotiatedSdp;
    }

private:
    void Clear();
    void Create();
    IMS_SINT32 CreateMid();
    IMS_SINT32 CompareMediaGroups(IN const SessionParameter* pPeerParam,
            OUT SessionParameter*& pProposalView, IN IMS_SINT32 nOptions);
    IMS_SINT32 CompareMediaParameters(IN IMS_BOOL bInitialOffer, IN IMS_BOOL bIsOffer,
            IN const SessionParameter* pPeerParam, OUT SessionParameter*& pProposalView,
            OUT SessionParameter*& pPeerView);
    IMS_SINT32 CompareSessionParameters(IN IMS_BOOL bIsOffer, IN const SessionParameter* pPeerParam,
            OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView);
    void RemoveMediaFromGroup(IN IMS_SINT32 nMid);
    void RemovePreconditionsIfNotSupport(
            IN const SessionParameter* pProposalView, IN const SessionParameter* pPeerView);

private:
    AString m_strRemoteVersion;
    IMS_BOOL m_bLastSdpProvidedWithNegotiatedSdp;
    // Session-level description
    SdpSessionParameter m_objSessionParam;
    // Attribute: "group" in the session level; it's related to "mid" attribute in the media-level
    ImsList<SdpMediaGroup> m_objMediaGroups;
    // Media lines
    IMS_SINT32 m_nMid;  // Next Media Parameter Identifier
    ImsList<SdpMediaParameter*> m_objMediaParams;
};

#endif
