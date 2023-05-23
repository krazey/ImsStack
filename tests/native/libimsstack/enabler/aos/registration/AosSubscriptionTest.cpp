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
#include <gmock/gmock.h>

#include "app/MockAosAppContext.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosRegistration.h"
#include "interface/MockIAosRetryRepository.h"
#include "interface/MockIAosSubscriptionListener.h"
#include "interface/MockIAosTransaction.h"
#include "../../interface/aos/MockIAosService.h"
#include "../../../engine/interface/registration/MockIRegInfo.h"
#include "../../../engine/interface/registration/MockIRegInfoContact.h"
#include "../../../engine/interface/registration/MockIRegInfoRegistration.h"
#include "../../../engine/interface/registration/MockIRegSubscription.h"
#include "../../../engine/interface/sipcore/MockISipMessage.h"

#include "CarrierConfig.h"
#include "IRegContact.h"
#include "IRegSubscription.h"
#include "ISipHeader.h"
#include "SipAddress.h"
#include "SipMessageBodyPart.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosConnection.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "registration/AosSubscription.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

class AosSubscriptionTest : public ::testing::Test
{
public:
    AosSubscription* m_pAosSubscription;

    AosStaticProfile* m_pAosStaticProfile;
    MockAosAppContext* m_pMockAosAppContext;
    MockIAosNetTracker m_objMockAosINetTracker;
    MockIRegSubscription m_objMockIRegSubscription;
    AString* m_pAor;
    SipAddress* m_pContactAddress;

    MockIAosSubscriptionListener m_objMockIAosSubscriptionListener;

    // for AosNConfig info
    IAosNConfiguration* m_pOriginAosNConfiguration;
    MockIAosNConfiguration m_objMockAosConfig;

    IAosService* m_pOriginAosService;
    MockIAosService m_objMockAosService;

    IAosRetryRepository* m_pOriginAosRetryRepository;
    MockIAosRetryRepository m_objMockAosRetryRepository;

    IAosTransaction* m_pOriginAosTransaction;
    MockIAosTransaction m_objMockIAosTransaction;

    enum
    {
        AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED = 0,
        AMSG_REG_SUBSCRIPTION_STARTED,
        AMSG_REG_SUBSCRIPTION_START_FAILED,
        AMSG_REG_SUBSCRIPTION_UPDATED,
        AMSG_REG_SUBSCRIPTION_UPDATE_FAILED,
        AMSG_REG_SUBSCRIPTION_REMOVED,
        AMSG_REG_SUBSCRIPTION_TERMINATED
    };

