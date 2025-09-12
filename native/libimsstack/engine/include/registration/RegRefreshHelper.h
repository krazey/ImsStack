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
#ifndef REGISTRATION_REFRESH_HELPER_H_
#define REGISTRATION_REFRESH_HELPER_H_

#include "SipAddress.h"
#include "util/RefreshHelper.h"

class ISipConnection;

class RegRefreshHelper : public RefreshHelper
{
public:
    explicit RegRefreshHelper(IN IRefreshable* piRefreshable);
    ~RegRefreshHelper() override;

    RegRefreshHelper(IN const RegRefreshHelper&) = delete;
    RegRefreshHelper& operator=(IN const RegRefreshHelper&) = delete;

public:
    inline IMS_BOOL AddSpecificHeader(IN ISipConnection* /*piSc*/) override { return IMS_TRUE; }
    IMS_RESULT SendRefreshRequest(IN ISipClientConnection* piScc) override;
    inline IMS_RESULT UpdateOnMessageReceived(IN const ISipConnection* /*piSc*/) override
    {
        return IMS_SUCCESS;
    }
    inline IMS_RESULT UpdateOnMessageSent(IN const ISipConnection* /*piSc*/) override
    {
        return IMS_SUCCESS;
    }

    inline const SipAddress& GetContactAddress() const { return m_objContactAddress; }
    inline void SetContactAddress(IN const SipAddress& objContactAddress)
    {
        m_objContactAddress = objContactAddress;
    }
    IMS_BOOL UpdateRefreshTimer(IN IMS_SINT32 nDuration);

protected:
    void RefreshCompleted(IN ISipClientConnection* piScc, IN IMS_SINT32 nCode = 0) override;
    void RefreshStarted() override;
    void RefreshTerminated() override;

private:
    SipAddress m_objContactAddress;
};

#endif
