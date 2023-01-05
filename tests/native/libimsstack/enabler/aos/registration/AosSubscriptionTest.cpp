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
    AosSubscription* pAosSubscription;

    AosStaticProfile* pAosStaticProfile;
    MockAosAppContext* pMockAosAppContext;
    MockIAosNetTracker m_objMockAosINetTracker;
    MockIRegSubscription objMockIRegSubscription;
    AString* pAor;
    SipAddress* pContactAddress;

    MockIAosSubscriptionListener objMockIAosSubscriptionListener;

    // for AosNConfig info
    IAosNConfiguration* pOriginAosNConfiguration;
    MockIAosNConfiguration objMockAosConfig;

    IAosService* pOriginAosService;
    MockIAosService objMockAosService;

    IAosRetryRepository* pOriginAosRetryRepository;
    MockIAosRetryRepository objMockAosRetryRepository;

    IAosTransaction* pOriginAosTransaction;
    MockIAosTransaction objMockIAosTransaction;

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
        pAosStaticProfile = new AosStaticProfile();
        pMockAosAppContext = new MockAosAppContext(pAosStaticProfile);

        pAor = new AString(ADDRESS1);
        pContactAddress = new SipAddress();

        EXPECT_CALL(*pMockAosAppContext, GetSlotId()).WillRepeatedly(Return(SLOT_ID));

        EXPECT_CALL(*pMockAosAppContext, GetNetTracker())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockAosINetTracker));
        EXPECT_CALL(m_objMockAosINetTracker, GetNetworkType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(static_cast<IMS_UINT32>(AosNetworkType::LTE)));

        pAosSubscription = new AosSubscription(static_cast<IAosAppContext*>(pMockAosAppContext),
                static_cast<IRegSubscription*>(&objMockIRegSubscription), *pAor, *pContactAddress);
        ASSERT_TRUE(pAosSubscription != nullptr);

        pAosSubscription->SetListener(&objMockIAosSubscriptionListener);
        EXPECT_CALL(objMockIAosSubscriptionListener, Subscription_StateChanged(_, _))
                .WillRepeatedly(Return());

        EXPECT_CALL(objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
                .WillRepeatedly(Return(IMS_TRUE));

        // save origin pointer
        pOriginAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&objMockAosConfig), SLOT_ID);

        pOriginAosService = AosProvider::GetInstance()->GetService(SLOT_ID);
        AosProvider::GetInstance()->SetService(
                static_cast<IAosService*>(&objMockAosService), SLOT_ID);

        pOriginAosRetryRepository = AosProvider::GetInstance()->GetRetryRepository(SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(
                static_cast<IAosRetryRepository*>(&objMockAosRetryRepository), SLOT_ID);

        pOriginAosTransaction = AosProvider::GetInstance()->GetTransaction(SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(
                static_cast<IAosTransaction*>(&objMockIAosTransaction), SLOT_ID);

        EXPECT_CALL(objMockIAosTransaction, SetListener(_, _)).Times(AnyNumber());
        EXPECT_CALL(objMockIAosTransaction, RemoveListener(_, _)).Times(AnyNumber());
        EXPECT_CALL(objMockIAosTransaction, IsTransactionAllowed(_))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));
        EXPECT_CALL(objMockIAosTransaction, StartTraffic(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));
        EXPECT_CALL(objMockIAosTransaction, StopTraffic(_)).Times(AnyNumber());
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(pOriginAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetService(pOriginAosService, SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(pOriginAosRetryRepository, SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(pOriginAosTransaction, SLOT_ID);

        if (pAor)
        {
            delete pAor;
        }

        if (pContactAddress)
        {
            delete pContactAddress;
        }

        if (pMockAosAppContext)
        {
            delete pMockAosAppContext;
        }

        if (pAosStaticProfile)
        {
            delete pAosStaticProfile;
        }

        if (pAosSubscription)
        {
            pAosSubscription->SetListener(IMS_NULL);
            pAosSubscription->Destroy();
        }
    }

    void SetState(IN IMS_SINT32 nState) { pAosSubscription->m_nState = nState; }

    void SetpiRegSubscription(IN IRegSubscription* piRegSubscription)
    {
        pAosSubscription->m_piRegSubscription = piRegSubscription;
    }

    void SetRetryCountSubTerminated(IN IMS_UINT32 nRetryCount)
    {
        pAosSubscription->m_nRetryCountSubTerminated = nRetryCount;
    }

    void SetRetryCountRegRequired(IN IMS_UINT32 nRetryCount)
    {
        pAosSubscription->m_nRetryCountRegRequired = nRetryCount;
    }

    void SetThrottlingCount(IN IMS_UINT32 nThrottlingCount)
    {
        pAosSubscription->m_nThrottlingCount = nThrottlingCount;
    }

    void SetObjContactAddress(IN const SipAddress& objContactAddress)
    {
        pAosSubscription->m_objContactAddress = objContactAddress;
    }

    IMS_SINT32 GetAorState() { return pAosSubscription->m_nAorState; }

    void SetTerminated(IN IMS_BOOL bTerminated) { pAosSubscription->SetTerminated(bTerminated); }

    void StartTimer(IN IMS_UINT32 nDuration) { pAosSubscription->StartTimer(nDuration); }

    ITimer* GetTimer() { return pAosSubscription->m_piRetryTimer; }

    void Timer_TimerExpired(IN ITimer* piTimer) { pAosSubscription->Timer_TimerExpired(piTimer); }

    void NotifyListenerEvent(IMS_UINT32 nEvent, IMS_SINT32 nReason, IN IMS_BOOL bHasBody)
    {
        SetpiRegSubscription(&objMockIRegSubscription);

        switch (nEvent)
        {
            case AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED:
                pAosSubscription->RegSubscription_NotifyReceived(
                        pAosSubscription->m_nState, nReason, bHasBody);
                break;

            case AMSG_REG_SUBSCRIPTION_STARTED:
                pAosSubscription->RegSubscription_Started();
                break;

            case AMSG_REG_SUBSCRIPTION_START_FAILED:
                pAosSubscription->RegSubscription_StartFailed(nReason);
                break;

            case AMSG_REG_SUBSCRIPTION_UPDATED:
                pAosSubscription->RegSubscription_Updated();
                break;

            case AMSG_REG_SUBSCRIPTION_UPDATE_FAILED:
                pAosSubscription->RegSubscription_UpdateFailed(nReason);
                break;

            case AMSG_REG_SUBSCRIPTION_REMOVED:
                pAosSubscription->RegSubscription_Removed();
                break;

            case AMSG_REG_SUBSCRIPTION_TERMINATED:
                pAosSubscription->RegSubscription_Terminated(nReason);
                break;

            default:
                break;
        }
    }

    void RefreshTimerExpiredListener(OUT IMS_BOOL& bDoImplicitRefresh)
    {
        pAosSubscription->RegSubscription_RefreshTimerExpired(bDoImplicitRefresh);
    }
};

TEST_F(AosSubscriptionTest, Initialize)
{
    EXPECT_CALL(objMockIRegSubscription, SetRefreshPolicy(0, 1200, 50, 600)).Times(1);
    EXPECT_CALL(objMockIRegSubscription, SetListener(pAosSubscription)).Times(1);

    pAosSubscription->Initialize();
}

TEST_F(AosSubscriptionTest, Stop)
{
    EXPECT_CALL(objMockAosConfig, IsUnSubscription())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));

    pAosSubscription->Stop();

    EXPECT_CALL(objMockIRegSubscription, Unsubscribe()).Times(1);

    pAosSubscription->Stop();
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_UNSUBSCRIBING);
}