    const AString ADDRESS1 = "sip:1234@ims.google.com:5060";
    const AString ADDRESS2 = "sip:1234@ims.google.com";
    const AString ANONYMOUS_ADDRESS = "sip:anonymous@anonymous.invalid";

protected:
    virtual void SetUp() override
    {
        m_pAosStaticProfile = new AosStaticProfile();
        m_pMockAosAppContext = new MockAosAppContext(m_pAosStaticProfile);

        m_pAor = new AString(ADDRESS1);
        m_pContactAddress = new SipAddress();

        EXPECT_CALL(*m_pMockAosAppContext, GetSlotId()).WillRepeatedly(Return(SLOT_ID));

        EXPECT_CALL(*m_pMockAosAppContext, GetNetTracker())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockAosINetTracker));
        EXPECT_CALL(m_objMockAosINetTracker, GetNetworkType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(static_cast<IMS_UINT32>(AosNetworkType::LTE)));

        m_pAosSubscription = new AosSubscription(static_cast<IAosAppContext*>(m_pMockAosAppContext),
                static_cast<IRegSubscription*>(&m_objMockIRegSubscription), *m_pAor,
                *m_pContactAddress);
        ASSERT_TRUE(m_pAosSubscription != nullptr);

        m_pAosSubscription->SetListener(&m_objMockIAosSubscriptionListener);
        EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_StateChanged(_, _))
                .WillRepeatedly(Return());

        EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
                .WillRepeatedly(Return(IMS_TRUE));

        // save origin pointer
        m_pOriginAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockAosConfig), SLOT_ID);

        m_pOriginAosService = AosProvider::GetInstance()->GetService(SLOT_ID);
        AosProvider::GetInstance()->SetService(
                static_cast<IAosService*>(&m_objMockAosService), SLOT_ID);

        m_pOriginAosRetryRepository = AosProvider::GetInstance()->GetRetryRepository(SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(
                static_cast<IAosRetryRepository*>(&m_objMockAosRetryRepository), SLOT_ID);

        m_pOriginAosTransaction = AosProvider::GetInstance()->GetTransaction(SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(
                static_cast<IAosTransaction*>(&m_objMockIAosTransaction), SLOT_ID);

        EXPECT_CALL(m_objMockIAosTransaction, RemoveListener(_, _)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(_))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));
        EXPECT_CALL(m_objMockIAosTransaction, StartTraffic(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));
        EXPECT_CALL(m_objMockIAosTransaction, StopTraffic(_)).Times(AnyNumber());
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_pOriginAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetService(m_pOriginAosService, SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(m_pOriginAosRetryRepository, SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(m_pOriginAosTransaction, SLOT_ID);

        if (m_pAor)
        {
            delete m_pAor;
        }

        if (m_pContactAddress)
        {
            delete m_pContactAddress;
        }

        if (m_pAosSubscription)
        {
            m_pAosSubscription->SetListener(IMS_NULL);
            m_pAosSubscription->Destroy();
        }

        if (m_pMockAosAppContext)
        {
            delete m_pMockAosAppContext;
        }

        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }
    }

    void SetState(IN IMS_SINT32 nState) { m_pAosSubscription->m_nState = nState; }

    void SetpiRegSubscription(IN IRegSubscription* piRegSubscription)
    {
        m_pAosSubscription->m_piRegSubscription = piRegSubscription;
    }

    void SetRetryCountSubTerminated(IN IMS_UINT32 nRetryCount)
    {
        m_pAosSubscription->m_nRetryCountSubTerminated = nRetryCount;
    }

    void SetRetryCountRegRequired(IN IMS_UINT32 nRetryCount)
    {
        m_pAosSubscription->m_nRetryCountRegRequired = nRetryCount;
    }

    void SetThrottlingCount(IN IMS_UINT32 nThrottlingCount)
    {
        m_pAosSubscription->m_nThrottlingCount = nThrottlingCount;
    }

    void SetObjContactAddress(IN const SipAddress& objContactAddress)
    {
        m_pAosSubscription->m_objContactAddress = objContactAddress;
    }

    IMS_SINT32 GetAorState() { return m_pAosSubscription->m_nAorState; }

    void SetTerminated(IN IMS_BOOL bTerminated) { m_pAosSubscription->SetTerminated(bTerminated); }

    void StartTimer(IN IMS_UINT32 nDuration) { m_pAosSubscription->StartTimer(nDuration); }

    ITimer* GetTimer() { return m_pAosSubscription->m_piRetryTimer; }

    void Timer_TimerExpired(IN ITimer* piTimer) { m_pAosSubscription->Timer_TimerExpired(piTimer); }

    void NotifyListenerEvent(IMS_UINT32 nEvent, IMS_SINT32 nReason, IN IMS_BOOL bHasBody)
    {
        SetpiRegSubscription(&m_objMockIRegSubscription);

        switch (nEvent)
        {
            case AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED:
                m_pAosSubscription->RegSubscription_NotifyReceived(
                        m_pAosSubscription->m_nState, nReason, bHasBody);
                break;

            case AMSG_REG_SUBSCRIPTION_STARTED:
                m_pAosSubscription->RegSubscription_Started();
                break;

            case AMSG_REG_SUBSCRIPTION_START_FAILED:
                m_pAosSubscription->RegSubscription_StartFailed(nReason);
                break;

            case AMSG_REG_SUBSCRIPTION_UPDATED:
                m_pAosSubscription->RegSubscription_Updated();
                break;

            case AMSG_REG_SUBSCRIPTION_UPDATE_FAILED:
                m_pAosSubscription->RegSubscription_UpdateFailed(nReason);
                break;

            case AMSG_REG_SUBSCRIPTION_REMOVED:
                m_pAosSubscription->RegSubscription_Removed();
                break;

            case AMSG_REG_SUBSCRIPTION_TERMINATED:
                m_pAosSubscription->RegSubscription_Terminated(nReason);
                break;

            default:
                break;
        }
    }

    void RefreshTimerExpiredListener(OUT IMS_BOOL& bDoImplicitRefresh)
    {
        m_pAosSubscription->RegSubscription_RefreshTimerExpired(bDoImplicitRefresh);
    }

    void Transaction_SetupPrepared()
    {
        m_pAosSubscription->Transaction_OnConnectionSetupPrepared();
    }
};

TEST_F(AosSubscriptionTest, Initialize)
{
    EXPECT_CALL(m_objMockIRegSubscription, SetRefreshPolicy(0, 1200, 50, 600)).Times(1);
    EXPECT_CALL(m_objMockIRegSubscription, SetListener(m_pAosSubscription)).Times(1);
    EXPECT_CALL(m_objMockIAosTransaction, SetListener(_, _)).Times(1);

    m_pAosSubscription->Initialize();
}

TEST_F(AosSubscriptionTest, Stop)
{
    EXPECT_CALL(m_objMockAosConfig, IsUnSubscription())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosSubscription->Stop();

    EXPECT_CALL(m_objMockIRegSubscription, Unsubscribe()).Times(1);

    m_pAosSubscription->Stop();
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_UNSUBSCRIBING);
}

TEST_F(AosSubscriptionTest, AosSubscriptionStart)
{
    SetState(AosSubscription::STATE_UNSUBSCRIBING);
    EXPECT_FALSE(m_pAosSubscription->Start(IMS_FALSE));

    SetState(AosSubscription::STATE_OFFLINE);

    SetpiRegSubscription(IMS_NULL);
    EXPECT_FALSE(m_pAosSubscription->Start(IMS_FALSE));

    SetState(AosSubscription::STATE_SUBSCRIBED);
    SetpiRegSubscription(static_cast<IRegSubscription*>(&m_objMockIRegSubscription));
    EXPECT_CALL(m_objMockIRegSubscription, Subscribe())
            .WillOnce(Return(IMS_FAILURE))
            .WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_FALSE(m_pAosSubscription->Start(IMS_FALSE));
    EXPECT_TRUE(m_pAosSubscription->Start(IMS_FALSE));
}

TEST_F(AosSubscriptionTest, IsRetryActionDueToRetrycounter)
{
    EXPECT_CALL(m_objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount())
            .WillOnce(Return(5))
            .WillOnce(Return(3))
            .WillOnce(Return(0));

    // AosRetryRepository::TYPE_NORMAL = 0
    EXPECT_CALL(m_objMockAosRetryRepository, IncreaseRetryCount(0))
            .Times(2)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED))
            .Times(1);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF, 0, IMS_FALSE))
            .Times(2);

    EXPECT_TRUE(m_pAosSubscription->IsRetryActionDueToRetrycounter(IMS_FALSE));

    SetTerminated(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED))
            .Times(1);

    EXPECT_TRUE(m_pAosSubscription->IsRetryActionDueToRetrycounter(IMS_TRUE));

    EXPECT_FALSE(m_pAosSubscription->IsRetryActionDueToRetrycounter(IMS_TRUE));
}

