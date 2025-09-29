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
#ifndef REG_INFO_REGISTRATION_H_
#define REG_INFO_REGISTRATION_H_

#include "IRegInfoRegistration.h"
#include "RegInfoContact.h"

class INamedNodeMap;
class INode;

class RegInfoRegistration : public IRegInfoRegistration
{
public:
    RegInfoRegistration();
    ~RegInfoRegistration() override;

public:
    // IRegInfoRegistration interface
    inline const SipAddress& GetAor() const override { return m_objAor; }
    IRegInfoContact* GetContact(IN const SipAddress& objContactUri) const override;
    ImsList<IRegInfoContact*> GetContacts() const override;
    RegInfoContact* GetPriorContact() const override;
    inline IMS_SINT32 GetState() const override { return m_nState; }

    IMS_BOOL Equals(IN INode* piNode) const;
    IMS_BOOL Update(IN INode* piNode);

    void DisplayRegInfo();

private:
    RegInfoContact* CheckNCreateContact(IN INode* piNode);
    IMS_BOOL SetAor(IN const INamedNodeMap* piNodeMap);
    void SetContacts(IN INode* piNode);
    IMS_BOOL SetId(IN const INamedNodeMap* piNodeMap);
    IMS_BOOL SetState(IN const INamedNodeMap* piNodeMap);

private:
    AString m_strId;
    IMS_SINT32 m_nState;
    SipAddress m_objAor;
    ImsList<RegInfoContact*> m_objContacts;
};

#endif
