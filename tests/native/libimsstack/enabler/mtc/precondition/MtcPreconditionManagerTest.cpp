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

#include "IMessage.h"
#include "INetworkWatcher.h"
#include "ImsEventDef.h"
#include "MediaDef.h"
#include "MockIMessage.h"
#include "MockIMtcImsEventReceiver.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MockISipMessage.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/UpdatingInfo.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "media/IMedia.h"
#include "media/MockIMedia.h"
#include "media/MockIMediaDescriptor.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionListener.h"
#include "precondition/MockIQosTimerListener.h"
#include "precondition/MockQosInfo.h"
#include "precondition/MockQosStatusTable.h"
#include "precondition/MockQosTimer.h"
#include "precondition/MockSdpPreconditionHelper.h"
#include "precondition/MtcPreconditionManager.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::AnyOf;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL CallKey CALL_KEY = 1;

class TestMtcPreconditionManager : public MtcPreconditionManager
{
public:
    inline explicit TestMtcPreconditionManager(IN IMtcCallContext& objContext) :
            MtcPreconditionManager(objContext)
    {
    }

    inline void ReplaceSdpPreconditionHelper(IN SdpPreconditionHelper* pSdpPreconditionHelper)
    {
        delete m_pSdpPreconditionHelper;
        m_pSdpPreconditionHelper = pSdpPreconditionHelper;
    }

    inline void ReplaceQosInfo(IN ISession* piSession, IN QosInfo* pQosInfo)
    {
        if (m_objQosInfos.GetIndexOfKey(piSession) >= 0)
        {
            QosInfo* pInfo = m_objQosInfos.GetValue(piSession);
            if (pInfo != IMS_NULL)
            {
                delete pInfo;
            }
        }
        m_objQosInfos.SetValue(piSession, pQosInfo);
    }

    inline void SetOnWlanForPrerequisite(IN IMS_BOOL bOnWlan) { m_bOnWlan = bOnWlan; }

    inline void SetCurrentRatTypeForPrerequisite(IN IMS_SINT32 eRatType)
    {
        m_ePreviousRatType = m_eCurrentRatType;
        m_eCurrentRatType = eRatType;
    }

    inline QosInfo* GetQosInfoPublic(IN ISession* piSession) const
    {
        return MtcPreconditionManager::GetQosInfo(piSession);
    }

    inline IMS_BOOL IsEpsFallbackPublic() const { return MtcPreconditionManager::IsEpsFallback(); }
};

class MtcPreconditionManagerTest : public ::testing::Test
{
public:
    inline MtcPreconditionManagerTest() :
            objListener(),
            objTimerListener(),
            pInfo(IMS_NULL),
            objTimer(&objTimerListener),
            objStatusTable(),
            pSdpPreconditionHelper(new MockSdpPreconditionHelper()),
            objIMessage(),
            objISipMessage(),
            objMediaDescriptor(),
            objCallContext(),
            objService(),
            objCallInfo(),
            objMessageUtils(),
            objMediaManager(),
            pConfigurationProxy(new MockMtcConfigurationProxy()),
            objSession(),
            objISession(),
            objUpdatingInfo(objCallContext),
            pPreconditionManager(IMS_NULL)
    {
    }

public:
    MockIMtcPreconditionListener objListener;
    MockIQosTimerListener objTimerListener;
    MockQosInfo* pInfo;
    MockQosTimer objTimer;
    MockQosStatusTable objStatusTable;
    MockSdpPreconditionHelper* pSdpPreconditionHelper;
    MockIMessage objIMessage;
    MockISipMessage objISipMessage;
    MockIMediaDescriptor objMediaDescriptor;
    MockIMtcCallContext objCallContext;
    MockIMtcService objService;
    CallInfo objCallInfo;
    MockIMessageUtils objMessageUtils;
    MockIMtcMediaManager objMediaManager;
    MockIMtcImsEventReceiver objImsEventReceiver;
    MockMtcConfigurationProxy* pConfigurationProxy;
    MockIMtcSession objSession;
    MockISession objISession;
    UpdatingInfo objUpdatingInfo;
    TestMtcPreconditionManager* pPreconditionManager;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objIMessage, GetMessage()).WillByDefault(Return(&objISipMessage));
        ON_CALL(objCallContext, GetService()).WillByDefault(ReturnRef(objService));
        ON_CALL(objCallContext, GetMessageUtils()).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objCallContext, GetMediaManager()).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objCallContext, GetImsEventReceiver).WillByDefault(ReturnRef(objImsEventReceiver));
        ON_CALL(objCallContext, GetConfigurationProxy())
                .WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objSession));
        ON_CALL(objCallContext, GetCallKey()).WillByDefault(Return(CALL_KEY));
        ON_CALL(objCallContext, GetUpdatingInfo).WillByDefault(ReturnRef(objUpdatingInfo));

        CreateMtcPreconditionManager();
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pPreconditionManager;
    }

    void CreateMtcPreconditionManager()
    {
        pPreconditionManager = new TestMtcPreconditionManager(objCallContext);
        pPreconditionManager->ReplaceSdpPreconditionHelper(pSdpPreconditionHelper);
        pPreconditionManager->SetListener(&objListener);
    }

    void SetUpMockQosInfo()
    {
        pInfo = new MockQosInfo(&objTimerListener);
        pPreconditionManager->ReplaceQosInfo(&objISession, pInfo);
        ON_CALL(*pInfo, GetTimer()).WillByDefault(ReturnRef(objTimer));
        ON_CALL(*pInfo, GetStatusTable()).WillByDefault(ReturnRef(objStatusTable));
    }

    void SetUpSupportingPreconditionInLocal(IN CallType eType, IN IMS_BOOL bSupported)
    {
        ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objSession, GetCallType()).WillByDefault(Return(eType));

        if (bSupported)
        {
            ON_CALL(*pConfigurationProxy,
                    GetBoolean(ConfigVoice::KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL))
                    .WillByDefault(Return(IMS_TRUE));
        }
        else
        {
            ON_CALL(*pConfigurationProxy,
                    GetBoolean(ConfigVoice::KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL))
                    .WillByDefault(Return(IMS_FALSE));
            ON_CALL(*pConfigurationProxy,
                    GetBoolean(ConfigVt::KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL))
                    .WillByDefault(Return(IMS_FALSE));
            ON_CALL(*pConfigurationProxy,
                    GetBoolean(ConfigRtt::KEY_TEXT_QOS_PRECONDITION_SUPPORTED_BOOL))
                    .WillByDefault(Return(IMS_FALSE));
        }
    }

    void SetUpNothingOnDefaultBearerSupported()
    {
        ON_CALL(*pConfigurationProxy,
                GetBoolean(ConfigVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(*pConfigurationProxy,
                GetBoolean(ConfigVt::KEY_VIDEO_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(*pConfigurationProxy,
                GetBoolean(ConfigRtt::KEY_TEXT_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
                .WillByDefault(Return(IMS_FALSE));
    }
};

TEST_F(MtcPreconditionManagerTest, CreateQosInfoForTheFirstTime)
{
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    pPreconditionManager->CreateQos(&objISession);
    EXPECT_NE(pPreconditionManager->GetQosInfoPublic(&objISession), nullptr);
}

TEST_F(MtcPreconditionManagerTest, DoNotCreateQosInfoIfAlreadyCreated)
{
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);

    pPreconditionManager->CreateQos(&objISession);
    const QosInfo* pInfo = pPreconditionManager->GetQosInfoPublic(&objISession);
    EXPECT_NE(pInfo, nullptr);

    pPreconditionManager->CreateQos(&objISession);
    EXPECT_EQ(pInfo, pPreconditionManager->GetQosInfoPublic(&objISession));
}

TEST_F(MtcPreconditionManagerTest, DestroyQos)
{
    pPreconditionManager->DestroyQos(&objISession);

    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    pPreconditionManager->CreateQos(&objISession);
    EXPECT_NE(pPreconditionManager->GetQosInfoPublic(&objISession), nullptr);

    pPreconditionManager->DestroyQos(&objISession);
    EXPECT_EQ(pPreconditionManager->GetQosInfoPublic(&objISession), nullptr);
}

TEST_F(MtcPreconditionManagerTest, IsPreconditionSupportedInLocalReturnsFalseInCaseOfUssi)
{
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_FALSE);
    EXPECT_FALSE(pPreconditionManager->IsPreconditionSupportedInLocal());
}

TEST_F(MtcPreconditionManagerTest, IsPreconditionSupportedInLocalInCaseOfEmergency)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    EXPECT_CALL(*pConfigurationProxy,
            GetBoolean(ConfigEmergency::KEY_EMERGENCY_QOS_PRECONDITION_SUPPORTED_BOOL))
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_TRUE(pPreconditionManager->IsPreconditionSupportedInLocal());
    EXPECT_FALSE(pPreconditionManager->IsPreconditionSupportedInLocal());
}

TEST_F(MtcPreconditionManagerTest, IsPreconditionSupportedInLocalInCaseOfVideoCall)
{
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL))
            .Times(3)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(
            *pConfigurationProxy, GetBoolean(ConfigVt::KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL))
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_TRUE(pPreconditionManager->IsPreconditionSupportedInLocal());
    EXPECT_TRUE(pPreconditionManager->IsPreconditionSupportedInLocal());
    EXPECT_FALSE(pPreconditionManager->IsPreconditionSupportedInLocal());
}