TEST_F(AosSubscriptionTest, CheckSubscriptionTerminated)
{
    EXPECT_CALL(m_objMockAosConfig, GetRetryCountSubErrorSubTerminated())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(2));

    EXPECT_FALSE(m_pAosSubscription->IsSubscriptionTerminated(403));

    ImsVector<IMS_SINT32> objErrSubTerminated;
    objErrSubTerminated.Clear();
    objErrSubTerminated.Add(404);
    objErrSubTerminated.Add(403);

    EXPECT_CALL(m_objMockAosConfig, GetSubErrorSubTerminated())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrSubTerminated));

    SetRetryCountSubTerminated(0);
    EXPECT_FALSE(m_pAosSubscription->IsSubscriptionTerminated(403));

    SetRetryCountSubTerminated(1);
    EXPECT_TRUE(m_pAosSubscription->IsSubscriptionTerminated(403));

    objErrSubTerminated.Clear();
    objErrSubTerminated.Add(5);
    objErrSubTerminated.Add(403);
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorSubTerminated())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrSubTerminated));

    SetRetryCountSubTerminated(1);
    EXPECT_TRUE(m_pAosSubscription->IsSubscriptionTerminated(504));
}

TEST_F(AosSubscriptionTest, CheckInitialRegRequired)
{
    SetState(AosSubscription::STATE_SUBSTOP);
    EXPECT_CALL(m_objMockAosConfig, GetRetryCountSubErrorRegRequired())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(2));

    EXPECT_FALSE(m_pAosSubscription->IsInitialRegistrationRequired(403, IMS_FALSE));

    ImsVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Clear();
    objErrRegRequired.Add(404);
    objErrRegRequired.Add(403);

    EXPECT_CALL(m_objMockAosConfig, GetSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequired));

    SetRetryCountRegRequired(0);
    EXPECT_FALSE(m_pAosSubscription->IsInitialRegistrationRequired(403, IMS_FALSE));

    SetRetryCountRegRequired(1);

    EXPECT_CALL(m_objMockAosConfig, GetRegRetryCountResetPolicy())
            .WillOnce(Return(0))
            .WillOnce(Return(1));

    // ReportState() if result is true.
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED))
            .Times(1);

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_FALSE))
            .Times(1);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_TRUE))
            .Times(1);

    EXPECT_TRUE(m_pAosSubscription->IsInitialRegistrationRequired(403, IMS_FALSE));

    SetTerminated(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED))
            .Times(1);

    objErrRegRequired.Clear();
    objErrRegRequired.Add(5);
    objErrRegRequired.Add(403);
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequired));

    SetRetryCountRegRequired(1);
    EXPECT_TRUE(m_pAosSubscription->IsInitialRegistrationRequired(504, IMS_TRUE));
}

TEST_F(AosSubscriptionTest, CheckInitialRegWithNextPcscfRequired)
{
    SetState(AosSubscription::STATE_SUBSTOP);
    ImsVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Clear();
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));

    EXPECT_FALSE(m_pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(403, IMS_FALSE));

    objErrRegRequiredWithNextPcscf.Add(404);
    objErrRegRequiredWithNextPcscf.Add(403);
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));

    // ReportState() if result is true.
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED))
            .Times(1);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF, 0, IMS_FALSE))
            .Times(2);
    EXPECT_TRUE(m_pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(403, IMS_FALSE));

    SetTerminated(IMS_FALSE);

    objErrRegRequiredWithNextPcscf.Clear();
    objErrRegRequiredWithNextPcscf.Add(6);
    objErrRegRequiredWithNextPcscf.Add(403);
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));
    EXPECT_TRUE(m_pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(603, IMS_TRUE));
}

TEST_F(AosSubscriptionTest, CheckInitialRegRequiredInWifi)
{
    SetState(AosSubscription::STATE_SUBSTOP);
    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(*m_pMockAosAppContext, GetConnection())
            .Times(6)
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(6)
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_FALSE(m_pAosSubscription->IsInitialRegistrationRequiredInWifi(403, IMS_FALSE));

    // GetVowifiSubErrorRegRequired() - no info
    ImsVector<IMS_SINT32> objErrRegRequiredInWifi;
    objErrRegRequiredInWifi.Clear();
    EXPECT_CALL(m_objMockAosConfig, GetVowifiSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredInWifi));

    EXPECT_FALSE(m_pAosSubscription->IsInitialRegistrationRequiredInWifi(403, IMS_FALSE));

    // GetVowifiSubErrorRegRequired() - 404, 403
    objErrRegRequiredInWifi.Add(404);
    objErrRegRequiredInWifi.Add(403);
    EXPECT_CALL(m_objMockAosConfig, GetVowifiSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredInWifi));

    EXPECT_CALL(m_objMockAosConfig,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_SUB_403))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    // RequestCommand
    EXPECT_CALL(m_objMockAosConfig, GetRegRetryCountResetPolicy())
            .WillOnce(Return(2))
            .WillOnce(Return(0));

    // ReportState() if result is true.
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED))
            .Times(1);

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED_WITH_SUB_403_MSG, 0, IMS_TRUE))
            .Times(1);

    EXPECT_TRUE(m_pAosSubscription->IsInitialRegistrationRequiredInWifi(403, IMS_FALSE));
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);

    SetTerminated(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED))
            .Times(1);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_FALSE))
            .Times(1);

    objErrRegRequiredInWifi.Clear();
    objErrRegRequiredInWifi.Add(3);
    objErrRegRequiredInWifi.Add(403);
    EXPECT_CALL(m_objMockAosConfig, GetVowifiSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredInWifi));
    EXPECT_TRUE(m_pAosSubscription->IsInitialRegistrationRequiredInWifi(301, IMS_TRUE));
}

