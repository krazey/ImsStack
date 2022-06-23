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

#ifndef MTC_APP_H_
#define MTC_APP_H_

#include "ImsApp.h"

#include "MtcImsEventReceiver.h"
#include "helper/CallStateProxy.h"
#include "IMtcApp.h"
#include "call/IMtcCallManager.h"
#include "IMtcContext.h"
#include "IMtcService.h"
#include "call/MtcCallController.h"
#include "call/MtcCallManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/MtcDialingPlan.h"
#include "vonr/MtcVonrManager.h"
#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "conferencecall/ConferenceManager.h"

class EctManager;
class MtcEmergencyServiceManager;
class IMtcCallController;
class IMtcCallManager;
class IMtcDialingPlan;
class IMtcVonrManager;
class ICallStateProxy;
class IMtcImsEventReceiver;
class IMtcAosConnector;
class IMtcSipInterfaceFactory;
class IConferenceManager;
class IEctManager;

class MtcApp : public ImsApp, public IMtcApp, public IMtcContext
{
public:
    MtcApp(IN IMS_SINT32 nSlotId);
    virtual ~MtcApp();
    MtcApp(IN const MtcApp&) = delete;
    MtcApp& operator=(IN const MtcApp&) = delete;

    // IMtcApp implementation
    virtual void Start() override;
    virtual void Stop() override;

    // IMtcContext implementation
    inline IMS_SINT32 GetSlotId() override { return m_nSlotId; }
    IMtcService* GetServiceByType(IN ServiceType eServiceType) override;
    inline IMtcDialingPlan& GetDialingPlan() override { return m_objDialingPlan; }
    inline IMtcCallController& GetCallController() override { return m_objCallController; }
    inline IMtcCallManager& GetCallManager() override { return m_objCallManager; }
    inline IMtcVonrManager& GetVonrManager() override { return m_objVonrManager; }
    inline MtcConfigurationProxy& GetConfigurationProxy() override
    {
        return m_objConfigurationProxy;
    }
    inline ICallStateProxy& GetCallStateProxy() override { return m_objCallStateProxy; }
    inline IMtcImsEventReceiver& GetImsEventReceiver() override { return m_objImsEventReceiver; }
    IMtcAosConnector* GetAosConnector(IN ServiceType eServiceType) override;
    inline IMtcSipInterfaceFactory& GetSipInterfaceFactory() override
    {
        return m_objSipInterfaceFactory;
    }
    inline IConferenceManager& GetConferenceManager() override { return m_objConferenceManager; }
    IEctManager* GetEctManager() override;
    MtcEmergencyServiceManager* GetEmergencyServiceManager() override;

private:
    void InitConfiguration();
    void CreateServices();
    void InitCallManager();
    void DestroyServices();

protected:
    IMS_SINT32 m_nSlotId;
    MtcConfigurationProxy m_objConfigurationProxy;
    IMSList<IMtcService*> m_lstServices;
    MtcDialingPlan m_objDialingPlan;
    MtcCallManager m_objCallManager;
    MtcCallController m_objCallController;
    MtcVonrManager m_objVonrManager;
    CallStateProxy m_objCallStateProxy;
    MtcImsEventReceiver m_objImsEventReceiver;
    MtcSipInterfaceFactory m_objSipInterfaceFactory;
    ConferenceManager m_objConferenceManager;
    EctManager* m_pEctManager;
    MtcEmergencyServiceManager* m_pEmergencyServiceManager;
};

#endif