TEST_F(AosSubscriptionTest, AosSubscriptionStart)
{
    SetState(AosSubscription::STATE_UNSUBSCRIBING);
    EXPECT_FALSE(pAosSubscription->Start(IMS_FALSE));

    SetState(AosSubscription::STATE_OFFLINE);

    SetpiRegSubscription(IMS_NULL);
    EXPECT_FALSE(pAosSubscription->Start(IMS_FALSE));

    SetState(AosSubscription::STATE_SUBSCRIBED);
    SetpiRegSubscription(static_cast<IRegSubscription*>(&objMockIRegSubscription));
    EXPECT_CALL(objMockIRegSubscription, Subscribe())
            .WillOnce(Return(IMS_FAILURE))
            .WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_FALSE(pAosSubscription->Start(IMS_FALSE));
    EXPECT_TRUE(pAosSubscription->Start(IMS_FALSE));
}

TEST_F(AosSubscriptionTest, IsRetryActionDueToRetrycounter)
{
    EXPECT_CALL(objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(objMockAosConfig, GetExtraRegErrMaxCount())
            .WillOnce(Return(5))
            .WillOnce(Return(3))
            .WillOnce(Return(0));

    // AosRetryRepository::TYPE_NORMAL = 0
    EXPECT_CALL(objMockAosRetryRepository, IncreaseRetryCount(0))
            .Times(2)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED))
            .Times(1);
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::COMMAND_REG_REQUIRED_WITH_NEXT_PCSCF, 0))
            .Times(2);

    EXPECT_TRUE(pAosSubscription->IsRetryActionDueToRetrycounter(IMS_FALSE));

    SetTerminated(IMS_FALSE);
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED))
            .Times(1);

    EXPECT_TRUE(pAosSubscription->IsRetryActionDueToRetrycounter(IMS_TRUE));

    EXPECT_FALSE(pAosSubscription->IsRetryActionDueToRetrycounter(IMS_TRUE));
}