TEST_F(AosSubscriptionTest, CheckIsReSubscriptionStopped)
{
    SetState(AosSubscription::STATE_SUBSTOP);
    ImsVector<IMS_SINT32> objErrResubStopped;
    objErrResubStopped.Clear();
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorStoppingResub())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrResubStopped));

    EXPECT_FALSE(m_pAosSubscription->IsResubscriptionStopped(403));

    objErrResubStopped.Add(404);
    objErrResubStopped.Add(403);
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorStoppingResub())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrResubStopped));
    EXPECT_TRUE(m_pAosSubscription->IsResubscriptionStopped(403));

    objErrResubStopped.Clear();
    objErrResubStopped.Add(403);
    objErrResubStopped.Add(5);
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorStoppingResub())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrResubStopped));
    EXPECT_TRUE(m_pAosSubscription->IsResubscriptionStopped(503));
}

TEST_F(AosSubscriptionTest, ProcessFailedStatusCode)
{
    // IsRetryActionDueToRetrycounter() - 1st: true, others: false
    EXPECT_CALL(m_objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount())
            .WillOnce(Return(5))
            .WillRepeatedly(Return(0));

    // AosRetryRepository::TYPE_NORMAL = 0
    EXPECT_CALL(m_objMockAosRetryRepository, IncreaseRetryCount(0)).WillOnce(Return(IMS_FALSE));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));

    // IsSubscriptionTerminated() - 1st: true, others: false
    EXPECT_CALL(m_objMockAosConfig, GetRetryCountSubErrorSubTerminated())
            .WillOnce(Return(1))
            .WillOnce(Return(2))
            .WillRepeatedly(Return(0));

    ImsVector<IMS_SINT32> objErrSubTerminated;
    objErrSubTerminated.Clear();
    objErrSubTerminated.Add(404);
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorSubTerminated())
            .WillOnce(ReturnRef(objErrSubTerminated))
            .WillOnce(ReturnRef(objErrSubTerminated));
    SetRetryCountSubTerminated(0);

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED))
            .Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_SUB_TERMINATED, 0, IMS_FALSE))
            .Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_FALSE))
            .Times(AnyNumber());

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(404, IMS_FALSE));

    SetRetryCountSubTerminated(0);
    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(404, IMS_TRUE));

    // IsInitialRegistrationRequired() - 1st: true, others: false
    EXPECT_CALL(m_objMockAosConfig, GetRetryCountSubErrorRegRequired())
            .WillOnce(Return(3))
            .WillRepeatedly(Return(0));

    ImsVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Clear();
    objErrRegRequired.Add(403);

    EXPECT_CALL(m_objMockAosConfig, GetSubErrorRegRequired())
            .WillOnce(ReturnRef(objErrRegRequired));

    SetRetryCountRegRequired(2);

    EXPECT_CALL(m_objMockAosConfig, GetRegRetryCountResetPolicy()).WillRepeatedly(Return(0));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));

    // IsInitialRegistrationWithNextPcscfRequired() - 1st: true, others: false
    ImsVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Clear();
    objErrRegRequiredWithNextPcscf.Add(403);
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .WillOnce(ReturnRef(objErrRegRequiredWithNextPcscf));

    SetTerminated(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF, 0, IMS_FALSE))
            .Times(1);
    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));

    // ImsVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Clear();
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));

    // IsInitialRegistrationRequiredInWifi() - 1st: true, others: false
    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(*m_pMockAosAppContext, GetConnection())
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    ImsVector<IMS_SINT32> objErrRegRequiredInWifi;
    objErrRegRequiredInWifi.Clear();
    objErrRegRequiredInWifi.Add(403);
    EXPECT_CALL(m_objMockAosConfig, GetVowifiSubErrorRegRequired())
            .WillOnce(ReturnRef(objErrRegRequiredInWifi));

    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_Request(_, _, _))
            .Times(AnyNumber());

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));

    // nStatusCode == SipStatusCode::SC_423 refresh: IMS_FALSE;
    MockISipMessage objMockSipMsg;

    EXPECT_CALL(m_objMockIRegSubscription, GetPreviousResponse())
            .WillRepeatedly(Return(static_cast<ISipMessage*>(&objMockSipMsg)));

    AString strMinTime = "600";
    AString strMinTime2 = "";
    EXPECT_CALL(objMockSipMsg, GetHeader(ISipHeader::MIN_EXPIRES, 0, AString::ConstNull()))
            .WillOnce(Return(strMinTime))
            .WillOnce(Return(strMinTime))
            .WillOnce(Return(strMinTime))
            .WillOnce(Return(strMinTime2));

    EXPECT_CALL(m_objMockIRegSubscription, SetExpires(600)).Times(3);

    EXPECT_CALL(m_objMockIRegSubscription, Subscribe())
            .WillOnce(Return(IMS_SUCCESS))
            .WillOnce(Return(IMS_SUCCESS))
            .WillOnce(Return(IMS_FAILURE));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(423, IMS_FALSE));

    // nStatusCode == SipStatusCode::SC_423 refresh: IMS_TRUE;
    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(423, IMS_TRUE));

    // nStatusCode == SipStatusCode::SC_423 sendSubscribe() : FALSE;
    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(423, IMS_FALSE));

    // nStatusCode == SipStatusCode::SC_423 nMinTime : 0;
    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(423, IMS_FALSE));

    // nStatusCode == SipStatusCode::SC_504;
    ImsList<ISipMessageBodyPart*> objBodyParts;
    objBodyParts.Clear();
    SipMessageBodyPart objBodyPart;
    ISipMessageBodyPart* piBodyPart = static_cast<ISipMessageBodyPart*>(&objBodyPart);

    AString strContent = "";
    strContent.Append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    strContent.Append("<ims-3gpp version=\"1\"><alternative-service>");
    strContent.Append("<type>restoration</type>");
    strContent.Append("<reason></reason>");
    strContent.Append("<action>initial-registration</action>");
    strContent.Append("</alternative-service></ims-3gpp>");
    ByteArray objContent(strContent);

    piBodyPart->SetContent(objContent);
    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, "application/3gpp-ims+xml");

    objBodyParts.Append(piBodyPart);

    EXPECT_CALL(objMockSipMsg, GetBodyParts())
            .WillOnce(Return(objBodyParts))
            .WillOnce(Return(objBodyParts));

    EXPECT_CALL(m_objMockAosConfig, GetRegRetryCountResetPolicy())
            .WillOnce(Return(0))
            .WillOnce(Return(0));
    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(504, IMS_FALSE));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(504, IMS_TRUE));

    // nStatusCode == SipStatusCode::SC_504 GetBodyParts empty;
    objBodyParts.Clear();
    EXPECT_CALL(objMockSipMsg, GetBodyParts()).WillOnce(Return(objBodyParts));
    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(504, IMS_FALSE));

    // nStatusCode == SipStatusCode::SC_504 SipMsg null;
    EXPECT_CALL(m_objMockIRegSubscription, GetPreviousResponse())
            .WillRepeatedly(Return(static_cast<ISipMessage*>(IMS_NULL)));
    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(504, IMS_FALSE));

    // bIsRefreshed IMS_TRUE nStatusCode == SipStatusCode::SC_481
    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(481, IMS_TRUE));

    // bIsRefreshed IMS_TRUE IsResubscriptionStopped
    ImsVector<IMS_SINT32> objErrResubStopped;
    objErrResubStopped.Clear();
    objErrResubStopped.Add(403);
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorStoppingResub())
            .WillOnce(ReturnRef(objErrResubStopped));
    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_TRUE));

    objErrResubStopped.Clear();
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorStoppingResub())
            .WillRepeatedly(ReturnRef(objErrResubStopped));

    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_TRUE));
}

