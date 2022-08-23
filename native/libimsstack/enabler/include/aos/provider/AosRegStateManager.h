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
#ifndef AOS_REG_STATE_MANAGER_H_
#define AOS_REG_STATE_MANAGER_H_

#include "IMSTypeDef.h"
#include "interface/IAosRegStateManager.h"

class AosRegStateManager : public IAosRegStateManager
{
public:
    AosRegStateManager();
    virtual ~AosRegStateManager();

    virtual IMS_SINT32 GetSlotId() const;
    virtual void SetSlotId(IN IMS_SINT32 nSlotId);

    virtual void SetImsRegState(IN IMS_UINT32 nState, IN IMS_BOOL bLimited);
    virtual IMS_SINT32 GetImsRegState();
    virtual void SetEImsRegState(IN IMS_UINT32 nState);
    virtual void SetRegState(IN IMS_UINT32 nServiceType, IN IMS_UINT32 nState);
    virtual IMS_SINT32 ConvertServiceType(IMS_UINT32 nServiceType);

    virtual void SetDetailState(IN IMS_SINT32 nState);
    virtual IMS_SINT32 GetDetailState();
    virtual void SetReason(IN IMS_UINT32 nReason);
    virtual void EnforceUpdateRegistration();
    virtual void UpdateRegistration();

    virtual void ClearRegServices();
    virtual IMS_UINT32 GetRegServices() const;
    virtual void UpdateRegServices(IN IMS_BOOL bUpdateCurrState = IMS_FALSE);

    // registration response
    virtual void SetRegRespCode(IN IMS_SINT32 nRespCode);

    // limited service mode for normal registration
    virtual IMS_BOOL IsLimitedMode() const;

protected:
    void AddRegService(IN IMS_UINT32 nType);
    void RemoveRegService(IN IMS_UINT32 nType);
    IMS_BOOL IsRegService(IN IMS_UINT32 nType);
    IMS_UINT32 GetConvertedRegServices();
    IMS_BOOL IsRegistered(IN IMS_UINT32 nDetailState) const;

protected:
    IMS_SINT32 m_nSlotId;
    IMS_UINT32 m_nRegState;
    IMS_UINT32 m_nERegState;
    IMS_UINT32 m_nRegServices;
    IMS_UINT32 m_nReportedRegServices;
    IMS_SINT32 m_nRegDetailState;
    IMS_SINT32 m_nReportedRegDetailState;
    IMS_UINT32 m_nRegReason;
    IMS_SINT32 m_nRegRespCode;
    IMS_BOOL m_bLimitedMode;

    AString m_strTag;

private:
    friend class AosRegStateManagerTest;
};
#endif  // AOS_REG_STATE_MANAGER_H_
