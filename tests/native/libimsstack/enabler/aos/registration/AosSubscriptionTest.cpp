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

#include "interface/MockIAosAppContext.h"
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

#define DECLARE_USING(Base)                                 \
    using Base::SetState;                                   \
    using Base::SetTerminated;                              \
    using Base::StartTimer;                                 \
    using Base::IsRadioWaiting;                             \
    using Base::IsTrafficPriorityBlocked;                   \
    using Base::SetRadioWaiting;                            \
    using Base::SetTrafficPriorityBlocked;                  \
    using Base::IsRetryActionDueToRetryCounter;             \
    using Base::IsSubscriptionTerminated;                   \
    using Base::IsInitialRegistrationRequired;              \
    using Base::IsInitialRegistrationWithNextPcscfRequired; \
    using Base::IsInitialRegistrationRequiredInWifi;        \
    using Base::IsResubscriptionStopped;                    \
    using Base::IsRegAfterWaitRequiredByNotify;             \
    using Base::IsWfcErrorMessageSupportedWithStateChecked; \
    using Base::ProcessFailed_StatusCode;                   \
    using Base::GetNextThrottlingTime;                      \
    using Base::RegSubscription_RefreshTimerExpired;        \
    using Base::Transaction_OnConnectionFailed;             \
    using Base::Transaction_OnTrafficPriorityChanged;

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
    DECLARE_USING(AosSubscription)

    inline TestAosSubscription(IN IAosAppContext* piAppContext,
            IN IRegSubscription* piRegSubscription, IN const AString& strAoR,
            IN const SipAddress& objContactAddress) :
            AosSubscription(piAppContext, piRegSubscription, strAoR, objContactAddress)
    {
    }

    FRIEND_TEST(AosSubscriptionTest, Initialize);
    FRIEND_TEST(AosSubscriptionTest, CheckNotifyReceived);
    FRIEND_TEST(AosSubscriptionTest, RegSubscription_End);

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
        m_pAosStaticProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

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
    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosNetTracker m_objMockIAosNetTracker;
    MockISipMessage m_objMockSipMsg;
    MockIRegSubscription m_objMockIRegSubscription;
    MockIRegInfo m_objMockIRegInfo;
    MockIRegInfoRegistration m_objMockIRegInfoRegistration;

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

    ImsVector<IMS_SINT32> m_objRetryIntervals;
    ImsVector<IMS_SINT32> m_objRetryRandomIntervals;
    ImsVector<IMS_SINT32> m_objErrRegRequiredWithNextPcscf;
    ImsVector<IMS_SINT32> m_objErrResubStopped;

    SipAddress m_objSipAddress = SipAddress("");
    const AString m_strSipAddrWithPort = AString("sip:1234@ims.google.com:5060");
    const AString m_strSipAddr = AString("sip:1234@ims.google.com");
    const AString m_strAnonymousAddr = AString("sip:anonymous@anonymous.invalid");

protected:
    virtual void SetUp() override
    {
        ConfigurationManager::GetInstance()->Initialize();

        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetStaticProfile())
                .WillByDefault(Return(m_pAosStaticProfile));
        ON_CALL(m_objMockIAosAppContext, GetConnection())
                .WillByDefault(Return(&m_objMockIAosConnection));
        ON_CALL(m_objMockIAosAppContext, GetNetTracker())
                .WillByDefault(Return(&m_objMockIAosNetTracker));

        ON_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
                .WillByDefault(Return(IMS_TRUE));

        ON_CALL(m_objMockIAosNetTracker, GetNetworkType())
                .WillByDefault(Return(static_cast<IMS_UINT32>(AosNetworkType::LTE)));

        ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_FALSE));

        ON_CALL(m_objMockIRegSubscription, GetPreviousResponse())
                .WillByDefault(Return(&m_objMockSipMsg));
        ON_CALL(m_objMockIRegSubscription, Subscribe()).WillByDefault(Return(IMS_SUCCESS));
        ON_CALL(m_objMockIRegSubscription, GetRegInfo()).WillByDefault(Return(&m_objMockIRegInfo));
        ON_CALL(m_objMockIRegInfo, GetRegistration(m_strSipAddrWithPort))
                .WillByDefault(Return(&m_objMockIRegInfoRegistration));

        ON_CALL(m_objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount()).WillByDefault(Return(0));
        ON_CALL(m_objMockAosConfig, GetRegistrationRetryBaseTime()).WillByDefault(Return(30000));
        ON_CALL(m_objMockAosConfig, GetRegistrationRetryMaxTime()).WillByDefault(Return(1800000));
        ON_CALL(m_objMockAosConfig, GetRegRetryCountResetPolicy()).WillByDefault(Return(0));
        ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorRegRequired()).WillByDefault(Return(0));
        ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorSubTerminated()).WillByDefault(Return(0));
        // m_objRetryIntervals.GetSize() = 0
        ON_CALL(m_objMockAosConfig, GetRegRetryIntervals())
                .WillByDefault(ReturnRef(m_objRetryIntervals));
        // m_objRetryRandomIntervals.GetSize() = 0
        ON_CALL(m_objMockAosConfig, GetRegRandomRetryIntervals())
                .WillByDefault(ReturnRef(m_objRetryRandomIntervals));
        // m_objErrRegRequiredWithNextPcscf.GetSize() = 0
        ON_CALL(m_objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
                .WillByDefault(ReturnRef(m_objErrRegRequiredWithNextPcscf));
        m_objErrRegRequiredWithNextPcscf.Clear();
        // m_objErrResubStopped.GetSize() = 0
        ON_CALL(m_objMockAosConfig, GetSubErrorStoppingResub())
                .WillByDefault(ReturnRef(m_objErrResubStopped));
        m_objErrResubStopped.Clear();

        ON_CALL(m_objMockIAosTransaction, IsTransactionAllowed(_)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIAosTransaction, StartTraffic(_, _)).WillByDefault(Return(IMS_TRUE));

        m_pAosSubscription = new TestAosSubscription(&m_objMockIAosAppContext,
                &m_objMockIRegSubscription, m_strSipAddrWithPort, m_objSipAddress);
        ASSERT_TRUE(m_pAosSubscription != nullptr);

        m_pAosSubscription->SetListener(&m_objMockIAosSubscriptionListener);
        m_pAosSubscription->SetRegSubscription(&m_objMockIRegSubscription);
    }

    virtual void TearDown() override
    {
        if (m_pAosSubscription)
        {
            m_pAosSubscription->SetListener(IMS_NULL);
            m_pAosSubscription->Destroy();
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
    EXPECT_CALL(objMockIRegInfo, GetRegistration(m_strSipAddrWithPort))
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
    objSipAddress.Create(m_strAnonymousAddr);
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
    objSipAddress2.Create(m_strSipAddrWithPort);
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
    objContactAddr.Create(m_strSipAddr);
    m_pAosSubscription->SetContactAddress(objContactAddr);
    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscription->GetAorState(), IRegInfoContact::STATE_ACTIVE);

    objContactAddr.RemoveAllParameters();
    objContactAddr.Create(m_strSipAddrWithPort);
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

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
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

TEST_F(AosSubscriptionTest, RegSubscription_End)
{
    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_REMOVED, IRegSubscription::REASON_NONE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_TERMINATED, IRegSubscription::REASON_NO_EXPIRES, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

/// Start()
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

/// Stop()
TEST_F(AosSubscriptionTest, CallUnsubscribeInStopWhenUnsubscriptionIsSupported)
{
    ON_CALL(m_objMockAosConfig, IsUnSubscription()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIRegSubscription, Unsubscribe()).Times(1);

    m_pAosSubscription->Stop();
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_UNSUBSCRIBING);
}

TEST_F(AosSubscriptionTest, NotCallUnsubscribeInStopWhenUnsubscriptionIsNotSupported)
{
    IMS_UINT32 nState = m_pAosSubscription->GetState();
    ON_CALL(m_objMockAosConfig, IsUnSubscription()).WillByDefault(Return(IMS_FALSE));

    m_pAosSubscription->Stop();
    EXPECT_EQ(m_pAosSubscription->GetState(), nState);
}

/// IsRetryActionDueToRetryCounter()
TEST_F(AosSubscriptionTest, ReturnTrueWhenRetryCountIsNeedToIncreaseAgainstInit)
{
    ON_CALL(m_objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount()).WillByDefault(Return(5));
    ON_CALL(m_objMockAosRetryRepository, IncreaseRetryCount(0)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED))
            .Times(1);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF, 0, IMS_FALSE))
            .Times(1);

    EXPECT_TRUE(m_pAosSubscription->IsRetryActionDueToRetryCounter(IMS_FALSE));
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenRetryCountIsNeedToIncreaseAgainstRefreshed)
{
    ON_CALL(m_objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount()).WillByDefault(Return(3));
    ON_CALL(m_objMockAosRetryRepository, IncreaseRetryCount(0)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF, 0, IMS_FALSE))
            .Times(1);

    EXPECT_TRUE(m_pAosSubscription->IsRetryActionDueToRetryCounter(IMS_TRUE));
}