TEST_F(AosSubscriptionTest, CheckSubscriptionTerminated)
{
    EXPECT_CALL(objMockAosConfig, GetRetryCountSubErrorSubTerminated())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(2));

    EXPECT_FALSE(pAosSubscription->IsSubscriptionTerminated(403));

    IMSVector<IMS_SINT32> objErrSubTerminated;
    objErrSubTerminated.Clear();
    objErrSubTerminated.Add(404);
    objErrSubTerminated.Add(403);

    EXPECT_CALL(objMockAosConfig, GetSubErrorSubTerminated())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrSubTerminated));

    SetRetryCountSubTerminated(0);
    EXPECT_FALSE(pAosSubscription->IsSubscriptionTerminated(403));

    SetRetryCountSubTerminated(1);
    EXPECT_TRUE(pAosSubscription->IsSubscriptionTerminated(403));

    objErrSubTerminated.Clear();
    objErrSubTerminated.Add(5);
    objErrSubTerminated.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorSubTerminated())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrSubTerminated));

    SetRetryCountSubTerminated(1);
    EXPECT_TRUE(pAosSubscription->IsSubscriptionTerminated(504));
}

TEST_F(AosSubscriptionTest, CheckInitialRegRequired)
{
    SetState(AosSubscription::STATE_SUBSTOP);
    EXPECT_CALL(objMockAosConfig, GetRetryCountSubErrorRegRequired())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(2));

    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequired(403, IMS_FALSE));

    IMSVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Clear();
    objErrRegRequired.Add(404);
    objErrRegRequired.Add(403);

    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequired));

    SetRetryCountRegRequired(0);
    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequired(403, IMS_FALSE));

    SetRetryCountRegRequired(1);

    EXPECT_CALL(objMockAosConfig, GetRegRetryCountResetPolicy())
            .WillOnce(Return(0))
            .WillOnce(Return(1));

    // ReportState() if result is true.
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED))
            .Times(1);

    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::COMMAND_REG_REQUIRED, 0))
            .Times(1);
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::COMMAND_REG_REQUIRED_WITH_REG_RETRY_TIME, 0))
            .Times(1);

    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationRequired(403, IMS_FALSE));

    SetTerminated(IMS_FALSE);
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED))
            .Times(1);

    objErrRegRequired.Clear();
    objErrRegRequired.Add(5);
    objErrRegRequired.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequired));

    SetRetryCountRegRequired(1);
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationRequired(504, IMS_TRUE));
}

TEST_F(AosSubscriptionTest, CheckInitialRegWithNextPcscfRequired)
{
    SetState(AosSubscription::STATE_SUBSTOP);
    IMSVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Clear();
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));

    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(403, IMS_FALSE));

    objErrRegRequiredWithNextPcscf.Add(404);
    objErrRegRequiredWithNextPcscf.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));

    // ReportState() if result is true.
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED))
            .Times(1);
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::COMMAND_REG_REQUIRED_WITH_NEXT_PCSCF, 0))
            .Times(2);
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(403, IMS_FALSE));

    SetTerminated(IMS_FALSE);

    objErrRegRequiredWithNextPcscf.Clear();
    objErrRegRequiredWithNextPcscf.Add(6);
    objErrRegRequiredWithNextPcscf.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(603, IMS_TRUE));
}

TEST_F(AosSubscriptionTest, CheckInitialRegRequiredInWifi)
{
    SetState(AosSubscription::STATE_SUBSTOP);
    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(*pMockAosAppContext, GetConnection())
            .Times(6)
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(6)
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequiredInWifi(403, IMS_FALSE));

    // GetVowifiSubErrorRegRequired() - no info
    IMSVector<IMS_SINT32> objErrRegRequiredInWifi;
    objErrRegRequiredInWifi.Clear();
    EXPECT_CALL(objMockAosConfig, GetVowifiSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredInWifi));

    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequiredInWifi(403, IMS_FALSE));

    // GetVowifiSubErrorRegRequired() - 404, 403
    objErrRegRequiredInWifi.Add(404);
    objErrRegRequiredInWifi.Add(403);
    EXPECT_CALL(objMockAosConfig, GetVowifiSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredInWifi));

    // bErrCodeByNo911Addr
    IMSVector<IMS_SINT32> objErrCodeByNo911Addr;
    objErrCodeByNo911Addr.Clear();
    objErrCodeByNo911Addr.Add(2);
    EXPECT_CALL(objMockAosConfig, GetWfcSubErrorByMissing911Address())
            .Times(1)
            .WillOnce(ReturnRef(objErrCodeByNo911Addr));

    // RequestCommand
    EXPECT_CALL(objMockAosConfig, GetRegRetryCountResetPolicy())
            .WillOnce(Return(2))
            .WillOnce(Return(0));

    // ReportState() if result is true.
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED))
            .Times(1);

    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_Request(
                    AosSubscription::COMMAND_REG_REQUIRED_WITH_NOTI_NO_911_ADDR_WITH_REG_RETRY_TIME,
                    0))
            .Times(1);

    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationRequiredInWifi(403, IMS_FALSE));
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);

    SetTerminated(IMS_FALSE);
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED))
            .Times(1);
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::COMMAND_REG_REQUIRED, 0))
            .Times(1);

    objErrRegRequiredInWifi.Clear();
    objErrRegRequiredInWifi.Add(3);
    objErrRegRequiredInWifi.Add(403);
    EXPECT_CALL(objMockAosConfig, GetVowifiSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredInWifi));
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationRequiredInWifi(301, IMS_TRUE));
}

