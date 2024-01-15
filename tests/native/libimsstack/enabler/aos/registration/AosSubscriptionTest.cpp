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
#include "IImsRadio.h"
#include "IRegSubscription.h"
#include "ISipHeader.h"
#include "SipAddress.h"
#include "SipMessageBodyPart.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosConnection.h"
#include "private/ConfigurationManager.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "registration/AosSubscription.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

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

class TestAosSubscription : public AosSubscription
{
public:
    inline TestAosSubscription(IN IAosAppContext* piAppContext,
            IN IRegSubscription* piRegSubscription, IN const AString& strAoR,
            IN const SipAddress& objContactAddress) :
            AosSubscription(piAppContext, piRegSubscription, strAoR, objContactAddress)
    {
    }

    FRIEND_TEST(AosSubscriptionTest, Initialize);
    FRIEND_TEST(AosSubscriptionTest, CallUnsubscribeInStopWhenUnsubscriptionIsSupported);
    FRIEND_TEST(AosSubscriptionTest, NotCallUnsubscribeInStopWhenUnsubscriptionIsNotSupported);
    FRIEND_TEST(AosSubscriptionTest, StateIsInvalidWhenStart);
    FRIEND_TEST(AosSubscriptionTest, SubscribeFailedInStartWhenRegSubscriptionIsNull);
    FRIEND_TEST(AosSubscriptionTest, StartReturnFalseWhenSendSubscribeFailed);
    FRIEND_TEST(AosSubscriptionTest, StartReturnTrueWhenSendSubscribePassed);
    FRIEND_TEST(AosSubscriptionTest, StartReturnFalseWhenStartWithCheckRadioIsTrue);

    FRIEND_TEST(AosSubscriptionTest, IsRetryActionDueToRetryCounter);
    FRIEND_TEST(AosSubscriptionTest, CheckSubscriptionTerminated);
    FRIEND_TEST(AosSubscriptionTest, CheckInitialRegRequired);
    FRIEND_TEST(AosSubscriptionTest, CheckInitialRegWithNextPcscfRequired);
    FRIEND_TEST(AosSubscriptionTest, CheckInitialRegRequiredInWifi);
    FRIEND_TEST(AosSubscriptionTest, CheckIsReSubscriptionStopped);
    FRIEND_TEST(AosSubscriptionTest, ProcessFailedStatusCode);

    FRIEND_TEST(AosSubscriptionTest, IsRegAfterWaitRequiredByNotify);
    FRIEND_TEST(AosSubscriptionTest, IsWfcErrorMessageSupportedWithStateChecked);
    FRIEND_TEST(AosSubscriptionTest, CheckNotifyReceived);
    FRIEND_TEST(AosSubscriptionTest, RegSubscription_RefreshTimerExpired);
    FRIEND_TEST(AosSubscriptionTest, RefreshTimerExpired_RadioReadyAndSetRadioWaiting);
    FRIEND_TEST(AosSubscriptionTest, CheckRegSubscriptionStarted);
    FRIEND_TEST(AosSubscriptionTest, CheckRegSubscriptionUpdated);
    FRIEND_TEST(AosSubscriptionTest, CheckRegSubscriptionStartFailed_Others);
    FRIEND_TEST(AosSubscriptionTest, CheckRegSubscriptionStartFailed_StatusCode_Done);
    FRIEND_TEST(AosSubscriptionTest, CheckRegSubscriptionStartFailed_StatusCode);
    FRIEND_TEST(AosSubscriptionTest, CheckRegSubscriptionUpdateFailed_others);
    FRIEND_TEST(AosSubscriptionTest, CheckRegSubscriptionUpdateFailed_StatusCode_Done);
    FRIEND_TEST(AosSubscriptionTest, CheckRegSubscriptionUpdateFailed_StatusCode);
    FRIEND_TEST(AosSubscriptionTest, RegSubscription_End);
    FRIEND_TEST(AosSubscriptionTest, ProcessTimerExpired_StateInvalid);
    FRIEND_TEST(AosSubscriptionTest, ProcessTimerExpired);
    FRIEND_TEST(AosSubscriptionTest, TransOnConnectionFailed);
    FRIEND_TEST(AosSubscriptionTest, TransOnConnectionFailed_CallPrepared);
    FRIEND_TEST(AosSubscriptionTest, TransOnTrafficPriorityChanged);
    FRIEND_TEST(AosSubscriptionTest, Print);

public:
    void SetRegSubscription(IN IRegSubscription* piRegSubscription)
    {
        m_piRegSubscription = piRegSubscription;
    }

