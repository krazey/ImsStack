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
#ifndef CONFIG_APP_H_
#define CONFIG_APP_H_

#include "IEventListener.h"
#include "ImsApp.h"
#include "ImsMessageDef.h"

class ConfigApp : public ImsApp, public IEventListener
{
public:
    explicit ConfigApp(IN const AString& strAppName);
    ~ConfigApp() override;

public:
    void Start();

protected:
    // ImsApp class
    inline IMS_BOOL OnPreprocess(IN ImsMessage& /*objMsg*/) override { return IMS_FALSE; }
    IMS_BOOL OnMessage(IN ImsMessage& objMsg) override;
    inline IMS_BOOL OnPostprocess(IN ImsMessage& /*objMsg*/) override { return IMS_FALSE; }

    // IEventListener class
    void Event_NotifyEvent(
            IN IMS_SINT32 nEvent, IN IMS_UINT32 nWparam, IN IMS_UINT32 nLparam) override;

    virtual void UpdateAllForHidden(IN IMS_SINT32 nItem, IN IMS_SINT32 nParam);
    virtual void UpdateAllForDm(IN IMS_SINT32 nItem, IN IMS_SINT32 nParam);

    IMS_BOOL UpdateSipConifgV(
            IN IMS_SINT32 nCpi, IN const AString& strServiceId = AString::ConstNull());
    IMS_BOOL UpdateSubscriberConfig(IN IMS_SINT32 nCpi);

protected:
    /// Internal messages
    enum
    {
        AMSG_START = (IMS_MSG_USER + 1),
    };
};

#endif
