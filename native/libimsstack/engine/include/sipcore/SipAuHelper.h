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
#ifndef SIP_AU_HELPER_H_
#define SIP_AU_HELPER_H_

#include "Credential.h"

#include "ISipGenericChallenge.h"
#include "SipStackHeaders.h"

class SipAuHelperPrivate;

class SipAuHelper
{
public:
    SipAuHelper();
    ~SipAuHelper();

    SipAuHelper(IN const SipAuHelper&) = delete;
    SipAuHelper& operator=(IN const SipAuHelper&) = delete;

public:
    IMS_BOOL AddChallenge(IN ISipGenericChallenge* piChallenge);
    IMS_BOOL AddCredential(IN const Credential& objCredential);
    IMS_BOOL FormCredentials(IN_OUT ::SipMessage*& pSipMsg);
    ISipGenericChallenge* GetChallenge(IN IMS_SINT32 nIndex = 0) const;
    IMS_BOOL IsChallengePresent() const;
    IMS_BOOL IsCredentialPresent() const;
    void RemoveAllChallenges();
    void RemoveAllCredentials();
    IMS_BOOL SetChallenges(IN ::SipMessage* pSipMsg);

private:
    SipAuHelperPrivate* m_pAuHelperPrivate;
};

#endif