TEST_F(AosSubscriptionTest, CheckIsReSubscriptionStopped)
{
    SetState(AosSubscription::STATE_SUBSTOP);
    IMSVector<IMS_SINT32> objErrResubStopped;
    objErrResubStopped.Clear();
    EXPECT_CALL(objMockAosConfig, GetSubErrorStoppingResub())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrResubStopped));

    EXPECT_FALSE(pAosSubscription->IsResubscriptionStopped(403));

    objErrResubStopped.Add(404);
    objErrResubStopped.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorStoppingResub())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrResubStopped));
    EXPECT_TRUE(pAosSubscription->IsResubscriptionStopped(403));

    objErrResubStopped.Clear();
    objErrResubStopped.Add(403);
    objErrResubStopped.Add(5);
    EXPECT_CALL(objMockAosConfig, GetSubErrorStoppingResub())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrResubStopped));
    EXPECT_TRUE(pAosSubscription->IsResubscriptionStopped(503));
}

TEST_F(AosSubscriptionTest, ProcessFailedStatusCode)
{
    // IsRetryActionDueToRetrycounter() - 1st: true, others: false
    EXPECT_CALL(objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockAosConfig, GetExtraRegErrMaxCount())
            .WillOnce(Return(5))
            .WillRepeatedly(Return(0));

    // AosRetryRepository::TYPE_NORMAL = 0
    EXPECT_CALL(objMockAosRetryRepository, IncreaseRetryCount(0)).WillOnce(Return(IMS_FALSE));

    EXPECT_TRUE(pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));

    // IsSubscriptionTerminated() - 1st: true, others: false
    EXPECT_CALL(objMockAosConfig, GetRetryCountSubErrorSubTerminated())
            .WillOnce(Return(1))
            .WillOnce(Return(2))
            .WillRepeatedly(Return(0));

    IMSVector<IMS_SINT32> objErrSubTerminated;
    objErrSubTerminated.Clear();
    objErrSubTerminated.Add(404);
    EXPECT_CALL(objMockAosConfig, GetSubErrorSubTerminated())
            .WillOnce(ReturnRef(objErrSubTerminated))
            .WillOnce(ReturnRef(objErrSubTerminated));
    SetRetryCountSubTerminated(0);

    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED))
            .Times(AnyNumber());
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::COMMAND_SUB_TERMINATED, 0))
            .Times(AnyNumber());
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::COMMAND_REG_REQUIRED, 0))
            .Times(AnyNumber());

    EXPECT_TRUE(pAosSubscription->ProcessFailed_StatusCode(404, IMS_FALSE));

    SetRetryCountSubTerminated(0);
    EXPECT_TRUE(pAosSubscription->ProcessFailed_StatusCode(404, IMS_TRUE));

    // IsInitialRegistrationRequired() - 1st: true, others: false
    EXPECT_CALL(objMockAosConfig, GetRetryCountSubErrorRegRequired())
            .WillOnce(Return(3))
            .WillRepeatedly(Return(0));

    IMSVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Clear();
    objErrRegRequired.Add(403);

    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequired()).WillOnce(ReturnRef(objErrRegRequired));

    SetRetryCountRegRequired(2);

    EXPECT_CALL(objMockAosConfig, GetRegRetryCountResetPolicy()).WillRepeatedly(Return(0));

    EXPECT_TRUE(pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));

    // IsInitialRegistrationWithNextPcscfRequired() - 1st: true, others: false
    IMSVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Clear();
    objErrRegRequiredWithNextPcscf.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .WillOnce(ReturnRef(objErrRegRequiredWithNextPcscf));

    SetTerminated(IMS_FALSE);
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::COMMAND_REG_REQUIRED_WITH_NEXT_PCSCF, 0))
            .Times(1);
    EXPECT_TRUE(pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));

    // IMSVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Clear();
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));

    // nStatusCode == SipStatusCode::SC_423 refresh: IMS_FALSE;
    MockISipMessage objMockSipMsg;

    EXPECT_CALL(objMockIRegSubscription, GetPreviousResponse())
            .WillRepeatedly(Return(static_cast<ISipMessage*>(&objMockSipMsg)));

    AString strMinTime = "600";
    AString strMinTime2 = "";
    EXPECT_CALL(objMockSipMsg, GetHeader(ISipHeader::MIN_EXPIRES, 0, AString::ConstNull()))
            .WillOnce(Return(strMinTime))
            .WillOnce(Return(strMinTime))
            .WillOnce(Return(strMinTime))
            .WillOnce(Return(strMinTime2));

    EXPECT_CALL(objMockIRegSubscription, SetExpires(600)).Times(3);

    EXPECT_CALL(objMockIRegSubscription, Subscribe())
            .WillOnce(Return(IMS_SUCCESS))
            .WillOnce(Return(IMS_SUCCESS))
            .WillOnce(Return(IMS_FAILURE));

    EXPECT_TRUE(pAosSubscription->ProcessFailed_StatusCode(423, IMS_FALSE));

    // nStatusCode == SipStatusCode::SC_423 refresh: IMS_TRUE;
    EXPECT_TRUE(pAosSubscription->ProcessFailed_StatusCode(423, IMS_TRUE));

    // nStatusCode == SipStatusCode::SC_423 sendSubscribe() : FALSE;
    EXPECT_FALSE(pAosSubscription->ProcessFailed_StatusCode(423, IMS_FALSE));

    // nStatusCode == SipStatusCode::SC_423 nMinTime : 0;
    EXPECT_FALSE(pAosSubscription->ProcessFailed_StatusCode(423, IMS_FALSE));

    // IsInitialRegistrationRequiredInWifi() - 1st: true, others: false
    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(*pMockAosAppContext, GetConnection())
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    IMSVector<IMS_SINT32> objErrRegRequiredInWifi;
    objErrRegRequiredInWifi.Clear();
    objErrRegRequiredInWifi.Add(403);
    EXPECT_CALL(objMockAosConfig, GetVowifiSubErrorRegRequired())
            .WillOnce(ReturnRef(objErrRegRequiredInWifi));

    EXPECT_TRUE(pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));

    // nStatusCode == SipStatusCode::SC_504;
    IMSList<ISipMessageBodyPart*> objBodyParts;
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

    EXPECT_CALL(objMockAosConfig, GetRegRetryCountResetPolicy())
            .WillOnce(Return(0))
            .WillOnce(Return(0));
    EXPECT_TRUE(pAosSubscription->ProcessFailed_StatusCode(504, IMS_FALSE));

    EXPECT_TRUE(pAosSubscription->ProcessFailed_StatusCode(504, IMS_TRUE));

    // nStatusCode == SipStatusCode::SC_504 GetBodyParts empty;
    objBodyParts.Clear();
    EXPECT_CALL(objMockSipMsg, GetBodyParts()).WillOnce(Return(objBodyParts));
    EXPECT_FALSE(pAosSubscription->ProcessFailed_StatusCode(504, IMS_FALSE));

    // nStatusCode == SipStatusCode::SC_504 SipMsg null;
    EXPECT_CALL(objMockIRegSubscription, GetPreviousResponse())
            .WillRepeatedly(Return(static_cast<ISipMessage*>(IMS_NULL)));
    EXPECT_FALSE(pAosSubscription->ProcessFailed_StatusCode(504, IMS_FALSE));

    // bIsRefreshed IMS_TRUE nStatusCode == SipStatusCode::SC_481
    EXPECT_TRUE(pAosSubscription->ProcessFailed_StatusCode(481, IMS_TRUE));

    // bIsRefreshed IMS_TRUE IsResubscriptionStopped
    IMSVector<IMS_SINT32> objErrResubStopped;
    objErrResubStopped.Clear();
    objErrResubStopped.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorStoppingResub())
            .WillOnce(ReturnRef(objErrResubStopped));
    EXPECT_TRUE(pAosSubscription->ProcessFailed_StatusCode(403, IMS_TRUE));

    objErrResubStopped.Clear();
    EXPECT_CALL(objMockAosConfig, GetSubErrorStoppingResub())
            .WillRepeatedly(ReturnRef(objErrResubStopped));

    EXPECT_FALSE(pAosSubscription->ProcessFailed_StatusCode(403, IMS_TRUE));
}