TEST_F(AosSubscriptionTest, IsRegAfterWaitRequiredByNotify)
{
    IMS_UINT32 nFeature = 0;
    nFeature |= 0x01;

    // NOTIFY_TERMINATED_EXPIRED = 0x01,
    // NOTIFY_TERMINATED_DEACTIVATED = 0x02,
    EXPECT_CALL(m_objMockAosConfig, GetNotifyEventForInitialRegWithWaitTime())
            .Times(2)
            .WillOnce(Return(0x02))
            .WillOnce(Return(0x01));
    EXPECT_FALSE(m_pAosSubscription->IsRegAfterWaitRequiredByNotify(nFeature));

    EXPECT_TRUE(m_pAosSubscription->IsRegAfterWaitRequiredByNotify(nFeature));
}

TEST_F(AosSubscriptionTest, IsWfcErrorMessageSupportedWithStateChecked)
{
    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(*m_pMockAosAppContext, GetConnection())
            .Times(5)
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(5)
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockAosConfig,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_SUB_403))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockAosConfig,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_NOTIFY_TERMINATED))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    SetState(AosSubscription::STATE_OFFLINE);
    EXPECT_FALSE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_SUB_403));

    SetState(AosSubscription::STATE_OFFLINE);
    EXPECT_FALSE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_SUB_403));

    SetState(AosSubscription::STATE_UNSUBSCRIBING);
    EXPECT_FALSE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_SUB_403));

    SetState(AosSubscription::STATE_SUBSCRIBED);
    EXPECT_TRUE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_SUB_403));

    SetState(AosSubscription::STATE_SUBSCRIBED);
    EXPECT_TRUE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_NOTIFY_TERMINATED));
}