    void SetRetryCountSubTerminated(IN IMS_UINT32 nRetryCount)
    {
        m_nRetryCountSubTerminated = nRetryCount;
    }

    void SetRetryCountRegRequired(IN IMS_UINT32 nRetryCount)
    {
        m_nRetryCountRegRequired = nRetryCount;
    }

    void SetThrottlingCount(IN IMS_UINT32 nThrottlingCount)
    {
        m_nThrottlingCount = nThrottlingCount;
    }

    void SetContactAddress(IN const SipAddress& objContactAddress)
    {
        m_objContactAddress = objContactAddress;
    }

    IMS_SINT32 GetAorState() { return m_nAorState; }

    ITimer* GetTimer() { return m_piRetryTimer; }

    void RefreshTimerExpiredListener(OUT IMS_BOOL& bDoImplicitRefresh)
    {
        RegSubscription_RefreshTimerExpired(bDoImplicitRefresh);
    }

    void NotifyListenerEvent(IMS_UINT32 nEvent, IMS_SINT32 nReason, IN IMS_BOOL bHasBody)
    {
        switch (nEvent)
        {
            case AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED:
                RegSubscription_NotifyReceived(GetState(), nReason, bHasBody);
                break;

            case AMSG_REG_SUBSCRIPTION_STARTED:
                RegSubscription_Started();
                break;

            case AMSG_REG_SUBSCRIPTION_START_FAILED:
                RegSubscription_StartFailed(nReason);
                break;

            case AMSG_REG_SUBSCRIPTION_UPDATED:
                RegSubscription_Updated();
                break;

            case AMSG_REG_SUBSCRIPTION_UPDATE_FAILED:
                RegSubscription_UpdateFailed(nReason);
                break;

            case AMSG_REG_SUBSCRIPTION_REMOVED:
                RegSubscription_Removed();
                break;

            case AMSG_REG_SUBSCRIPTION_TERMINATED:
                RegSubscription_Terminated(nReason);
                break;

            default:
                break;
        }
    }
};

class AosSubscriptionTest : public ::testing::Test
{
public:
    AosSubscriptionTest()
    {
        m_pAosStaticProfile = new AosStaticProfile();

        // save origin pointer
        m_pOriginAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockAosConfig, SLOT_ID);

        m_pOriginAosService = AosProvider::GetInstance()->GetService(SLOT_ID);
        AosProvider::GetInstance()->SetService(&m_objMockAosService, SLOT_ID);