TEST_F(AosSubscriptionTest, ReturnFalseWhenExtraRegErrRetryCntSharedIsNotUsed)
{
    ON_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount()).WillByDefault(Return(0));

    EXPECT_FALSE(m_pAosSubscription->IsRetryActionDueToRetryCounter(IMS_TRUE));
}

/// IsSubscriptionTerminated()
TEST_F(AosSubscriptionTest, ReturnFalseWhenRetryCountForTerminatingSubIsZero)
{
    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorSubTerminated()).WillByDefault(Return(0));

    EXPECT_FALSE(m_pAosSubscription->IsSubscriptionTerminated(403));
}

TEST_F(AosSubscriptionTest, ReturnFalseWhenNumberOfCountsForTerminatingSubIsNotReached)
{
    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorSubTerminated()).WillByDefault(Return(2));

    ImsVector<IMS_SINT32> objErrSubTerminated;
    objErrSubTerminated.Add(404);
    objErrSubTerminated.Add(403);

    ON_CALL(m_objMockAosConfig, GetSubErrorSubTerminated())
            .WillByDefault(ReturnRef(objErrSubTerminated));

    m_pAosSubscription->SetRetryCountSubTerminated(0);
    EXPECT_FALSE(m_pAosSubscription->IsSubscriptionTerminated(403));
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenNumberOfCountsForTerminatingSubIsReached)
{
    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorSubTerminated()).WillByDefault(Return(2));

    ImsVector<IMS_SINT32> objErrSubTerminated;
    objErrSubTerminated.Add(404);
    objErrSubTerminated.Add(403);

    ON_CALL(m_objMockAosConfig, GetSubErrorSubTerminated())
            .WillByDefault(ReturnRef(objErrSubTerminated));

    m_pAosSubscription->SetRetryCountSubTerminated(1);
    EXPECT_TRUE(m_pAosSubscription->IsSubscriptionTerminated(403));
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenSetErrIsIndicatedBySingleDigit)
{
    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorSubTerminated()).WillByDefault(Return(2));

    ImsVector<IMS_SINT32> objErrSubTerminated;
    objErrSubTerminated.Add(5);
    objErrSubTerminated.Add(403);

    ON_CALL(m_objMockAosConfig, GetSubErrorSubTerminated())
            .WillByDefault(ReturnRef(objErrSubTerminated));

    m_pAosSubscription->SetRetryCountSubTerminated(1);
    EXPECT_TRUE(m_pAosSubscription->IsSubscriptionTerminated(504));
}

/// IsInitialRegistrationRequired()
TEST_F(AosSubscriptionTest, ReturnFalseWhenRetryCountForRegRequiredIsLessthanOne)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorRegRequired()).WillByDefault(Return(0));

    EXPECT_FALSE(m_pAosSubscription->IsInitialRegistrationRequired(403, IMS_FALSE));
}

TEST_F(AosSubscriptionTest, ReturnFalseWhenErrForRegRequiredIsNotMatched)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorRegRequired()).WillByDefault(Return(2));

    ImsVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Add(404);
    objErrRegRequired.Add(403);

    ON_CALL(m_objMockAosConfig, GetSubErrorRegRequired())
            .WillByDefault(ReturnRef(objErrRegRequired));

    EXPECT_FALSE(m_pAosSubscription->IsInitialRegistrationRequired(400, IMS_FALSE));
}

TEST_F(AosSubscriptionTest, ReturnFalseWhenErrForRegRequiredIsMatchedBySingleDigit)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorRegRequired()).WillByDefault(Return(2));

    ImsVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Add(3);
    objErrRegRequired.Add(403);

    ON_CALL(m_objMockAosConfig, GetSubErrorRegRequired())
            .WillByDefault(ReturnRef(objErrRegRequired));

    m_pAosSubscription->SetRetryCountRegRequired(0);
    EXPECT_FALSE(m_pAosSubscription->IsInitialRegistrationRequired(380, IMS_FALSE));
}

TEST_F(AosSubscriptionTest, ReturnFalseWhenRetryCountForRegRequiredIsNotReached)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorRegRequired()).WillByDefault(Return(2));

    ImsVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Add(404);
    objErrRegRequired.Add(403);

    ON_CALL(m_objMockAosConfig, GetSubErrorRegRequired())
            .WillByDefault(ReturnRef(objErrRegRequired));

    m_pAosSubscription->SetRetryCountRegRequired(0);
    EXPECT_FALSE(m_pAosSubscription->IsInitialRegistrationRequired(403, IMS_FALSE));
}

TEST_F(AosSubscriptionTest, ReturnFalseWhenRetryCountForRegRequiredIsReached)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorRegRequired()).WillByDefault(Return(3));

    ImsVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Add(404);
    objErrRegRequired.Add(403);

    ON_CALL(m_objMockAosConfig, GetSubErrorRegRequired())
            .WillByDefault(ReturnRef(objErrRegRequired));

    m_pAosSubscription->SetRetryCountRegRequired(2);

    // ReportState() if result is true.
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED))
            .Times(1);

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_FALSE))
            .Times(1);

    EXPECT_TRUE(m_pAosSubscription->IsInitialRegistrationRequired(403, IMS_FALSE));
}

TEST_F(AosSubscriptionTest, ReturnFalseWhenErrForRegRequiredWithNextPcscfIsLessthanOne)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
    ImsVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;

    ON_CALL(m_objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .WillByDefault(ReturnRef(objErrRegRequiredWithNextPcscf));

    EXPECT_FALSE(m_pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(403, IMS_FALSE));
}

