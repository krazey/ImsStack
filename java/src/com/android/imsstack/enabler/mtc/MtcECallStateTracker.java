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

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.reg.MtcServiceState;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

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
    // This handler will be run on the main thread.
    private ECallStateHandler mHandler;
    private int mOldState = IUMtcService.ES_IDLE;
    private int mState = IUMtcService.ES_IDLE;
    private boolean mECallStarted = false;
    private boolean mEcbmEntered = false;

    private MtcECallStateListener mCallListener = null;
    private MtcServiceStateListener mServiceStateListener = null;
    private int mECallState = ECALLSTATE_IDLE;
    private boolean mEcbmExitedByNewCall = false;
    private boolean mProceedingExitEmergency = false;
    private int mEcbmSupportType = ECBM_SUPPORT_TYPE_NONE;

    public MtcECallStateTracker(IBaseContext context, ICallStateTracker csTracker) {
        mContext = context;
        mCallStateTracker = csTracker;

        mHandler = new ECallStateHandler(mContext.getContext().getMainLooper());

        IServiceStateTracker sst = mContext.getServiceStateTracker();
        mServiceStateListener = new MtcServiceStateListener();
        sst.addListener(mServiceStateListener);

        initEcbmSupportType();
    }

    public void dispose() {
        log("dispose");

        if (mServiceStateListener != null) {
            IServiceStateTracker sst = mContext.getServiceStateTracker();
            sst.removeListener(mServiceStateListener);
            mServiceStateListener = null;
        }

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
        mEcbmExitedByNewCall = bExitedByNewCall;

        sendEmptyMessage(EVENT_EXIT_ECBM);
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

    @VisibleForTesting
    protected void setECallListener() {
        if (mCallListener == null) {
            mCallListener = new MtcECallStateListener();
            mCallStateTracker.addListener(mCallListener);
        }
    }

    @VisibleForTesting
    protected void setECallState(int state) {
        mECallState = state;
    }

    @VisibleForTesting
    protected boolean isECallState(int state) {
        if (mECallState == state) {
            return true;
        }
        return false;
    }

    @VisibleForTesting
    protected void setProceedingExitEmergency(boolean proceeding) {
        mProceedingExitEmergency = proceeding;
    }

    @VisibleForTesting
    protected boolean isProceedingExitEmergency() {
        if (mProceedingExitEmergency) {
            return true;
        }
        return false;
    }

    @VisibleForTesting
    protected void setEcbmEntered(boolean entered) {
        if (mEcbmEntered != entered) {
            log("ECallState :: ecbmEntered - " + mEcbmEntered + " >> " + entered);
            mEcbmEntered = entered;
        }
    }

    private void setEcbmSupportType(int ecbmType) {
        mEcbmSupportType = ecbmType;
    }

    private void initEcbmSupportType() {
        int ecbmType = ECBM_SUPPORT_TYPE_NONE;

        if (isEcbmSupportedForVolte(mContext.getSlotId())) {
            ecbmType |= ECBM_SUPPORT_TYPE_VOLTE;
        }

        if (isEcbmSupportedForVowifi(mContext.getSlotId())) {
            ecbmType |= ECBM_SUPPORT_TYPE_VOWIFI;
        }

        setEcbmSupportType(ecbmType);
    }

    private boolean isEcbmSupportedType(int type) {
        return (mEcbmSupportType & type) == type;
    }

    private boolean checkEcbmExitedDelayAndDisconnectEApn() {
        return false;
    }

    private void exitEcbm() {
        sendStatus(EMERGENCY_CALL_STOP, 0);

        if (isEcbmEntered()) {
            setEcbmEntered(false);

            if (checkEcbmExitedDelayAndDisconnectEApn()) {
                sendEmptyMessageDelayed(EVENT_ECBM_EXITED, 1000);
            } else {
                sendEmptyMessage(EVENT_ECBM_EXITED);
            }
        }
        // Calls onEcbmExited() even the ECBM is not entered for preventing timing issue.
        else {
            sendEmptyMessage(EVENT_ECBM_EXITED);
        }

        setECallStarted(false);
    }

    private void exitEmergencyByCallTerminated() {
        if (isECallState(ECALLSTATE_IDLE)) {
            // Call "exit emergency mode" after call is canceled after started (User cancel case)
            sendExitEmergencyMode();
        }

        setProceedingExitEmergency(false);
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

        setProceedingExitEmergency(false);
    }

    private void notifyEventForEcmState(boolean isEntered) {
        ISystem system = mContext.getSystem();

        int nEcmState = -1;

        if (isEntered) {
            nEcmState = ImsEventDef.IMS_ECM_STATE_ON;
        } else {
            if (mEcbmExitedByNewCall) {
                nEcmState = ImsEventDef.IMS_ECM_STATE_OFF_BY_NEW_ECALL;
                mEcbmExitedByNewCall = false;
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
        setECallListener();

        if (isEcbmSupportedType(ECBM_SUPPORT_TYPE_VOWIFI)
                || isEcbmSupportedType(ECBM_SUPPORT_TYPE_VOLTE)) {
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
                    sendEmptyMessage(EVENT_ECBM_ENTERED);
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
        log("ECallState(forModem) :: status="
                + e911StatusToString(status) + ", reason=" + reason);
    }

    private void setECallStarted(boolean started) {
        if (mECallStarted != started) {
            log("ECallState :: ecallStarted - " + mECallStarted + " >> " + started);
            mECallStarted = started;
        }
    }

    private void sendExitEmergencyMode() {
        log("sendExitEmergencyMode");
        //exitVoLteEmergencyMode();
    }

    private boolean isRetryReason(int reason) {
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

    /**
     * Checks whether emergency callback mode for VoLTE is supported or not.
     *
     * @param slotId The slot-id to be retrieved.
     * @return whether emergency callback mode for VoLTE is supported or not.
     */
    public static boolean isEcbmSupportedForVolte(int slotId) {
        return getBoolean(slotId, CarrierConfig.ImsEmergency.KEY_SUPPORT_ECBM_FOR_VOLTE_BOOL);
    }

    /**
     * Checks whether emergency callback mode for VoWIFI is supported or not.
     *
     * @param slotId The slot-id to be retrieved.
     * @return whether emergency callback mode for VoWIFI is supported or not.
     */
    public static boolean isEcbmSupportedForVowifi(int slotId) {
        return getBoolean(slotId, CarrierConfig.ImsEmergency.KEY_SUPPORT_ECBM_FOR_VOWIFI_BOOL);
    }

    private static boolean getBoolean(int slotId, String key) {
        CarrierConfig cc = getCarrierConfig(slotId);
        return cc != null ? cc.getBoolean(key) : false;
    }

    private static CarrierConfig getCarrierConfig(int slotId) {
        ConfigInterface config = AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
        return config != null ? config.getCarrierConfig() : null;
    }

    private static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }

    private boolean sendEmptyMessage(int what) {
        return mHandler != null ? mHandler.sendEmptyMessage(what) : false;
    }

    private boolean sendEmptyMessageDelayed(int what, long delayMillis) {
        return mHandler != null ? mHandler.sendEmptyMessageDelayed(what, delayMillis) : false;
    }

    @VisibleForTesting
    protected final class ECallStateHandler extends Handler {

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
                    MtcServiceState ss = (MtcServiceState) msg.obj;
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

    @VisibleForTesting
    protected class MtcECallStateListener extends CallStateListener {

        public MtcECallStateListener() {
        }

        @Override
        public void onCallCreated(Call call) {
            if (call.isEmergencyCall()) {
                logi("ECall :: Created");
                setECallState(ECALLSTATE_CREATED);
            }
        }

        @Override
        public void onCallDestroyed(Call call) {
            if (call.isEmergencyCall()) {
                logi("ECall :: Destroyed - ecallState=" + mECallState);

                if (isECallState(ECALLSTATE_CREATED)) {
                    if (!isProceedingExitEmergency()) {
                        sendEmptyMessageDelayed(EVENT_EXIT_EMERGENCY_BY_CALL_DESTROYED, 1000);
                        setProceedingExitEmergency(true);
                    }
                }

                setECallState(ECALLSTATE_IDLE);
            }
        }

        @Override
        public void onCallEstablishing(Call call) {
            if (call.isEmergencyCall()) {
                logi("ECall :: Establishing");
                setECallState(ECALLSTATE_ESTABLISHING);
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
                setECallState(ECALLSTATE_ESTABLISHED);
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
                    if (isECallState(ECALLSTATE_CREATED)) {
                        logi("ECall :: Terminated w/o reason on CREATED");

                        // If emergency call is terminated directly after created,
                        // it means that it terminated by native MTC with retry rat.
                        // (like Tlte_911 fail timer)
                        // So, do not trigger "EXiT_EMEREGNCY_MODE"
                    }
                } else {
                    if (isECallState(ECALLSTATE_ESTABLISHING)) {
                        logi("ECall :: Terminated w/ reason="
                                + callReasonInfo.mCode + " on ESTABLISHING");

                        if (!isRetryReason(callReasonInfo.mCode)) {
                            if (!isProceedingExitEmergency()) {
                                sendEmptyMessageDelayed(
                                        EVENT_EXIT_EMERGENCY_BY_CALL_TERMINATED, 1000);
                                setProceedingExitEmergency(true);
                            }
                        }
                    }

                    //If ECBM is not supported, trigger "EXiT_EMEREGNCY_MODE"
                    //when eCall is terminated after started, too
                    boolean isWifiECall =
                        call.getCallExtraBoolean(Call.EXTRA_WIFI_E_CALL, false);

                    boolean ecbmSupported = false;

                    if (isWifiECall && isEcbmSupportedType(ECBM_SUPPORT_TYPE_VOWIFI)) {
                        ecbmSupported = true;
                    } else if (!isWifiECall && isEcbmSupportedType(ECBM_SUPPORT_TYPE_VOLTE)) {
                        ecbmSupported = true;
                    }

                    if (!ecbmSupported && (isECallState(ECALLSTATE_ESTABLISHED))) {
                        if (!isProceedingExitEmergency()) {
                            sendEmptyMessageDelayed(
                                    EVENT_EXIT_EMERGENCY_BY_CALL_TERMINATED, 1000);
                            setProceedingExitEmergency(true);
                        }
                    }
                }

                setECallState(ECALLSTATE_IDLE);
            }
        }
    }

    protected class MtcServiceStateListener implements IServiceStateTracker.Listener {
        @Override
        public void onEmergencyServiceStateChanged(MtcServiceState serviceState) {
            if (mHandler != null) {
                Message.obtain(mHandler, EVENT_EMERGENCY_SERVICE_STATE_CHANGED, serviceState)
                        .sendToTarget();
            }
        }
    }
}
