package com.android.imsstack.core.service;

import android.content.Context;
import android.os.Handler;
import android.os.Message;

import com.android.imsstack.core.CapabilityConfigs;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ImsPhoneProxyApi;
import com.android.imsstack.core.agents.agentif.IAlarmTimer;
import com.android.imsstack.core.agents.agentif.ICallStateNotificationService;
import com.android.imsstack.core.config.ImsDbController;
import com.android.imsstack.core.config.ProviderInterface;
import com.android.imsstack.core.service.serviceif.IService;
import com.android.imsstack.core.service.serviceif.IVoLteService;
import com.android.imsstack.enabler.mtc.IUMtcCall;
import com.android.imsstack.system.IJNIUpCallEvt;
import com.android.imsstack.system.JNIUpCallEvtManager;
import com.android.imsstack.util.DBUtils;
import com.android.imsstack.util.ImsLog;

import java.util.HashMap;
import java.util.Iterator;
import java.util.concurrent.CopyOnWriteArrayList;

public class CallStateNotificationService implements
        IService, ICallStateNotificationService, CallSysModeTracker.IListener {

    /* RIL constants */
    private static final int RIL_CALL_TYPE_INVALID = 0;
    private static final int RIL_CALL_TYPE_VOICE = 1;
    private static final int RIL_CALL_TYPE_VIDEO = 2;
    private static final int RIL_CALL_TYPE_ECALL = 5;

    private static final int RIL_CALL_STATE_END = 0;
    private static final int RIL_CALL_STATE_START = 1;
    private static final int RIL_CALL_STATE_CONNECT = 2; // whole state only.

    private static final int RIL_CALL_DIR_NONE = -1;
    private static final int RIL_CALL_DIR_MO = 0;
    private static final int RIL_CALL_DIR_MT = 1;

    // VoNR: 5G_NR_SA_MODE_CONTROL {
    private static final int EVENT_TIMER_ENABLE_SA = 1;
    private static final int EVENT_TIMER_SEND_CALL_IDLE = 2;
    private static final int NR_SA_DISABLED = 1;
    private static final int NR_SA_ENABLED = 0;
    private static final int MAX_DELAY_FOR_ENABLE_SA_ON_IDLE = 2000;
    private static final int MAX_DELAY_FOR_CALL_IDLE = 1000;
    // }

    private IVoLteService mVoLteService = null;
    private CallSysModeTracker mCallSysModeTracker = null;
    private ImsPhoneProxyApi mIPPApi = null;

    private HashMap<Integer, Integer> mSysModeByIpCan;

    private final CopyOnWriteArrayList<CategorizedCallInfoList> mCategories
            = new CopyOnWriteArrayList<CategorizedCallInfoList>();

    private int mTotalCallState = RIL_CALL_STATE_END;

    private boolean mEpdnUsedForWiFi;
    private boolean mTestModeEnabled;

    // VoNR: 5G_NR_SA_MODE_CONTROL {
    private MessageHandler mHandler = null;
    private boolean mAvailableForNrSaMode = false;
    private int mNrSaMode = NR_SA_ENABLED;
    private int mEnableSaTimer = 0;
    private int mSendCallIdleTimer = 0;
    private CallInfoCache mCallInfoForIdle = null;
    // }

    public CallStateNotificationService() {
    }

    @Override
    public boolean start(IVoLteService voLteService) {
        mVoLteService = voLteService;
        ImsLog.i(getSlotId(), "");

        final IJNIUpCallEvt jniEvt =
                JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(getSlotId());
        if (jniEvt != null) {
            jniEvt.setInterfaceForCallStateNotificationService(this);
        }

        startSysModeTracker();
        mEpdnUsedForWiFi = isEmergencyPdnUsedForEmergencyCallViaWfc();
        mTestModeEnabled = isTestModeEnabled();
        mIPPApi = new ImsPhoneProxyApi(getSlotId());

        mAvailableForNrSaMode = CapabilityConfigs.isVoNrEnabled(getSlotId());

        if (mAvailableForNrSaMode) {
            mHandler = new MessageHandler();
            sendNrSaMode(NR_SA_ENABLED);
        }

        return true;
    }

    @Override
    public void cleanup(Context context) {
        ImsLog.i(getSlotId(), "");

        stopEnableSaTimer();
        setNrSaMode(NR_SA_ENABLED);
        stopSendCallIdleTimer();
        sendDelayedCallIdle();

        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }

        final IJNIUpCallEvt jniEvt =
                JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(getSlotId());
        if (jniEvt != null) {
            jniEvt.setInterfaceForCallStateNotificationService(null);
        }

        if (mCallSysModeTracker != null) {
            mCallSysModeTracker.clear();
        }

        clearCategories();
        mTotalCallState = RIL_CALL_STATE_END;
    }

    @Override
    public void update(Context context) {
    }

    @Override
    public void onIpCanChanged(int apnType) {
        ImsLog.i(getSlotId(), "apnType=" + apnType);

        int newSysMode = getCurrentSysMode(apnType);
        if (newSysMode == CallSysModeTracker.RIL_SYS_MODE_UNKNOWN) {
            return;
        }

        if (newSysMode == getLastSysMode(apnType)) {
            return;
        }

        updateRilCallStateByIpCanChanged(apnType, newSysMode);
        mSysModeByIpCan.put(apnType, newSysMode);

        if (mTotalCallState == RIL_CALL_STATE_CONNECT) {
            if (newSysMode == CallSysModeTracker.RIL_SYS_MODE_WIFI) {
                setNrSaMode(NR_SA_DISABLED);
            }
        }
    }

    @Override
    public void setCallState(int key, int param) {
        if (key == 0) {
            return;
        }
        int sessionKey = key;
        boolean updated = false;

        CallInfo callInfo = getCallInfo(sessionKey);
        if (callInfo != null) {
            callInfo.setInfo(param);
            updateSysMode(callInfo);
            updated = updateCallInfo(callInfo);
        } else {
            callInfo = new CallInfo(sessionKey, param);

            if (callInfo.mState == RIL_CALL_STATE_END
                    || updateSysMode(callInfo) == false) {
                ImsLog.i(getSlotId(), "invalid call is added.");
                return;
            }
            updated = addCallInfo(callInfo);
        }

        if (getAllCallSize() > 1) {
            mTotalCallState = RIL_CALL_STATE_CONNECT;
        } else {
            mTotalCallState = callInfo.mState;
        }

        ImsLog.i(getSlotId(), "TotalState=" + mTotalCallState + " updated=" + updated);

        if (updated == true) {
            setNrSaModeOnCallStateChanged();
            sendToRil(key);
            removeEndedCallInfo(key);
        }
    }

    private void startSysModeTracker() {
        mCallSysModeTracker = new CallSysModeTracker(getSlotId());

        mSysModeByIpCan = new HashMap<Integer, Integer>();
        mSysModeByIpCan.put(CallSysModeTracker.APN_TYPE_IMS,
                CallSysModeTracker.RIL_SYS_MODE_UNKNOWN);
        mSysModeByIpCan.put(CallSysModeTracker.APN_TYPE_EMERGENCY,
                CallSysModeTracker.RIL_SYS_MODE_UNKNOWN);
    }

    private void clearCategories() {
        Iterator<CategorizedCallInfoList> it = mCategories.iterator();

        while (it.hasNext()) {
            CategorizedCallInfoList cur = it.next();

            cur.clear();
        }

        mCategories.clear();
    }

    private CategorizedCallInfoList getCategory(CallInfo callInfo) {
        Iterator<CategorizedCallInfoList> it = mCategories.iterator();

        while (it.hasNext()) {
            CategorizedCallInfoList cur = it.next();

            if (cur.isCategory(callInfo)) {
                return cur;
            }
        }

        ImsLog.i(getSlotId(), "getCategory null");
        return null;
    }

    private int getAllCallSize() {
        int size = 0;
        Iterator<CategorizedCallInfoList> it = mCategories.iterator();

        while (it.hasNext()) {
            CategorizedCallInfoList cur = it.next();

            size += cur.getSize();
        }

        return size;
    }

    private void removeEndedCallInfo(int sessionKey) {
        ImsLog.i(getSlotId(), "sessionKey=" + sessionKey);

        CallInfo callInfo = getCallInfo(sessionKey);

        if (callInfo == null) {
            return;
        }

        if (callInfo.mState == RIL_CALL_STATE_END) {
            removeCallInfo(callInfo);
        }
    }

    private CallInfo getCallInfo(int sessionKey) {

        Iterator<CategorizedCallInfoList> it = mCategories.iterator();

        while (it.hasNext()) {
            CategorizedCallInfoList cur = it.next();
            CallInfo temp = cur.getCallInfo(sessionKey);
            if (temp != null) {
                return temp;
            }
        }

        return null;
    }

    private boolean addCallInfo(CallInfo callInfo) {

        ImsLog.i(getSlotId(), "addCallInfo");

        CategorizedCallInfoList category = getCategory(callInfo);
        if (category == null) {
            category = new CategorizedCallInfoList(callInfo);
            mCategories.add(category);
        }

        return category.add(callInfo);
    }

    private void removeCallInfo(CallInfo callInfo) {

        CategorizedCallInfoList category = getCategory(callInfo);
        if (category == null) {
            return;
        }

        category.remove(callInfo);
        if (category.getSize() == 0) {
            category.clear();
            mCategories.remove(category);
            ImsLog.i(getSlotId(), "this category is empty. category count=" + mCategories.size());
        }
    }

    private boolean updateCallInfo(CallInfo callInfo) {

        ImsLog.i(getSlotId(), "updateCallInfo");

        CategorizedCallInfoList category = getCategory(callInfo);
        if (category == null) {
            return false;
        }

        return category.updateState(callInfo);
    }

    private void sendToRil(int sessionKey) {

        CallInfo callInfo = getCallInfo(sessionKey);
        if (callInfo == null) {
            return;
        }

        int sysMode = getLastSysMode(callInfo.mApnType);
        ImsLog.i(getSlotId(), "type=" + callInfo.mType + " state=" + callInfo.mState
                + " sysMode=" + sysMode + " dir=" + callInfo.mDir
                + " totalState=" + mTotalCallState + " apn=" + callInfo.mApnType);

        stopSendCallIdleTimer();

        if (mNrSaMode == NR_SA_DISABLED
                && mTotalCallState == RIL_CALL_STATE_END) {
            synchronized (this) {
                mCallInfoForIdle = CallInfoCache.createFrom(callInfo, sysMode);
            }
            return;
        }

        boolean sendCallIdle = false;

        synchronized (this) {
            // index-4: call-type, index-7: call-direction
            if (mCallInfoForIdle != null) {
                if (mCallInfoForIdle.mType != callInfo.mType
                        || mCallInfoForIdle.mDir != callInfo.mDir) {
                    sendCallIdle = true;
                } else {
                    mCallInfoForIdle = null;
                }
            }
        }

        if (isPendingRequiredCall(callInfo)) {
            sendToRilSync(callInfo.mType, callInfo.mState, sysMode, callInfo.mDir);
        } else {
            sendToRilAsync(callInfo.mType, callInfo.mState, sysMode, callInfo.mDir);
        }

        if (sendCallIdle) {
            if (mHandler != null) {
                mHandler.postDelayed(() -> {
                    sendDelayedCallIdle();
                }, 100);
            } else {
                android.os.SystemClock.sleep(100);
                sendDelayedCallIdle();
            }
        }
    }

    private void sendToRilAsync(int type, int state, int sysMode, int dir) {
        //setImsCallStateForMSim(type, state, sysMode, dir, mTotalCallState);
    }

    private void sendToRilSync(int type, int state, int sysMode, int dir) {
        if (mIPPApi != null) {
            mIPPApi.setImsCallStateForMSim(type, state, sysMode, dir, mTotalCallState);
        }
    }

    private boolean isPendingRequiredCall(CallInfo callInfo) {

        if (getAllCallSize() > 1) {
            return false;
        }

        if (callInfo.mDir != RIL_CALL_DIR_MO) {
            return false;
        }

        if (callInfo.mType == RIL_CALL_TYPE_ECALL) {
            return false;
        }

        if (callInfo.mState != RIL_CALL_STATE_START) {
            return false;
        }

        return true;
    }

    private void updateRilCallStateByIpCanChanged(int apnType, int newSysMode) {

        Iterator<CategorizedCallInfoList> it = mCategories.iterator();

        while (it.hasNext()) {
            CategorizedCallInfoList cur = it.next();
            if (cur.mApnType != apnType) {
                continue;
            }

            sendToRilAsync(cur.mType, cur.mTotalState, newSysMode, cur.mDirection);
        }
    }

    private boolean updateSysMode(CallInfo callInfo) {

        int sysMode = getCurrentSysMode(callInfo.mApnType);
        if (sysMode == CallSysModeTracker.RIL_SYS_MODE_UNKNOWN) {
            if (mTestModeEnabled) {
                return true;
            } else {
                ImsLog.i(getSlotId(), "invalid sys mode");
                return false;
            }
        }
        mSysModeByIpCan.put(callInfo.mApnType, sysMode);
        return true;
    }

    private int getCurrentSysMode(int apnType) {
        if (mCallSysModeTracker == null) {
            return CallSysModeTracker.RIL_SYS_MODE_UNKNOWN;
        }

        return mCallSysModeTracker.getSysMode(apnType);
    }

    private int getLastSysMode(int apnType) {
        return mSysModeByIpCan.getOrDefault(apnType, CallSysModeTracker.RIL_SYS_MODE_UNKNOWN);
    }

    private Context getContext() {
        return (mVoLteService != null) ? mVoLteService.getContext() : null;
    }

    private int getSlotId() {
        return (mVoLteService != null) ? mVoLteService.getSlotID() : 0;
    }

    private boolean isEmergencyPdnUsedForEmergencyCallViaWfc() {

        Context context = getContext();
        if (context == null) {
            return false;
        }

        int callViaEPdn = DBUtils.CP.getInt(
                getSlotId(),
                context.getContentResolver(),
                ProviderInterface.UCEmergency.CONTENT_URI,
                ProviderInterface.UCEmergency.USINGEPDN_WIFI, 1);

        return (callViaEPdn == 1);
    }

    private boolean isTestModeEnabled() {

        int adminFeatures = ImsDbController.Subscriber.getAdminFeatures(getSlotId());
        return ImsDbController.isTestModeEnabled(adminFeatures);
    }

    // VoNR: 5G_NR_SA_MODE_CONTROL {
    private void sendNrSaMode(int nrSaMode) {
        /*setModemInfo(12 MODEM_I_SET_NR_SA_MODE,
                1 MODEM_I_DATA_TYPE_INT, nrSaMode, "");*/
    }

    private void setNrSaMode(int nrSaMode) {
        if (!mAvailableForNrSaMode) {
            return;
        }

        if (mNrSaMode == nrSaMode) {
            return;
        }

        ImsLog.i(getSlotId(), "setNrSaMode: " + mNrSaMode + " >> " + nrSaMode);

        mNrSaMode = nrSaMode;

        sendNrSaMode(mNrSaMode);
    }

    private void setNrSaModeOnCallStateChanged() {
        if (!mAvailableForNrSaMode) {
            return;
        }

        if (mTotalCallState > RIL_CALL_STATE_END) {
            stopEnableSaTimer();
        }

        if (mTotalCallState == RIL_CALL_STATE_END) {
            stopEnableSaTimer();

            if (mNrSaMode == NR_SA_DISABLED) {
                int sysMode = getCurrentSysMode(CallSysModeTracker.APN_TYPE_IMS);

                if (sysMode == CallSysModeTracker.RIL_SYS_MODE_WIFI) {
                    setNrSaMode(NR_SA_ENABLED);
                } else if (startEnableSaTimer()) {
                    ImsLog.i(getSlotId(), "NrSaMode: enable SA (delay) - S");
                } else {
                    setNrSaMode(NR_SA_ENABLED);
                }
            }
        } else if (mTotalCallState == RIL_CALL_STATE_CONNECT) {
            int sysMode = getCurrentSysMode(CallSysModeTracker.APN_TYPE_IMS);

            if (sysMode == CallSysModeTracker.RIL_SYS_MODE_WIFI) {
                setNrSaMode(NR_SA_DISABLED);
            } else {
                // Do not enable SA before call ends
                // setNrSaMode(NR_SA_ENABLED);
            }
        }
    }

    private boolean sendDelayedCallIdle() {
        CallInfoCache callInfo = null;

        synchronized (this) {
            callInfo = mCallInfoForIdle;
            mCallInfoForIdle = null;
        }

        if (callInfo != null) {
            ImsLog.i(getSlotId(), "sendDelayedCallIdle");
            sendToRilAsync(callInfo.mType, callInfo.mState, callInfo.mSysMode, callInfo.mDir);
            return true;
        }

        return false;
    }

    private boolean startEnableSaTimer() {
        mEnableSaTimer = startTimer(
                MAX_DELAY_FOR_ENABLE_SA_ON_IDLE,
                EVENT_TIMER_ENABLE_SA,
                "NrSaMode-EnableSa");
        return isTimerStarted(mEnableSaTimer);
    }

    private void stopEnableSaTimer() {
        stopTimer(mEnableSaTimer, "NrSaMode-EnableSa");
        mEnableSaTimer = -1;
    }

    private boolean startSendCallIdleTimer() {
        mSendCallIdleTimer = startTimer(
                MAX_DELAY_FOR_CALL_IDLE,
                EVENT_TIMER_SEND_CALL_IDLE,
                "NrSaMode-sendCallIdle");
        return isTimerStarted(mSendCallIdleTimer);
    }

    private void stopSendCallIdleTimer() {
        stopTimer(mSendCallIdleTimer, "NrSaMode-sendCallIdle");
        mSendCallIdleTimer = -1;
    }

    private int startTimer(int duration, int event, String logTag) {
        if (mHandler == null) {
            return -1;
        }

        IAlarmTimer atm = (IAlarmTimer)AgentFactory.getAgent(
                AgentFactory.ALARM_TIMER, getSlotId());
        int timerId = (atm != null) ? atm.getTimerId() : -1;

        if (timerId > 0) {
            atm.registerForTimerExpired(timerId, mHandler, event, null);

            if (!atm.startTimer(timerId, duration)) {
                atm.unregisterForTimerExpired(timerId, mHandler);
                return -1;
            }

            ImsLog.d(getSlotId(), logTag + ": startTimer=" + timerId);
            return timerId;
        }

        return -1;
    }

    private void stopTimer(int timerId, String logTag) {
        if (!isTimerStarted(timerId)) {
            return;
        }

        IAlarmTimer atm = (IAlarmTimer)AgentFactory.getAgent(
                AgentFactory.ALARM_TIMER, getSlotId());

        if (atm == null) {
            return;
        }

        ImsLog.d(getSlotId(), logTag + ": stopTimer=" + timerId);

        atm.unregisterForTimerExpired(timerId, mHandler);
        atm.stopTimer(timerId);
    }

    private static boolean isTimerStarted(int timerId) {
        return timerId > 0;
    }
    // }

    /* Class : CallInfo */
    public class CallInfo {
        public int mSessionKey = 0;

        public int mType = RIL_CALL_TYPE_INVALID;
        public int mState;
        public int mDir = RIL_CALL_DIR_NONE;
        public int mApnType;

        public CallInfo(int sessionKey, int param) {
            mSessionKey = sessionKey;

            ImsLog.i(getSlotId(), "sessionKey=" + mSessionKey);
            setInfo(param);
        }

        public void setInfo(int param) {

            // pre-operation
            int sessionType = (param & 0x00f00000) >> 20;
            int volteCallType = (param & 0x000f0000) >> 16;
            int sessionState = (param & 0x0000000f);

            ImsLog.i(getSlotId(), "sessionKey=" + mSessionKey + " sessionType=" + sessionType
                    + " callType=" + volteCallType + " sessionState=" + sessionState);

            setRilCallType(sessionType, volteCallType);
            setRilCallState(sessionState);
            setDirection(sessionState);
            setApnType(volteCallType);
        }

        private void setRilCallType(int sessionType, int volteCallType) {

            if (mType != RIL_CALL_TYPE_INVALID) {
                // keep the original type only.
                return;
            }

            // VOLTE_CALL_TYPE
            if (volteCallType == IUMtcCall.VOLTE_CALL_TYPE_EMERGENCY
                    || volteCallType == IUMtcCall.VOLTE_CALL_TYPE_EMERGENCY_WIFI) {
                mType = RIL_CALL_TYPE_ECALL;
                return;
            }

            // SESIONTYPE
            switch (sessionType) {
                case IUMtcCall.CALLTYPE_VT: // FALL-THROUGH
                case IUMtcCall.CALLTYPE_VIDEO_RTT:
                    mType = RIL_CALL_TYPE_VIDEO;
                    return;

                default:
                    mType = RIL_CALL_TYPE_VOICE;
                    return;
            }
        }

        private void setRilCallState(int sessionState) {

            if (sessionState == IUMtcCall.VOLTE_CALL_STATE_TERMINATING) {
                // ignore terminating
                return;
            }

            switch (sessionState) {
                case IUMtcCall.VOLTE_CALL_STATE_RINGING: // _FALL_THROUGH_
                case IUMtcCall.VOLTE_CALL_STATE_RINGBACK: // _FALL_THROUGH_
                case IUMtcCall.VOLTE_CALL_STATE_ALERTING:
                    mState = RIL_CALL_STATE_START;
                    break;

                case IUMtcCall.VOLTE_CALL_STATE_OFFHOOK: // _FALL_THROUGH_
                    mState = RIL_CALL_STATE_CONNECT;
                    break;

                default:
                    mState = RIL_CALL_STATE_END;
                    break;
            }
        }

        private void setDirection(int sessionState) {
            if (mDir == RIL_CALL_DIR_NONE) {
                if (sessionState == IUMtcCall.VOLTE_CALL_STATE_RINGING
                        || sessionState == IUMtcCall.VOLTE_CALL_STATE_ALERTING) {
                    mDir = RIL_CALL_DIR_MT;
                } else {
                    mDir = RIL_CALL_DIR_MO;
                }
            }
        }

        private void setApnType(int volteCallType) {
            if (volteCallType == IUMtcCall.VOLTE_CALL_TYPE_EMERGENCY) {
                mApnType = CallSysModeTracker.APN_TYPE_EMERGENCY;
            } else if (volteCallType == IUMtcCall.VOLTE_CALL_TYPE_EMERGENCY_WIFI) {
                if (mEpdnUsedForWiFi) {
                    mApnType = CallSysModeTracker.APN_TYPE_EMERGENCY;
                } else {
                    mApnType = CallSysModeTracker.APN_TYPE_IMS;
                }
            } else {
                mApnType = CallSysModeTracker.APN_TYPE_IMS;
            }
        }
    }

    /* Class : CallInfo */
    private class CategorizedCallInfoList {
        private CopyOnWriteArrayList<CallInfo> mCallInfosList;
        public int mType;
        public int mDirection;
        public int mTotalState;
        public int mApnType;
        private static final int RIL_CALL_STATE_INVALID = -1;

        public CategorizedCallInfoList(CallInfo callInfo) {
            mCallInfosList = new CopyOnWriteArrayList<CallInfo>();
            mType = callInfo.mType;
            mDirection = callInfo.mDir;
            mTotalState = RIL_CALL_STATE_INVALID;
            mApnType = callInfo.mApnType;

            ImsLog.i(getSlotId(), "CategorizedCallInfoList[" + mType + "," + mDirection + "]");
        }

        public boolean isCategory(CallInfo callInfo) {
            if (callInfo == null) {
                return false;
            }

            if (callInfo.mType == mType && callInfo.mDir == mDirection) {
                return true;
            }
            return false;
        }

        public boolean add(CallInfo callInfo) {
            mCallInfosList.add(callInfo);
            ImsLog.i(getSlotId(), "add " + callInfo.mSessionKey +
                    " size:" + mCallInfosList.size());
            return updateTotalState(callInfo);
        }

        public void remove(CallInfo callInfo) {
            mCallInfosList.remove(callInfo);
            ImsLog.i(getSlotId(), "remove " + callInfo.mSessionKey +
                    " size:" + mCallInfosList.size());
        }

        public boolean updateState(CallInfo callInfo) {
            return updateTotalState(callInfo);
        }

        public void clear() {
            mCallInfosList.clear();
        }

        public int getSize() {
            return mCallInfosList.size();
        }

        public CallInfo getCallInfo(int sessionKey) {
            Iterator<CallInfo> it = mCallInfosList.iterator();

            while (it.hasNext()) {
                CallInfo cur = it.next();

                if (cur.mSessionKey == sessionKey) {
                    return cur;
                }
            }

            return null;
        }

        private boolean updateTotalState(CallInfo callInfo) {
            if (mTotalState == callInfo.mState) {
                return false;
            }
            ImsLog.i(getSlotId(), "updateTotalState[" + mType + "," + mDirection
                    + "] newState=" + callInfo.mState);

            Iterator<CallInfo> it = mCallInfosList.iterator();
            int newTotalState = RIL_CALL_STATE_INVALID;
            while (it.hasNext()) {
                CallInfo cur = it.next();

                if (cur.mState > newTotalState) {
                    newTotalState = cur.mState;
                }
            }
            if (mTotalState == newTotalState) {
                ImsLog.i(getSlotId(), "updateTotalState same total state");
                return false;
            }

            ImsLog.i(getSlotId(), "updateTotalState total state updated " + newTotalState);

            mTotalState = newTotalState;
            return true;
        }
    }

    private static class CallInfoCache {
        public int mType = RIL_CALL_TYPE_INVALID;
        public int mState = RIL_CALL_STATE_END;
        public int mDir = RIL_CALL_DIR_NONE;
        public int mSysMode = CallSysModeTracker.RIL_SYS_MODE_LTE;

        private CallInfoCache() {
        }

        public static CallInfoCache createFrom(CallInfo callInfo, int sysMode) {
            CallInfoCache cic = new CallInfoCache();

            if (callInfo != null) {
                cic.mType = callInfo.mType;
                cic.mState = callInfo.mState;
                cic.mDir = callInfo.mDir;
                cic.mSysMode = sysMode;
            }

            return cic;
        }
    }

    private class MessageHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            ImsLog.i(getSlotId(), "handleMessage: msg=" + msg.what);

            switch (msg.what) {
                case EVENT_TIMER_ENABLE_SA: {
                    ImsLog.i(getSlotId(), "NrSaMode: enable SA (delay) - E");
                    stopEnableSaTimer();
                    setNrSaMode(NR_SA_ENABLED);

                    if (startSendCallIdleTimer()) {
                        ImsLog.i(getSlotId(), "NrSaMode: sendCallIdleDelayed - S");
                    } else {
                        sendDelayedCallIdle();
                    }
                    break;
                }
                case EVENT_TIMER_SEND_CALL_IDLE: {
                    ImsLog.i(getSlotId(), "NrSaMode: sendCallIdleDelayed - E");
                    stopSendCallIdleTimer();
                    sendDelayedCallIdle();
                    break;
                }
                default:
                    // no-op
                    break;
            }
        }
    }
}
