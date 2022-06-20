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
#ifndef IMS_APP_H_
#define IMS_APP_H_

#include "ImsActivity.h"
#include "ImsService.h"

class ImsApp : public ImsActivity
{
public:
    ImsApp(IN const AString& strAppName);
    virtual ~ImsApp();

    ImsApp(IN const ImsApp&) = delete;
    ImsApp& operator=(IN const ImsApp&) = delete;

public:
    IMS_BOOL AttachService(IN ImsService* pService);
    void DetachService(IN ImsService* pService);
    ImsService* GetService(IN const AString& strServiceName);
    inline const IMSList<ImsService*> GetServices() { return m_objServices; }

protected:
    inline virtual IMS_BOOL OnPreprocess(IN ImsMessage& /*objMsg*/) { return IMS_FALSE; }
    inline virtual IMS_BOOL OnMessage(IN ImsMessage& /*objMsg*/) { return IMS_FALSE; }
    inline virtual IMS_BOOL OnPostprocess(IN ImsMessage& /*objMsg*/) { return IMS_FALSE; }

    inline IImsActivityController* GetController() override { return IMS_NULL; }

private:
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

private:
    IMSList<ImsService*> m_objServices;
};

// Definition of function pointer to create the ImsApp derived classes
typedef ImsApp* (*ImsApp_Creator)(IN const AString& strName);

#endif