TEST_F(MtcPreconditionManagerTest, IsPreconditionSupportedInLocalInCaseOfRttCall)
{
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::RTT));

    EXPECT_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL))
            .Times(3)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(
            *pConfigurationProxy, GetBoolean(ConfigRtt::KEY_TEXT_QOS_PRECONDITION_SUPPORTED_BOOL))
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_TRUE(pPreconditionManager->IsPreconditionSupportedInLocal());
    EXPECT_TRUE(pPreconditionManager->IsPreconditionSupportedInLocal());
    EXPECT_FALSE(pPreconditionManager->IsPreconditionSupportedInLocal());
}

TEST_F(MtcPreconditionManagerTest, IsPreconditionSupportedInLocalInCaseOfVideoRttCall)
{
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VIDEO_RTT));

    EXPECT_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL))
            .Times(4)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(
            *pConfigurationProxy, GetBoolean(ConfigVt::KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL))
            .Times(3)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(
            *pConfigurationProxy, GetBoolean(ConfigRtt::KEY_TEXT_QOS_PRECONDITION_SUPPORTED_BOOL))
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_TRUE(pPreconditionManager->IsPreconditionSupportedInLocal());
    EXPECT_TRUE(pPreconditionManager->IsPreconditionSupportedInLocal());
    EXPECT_TRUE(pPreconditionManager->IsPreconditionSupportedInLocal());
    EXPECT_FALSE(pPreconditionManager->IsPreconditionSupportedInLocal());
}

TEST_F(MtcPreconditionManagerTest, IsPreconditionSupportedInLocalInCaseOfVoiceCall)
{
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL))
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_TRUE(pPreconditionManager->IsPreconditionSupportedInLocal());
    EXPECT_FALSE(pPreconditionManager->IsPreconditionSupportedInLocal());
}

TEST_F(MtcPreconditionManagerTest, IsDedicatedBearerAllocatedReturnsTrueIfQosStatusIsAvailable)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(*pInfo, GetTextStatus()).WillByDefault(Return(QosStatus::AVAILABLE));

    EXPECT_TRUE(pPreconditionManager->IsDedicatedBearerAllocated(&objISession, MEDIATYPE_AUDIO));
    EXPECT_TRUE(pPreconditionManager->IsDedicatedBearerAllocated(&objISession, MEDIATYPE_VIDEO));
    EXPECT_TRUE(pPreconditionManager->IsDedicatedBearerAllocated(&objISession, MEDIATYPE_TEXT));
}

TEST_F(MtcPreconditionManagerTest, IsDedicatedBearerAllocatedReturnsFalseIfQosStatusIsNotAvailable)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetTextStatus()).WillByDefault(Return(QosStatus::IDLE));

    EXPECT_FALSE(pPreconditionManager->IsDedicatedBearerAllocated(&objISession, MEDIATYPE_AUDIO));
    EXPECT_FALSE(pPreconditionManager->IsDedicatedBearerAllocated(&objISession, MEDIATYPE_VIDEO));
    EXPECT_FALSE(pPreconditionManager->IsDedicatedBearerAllocated(&objISession, MEDIATYPE_TEXT));
}

TEST_F(MtcPreconditionManagerTest,
        IsCheckingResourcesRequiredToAlertUserIfLocalPreconditionIsSupported)
{
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_WAIT_QOS_WHEN_LOCAL_PRECONDITION_NOT_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_TRUE(pPreconditionManager->IsCheckingResourcesRequiredToAlertUser());

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_WAIT_QOS_WHEN_LOCAL_PRECONDITION_NOT_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(pPreconditionManager->IsCheckingResourcesRequiredToAlertUser());
}

TEST_F(MtcPreconditionManagerTest,
        IsCheckingResourcesRequiredToAlertUserIfLocalPreconditionIsNotSupported)
{
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_FALSE);
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_WAIT_QOS_WHEN_LOCAL_PRECONDITION_NOT_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_FALSE(pPreconditionManager->IsCheckingResourcesRequiredToAlertUser());

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_WAIT_QOS_WHEN_LOCAL_PRECONDITION_NOT_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(pPreconditionManager->IsCheckingResourcesRequiredToAlertUser());
}

TEST_F(MtcPreconditionManagerTest,
        IsCheckingResourcesRequiredToAlertUserReturnsFalseIfSupportsAudioDefaultBearer)
{
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_WAIT_QOS_WHEN_LOCAL_PRECONDITION_NOT_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_FALSE(pPreconditionManager->IsCheckingResourcesRequiredToAlertUser());
}

TEST_F(MtcPreconditionManagerTest,
        IsCheckingResourcesRequiredToAlertUserReturnsTrueIfSupportsAudioDefaultBearerButRoaming)
{
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_WAIT_QOS_WHEN_LOCAL_PRECONDITION_NOT_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsRoaming()).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(pPreconditionManager->IsCheckingResourcesRequiredToAlertUser());
}

TEST_F(MtcPreconditionManagerTest,
        IsCheckingResourcesRequiredToAlertUserReturnsTrueIfNotSupportsAudioDefaultBearer)
{
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_WAIT_QOS_WHEN_LOCAL_PRECONDITION_NOT_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_TRUE(pPreconditionManager->IsCheckingResourcesRequiredToAlertUser());
}

TEST_F(MtcPreconditionManagerTest, IsAvailableToAlertUserReturnsFalseIfSessionIsNull)
{
    EXPECT_FALSE(pPreconditionManager->IsAvailableToAlertUser(nullptr));
}

TEST_F(MtcPreconditionManagerTest, IsAvailableToAlertUserReturnsTrueIfDedicatedBearerIsAllocated)
{
    SetUpMockQosInfo();
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));

    EXPECT_TRUE(pPreconditionManager->IsAvailableToAlertUser(&objISession));
}

TEST_F(MtcPreconditionManagerTest,
        IsAvailableToAlertUserReturnsFalseIfDedicatedBearerIsNotAllocated)
{
    SetUpMockQosInfo();
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_FALSE(pPreconditionManager->IsAvailableToAlertUser(&objISession));
}

TEST_F(MtcPreconditionManagerTest, IsAvailableToAlertUserReturnsTrueIfRemoteReserved)
{
    SetUpMockQosInfo();

    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objStatusTable, IsCurrentStatusEnabled(_, SdpPrecondition::STATUS_REMOTE))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(pPreconditionManager->IsAvailableToAlertUser(&objISession));
}

TEST_F(MtcPreconditionManagerTest,
        IsAvailableToAlertUserReturnsFalseIfRemoteNotReservedWhenMediaTypeIsNone)
{
    SetUpMockQosInfo();

    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objStatusTable, IsCurrentStatusEnabled(_, SdpPrecondition::STATUS_REMOTE))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_TRUE));

    // For coverage, not a real case
    EXPECT_CALL(objSession, GetCallType())
            .Times(2)
            .WillOnce(Return(CallType::VOIP))      // IsLocalResourceReserved
            .WillOnce(Return(CallType::UNKNOWN));  // IsRemoteResourceReserved
    EXPECT_FALSE(pPreconditionManager->IsAvailableToAlertUser(&objISession));
}

TEST_F(MtcPreconditionManagerTest, IsAvailableToAlertUserReturnsFalseIfRemoteNotReserved)
{
    SetUpMockQosInfo();

    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objStatusTable,
            IsCurrentStatusEnabled(SdpMedia::TYPE_AUDIO, SdpPrecondition::STATUS_REMOTE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objStatusTable,
            IsCurrentStatusEnabled(SdpMedia::TYPE_VIDEO, SdpPrecondition::STATUS_REMOTE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_FALSE(pPreconditionManager->IsAvailableToAlertUser(&objISession));
}

TEST_F(MtcPreconditionManagerTest,
        IsAvailableToAlertUserReturnsFalseIfOnlyAudioIsReservedForVtUpgrade)
{
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    SetUpMockQosInfo();
    SetUpNothingOnDefaultBearerSupported();
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVt::KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    // local
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER))
            .WillByDefault(Return(IMS_FALSE));

    // remote is not reached.

    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));

    EXPECT_FALSE(pPreconditionManager->IsAvailableToAlertUser(&objISession));
}

TEST_F(MtcPreconditionManagerTest,
        IsAvailableToAlertUserReturnsTrueIfVoiceAndVideoAreReservedForVtUpgrade)
{
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    SetUpMockQosInfo();
    SetUpNothingOnDefaultBearerSupported();
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVt::KEY_VIDEO_QOS_PRECONDITION_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    // local
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::AVAILABLE));

    // remote
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(objStatusTable,
            IsCurrentStatusEnabled(
                    AnyOf(SdpMedia::TYPE_AUDIO, SdpMedia::TYPE_VIDEO, SdpMedia::TYPE_TEXT),
                    SdpPrecondition::STATUS_REMOTE))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(pPreconditionManager->IsAvailableToAlertUser(&objISession));
}

TEST_F(MtcPreconditionManagerTest,
        IsLocalResourceConfirmationRequiredReturnsFalseIfPreconditionIsNotSupported)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_FALSE));
    EXPECT_FALSE(pPreconditionManager->IsLocalResourceConfirmationRequired(&objISession));
}