/// IsInitialRegistrationWithNextPcscfRequired()
TEST_F(AosSubscriptionTest, ReturnFalseWhenErrForRegRequiredWithNextPcscfIsNotReached)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
    ImsVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Add(400);
    objErrRegRequiredWithNextPcscf.Add(403);

    ON_CALL(m_objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .WillByDefault(ReturnRef(objErrRegRequiredWithNextPcscf));
    EXPECT_FALSE(m_pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(408, IMS_FALSE));
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenErrForRegRequiredWithNextPcscfIsReached)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
    ImsVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Add(400);
    objErrRegRequiredWithNextPcscf.Add(403);

    ON_CALL(m_objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .WillByDefault(ReturnRef(objErrRegRequiredWithNextPcscf));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF, 0, IMS_FALSE))
            .Times(1);

    EXPECT_TRUE(m_pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(403, IMS_FALSE));
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenErrForRegRequiredWithNextPcscfIsMatchedBySingleDigit)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
    ImsVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Add(6);
    objErrRegRequiredWithNextPcscf.Add(403);

    ON_CALL(m_objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .WillByDefault(ReturnRef(objErrRegRequiredWithNextPcscf));
    m_pAosSubscription->SetTerminated(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF, 0, IMS_FALSE))
            .Times(1);

    EXPECT_TRUE(m_pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(603, IMS_TRUE));
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

/// IsInitialRegistrationRequiredInWifi()
TEST_F(AosSubscriptionTest, ReturnFalseWhenThereIsNoInfoForErrRegRequiredInWifi)
{
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));

    // GetVowifiSubErrorRegRequired() - no info
    ImsVector<IMS_SINT32> objErrRegRequiredInWifi;
    ON_CALL(m_objMockAosConfig, GetVowifiSubErrorRegRequired())
            .WillByDefault(ReturnRef(objErrRegRequiredInWifi));

    EXPECT_FALSE(m_pAosSubscription->IsInitialRegistrationRequiredInWifi(403, IMS_FALSE));
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenErrRegRequiredInWifiIsMatchedBySingleDigit)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));

    ImsVector<IMS_SINT32> objErrRegRequiredInWifi;
    objErrRegRequiredInWifi.Add(3);
    objErrRegRequiredInWifi.Add(403);
    ON_CALL(m_objMockAosConfig, GetVowifiSubErrorRegRequired())
            .WillByDefault(ReturnRef(objErrRegRequiredInWifi));
    ON_CALL(m_objMockAosConfig,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_SUB_403))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockAosConfig, GetRegRetryCountResetPolicy()).WillByDefault(Return(2));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED));
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_TRUE));
    EXPECT_TRUE(m_pAosSubscription->IsInitialRegistrationRequiredInWifi(301, IMS_FALSE));
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenWfcErrorMessageIsSupported)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSTOP);
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));

    ImsVector<IMS_SINT32> objErrRegRequiredInWifi;
    objErrRegRequiredInWifi.Add(404);
    objErrRegRequiredInWifi.Add(403);
    ON_CALL(m_objMockAosConfig, GetVowifiSubErrorRegRequired())
            .WillByDefault(ReturnRef(objErrRegRequiredInWifi));

    ON_CALL(m_objMockAosConfig,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_SUB_403))
            .WillByDefault(Return(IMS_TRUE));

    // RequestCommand
    ON_CALL(m_objMockAosConfig, GetRegRetryCountResetPolicy()).WillByDefault(Return(2));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED_WITH_SUB_403_MSG, 0, IMS_TRUE));

    EXPECT_TRUE(m_pAosSubscription->IsInitialRegistrationRequiredInWifi(403, IMS_FALSE));
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenWfcErrorMessageIsNotSupportedInRefreshState)
{
    // UE is in SUBREFRESHSTOP state and the UE isn't terminated yet
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHSTOP);
    m_pAosSubscription->SetTerminated(IMS_FALSE);

    // Condition: The UE is in WiFi State
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));

    // Condition: The 500 error in the test scenario is matched
    ImsVector<IMS_SINT32> objErrRegRequiredInWifi;
    objErrRegRequiredInWifi.Add(400);
    objErrRegRequiredInWifi.Add(500);
    ON_CALL(m_objMockAosConfig, GetVowifiSubErrorRegRequired())
            .WillByDefault(ReturnRef(objErrRegRequiredInWifi));

    // Condition: The Error isn't supported for showing wfc error message
    ON_CALL(m_objMockAosConfig,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_SUB_403))
            .WillByDefault(Return(IMS_FALSE));

    // Expected result: The UE will have to call Subscription_StateChanged with OFFLINE state and
    // SUB_TERMINATED reason
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED));

    // Expected result: The UE will have to request registration without wait time
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_FALSE));

    // The result will be true
    // when the UE calls IsInitialRegistrationRequiredInWifi() with 500 error & UE in refresh state
    EXPECT_TRUE(m_pAosSubscription->IsInitialRegistrationRequiredInWifi(500, IMS_TRUE));
    // The result state of AosSubscription will be offline after all of the above
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

// IsResubscriptionStopped()
TEST_F(AosSubscriptionTest, ReturnFalseWhenResubStoppedIsLessthanOne)
{
    // m_objErrResubStopped.GetSize() = 0 condition
    // This conditions was defined in the SetUp() function.

    EXPECT_FALSE(m_pAosSubscription->IsResubscriptionStopped(403));
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenResubStoppedIsReachedSecond)
{
    m_objErrResubStopped.Add(404);
    m_objErrResubStopped.Add(403);

    EXPECT_TRUE(m_pAosSubscription->IsResubscriptionStopped(403));
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenResubStoppedIsMatchedBySingleDigit)
{
    m_objErrResubStopped.Add(403);
    m_objErrResubStopped.Add(5);

    EXPECT_TRUE(m_pAosSubscription->IsResubscriptionStopped(503));
}

/// IsRegAfterWaitRequiredByNotify()
TEST_F(AosSubscriptionTest, ReturnFalseWhenWaitingRetryAfterByNotifyIsNotSet)
{
    IMS_UINT32 nFeature = 0;
    nFeature |= IAosNConfiguration::NOTIFY_TERMINATED_EXPIRED;
    nFeature |= IAosNConfiguration::NOTIFY_TERMINATED_UNREGISTERED;

    ON_CALL(m_objMockAosConfig, GetNotifyEventForInitialRegWithWaitTime())
            .WillByDefault(Return(IAosNConfiguration::NOTIFY_TERMINATED_DEACTIVATED));

    EXPECT_FALSE(m_pAosSubscription->IsRegAfterWaitRequiredByNotify(nFeature));
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenWaitingRetryAfterByNotifyIsSet)
{
    IMS_UINT32 nFeature = 0;
    nFeature |= IAosNConfiguration::NOTIFY_TERMINATED_PROBATION;
    nFeature |= IAosNConfiguration::NOTIFY_TERMINATED_UNREGISTERED;

    ON_CALL(m_objMockAosConfig, GetNotifyEventForInitialRegWithWaitTime())
            .WillByDefault(Return(IAosNConfiguration::NOTIFY_TERMINATED_PROBATION));

    EXPECT_TRUE(m_pAosSubscription->IsRegAfterWaitRequiredByNotify(nFeature));
}

