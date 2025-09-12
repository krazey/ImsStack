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
#ifndef REG_INFO_CONTACT_H_
#define REG_INFO_CONTACT_H_

#include "IRegInfoContact.h"

class INamedNodeMap;
class INode;

class RegInfoContact : public IRegInfoContact
{
public:
    RegInfoContact();
    ~RegInfoContact() override;

public:
    // IRegInfoContact interface
    inline IMS_UINT32 GetCSeq() const override { return m_nCSeq; }
    inline const AString& GetDisplayName() const override { return m_strDisplayName; }
    inline IMS_SINT32 GetEvent() const override { return m_nEvent; }
    inline IMS_UINT32 GetExpiresValue() const override { return m_nExpires; }
    inline IMS_UINT32 GetFirstCSeq() const override { return m_objTempGruu.m_nFirstCSeq; }
    inline const AString& GetPublicGruu() const override { return m_strPubGruu; }
    inline const AString& GetTemporaryGruu() const override { return m_objTempGruu.m_strGruu; }
    inline const AString& GetQValue() const override { return m_strQValue; }
    inline IMS_UINT32 GetRetryAfterValue() const override { return m_nRetryAfter; }
    inline IMS_SINT32 GetState() const override { return m_nState; }
    const AString& GetUnknownParameter(IN const AString& strName) const override;
    inline const ImsMap<AString, AString>& GetUnknownParameters() const override
    {
        return m_objUnknownParameters;
    }
    inline const SipAddress& GetUri() const override { return m_objUri; }

    IMS_BOOL Equals(IN INode* piNode) const;
    IMS_BOOL Update(IN INode* piNode);

    void DisplayRegInfo(IN const AString& strTag = AString::ConstNull());

private:
    // Attributes
    void SetCallId(IN INamedNodeMap* piNodeMap);
    void SetCSeq(IN INamedNodeMap* piNodeMap);
    void SetDurationRegistered(IN INamedNodeMap* piNodeMap);
    IMS_BOOL SetEvent(IN INamedNodeMap* piNodeMap);
    void SetExpiresValue(IN INamedNodeMap* piNodeMap);
    IMS_BOOL SetId(IN INamedNodeMap* piNodeMap);
    void SetQValue(IN INamedNodeMap* piNodeMap);
    void SetRetryAfterValue(IN INamedNodeMap* piNodeMap);
    IMS_BOOL SetState(IN INamedNodeMap* piNodeMap);

    // Elements
    void SetDisplayName(IN INode* piNode);
    void SetPublicGruu(IN INode* piNode);
    void SetTemporaryGruu(IN INode* piNode);
    void SetUnknownParameter(IN INode* piNode);
    IMS_BOOL SetUri(IN INode* piNode);

private:
    class TempGruu
    {
    public:
        inline TempGruu() :
                m_strGruu(AString::ConstNull()),
                m_nFirstCSeq(0)
        {
        }
        inline TempGruu(IN const TempGruu& other) :
                m_strGruu(other.m_strGruu),
                m_nFirstCSeq(other.m_nFirstCSeq)
        {
        }
        inline ~TempGruu() {}

    public:
        inline TempGruu& operator=(IN const TempGruu& other)
        {
            if (this != &other)
            {
                m_strGruu = other.m_strGruu;
                m_nFirstCSeq = other.m_nFirstCSeq;
            }

            return (*this);
        }

    public:
        AString m_strGruu;
        IMS_UINT32 m_nFirstCSeq;
    };

private:
    AString m_strId;
    IMS_SINT32 m_nState;
    IMS_SINT32 m_nEvent;
    // The amount of time that the contact has been bound to the address-of-record, in seconds
    IMS_UINT32 m_nDurationRegistered;

    // The number of seconds remaining until the binding is due to expire
    // "shortened" event
    IMS_UINT32 m_nExpires;
    // The amount of seconds after which the owner of the contact is expected
    // to retry its registration
    // "probation" event
    IMS_UINT32 m_nRetryAfter;
    // URI associated with this contact
    SipAddress m_objUri;
    // Display name
    AString m_strDisplayName;

    // The relative priority of this contact compared to other registered contacts
    AString m_strQValue;
    // The current Call-ID carried in the REGISTER that was last used to update this contact
    AString m_strCallId;
    // The last CSeq value present in a REGISTER request that updated this contact
    IMS_UINT32 m_nCSeq;

    // Public GRUU
    AString m_strPubGruu;
    // Temporary GRUU
    TempGruu m_objTempGruu;

    // Map for unknown parameters
    ImsMap<AString, AString> m_objUnknownParameters;
};

#endif