TEST_F(MtcPreconditionManagerTest,
        IsLocalResourceConfirmationRequiredReturnsFalseIfLocalResourceIsAlreadyConfirmed)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objStatusTable, IsLocalResourceConfirmed(SdpMedia::TYPE_AUDIO))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_FALSE(pPreconditionManager->IsLocalResourceConfirmationRequired(&objISession));
}

TEST_F(MtcPreconditionManagerTest, IsLocalResourceConfirmationRequiredReturnsTrue)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(objStatusTable, IsLocalResourceConfirmed(SdpMedia::TYPE_AUDIO))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objStatusTable, IsLocalResourceConfirmed(SdpMedia::TYPE_VIDEO))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    SetUpNothingOnDefaultBearerSupported();

    EXPECT_TRUE(pPreconditionManager->IsLocalResourceConfirmationRequired(&objISession));
}

TEST_F(MtcPreconditionManagerTest,
        IsAvailableToSendLocalResourceConfirmationReturnsFalseIfLocalResourceIsNotReserved)
{
    SetUpMockQosInfo();
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetTextStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();

    EXPECT_CALL(objSession, GetCallType())
            .Times(2)
            .WillOnce(Return(CallType::UNKNOWN))
            .WillOnce(Return(CallType::VIDEO_RTT));
    EXPECT_FALSE(pPreconditionManager->IsAvailableToSendLocalResourceConfirmation(&objISession));
    EXPECT_FALSE(pPreconditionManager->IsAvailableToSendLocalResourceConfirmation(&objISession));
}

TEST_F(MtcPreconditionManagerTest,
        IsAvailableToSendLocalResourceConfirmationReturnsTrueIfLocalResourceIsReserved)
{
    SetUpMockQosInfo();
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHING));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VIDEO_RTT));
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    SetUpNothingOnDefaultBearerSupported();

    // In early-dialog, bAtLeastOneReserved is true so it returns true even if only one media is
    // available.
    EXPECT_CALL(*pInfo, GetAudioStatus())
            .Times(3)
            .WillOnce(Return(QosStatus::AVAILABLE))
            .WillRepeatedly(Return(QosStatus::IDLE));
    EXPECT_CALL(*pInfo, GetVideoStatus())
            .Times(2)
            .WillOnce(Return(QosStatus::AVAILABLE))
            .WillOnce(Return(QosStatus::IDLE));
    EXPECT_CALL(*pInfo, GetTextStatus()).Times(1).WillOnce(Return(QosStatus::AVAILABLE));
    // by Audio
    EXPECT_TRUE(pPreconditionManager->IsAvailableToSendLocalResourceConfirmation(&objISession));
    // by Video
    EXPECT_TRUE(pPreconditionManager->IsAvailableToSendLocalResourceConfirmation(&objISession));
    // by Text
    EXPECT_TRUE(pPreconditionManager->IsAvailableToSendLocalResourceConfirmation(&objISession));
}

TEST_F(MtcPreconditionManagerTest,
        IsAvailableToSendLocalResourceConfirmationReturnsFalseIfWaitingNonAudioMediaQoSAvailable)
{
    SetUpMockQosInfo();
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHING));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VIDEO_RTT));
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    SetUpNothingOnDefaultBearerSupported();

    EXPECT_CALL(*pInfo, GetAudioStatus())
            .Times(3)
            .WillOnce(Return(QosStatus::AVAILABLE))
            .WillRepeatedly(Return(QosStatus::IDLE));
    EXPECT_CALL(*pInfo, GetVideoStatus())
            .Times(2)
            .WillOnce(Return(QosStatus::AVAILABLE))
            .WillOnce(Return(QosStatus::IDLE));
    EXPECT_CALL(*pInfo, GetTextStatus()).Times(1).WillOnce(Return(QosStatus::AVAILABLE));
    EXPECT_FALSE(pPreconditionManager->IsAvailableToSendLocalResourceConfirmation(&objISession));
    EXPECT_FALSE(pPreconditionManager->IsAvailableToSendLocalResourceConfirmation(&objISession));
    EXPECT_FALSE(pPreconditionManager->IsAvailableToSendLocalResourceConfirmation(&objISession));
}

TEST_F(MtcPreconditionManagerTest,
        IsAvailableToSendLocalResourceConfirmationReturnsTrueIfDefaultBearerIsUsed)
{
    SetUpMockQosInfo();
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHING));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VIDEO_RTT));
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetTextStatus()).WillByDefault(Return(QosStatus::IDLE));

    EXPECT_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
            .Times(3)
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(
            *pConfigurationProxy, GetBoolean(ConfigVt::KEY_VIDEO_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(
            *pConfigurationProxy, GetBoolean(ConfigRtt::KEY_TEXT_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(pPreconditionManager->IsAvailableToSendLocalResourceConfirmation(&objISession));
    EXPECT_TRUE(pPreconditionManager->IsAvailableToSendLocalResourceConfirmation(&objISession));
    EXPECT_TRUE(pPreconditionManager->IsAvailableToSendLocalResourceConfirmation(&objISession));
}

TEST_F(MtcPreconditionManagerTest, RemovePreconditionSdpWhenQosInfoDoesNotExist)
{
    EXPECT_CALL(*pSdpPreconditionHelper, RemovePreconditionSdp(&objISession));
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest, RemovePreconditionSdpWhenPreconditionIsNotSupported)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(*pSdpPreconditionHelper, RemovePreconditionSdp(&objISession));
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest, FormFailurePreconditionSdp)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(*pSdpPreconditionHelper, FormFailurePreconditionSdp(&objISession));
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_TRUE);
}