/// IsWfcErrorMessageSupportedWithStateChecked()
TEST_F(AosSubscriptionTest, ReturnFalseWhenEpdgEnabledIsFalseWhileCheckingWfcErrMessage)
{
    // Condition: IsEpdgEnabled() is false. It was defined in the SetUp() function.

    EXPECT_FALSE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_SUB_403));
}

TEST_F(AosSubscriptionTest, ReturnFalseWhenStateIsNotAppropriateWhileCheckingWfcErrMessage)
{
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockAosConfig,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_SUB_403))
            .WillByDefault(Return(IMS_TRUE));

    // state is STATE_OFFLINE
    m_pAosSubscription->SetState(AosSubscription::STATE_OFFLINE);
    EXPECT_FALSE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_SUB_403));

    // state is STATE_UNSUBSCRIBING
    m_pAosSubscription->SetState(AosSubscription::STATE_UNSUBSCRIBING);
    EXPECT_FALSE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_SUB_403));
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenWfcErrorMessageIsSupported403)
{
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockAosConfig,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_SUB_403))
            .WillByDefault(Return(IMS_TRUE));

    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBED);
    EXPECT_TRUE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_SUB_403));
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenWfcErrorMessageIsSupportedNotifyWithTerminated)
{
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockAosConfig,
            IsWfcErrorMessageSupported(CarrierConfig::Assets::WFC_ERROR_NOTIFY_TERMINATED))
            .WillByDefault(Return(IMS_TRUE));

    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBED);
    EXPECT_TRUE(m_pAosSubscription->IsWfcErrorMessageSupportedWithStateChecked(
            CarrierConfig::Assets::WFC_ERROR_NOTIFY_TERMINATED));
}

// ProcessFailed_StatusCode - IsRetryActionDueToRetryCounter()
TEST_F(AosSubscriptionTest, RequestRegWithNextPcscfWhenRetryIsReachedMax)
{
    // IsRetryActionDueToRetryCounter()
    ON_CALL(m_objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount()).WillByDefault(Return(5));

    // AosRetryRepository::TYPE_NORMAL = 0, Retry Count is reached MAX
    ON_CALL(m_objMockAosRetryRepository, IncreaseRetryCount(0)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED));
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF, 0, IMS_FALSE));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));
}

// ProcessFailed_StatusCode - IsSubscriptionTerminated()
TEST_F(AosSubscriptionTest, RequestSubTerminatedWhenRetryCntSubTerminatedIsReached)
{
    m_pAosSubscription->SetRetryCountSubTerminated(0);

    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorSubTerminated()).WillByDefault(Return(1));

    ImsVector<IMS_SINT32> objErrSubTerminated;
    objErrSubTerminated.Add(404);
    ON_CALL(m_objMockAosConfig, GetSubErrorSubTerminated())
            .WillByDefault(ReturnRef(objErrSubTerminated));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED));
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_SUB_TERMINATED, 0, IMS_FALSE));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(404, IMS_FALSE));
}

TEST_F(AosSubscriptionTest, NotRequestSubTerminatedWhenRetryCntSubTerminatedIsNotReached)
{
    m_pAosSubscription->SetRetryCountSubTerminated(0);

    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorSubTerminated()).WillByDefault(Return(2));

    ImsVector<IMS_SINT32> objErrSubTerminated;
    objErrSubTerminated.Add(404);
    ON_CALL(m_objMockAosConfig, GetSubErrorSubTerminated())
            .WillByDefault(ReturnRef(objErrSubTerminated));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED))
            .Times(0);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_SUB_TERMINATED, 0, IMS_FALSE))
            .Times(0);

    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(404, IMS_FALSE));
}

// ProcessFailed_StatusCode - IsInitialRegistrationRequired()
TEST_F(AosSubscriptionTest, RequestRegRequiredWhenRetryCntRegRequiredIsReached)
{
    m_pAosSubscription->SetRetryCountRegRequired(2);

    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorRegRequired()).WillByDefault(Return(3));

    ImsVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Add(403);
    ON_CALL(m_objMockAosConfig, GetSubErrorRegRequired())
            .WillByDefault(ReturnRef(objErrRegRequired));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED));
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_FALSE));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));
}

TEST_F(AosSubscriptionTest, NotRequestRegRequiredWhenRetryCntRegRequiredIsNotReached)
{
    m_pAosSubscription->SetRetryCountRegRequired(0);

    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorRegRequired()).WillByDefault(Return(2));

    ImsVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Add(403);
    ON_CALL(m_objMockAosConfig, GetSubErrorRegRequired())
            .WillByDefault(ReturnRef(objErrRegRequired));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED))
            .Times(0);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_FALSE))
            .Times(0);

    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));
}

// ProcessFailed_StatusCode - IsInitialRegistrationWithNextPcscfRequired()
TEST_F(AosSubscriptionTest, RequestRegRequiredWithNextPcscfWhenErrRegRequiredWithNextPcscfIsMatched)
{
    m_objErrRegRequiredWithNextPcscf.Add(403);

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED));
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF, 0, IMS_FALSE));
    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));
}

TEST_F(AosSubscriptionTest,
        NotRequestRegRequiredWithNextPcscfWhenErrRegRequiredWithNextPcscfIsNotMatched)
{
    m_objErrRegRequiredWithNextPcscf.Add(403);

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED))
            .Times(0);
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF, 0, IMS_FALSE))
            .Times(0);
    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(480, IMS_FALSE));
}

// ProcessFailed_StatusCode - IsInitialRegistrationRequiredInWifi()
TEST_F(AosSubscriptionTest, RequestRegRequiredWhenErrRegRequiredInWifiIsMatched)
{
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));

    ImsVector<IMS_SINT32> objErrRegRequiredInWifi;
    objErrRegRequiredInWifi.Add(403);
    ON_CALL(m_objMockAosConfig, GetVowifiSubErrorRegRequired())
            .WillByDefault(ReturnRef(objErrRegRequiredInWifi));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED));
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_FALSE));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_FALSE));
}

TEST_F(AosSubscriptionTest, ReturnFalseWhenEpdgIsNotEnabledWhileCheckingErrorCode)
{
    // Condition: IsEpdgEnabled() is false. It was defined in the SetUp() function.

    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(500, IMS_FALSE));
}

// ProcessFailed_StatusCode - 423 error code
TEST_F(AosSubscriptionTest, ReturnTrueWhen423With600MinExpires)
{
    AString strMinTime = "600";
    ON_CALL(m_objMockSipMsg, GetHeader(ISipHeader::MIN_EXPIRES, 0, AString::ConstNull()))
            .WillByDefault(Return(strMinTime));

    ON_CALL(m_objMockIRegSubscription, Subscribe()).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(m_objMockIRegSubscription, SetExpires(600));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(423, IMS_FALSE));
}

