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
#ifndef REG_INFO_H_
#define REG_INFO_H_

#include "ImsList.h"

#include "IRegInfo.h"

class IDocument;
class INode;
class IRegInfoListener;
class RegInfoRegistration;

class RegInfo : public IRegInfo
{
public:
    RegInfo();
    ~RegInfo() override;

public:
    // IRegInfo interface
    IRegInfoRegistration* GetRegistration(IN const AString& strAor) const override;
    IRegInfoRegistration* GetRegistration(IN const SipAddress& objAor) const override;
    ImsList<IRegInfoRegistration*> GetRegistrations() const override;

    void AddListener(IN IRegInfoListener* piListener);
    void RemoveListener(IN const IRegInfoListener* piListener);
    IMS_BOOL Update(IN IDocument* piDocument);

    void DisplayRegInfo();

private:
    void CallListener(IN IMS_SINT32 nStatus);
    RegInfoRegistration* CheckNCreateRegistration(IN INode* piNode);
    void RemoveAllRegistrations();

private:
    enum
    {
        STATUS_REFRESH_REQUIRED = 0,
        STATUS_UPDATED,
        STATUS_UPDATE_FAILED
    };

    IMS_BOOL m_bIsCreated;
    IMS_UINT32 m_nVersion;
    ImsList<IRegInfoListener*> m_objListeners;
    ImsList<RegInfoRegistration*> m_objRegistrations;
};

#endif
