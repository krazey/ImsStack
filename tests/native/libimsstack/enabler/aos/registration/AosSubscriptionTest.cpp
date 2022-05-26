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
#include "interface/MockIAosSubscriptionListener.h"
#include "../../../engine/interface/registration/MockIRegInfo.h"
#include "../../../engine/interface/registration/MockIRegInfoContact.h"
#include "../../../engine/interface/registration/MockIRegInfoRegistration.h"
#include "../../../engine/interface/registration/MockIRegSubscription.h"

#include "IRegContact.h"
#include "IRegSubscription.h"
#include "SipAddress.h"
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
    MockIRegSubscription objMockIRegSubscription;
    AString* pAor;
    SipAddress* pContactAddress;

    MockIAosSubscriptionListener objMockIAosSubscriptionListener;

    // for AosNConfig info
    IAosNConfiguration* pOriginAosNConfiguration;
    MockIAosNConfiguration objMockAosConfig;

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

        pAosSubscription = new AosSubscription(static_cast<IAosAppContext*>(pMockAosAppContext),
                static_cast<IRegSubscription*>(&objMockIRegSubscription), *pAor, *pContactAddress);
        ASSERT_TRUE(pAosSubscription != nullptr);

        pAosSubscription->SetListener(&objMockIAosSubscriptionListener);
        EXPECT_CALL(objMockIAosSubscriptionListener, Subscription_StateChanged(_, _))
                .WillRepeatedly(Return());

        pOriginAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&objMockAosConfig), SLOT_ID);
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(pOriginAosNConfiguration, SLOT_ID);

        if (pAor)
        {
            delete pAor;
        }

        if (pContactAddress)
        {
            delete pContactAddress;
        }

        if (pAosStaticProfile)
        {
            delete pAosStaticProfile;
        }

        if (pMockAosAppContext)
        {
            delete pMockAosAppContext;
        }

        if (pAosSubscription)
        {
            pAosSubscription->SetListener(IMS_NULL);
            delete pAosSubscription;
        }
    }

    void SetState(IN IMS_SINT32 nState) { pAosSubscription->m_nState = nState; }

    void SetpiRegSubscription(IN IRegSubscription* piRegSubscription)
    {
        pAosSubscription->m_piRegSubscription = piRegSubscription;
    }

    void SetRetryCountRegRequired(IN IMS_UINT32 nRetryCount)
    {
        pAosSubscription->m_nRetryCountRegRequired = nRetryCount;
    }

    void SetObjContactAddress(IN SipAddress objContactAddress)
    {
        pAosSubscription->m_objContactAddress = objContactAddress;
    }

    IMS_SINT32 GetAorState() { return pAosSubscription->m_nAorState; }

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
};

TEST_F(AosSubscriptionTest, aosSubscriptionStart)
{
    SetState(AosSubscription::STATE_UNSUBSCRIBING);
    EXPECT_FALSE(pAosSubscription->Start());

    SetState(AosSubscription::STATE_OFFLINE);

    SetpiRegSubscription(IMS_NULL);
    EXPECT_FALSE(pAosSubscription->Start());

    SetState(AosSubscription::STATE_SUBSCRIBED);
    SetpiRegSubscription(static_cast<IRegSubscription*>(&objMockIRegSubscription));
    EXPECT_CALL(objMockIRegSubscription, Subscribe())
            .WillOnce(Return(IMS_FAILURE))
            .WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_FALSE(pAosSubscription->Start());
    EXPECT_TRUE(pAosSubscription->Start());
}

TEST_F(AosSubscriptionTest, checkInitialRegRequired)
{
    EXPECT_CALL(objMockAosConfig, GetRetryCountSubErrorRegRequired())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(2));

    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequired(403));

    IMSVector<IMS_SINT32> objErrRegRequired;
    objErrRegRequired.Clear();
    objErrRegRequired.Add(404);
    objErrRegRequired.Add(403);

    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequired));

    SetRetryCountRegRequired(0);
    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequired(403));

    SetRetryCountRegRequired(1);
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationRequired(403));

    objErrRegRequired.Clear();
    objErrRegRequired.Add(5);
    objErrRegRequired.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequired));

    SetRetryCountRegRequired(1);
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationRequired(504));
}

TEST_F(AosSubscriptionTest, checkInitialRegWithNextPcscfRequired)
{
    IMSVector<IMS_SINT32> objErrRegRequiredWithNextPcscf;
    objErrRegRequiredWithNextPcscf.Clear();
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));

    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(403));

    objErrRegRequiredWithNextPcscf.Add(404);
    objErrRegRequiredWithNextPcscf.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(403));

    objErrRegRequiredWithNextPcscf.Clear();
    objErrRegRequiredWithNextPcscf.Add(6);
    objErrRegRequiredWithNextPcscf.Add(403);
    EXPECT_CALL(objMockAosConfig, GetSubErrorRegRequiredWithNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredWithNextPcscf));
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationWithNextPcscfRequired(603));
}

TEST_F(AosSubscriptionTest, checkInitialRegRequiredInWifi)
{
    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(*pMockAosAppContext, GetConnection())
            .Times(4)
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));

    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(4)
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequiredInWifi(403));

    IMSVector<IMS_SINT32> objErrRegRequiredInWifi;
    objErrRegRequiredInWifi.Clear();
    EXPECT_CALL(objMockAosConfig, GetVowifiSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredInWifi));

    EXPECT_FALSE(pAosSubscription->IsInitialRegistrationRequiredInWifi(403));

    objErrRegRequiredInWifi.Add(404);
    objErrRegRequiredInWifi.Add(403);
    EXPECT_CALL(objMockAosConfig, GetVowifiSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredInWifi));
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationRequiredInWifi(403));

    objErrRegRequiredInWifi.Clear();
    objErrRegRequiredInWifi.Add(3);
    objErrRegRequiredInWifi.Add(403);
    EXPECT_CALL(objMockAosConfig, GetVowifiSubErrorRegRequired())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrRegRequiredInWifi));
    EXPECT_TRUE(pAosSubscription->IsInitialRegistrationRequiredInWifi(301));
}

TEST_F(AosSubscriptionTest, checkIsReSubscriptionStopped)
{
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

TEST_F(AosSubscriptionTest, checkNotifyReceived)
{
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
    EXPECT_CALL(objMockIRegInfoContact1, GetURI())
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
    EXPECT_CALL(objMockIRegInfoContact2, GetURI())
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
            .WillOnce(Return(IRegInfoContact::EVENT_EXPIRED))  // 6
            .WillOnce(Return(IRegInfoContact::EVENT_DEACTIVATED))
            .WillOnce(Return(IRegInfoContact::EVENT_DEACTIVATED))
            .WillOnce(Return(IRegInfoContact::EVENT_PROBATION))
            .WillOnce(Return(IRegInfoContact::EVENT_PROBATION))
            .WillOnce(Return(IRegInfoContact::EVENT_UNREGISTERED))
            .WillOnce(Return(IRegInfoContact::EVENT_UNREGISTERED))
            .WillOnce(Return(IRegInfoContact::EVENT_REJECTED))
            .WillOnce(Return(IRegInfoContact::EVENT_REJECTED));

    EXPECT_CALL(objMockIRegInfoContact3, GetURI())
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

    NotifyListenerEvent(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, IMS_TRUE);
    EXPECT_EQ(GetAorState(), IRegInfoContact::STATE_TERMINATED);

    EXPECT_CALL(objMockAosConfig, GetNotifyWaitTime()).Times(2).WillOnce(Return(60));

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