// ProcessFailed_StatusCode - 423 error code
TEST_F(AosSubscriptionTest, ReturnTrueWhen423With600MinExpiresAgainstRefresh)
{
    AString strMinTime = "600";
    ON_CALL(m_objMockSipMsg, GetHeader(ISipHeader::MIN_EXPIRES, 0, AString::ConstNull()))
            .WillByDefault(Return(strMinTime));
    ON_CALL(m_objMockIRegSubscription, Subscribe()).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(m_objMockIRegSubscription, SetExpires(600));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(423, IMS_TRUE));
}

TEST_F(AosSubscriptionTest, ReturnFalseWhen423With600MinExpiresAndSendingSubscribeFailed)
{
    AString strMinTime = "600";
    ON_CALL(m_objMockSipMsg, GetHeader(ISipHeader::MIN_EXPIRES, 0, AString::ConstNull()))
            .WillByDefault(Return(strMinTime));

    ON_CALL(m_objMockIRegSubscription, Subscribe()).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(m_objMockIRegSubscription, SetExpires(600));

    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(423, IMS_FALSE));
}

TEST_F(AosSubscriptionTest, ReturnFalseWhen423WithZeroMinExpires)
{
    AString strMinTime = "";
    ON_CALL(m_objMockSipMsg, GetHeader(ISipHeader::MIN_EXPIRES, 0, AString::ConstNull()))
            .WillByDefault(Return(strMinTime));

    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(423, IMS_FALSE));
}

// ProcessFailed_StatusCode - 503 error code
TEST_F(AosSubscriptionTest, ShouldRequestScscfRestorationWithoutRetryAfterIfNoRetryAfter)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHING);
    m_pAosSubscription->SetTerminated(IMS_FALSE);

    // 503 error response without retry after
    ON_CALL(m_objMockSipMsg, GetStatusCode()).WillByDefault(Return(503));
    AString strRetryAfter = AString::ConstNull();
    ON_CALL(m_objMockSipMsg, GetHeader(ISipHeader::RETRY_AFTER_SEC, 0, AString::ConstNull()))
            .WillByDefault(Return(strRetryAfter));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(
                    AosSubscription::CMD_REG_REQUIRED_WITH_SCSCF_RESTORATION, 0, IMS_FALSE));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(503, IMS_TRUE));
}

TEST_F(AosSubscriptionTest,
        ShouldRequestScscfRestorationWithRetryAfterIfRetryAfterIsBiggerThanTimerF)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHING);
    m_pAosSubscription->SetTerminated(IMS_FALSE);

    // 503 error response with retry after which value is 600 seconds
    ON_CALL(m_objMockSipMsg, GetStatusCode()).WillByDefault(Return(503));
    AString strRetryAfter("600");
    ON_CALL(m_objMockSipMsg, GetHeader(ISipHeader::RETRY_AFTER_SEC, 0, AString::ConstNull()))
            .WillByDefault(Return(strRetryAfter));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(
                    AosSubscription::CMD_REG_REQUIRED_WITH_SCSCF_RESTORATION, 600, IMS_FALSE));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(503, IMS_TRUE));
}

TEST_F(AosSubscriptionTest,
        ShouldNotRequestScscfRestorationIfRetryAfterIsSmallerThanOrEqualToTimerF)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHING);
    m_pAosSubscription->SetTerminated(IMS_FALSE);

    // 503 error response with retry after which value is 60 seconds
    ON_CALL(m_objMockSipMsg, GetStatusCode()).WillByDefault(Return(503));
    AString strRetryAfter("60");
    ON_CALL(m_objMockSipMsg, GetHeader(ISipHeader::RETRY_AFTER_SEC, 0, AString::ConstNull()))
            .WillByDefault(Return(strRetryAfter));

    // Should Not Request CMD_REG_REQUIRED_WITH_SCSCF_RESTORATION
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(
                    AosSubscription::CMD_REG_REQUIRED_WITH_SCSCF_RESTORATION, _, IMS_FALSE))
            .Times(0);

    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(503, IMS_TRUE));
}

// ProcessFailed_StatusCode - 504 error code
TEST_F(AosSubscriptionTest, ReturnTrueWhen504WithXmlAgainstInitAndRefresh)
{
    ImsList<ISipMessageBodyPart*> objBodyParts;
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

    ON_CALL(m_objMockSipMsg, GetBodyParts()).WillByDefault(Return(objBodyParts));

    // 504 response against init SUBSCRIBE
    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(504, IMS_FALSE));

    // 504 response against refresh SUBSCRIBE
    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(504, IMS_TRUE));
}

TEST_F(AosSubscriptionTest, ReturnTrueWhen504WithoutXml)
{
    ImsList<ISipMessageBodyPart*> objBodyParts;
    SipMessageBodyPart objBodyPart;
    ISipMessageBodyPart* piBodyPart = static_cast<ISipMessageBodyPart*>(&objBodyPart);

    // nStatusCode == SipStatusCode::SC_504 GetBodyParts empty;
    EXPECT_CALL(m_objMockSipMsg, GetBodyParts()).WillOnce(Return(objBodyParts));
    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(504, IMS_FALSE));
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenSipMsgIsNullAndReceive504)
{
    // nStatusCode == SipStatusCode::SC_504 SipMsg null;
    EXPECT_CALL(m_objMockIRegSubscription, GetPreviousResponse())
            .WillRepeatedly(Return(static_cast<ISipMessage*>(IMS_NULL)));

    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(504, IMS_FALSE));
}

// ProcessFailed_StatusCode - 481 error code in refresh state
TEST_F(AosSubscriptionTest, ReturnTrueWhen481againstRefresh)
{
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED));
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_SUB_REQUIRED, 0, IMS_FALSE));

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(481, IMS_TRUE));
}

// ProcessFailed_StatusCode - IsResubscriptionStopped in refresh state
TEST_F(AosSubscriptionTest, ReturnTrueWhenErrResubStoppedIsMatchedInRefresh)
{
    m_objErrResubStopped.Add(403);

    EXPECT_TRUE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_TRUE));
}

TEST_F(AosSubscriptionTest, ReturnTrueWhenErrResubStoppedIsNotMatchedInRefresh)
{
    // All conditions were defined in the SetUp() function.
    // Skipped all functions in ProcessFailed_StatusCode()

    EXPECT_FALSE(m_pAosSubscription->ProcessFailed_StatusCode(403, IMS_TRUE));
}

/// Called ProcessStartFailed_Others() > SetRetryTimer(IMS_FALSE)
TEST_F(AosSubscriptionTest, RunSetRetryTimerWith3gppRetryRuleWhenStartFailed_Others)
{
    m_pAosSubscription->SetThrottlingCount(0);

    ON_CALL(m_objMockAosConfig, IsRegRetryIntervalsUsedForSub()).WillByDefault(Return(IMS_TRUE));

    // When UE calls ProcessStartFailed_Others()
    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_INTERNAL_ERROR, 0);

    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);
}