        m_pOriginAosRetryRepository = AosProvider::GetInstance()->GetRetryRepository(SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(&m_objMockAosRetryRepository, SLOT_ID);

        m_pOriginAosTransaction = AosProvider::GetInstance()->GetTransaction(SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(&m_objMockIAosTransaction, SLOT_ID);
    }

    virtual ~AosSubscriptionTest()
    {
        ConfigurationManager::GetInstance()->DestroyConfigs();

        AosProvider::GetInstance()->SetNConfiguration(m_pOriginAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetService(m_pOriginAosService, SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(m_pOriginAosRetryRepository, SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(m_pOriginAosTransaction, SLOT_ID);

        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }
    }

    TestAosSubscription* m_pAosSubscription;

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

    const AString ADDRESS1 = "sip:1234@ims.google.com:5060";
    const AString ADDRESS2 = "sip:1234@ims.google.com";
    const AString ANONYMOUS_ADDRESS = "sip:anonymous@anonymous.invalid";

protected:
    virtual void SetUp() override
    {
        ConfigurationManager::GetInstance()->Initialize();

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

        m_pAosSubscription =
                new TestAosSubscription(static_cast<IAosAppContext*>(m_pMockAosAppContext),
                        static_cast<IRegSubscription*>(&m_objMockIRegSubscription), *m_pAor,
                        *m_pContactAddress);
        ASSERT_TRUE(m_pAosSubscription != nullptr);

        m_pAosSubscription->SetListener(&m_objMockIAosSubscriptionListener);
        m_pAosSubscription->SetRegSubscription(&m_objMockIRegSubscription);
        ON_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
                .WillByDefault(Return(IMS_TRUE));

        EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_StateChanged(_, _))
                .WillRepeatedly(Return());

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
    }
};

TEST_F(AosSubscriptionTest, Initialize)
{
    EXPECT_CALL(m_objMockIRegSubscription, SetRefreshPolicy(0, 1200, 50, 600)).Times(1);
    EXPECT_CALL(m_objMockIRegSubscription, SetListener(m_pAosSubscription)).Times(1);
    EXPECT_CALL(m_objMockIAosTransaction, SetListener(_, _)).Times(1);

    m_pAosSubscription->Initialize();
}

TEST_F(AosSubscriptionTest, CallUnsubscribeInStopWhenUnsubscriptionIsSupported)
{
    ON_CALL(m_objMockAosConfig, IsUnSubscription()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIRegSubscription, Unsubscribe()).Times(1);

    m_pAosSubscription->Stop();
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_UNSUBSCRIBING);
}

TEST_F(AosSubscriptionTest, NotCallUnsubscribeInStopWhenUnsubscriptionIsNotSupported)
{
    int nState = m_pAosSubscription->GetState();
    ON_CALL(m_objMockAosConfig, IsUnSubscription()).WillByDefault(Return(IMS_FALSE));

    m_pAosSubscription->Stop();
    EXPECT_EQ(m_pAosSubscription->GetState(), nState);
}

TEST_F(AosSubscriptionTest, StateIsInvalidWhenStart)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_UNSUBSCRIBING);
    EXPECT_FALSE(m_pAosSubscription->Start(IMS_FALSE));
}

TEST_F(AosSubscriptionTest, SubscribeFailedInStartWhenRegSubscriptionIsNull)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_OFFLINE);
    m_pAosSubscription->SetRegSubscription(IMS_NULL);

    EXPECT_FALSE(m_pAosSubscription->Start(IMS_FALSE));
}

TEST_F(AosSubscriptionTest, StartReturnFalseWhenSendSubscribeFailed)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBED);
    ON_CALL(m_objMockIRegSubscription, Subscribe()).WillByDefault(Return(IMS_FAILURE));

    EXPECT_FALSE(m_pAosSubscription->Start(IMS_FALSE));
}

TEST_F(AosSubscriptionTest, StartReturnTrueWhenSendSubscribePassed)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBED);
    ON_CALL(m_objMockIRegSubscription, Subscribe()).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_TRUE(m_pAosSubscription->Start(IMS_FALSE));
}

TEST_F(AosSubscriptionTest, StartReturnFalseWhenStartWithCheckRadioIsTrue)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_OFFLINE);

    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(_)).WillOnce(Return(IMS_FALSE));

    EXPECT_FALSE(m_pAosSubscription->Start(IMS_TRUE));
}

TEST_F(AosSubscriptionTest, IsRetryActionDueToRetryCounter)
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

    EXPECT_TRUE(m_pAosSubscription->IsRetryActionDueToRetryCounter(IMS_FALSE));

    m_pAosSubscription->SetTerminated(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED))
            .Times(1);

    EXPECT_TRUE(m_pAosSubscription->IsRetryActionDueToRetryCounter(IMS_TRUE));

    EXPECT_FALSE(m_pAosSubscription->IsRetryActionDueToRetryCounter(IMS_TRUE));
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

    m_pAosSubscription->SetRetryCountSubTerminated(0);
    EXPECT_FALSE(m_pAosSubscription->IsSubscriptionTerminated(403));

    m_pAosSubscription->SetRetryCountSubTerminated(1);
    EXPECT_TRUE(m_pAosSubscription->IsSubscriptionTerminated(403));

    objErrSubTerminated.Clear();
    objErrSubTerminated.Add(5);
    objErrSubTerminated.Add(403);
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorSubTerminated())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrSubTerminated));

    m_pAosSubscription->SetRetryCountSubTerminated(1);
    EXPECT_TRUE(m_pAosSubscription->IsSubscriptionTerminated(504));
}