TEST_F(MtcPreconditionManagerTest,
        FormPreconditionSdpIncludingConfirmationStatusBeforeDialogConfirmed)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ImsList<IMedia*> lstMedias;
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));
    objCallInfo.ePeerType = PeerType::MT;
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHING));

    EXPECT_CALL(
            *pSdpPreconditionHelper, FormPreconditionSdp(&objISession, &objStatusTable, IMS_TRUE))
            .Times(1);
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest,
        FormPreconditionSdpIncludingConfirmationStatusAfterDialogConfirmed)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ImsList<IMedia*> lstMedias;
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));
    objCallInfo.ePeerType = PeerType::MT;
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    ON_CALL(objISession, GetPreviousResponse(IMessage::SESSION_UPDATE))
            .WillByDefault(Return(nullptr));

    EXPECT_CALL(
            *pSdpPreconditionHelper, FormPreconditionSdp(&objISession, &objStatusTable, IMS_TRUE))
            .Times(1);

    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest, FormPreconditionSdpNotIncludingConfirmationStatus)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ImsList<IMedia*> lstMedias;
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));

    EXPECT_CALL(objCallContext, GetCallInfo()).Times(1).WillOnce(ReturnRef(objCallInfo));
    EXPECT_CALL(objISession, GetState())
            .Times(2)
            .WillOnce(Return(ISession::STATE_ESTABLISHED))
            .WillOnce(Return(ISession::STATE_ESTABLISHING));
    EXPECT_CALL(
            *pSdpPreconditionHelper, FormPreconditionSdp(&objISession, &objStatusTable, IMS_FALSE))
            .Times(2);

    objUpdatingInfo.SetModifier();
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest, FormPreconditionSdpIfIMediaIsNull)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ImsList<IMedia*> lstMedias;
    lstMedias.Append(nullptr);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHING));

    EXPECT_CALL(objStatusTable, InitializeRecords(_)).Times(0);
    EXPECT_CALL(
            *pSdpPreconditionHelper, FormPreconditionSdp(&objISession, &objStatusTable, IMS_FALSE))
            .Times(1);
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest, FormPreconditionSdpIfMediaStateIsDeleted)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ImsList<IMedia*> lstMedias;
    MockIMedia objAudioMedia;
    lstMedias.Append(&objAudioMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));
    ON_CALL(objAudioMedia, GetUpdateState()).WillByDefault(Return(IMedia::UPDATE_MODIFIED));
    ON_CALL(objAudioMedia, GetProposal()).WillByDefault(Return(&objAudioMedia));
    ON_CALL(objAudioMedia, GetMediaDescriptor()).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objAudioMedia, GetState()).WillByDefault(Return(IMedia::STATE_DELETED));
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHING));

    EXPECT_CALL(objStatusTable, InitializeRecords(_)).Times(0);
    EXPECT_CALL(
            *pSdpPreconditionHelper, FormPreconditionSdp(&objISession, &objStatusTable, IMS_FALSE))
            .Times(1);
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest, FormPreconditionSdpIfPortOfLocalSdpIsZero)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ImsList<IMedia*> lstMedias;
    MockIMedia objAudioMedia;
    lstMedias.Append(&objAudioMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));
    ON_CALL(objAudioMedia, GetUpdateState()).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objAudioMedia, GetMediaDescriptor()).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objAudioMedia, GetState()).WillByDefault(Return(IMedia::STATE_ACTIVE));
    SdpMedia objSdpMedia;
    ON_CALL(objMediaDescriptor, GetMediaDescriptionExAsLocal()).WillByDefault(Return(&objSdpMedia));
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHING));

    EXPECT_CALL(objStatusTable, InitializeRecords(_)).Times(0);
    EXPECT_CALL(
            *pSdpPreconditionHelper, FormPreconditionSdp(&objISession, &objStatusTable, IMS_FALSE))
            .Times(1);
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest, FormPreconditionSdpIfRecordsListAlreadyExist)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ImsList<IMedia*> lstMedias;
    MockIMedia objAudioMedia;
    lstMedias.Append(&objAudioMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));
    ON_CALL(objAudioMedia, GetUpdateState()).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objAudioMedia, GetMediaDescriptor()).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objAudioMedia, GetState()).WillByDefault(Return(IMedia::STATE_ACTIVE));
    SdpMedia objSdpMedia;
    objSdpMedia.SetPort(10000);
    ON_CALL(objMediaDescriptor, GetMediaDescriptionExAsLocal()).WillByDefault(Return(&objSdpMedia));
    ImsList<QosStatusRecord*> lstQosRecords;
    lstQosRecords.Append(nullptr);  // Any element
    ON_CALL(objStatusTable, GetRecords(_)).WillByDefault(Return(lstQosRecords));
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHING));

    EXPECT_CALL(objStatusTable, InitializeRecords(_)).Times(0);
    EXPECT_CALL(
            *pSdpPreconditionHelper, FormPreconditionSdp(&objISession, &objStatusTable, IMS_FALSE))
            .Times(1);
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest, CreateRecordsWithMediaTypeNoneAndFormPreconditionSdp)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ImsList<IMedia*> lstMedias;
    MockIMedia objAudioMedia;
    lstMedias.Append(&objAudioMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));
    ON_CALL(objAudioMedia, GetUpdateState()).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objAudioMedia, GetMediaDescriptor()).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objAudioMedia, GetState()).WillByDefault(Return(IMedia::STATE_ACTIVE));
    SdpMedia objSdpMedia;
    objSdpMedia.SetPort(10000);
    ON_CALL(objMediaDescriptor, GetMediaDescriptionExAsLocal()).WillByDefault(Return(&objSdpMedia));
    ImsList<QosStatusRecord*> lstQosRecords;
    ON_CALL(objStatusTable, GetRecords(_)).WillByDefault(Return(lstQosRecords));
    ON_CALL(*pSdpPreconditionHelper, GetMediaType(_, _)).WillByDefault(Return(MEDIATYPE_NONE));
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHING));

    EXPECT_CALL(objStatusTable, InitializeRecords(_)).Times(0);
    EXPECT_CALL(
            *pSdpPreconditionHelper, FormPreconditionSdp(&objISession, &objStatusTable, IMS_FALSE))
            .Times(1);
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest, CreateRecordsAndFormPreconditionSdpInEarlyDialogState)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));
    ImsList<IMedia*> lstMedias;
    MockIMedia objAudioMedia;
    lstMedias.Append(&objAudioMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));
    ON_CALL(objAudioMedia, GetUpdateState()).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objAudioMedia, GetMediaDescriptor()).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objAudioMedia, GetState()).WillByDefault(Return(IMedia::STATE_ACTIVE));
    SdpMedia objSdpMedia;
    objSdpMedia.SetPort(10000);
    ON_CALL(objMediaDescriptor, GetMediaDescriptionExAsLocal()).WillByDefault(Return(&objSdpMedia));
    ImsList<QosStatusRecord*> lstQosRecords;
    ON_CALL(objStatusTable, GetRecords(_)).WillByDefault(Return(lstQosRecords));
    ON_CALL(*pSdpPreconditionHelper, GetMediaType(_, _)).WillByDefault(Return(MEDIATYPE_AUDIO));
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHING));

    EXPECT_CALL(objStatusTable, InitializeRecords(SdpMedia::TYPE_AUDIO)).Times(1);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_AUDIO, IMS_TRUE)).Times(1);
    EXPECT_CALL(
            *pSdpPreconditionHelper, FormPreconditionSdp(&objISession, &objStatusTable, IMS_FALSE))
            .Times(1);
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest, CreateRecordsAndFormPreconditionSdpInConfirmedDialogState)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsPreconditionSupported()).WillByDefault(Return(IMS_TRUE));

    ImsList<IMedia*> lstMedias;
    MockIMedia objAudioMedia;
    lstMedias.Append(&objAudioMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));
    ON_CALL(objAudioMedia, GetUpdateState()).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objAudioMedia, GetMediaDescriptor()).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objAudioMedia, GetState()).WillByDefault(Return(IMedia::STATE_ACTIVE));
    SdpMedia objSdpMedia;
    objSdpMedia.SetPort(10000);
    ON_CALL(objMediaDescriptor, GetMediaDescriptionExAsLocal()).WillByDefault(Return(&objSdpMedia));
    ImsList<QosStatusRecord*> lstQosRecords;
    ON_CALL(objStatusTable, GetRecords(_)).WillByDefault(Return(lstQosRecords));
    ON_CALL(*pSdpPreconditionHelper, GetMediaType(_, _)).WillByDefault(Return(MEDIATYPE_AUDIO));
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHED));

    EXPECT_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT))
            .Times(2)
            .WillOnce(Return(ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_NOT_AVAILABLE))
            .WillOnce(Return(ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_AFTER_UPGRADE));
    EXPECT_CALL(objStatusTable, InitializeRecords(SdpMedia::TYPE_AUDIO)).Times(2);
    EXPECT_CALL(*pInfo, SetAudioStatus(QosStatus::AVAILABLE)).Times(1);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_AUDIO, IMS_TRUE)).Times(2);
    EXPECT_CALL(
            *pSdpPreconditionHelper, FormPreconditionSdp(&objISession, &objStatusTable, IMS_FALSE))
            .Times(2);

    objUpdatingInfo.SetModifier();
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
    pPreconditionManager->FormPreconditionSdp(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest, DoNotStartQosTimerOnSdpReceivedIfSessionIsNull)
{
    SetUpMockQosInfo();
    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER, _)).Times(0);

    pPreconditionManager->OnSdpReceived(nullptr);
}

TEST_F(MtcPreconditionManagerTest, DoNotStartQosTimerOnSdpReceivedIfItHasBeenStarted)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, IsWaitAudioDedicatedBearerTimerStarted()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER, _)).Times(0);

    pPreconditionManager->OnSdpReceived(&objISession);
}

TEST_F(MtcPreconditionManagerTest,
        UpdatesQosAttributesAndNotStartsQosTimerOnSdpReceivedIfMediaIsNotNegotiated)
{
    SetUpMockQosInfo();
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    ImsList<QosStatusRecord*> lstQosRecords;
    lstQosRecords.Append(nullptr);  // Any element
    ON_CALL(objStatusTable, GetRecords(SdpMedia::TYPE_AUDIO)).WillByDefault(Return(lstQosRecords));
    ImsList<IMedia*> lstMedias;
    MockIMedia objAudioMedia;
    lstMedias.Append(nullptr);
    lstMedias.Append(&objAudioMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));
    ON_CALL(*pSdpPreconditionHelper, IsPreconditionIncludedInSdp(&objISession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(&objISession))
            .WillByDefault(Return(NEGO_STATE::STATE_OFFER_SENT));

    EXPECT_CALL(objStatusTable, RemoveUnusedRecords(MEDIATYPE_AUDIO)).Times(1);
    EXPECT_CALL(objStatusTable, UpdateStatusTableWithRemoteSdp(Ref(objAudioMedia))).Times(1);
    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER, _)).Times(0);

    pPreconditionManager->OnSdpReceived(&objISession);
}

TEST_F(MtcPreconditionManagerTest, DoNotStartsQosTimerOnSdpReceivedIfPreconditionNotSupported)
{
    SetUpMockQosInfo();
    pPreconditionManager->SetCurrentRatTypeForPrerequisite(INetworkWatcher::RADIOTECH_TYPE_LTE);
    ON_CALL(objMediaManager, GetNegotiationState(&objISession))
            .WillByDefault(Return(NEGO_STATE::STATE_NEGOTIATED));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();
    objCallInfo.bUssi = IMS_TRUE;
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));

    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER, _)).Times(0);

    pPreconditionManager->OnSdpReceived(&objISession);
}

TEST_F(MtcPreconditionManagerTest, DoNotStartsQosTimerOnSdpReceivedIfAudioQosStatusIsNotIdle)
{
    SetUpMockQosInfo();
    ON_CALL(objMediaManager, GetNegotiationState(&objISession))
            .WillByDefault(Return(NEGO_STATE::STATE_NEGOTIATED));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::LOST));

    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER, _)).Times(0);
    pPreconditionManager->OnSdpReceived(&objISession);
}

TEST_F(MtcPreconditionManagerTest, DoNotStartsQosTimerOnSdpReceivedIfDefaultBearerForAudioIsAllowed)
{
    SetUpMockQosInfo();
    ON_CALL(objMediaManager, GetNegotiationState(&objISession))
            .WillByDefault(Return(NEGO_STATE::STATE_NEGOTIATED));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_VOICE_ON_DEFAULT_BEARER_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER, _)).Times(0);
    pPreconditionManager->OnSdpReceived(&objISession);
}

