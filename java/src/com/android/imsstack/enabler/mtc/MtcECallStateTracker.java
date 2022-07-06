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
package com.android.imsstack.enabler.mtc;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.VoLteFactory;
import com.android.imsstack.core.agents.agentif.ILocationAgent;
import com.android.imsstack.core.agents.agentif.ILocationAgentManager;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.reg.ImsServiceState;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.MSimUtils;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

public class MtcECallStateTracker implements IECallStateTracker {

    private static final int EVENT_EMERGENCY_SERVICE_STATE_CHANGED = 101;
    private static final int EVENT_ECBM_ENTERED = 102;
    private static final int EVENT_ECBM_EXITED = 103;
    private static final int EVENT_EXIT_ECBM = 201;
    private static final int EVENT_EXIT_EMERGENCY_BY_CALL_DESTROYED = 301;
    private static final int EVENT_EXIT_EMERGENCY_BY_CALL_TERMINATED = 302;

    private static final int ECALLSTATE_IDLE = 0;
    private static final int ECALLSTATE_CREATED = 1;
    private static final int ECALLSTATE_ESTABLISHING = 2;
    private static final int ECALLSTATE_ESTABLISHED = 3;

    private static final int ECBM_SUPPORT_TYPE_NONE = 0x00000000;
    private static final int ECBM_SUPPORT_TYPE_VOLTE = 0x00000001;
    private static final int ECBM_SUPPORT_TYPE_VOWIFI = 0x00000002;

    private static final int EMERGENCY_CALL_START = 0;
    private static final int EMERGENCY_CALL_STOP_WITH_ECB = 1;
    private static final int EMERGENCY_CALL_STOP = 2;
    private static final int EMERGENCY_CALL_ECB_START = 3;

    private final IBaseContext mContext;
    private final ICallStateTracker mCallStateTracker;
    private final Set<EcbmListener> mEcbmListeners = new CopyOnWriteArraySet<EcbmListener>();
    private ECallStateReceiver mReceiver;
    // This handler will be run on the main thread.
    private ECallStateHandler mHandler;
    private int mOldState = IUMtcService.ES_IDLE;
    private int mState = IUMtcService.ES_IDLE;
    private boolean mECallStarted = false;
    private boolean mEcbmEntered = false;

    private MtcECallStateListener mCallListener = null;
    private int mECallState = ECALLSTATE_IDLE;
    private boolean mECBMExitedByNewCall = false;
    private boolean mProceedingExitEmergency = false;

    private int mECBMSupportType = ECBM_SUPPORT_TYPE_NONE;
    private boolean mEmergencyOriginatingMonitor = false;

    public MtcECallStateTracker(IBaseContext context, ICallStateTracker csTracker) {
        mContext = context;
        mCallStateTracker = csTracker;

        mHandler = new ECallStateHandler(mContext.getContext().getMainLooper());
        mReceiver = new ECallStateReceiver();
        mReceiver.register();

        IServiceStateTracker sst = mContext.getServiceStateTracker();
        sst.registerForEmergencyServiceStateChanged(
                mHandler, EVENT_EMERGENCY_SERVICE_STATE_CHANGED, null);

        setECBMSupportType();

        if (ImsGlobal.isOperator(context.getSlotId(), "VZW")) {
            mEmergencyOriginatingMonitor = true;
        }
    }

    public void dispose() {
        log("dispose");

        IServiceStateTracker sst = mContext.getServiceStateTracker();
        sst.unregisterForEmergencyServiceStateChanged(mHandler);

        mReceiver.unregister();
        mReceiver = null;

        if (mCallListener != null) {
            mCallStateTracker.removeListener(mCallListener);
            mCallListener = null;
        }

        mEcbmListeners.clear();

        mHandler = null;
    }

    @Override
    public void addEcbmListener(EcbmListener listener) {
        mEcbmListeners.add(listener);
    }

    @Override
    public void removeEcbmListener(EcbmListener listener) {
        mEcbmListeners.remove(listener);
    }