/// Called ProcessStartFailed_Others() > SetRetryTimer(IMS_FALSE)
TEST_F(AosSubscriptionTest, RunSetRetryTimerWithRetryIntervalsWhenStartFailed_Others)
{
    m_pAosSubscription->SetThrottlingCount(0);

    ON_CALL(m_objMockAosConfig, IsRegRetryIntervalsUsedForSub()).WillByDefault(Return(IMS_TRUE));

    ImsVector<IMS_SINT32> objRetryIntervals;
    objRetryIntervals.Add(30);
    objRetryIntervals.Add(30);
    objRetryIntervals.Add(60);
    objRetryIntervals.Add(120);
    objRetryIntervals.Add(480);
    objRetryIntervals.Add(900);
    ON_CALL(m_objMockAosConfig, GetRegRetryIntervals()).WillByDefault(ReturnRef(objRetryIntervals));

    ImsVector<IMS_SINT32> objRetryRandomIntervals;
    objRetryRandomIntervals.Add(0);
    objRetryRandomIntervals.Add(0);
    objRetryRandomIntervals.Add(15);
    objRetryRandomIntervals.Add(0);
    objRetryRandomIntervals.Add(0);
    objRetryRandomIntervals.Add(0);
    ON_CALL(m_objMockAosConfig, GetRegRandomRetryIntervals())
            .WillByDefault(ReturnRef(objRetryRandomIntervals));

    // When UE calls ProcessStartFailed_Others()
    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_INTERNAL_ERROR, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);
}

/// Called ProcessStartFailed_Others() > SetRetryTimer(IMS_FALSE)
TEST_F(AosSubscriptionTest, RunSetRetryTimerWithOnlyRetryIntervalsWhenStartFailed_Others)
{
    m_pAosSubscription->SetThrottlingCount(0);

    ON_CALL(m_objMockAosConfig, IsRegRetryIntervalsUsedForSub()).WillByDefault(Return(IMS_TRUE));

    ImsVector<IMS_SINT32> objRetryIntervals;
    objRetryIntervals.Add(10);
    ON_CALL(m_objMockAosConfig, GetRegRetryIntervals()).WillByDefault(ReturnRef(objRetryIntervals));

    // When UE calls ProcessStartFailed_Others()
    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_INTERNAL_ERROR, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);
}

/// Called ProcessStartFailed_Others() > SetRetryTimer(IMS_FALSE)
TEST_F(AosSubscriptionTest,
        RunSetRetryTimerWithOnlyRetryIntervalsWhenStartFailed_OthersAndThrottlingCountIsOver)
{
    m_pAosSubscription->SetThrottlingCount(3);

    ON_CALL(m_objMockAosConfig, IsRegRetryIntervalsUsedForSub()).WillByDefault(Return(IMS_TRUE));

    ImsVector<IMS_SINT32> objRetryIntervals;
    objRetryIntervals.Add(10);
    objRetryIntervals.Add(10);
    objRetryIntervals.Add(10);
    ON_CALL(m_objMockAosConfig, GetRegRetryIntervals()).WillByDefault(ReturnRef(objRetryIntervals));

    // When UE calls ProcessStartFailed_Others()
    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_INTERNAL_ERROR, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);
}

/// GetNextThrottlingTime()
TEST_F(AosSubscriptionTest,
        GetNextThrottlingTimeWhenRetryIntervalsWhichIsDifferentFromRandomIntervalsIsApplied)
{
    m_pAosSubscription->SetThrottlingCount(2);

    ImsVector<IMS_SINT32> objRetryRandomIntervals;
    objRetryRandomIntervals.Add(0);
    objRetryRandomIntervals.Add(5);
    objRetryRandomIntervals.Add(15);
    objRetryRandomIntervals.Add(0);
    objRetryRandomIntervals.Add(0);
    objRetryRandomIntervals.Add(0);
    ON_CALL(m_objMockAosConfig, GetRegRandomRetryIntervals())
            .WillByDefault(ReturnRef(objRetryRandomIntervals));

    ImsVector<IMS_SINT32> objRetryIntervals;
    objRetryIntervals.Add(30);
    objRetryIntervals.Add(60);
    objRetryIntervals.Add(120);
    objRetryIntervals.Add(480);
    objRetryIntervals.Add(900);
    EXPECT_EQ(120000, m_pAosSubscription->GetNextThrottlingTime(objRetryIntervals));
}

TEST_F(AosSubscriptionTest, GetNextThrottlingTimeWhenRetryRandomIntervalshaveValuesLessThenZero)
{
    m_pAosSubscription->SetThrottlingCount(3);

    ImsVector<IMS_SINT32> objRetryRandomIntervals;
    objRetryRandomIntervals.Add(0);
    objRetryRandomIntervals.Add(5);
    objRetryRandomIntervals.Add(15);
    objRetryRandomIntervals.Add(-5);
    objRetryRandomIntervals.Add(-2);
    ON_CALL(m_objMockAosConfig, GetRegRandomRetryIntervals())
            .WillByDefault(ReturnRef(objRetryRandomIntervals));

    ImsVector<IMS_SINT32> objRetryIntervals;
    objRetryIntervals.Add(30);
    objRetryIntervals.Add(60);
    objRetryIntervals.Add(120);
    objRetryIntervals.Add(480);
    objRetryIntervals.Add(900);
    EXPECT_EQ(480000, m_pAosSubscription->GetNextThrottlingTime(objRetryIntervals));
}

/// RegSubscription_RefreshTimerExpired - CheckRadioReadyAndSetRadioWaiting() - IMS_TRUE
TEST_F(AosSubscriptionTest, SubRefreshingStateWhenTransmissionIsPossible)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    // Subscription_CanBeTransmitted() is IMS_TRUE in SetUp()

    m_pAosSubscription->RegSubscription_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHING);
    EXPECT_EQ(bDoImplicitRefresh, IMS_TRUE);
}

TEST_F(AosSubscriptionTest, OfflineStateWhenTransmissionIsNotPossible)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    ON_CALL(m_objMockIAosSubscriptionListener, Subscription_CanBeTransmitted())
            .WillByDefault(Return(IMS_FALSE));

    m_pAosSubscription->RegSubscription_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
    EXPECT_EQ(bDoImplicitRefresh, IMS_FALSE);
}

/// RegSubscription_RefreshTimerExpired - CheckRadioReadyAndSetRadioWaiting() - IMS_FALSE
TEST_F(AosSubscriptionTest, SubRefreshstopStateWhenTransmissionIsPossibleAndTransactionIsNotAllowed)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    ON_CALL(m_objMockIAosTransaction, IsTransactionAllowed(_)).WillByDefault(Return(IMS_FALSE));

    m_pAosSubscription->RegSubscription_RefreshTimerExpired(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);
    EXPECT_EQ(bDoImplicitRefresh, IMS_FALSE);
}

TEST_F(AosSubscriptionTest, SubRefreshstopStateWhenTransmissionIsPossibleAndTrafficIsNotStarted)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    ON_CALL(m_objMockIAosTransaction, StartTraffic(_, _)).WillByDefault(Return(IMS_FALSE));

    m_pAosSubscription->RegSubscription_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_TRUE(m_pAosSubscription->IsRadioWaiting());
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);
    EXPECT_EQ(bDoImplicitRefresh, IMS_FALSE);
}

/// RegSubscription_Started()
TEST_F(AosSubscriptionTest, SubscribedStateWhenStartedCalled)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBING);

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_SUBSCRIBED, AosSubscription::REASON_SUB_ESTABLISHED));

    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_STARTED, 0, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBED);
}

/// RegSubscription_StartFailed() > ProcessStartFailed_StatusCode()
TEST_F(AosSubscriptionTest, SubStopStateWhenStartFailedWithTransactionTimeout)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBING);
    // All other conditions in SetUp()

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_TRANSACTION_TIMEOUT, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);
}

