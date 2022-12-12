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
    explicit UceApp(IN const IMS_SINT32 nSlotId);
    virtual ~UceApp();
    /* ------------------------------------------------------------------------------------------
        Methods
    ---------------------------------------------------------------------------------------------
   */

protected:
    virtual IMS_BOOL OnPreprocess(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnPostprocess(IN IMSMSG& objMSG);
    virtual IImsActivityController* GetController() override;
    virtual IMS_BOOL Control(
            IN IMS_UINT32 nCmdType, IN IMS_UINTP nInParam, OUT IMS_UINTP* pnOutParam) override;
    virtual void NetworkWatcher_NotifyStatus(IN INetworkWatcher* piNetWatcherInfo) override;
    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    void StopTimer(IN IMS_UINT32 nType);
    void ClearTimer();

    // ITimerListener Interface
    virtual void Timer_TimerExpired(IN ITimer* piTimer) override;

    virtual void ImsAos_Connected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan) override;
    virtual void ImsAos_Connecting() override;
    virtual void ImsAos_Disconnecting(IN IMS_UINT32 nReason) override;
    virtual void ImsAos_Disconnected(IN IMS_UINT32 nReason) override;
    virtual void ImsAos_Suspended(IN IMS_UINT32 nReason) override;
    virtual void ImsAos_Resumed() override;

    virtual void ImsAosMonitor_Connected(IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan) override;
    virtual void ImsAosMonitor_Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState) override;

    ITimer* GetTimer();

    inline void NotifyJniEnablerSet() override {}

    // JNI -> Native
    virtual void SendPublishCmd(IMS_UINT32 key, IMS_UINT32 extended, IMS_UINT32 capability,
            AString pidfXml, AString eTag) override;
    virtual void SendSingleSubscribeCmd(IMS_UINT32 key, AString user) override;
    virtual void SendListSubscribeCmd(IMS_UINT32 key, IMSList<AString> userList) override;
    virtual void SendOptionsCmd(IMS_UINT32 key, IMS_UINT32 myCaps, AString remoteUri) override;
    virtual void SendOptionsRespCmd(
            IMS_UINT32 key, IMS_SINT32 responseCode, AString reason, IMS_UINT32 myCaps) override;
    virtual void ImsRegistrationCheck() override;

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

private:
    enum
    {
        AOS_CONNECTED,
        AOS_DISCONNECTING,
        AOS_DISCONNECTED,
        AOS_SUSPENDED,
        AOS_RESUMED,
    };

    IImsAos* m_piImsAos;

    INetworkWatcher* m_piNetWatcherInfo;
    ITimer* m_piDeBounceTimer;
    IMS_SINT32 m_RegisteredNetwork;
    IMS_SINT32 m_eCurrentNetwork;
    UceService* m_pUceService;
};
#endif /* _UCE_APP_H_ */