    @Override
    public void exitEmergencyCallbackMode(boolean bExitedByNewCall) {
        log("exitEmergencyCallbackMode - ExitedByNewCall=" + bExitedByNewCall);
        mECBMExitedByNewCall = bExitedByNewCall;

        mHandler.sendEmptyMessage(EVENT_EXIT_ECBM);
    }

    @Override
    public boolean isEcbmEntered() {
        return mEcbmEntered;
    }

    /**
     * Returns the emergency service state.
     * IUMtcService.ES_XXX
     */
    public int getState() {
        return mState;
    }

    public int getOldState() {
        return mOldState;
    }

    public void setState(int state) {
        mOldState = mState;
        mState = state;
    }

    public boolean isECallStarted() {
        return mECallStarted;
    }

    public static boolean isFeatureSupported(int slotId) {
        //VZW : VoLTE/VoWifi both ECBM supported
        //USC : VoLTE ECBM supported
        //SPR : VoLTE ECBM supported
        //ACG : VoLTE ECBM supported
        if (ImsGlobal.isOperator(slotId, "VZW")
            || ImsGlobal.isOperator(slotId, "USC")
            || ImsGlobal.isOperator(slotId, "ACG")
            || ImsGlobal.isOperator(slotId, "SPR")) {
            return true;
        }

        return false;
    }

    private void setECBMSupportType() {
        int slotId = mContext.getSlotId();
        if (ImsGlobal.isOperator(slotId, "VZW")) {
            mECBMSupportType = (ECBM_SUPPORT_TYPE_VOLTE | ECBM_SUPPORT_TYPE_VOWIFI);
        } else if (ImsGlobal.isOperator(slotId, "USC")) {
            mECBMSupportType = ECBM_SUPPORT_TYPE_VOLTE;
        } else if (ImsGlobal.isOperator(slotId, "SPR")) {
            mECBMSupportType = ECBM_SUPPORT_TYPE_VOLTE;
        } else if (ImsGlobal.isOperator(slotId, "ACG")) {
            mECBMSupportType = ECBM_SUPPORT_TYPE_VOLTE;
        }
    }

    private boolean isECBMSupportedForVoWifi() {
        return isECBMSupportedType(ECBM_SUPPORT_TYPE_VOWIFI);
    }

    private boolean isECBMSupportedForVoLTE() {
        return isECBMSupportedType(ECBM_SUPPORT_TYPE_VOLTE);
    }

    private boolean isECBMSupportedType(int type) {
        return (mECBMSupportType & type) == type;
    }

    private boolean checkEcbmExitedDelayAndDisconnectEApn() {
        if (ImsProperties.isChipVendorMtk()) {
            if (mECBMExitedByNewCall) {
                return false;
            }

            IDcApn dcapn = mContext.getDcApn();
            if ((dcapn != null) && (dcapn.isConnected(EApnType.EMERGENCY.getType()))) {
                log("Ecbm exit is delayed and e-pdn is released");
                dcapn.disconnect(EApnType.EMERGENCY.getType(), 0, 0);
                return true;
            }
        }

        return false;
    }

    private void exitEcbm() {
        sendStatus(EMERGENCY_CALL_STOP, 0);

        if (isEcbmEntered()) {
            setEcbmEntered(false);

            if (checkEcbmExitedDelayAndDisconnectEApn()) {
                mHandler.sendEmptyMessageDelayed(EVENT_ECBM_EXITED, 1000);
            } else {
                mHandler.sendEmptyMessage(EVENT_ECBM_EXITED);
            }
        }
        // Calls onEcbmExited() even the ECBM is not entered for preventing timing issue.
        else {
            mHandler.sendEmptyMessage(EVENT_ECBM_EXITED);
        }

        setECallStarted(false);
    }

    private void exitEmergencyByCallTerminated() {
        if ( mECallState == ECALLSTATE_IDLE ) {
            // Call "exit emergency mode" after call is canceled after started (User cancel case)
            sendExitEmergencyMode();
        }

        mProceedingExitEmergency = false;
    }

