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

#include <gtest/gtest.h>
#include "call/IMtcCall.h"
#include "call/MtcSession.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "configuration/ConfigDef.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSessionInterfaceHolder.h"
#include "media/MockIMtcMediaManager.h"
#include "MockIMtcService.h"
#include "MediaDef.h"
#include "precondition/MockIMtcPreconditionManager.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class MtcSessionTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcCallManager objCallManager;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MockIMtcPreconditionManager objPreconditionManager;
    MockIMtcMediaManager objMediaManager;
    MockISession objSession;
    MockSessionInterfaceHolder* pSessionInterfaceHolder;
    MockIMtcSipInterfaceFactory objSipInterfaceFactory;
    MockIInterfaceHolderListener objInterfaceHolderListener;
    MockIMtcService objMtcService;
    CallInfo objCallInfo;
    MtcSession* pMtcSession;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMediaManager)
                .WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objContext, GetCallManager)
                .WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objPreconditionManager));

        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objContext, GetCallInfo)
                .WillByDefault(ReturnRef(objCallInfo));

        pSessionInterfaceHolder = new MockSessionInterfaceHolder(objInterfaceHolderListener);
        ON_CALL(objSipInterfaceFactory, GetISessionHolder)
                .WillByDefault(Return(pSessionInterfaceHolder));
        ON_CALL(objContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(objSipInterfaceFactory));

        ON_CALL(objContext, GetService)
                .WillByDefault(ReturnRef(objMtcService));

        pMtcSession = new MtcSession(objContext, objSession, CallType::VOIP);
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pMtcSession;
        delete pSessionInterfaceHolder;
    }
};

TEST_F(MtcSessionTest, SendProvisionalResponse180NotReliableWithoutSdp)
{
    ON_CALL(*pConfigurationManager, IsSend180ForInitialInvite)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_RECEIVED));
    ON_CALL(objPreconditionManager, FormPreconditionSdp(_, _))
            .WillByDefault(Return());
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCalls)
            .WillByDefault(Return(objCalls));

    MockIMessage objMessage;
    ON_CALL(objSession, GetNextResponse)
            .WillByDefault(Return(&objMessage));

    EXPECT_CALL(objSession, SendProvisionalResponse(180, _, _)).Times(1);

    pMtcSession->SendProvisionalResponse(IMS_TRUE);
}

TEST_F(MtcSessionTest, SendProvisionalResponse180NotReliableWithSdp)
{
    // TODO: this must be RPR by SDP. Once UT is ready, please update.
    ON_CALL(*pConfigurationManager, IsSend180ForInitialInvite)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));
    ON_CALL(objPreconditionManager, FormPreconditionSdp(_, _))
            .WillByDefault(Return());
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCalls)
            .WillByDefault(Return(objCalls));

    MockIMessage objMessage;
    ON_CALL(objSession, GetNextResponse)
            .WillByDefault(Return(&objMessage));

    EXPECT_CALL(objSession, SendProvisionalResponse(180, _, _)).Times(1);

    pMtcSession->SendProvisionalResponse(IMS_TRUE);
}

TEST_F(MtcSessionTest, SendProvisionalResponse183NotReliableWithoutSdp)
{
    ON_CALL(*pConfigurationManager, IsSend180ForInitialInvite)
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_RECEIVED));
    ON_CALL(objPreconditionManager, FormPreconditionSdp(_, _))
            .WillByDefault(Return());
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCalls)
            .WillByDefault(Return(objCalls));

    MockIMessage objMessage;
    ON_CALL(objSession, GetNextResponse)
            .WillByDefault(Return(&objMessage));

    EXPECT_CALL(objSession, SendProvisionalResponse(183, _, _)).Times(1);

    pMtcSession->SendProvisionalResponse(IMS_FALSE);
}

TEST_F(MtcSessionTest, SendProvisionalResponse183NotReliableWithSdp)
{
    // TODO: this must be RPR by SDP. Once UT is ready, please update.
    ON_CALL(*pConfigurationManager, IsSend180ForInitialInvite)
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));
    ON_CALL(objPreconditionManager, FormPreconditionSdp(_, _))
            .WillByDefault(Return());
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCalls)
            .WillByDefault(Return(objCalls));

    MockIMessage objMessage;
    ON_CALL(objSession, GetNextResponse)
            .WillByDefault(Return(&objMessage));

    EXPECT_CALL(objSession, SendProvisionalResponse(183, _, _)).Times(1);

    pMtcSession->SendProvisionalResponse(IMS_FALSE);
}
