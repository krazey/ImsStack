/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090622  toastops@                 Created
    </table>

    Description

*/

#ifndef _SESSION_PARAMETER_H_
#define _SESSION_PARAMETER_H_

#include "ISessionParameter.h"
#include "offeranswer/SdpSessionParameter.h"
#include "offeranswer/SdpMediaGroup.h"
#include "offeranswer/SdpMediaParameter.h"

class SessionParameter : public ISessionParameter
{
public:
    SessionParameter();
    SessionParameter(IN CONST SessionParameter& objRHS);
    virtual ~SessionParameter();

public:
    SessionParameter& operator=(IN CONST SessionParameter& objRHS);

public:
    virtual const SdpSessionParameter& GetSessionParameter() const;
    virtual IMS_SINT32 GetMediaCount() const;
    virtual SdpMediaParameter* GetMediaParameter(IN IMS_UINT32 nMid) const;

    IMS_BOOL Create(IN CONST SdpSessionDescription& objSessionDescription,
            IN CONST IMSList<SdpMediaDescription>& objMediaDescriptions);
    // Get session parameter as non-const
    SdpSessionParameter& GetSessionParameterNC();
    SdpMediaParameter* CreateMediaParameter();
    const SdpMediaGroup* GetMediaGroup(IN CONST AString& strMid) const;
    const IMSList<SdpMediaParameter*>& GetMediaParameters() const;
    const AString& GetRemoteVersion() const;
    IMS_BOOL IsSameVersion(IN CONST SessionParameter* pSessionParam) const;
    void RemoveMediaParameter(IN IMS_UINT32 nMid, IN IMS_BOOL bRejectedOrRemoved);
    IMS_BOOL FindGroupStartingWithMediaParameter(
            IN IMS_SINT32 nIndex, OUT IMSList<SdpMediaParameter*>& objGroupMediaParams) const;
    IMS_SINT32 GenerateAnswer(IN CONST SessionParameter* pOffer,
            OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView);
    IMS_SINT32 GenerateAnswer(IN CONST SessionParameter* pOffer,
            OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView,
            IN IMS_SINT32 nOptions, IN IMS_BOOL bInitialOffer = IMS_FALSE);
    IMS_SINT32 ProcessAnswer(IN CONST SessionParameter* pAnswer,
            OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView,
            IN IMS_SINT32 nOptions);
    AString ToSDP() const;
    void UpdateDirection(IN CONST SessionParameter* pOther);
    void UpdateRemoteVersion(IN CONST AString& strRemoteVersion);

private:
    void Clear();
    IMS_BOOL Create();
    IMS_SINT32 CreateMid();
    IMS_SINT32 CompareMediaGroups(IN CONST SessionParameter* pPeerParam,
            OUT SessionParameter*& pProposalView, IN IMS_SINT32 nOptions);
    IMS_SINT32 CompareMediaParameters(IN IMS_BOOL bInitialOffer, IN IMS_BOOL bIsOffer,
            IN CONST SessionParameter* pPeerParam, OUT SessionParameter*& pProposalView,
            OUT SessionParameter*& pPeerView);
    IMS_SINT32 CompareSessionParameters(IN IMS_BOOL bIsOffer, IN CONST SessionParameter* pPeerParam,
            OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView);
    void RemoveMediaFromGroup(IN IMS_SINT32 nMid);
    void RemovePreconditionsIfNotSupport(
            OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView);

private:
    AString strRemoteVersion;

    // Session-level description
    SdpSessionParameter objSessionParam;

    // Attribute: "group" in the session level; it's related to "mid" attribute in the media-level
    IMSList<SdpMediaGroup> objMediaGroups;

    // Media lines
    IMS_SINT32 nMid;  // Next Media Parameter Identifier
    IMSList<SdpMediaParameter*> objMediaParams;
};

#endif  // _SDP_SESSION_PARAMETER_H_