TEST_F(AosSubscriptionTest, CheckInitialRegRequired)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
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

    m_pAosSubscription->SetRetryCountRegRequired(0);
    EXPECT_FALSE(m_pAosSubscription->IsInitialRegistrationRequired(403, IMS_FALSE));

    m_pAosSubscription->SetRetryCountRegRequired(1);

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

    m_pAosSubscription->SetTerminated(IMS_FALSE);
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

    m_pAosSubscription->SetRetryCountRegRequired(1);
    EXPECT_TRUE(m_pAosSubscription->IsInitialRegistrationRequired(504, IMS_TRUE));
}

TEST_F(AosSubscriptionTest, CheckInitialRegWithNextPcscfRequired)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
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

    m_pAosSubscription->SetTerminated(IMS_FALSE);

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
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
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

    m_pAosSubscription->SetTerminated(IMS_FALSE);
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
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
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
    // IsRetryActionDueToRetryCounter() - 1st: true, others: false
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
    m_pAosSubscription->SetRetryCountSubTerminated(0);

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

    m_pAosSubscription->SetRetryCountSubTerminated(0);
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

    m_pAosSubscription->SetRetryCountRegRequired(2);

    EXPECT_CALL(m_objMockAosConfig, GetRegRetryCountResetPolicy()).WillRepeatedly(Return(0));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));

    // IsInitialRegistrationWithNextPcscfRequired() - 1st: true, others: false
    ImsVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Clear();
    objErrRegRequiredWithNextPcscf.Add(403);
    EXPECT_CALL(m_objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .WillOnce(ReturnRef(objErrRegRequiredWithNextPcscf));

    m_pAosSubscription->SetTerminated(IMS_FALSE);
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

    // nStatusCode == SipStatusCode::SC_503 No Retry-After;
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHING);
    m_pAosSubscription->SetTerminated(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(
                    AosSubscription::CMD_REG_REQUIRED_WITH_AVAILABLE_NEXT_PCSCF, 0, IMS_FALSE))
            .Times(2);

    EXPECT_CALL(objMockSipMsg, GetStatusCode()).WillRepeatedly(Return(503));

    AString strRetryAfter = AString::ConstNull();
    EXPECT_CALL(objMockSipMsg, GetHeader(ISipHeader::RETRY_AFTER_SEC, 0, AString::ConstNull()))
            .WillOnce(Return(strRetryAfter));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(503, IMS_TRUE));

    // nStatusCode == SipStatusCode::SC_503 Retry-After > TimerF(default 128);
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHING);
    m_pAosSubscription->SetTerminated(IMS_FALSE);
    strRetryAfter = AString("600");
    EXPECT_CALL(objMockSipMsg, GetHeader(ISipHeader::RETRY_AFTER_SEC, 0, AString::ConstNull()))
            .WillOnce(Return(strRetryAfter));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(503, IMS_TRUE));

    // nStatusCode == SipStatusCode::SC_503 Retry-After <= TimerF(default 128);
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHING);
    m_pAosSubscription->SetTerminated(IMS_FALSE);
    strRetryAfter = AString("60");
    EXPECT_CALL(objMockSipMsg, GetHeader(ISipHeader::RETRY_AFTER_SEC, 0, AString::ConstNull()))
            .WillOnce(Return(strRetryAfter));

    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(503, IMS_TRUE));

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

    m_pAosSubscription->SetState(AosSubscription::STATE_OFFLINE);
    EXPECT_FALSE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_SUB_403));

    m_pAosSubscription->SetState(AosSubscription::STATE_OFFLINE);
    EXPECT_FALSE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_SUB_403));

    m_pAosSubscription->SetState(AosSubscription::STATE_UNSUBSCRIBING);
    EXPECT_FALSE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_SUB_403));

    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBED);
    EXPECT_TRUE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_SUB_403));

    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBED);
    EXPECT_TRUE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_NOTIFY_TERMINATED));
}