/// RegSubscription_StartFailed() > ProcessStartFailed_StatusCode() bDone == IMS_TRUE
TEST_F(AosSubscriptionTest, ReturnBecauseErrProcessingIsDoneDueToInitRegRequiredWhenStartFailed)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBING);

    ON_CALL(m_objMockSipMsg, GetStatusCode()).WillByDefault(Return(403));

    // IsInitialRegistrationRequired() is IMS_TRUE
    ON_CALL(m_objMockAosConfig, GetRetryCountSubErrorRegRequired()).WillByDefault(Return(2));
    ImsVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Add(403);
    ON_CALL(m_objMockAosConfig, GetSubErrorRegRequired())
            .WillByDefault(ReturnRef(objErrRegRequired));
    m_pAosSubscription->SetRetryCountRegRequired(1);

    // ReportState() if result is true.
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_FAILED));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_FALSE));

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

/// RegSubscription_StartFailed() > ProcessStartFailed_StatusCode() > SetRetryTimer(IMS_TRUE)
TEST_F(AosSubscriptionTest, SubStopStateWhenStartFailedWithStatusCodeWithRetryAfter)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBING);

    ON_CALL(m_objMockSipMsg, GetStatusCode()).WillByDefault(Return(481));
    AString strHeader = "60";
    ON_CALL(m_objMockSipMsg, GetHeader(ISipHeader::RETRY_AFTER_SEC, 0, AString::ConstNull()))
            .WillByDefault(Return(strHeader));

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);
}

/// RegSubscription_StartFailed() > ProcessStartFailed_StatusCode()
TEST_F(AosSubscriptionTest, SubStopStateWhenStartFailedWithStatusCodeButResponseCodeIsNull)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBING);

    ON_CALL(m_objMockIRegSubscription, GetPreviousResponse()).WillByDefault(ReturnNull());

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);
}

/// RegSubscription_StartFailed() > ProcessStartFailed_Others()
TEST_F(AosSubscriptionTest, SubStopStateWhenStartFailedWithInternalError)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBING);

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_START_FAILED, IRegSubscription::REASON_INTERNAL_ERROR, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSTOP);
}

/// RegSubscription_Updated()
TEST_F(AosSubscriptionTest, SubscribedStateWhenUpdatedCalled)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHING);

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_SUBSCRIBED, AosSubscription::REASON_SUB_ESTABLISHED));

    m_pAosSubscription->NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_UPDATED, 0, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBED);
}

/// RegSubscription_UpdateFailed() > ProcessUpdateFailed_StatusCode()
TEST_F(AosSubscriptionTest, SubRefreshStopStateWhenUpdateFailedWithTransactionTimeout)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHING);
    // All other conditions in SetUp()

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_TRANSACTION_TIMEOUT, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);
}

/// RegSubscription_UpdateFailed() > ProcessUpdateFailed_StatusCode() bDone == IMS_TRUE
TEST_F(AosSubscriptionTest, ReturnBecauseErrProcessingIsDoneDueToRetryCounterWhenUpdateFailed)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHING);

    ON_CALL(m_objMockSipMsg, GetStatusCode()).WillByDefault(Return(403));

    // IsRetryActionDueToRetryCounter() - true;
    ON_CALL(m_objMockAosConfig, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosConfig, GetExtraRegErrMaxCount()).WillByDefault(Return(5));
    // AosRetryRepository::TYPE_NORMAL = 0
    ON_CALL(m_objMockAosRetryRepository, IncreaseRetryCount(0)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_StateChanged(
                    AosSubscription::STATE_OFFLINE, AosSubscription::REASON_SUB_TERMINATED));
    EXPECT_CALL(m_objMockIAosSubscriptionListener,
            Subscription_Request(AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF, 0, IMS_FALSE));

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_OFFLINE);
}

/// RegSubscription_UpdateFailed() > ProcessUpdateFailed_StatusCode() > SetRetryTimer(IMS_TRUE)
TEST_F(AosSubscriptionTest, SubRefreshStopStateWhenUpdateFailedWithStatusCodeWithRetryAfter)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHING);

    ON_CALL(m_objMockSipMsg, GetStatusCode()).WillByDefault(Return(404));

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);
}

/// RegSubscription_UpdateFailed() > ProcessUpdateFailed_StatusCode()
TEST_F(AosSubscriptionTest, SubRefreshStopStateWhenUpdateFailedWithStatusCodeButResponseCodeIsNull)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHING);

    ON_CALL(m_objMockIRegSubscription, GetPreviousResponse()).WillByDefault(ReturnNull());

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_STATUS_CODE, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);
}

/// RegSubscription_UpdateFailed() > ProcessUpdateFailed_Others()
TEST_F(AosSubscriptionTest, SubRefreshStopStateWhenUpdateFailedWithRefreshTimeout)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBREFRESHING);

    m_pAosSubscription->NotifyListenerEvent(
            AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, IRegSubscription::REASON_REFRESH_TIMEOUT, 0);
    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBREFRESHSTOP);
}

/// Transaction_OnConnectionFailed():
TEST_F(AosSubscriptionTest, ReturnImmediatelyWhenRadioIsNotWaiting)
{
    m_pAosSubscription->SetRadioWaiting(IMS_FALSE);

    m_pAosSubscription->Transaction_OnConnectionFailed(IImsRadio::REASON_NO_SERVICE, 0, 0);

    EXPECT_FALSE(m_pAosSubscription->IsRadioWaiting());
}

TEST_F(AosSubscriptionTest, RadioWaitingIsFalseWhenTransConnectionFailedWithAccessDenied)
{
    m_pAosSubscription->SetRadioWaiting(IMS_TRUE);

    m_pAosSubscription->Transaction_OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 0, 0);

    EXPECT_FALSE(m_pAosSubscription->IsRadioWaiting());
}

TEST_F(AosSubscriptionTest, RadioWaitingIsFalseWhenTransConnectionFailedWithRrcTimeoutAndWaitTime)
{
    m_pAosSubscription->SetRadioWaiting(IMS_TRUE);

    m_pAosSubscription->Transaction_OnConnectionFailed(IImsRadio::REASON_RRC_TIMEOUT, 0, 15000);

    EXPECT_FALSE(m_pAosSubscription->IsRadioWaiting());
}

/// Transaction_OnConnectionSetupPrepared()
TEST_F(AosSubscriptionTest, CallPreparedWhenTransConnectionFailedWithNoService)
{
    m_pAosSubscription->SetRadioWaiting(IMS_TRUE);

    // call Transaction_OnConnectionSetupPrepared()
    m_pAosSubscription->Transaction_OnConnectionFailed(IImsRadio::REASON_NO_SERVICE, 0, 0);

    EXPECT_FALSE(m_pAosSubscription->IsRadioWaiting());
}

/// Transaction_OnTrafficPriorityChanged()
TEST_F(AosSubscriptionTest, TrafficPriorityBlockedIsFalseWhenPriorityChangedAndPriorityIsBlocked)
{
    m_pAosSubscription->SetTrafficPriorityBlocked(IMS_TRUE);

    m_pAosSubscription->Transaction_OnTrafficPriorityChanged();

    EXPECT_FALSE(m_pAosSubscription->IsTrafficPriorityBlocked());
}

