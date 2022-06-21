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
#ifndef JNI_CONNECTOR_FACTORY_H_
#define JNI_CONNECTOR_FACTORY_H_

#include "IMSTypeDef.h"
#include "IMutex.h"
#include "JniConnector.h"

class IAosService;
class IMtcCallController;
class IMtcService;
class IMtsService;
class JniAosService;
class JniConnectorHolder;
class JniMediaSession;
class JniMtcCall;
class JniMtcService;
class JniMtsService;
class IMediaManager;

class JniConnectorFactory
{
private:
    JniConnectorFactory();

public:
    ~JniConnectorFactory();

public:
    static JniConnectorFactory* GetInstance();
    void ReleaseConnectors(IN IMS_SINT32 nSlotId);

    JniConnector<IAosService, JniAosService>* GetAosServiceConnector(IN IMS_SINT32 nSlotId);
    JniConnector<IMtcCallController, JniMtcCall>* GetMtcCallConnector(IN IMS_SINT32 nSlotId);
    JniConnector<IMtcService, JniMtcService>* GetMtcServiceConnector(IN IMS_SINT32 nSlotId);
    JniConnector<IMtsService, JniMtsService>* GetMtsServiceConnector(IN IMS_SINT32 nSlotId);
    JniConnector<IMediaManager, JniMediaSession>* GetMediaSessionConnector(IN IMS_SINT32 nSlotId);

private:
    static JniConnectorFactory* s_pFactory;
    JniConnectorHolder* m_pConnectorHolder;
    IMutex* m_piLock;
};

#endif
