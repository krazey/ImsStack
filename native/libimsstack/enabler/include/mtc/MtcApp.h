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

#include "Engine.h"
#include "IConfiguration.h"
#include "IMtcApp.h"
#include "IMtcContext.h"
#include "ImsApp.h"
#include "ImsList.h"
#include "MtcImsEventReceiver.h"
#include "call/CallConnectionIdManager.h"
#include "call/MtcCallController.h"
#include "call/MtcCallManager.h"
#include "call/radio/MtcRadioChecker.h"
#include "conferencecall/ConferenceManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/MtcDialingPlan.h"
#include "dialogevent/MultiEndpointManager.h"
#include "helper/CallStateProxy.h"
#include "helper/MtcLocationRefresher.h"
#include "helper/OperationAsyncRunnerManager.h"
#include "helper/PassiveTimerHolder.h"
#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "utility/MessageUtils.h"
#include <functional>
#include <memory>

class EctManager;
class ICallStateProxy;
class IConferenceManager;
class IEctManager;
class ILastComeFirstServedHelper;
class IMessageUtils;
class IMtcAosConnector;
class IMtcCallController;
class IMtcCallManager;
class IMtcDialingPlan;
class IMtcEmergencyServiceManager;
class IMtcImsEventReceiver;
class IMtcRadioChecker;
class IMtcService;
class IMtcSipInterfaceFactory;
class IMultiEndpointManager;
class IPassiveTimerHolder;
class LastComeFirstServedHelper;
class MtcTimerWrapper;
class OperationAsyncRunner;
class RttAutoUpgrader;
enum class ServiceType;

class MtcApp : public ImsApp, public IMtcApp, public IMtcContext
{
public:
    explicit MtcApp(IN IMS_SINT32 nSlotId);
    virtual ~MtcApp() override;
    MtcApp(IN const MtcApp&) = delete;
    MtcApp& operator=(IN const MtcApp&) = delete;

    // IMtcApp implementation
    virtual void Start() override;
    virtual void Stop() override;

    // IMtcContext implementation
    inline IMS_SINT32 GetSlotId() const override { return m_nSlotId; }
    inline const ISubscriberConfig* GetSubscriberConfig() const override
    {
        return Engine::GetConfiguration()->GetSubscriberConfig(GetSlotId());
    }
    IMtcService* GetServiceByType(IN ServiceType eServiceType) override;
    inline IMtcDialingPlan& GetDialingPlan() override { return m_objDialingPlan; }
    inline IMtcCallController& GetCallController() override { return m_objCallController; }
    inline IMtcRadioChecker& GetRadioChecker() override { return m_objMtcRadioChecker; }
    inline IMtcCallManager& GetCallManager() override { return m_objCallManager; }
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
    IEctManager& GetEctManager() override;
    IMtcEmergencyServiceManager& GetEmergencyServiceManager() override;
    void RunAsyncOperation(IN void* pOwner, IN std::function<void()> objOperation) override;
    inline void ReleaseAsyncOperation(IN void* pOwner) override
    {
        m_objOperationAsyncRunnerManager.Release(pOwner);
    }
    std::unique_ptr<MtcTimerWrapper> CreateTimer() override;
    inline IMessageUtils& GetMessageUtils() override { return m_objMessageUtils; }
    inline IPassiveTimerHolder& GetPassiveTimerHolder() override { return m_objPassiveTimerHolder; }
    inline IMultiEndpointManager* GetMultiEndpointManager() override
    {
        return m_pMultiEndpointManager.get();
    }
    ILastComeFirstServedHelper& GetLastComeFirstServedHelper() override;
    inline CallConnectionIdManager& GetCallConnectionIdManager() override
    {
        return m_objCallConnectionIdManager;
    }
    inline MtcLocationRefresher& GetLocationRefresher() override { return m_objLocationRefresher; }
    inline IMS_BOOL IsWifiTestMode() override { return m_bWifiTestMode; }
    void CreateRttAutoUpgrader() override;
    void DestroyRttAutoUpgrader() override;

protected:
    virtual void CreateServices();
    virtual void InitCallManager();
    virtual void DestroyServices();

protected:
    IMS_SINT32 m_nSlotId;
    MtcConfigurationProxy m_objConfigurationProxy;
    OperationAsyncRunnerManager m_objOperationAsyncRunnerManager;
    ImsList<IMtcService*> m_lstServices;
    MtcDialingPlan m_objDialingPlan;
    MtcCallManager m_objCallManager;
    MtcCallController m_objCallController;
    CallStateProxy m_objCallStateProxy;
    MtcImsEventReceiver m_objImsEventReceiver;
    MtcSipInterfaceFactory m_objSipInterfaceFactory;
    ConferenceManager m_objConferenceManager;
    std::unique_ptr<EctManager> m_pEctManager;
    std::unique_ptr<IMtcEmergencyServiceManager> m_pEmergencyServiceManager;
    MessageUtils m_objMessageUtils;
    PassiveTimerHolder m_objPassiveTimerHolder;
    std::unique_ptr<MultiEndpointManager> m_pMultiEndpointManager;
    MtcRadioChecker m_objMtcRadioChecker;
    std::unique_ptr<LastComeFirstServedHelper> m_pLastComeFirstServedHelper;
    CallConnectionIdManager m_objCallConnectionIdManager;
    MtcLocationRefresher m_objLocationRefresher;
    std::unique_ptr<RttAutoUpgrader> m_pRttAutoUpgrader;

    IMS_BOOL m_bWifiTestMode;
};

#endif