TEST_F(AosSubscriptionTest, IsRegAfterWaitRequiredByNotify)
{
    IMS_UINT32 nFeature = 0;
    nFeature |= 0x01;

    // NOTIFY_TERMINATED_EXPIRED = 0x01,
    // NOTIFY_TERMINATED_DEACTIVATED = 0x02,
    EXPECT_CALL(objMockAosConfig, GetNotifyEventForInitialRegWithWaitTime())
            .Times(2)
            .WillOnce(Return(0x02))
            .WillOnce(Return(0x01));
    EXPECT_FALSE(pAosSubscription->IsRegAfterWaitRequiredByNotify(nFeature));

    EXPECT_TRUE(pAosSubscription->IsRegAfterWaitRequiredByNotify(nFeature));
}

TEST_F(AosSubscriptionTest, IsWfcErrorCodeByMissing911Address)
{
    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(*pMockAosAppContext, GetConnection())
            .Times(5)
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(5)
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));

    SetState(AosSubscription::STATE_OFFLINE);
    EXPECT_FALSE(pAosSubscription->IsWfcErrorCodeByMissing911Address(
            CarrierConfig::Assets::WFC_NO_ADDRESSS_ERROR_CODE_SUBSCRIPTION_403));

    SetState(AosSubscription::STATE_OFFLINE);
    EXPECT_FALSE(pAosSubscription->IsWfcErrorCodeByMissing911Address(
            CarrierConfig::Assets::WFC_NO_ADDRESSS_ERROR_CODE_SUBSCRIPTION_403));

    SetState(AosSubscription::STATE_UNSUBSCRIBING);
    EXPECT_FALSE(pAosSubscription->IsWfcErrorCodeByMissing911Address(
            CarrierConfig::Assets::WFC_NO_ADDRESSS_ERROR_CODE_SUBSCRIPTION_403));

    IMSVector<IMS_SINT32> objErrDisplayRequired;
    objErrDisplayRequired.Clear();
    objErrDisplayRequired.Add(CarrierConfig::Assets::WFC_NO_ADDRESSS_ERROR_CODE_SUBSCRIPTION_403);
    EXPECT_CALL(objMockAosConfig, GetWfcSubErrorByMissing911Address())
            .Times(2)
            .WillRepeatedly(ReturnRef(objErrDisplayRequired));

    SetState(AosSubscription::STATE_SUBSCRIBED);
    EXPECT_TRUE(pAosSubscription->IsWfcErrorCodeByMissing911Address(
            CarrierConfig::Assets::WFC_NO_ADDRESSS_ERROR_CODE_SUBSCRIPTION_403));

    SetState(AosSubscription::STATE_SUBSCRIBED);
    EXPECT_FALSE(pAosSubscription->IsWfcErrorCodeByMissing911Address(
            CarrierConfig::Assets::WFC_NO_ADDRESSS_ERROR_CODE_NOTIFY_TERMINATED));
}