TEST_F(MtcPreconditionManagerTest, StartsQosTimerOnSdpReceived)
{
    SetUpMockQosInfo();
    pPreconditionManager->SetCurrentRatTypeForPrerequisite(INetworkWatcher::RADIOTECH_TYPE_LTE);
    ON_CALL(objMediaManager, GetNegotiationState(&objISession))
            .WillByDefault(Return(NEGO_STATE::STATE_NEGOTIATED));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT))
            .WillByDefault(Return(20000));

    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER, 20000)).Times(1);
    pPreconditionManager->OnSdpReceived(&objISession);
}

TEST_F(MtcPreconditionManagerTest,
        StartsOrNotStartsQosTimerByWaitQosWhenLocalPreconditionNotSupportConfig)
{
    SetUpMockQosInfo();
    pPreconditionManager->SetCurrentRatTypeForPrerequisite(INetworkWatcher::RADIOTECH_TYPE_LTE);
    ON_CALL(objMediaManager, GetNegotiationState(&objISession))
            .WillByDefault(Return(NEGO_STATE::STATE_NEGOTIATED));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_FALSE);
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT))
            .WillByDefault(Return(20000));

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_WAIT_QOS_WHEN_LOCAL_PRECONDITION_NOT_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objTimer, StartQosTimer(_, _)).Times(0);
    pPreconditionManager->OnSdpReceived(&objISession);

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_WAIT_QOS_WHEN_LOCAL_PRECONDITION_NOT_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, StartQosTimer(_, _)).Times(1);
    pPreconditionManager->OnSdpReceived(&objISession);
}

TEST_F(MtcPreconditionManagerTest,
        StartsQosTimerOnSdpSentIfInitialInviteIsSentAndTriggeringDedicatedWaitTimerIsEnabled)
{
    SetUpMockQosInfo();
    pPreconditionManager->SetCurrentRatTypeForPrerequisite(INetworkWatcher::RADIOTECH_TYPE_LTE);
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::
                            KEY_TRIGGER_DEDICATED_BEARER_WAIT_TIMER_BY_SENDING_INITIAL_INVITE_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT))
            .WillByDefault(Return(20000));

    EXPECT_CALL(objMediaManager, GetNegotiationState(&objISession)).Times(0);
    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER, 20000)).Times(1);
    pPreconditionManager->OnSdpSent(&objISession, IMS_TRUE);
}

TEST_F(MtcPreconditionManagerTest,
        DoNotStartsQosTimerOnSdpSentIfDedicatedWaitTimerIsNotUsedByRatCondition)
{
    SetUpMockQosInfo();

    // EPS fallback
    pPreconditionManager->SetCurrentRatTypeForPrerequisite(INetworkWatcher::RADIOTECH_TYPE_NR);
    pPreconditionManager->SetCurrentRatTypeForPrerequisite(INetworkWatcher::RADIOTECH_TYPE_LTE);

    ON_CALL(objMediaManager, GetNegotiationState(&objISession))
            .WillByDefault(Return(NEGO_STATE::STATE_NEGOTIATED));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();

    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_RAT_CONDITION_FOR_NOT_WAITING_DEDICATED_BEARER_INT_ARRAY,
                    ConfigVoice::NO_WAIT_DEDICATED_BEARER_IN_EPS_FALLBACK))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER, _)).Times(0);
    pPreconditionManager->OnSdpSent(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest,
        DoNotStartsQosTimerOnSdpSentIfDedicatedWaitTimerIsNotUsedByRatConditionEpsOnly)
{
    SetUpMockQosInfo();

    pPreconditionManager->SetCurrentRatTypeForPrerequisite(INetworkWatcher::RADIOTECH_TYPE_LTE);

    ON_CALL(objMediaManager, GetNegotiationState(&objISession))
            .WillByDefault(Return(NEGO_STATE::STATE_NEGOTIATED));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();

    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_RAT_CONDITION_FOR_NOT_WAITING_DEDICATED_BEARER_INT_ARRAY,
                    ConfigVoice::NO_WAIT_DEDICATED_BEARER_IN_EPS_ONLY_ATTACH))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsEpsOnlyAttach()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER, _)).Times(0);
    pPreconditionManager->OnSdpSent(&objISession, IMS_FALSE);
}

TEST_F(MtcPreconditionManagerTest, DoNothingOnMessageReceivedIfPreconditionNotSupported)
{
    SetUpMockQosInfo();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_FALSE);

    EXPECT_CALL(*pInfo, SetSupportingPrecondition(_)).Times(0);
    pPreconditionManager->OnMessageReceived(&objISession, &objIMessage);
}

TEST_F(MtcPreconditionManagerTest, DoNotingOn183NonReliableResponseReceived)
{
    SetUpMockQosInfo();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    ON_CALL(objIMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_183));
    ON_CALL(objISipMessage, IsMessageRpr()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pInfo, SetSupportingPrecondition(_)).Times(0);
    pPreconditionManager->OnMessageReceived(&objISession, &objIMessage);
}

TEST_F(MtcPreconditionManagerTest, SetsRemoteResourceAsAvailableOnFinalSuccessResponseReceived)
{
    SetUpMockQosInfo();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    ON_CALL(objIMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_200));
    ImsList<IMedia*> lstMedias;
    MockIMedia objAudioMedia;
    MockIMedia objVideoMedia;
    lstMedias.Append(nullptr);
    lstMedias.Append(&objAudioMedia);
    lstMedias.Append(&objVideoMedia);
    ON_CALL(objISession, GetMedia()).WillByDefault(Return(lstMedias));
    ON_CALL(objAudioMedia, GetUpdateState()).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objVideoMedia, GetUpdateState()).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
    ON_CALL(objAudioMedia, GetMediaDescriptor()).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objVideoMedia, GetMediaDescriptor()).WillByDefault(Return(&objMediaDescriptor));
    SipMethod objSipMethod = SipMethod::INVITE;
    ON_CALL(objIMessage, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(objISipMessage, GetType()).WillByDefault(Return(ISipMessage::TYPE_RESPONSE));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_FALSE));

    SdpMedia objSdpAudioMedia;
    EXPECT_CALL(objMediaDescriptor, GetMediaDescriptionEx())
            .Times(2)
            .WillOnce(Return(&objSdpAudioMedia))
            .WillOnce(Return(nullptr));
    EXPECT_CALL(objStatusTable, EnableRemoteCurrentStatus(_)).Times(1);
    pPreconditionManager->OnMessageReceived(&objISession, &objIMessage);
}

TEST_F(MtcPreconditionManagerTest, UpdatesSupportingPreconditionOnInviteReceivedRegardlessOfSdp)
{
    SetUpMockQosInfo();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    ON_CALL(objIMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_INVALID));
    SipMethod objInviteMethod = SipMethod::INVITE;
    ON_CALL(objIMessage, GetMethod()).WillByDefault(ReturnRef(objInviteMethod));
    ON_CALL(objISipMessage, GetType()).WillByDefault(Return(ISipMessage::TYPE_REQUEST));
    AString strPrecondition = "precondition";
    ON_CALL(objMessageUtils, HasValue(&objIMessage, strPrecondition, ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, HasValue(&objIMessage, strPrecondition, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pInfo, SetSupportingPrecondition(IMS_TRUE)).Times(1);
    pPreconditionManager->OnMessageReceived(&objISession, &objIMessage);
}

TEST_F(MtcPreconditionManagerTest, UpdatesSupportingPreconditionOn183ReliableResponseReceived)
{
    SetUpMockQosInfo();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    ON_CALL(objIMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_183));
    ON_CALL(objISipMessage, IsMessageRpr()).WillByDefault(Return(IMS_TRUE));
    SipMethod objSipMethod = SipMethod::INVITE;
    ON_CALL(objIMessage, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(objISipMessage, GetType()).WillByDefault(Return(ISipMessage::TYPE_RESPONSE));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pSdpPreconditionHelper, IsPreconditionIncludedInSdp(&objISession))
            .WillByDefault(Return(IMS_FALSE));
    AString strPrecondition = "precondition";
    ON_CALL(objMessageUtils, HasValue(&objIMessage, strPrecondition, ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasValue(&objIMessage, strPrecondition, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pInfo, SetSupportingPrecondition(IMS_FALSE)).Times(1);
    pPreconditionManager->OnMessageReceived(&objISession, &objIMessage);
}

TEST_F(MtcPreconditionManagerTest, DoNotUpdateSupportingPreconditionOnInviteReceivedIfSessionIsNull)
{
    SetUpMockQosInfo();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    ON_CALL(objIMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_INVALID));
    SipMethod objSipMethod = SipMethod::INVITE;
    ON_CALL(objIMessage, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(objISipMessage, GetType()).WillByDefault(Return(ISipMessage::TYPE_REQUEST));
    AString strPrecondition = "precondition";
    ON_CALL(objMessageUtils, HasValue(&objIMessage, strPrecondition, ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, HasValue(&objIMessage, strPrecondition, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pInfo, SetSupportingPrecondition(IMS_FALSE)).Times(0);
    pPreconditionManager->OnMessageReceived(nullptr, &objIMessage);
}

TEST_F(MtcPreconditionManagerTest,
        DoNotSetRemoteResourceAsAvailableOn180NonReliableResponseIfSessionIsNull)
{
    SetUpMockQosInfo();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    ON_CALL(objIMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_180));
    ON_CALL(objISipMessage, IsMessageRpr()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objStatusTable, EnableRemoteCurrentStatus(_)).Times(0);
    pPreconditionManager->OnMessageReceived(nullptr, &objIMessage);
}

TEST_F(MtcPreconditionManagerTest,
        DoNotStartQosTimerOnCallEstablishedIfDedicatedWaitTimerIsNotUsedByRatCondition)
{
    SetUpMockQosInfo();
    pPreconditionManager->SetCurrentRatTypeForPrerequisite(INetworkWatcher::RADIOTECH_TYPE_NR);
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_RAT_CONDITION_FOR_NOT_WAITING_DEDICATED_BEARER_INT_ARRAY,
                    ConfigVoice::NO_WAIT_DEDICATED_BEARER_IN_NR))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE, _)).Times(0);
    pPreconditionManager->OnCallEstablished(&objISession);
}