TEST_F(AosSubscriptionTest, CheckNotifyReceived)
{
    SetState(AosSubscription::STATE_SUBSCRIBED);
    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_FALSE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    MockIRegInfo objMockIRegInfo;
    EXPECT_CALL(m_objMockIRegSubscription, GetRegInfo())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IRegInfo*>(&objMockIRegInfo)));

    MockIRegInfoRegistration objMockIRegInfoRegistration;
    EXPECT_CALL(objMockIRegInfo, GetRegistration(*m_pAor))
            .Times(AnyNumber())
            .WillOnce(ReturnNull())
            .WillOnce(ReturnNull())
            .WillRepeatedly(
                    Return(static_cast<IRegInfoRegistration*>(&objMockIRegInfoRegistration)));

    EXPECT_CALL(m_objMockAosConfig, IsRegistrationEventForCatRequired())
            .Times(4)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    ImsList<IRegInfoContact*> objContact;
    objContact.Clear();

    EXPECT_CALL(objMockIRegInfoRegistration, GetContacts()).Times(1).WillOnce(Return(objContact));
    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    // RegInfoContact1
    MockIRegInfoContact objMockIRegInfoContact1;
    EXPECT_CALL(objMockIRegInfoContact1, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IRegInfoContact::STATE_ACTIVE));

    EXPECT_CALL(objMockIRegInfoContact1, GetEvent())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IRegInfoContact::EVENT_REGISTERED));

    SipAddress objSipAddress;
    objSipAddress.Create(ANONYMOUS_ADDRESS);
    EXPECT_CALL(objMockIRegInfoContact1, GetUri())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objSipAddress));

    AString strQvalue = "";
    EXPECT_CALL(objMockIRegInfoContact1, GetQValue())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strQvalue));

    EXPECT_CALL(objMockIRegInfoContact1, GetRetryAfterValue())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(objMockIRegInfoContact1, GetExpiresValue())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3600));

    // RegInfoContact2
    MockIRegInfoContact objMockIRegInfoContact2;
    EXPECT_CALL(objMockIRegInfoContact2, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IRegInfoContact::STATE_ACTIVE));
    EXPECT_CALL(objMockIRegInfoContact2, GetEvent())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IRegInfoContact::EVENT_REGISTERED));

    SipAddress objSipAddress2;
    objSipAddress2.Create(ADDRESS1);
    EXPECT_CALL(objMockIRegInfoContact2, GetUri())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objSipAddress2));

    strQvalue = "qvalue";
    EXPECT_CALL(objMockIRegInfoContact2, GetQValue())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strQvalue));

    EXPECT_CALL(objMockIRegInfoContact2, GetRetryAfterValue())
            .Times(AnyNumber())
            .WillRepeatedly(Return(30));

    EXPECT_CALL(objMockIRegInfoContact2, GetExpiresValue())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3600));

    objContact.Append(static_cast<IRegInfoContact*>(&objMockIRegInfoContact1));
    objContact.Append(static_cast<IRegInfoContact*>(&objMockIRegInfoContact2));

    EXPECT_CALL(objMockIRegInfoRegistration, GetContacts())
            .Times(3)
            .WillRepeatedly(Return(objContact));

    // GetRegInfoContact() == IMS_NULL
    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    EXPECT_CALL(m_objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount())
            .WillOnce(Return(5))
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockAosService, NotifyRegEventState(AosRegEvent::ACTIVE)).Times(1);

    EXPECT_CALL(m_objMockAosRetryRepository, ResetRetryCount(0)).Times(1);

    SipAddress objContactAddr;
    objContactAddr.Create(ADDRESS2);
    SetObjContactAddress(objContactAddr);
    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_ACTIVE);

    objContactAddr.RemoveAllParameters();
    objContactAddr.Create(ADDRESS1);
    SetObjContactAddress(objContactAddr);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_ACTIVE);

    // For STATE_TERMINATED
    MockIRegInfoContact objMockIRegInfoContact3;
    EXPECT_CALL(objMockIRegInfoContact3, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IRegInfoContact::STATE_TERMINATED));
    EXPECT_CALL(objMockIRegInfoContact3, GetEvent())
            .Times(14)
            .WillOnce(Return(IRegInfoContact::EVENT_SHORTENED))
            .WillOnce(Return(IRegInfoContact::EVENT_SHORTENED))
            .WillOnce(Return(IRegInfoContact::EVENT_EXPIRED))
            .WillOnce(Return(IRegInfoContact::EVENT_EXPIRED))
            .WillOnce(Return(IRegInfoContact::EVENT_EXPIRED))
            .WillOnce(Return(IRegInfoContact::EVENT_EXPIRED))
            .WillOnce(Return(IRegInfoContact::EVENT_DEACTIVATED))
            .WillOnce(Return(IRegInfoContact::EVENT_DEACTIVATED))
            .WillOnce(Return(IRegInfoContact::EVENT_PROBATION))
            .WillOnce(Return(IRegInfoContact::EVENT_PROBATION))
            .WillOnce(Return(IRegInfoContact::EVENT_UNREGISTERED))
            .WillOnce(Return(IRegInfoContact::EVENT_UNREGISTERED))
            .WillOnce(Return(IRegInfoContact::EVENT_REJECTED))
            .WillOnce(Return(IRegInfoContact::EVENT_REJECTED));

    EXPECT_CALL(objMockIRegInfoContact3, GetUri())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objSipAddress2));

    strQvalue = "";
    EXPECT_CALL(objMockIRegInfoContact3, GetQValue())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strQvalue));

    EXPECT_CALL(objMockIRegInfoContact3, GetRetryAfterValue())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(30));

    EXPECT_CALL(objMockIRegInfoContact3, GetExpiresValue())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3600));

    objContact.Clear();
    objContact.Append(static_cast<IRegInfoContact*>(&objMockIRegInfoContact3));

    EXPECT_CALL(objMockIRegInfoRegistration, GetContacts())
            .Times(AnyNumber())
            .WillRepeatedly(Return(objContact));

    // STATE_TERMINATE
    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    EXPECT_CALL(m_objMockAosConfig, GetNotifyEventForInitialRegWithWaitTime())
            .Times(6)
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_EXPIRED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED));

    EXPECT_CALL(m_objMockAosConfig, GetNotifyEventForInitialRegistration())
            .Times(4)
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_EXPIRED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_DEACTIVATED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_PROBATION));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_TERMINATED, 0, IMS_FALSE))
            .Times(1);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    // ReportNotifyEvent() - bRegRequired == TRUE
    EXPECT_CALL(m_objMockAosConfig, GetNotifyWaitTime()).Times(2).WillRepeatedly(Return(60));

    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(*m_pMockAosAppContext, GetConnection())
            .Times(2)
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_TERMINATED, 0, IMS_FALSE))
            .Times(3);

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 60, IMS_FALSE))
            .Times(1);

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(
                    AosSubscription::CMD_REG_REQUIRED_WITH_NOTIFY_TERMINATED_MSG, 0, IMS_FALSE))
            .Times(1);

    EXPECT_CALL(m_objMockAosConfig,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_NOTIFY_TERMINATED))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);
}