TEST_F(AosSubscriptionTest, CheckNotifyReceived)
{
    SetState(AosSubscription::STATE_SUBSCRIBED);
    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_FALSE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    MockIRegInfo objMockIRegInfo;
    EXPECT_CALL(objMockIRegSubscription, GetRegInfo())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IRegInfo*>(&objMockIRegInfo)));

    MockIRegInfoRegistration objMockIRegInfoRegistration;
    EXPECT_CALL(objMockIRegInfo, GetRegistration(*pAor))
            .Times(AnyNumber())
            .WillOnce(ReturnNull())
            .WillOnce(ReturnNull())
            .WillRepeatedly(
                    Return(static_cast<IRegInfoRegistration*>(&objMockIRegInfoRegistration)));

    EXPECT_CALL(objMockAosConfig, IsRegistrationEventForCatRequired())
            .Times(4)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    IMSList<IRegInfoContact*> objContact;
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

    EXPECT_CALL(objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockAosConfig, GetExtraRegErrMaxCount())
            .WillOnce(Return(5))
            .WillRepeatedly(Return(0));

    EXPECT_CALL(objMockAosService, NotifyRegEventState(AosRegEvent::ACTIVE)).Times(1);

    EXPECT_CALL(objMockAosRetryRepository, ResetRetryCount(0)).Times(1);

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

    EXPECT_CALL(objMockAosConfig, GetNotifyEventForInitialRegistration())
            .Times(6)
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_EXPIRED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_DEACTIVATED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_PROBATION))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_UNREGITERED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED));

    EXPECT_CALL(objMockAosConfig, GetNotifyEventForInitialRegWithWaitTime())
            .Times(6)
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_EXPIRED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED))
            .WillOnce(Return(IAosNConfiguration::NOTIFY_TERMINATED_REJECTED));

    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::COMMAND_REG_TERMINATED, 0))
            .Times(1);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    // ReportNotifyEvent() - bRegRequired == TRUE
    EXPECT_CALL(objMockAosConfig, GetNotifyWaitTime()).Times(2).WillRepeatedly(Return(60));

    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(*pMockAosAppContext, GetConnection())
            .Times(5)
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(5)
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    IMSVector<IMS_SINT32> objErrDisplayRequired;
    objErrDisplayRequired.Clear();
    objErrDisplayRequired.Add(1);
    EXPECT_CALL(objMockAosConfig, GetWfcSubErrorByMissing911Address())
            .Times(1)
            .WillOnce(ReturnRef(objErrDisplayRequired));
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::COMMAND_REG_REQUIRED_WITH_NOTI_NO_911_ADDR, 0))
            .Times(1);

    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::COMMAND_REG_REQUIRED, _))
            .Times(4);

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

    EXPECT_CALL(objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_TRUE));
    RefreshTimerExpiredListener(bDoImplicitRefresh);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHING);
    EXPECT_EQ(bDoImplicitRefresh, IMS_TRUE);

    EXPECT_CALL(objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_FALSE));
    RefreshTimerExpiredListener(bDoImplicitRefresh);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
    EXPECT_EQ(bDoImplicitRefresh, IMS_FALSE);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionStarted)
{
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_SUBSCRIBED, AosSubscription::REASON_SUB_ESTABLISHED))
            .Times(1);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_STARTED, 0, 0);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBED);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionUpdated)
{
    EXPECT_CALL(objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_SUBSCRIBED, AosSubscription::REASON_SUB_ESTABLISHED))
            .Times(1);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_UPDATED, 0, 0);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBED);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionStartFailed_Others)
{
    EXPECT_CALL(objMockAosConfig, IsRegRetryIntervalsUsedForSub())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objMockAosConfig, GetRegistrationRetryBaseTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(30000));
    EXPECT_CALL(objMockAosConfig, GetRegistrationRetryMaxTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1800000));

    // SetRetryTimer - GetNextThrottlingTime(objRetryIntervals)
    IMSVector<IMS_SINT32> objRetryIntervals;
    objRetryIntervals.Clear();
    objRetryIntervals.Add(10);
    objRetryIntervals.Add(10);
    objRetryIntervals.Add(10);
    EXPECT_CALL(objMockAosConfig, GetRegRetryIntervals())
            .WillOnce(ReturnRef(objRetryIntervals))
            .WillOnce(ReturnRef(objRetryIntervals));

    SetThrottlingCount(0);
    IMSVector<IMS_SINT32> objRetryRandomIntervals;
    objRetryRandomIntervals.Clear();
    objRetryRandomIntervals.Add(0);
    objRetryRandomIntervals.Add(5);
    objRetryRandomIntervals.Add(0);
    EXPECT_CALL(objMockAosConfig, GetRegRandomRetryIntervals())
            .WillOnce(ReturnRef(objRetryRandomIntervals))
            .WillOnce(ReturnRef(objRetryRandomIntervals));

    // ProcessStartFailed_Others - pAosSubscription->SetRetryTimer(IMS_FALSE);
    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_INTERNAL_ERROR, 0);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    SetThrottlingCount(3);
    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_INTERNAL_ERROR, 0);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionStartFailed_StatusCode)
{
    // ProcessStartFailed_StatusCode - pAosSubscription->SetRetryTimer(IMS_FALSE);
    // IsRetryActionDueToRetrycounter - FALSE
    EXPECT_CALL(objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(3)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockAosConfig, GetExtraRegErrMaxCount()).Times(3).WillRepeatedly(Return(0));

    // IsSubscriptionTerminated - FALSE
    EXPECT_CALL(objMockAosConfig, GetRetryCountSubErrorSubTerminated())
            .Times(3)
            .WillRepeatedly(Return(0));

    // IsInitialRegistrationRequired - FALSE
    EXPECT_CALL(objMockAosConfig, GetRetryCountSubErrorRegRequired())
            .Times(3)
            .WillRepeatedly(Return(0));

    // IsInitialRegistrationWithNextPcscfRequired - FALSE
    IMSVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Clear();
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(3)
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));

    // IsInitialRegistrationRequiredInWifi - FALSE
    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled()).Times(3).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(*pMockAosAppContext, GetConnection())
            .Times(3)
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    SetThrottlingCount(0);
    EXPECT_CALL(objMockAosConfig, IsRegRetryIntervalsUsedForSub())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objMockAosConfig, GetRegistrationRetryBaseTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(30000));
    EXPECT_CALL(objMockAosConfig, GetRegistrationRetryMaxTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1800000));

    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_TRANSACTION_TIMEOUT, 0);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    MockISipMessage objMockSipMsg;
    EXPECT_CALL(objMockSipMsg, GetStatusCode()).WillRepeatedly(Return(481));

    AString strHeader = "";
    strHeader.Append("60");
    EXPECT_CALL(objMockSipMsg, GetHeader(ISipHeader::RETRY_AFTER_SEC, 0, AString::ConstNull()))
            .WillOnce(Return(strHeader));

    EXPECT_CALL(objMockIRegSubscription, GetPreviousResponse())
            .WillOnce(Return(static_cast<ISipMessage*>(&objMockSipMsg)))
            .WillOnce(Return(static_cast<ISipMessage*>(&objMockSipMsg)))
            .WillOnce(ReturnNull())
            .WillOnce(ReturnNull());

    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionUpdateFailed_others)
{
    EXPECT_CALL(objMockAosConfig, IsRegRetryIntervalsUsedForSub())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockAosConfig, GetRegistrationRetryBaseTime()).Times(1).WillOnce(Return(30000));
    EXPECT_CALL(objMockAosConfig, GetRegistrationRetryMaxTime()).Times(1).WillOnce(Return(1800000));

    // pAosSubscription->SetRetryTimer(IMS_FALSE);
    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_UPDATE_FAILED,
            IRegSubscription::REASON_REFRESH_INTERNAL_ERROR, 0);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);
}