    private void exitEmergencyByCallDestroyed() {
        if (getState() == IUMtcService.ES_IDLE) {
            if (getOldState() == IUMtcService.ES_UNAVAILABLE) {
                // If previous MtcService status is UNAVAILABLE,
                // it means that E911 call is aborted by E-PDN problem.
                // So this case should be retried to next rat. Do NOT call "EXIT_EMEREGENCY_MODE"
            } else {
                // Call "exit emergency mode" after call is aborted after created(User cancel case)
                sendExitEmergencyMode();
            }
        }

        mProceedingExitEmergency = false;
    }


    private void notifyEventForEcmState(boolean isEntered) {
        ISystem system = mContext.getSystem();

        int nEcmState = -1;

        if (isEntered) {
            nEcmState = ImsEventDef.IMS_ECM_STATE_ON;
        } else {
            if (mECBMExitedByNewCall) {
                nEcmState = ImsEventDef.IMS_ECM_STATE_OFF_BY_NEW_ECALL;
                mECBMExitedByNewCall = false;
            } else {
                nEcmState = ImsEventDef.IMS_ECM_STATE_OFF;
            }
        }

        if (system != null) {
            system.notifyEvent(ImsEventDef.IMS_EVENT_ECM_STATE, nEcmState, 0);
        }
    }

    private void onEcbmEntered() {
        for (EcbmListener l : mEcbmListeners) {
            l.onEcbmEntered();
        }

        notifyEventForEcmState(true);
    }

    private void onEcbmExited() {
        for (EcbmListener l : mEcbmListeners) {
            l.onEcbmExited();
        }

        notifyEventForEcmState(false);
    }

    private void onEmergencyServiceStateChanged(int state, int reason) {
        setState(state);
        int oldState = getOldState();

        logi("onEmergencyServiceStateChanged :: oldState="
                + oldState + ", state=" + state + ", reason=" + reason);

        // Initialize call state listener only one time
        if (mCallListener == null) {
            mCallListener = new MtcECallStateListener();
            mCallStateTracker.addListener(mCallListener);
        }

        if (isECBMSupportedForVoWifi() || isECBMSupportedForVoLTE()) {
            // Sends emergency status to modem
            if ((state == IUMtcService.ES_OPENED)
                    && ((oldState == IUMtcService.ES_IDLE)
                        || (oldState == IUMtcService.ES_OPENING))) {
                if (!isECallStarted()) {
                    sendStatus(EMERGENCY_CALL_START, 0);
                    setECallStarted(true);
                }
            } else if ((oldState != IUMtcService.ES_IDLE)
                    && (state == IUMtcService.ES_IDLE)) {
                if (reason == IUMtcService.ES_IDLE_REASON_WITH_ECM) {
                    setEcbmEntered(true);
                    sendStatus(EMERGENCY_CALL_STOP_WITH_ECB, 0);
                    mHandler.sendEmptyMessage(EVENT_ECBM_ENTERED);
                } else if (isECallStarted()) {
                    exitEcbm();
                }
            }
        }
    }

    /**
     * Sends an emergency call status to modem.
     */
    private void sendStatus(int status, int reason) {
        int slotId = mContext.getSlotId();
        if (ImsGlobal.isOperator(slotId, "SPR")
                && (status != EMERGENCY_CALL_STOP_WITH_ECB)) {
            return;
        }

        log("ECallState(forModem) :: status="
                + e911StatusToString(status) + ", reason=" + reason);
    }

    private void setECallStarted(boolean started) {
        if (mECallStarted != started) {
            log("ECallState :: ecallStarted - " + mECallStarted + " >> " + started);
            mECallStarted = started;
        }
    }

    private void setEcbmEntered(boolean entered) {
        if (mEcbmEntered != entered) {
            log("ECallState :: ecbmEntered - " + mEcbmEntered + " >> " + entered);
            mEcbmEntered = entered;
        }
    }

    private void sendExitEmergencyMode() {
        log("sendExitEmergencyMode");
        //exitVoLteEmergencyMode();
    }