TEST_F(AosSubscriptionTest, CheckNotifyReceived)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBED);
    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_FALSE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_TERMINATED);

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

    EXPECT_CALL(m_objMockAosConfig, GetUsatRegEventDownloadPolicy())
            .Times(4)
            .WillOnce(Return(CarrierConfig::Assets::USAT_REG_EVENT_NOT_DOWNLOAD))
            .WillOnce(Return(CarrierConfig::Assets::USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD))
            .WillOnce(Return(CarrierConfig::Assets::USAT_REG_EVENT_NOT_DOWNLOAD))
            .WillOnce(Return(CarrierConfig::Assets::USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD));

    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_TERMINATED);

    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_TERMINATED);

    ImsList<IRegInfoContact*> objContact;
    objContact.Clear();

    EXPECT_CALL(objMockIRegInfoRegistration, GetContacts()).Times(1).WillOnce(Return(objContact));
    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_TERMINATED);

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
    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_TERMINATED);

    EXPECT_CALL(m_objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount())
            .WillOnce(Return(5))
            .WillRepeatedly(Return(0));

    ImsList<AString> objImpus;
    EXPECT_CALL(m_objMockAosService, NotifyRegEventState(200, objImpus)).Times(1);
    EXPECT_CALL(m_objMockAosRetryRepository, ResetRetryCount(0)).Times(1);

    SipAddress objContactAddr;
    objContactAddr.Create(ADDRESS2);
    m_pAosSubscription->SetContactAddress(objContactAddr);
    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_ACTIVE);

    objContactAddr.RemoveAllParameters();
    objContactAddr.Create(ADDRESS1);
    m_pAosSubscription->SetContactAddress(objContactAddr);

    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_ACTIVE);

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
    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_TERMINATED);

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

    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_TERMINATED);

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

    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_TERMINATED);

    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_TERMINATED);

    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_TERMINATED);

    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_TERMINATED);

    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_TERMINATED);
}