TEST_F(AosSubscriptionTest, CheckRegSubscriptionUpdateFailed_StatusCode)
{
    // IsRetryActionDueToRetrycounter - FALSE
    EXPECT_CALL(objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(2)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockAosConfig, GetExtraRegErrMaxCount()).Times(2).WillRepeatedly(Return(0));

    // IsSubscriptionTerminated - FALSE
    EXPECT_CALL(objMockAosConfig, GetRetryCountSubErrorSubTerminated())
            .Times(2)
            .WillRepeatedly(Return(0));

    // IsInitialRegistrationRequired - FALSE
    EXPECT_CALL(objMockAosConfig, GetRetryCountSubErrorRegRequired())
            .Times(2)
            .WillRepeatedly(Return(0));

    // IsInitialRegistrationWithNextPcscfRequired - FALSE
    IMSVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Clear();
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));

    // IsInitialRegistrationRequiredInWifi - FALSE
    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(*pMockAosAppContext, GetConnection())
            .Times(2)
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    // IsResubscriptionStopped - FALSE
    IMSVector<IMS_SINT32> objErrResubStopped;
    objErrResubStopped.Clear();
    EXPECT_CALL(objMockAosConfig, GetSubErrorStoppingResub())
            .Times(2)
            .WillRepeatedly(ReturnRef(objErrResubStopped));

    EXPECT_CALL(objMockAosConfig, IsRegRetryIntervalsUsedForSub())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objMockAosConfig, GetRegistrationRetryBaseTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(30000));
    EXPECT_CALL(objMockAosConfig, GetRegistrationRetryMaxTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1800000));

    MockISipMessage objMockSipMsg;
    EXPECT_CALL(objMockSipMsg, GetStatusCode()).WillRepeatedly(Return(404));

    EXPECT_CALL(objMockIRegSubscription, GetPreviousResponse()).WillRepeatedly(ReturnNull());

    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_TRANSACTION_TIMEOUT, 0);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);

    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_REFRESH_TIMEOUT, 0);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);

    EXPECT_CALL(objMockIRegSubscription, GetPreviousResponse()).WillRepeatedly(ReturnNull());
    NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);
}