    private boolean isRetryReason(int reason) {
        // TODO : need to modify this after emergency domain selection policy is decided.
        /*if ((reason >= IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY)
                && (reason <= IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY_E_RAT)) {*/
        if ((reason == CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED)
                || reason == CallReasonInfo.CODE_LOCAL_CALL_VOLTE_RETRY_REQUIRED) {
            return true;
        }

        return false;
    }

    private static String e911StatusToString(int state) {
        switch (state) {
        case EMERGENCY_CALL_START:
            return "START";
        case EMERGENCY_CALL_STOP_WITH_ECB:
            return "STOP_WITH_ECB";
        case EMERGENCY_CALL_STOP:
            return "STOP";
        case EMERGENCY_CALL_ECB_START:
            return "ECB_START";
        default:
            return "__UNKNOWN__";
        }
    }

    private static void log(String s) {
        ImsLog.d("[GII-MTC] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-MTC] " + s);
    }

    private final class ECallStateHandler extends Handler {

        public ECallStateHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                return;
            }

            logi("ECallState :: msg=" + msg.what);

            switch (msg.what) {
            case EVENT_EMERGENCY_SERVICE_STATE_CHANGED: {
                AsyncResult ar = (AsyncResult)msg.obj;
                ImsServiceState ss = (ar != null) ? (ImsServiceState)ar.result : null;

                if (ss != null) {
                    onEmergencyServiceStateChanged(ss.mExtraState, ss.mReason);
                }
                break;
            }

            case EVENT_ECBM_ENTERED: {
                onEcbmEntered();
                break;
            }

            case EVENT_ECBM_EXITED: {
                onEcbmExited();
                break;
            }

            case EVENT_EXIT_ECBM: {
                exitEcbm();
                break;
            }

            case EVENT_EXIT_EMERGENCY_BY_CALL_DESTROYED: {
                exitEmergencyByCallDestroyed();
                break;
            }

            case EVENT_EXIT_EMERGENCY_BY_CALL_TERMINATED: {
                exitEmergencyByCallTerminated();
                break;
            }

            default:
                break;
            }
        }
    }

    private final class ECallStateReceiver extends BroadcastReceiver {
        public static final String ACTION_ECBM_EXIT_FOR_VOLTE
                = "ACTION_ECBM_EXIT_FOR_VOLTE";

        public ECallStateReceiver() {
        }

        public void register() {
            mContext.getContext().registerReceiver(this,
                    new IntentFilter(ACTION_ECBM_EXIT_FOR_VOLTE),
                    Context.RECEIVER_EXPORTED);
            mContext.getContext().registerReceiver(this,
                    new IntentFilter(Intent.ACTION_NEW_OUTGOING_CALL),
                    Context.RECEIVER_EXPORTED);
        }

        public void unregister() {
            mContext.getContext().unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null) {
                return;
            }

            if (ACTION_ECBM_EXIT_FOR_VOLTE.equals(intent.getAction())) {
                exitEmergencyCallbackMode(false);
            } else if (Intent.ACTION_NEW_OUTGOING_CALL.equals(intent.getAction())) {
                // Monitoring the originating emergency call for updating geolocation
                if (mEmergencyOriginatingMonitor) {
                    String number = intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER);
                    logi("ECall :: ACTION_NEW_OUTGOING_CALL, num=" + number);

                    TelephonyManager tm = null;
                    int subId = MSimUtils.getSubId(mContext.getSlotId());
                    if (MSimUtils.isValidSubId(subId)) {
                        tm = AppContext.getTelephonyManager(subId);
                    }

                    if (tm != null && tm.isEmergencyNumber(number)) {
                        // Update geolocation
                        logi("ECall :: Update Geolocation");

                        ILocationAgentManager lam =
                            (ILocationAgentManager)VoLteFactory.getInstance().getAgent(VoLteFactory.AGENT_LOCATION_AGENT_MANAGER);
                        ILocationAgent locAgent = (lam != null) ? lam.getAgent(mContext.getSlotId()) : null;

                        if (locAgent != null) {
                            locAgent.makeInstantLocationUpdates4Sys();
                        }
                    }
                }
            }
        }
    }

    private class MtcECallStateListener extends CallStateListener {

        public MtcECallStateListener() {
        }

        @Override
        public void onCallCreated(Call call) {
            if (call.isEmergencyCall()) {
                logi("ECall :: Created");
                mECallState = ECALLSTATE_CREATED;
            }
        }

        @Override
        public void onCallDestroyed(Call call) {
            if (call.isEmergencyCall()) {
                logi("ECall :: Destroyed - ecallState=" + mECallState);

                if (mECallState == ECALLSTATE_CREATED) {
                    if (!mProceedingExitEmergency) {
                        mHandler.sendEmptyMessageDelayed(
                                EVENT_EXIT_EMERGENCY_BY_CALL_DESTROYED, 1000);
                        mProceedingExitEmergency = true;
                    }
                }

                mECallState = ECALLSTATE_IDLE;
            }
        }

        @Override
        public void onCallEstablishing(Call call) {
            if (call.isEmergencyCall()) {
                logi("ECall :: Establishing");
                mECallState = ECALLSTATE_ESTABLISHING;
            }
        }

        @Override
        public void onCallRinging(Call call) {
            // no-op
        }

        @Override
        public void onCallAccepted(Call call) {
            // no-op
        }

        @Override
        public void onCallEstablished(Call call) {
            if (call.isEmergencyCall()) {
                logi("ECall :: Established");
                mECallState = ECALLSTATE_ESTABLISHED;
            }
        }

        @Override
        public void onCallUpdated(Call call) {
            // no-op
        }

        @Override
        public void onCallTerminating(Call call) {
            // no-op
        }

        @Override
        public void onCallTerminated(Call call) {
            if (call.isEmergencyCall()) {
                logi("ECall :: Terminated");
                MtcCall mtcCall = (MtcCall)call;

                CallReasonInfo callReasonInfo = mtcCall.getTerminationReason();

                if (callReasonInfo == null) {
                    if (mECallState == ECALLSTATE_CREATED) {
                        logi("ECall :: Terminated w/o reason on CREATED");

                        // If emergency call is terminated directly after created,
                        // it means that it terminated by native MTC with retry rat.
                        // (like Tlte_911 fail timer)
                        // So, do not trigger "EXiT_EMEREGNCY_MODE"
                    }
                } else {
                    if (mECallState == ECALLSTATE_ESTABLISHING) {
                        logi("ECall :: Terminated w/ reason="
                                + callReasonInfo.mCode + " on ESTABLISHING");

                        if (!isRetryReason(callReasonInfo.mCode)) {
                            if (!mProceedingExitEmergency) {
                                mHandler.sendEmptyMessageDelayed(
                                        EVENT_EXIT_EMERGENCY_BY_CALL_TERMINATED, 1000);
                                mProceedingExitEmergency = true;
                            }
                        }
                    }

                    //If ECBM is not supported, trigger "EXiT_EMEREGNCY_MODE"
                    //when eCall is terminated after started, too
                    boolean isWifiECall =
                        call.getCallExtraBoolean(Call.EXTRA_WIFI_E_CALL, false);

                    boolean ecbmSupported = false;

                    if (isWifiECall && isECBMSupportedForVoWifi()) {
                        ecbmSupported = true;
                    } else if (!isWifiECall && isECBMSupportedForVoLTE()) {
                        ecbmSupported = true;
                    }

                    if (!ecbmSupported && (mECallState == ECALLSTATE_ESTABLISHED)) {
                        if (!mProceedingExitEmergency) {
                            mHandler.sendEmptyMessageDelayed(
                                    EVENT_EXIT_EMERGENCY_BY_CALL_TERMINATED, 1000);
                            mProceedingExitEmergency = true;
                        }
                    }
                }

                mECallState = ECALLSTATE_IDLE;
            }
        }
    }
}
