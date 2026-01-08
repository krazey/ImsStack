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

#ifndef TEST_CORE_SERVICE_H_
#define TEST_CORE_SERVICE_H_

#include "CoreService.h"
#include "MockIRegBinding.h"
#include "MockISipClientConnection.h"
#include "private/TestAppConfig.h"

class TestCoreService : public CoreService
{
public:
    TestCoreService();
    ~TestCoreService() override;

    inline ISipClientConnection* CreateConnection(IN const SipAddress* /*pFrom*/,
            IN const SipAddress* /*pTo*/, IN const SipMethod& /*objMethod*/,
            IN IMS_BOOL bPrivacy = IMS_FALSE) override
    {
        (void)bPrivacy;
        return m_piScc;
    }
    inline ISipClientConnection* CreateConnection(IN ISipDialog* /*piDialog*/,
            IN const SipMethod& /*objMethod*/, IN IMS_BOOL bPrivacy = IMS_FALSE) override
    {
        (void)bPrivacy;
        return m_piSccForMidDialog;
    }
    inline ISipClientConnection* CreateCancelConnection(IN ISipClientConnection* /*piScc*/) override
    {
        return m_piSccForCancel;
    }
    inline IMS_BOOL CreateResponse(IN_OUT ISipServerConnection* /*piSsc*/,
            IN IMS_SINT32 /*nStatusCode*/, IN const AString& strPhrase = AString::ConstNull(),
            IN IMS_BOOL bPrivacy = IMS_FALSE) override
    {
        (void)strPhrase;
        (void)bPrivacy;
        return IMS_TRUE;
    }
    inline IMS_BOOL InitAck(IN ISipClientConnection* /*piScc*/) override { return IMS_TRUE; }

    inline MockISipClientConnection& GetMockScc() { return m_objScc; }
    inline MockISipClientConnection& GetMockSccForMidDialog() { return m_objSccForMidDialog; }
    inline MockISipClientConnection& GetMockSccForCancel() { return m_objSccForCancel; }
    inline MockIRegBinding& GetMockRegBinding() { return m_objRegBinding; }
    inline void SetScc(IN ISipClientConnection* piScc) { m_piScc = piScc; }
    inline void SetSccForMidDialog(IN ISipClientConnection* piScc) { m_piSccForMidDialog = piScc; }
    inline void SetSccForCancel(IN ISipClientConnection* piScc) { m_piSccForCancel = piScc; }
    inline void MarkAsImsConnected(IN IMS_BOOL bImsConnected)
    {
        CoreService::SetImsConnected(bImsConnected);
    }

public:
    MockIRegBinding m_objRegBinding;
    MockISipClientConnection m_objScc;
    MockISipClientConnection m_objSccForMidDialog;
    MockISipClientConnection m_objSccForCancel;

    ISipClientConnection* m_piScc;
    ISipClientConnection* m_piSccForMidDialog;
    ISipClientConnection* m_piSccForCancel;
};

#endif