TEST_F(AosSubscriptionTest, RegSubscription_End)
{
    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_REMOVED, IRegSubscription::REASON_NONE, 0);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_TERMINATED, IRegSubscription::REASON_NO_EXPIRES, 0);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

TEST_F(AosSubscriptionTest, ProcessTimerExpired)
{
    IMS_UINT32 nCurrState = pAosSubscription->GetState();
    Timer_TimerExpired(IMS_NULL);
    EXPECT_EQ(pAosSubscription->GetState(), nCurrState);

    // Timer_TimerExpired (piTimer != m_piRetryTimer) to do

    EXPECT_CALL(objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_FALSE));

    // ProcessTimerExpired() - return;
    SetState(AosSubscription::STATE_OFFLINE);
    StartTimer(30);
    Timer_TimerExpired(GetTimer());
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);

    // ProcessTimerExpired() - Subscription_CanBeTransmitted() == IMS_FALSE
    SetState(AosSubscription::STATE_SUBSTOP);

    StartTimer(30);
    Timer_TimerExpired(GetTimer());
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    SetState(AosSubscription::STATE_SUBREFRESHSTOP);
    StartTimer(30);
    Timer_TimerExpired(GetTimer());
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);

    EXPECT_CALL(objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillRepeatedly(Return(IMS_TRUE));

    // ProcessTimerExpired() - Subscription_CanBeTransmitted() == IMS_TRUE
    // SendSubscribe() == IMS_FALSE m_piRegSubscription == IMS_NULL
    SetState(AosSubscription::STATE_SUBSTOP);
    StartTimer(30);
    Timer_TimerExpired(GetTimer());

    SetpiRegSubscription(IMS_NULL);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBING);

    // ProcessTimerExpired() - SendSubscribe() = IMS_TRUE Subscribe() == IMS_SUCCESS
    SetpiRegSubscription(static_cast<IRegSubscription*>(&objMockIRegSubscription));
    EXPECT_CALL(objMockIRegSubscription, Subscribe())
            .WillOnce(Return(IMS_FAILURE))
            .WillRepeatedly(Return(IMS_SUCCESS));
    Timer_TimerExpired(GetTimer());
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBING);

    SetState(AosSubscription::STATE_SUBSTOP);
    StartTimer(30);
    Timer_TimerExpired(GetTimer());

    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);

    SetState(AosSubscription::STATE_SUBREFRESHSTOP);
    StartTimer(30);
    Timer_TimerExpired(GetTimer());
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHING);
}

TEST_F(AosSubscriptionTest, Print)
{
    int nState = AosSubscription::STATE_OFFLINE;
    while (nState <= AosSubscription::STATE_UNSUBSCRIBING)
    {
        SetState(nState);
        pAosSubscription->StateToString(nState);
        nState++;
    }
    pAosSubscription->StateToString(nState);

    int nReason = IRegSubscription::REASON_NONE;
    while (nReason <= IRegSubscription::REASON_NOTIFY_TERMINATED)
    {
        pAosSubscription->RegSubReasonToString(nReason);
        nReason++;
    }
    pAosSubscription->RegSubReasonToString(nReason);
    nReason = IRegSubscription::REASON_NONE;

    int nNotifyState = IRegInfoContact::STATE_CREATED;
    while (nNotifyState <= IRegInfoContact::STATE_TERMINATED)
    {
        pAosSubscription->RegInfoStateToString(nNotifyState);
        nNotifyState++;
    }
    pAosSubscription->RegInfoStateToString(nNotifyState);
    nNotifyState = IRegInfoContact::STATE_CREATED;

    int nEvent = IRegInfoContact::EVENT_REGISTERED;
    while (nEvent <= IRegInfoContact::EVENT_REJECTED)
    {
        pAosSubscription->RegInfoEventToString(nEvent);
        nEvent++;
    }
    pAosSubscription->RegInfoEventToString(nEvent);
    nEvent = IRegInfoContact::EVENT_REGISTERED;

    SetState(AosSubscription::STATE_OFFLINE);
    EXPECT_EQ(pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}