TEST_F(AosSubscriptionTest, NothingWhenPriorityChangedAndPriotyIsNotBlocked)
{
    m_pAosSubscription->SetTrafficPriorityBlocked(IMS_FALSE);

    m_pAosSubscription->Transaction_OnTrafficPriorityChanged();

    EXPECT_FALSE(m_pAosSubscription->IsTrafficPriorityBlocked());
}

/// Timer_TimerExpired()
TEST_F(AosSubscriptionTest, NothingWhenTimerIsNullInTimerExpired)
{
    IMS_UINT32 nCurrState = m_pAosSubscription->GetState();
    m_pAosSubscription->Timer_TimerExpired(IMS_NULL);

    EXPECT_EQ(m_pAosSubscription->GetState(), nCurrState);
}

/// Timer_TimerExpired() > ProcessTimerExpired()
TEST_F(AosSubscriptionTest, MaintainStateWhenInvalidStateInProcessTimerExpired)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_SUBSCRIBED);
    m_pAosSubscription->StartTimer(30);

    m_pAosSubscription->Timer_TimerExpired(m_pAosSubscription->GetTimer());

    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBED);
}

/// Timer_TimerExpired() > ProcessTimerExpired() > Start()
TEST_F(AosSubscriptionTest, SubscribingStateWhenProcessTimerExpired)
{
    m_pAosSubscription->SetState(AosSubscription::STATE_OFFLINE);
    m_pAosSubscription->StartTimer(30);

    m_pAosSubscription->Timer_TimerExpired(m_pAosSubscription->GetTimer());

    EXPECT_EQ(m_pAosSubscription->GetState(), AosSubscription::STATE_SUBSCRIBING);
}

/// StateToString()
TEST_F(AosSubscriptionTest, PrintState)
{
    EXPECT_STREQ(
            "STATE_OFFLINE", m_pAosSubscription->StateToString(AosSubscription::STATE_OFFLINE));
    EXPECT_STREQ("STATE_SUBSCRIBING",
            m_pAosSubscription->StateToString(AosSubscription::STATE_SUBSCRIBING));
    EXPECT_STREQ(
            "STATE_SUBSTOP", m_pAosSubscription->StateToString(AosSubscription::STATE_SUBSTOP));
    EXPECT_STREQ("STATE_SUBSCRIBED",
            m_pAosSubscription->StateToString(AosSubscription::STATE_SUBSCRIBED));
    EXPECT_STREQ("STATE_SUBREFRESHING",
            m_pAosSubscription->StateToString(AosSubscription::STATE_SUBREFRESHING));
    EXPECT_STREQ("STATE_SUBREFRESHSTOP",
            m_pAosSubscription->StateToString(AosSubscription::STATE_SUBREFRESHSTOP));
    EXPECT_STREQ("STATE_UNSUBSCRIBING",
            m_pAosSubscription->StateToString(AosSubscription::STATE_UNSUBSCRIBING));
    EXPECT_STREQ("__INVALID__", m_pAosSubscription->StateToString(1000));
}

/// RegSubReasonToString()
TEST_F(AosSubscriptionTest, PrintReason)
{
    EXPECT_STREQ(
            "REASON_NONE", m_pAosSubscription->RegSubReasonToString(IRegSubscription::REASON_NONE));
    EXPECT_STREQ("REASON_STATUS_CODE",
            m_pAosSubscription->RegSubReasonToString(IRegSubscription::REASON_STATUS_CODE));
    EXPECT_STREQ("REASON_NO_EXPIRES",
            m_pAosSubscription->RegSubReasonToString(IRegSubscription::REASON_NO_EXPIRES));
    EXPECT_STREQ("REASON_INTERNAL_ERROR",
            m_pAosSubscription->RegSubReasonToString(IRegSubscription::REASON_INTERNAL_ERROR));
    EXPECT_STREQ("REASON_TRANSACTION_TIMEOUT",
            m_pAosSubscription->RegSubReasonToString(IRegSubscription::REASON_TRANSACTION_TIMEOUT));
    EXPECT_STREQ("REASON_REFRESH_TIMEOUT",
            m_pAosSubscription->RegSubReasonToString(IRegSubscription::REASON_REFRESH_TIMEOUT));
    EXPECT_STREQ("REASON_REFRESH_INTERNAL_ERROR",
            m_pAosSubscription->RegSubReasonToString(
                    IRegSubscription::REASON_REFRESH_INTERNAL_ERROR));
    EXPECT_STREQ("REASON_NOTIFY_TERMINATED",
            m_pAosSubscription->RegSubReasonToString(IRegSubscription::REASON_NOTIFY_TERMINATED));
    EXPECT_STREQ("__INVALID__", m_pAosSubscription->RegSubReasonToString(37));
}

/// RegInfoStateToString()
TEST_F(AosSubscriptionTest, PrintNotifyState)
{
    EXPECT_STREQ("STATE_CREATED",
            m_pAosSubscription->RegInfoStateToString(IRegInfoContact::STATE_CREATED));
    EXPECT_STREQ("STATE_ACTIVE",
            m_pAosSubscription->RegInfoStateToString(IRegInfoContact::STATE_ACTIVE));
    EXPECT_STREQ("STATE_TERMINATED",
            m_pAosSubscription->RegInfoStateToString(IRegInfoContact::STATE_TERMINATED));
    EXPECT_STREQ("__INVALID__", m_pAosSubscription->RegInfoStateToString(50));
}

/// RegInfoEventToString()
TEST_F(AosSubscriptionTest, PrintRegInfoEvent)
{
    EXPECT_STREQ("EVENT_REGISTERED",
            m_pAosSubscription->RegInfoEventToString(IRegInfoContact::EVENT_REGISTERED));
    EXPECT_STREQ("EVENT_CREATED",
            m_pAosSubscription->RegInfoEventToString(IRegInfoContact::EVENT_CREATED));
    EXPECT_STREQ("EVENT_REFRESHED",
            m_pAosSubscription->RegInfoEventToString(IRegInfoContact::EVENT_REFRESHED));
    EXPECT_STREQ("EVENT_SHORTENED",
            m_pAosSubscription->RegInfoEventToString(IRegInfoContact::EVENT_SHORTENED));
    EXPECT_STREQ("EVENT_EXPIRED",
            m_pAosSubscription->RegInfoEventToString(IRegInfoContact::EVENT_EXPIRED));
    EXPECT_STREQ("EVENT_DEACTIVATED",
            m_pAosSubscription->RegInfoEventToString(IRegInfoContact::EVENT_DEACTIVATED));
    EXPECT_STREQ("EVENT_PROBATION",
            m_pAosSubscription->RegInfoEventToString(IRegInfoContact::EVENT_PROBATION));
    EXPECT_STREQ("EVENT_UNREGISTERED",
            m_pAosSubscription->RegInfoEventToString(IRegInfoContact::EVENT_UNREGISTERED));
    EXPECT_STREQ("EVENT_REJECTED",
            m_pAosSubscription->RegInfoEventToString(IRegInfoContact::EVENT_REJECTED));
    EXPECT_STREQ("__INVALID__", m_pAosSubscription->RegInfoEventToString(90));
}