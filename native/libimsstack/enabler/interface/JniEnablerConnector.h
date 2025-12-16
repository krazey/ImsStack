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
#ifndef JNI_ENABLER_CONNECTOR_H_
#define JNI_ENABLER_CONNECTOR_H_

#include "ImsList.h"
#include "ImsTypeDef.h"

class IJniEnabler;
class IMutex;
class INativeEnabler;
class JniConnection;
enum class EnablerType
{
    AOS_SERVICE,
    MTC_SERVICE,
    MTC_CALL,
    MTS,
    MEDIA_SESSION,
    UCE
};

class JniEnablerConnector final
{
private:
    explicit JniEnablerConnector();

public:
    virtual ~JniEnablerConnector();
    JniEnablerConnector(IN const JniEnablerConnector&) = delete;
    JniEnablerConnector& operator=(IN const JniEnablerConnector&) = delete;

    static JniEnablerConnector& GetInstance();

    void SetNativeEnabler(
            IN IMS_SINT32 nSlotId, IN EnablerType eType, IN INativeEnabler* piNativeEnabler);
    void SetJniEnabler(IN IMS_SINT32 nSlotId, IN EnablerType eType, IN IJniEnabler* piJniEnabler,
            IN IMS_ULONG nKey = KEY_UNIQUE);
    INativeEnabler* GetNativeEnabler(IN IMS_SINT32 nSlotId, IN EnablerType eType);
    IJniEnabler* GetJniEnabler(
            IN IMS_SINT32 nSlotId, IN EnablerType eType, IN IMS_ULONG nKey = KEY_UNIQUE);

    // For Unit Test
    inline IMS_UINT32 GetConnectionSize() const { return m_objConnections.GetSize(); }

private:
    JniConnection* GetConnection(IN IMS_SINT32 nSlotId, IN EnablerType eType) const;
    JniConnection& CreateConnectionIfNotExist(IN IMS_SINT32 nSlotId, IN EnablerType eType);
    void CheckAndRemoveConnection(IN JniConnection* pConnection);

public:
    static const IMS_ULONG KEY_UNIQUE = 0;

private:
    static JniEnablerConnector* s_pConnector;
    ImsList<JniConnection*> m_objConnections;
    IMutex* m_piLock;
};

#endif