TEST_F(AosSubscriptionTest, RegSubscription_RefreshTimerExpired)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;

    // CheckRadioReadyAndSetRadioWaiting() - IMS_TRUE
    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_TRUE));
    RefreshTimerExpiredListener(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHING);
    EXPECT_EQ(bDoImplicitRefresh, IMS_TRUE);

    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_FALSE));
    RefreshTimerExpiredListener(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
    EXPECT_EQ(bDoImplicitRefresh, IMS_FALSE);
}

TEST_F(AosSubscriptionTest, RefreshTimerExpired_RadioReadyAndSetRadioWaiting)
{
    // CheckRadioReadyAndSetRadioWaiting() - IMS_FALSE (2 cases)
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;

    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(_))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));

    RefreshTimerExpiredListener(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);

    EXPECT_CALL(m_objMockIAosTransaction, StartTraffic(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    RefreshTimerExpiredListener(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);

    // for checking SetRadioWaiting(IMS_TRUE)
    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillOnce(Return(IMS_FALSE));
    // IsRadioWaiting()
    Transaction_SetupPrepared();
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionStarted)
{
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_SUBSCRIBED, AosSubscription::REASON_SUB_ESTABLISHED))
            .Times(1);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_STARTED, 0, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBED);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionUpdated)
{
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_SUBSCRIBED, AosSubscription::REASON_SUB_ESTABLISHED))
            .Times(1);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_UPDATED, 0, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBED);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionStartFailed_Others)
{
    EXPECT_CALL(m_objMockAosConfig, IsRegRetryIntervalsUsedForSub())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockAosConfig, GetRegistrationRetryBaseTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(30000));
    EXPECT_CALL(m_objMockAosConfig, GetRegistrationRetryMaxTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1800000));

    // SetRetryTimer - GetNextThrottlingTime(objRetryIntervals)
    ImsVector<IMS_SINT32> objRetryIntervals;
    objRetryIntervals.Clear();
    objRetryIntervals.Add(10);
    objRetryIntervals.Add(10);
    objRetryIntervals.Add(10);
    EXPECT_CALL(m_objMockAosConfig, GetRegRetryIntervals())
            .WillOnce(ReturnRef(objRetryIntervals))
            .WillOnce(ReturnRef(objRetryIntervals));

    SetThrottlingCount(0);
    ImsVector<IMS_SINT32> objRetryRandomIntervals;
    objRetryRandomIntervals.Clear();
    objRetryRandomIntervals.Add(0);
    objRetryRandomIntervals.Add(5);
    objRetryRandomIntervals.Add(0);
    EXPECT_CALL(m_objMockAosConfig, GetRegRandomRetryIntervals())
            .WillOnce(ReturnRef(objRetryRandomIntervals))
            .WillOnce(ReturnRef(objRetryRandomIntervals));

    // ProcessStartFailed_Others - m_pAosSubscription->SetRetryTimer(IMS_FALSE);
    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_INTERNAL_ERROR, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    SetThrottlingCount(3);
    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_INTERNAL_ERROR, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionStartFailed_StatusCode)
{
    // ProcessStartFailed_StatusCode - m_pAosSubscription->SetRetryTimer(IMS_FALSE);
    // IsRetryActionDueToRetrycounter - FALSE
    EXPECT_CALL(m_objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(3)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount()).Times(3).WillRepeatedly(Return(0));

    // IsSubscriptionTerminated - FALSE
    EXPECT_CALL(m_objMockAosConfig, GetRetryCountSubErrorSubTerminated())
            .Times(3)
            .WillRepeatedly(Return(0));

    // IsInitialRegistrationRequired - FALSE
    EXPECT_CALL(m_objMockAosConfig, GetRetryCountSubErrorRegRequired())
            .Times(3)
            .WillRepeatedly(Return(0));

    // IsInitialRegistrationWithNextPcscfRequired - FALSE
    ImsVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Clear();
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(3)
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));

    // IsInitialRegistrationRequiredInWifi - FALSE
    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled()).Times(3).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(*m_pMockAosAppContext, GetConnection())
            .Times(3)
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    SetThrottlingCount(0);
    EXPECT_CALL(m_objMockAosConfig, IsRegRetryIntervalsUsedForSub())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockAosConfig, GetRegistrationRetryBaseTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(30000));
    EXPECT_CALL(m_objMockAosConfig, GetRegistrationRetryMaxTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1800000));

    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_TRANSACTION_TIMEOUT, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    MockISipMessage objMockSipMsg;
    EXPECT_CALL(objMockSipMsg, GetStatusCode()).WillRepeatedly(Return(481));

    AString strHeader = "";
    strHeader.Append("60");
    EXPECT_CALL(objMockSipMsg, GetHeader(ISipHeader::RETRY_AFTER_SEC, 0, AString::ConstNull()))
            .WillOnce(Return(strHeader));

    EXPECT_CALL(m_objMockIRegSubscription, GetPreviousResponse())
            .WillOnce(Return(static_cast<ISipMessage*>(&objMockSipMsg)))
            .WillOnce(Return(static_cast<ISipMessage*>(&objMockSipMsg)))
            .WillOnce(ReturnNull())
            .WillOnce(ReturnNull());

    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionUpdateFailed_others)
{
    EXPECT_CALL(m_objMockAosConfig, IsRegRetryIntervalsUsedForSub())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockAosConfig, GetRegistrationRetryBaseTime())
            .Times(1)
            .WillOnce(Return(30000));
    EXPECT_CALL(m_objMockAosConfig, GetRegistrationRetryMaxTime())
            .Times(1)
            .WillOnce(Return(1800000));

    // m_pAosSubscription->SetRetryTimer(IMS_FALSE);
    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_UPDATE_FAILED,
            IRegSubscription::REASON_REFRESH_INTERNAL_ERROR, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionUpdateFailed_StatusCode)
{
    // IsRetryActionDueToRetrycounter - FALSE
    EXPECT_CALL(m_objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(2)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount()).Times(2).WillRepeatedly(Return(0));

    // IsSubscriptionTerminated - FALSE
    EXPECT_CALL(m_objMockAosConfig, GetRetryCountSubErrorSubTerminated())
            .Times(2)
            .WillRepeatedly(Return(0));

    // IsInitialRegistrationRequired - FALSE
    EXPECT_CALL(m_objMockAosConfig, GetRetryCountSubErrorRegRequired())
            .Times(2)
            .WillRepeatedly(Return(0));

    // IsInitialRegistrationWithNextPcscfRequired - FALSE
    ImsVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Clear();
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));

    // IsInitialRegistrationRequiredInWifi - FALSE
    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(*m_pMockAosAppContext, GetConnection())
            .Times(2)
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    // IsResubscriptionStopped - FALSE
    ImsVector<IMS_SINT32> objErrResubStopped;
    objErrResubStopped.Clear();
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorStoppingResub())
            .Times(2)
            .WillRepeatedly(ReturnRef(objErrResubStopped));

    EXPECT_CALL(m_objMockAosConfig, IsRegRetryIntervalsUsedForSub())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockAosConfig, GetRegistrationRetryBaseTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(30000));
    EXPECT_CALL(m_objMockAosConfig, GetRegistrationRetryMaxTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1800000));

    MockISipMessage objMockSipMsg;
    EXPECT_CALL(objMockSipMsg, GetStatusCode()).WillRepeatedly(Return(404));

    EXPECT_CALL(m_objMockIRegSubscription, GetPreviousResponse()).WillRepeatedly(ReturnNull());

    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_TRANSACTION_TIMEOUT, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);

    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_REFRESH_TIMEOUT, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);

    EXPECT_CALL(m_objMockIRegSubscription, GetPreviousResponse()).WillRepeatedly(ReturnNull());
    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);
}

