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
#ifndef AOS_HANDLE_MTS_H_
#define AOS_HANDLE_MTS_H_

#include "handle/AosHandle.h"

class AosHandleMts : public AosHandle
{
    // Operation
public:
    AosHandleMts(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strServiceId, IN const IMS_SINT32 nServiceType);
    ~AosHandleMts() override;

    // IAosNConfigurationListener
    void NConfiguration_NotifyConfigChanged() override;

    // IAosNetTrackerListener
    void NetTracker_StatusChanged() override;

protected:
    void InitializeSupportedRats();

    void Init() override;

    void InitializeServiceBlock() override;
    void InitializeServiceFeature() override;

    void ProcessCapabilitiesChanged(
            IN const ImsMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities) override;

    void ProcessNetworkChanged() override;

    IMS_BOOL IsHandleBlocked() const override;
    IMS_BOOL IsSupportedNetworkTypeForCellular(IN IMS_UINT32 nType) const override;
    void Handle_Notify(IN IMS_UINT32 nType, IN IMS_BOOL bBlocked) override;

    // IAosHandle
    void Request(IN IMS_UINT32 nType, IN IMS_UINT32 nState = 0) override;

private:
    IMS_BOOL m_bMtcBlocked;
    IMS_UINT32 m_nSupportedRats;

private:
    friend class AosHandleMtsTest;
};
#endif  // AOS_HANDLE_MTS_H_
