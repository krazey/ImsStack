#ifndef MOCK_AOS_APPLICATION_H_
#define MOCK_AOS_APPLICATION_H_

#include <gmock/gmock.h>

#include "IMSActivityEx.h"
#include "IMSStateMachine.h"
#include "IEventListener.h"
#include "ITimer.h"
#include "interface/AosInternalMsgDef.h"
#include "AoSReason.h"
#include "interface/IAosApplication.h"
#include "interface/IAosBlock.h"
#include "interface/IAosCallTrackerListener.h"
#include "interface/IAosConditionListener.h"
#include "interface/IAosConnectorListener.h"
#include "interface/IAosNetTrackerListener.h"
#include "interface/IAosRegistrationControlListener.h"
#include "interface/IAosRegistrationListener.h"
#include "provider/AosStaticProfile.h"
#include "app/AosApplication.h"

class IAosAppContext;
class IAosRegistration;
class IAosCallTracker;
class IAosNetTracker;
class AosCondition;
class AosConnector;
class AosUtil;

class MockAosApplication : public AosApplication
{
public:
    MockAosApplication(IN IAosAppContext* piAppContext, IN AString& strAppId) :
            AosApplication(piAppContext, strAppId)
    {
    }
    ~MockAosApplication() {}
    MOCK_METHOD(void, Reconfig, (), (override));
    MOCK_METHOD(IMS_BOOL, RequestCmd, (IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(const AString&, GetActivityName, (), (override));
    MOCK_METHOD(void, GetProperty,
            (IN IMS_UINT32 nType, OUT IMS_UINT32& nValue, OUT AString& strValue), (override));
    MOCK_METHOD(IMS_UINT32, GetAppState, (), (override));
    MOCK_METHOD(IMS_UINT32, GetOffReason, (), (override));
    MOCK_METHOD(IMS_BOOL, IsActivated, (), (override));
    MOCK_METHOD(IMS_BOOL, IsOn, (), (override));
    MOCK_METHOD(void, SetActivation, (IN IMS_BOOL bActivation), (override));
    MOCK_METHOD(void, NotifyPublishState, (IN IMS_BOOL bStart), (override));
    MOCK_METHOD(void, CreateAosCondition, (), (override));
    MOCK_METHOD(void, CreateAosConnector, (), (override));
    MOCK_METHOD(void, CreateAosLocationStarter, (IN IMS_BOOL bInitiation), (override));
    MOCK_METHOD(void, AddEventListener, (), (override));
    MOCK_METHOD(void, RemoveEventListener, (), (override));
    MOCK_METHOD(void, SetNetTrackerListener, (), (override));
    MOCK_METHOD(void, SetAppType, (IN AosRegistrationType eRegType), (override));
    MOCK_METHOD(void, SetAppState, (IN IMS_UINT32 nState), (override));
    MOCK_METHOD(void, SetCleanState, (), (override));
    MOCK_METHOD(IMS_BOOL, IsUpdateAvailable, (), (override));
    MOCK_METHOD(IMS_BOOL, IsRegReconfigAvailable, (), (override));
    MOCK_METHOD(IMS_BOOL, IsReconfigHandleChanged, (), (override));
    MOCK_METHOD(IMS_BOOL, IsRequestCmdHeldByCondition,
            (IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(IMS_BOOL, IsAllHandleDetached, (), (override));
    MOCK_METHOD(IMS_BOOL, IsConditionTimerSkippedDueToTimer, (), (override));
    MOCK_METHOD(IMS_BOOL, IsRegUpdatedByNrLteRatChange, (), (override));
    MOCK_METHOD(void, CleanAll, (IN IMS_UINT32 nOffReason), (override));
    MOCK_METHOD(void, ClearConnection, (), (override));
    MOCK_METHOD(IMS_UINT32, GetReportState, (), (override));
    MOCK_METHOD(IMS_BOOL, OnMessage, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, ProcessMessage, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessRegStart, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessRegUpdate, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessRegStop, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessRegReconfig, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessRegRecovery, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessIpcanChanged, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessDestroy, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessServiceControl, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessRegExchange, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessAutoConfigurationComplete, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessPcscfRecovery, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessScscfRestoration, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessOthers, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, PreprocessStateMessage, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, PreprocessStateMessage_Connection, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateNotReady_Condition, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateNotReady_Connection, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateReady_Condition, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateReady_Connection, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateConnecting_Condition, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateConnecting_Connection, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateConnecting_Registration, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateConnected_Condition, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateConnected_Connection, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateConnected_Registration, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateUpdating_Condition, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateUpdating_Connection, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateUpdating_Registration, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateDisconnecting_Condition, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateDisconnecting_Connection, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, StateDisconnecting_Registration, (IN IMSMSG & objMsg), (override));
    MOCK_METHOD(void, ProcessRegTrying_StateConnecting, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessRegFailed_StateConnecting, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessRegTrying_StateConnected, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessRegFailed_StateConnected, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessRegTrying_StateUpdating, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessRegFailed_StateUpdating, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(
            void, ProcessConnectionUpdated_StateDisconnecting, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessConnectionDeactivated, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessConnectionUpdated, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessConnectionUpdated_Pcscf, (), (override));
    MOCK_METHOD(void, ProcessRegSucceeded, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessRegFailed_Start, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessRegFailed_Update, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessRegFailed_Terminated, (), (override));
    MOCK_METHOD(void, ProcessDisconnectingState, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessIpcanHandoverEvent,
            (IN IMS_UINT32 nResult, IN IMS_UINT32 nPreferredRat), (override));
    MOCK_METHOD(void, ProcessNetworkEvent, (IN IMS_UINT32 nType, IN IMS_UINT32 nState), (override));
    MOCK_METHOD(void, ProcessStateStart, (IN IMS_UINT32 nTime), (override));
    MOCK_METHOD(
            void, ProcessRegControlEvent, (IN IMS_UINT32 nType, IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessRegInternalFailed, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ProcessRegAuthenticationFailed, (), (override));
    MOCK_METHOD(void, ProcessRegTerminated, (), (override));
    MOCK_METHOD(void, ProcessPingCommand, (), (override));
    MOCK_METHOD(void, ProcessPdnDisconnect, (), (override));
    MOCK_METHOD(void, ProcessAppActivatedTimerExpired, (), (override));
    MOCK_METHOD(void, ProcessAppConnectedTimerExpired, (), (override));
    MOCK_METHOD(void, ProcessAppTerminatedTimerExpired, (), (override));
    MOCK_METHOD(void, ProcessReconfigTimerExpired, (), (override));
    MOCK_METHOD(void, ProcessRegBlockedTimerExpired, (), (override));
    MOCK_METHOD(void, ProcessRegStopTimerExpired, (), (override));
    MOCK_METHOD(void, ProcessPdnBlockedTimerExpired, (), (override));
    MOCK_METHOD(void, ProcessPdnBlock, (), (override));
    MOCK_METHOD(void, ProcessPdnBlockWithTime, (), (override));
    MOCK_METHOD(void, OnAppStateChanged, (), (override));
    MOCK_METHOD(void, Report_StateChanged, (IN IMS_BOOL bIsStateChecked), (override));
    MOCK_METHOD(void, Report_Notify, (), (override));
    MOCK_METHOD(void, Report_Request, (IN IMS_UINT32 nType, IN IMS_UINT32 nState), (override));
    MOCK_METHOD(IMS_BOOL, UpdateRegRecoveryHeld, (), (override));
    MOCK_METHOD(IMS_BOOL, UpdateRegStopHeld, (), (override));
    MOCK_METHOD(void, StartTimer, (IN IMS_UINT32 nType, IN IMS_UINT32 nDuration), (override));
    MOCK_METHOD(void, StopTimer, (IN IMS_UINT32 nType), (override));
    MOCK_METHOD(void, ClearTimers, (), (override));
    MOCK_METHOD(void, UpdateRegState, (), (override));
    MOCK_METHOD(IMS_UINT32, UpdateConnectedServices, (IN IMS_BOOL bEnforceUpdateRegService),
            (override));
    MOCK_METHOD(void, UpdateRegisteredRat, (IN IMS_UINT32 nRegisteredRat), (override));
    MOCK_METHOD(void, UpdateMonitorNotify, (IN IMS_UINT32 nType, IN IMS_UINT32 nState), (override));
    MOCK_METHOD(void, Condition_Changed, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, Condition_RequestCommand, (IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason),
            (override));
    MOCK_METHOD(void, Connector_Activated, (), (override));
    MOCK_METHOD(void, Connector_Deactivated, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, Connector_Updated, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, Registration_StateChanged, (IN IMS_UINT32 nResult, IN IMS_UINT32 nReason),
            (override));
    MOCK_METHOD(void, Registration_PreNotify, (IN IMS_UINT32 nPreReason), (override));
    MOCK_METHOD(
            void, CallTracker_StateChanged, (IN IMS_UINT32 nType, IN CallState eState), (override));
    MOCK_METHOD(void, NetTracker_StatusChanged, (), (override));
    MOCK_METHOD(void, Event_NotifyEvent,
            (IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam), (override));
    MOCK_METHOD(void, Timer_TimerExpired, (IN ITimer * piTimer), (override));
    MOCK_METHOD(void, RegistrationControl_ControlRegistration,
            (IN AosRegRequestType eType, IN AosPcscfOrder eOrder, IN AosControlCause eCause),
            (override));
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, CleanUp, (), (override));
};

#endif  // MOCK_AOS_APPLICATION_H_
