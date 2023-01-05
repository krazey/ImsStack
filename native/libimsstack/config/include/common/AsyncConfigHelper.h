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
#ifndef ASYNC_CONFIG_HELPER_H_
#define ASYNC_CONFIG_HELPER_H_

#include "ImsActivityEx.h"
#include "ImsMessageDef.h"

class IAsyncConfig;

class AsyncConfigHelper : public ImsActivityEx
{
public:
    AsyncConfigHelper();
    virtual ~AsyncConfigHelper();

public:
    void Register(IN IAsyncConfig* piConfig);
    IMS_BOOL SendTo(IN IAsyncConfig* piConfig, IN IMS_SINT32 nMsg, IN IMS_SINTP nParam1,
            IN IMS_SINTP nParam2);
    void Unregister(IN const IAsyncConfig* piConfig);

protected:
    // ImsActivityEx class
    IMS_BOOL OnMessage(IN ImsMessage& objMsg) override;
    IMS_BOOL IsRegisteredConfig(IN const IAsyncConfig* piConfig);

private:
    /// This message will be used in the first argument in SendTo(...) method
    enum
    {
        AMSG_START = (IMS_MSG_USER + 1),
        /// Message base of configuration
        AMSG_SEND_TO = (IMS_MSG_USER + 11)
    };

    IMSList<IAsyncConfig*> m_objAsyncConfigs;
};

#endif