TEST_F(MtcPreconditionManagerTest, StartsQosTimerOnCallEstablishedIfLocalResourceIsNotReserved)
{
    SetUpMockQosInfo();
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetTextStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();
    SetUpSupportingPreconditionInLocal(CallType::RTT, IMS_TRUE);
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::
                            KEY_WAIT_VIDEO_TEXT_QOS_AFTER_AUDIO_QOS_ACQUISITION_TIMER_MILLIS_INT))
            .WillByDefault(Return(1000));

    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE, 1000)).Times(1);
    pPreconditionManager->OnCallEstablished(&objISession);
}

TEST_F(MtcPreconditionManagerTest, DoNothingOnCallModifiedIfQosIsNotCheckedAfterCallUpgrade)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT))
            .WillByDefault(Return(ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_NOT_AVAILABLE));
    pPreconditionManager->OnCallModified(&objISession);
}

TEST_F(MtcPreconditionManagerTest,
        DoNotStartQosTimerOnCallModifiedIfCallTypeDoesNotIncludeVideoOrText)
{
    SetUpMockQosInfo();
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT))
            .WillByDefault(Return(ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_AFTER_UPGRADE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE, _)).Times(0);
    pPreconditionManager->OnCallModified(&objISession);
}

TEST_F(MtcPreconditionManagerTest, DoNotStartQosTimerOnCallModifiedIfLocalResourceIsReserved)
{
    SetUpMockQosInfo();
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT))
            .WillByDefault(Return(ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_AFTER_UPGRADE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::AVAILABLE));

    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE, _)).Times(0);
    pPreconditionManager->OnCallModified(&objISession);
}

TEST_F(MtcPreconditionManagerTest, StartsQosTimerOnCallModifiedIfLocalResourceIsNotReserved)
{
    SetUpMockQosInfo();
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT))
            .WillByDefault(Return(ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_AFTER_UPGRADE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();
    SetUpSupportingPreconditionInLocal(CallType::VT, IMS_TRUE);

    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::
                            KEY_WAIT_VIDEO_TEXT_QOS_AFTER_AUDIO_QOS_ACQUISITION_TIMER_MILLIS_INT))
            .WillByDefault(Return(1000));

    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE, 1000)).Times(1);
    pPreconditionManager->OnCallModified(&objISession);
}

TEST_F(MtcPreconditionManagerTest, OnCallModifiedInitializedLocalAndRemoteQosForRemovedMedias)
{
    SetUpMockQosInfo();
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT))
            .WillByDefault(
                    Return(ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_DURING_UPGRADING));
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);

    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::LOST));
    ON_CALL(*pInfo, GetTextStatus()).WillByDefault(Return(QosStatus::IDLE));

    EXPECT_CALL(*pInfo, SetVideoStatus(QosStatus::IDLE));

    EXPECT_CALL(objStatusTable, RemoveUnusedRecords(MEDIATYPE_AUDIO)).Times(1);

    pPreconditionManager->OnCallModified(&objISession);
}

TEST_F(MtcPreconditionManagerTest, OnRatChangedUpdatesMobileRatType)
{
    pPreconditionManager->SetCurrentRatTypeForPrerequisite(INetworkWatcher::RADIOTECH_TYPE_NR);
    ON_CALL(objService, GetMobileRatType())
            .WillByDefault(Return(INetworkWatcher::RADIOTECH_TYPE_LTE_CA));

    pPreconditionManager->OnRatChanged(INetworkWatcher::RADIOTECH_TYPE_IWLAN);

    EXPECT_TRUE(pPreconditionManager->IsEpsFallbackPublic());
}

TEST_F(MtcPreconditionManagerTest, OnRatChangedIfHandoversToWlanFromMobile)
{
    SetUpMockQosInfo();
    pPreconditionManager->SetOnWlanForPrerequisite(IMS_FALSE);
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::GUARD_AFTER_LOST)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE)).Times(1);
    EXPECT_CALL(objListener, QosReserved(&objISession, MEDIATYPE_AUDIO)).Times(1);

    pPreconditionManager->OnRatChanged(INetworkWatcher::RADIOTECH_TYPE_IWLAN);
}

TEST_F(MtcPreconditionManagerTest, OnRatChangedIfHandoversToMobileFromWlan)
{
    SetUpMockQosInfo();
    pPreconditionManager->SetOnWlanForPrerequisite(IMS_TRUE);
    ON_CALL(*pInfo, IsWaitAudioDedicatedBearerTimerStarted()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();
    SetUpSupportingPreconditionInLocal(CallType::VT, IMS_TRUE);
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_QOS_ACQUISITION_AFTER_W2L_HANDOVER_WAIT_TIMER_MILLIS_INT))
            .WillByDefault(Return(5000));

    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER, 5000))
            .Times(1);

    pPreconditionManager->OnRatChanged(INetworkWatcher::RADIOTECH_TYPE_LTE);
}

TEST_F(MtcPreconditionManagerTest, OnRatChangedDoNotRestartQosTimerForEpsFallbackIfItIsNotRequired)
{
    SetUpMockQosInfo();
    pPreconditionManager->SetOnWlanForPrerequisite(IMS_FALSE);
    pPreconditionManager->SetCurrentRatTypeForPrerequisite(INetworkWatcher::RADIOTECH_TYPE_NR);

    ON_CALL(objService, GetMobileRatType())
            .WillByDefault(Return(INetworkWatcher::RADIOTECH_TYPE_LTE));

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_RESTART_DEDICATED_BEARER_WAIT_TIMER_BY_EPS_FALLBACK_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER)).Times(0);
    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER, _)).Times(0);

    pPreconditionManager->OnRatChanged(INetworkWatcher::RADIOTECH_TYPE_LTE);
}

TEST_F(MtcPreconditionManagerTest, OnRatChangedRestartsQosTimerForEpsFallback)
{
    SetUpMockQosInfo();
    pPreconditionManager->SetOnWlanForPrerequisite(IMS_FALSE);
    pPreconditionManager->SetCurrentRatTypeForPrerequisite(INetworkWatcher::RADIOTECH_TYPE_NR);

    ON_CALL(objService, GetMobileRatType())
            .WillByDefault(Return(INetworkWatcher::RADIOTECH_TYPE_LTE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_RESTART_DEDICATED_BEARER_WAIT_TIMER_BY_EPS_FALLBACK_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER))
            .WillByDefault(Return(IMS_TRUE));
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_DEDICATED_BEARER_WAIT_TIMER_MILLIS_INT))
            .WillByDefault(Return(20000));

    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER)).Times(1);
    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER, 20000)).Times(1);

    pPreconditionManager->OnRatChanged(INetworkWatcher::RADIOTECH_TYPE_LTE);
}

TEST_F(MtcPreconditionManagerTest, DoNothingOnQosStatusChangedIfStatusIsNotChanged)
{
    // AVAILABLE -> AVAILABLE
    SetUpMockQosInfo();
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::AVAILABLE));

    EXPECT_CALL(*pInfo, SetAudioStatus(QosStatus::AVAILABLE)).Times(0);

    pPreconditionManager->OnQosStatusChanged(&objISession, QosStatus::AVAILABLE, MEDIATYPE_AUDIO);
}

TEST_F(MtcPreconditionManagerTest, DoNothingOnQosStatusChangedIfStatusChangeIsInvalid)
{
    // IDLE -> LOST
    SetUpMockQosInfo();
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));

    EXPECT_CALL(*pInfo, SetAudioStatus(QosStatus::LOST)).Times(0);

    pPreconditionManager->OnQosStatusChanged(&objISession, QosStatus::LOST, MEDIATYPE_AUDIO);
}

TEST_F(MtcPreconditionManagerTest, DoNothingOnQosStatusChangedIfSessionIsNull)
{
    SetUpMockQosInfo();
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::UNKNOWN));

    EXPECT_CALL(*pInfo, SetAudioStatus(QosStatus::AVAILABLE)).Times(0);

    pPreconditionManager->OnQosStatusChanged(nullptr, QosStatus::AVAILABLE, MEDIATYPE_AUDIO);
}