TEST_F(AosSubscriptionTest, RegSubscription_RefreshTimerExpired)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;

    // CheckRadioReadyAndSetRadioWaiting() - IMS_TRUE
    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pAosSubscription->RefreshTimerExpiredListener(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHING);
    EXPECT_EQ(bDoImplicitRefresh, IMS_TRUE);

    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_FALSE));
    m_pAosSubscription->RefreshTimerExpiredListener(bDoImplicitRefresh);
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

    m_pAosSubscription->RefreshTimerExpiredListener(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);

    EXPECT_CALL(m_objMockIAosTransaction, StartTraffic(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosSubscription->RefreshTimerExpiredListener(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);

    // for checking SetRadioWaiting(IMS_TRUE)
    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillOnce(Return(IMS_FALSE));
    // IsRadioWaiting()
    m_pAosSubscription->Transaction_OnConnectionSetupPrepared();
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionStarted)
{
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_SUBSCRIBED, AosSubscription::REASON_SUB_ESTABLISHED))
            .Times(1);

    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_STARTED, 0, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBED);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionUpdated)
{
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_SUBSCRIBED, AosSubscription::REASON_SUB_ESTABLISHED))
            .Times(1);

    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_UPDATED, 0, 0);
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

    m_pAosSubscription->SetThrottlingCount(0);
    ImsVector<IMS_SINT32> objRetryRandomIntervals;
    objRetryRandomIntervals.Clear();
    objRetryRandomIntervals.Add(0);
    objRetryRandomIntervals.Add(5);
    objRetryRandomIntervals.Add(0);
    EXPECT_CALL(m_objMockAosConfig, GetRegRandomRetryIntervals())
            .WillOnce(ReturnRef(objRetryRandomIntervals))
            .WillOnce(ReturnRef(objRetryRandomIntervals));

    // ProcessStartFailed_Others - m_pAosSubscription->SetRetryTimer(IMS_FALSE);
    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_INTERNAL_ERROR, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    m_pAosSubscription->SetThrottlingCount(3);
    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_INTERNAL_ERROR, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionStartFailed_StatusCode_Done)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBING);
    MockISipMessage objMockSipMsg;
    EXPECT_CALL(objMockSipMsg, GetStatusCode()).WillRepeatedly(Return(403));

    EXPECT_CALL(m_objMockIRegSubscription, GetPreviousResponse())
            .WillRepeatedly(Return(static_cast<ISipMessage*>(&objMockSipMsg)));

    // IsRetryActionDueToRetryCounter() - true;
    EXPECT_CALL(m_objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount()).WillOnce(Return(5));

    // AosRetryRepository::TYPE_NORMAL = 0
    EXPECT_CALL(m_objMockAosRetryRepository, IncreaseRetryCount(0)).WillOnce(Return(IMS_FALSE));

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionStartFailed_StatusCode)
{
    // ProcessStartFailed_StatusCode - m_pAosSubscription->SetRetryTimer(IMS_FALSE);
    // IsRetryActionDueToRetryCounter - FALSE
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

    m_pAosSubscription->SetThrottlingCount(0);
    EXPECT_CALL(m_objMockAosConfig, IsRegRetryIntervalsUsedForSub())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockAosConfig, GetRegistrationRetryBaseTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(30000));
    EXPECT_CALL(m_objMockAosConfig, GetRegistrationRetryMaxTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1800000));

    m_pAosSubscription->NotifyListenerEvent(
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

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    m_pAosSubscription->NotifyListenerEvent(
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
    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_UPDATE_FAILED,
            IRegSubscription::REASON_REFRESH_INTERNAL_ERROR, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionUpdateFailed_StatusCode_Done)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHING);
    MockISipMessage objMockSipMsg;
    EXPECT_CALL(objMockSipMsg, GetStatusCode()).WillRepeatedly(Return(403));

    EXPECT_CALL(m_objMockIRegSubscription, GetPreviousResponse())
            .WillRepeatedly(Return(static_cast<ISipMessage*>(&objMockSipMsg)));

    // IsRetryActionDueToRetryCounter() - true;
    EXPECT_CALL(m_objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount()).WillOnce(Return(5));

    // AosRetryRepository::TYPE_NORMAL = 0
    EXPECT_CALL(m_objMockAosRetryRepository, IncreaseRetryCount(0)).WillOnce(Return(IMS_FALSE));

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionUpdateFailed_StatusCode)
{
    // IsRetryActionDueToRetryCounter - FALSE
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

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_TRANSACTION_TIMEOUT, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_REFRESH_TIMEOUT, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);

    EXPECT_CALL(m_objMockIRegSubscription, GetPreviousResponse()).WillRepeatedly(ReturnNull());
    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);
}

TEST_F(AosSubscriptionTest, RegSubscription_End)
{
    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_REMOVED, IRegSubscription::REASON_NONE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_TERMINATED, IRegSubscription::REASON_NO_EXPIRES, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

TEST_F(AosSubscriptionTest, ProcessTimerExpired_StateInvalid)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBED);
    m_pAosSubscription->StartTimer(30);
    m_pAosSubscription->Timer_TimerExpired(m_pAosSubscription->GetTimer());

    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBED);
}

