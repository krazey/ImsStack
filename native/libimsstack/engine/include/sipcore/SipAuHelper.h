/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_AUTHENTICATION_HELPER_H_
#define _SIP_AUTHENTICATION_HELPER_H_

#include "ISipGenericChallenge.h"
#include "Credential.h"
#include "SipStackHeaders.h"

class SIPAuHelperPrivate;

class SIPAuHelper
{
public:
    SIPAuHelper();
    ~SIPAuHelper();

private:
    SIPAuHelper(IN CONST SIPAuHelper& objRHS);
    SIPAuHelper& operator=(IN CONST SIPAuHelper& objRHS);

public:
    IMS_BOOL AddChallenge(IN ISipGenericChallenge* piChallenge);
    IMS_BOOL AddCredential(IN CONST Credential& objCredential);
    IMS_BOOL FormCredentials(IN_OUT SipMessage*& pstMessage);
    ISipGenericChallenge* GetChallenge(IN IMS_SINT32 nIndex = 0) const;
    IMS_BOOL IsChallengePresent() const;
    IMS_BOOL IsCredentialPresent() const;
    void RemoveAllChallenges();
    void RemoveAllCredentials();
    IMS_BOOL SetChallenges(IN SipMessage* pstMessage);

private:
    SIPAuHelperPrivate* pSAHelper;
};

#endif  // _SIP_AUTHENTICATION_HELPER_H_