TEST_F(MtcPreconditionManagerTest, OnQosStatusChangedIfStatusIsChangedToLostFromAvailable)
{
    // AVAILABLE -> LOST
    SetUpMockQosInfo();
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    SetUpNothingOnDefaultBearerSupported();
    SetUpSupportingPreconditionInLocal(CallType::VT, IMS_TRUE);
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_QOS_LOST_GUARD_TIMER_MILLIS_INT))
            .WillByDefault(Return(5000));

    EXPECT_CALL(*pInfo, GetVideoStatus())
            .Times(AnyNumber())
            .WillOnce(Return(QosStatus::AVAILABLE))
            .WillRepeatedly(Return(QosStatus::LOST));
    EXPECT_CALL(*pInfo, SetVideoStatus(QosStatus::LOST)).Times(1);
    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::GUARD_AFTER_LOST, 5000)).Times(1);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_VIDEO, IMS_FALSE)).Times(1);
    pPreconditionManager->OnQosStatusChanged(&objISession, QosStatus::LOST, MEDIATYPE_VIDEO);
}

TEST_F(MtcPreconditionManagerTest,
        OnQosStatusChangedSetsIdleIfStatusIsChangedToLostAndTheMediaTypeIsRemoved)
{
    // AVAILABLE -> LOST -> IDLE
    SetUpMockQosInfo();
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    SetUpNothingOnDefaultBearerSupported();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);

    EXPECT_CALL(*pInfo, GetVideoStatus())
            .Times(AnyNumber())
            .WillOnce(Return(QosStatus::AVAILABLE))
            .WillRepeatedly(Return(QosStatus::LOST));
    EXPECT_CALL(*pInfo, SetVideoStatus(QosStatus::LOST)).Times(1);
    EXPECT_CALL(*pInfo, SetVideoStatus(QosStatus::IDLE)).Times(1);
    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::GUARD_AFTER_LOST, _)).Times(0);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_VIDEO, IMS_FALSE)).Times(1);
    pPreconditionManager->OnQosStatusChanged(&objISession, QosStatus::LOST, MEDIATYPE_VIDEO);
}

TEST_F(MtcPreconditionManagerTest, OnQosStatusChangedIfStatusIsChangedToAvailableFromLost)
{
    // LOST -> AVAILABLE
    SetUpMockQosInfo();
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    SetUpNothingOnDefaultBearerSupported();
    SetUpSupportingPreconditionInLocal(CallType::RTT, IMS_TRUE);

    EXPECT_CALL(*pInfo, GetTextStatus())
            .Times(AnyNumber())
            .WillOnce(Return(QosStatus::LOST))
            .WillRepeatedly(Return(QosStatus::AVAILABLE));
    EXPECT_CALL(*pInfo, SetTextStatus(QosStatus::AVAILABLE)).Times(1);

    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::GUARD_AFTER_LOST)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::FORCE_AVAILABLE)).Times(1);

    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_TEXT, IMS_TRUE)).Times(1);
    pPreconditionManager->OnQosStatusChanged(&objISession, QosStatus::AVAILABLE, MEDIATYPE_TEXT);
}

TEST_F(MtcPreconditionManagerTest,
        OnQosStatusChangedIfStatusIsChangedToAvailableFromIdleAndPartialResourceIsReserved)
{
    // IDLE -> AVAILABLE
    SetUpMockQosInfo();
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();
    SetUpSupportingPreconditionInLocal(CallType::VT, IMS_TRUE);
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::
                            KEY_WAIT_VIDEO_TEXT_QOS_AFTER_AUDIO_QOS_ACQUISITION_TIMER_MILLIS_INT))
            .WillByDefault(Return(1000));

    EXPECT_CALL(*pInfo, GetAudioStatus())
            .Times(AnyNumber())
            .WillOnce(Return(QosStatus::IDLE))
            .WillRepeatedly(Return(QosStatus::AVAILABLE));
    EXPECT_CALL(*pInfo, SetAudioStatus(QosStatus::AVAILABLE)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER)).Times(1);
    EXPECT_CALL(objTimer, StartQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE, 1000)).Times(1);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_AUDIO, IMS_TRUE)).Times(1);
    EXPECT_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objListener, QosReserved(&objISession, MEDIATYPE_AUDIO)).Times(0);

    pPreconditionManager->OnQosStatusChanged(&objISession, QosStatus::AVAILABLE, MEDIATYPE_AUDIO);
}

TEST_F(MtcPreconditionManagerTest,
        OnQosStatusChangedIfStatusIsChangedToAvailableFromIdleAndAllResourcesAreReserved)
{
    // IDLE -> AVAILABLE
    SetUpMockQosInfo();
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    SetUpNothingOnDefaultBearerSupported();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);

    EXPECT_CALL(*pInfo, GetAudioStatus())
            .Times(AnyNumber())
            .WillOnce(Return(QosStatus::IDLE))
            .WillRepeatedly(Return(QosStatus::AVAILABLE));
    EXPECT_CALL(*pInfo, SetAudioStatus(QosStatus::AVAILABLE)).Times(1);

    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::GUARD_AFTER_LOST)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::FORCE_AVAILABLE)).Times(1);

    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_AUDIO, IMS_TRUE)).Times(1);
    EXPECT_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objListener, QosReserved(&objISession, MEDIATYPE_AUDIO)).Times(1);

    pPreconditionManager->OnQosStatusChanged(&objISession, QosStatus::AVAILABLE, MEDIATYPE_AUDIO);
}

TEST_F(MtcPreconditionManagerTest, OnQosStatusChangedStartsWaitVideoTextAvailableTimerInNr)
{
    pPreconditionManager->SetCurrentRatTypeForPrerequisite(INetworkWatcher::RADIOTECH_TYPE_NR);
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_RAT_CONDITION_FOR_NOT_WAITING_DEDICATED_BEARER_INT_ARRAY,
                    ConfigVoice::NO_WAIT_DEDICATED_BEARER_IN_NR))
            .WillByDefault(Return(IMS_FALSE));

    // IDLE -> AVAILABLE
    SetUpMockQosInfo();
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    SetUpNothingOnDefaultBearerSupported();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);

    EXPECT_CALL(*pInfo, GetAudioStatus())
            .Times(AnyNumber())
            .WillOnce(Return(QosStatus::IDLE))
            .WillRepeatedly(Return(QosStatus::AVAILABLE));
    EXPECT_CALL(*pInfo, SetAudioStatus(QosStatus::AVAILABLE)).Times(1);

    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::GUARD_AFTER_LOST)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::FORCE_AVAILABLE)).Times(1);

    pPreconditionManager->OnQosStatusChanged(&objISession, QosStatus::AVAILABLE, MEDIATYPE_AUDIO);
}

TEST_F(MtcPreconditionManagerTest,
        OnQosStatusChangedStartsWaitVideoTextAvailableTimerInNrAndNotUsingConfigContainsNr)
{
    pPreconditionManager->SetCurrentRatTypeForPrerequisite(INetworkWatcher::RADIOTECH_TYPE_NR);
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_RAT_CONDITION_FOR_NOT_WAITING_DEDICATED_BEARER_INT_ARRAY,
                    ConfigVoice::NO_WAIT_DEDICATED_BEARER_IN_NR))
            .WillByDefault(Return(IMS_TRUE));

    // IDLE -> AVAILABLE
    SetUpMockQosInfo();
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    SetUpNothingOnDefaultBearerSupported();
    SetUpSupportingPreconditionInLocal(CallType::VOIP, IMS_TRUE);

    EXPECT_CALL(*pInfo, GetAudioStatus())
            .Times(AnyNumber())
            .WillOnce(Return(QosStatus::IDLE))
            .WillRepeatedly(Return(QosStatus::AVAILABLE));
    EXPECT_CALL(*pInfo, SetAudioStatus(QosStatus::AVAILABLE)).Times(1);

    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::GUARD_AFTER_LOST)).Times(1);
    EXPECT_CALL(objTimer, StopQosTimer(QosTimerType::FORCE_AVAILABLE)).Times(1);

    pPreconditionManager->OnQosStatusChanged(&objISession, QosStatus::AVAILABLE, MEDIATYPE_AUDIO);
}

TEST_F(MtcPreconditionManagerTest, DoNothingOnTimerExpiredIfSessionIsNull)
{
    pPreconditionManager->ReplaceQosInfo(&objISession, nullptr);

    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objListener, QosReserveFailed(_, _)).Times(0);
    EXPECT_CALL(objListener, QosReserved(_, _)).Times(0);

    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::WAIT_AUDIO_DEDICATED_BEARER);
    pPreconditionManager->OnTimerExpired(
            &objTimer, QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER);
    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE);
    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::GUARD_AFTER_LOST);
    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::FORCE_AVAILABLE);
}

TEST_F(MtcPreconditionManagerTest, OnForceAvailableTimerExpiredForUnknown)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetTextStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::UNKNOWN));

    EXPECT_CALL(*pInfo, SetAudioStatus(QosStatus::AVAILABLE)).Times(0);
    EXPECT_CALL(*pInfo, SetVideoStatus(QosStatus::AVAILABLE)).Times(0);
    EXPECT_CALL(*pInfo, SetTextStatus(QosStatus::AVAILABLE)).Times(0);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_AUDIO, IMS_TRUE)).Times(0);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_VIDEO, IMS_TRUE)).Times(0);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_TEXT, IMS_TRUE)).Times(0);
    EXPECT_CALL(objListener, QosReserved(&objISession, _)).Times(0);

    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::FORCE_AVAILABLE);
}

