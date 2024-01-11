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

#ifndef MOCK_QOS_STATUS_TABLE_H_
#define MOCK_QOS_STATUS_TABLE_H_

#include "ImsTypeDef.h"
#include "precondition/QosStatusTable.h"
#include <gmock/gmock.h>

class IMedia;
class IMediaDescriptor;

class MockQosStatusTable : public QosStatusTable
{
public:
    virtual ~MockQosStatusTable() {}

    MOCK_METHOD(ImsList<QosStatusRecord*>, GetRecords, (IN IMS_SINT32 eSdpMediaType),
            (const, override));
    MOCK_METHOD(void, ClearRecords, (IN IMS_SINT32 eSdpMediaType), (override));
    MOCK_METHOD(void, UpdateStatusTableWithRemoteSdp, (IN const IMedia& objMedia), (override));
    MOCK_METHOD(void, UpdateLocalCurrentStatus,
            (IN IMS_SINT32 eSdpMediaType, IN IMS_BOOL bLocalQoSEnabled), (override));
    MOCK_METHOD(void, EnableRemoteCurrentStatus, (IN IMS_SINT32 eSdpMediaType), (override));
    MOCK_METHOD(IMS_BOOL, IsCurrentStatusEnabled,
            (IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eStatusType), (override));
    MOCK_METHOD(IMS_SINT32, GetDirectionTag,
            (IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eAttrType, IN IMS_SINT32 eStatusType),
            (override));
    MOCK_METHOD(IMS_SINT32, GetStrengthTag,
            (IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eStatusType, IN IMS_SINT32 eDirTag),
            (override));
    MOCK_METHOD(void, SetDirectionTag,
            (IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eAttrType, IN IMS_SINT32 eStatusType,
                    IN IMS_SINT32 eDirTag),
            (override));
    MOCK_METHOD(void, SetStrengthTag,
            (IN IMS_SINT32 eSdpMediaType, IN IMS_SINT32 eStatusType, IN IMS_SINT32 eDirTag,
                    IN IMS_SINT32 eStrengthTag),
            (override));
    MOCK_METHOD(void, SetLocalResourceConfirmed,
            (IN IMS_SINT32 eSdpMediaType, IN IMS_BOOL bConfirmed), (override));
    MOCK_METHOD(IMS_BOOL, IsLocalResourceConfirmed, (IN IMS_SINT32 eSdpMediaType), (override));
    MOCK_METHOD(void, InitializeRecords, (IN IMS_SINT32 eSdpMediaType), (override));
    MOCK_METHOD(void, RemoveUnusedRecords, (IN IMS_UINT32 eMediaTypes), (override));
};

#endif
