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

#include "call/IMtcCall.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// TODO: Unit tests of only the uncovered lines are added. The other cases will be added.

LOCAL PeerType ANY_PEER_TYPE = PeerType::MT;
LOCAL CallType ANY_CALL_TYPE = CallType::VIDEO_RTT;
LOCAL EmergencyType ANY_EMERGENCY_TYPE = EmergencyType::NORMAL_ROUTING;
LOCAL IMS_BOOL ANY_OFFLINE_BOOL = IMS_TRUE;
LOCAL IMS_BOOL ANY_USSI_BOOL = IMS_TRUE;
LOCAL IMS_BOOL ANY_CONFERENCE_BOOL = IMS_TRUE;

TEST(CallInfoTest, CallInfoAssignmentOperator)
{
    CallInfo objCallInfo;
    CallInfo objRightHandSide;
    objRightHandSide.ePeerType = ANY_PEER_TYPE;
    objRightHandSide.eInitialCallType = ANY_CALL_TYPE;
    objRightHandSide.eEmergencyType = ANY_EMERGENCY_TYPE;
    objRightHandSide.bOffline = ANY_OFFLINE_BOOL;
    objRightHandSide.bUssi = ANY_USSI_BOOL;
    objRightHandSide.bConference = ANY_CONFERENCE_BOOL;

    objCallInfo = objRightHandSide;

    EXPECT_EQ(objCallInfo.ePeerType, objRightHandSide.ePeerType);
    EXPECT_EQ(objCallInfo.eInitialCallType, objRightHandSide.eInitialCallType);
    EXPECT_EQ(objCallInfo.eEmergencyType, objRightHandSide.eEmergencyType);
    EXPECT_EQ(objCallInfo.bOffline, objRightHandSide.bOffline);
    EXPECT_EQ(objCallInfo.bUssi, objRightHandSide.bUssi);
    EXPECT_EQ(objCallInfo.bConference, objRightHandSide.bConference);
}

TEST(CallInfoTest, CallInfoEqualToOperator)
{
    CallInfo objCallInfo;
    CallInfo* pCallInfo = &objCallInfo;
    EXPECT_TRUE(objCallInfo == *pCallInfo);

    CallInfo objCallInfoWithValues;
    objCallInfoWithValues.ePeerType = ANY_PEER_TYPE;
    objCallInfoWithValues.eInitialCallType = ANY_CALL_TYPE;
    objCallInfoWithValues.eEmergencyType = ANY_EMERGENCY_TYPE;
    objCallInfoWithValues.bOffline = ANY_OFFLINE_BOOL;
    objCallInfoWithValues.bUssi = ANY_USSI_BOOL;
    objCallInfoWithValues.bConference = ANY_CONFERENCE_BOOL;

    CallInfo objCallInfoWithValuesToCompare;
    objCallInfoWithValuesToCompare.ePeerType = ANY_PEER_TYPE;
    objCallInfoWithValuesToCompare.eInitialCallType = ANY_CALL_TYPE;
    objCallInfoWithValuesToCompare.eEmergencyType = ANY_EMERGENCY_TYPE;
    objCallInfoWithValuesToCompare.bOffline = ANY_OFFLINE_BOOL;
    objCallInfoWithValuesToCompare.bUssi = ANY_USSI_BOOL;
    objCallInfoWithValuesToCompare.bConference = ANY_CONFERENCE_BOOL;
    EXPECT_TRUE(objCallInfoWithValues == objCallInfoWithValuesToCompare);
}

TEST(CallInfoTest, CallInfoNotEqual)
{
    CallType __DIFF_CALL_TYPE__ = CallType::VT;

    CallInfo objCallInfoWithValues;
    objCallInfoWithValues.ePeerType = ANY_PEER_TYPE;
    objCallInfoWithValues.eInitialCallType = ANY_CALL_TYPE;
    objCallInfoWithValues.eEmergencyType = ANY_EMERGENCY_TYPE;
    objCallInfoWithValues.bOffline = ANY_OFFLINE_BOOL;
    objCallInfoWithValues.bUssi = ANY_USSI_BOOL;
    objCallInfoWithValues.bConference = ANY_CONFERENCE_BOOL;

    CallInfo objCallInfoWithValuesToCompare;
    objCallInfoWithValuesToCompare.ePeerType = ANY_PEER_TYPE;
    objCallInfoWithValuesToCompare.eInitialCallType = __DIFF_CALL_TYPE__;
    objCallInfoWithValuesToCompare.eEmergencyType = ANY_EMERGENCY_TYPE;
    objCallInfoWithValuesToCompare.bOffline = ANY_OFFLINE_BOOL;
    objCallInfoWithValuesToCompare.bUssi = ANY_USSI_BOOL;
    objCallInfoWithValuesToCompare.bConference = ANY_CONFERENCE_BOOL;
    EXPECT_TRUE(objCallInfoWithValues != objCallInfoWithValuesToCompare);
}
