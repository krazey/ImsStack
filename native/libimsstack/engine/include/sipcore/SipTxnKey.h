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
#ifndef SIP_TXN_KEY_H_
#define SIP_TXN_KEY_H_

#include "SipMethod.h"

namespace sipcore
{

class SipTxnKey
{
public:
    SipTxnKey();
    SipTxnKey(IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode,
            IN const AString& strViaBranch, IN IMS_UINT32 nCSeq);
    SipTxnKey(IN const SipTxnKey& other);
    inline ~SipTxnKey() {}

public:
    SipTxnKey& operator=(IN const SipTxnKey& other);

public:
    IMS_BOOL Equals(IN const SipTxnKey* pKey) const;
    inline IMS_SINT32 GetExtraInt() const { return m_nExtraInt; }
    inline const AString& GetExtraString() const { return m_strExtraString; }
    inline const SipMethod& GetMethod() const { return m_objMethod; }
    inline IMS_SINT32 GetCSeq() const { return m_nCSeq; }
    inline IMS_SINT32 GetStatusCode() const { return m_nStatusCode; }
    inline const AString& GetViaBranch() const { return m_strViaBranch; }

    inline void SetExtraInt(IN IMS_SINT32 nExtraInt) { m_nExtraInt = nExtraInt; }
    inline void SetExtraString(IN const AString& strExtraString)
    {
        m_strExtraString = strExtraString;
    }

private:
    SipMethod m_objMethod;
    IMS_SINT32 m_nStatusCode;
    AString m_strViaBranch;
    IMS_UINT32 m_nCSeq;

    IMS_SINT32 m_nExtraInt;
    AString m_strExtraString;
};

}  // namespace sipcore

#endif
