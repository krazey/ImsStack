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
#ifndef _UCE_APP_H_
#define _UCE_APP_H_

#include "IImsActivityController.h"
#include "ImsApp.h"
#include "INetworkWatcher.h"
#include "ITimer.h"
#include "aos/IImsAosListener.h"
#include "aos/IImsAosMonitor.h"
#include "IUceJni.h"

class IImsAos;
class UceService;
class IUceJniThread;

class UceApp :
        public ImsApp,
        public IImsActivityController,
        public IImsAosListener,
        public IImsAosMonitor,
        public INetworkWatcherListener,
        public ITimerListener,
        public IUceJni
{
    /* ------------------------------------------------------------------------------------------
        Constructor, Destructor, Operator Overloading
    ---------------------------------------------------------------------------------------------
  */
public:
    explicit UceApp(IN const IMS_SINT32 nSlotId, IN const AString& strAppName);
    explicit UceApp(IN const IMS_SINT32 nSlotId, IN IImsAos* piImsAos);
    virtual ~UceApp() override;
    /* ------------------------------------------------------------------------------------------
        Methods
    ---------------------------------------------------------------------------------------------
   */

protected:
    virtual IMS_BOOL OnPreprocess(IN IMSMSG& objMSG) override;
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG) override;
    virtual IMS_BOOL OnPostprocess(IN IMSMSG& objMSG) override;
    virtual IImsActivityController* GetController() override;
    virtual IMS_BOOL Control(
            IN IMS_UINT32 nCmdType, IN IMS_UINTP nInParam, OUT IMS_UINTP* pnOutParam) override;
    virtual void NetworkWatcher_NotifyStatus(IN INetworkWatcher* piNetWatcherInfo) override;
    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    void StopTimer(IN IMS_UINT32 nType);
    void ClearTimer();
    IMS_UINT32 GetRegisteredService(IN IMS_UINT32 features);

    // ITimerListener Interface
    virtual void Timer_TimerExpired(IN ITimer* piTimer) override;

    virtual void ImsAos_Connected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan) override;
    virtual void ImsAos_Connecting() override;
    virtual void ImsAos_Disconnecting(IN IMS_UINT32 nReason) override;
    virtual void ImsAos_Disconnected(
            IN IMS_UINT32 nReason, IN IMS_SINT32 nDataFailureReason) override;
    virtual void ImsAos_Suspended(IN IMS_UINT32 nReason) override;
    virtual void ImsAos_Resumed() override;

    virtual void ImsAosMonitor_Connected(IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan) override;
    virtual void ImsAosMonitor_Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState) override;

    ITimer* GetTimer();

    inline void NotifyJniEnablerSet() override {}

    // JNI -> Native
    virtual IMS_BOOL SendPublishCmd(IMS_UINT32 key, IMS_UINT32 extended, IMS_UINT32 capability,
            const AString& pidfXml, const AString& eTag) override;
    virtual IMS_BOOL SendSingleSubscribeCmd(IMS_UINT32 key, const AString& user) override;
    virtual IMS_BOOL SendListSubscribeCmd(
            IMS_UINT32 key, const ImsList<AString>& userList) override;
    virtual IMS_BOOL SendOptionsCmd(
            IMS_UINT32 key, IMS_UINT32 myCaps, const AString& remoteUri) override;
    virtual IMS_BOOL SendOptionsRespCmd(IMS_UINT32 key, IMS_SINT32 responseCode,
            const AString& reason, IMS_UINT32 myCaps) override;
    virtual IMS_BOOL ImsRegistrationCheck() override;

private:
    void EnableUceService(void);
    void DisableUceService(void);
    IMS_SINT32 GetServiceNetworkType(IN IMS_UINT32 nIpcan);
    IMS_SINT32 ConvertNetworkType(IN IMS_SINT32 nRAT);
    void NotifyRATChanged(void);
    void EnableAllAoSApps();
    void SelectActiveAoSApp();
    void SetPublishStatusToAos(IN IMS_BOOL bIsPublishStarted);
    void SendRegistrationRecoveryRequestToAos(IN IMS_UINT32 nAosControlType);
    IUceJniThread* GetJniThread();
    /* ------------------------------------------------------------------------------------------
        Variables
    ---------------------------------------------------------------------------------------------
  */
protected:
    enum
    {
        AMSG_BASE = IMS_MSG_USER + 1,
        AMSG_CREATE_SERVICE,
        AMSG_MAX
    };
    enum
    {
        TIMER_NETWORK_CHANGED = 0,
    };
    IMS_SINT32 m_nSlotId;
    AString m_strAppName;
    AString m_strAppID;
    AString m_strServiceID;
    IMS_SINT32 m_eAoSStatus;
    INetworkWatcher* m_piNetWatcherInfo;
    IMS_SINT32 m_eCurrentNetwork;
    ITimer* m_piDeBounceTimer;
    UceService* m_pUceService;

private:
    enum
    {
        AOS_CONNECTING,
        AOS_CONNECTED,
        AOS_DISCONNECTING,
        AOS_DISCONNECTED,
        AOS_SUSPENDED,
        AOS_RESUMED,
    };

    IImsAos* m_piImsAos;
    IMS_SINT32 m_RegisteredNetwork;
};
#endif /* _UCE_APP_H_ */