TEST_F(AosSubscriptionTest, ProcessTimerExpired)
{
    IMS_UINT32 nCurrState = m_pAosSubscription->GetState();
    m_pAosSubscription->Timer_TimerExpired(IMS_NULL);
    EXPECT_EQ(m_pAosSubscription->GetState(), nCurrState);

    // Timer_TimerExpired (piTimer != m_piRetryTimer) to do

    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_FALSE));

    // ProcessTimerExpired() - return;
    m_pAosSubscription->SetState(AosSubscription::STATE_OFFLINE);
    m_pAosSubscription->StartTimer(30);
    m_pAosSubscription->Timer_TimerExpired(m_pAosSubscription->GetTimer());
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);

    // ProcessTimerExpired() - Subscription_CanBeTransmitted() == IMS_FALSE
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);

    m_pAosSubscription->StartTimer(30);
    m_pAosSubscription->Timer_TimerExpired(m_pAosSubscription->GetTimer());
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHSTOP);
    m_pAosSubscription->StartTimer(30);
    m_pAosSubscription->Timer_TimerExpired(m_pAosSubscription->GetTimer());
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);

    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_TRUE));

    // ProcessTimerExpired() - Subscription_CanBeTransmitted() == IMS_TRUE
    // SendSubscribe() == IMS_FALSE m_piRegSubscription == IMS_NULL
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
    m_pAosSubscription->StartTimer(30);
    m_pAosSubscription->Timer_TimerExpired(m_pAosSubscription->GetTimer());

    m_pAosSubscription->SetRegSubscription(IMS_NULL);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBING);

    // ProcessTimerExpired() - SendSubscribe() = IMS_TRUE Subscribe() == IMS_SUCCESS
    m_pAosSubscription->SetRegSubscription(
            static_cast<IRegSubscription*>(&m_objMockIRegSubscription));
    EXPECT_CALL(m_objMockIRegSubscription, Subscribe())
            .WillOnce(Return(IMS_FAILURE))
            .WillRepeatedly(Return(IMS_SUCCESS));
    m_pAosSubscription->Timer_TimerExpired(m_pAosSubscription->GetTimer());
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBING);

    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
    m_pAosSubscription->StartTimer(30);
    m_pAosSubscription->Timer_TimerExpired(m_pAosSubscription->GetTimer());

    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHSTOP);
    m_pAosSubscription->StartTimer(30);
    m_pAosSubscription->Timer_TimerExpired(m_pAosSubscription->GetTimer());
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHING);
}

TEST_F(AosSubscriptionTest, TransOnConnectionFailed)
{
    m_pAosSubscription->SetRadioWaiting(IMS_FALSE);
    m_pAosSubscription->Transaction_OnConnectionFailed(IImsRadio::REASON_NO_SERVICE, 0, 0);
    EXPECT_FALSE(m_pAosSubscription->IsRadioWaiting());

    m_pAosSubscription->SetRadioWaiting(IMS_TRUE);
    m_pAosSubscription->Transaction_OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 0, 0);
    EXPECT_FALSE(m_pAosSubscription->IsRadioWaiting());

    m_pAosSubscription->SetRadioWaiting(IMS_TRUE);
    m_pAosSubscription->Transaction_OnConnectionFailed(IImsRadio::REASON_RRC_TIMEOUT, 0, 15000);
    EXPECT_FALSE(m_pAosSubscription->IsRadioWaiting());
}

TEST_F(AosSubscriptionTest, TransOnConnectionFailed_CallPrepared)
{
    m_pAosSubscription->SetRadioWaiting(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(_))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE));

    // call Transaction_OnConnectionSetupPrepared()
    m_pAosSubscription->Transaction_OnConnectionFailed(IImsRadio::REASON_NO_SERVICE, 0, 0);
    EXPECT_FALSE(m_pAosSubscription->IsRadioWaiting());
}

TEST_F(AosSubscriptionTest, TransOnTrafficPriorityChanged)
{
    // called Start() - return IMS_FALSE
    m_pAosSubscription->SetTrafficPriorityBlocked(IMS_TRUE);
    m_pAosSubscription->SetState(AosSubscription::STATE_OFFLINE);
    EXPECT_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillOnce(Return(IMS_FALSE));

    m_pAosSubscription->Transaction_OnTrafficPriorityChanged();
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

TEST_F(AosSubscriptionTest, Print)
{
    int nState = AosSubscription::STATE_OFFLINE;
    while (nState <= AosSubscription::STATE_UNSUBSCRIBING)
    {
        m_pAosSubscription->SetState(nState);
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

    m_pAosSubscription->SetState(AosSubscription::STATE_OFFLINE);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}