TEST_F(MtcPreconditionManagerTest, OnForceAvailableTimerExpiredForVoip)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetTextStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(*pInfo, SetAudioStatus(QosStatus::AVAILABLE)).Times(1);
    EXPECT_CALL(*pInfo, SetVideoStatus(QosStatus::AVAILABLE)).Times(0);
    EXPECT_CALL(*pInfo, SetTextStatus(QosStatus::AVAILABLE)).Times(0);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_AUDIO, IMS_TRUE)).Times(1);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_VIDEO, IMS_TRUE)).Times(0);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_TEXT, IMS_TRUE)).Times(0);
    EXPECT_CALL(objListener, QosReserved(&objISession, MEDIA_TYPE_AUDIO)).Times(1);

    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::FORCE_AVAILABLE);
}

TEST_F(MtcPreconditionManagerTest, OnForceAvailableTimerExpiredForVt)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetTextStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(*pInfo, SetAudioStatus(QosStatus::AVAILABLE)).Times(1);
    EXPECT_CALL(*pInfo, SetVideoStatus(QosStatus::AVAILABLE)).Times(1);
    EXPECT_CALL(*pInfo, SetTextStatus(QosStatus::AVAILABLE)).Times(0);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_AUDIO, IMS_TRUE)).Times(1);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_VIDEO, IMS_TRUE)).Times(1);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_TEXT, IMS_TRUE)).Times(0);
    EXPECT_CALL(objListener, QosReserved(&objISession, MEDIA_TYPE_AUDIOVIDEO)).Times(1);

    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::FORCE_AVAILABLE);
}

TEST_F(MtcPreconditionManagerTest, OnForceAvailableTimerExpiredForRtt)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetTextStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::RTT));

    EXPECT_CALL(*pInfo, SetAudioStatus(QosStatus::AVAILABLE)).Times(1);
    EXPECT_CALL(*pInfo, SetVideoStatus(QosStatus::AVAILABLE)).Times(0);
    EXPECT_CALL(*pInfo, SetTextStatus(QosStatus::AVAILABLE)).Times(0);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_AUDIO, IMS_TRUE)).Times(1);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_VIDEO, IMS_TRUE)).Times(0);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_TEXT, IMS_TRUE)).Times(0);
    EXPECT_CALL(objListener, QosReserved(&objISession, MEDIA_TYPE_AUDIO)).Times(1);

    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::FORCE_AVAILABLE);
}

TEST_F(MtcPreconditionManagerTest, OnForceAvailableTimerExpiredForVideoRtt)
{
    SetUpMockQosInfo();
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::IDLE));
    ON_CALL(*pInfo, GetTextStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VIDEO_RTT));

    EXPECT_CALL(*pInfo, SetAudioStatus(QosStatus::AVAILABLE)).Times(1);
    EXPECT_CALL(*pInfo, SetVideoStatus(QosStatus::AVAILABLE)).Times(1);
    EXPECT_CALL(*pInfo, SetTextStatus(QosStatus::AVAILABLE)).Times(0);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_AUDIO, IMS_TRUE)).Times(1);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_VIDEO, IMS_TRUE)).Times(1);
    EXPECT_CALL(objStatusTable, UpdateLocalCurrentStatus(SdpMedia::TYPE_TEXT, IMS_TRUE)).Times(0);
    EXPECT_CALL(objListener, QosReserved(&objISession, MEDIA_TYPE_AUDIOVIDEO)).Times(1);

    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::FORCE_AVAILABLE);
}

TEST_F(MtcPreconditionManagerTest, DoNothingOnWaitAudioDedicatedBearerTimerExpiredIfOnWlan)
{
    SetUpMockQosInfo();
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objListener, QosReserveFailed(&objISession, _)).Times(0);

    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::WAIT_AUDIO_DEDICATED_BEARER);
}

TEST_F(MtcPreconditionManagerTest,
        DoNothingOnWaitAudioDedicatedBearerTimerExpiredIfWaitingQosAfterW2LHandover)
{
    SetUpMockQosInfo();
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objListener, QosReserveFailed(&objISession, _)).Times(0);

    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::WAIT_AUDIO_DEDICATED_BEARER);
}

TEST_F(MtcPreconditionManagerTest, NotifiesQosReserveFailedOnWaitAudioDedicatedBearerTimerExpired)
{
    SetUpMockQosInfo();
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_POLICY_ON_AUDIO_QOS_DEACTIVATION_INT))
            .WillByDefault(Return(ConfigVoice::QOS_DEACTIVATION_POLICY_TERMINATE_CALL));

    EXPECT_CALL(objListener, QosReserveFailed(&objISession, QosLossPolicy::RELEASE)).Times(1);

    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::WAIT_AUDIO_DEDICATED_BEARER);
}

TEST_F(MtcPreconditionManagerTest,
        DoNothingOnWaitAvailableAfterW2LHandoverTimerExpiredIfWaitingAudioDedicatedBearer)
{
    SetUpMockQosInfo();
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objListener, QosReserveFailed(&objISession, _)).Times(0);

    pPreconditionManager->OnTimerExpired(
            &objTimer, QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER);
}

TEST_F(MtcPreconditionManagerTest,
        NotifiesQosReserveFailedOnWaitAvailableAfterW2LHandoverTimerExpired)
{
    SetUpMockQosInfo();
    ON_CALL(objTimer, IsQosTimerActivated(QosTimerType::WAIT_AUDIO_DEDICATED_BEARER))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::LOST));
    SetUpNothingOnDefaultBearerSupported();
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_ON_VIDEO_QOS_DEACTIVATION_INT))
            .WillByDefault(Return(ConfigVoice::QOS_DEACTIVATION_POLICY_MODIFY_CALL));

    EXPECT_CALL(objListener, QosReserveFailed(&objISession, QosLossPolicy::MODIFY)).Times(1);

    pPreconditionManager->OnTimerExpired(
            &objTimer, QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER);
}

TEST_F(MtcPreconditionManagerTest,
        DoNothingOnWaitVideoTextAvailableTimerExpiredInConfirmedStateAndQosLossPolicyIsMaintain)
{
    SetUpMockQosInfo();
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_ON_VIDEO_QOS_DEACTIVATION_INT))
            .WillByDefault(Return(-1));

    EXPECT_CALL(objListener, QosReserveFailed(&objISession, _)).Times(0);
    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE);
}

TEST_F(MtcPreconditionManagerTest,
        NotifiesQosReservedOnWaitVideoTextAvailableTimerExpiredInEarlyState)
{
    SetUpMockQosInfo();
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHING));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::RTT));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::AVAILABLE));
    ON_CALL(*pInfo, GetTextStatus()).WillByDefault(Return(QosStatus::IDLE));

    EXPECT_CALL(objListener, QosReserved(&objISession, MEDIATYPE_AUDIO)).Times(1);
    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE);
}

TEST_F(MtcPreconditionManagerTest, NotifiesQosReserveFailedToListenerOnGuardAfterLostTimerExpired)
{
    SetUpMockQosInfo();
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VIDEO_RTT));
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::LOST));
    ON_CALL(*pInfo, GetVideoStatus()).WillByDefault(Return(QosStatus::LOST));
    ON_CALL(*pInfo, GetTextStatus()).WillByDefault(Return(QosStatus::LOST));
    SetUpNothingOnDefaultBearerSupported();
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_POLICY_ON_AUDIO_QOS_DEACTIVATION_INT))
            .WillByDefault(Return(ConfigVoice::QOS_DEACTIVATION_POLICY_MAINTAIN_CALL));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_ON_VIDEO_QOS_DEACTIVATION_INT))
            .WillByDefault(Return(ConfigVoice::QOS_DEACTIVATION_POLICY_MODIFY_CALL));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigRtt::KEY_POLICY_ON_TEXT_QOS_DEACTIVATION_INT))
            .WillByDefault(Return(ConfigVoice::QOS_DEACTIVATION_POLICY_TERMINATE_CALL));

    EXPECT_CALL(objListener, QosReserveFailed(&objISession, QosLossPolicy::RELEASE)).Times(1);

    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::GUARD_AFTER_LOST);
}

TEST_F(MtcPreconditionManagerTest,
        NotifiesQosReserveFailedOnQosStatusChangedLostInEstablishingState)
{
    SetUpMockQosInfo();
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHING));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_POLICY_ON_AUDIO_QOS_DEACTIVATION_INT))
            .WillByDefault(Return(ConfigVoice::QOS_DEACTIVATION_POLICY_MAINTAIN_CALL));

    EXPECT_CALL(objListener, QosReserveFailed(&objISession, _)).Times(1);
    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::GUARD_AFTER_LOST);
}

TEST_F(MtcPreconditionManagerTest,
        NotifiesQosReserveFailedOnWaitAudioAvailableTimerExpiredInEstablishingState)
{
    SetUpMockQosInfo();
    ON_CALL(objISession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHING));
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pInfo, GetAudioStatus()).WillByDefault(Return(QosStatus::IDLE));
    SetUpNothingOnDefaultBearerSupported();
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_POLICY_ON_AUDIO_QOS_DEACTIVATION_INT))
            .WillByDefault(Return(ConfigVoice::QOS_DEACTIVATION_POLICY_MAINTAIN_CALL));

    EXPECT_CALL(objListener, QosReserveFailed(&objISession, _)).Times(1);
    pPreconditionManager->OnTimerExpired(&objTimer, QosTimerType::WAIT_AUDIO_DEDICATED_BEARER);
}