TEST_F(AosSubscriptionTest, RegSubscription_End)
{
    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_REMOVED, IRegSubscription::REASON_NONE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_TERMINATED, IRegSubscription::REASON_NO_EXPIRES, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

TEST_F(AosSubscriptionTest, ProcessTimerExpired)
{
    IMS_UINT32 nCurrState = m_pAosSubscription->GetState();
    Timer_TimerExpired(IMS_NULL);
    EXPECT_EQ(m_pAosSubscription->GetState(), nCurrState);

    // Timer_TimerExpired (piTimer != m_piRetryTimer) to do

    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_FALSE));

    // ProcessTimerExpired() - return;
    SetState(AosSubscription::STATE_OFFLINE);
    StartTimer(30);
    Timer_TimerExpired(GetTimer());
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);

    // ProcessTimerExpired() - Subscription_CanBeTransmitted() == IMS_FALSE
    SetState(AosSubscription::STATE_SUBSTOP);

    StartTimer(30);
    Timer_TimerExpired(GetTimer());
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    SetState(AosSubscription::STATE_SUBREFRESHSTOP);
    StartTimer(30);
    Timer_TimerExpired(GetTimer());
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);

    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_TRUE));

    // ProcessTimerExpired() - Subscription_CanBeTransmitted() == IMS_TRUE
    // SendSubscribe() == IMS_FALSE m_piRegSubscription == IMS_NULL
    SetState(AosSubscription::STATE_SUBSTOP);
    StartTimer(30);
    Timer_TimerExpired(GetTimer());

    SetpiRegSubscription(IMS_NULL);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBING);

    // ProcessTimerExpired() - SendSubscribe() = IMS_TRUE Subscribe() == IMS_SUCCESS
    SetpiRegSubscription(static_cast<IRegSubscription*>(&m_objMockIRegSubscription));
    EXPECT_CALL(m_objMockIRegSubscription, Subscribe())
            .WillOnce(Return(IMS_FAILURE))
            .WillRepeatedly(Return(IMS_SUCCESS));
    Timer_TimerExpired(GetTimer());
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBING);

    SetState(AosSubscription::STATE_SUBSTOP);
    StartTimer(30);
    Timer_TimerExpired(GetTimer());

    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    SetState(AosSubscription::STATE_SUBREFRESHSTOP);
    StartTimer(30);
    Timer_TimerExpired(GetTimer());
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHING);
}

TEST_F(AosSubscriptionTest, Print)
{
    int nState = AosSubscription::STATE_OFFLINE;
    while (nState <= AosSubscription::STATE_UNSUBSCRIBING)
    {
        SetState(nState);
        m_pAosSubscription->StateToString(nState);
        nState++;
    }
    m_pAosSubscription->StateToString(nState);

    int nReason = IRegSubscription::REASON_NONE;
    while (nReason <= IRegSubscription::REASON_NOTIFY_TERMINATED)
    {
        m_pAosSubscription->RegSubReasonToString(nReason);
        nReason++;
    }
    m_pAosSubscription->RegSubReasonToString(nReason);

    int nNotifyState = IRegInfoContact::STATE_CREATED;
    while (nNotifyState <= IRegInfoContact::STATE_TERMINATED)
    {
        m_pAosSubscription->RegInfoStateToString(nNotifyState);
        nNotifyState++;
    }
    m_pAosSubscription->RegInfoStateToString(nNotifyState);

    int nEvent = IRegInfoContact::EVENT_REGISTERED;
    while (nEvent <= IRegInfoContact::EVENT_REJECTED)
    {
        m_pAosSubscription->RegInfoEventToString(nEvent);
        nEvent++;
    }
    m_pAosSubscription->RegInfoEventToString(nEvent);

    SetState(AosSubscription::STATE_OFFLINE);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}