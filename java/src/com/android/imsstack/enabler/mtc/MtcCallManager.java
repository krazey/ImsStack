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

import android.content.Context;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.imsservice.mmtel.ImsRegistrationTracker;
import com.android.imsstack.imsservice.mmtel.ImsServiceManager;
import com.android.imsstack.imsservice.mmtel.ImsServiceRecord;
import com.android.imsstack.internal.enabler.ImsStateStore;
import com.android.imsstack.internal.enabler.MtcCallRegistry;
import com.android.imsstack.util.ImsLog;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * Provides APIs for keeping, tracking calls.
 */
public final class MtcCallManager implements ICallStateTracker, IMtcCallManager {
    private static final int MAX_CALL_INDEX = 1000;
    private static int sNextCallIndex = 1;
    private final IBaseContext mContext;
    private CallStateNotifier mCallStateNotifier = new CallStateNotifier();
    private List<CallNode> mCallNodes = new ArrayList<CallNode>();
    private List<CallNode> mPendingCallNodes = new ArrayList<CallNode>();
    private MtcCallTracker mCT = new MtcCallTracker();
    // E-call tracker
    private MtcECallStateTracker mECallStateTracker = null;
    private int mCallStateVoLte = MtcStateUtils.STATE_INACTIVE;
    private int mCallStateVt = MtcStateUtils.STATE_INACTIVE;

    public MtcCallManager(IBaseContext context) {
        mContext = context;
    }

    @Override
    public void dispose() {
        log("dispose");

        clear();

        mCallStateNotifier = null;
        mCT = null;
    }

    @Override
    public void init() {
        log("init");

        if (MtcECallStateTracker.isEcbmSupportedForVolte(mContext.getSlotId())
                || MtcECallStateTracker.isEcbmSupportedForVowifi(mContext.getSlotId())) {
            mECallStateTracker = new MtcECallStateTracker(mContext, this);
        }
    }

    @Override
    public void clear() {
        log("clear");

        if (mECallStateTracker != null) {
            mECallStateTracker.dispose();
            mECallStateTracker = null;
        }

        mPendingCallNodes.clear();
        mCallNodes.clear();
    }

    @Override
    public void addListener(CallStateListener listener) {
        mCallStateNotifier.addListener(listener);
    }

    @Override
    public void removeListener(CallStateListener listener) {
        mCallStateNotifier.removeListener(listener);
    }

    @Override
    public boolean hasCall() {
        return hasCalls() || hasPendingCalls();
    }

    @Override
    public boolean hasEstablishedCall() {
        return hasAtLeastOneEstablishedCall();
    }

    @Override
    public MtcECallStateTracker getECallStateTracker() {
        return mECallStateTracker;
    }

    // For originating call
    @Override
    public void attachCall(Call call) {
        if (call == null) {
            return;
        }

        mCallNodes.add(new CallNode(call));

        mCallStateNotifier.onCallCreated(call);
    }

    /**
     * Adds pending call for incoming call.
     */
    @Override
    public void attachPreIncomingCall(Call call) {
        if (call == null) {
            return;
        }

        CallNode node = new CallNode(call);
        mPendingCallNodes.add(node);
    }

    /**
     * Closes all active and pending calls.
     *
     * @param terminate {@code true} if the calls should be terminated before closing,
     *                  {@code false} otherwise.
     */
    public void closeAllCalls(boolean terminate) {
        List<Call> allCalls = getAllCalls();

        // Closes all the calls
        for (Call call : allCalls) {
            if (call != null) {
                if (terminate) {
                    call.terminate(0, true);
                }

                call.close();
            }
        }
    }

    @Override
    public CallTracker getCallTracker() {
        return mCT;
    }

    @Override
    public ICallStateTracker getCallStateTracker() {
        return this;
    }

    @Override
    public Call getCall(long callId) {
        CallNode node = getCallNode(callId);

        if (node != null) {
            return node.mCall;
        }

        return null;
    }

    /**
     * Gets pending call for incoming call.
     */
    @Override
    public Call getPendingCall(long callId) {
        CallNode node = getPendingCallNode(callId);

        return (node != null) ? node.mCall : null;
    }

    /**
     * Checks if there is at least one established call (in OFFHOOK state).
     *
     * @return {@code true} if there is at least one established call, {@code false} otherwise.
     */
    public boolean hasAtLeastOneEstablishedCall() {
        if (mCallNodes.isEmpty()) {
            return false;
        }

        for (CallNode node : mCallNodes) {
            if ((node.mCall != null)
                    && (node.getCallState() == CallTracker.CALL_STATE_OFFHOOK)) {
                return true;
            }
        }

        return false;
    }

    /**
     * Checks if there are any active calls.
     *
     * @return {@code true} if there are active calls, {@code false} otherwise.
     */
    public boolean hasCalls() {
        return !mCallNodes.isEmpty();
    }

    /**
     * Checks if there are any pending calls.
     *
     * @return {@code true} if there are pending calls, {@code false} otherwise.
     */
    public boolean hasPendingCalls() {
        return !mPendingCallNodes.isEmpty();
    }

    /**
     * Terminates all active and pending calls.
     */
    public void terminateAllCalls() {
        List<Call> allCalls = getAllCalls();

        // Terminates all the calls
        for (Call call : allCalls) {
            if (call != null) {
                call.terminate(0, true);
            }
        }
    }

    @Override
    public int getNextCallIndex() {
        if (sNextCallIndex >= MAX_CALL_INDEX) {
            sNextCallIndex = 1;
        }
        return sNextCallIndex++;
    }

    private void doCallStateChanged() {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                updateCallState();
                updateTelephonyCallState();
            }
        });
    }

    private CallNode getCallNode(long callId) {
        for (CallNode node : mCallNodes) {
            if (node.mCall == null) {
                continue;
            }

            if (node.mCall.getNativeCallId() == callId) {
                return node;
            }
        }

        return null;
    }

    private CallNode getPendingCallNode(long callId) {
        for (CallNode node : mPendingCallNodes) {
            if (node.mCall == null) {
                continue;
            }

            if (node.mCall.getNativeCallId() == callId) {
                return node;
            }
        }

        return null;
    }

    private List<Call> getAllCalls() {
        List<Call> allCalls = new ArrayList<Call>();

        // Gets the pending calls
        for (CallNode node : mPendingCallNodes) {
            if (node.mCall != null) {
                allCalls.add(node.mCall);
            }
        }

        // Gets the active calls
        for (CallNode node : mCallNodes) {
            if (node.mCall != null) {
                allCalls.add(node.mCall);
            }
        }

        return allCalls;
    }

    private int getActiveCallCount() {
        if (mCallNodes.isEmpty()) {
            return 0;
        }

        int activeCallCount = 0;

        for (CallNode node : mCallNodes) {
            if ((node.mCall != null)
                    && (node.getCallState() == CallTracker.CALL_STATE_OFFHOOK)) {
                activeCallCount++;
            }
        }

        return activeCallCount;
    }

    private void onCallCreate(Call call) {
        CallNode node = new CallNode(call);

        if (call.isConference() && call.isInCall()) {
            node.setCallState(CallTracker.CALL_STATE_OFFHOOK);
        }

        mCallNodes.add(node);
        doCallStateChanged();
        mCallStateNotifier.onCallCreated(call);
    }

    private void onCallEstablishing(Call call) {
        CallNode node = getCallNode(call.getNativeCallId());

        if (node != null) {
            node.setCallState(CallTracker.CALL_STATE_RINGBACK);
            doCallStateChanged();
            mCallStateNotifier.onCallEstablishing(call);
        }
    }

    private void onCallRinging(Call call) {
        mCallStateNotifier.onCallRinging(call);
    }

    private void onCallAccept(Call call) {
        CallNode node = getCallNode(call.getNativeCallId());

        if (node != null) {
            node.setCallState(CallTracker.CALL_STATE_OFFHOOK);
            doCallStateChanged();
            mCallStateNotifier.onCallAccepted(call);
        }
    }

    private void onCallEstablished(Call call) {
        CallNode node = getCallNode(call.getNativeCallId());

        if (node != null) {
            node.setCallState(CallTracker.CALL_STATE_OFFHOOK);
        }

        doCallStateChanged();
        mCallStateNotifier.onCallEstablished(call);
    }

    private void onCallUpdated(Call call) {
        doCallStateChanged();
        mCallStateNotifier.onCallUpdated(call);
    }

    private void onCallTerminating(Call call) {
        mCallStateNotifier.onCallTerminating(call);
    }

    private void onCallTerminated(Call call) {
        CallNode node = getPendingCallNode(call.getNativeCallId());

        if (node != null) {
            log("onCallTerminated :: A pending call is terminated; call=" +
                    Long.toHexString(call.getNativeCallId()));
            node.setCallState(CallTracker.CALL_STATE_IDLE);
            doCallStateChanged();
            mCallStateNotifier.onCallTerminated(call);
            return;
        }

        node = getCallNode(call.getNativeCallId());

        if (node != null) {
            log("onCallTerminated :: An active call is terminated; call=" +
                    Long.toHexString(call.getNativeCallId()));
            node.setCallState(CallTracker.CALL_STATE_IDLE);
            doCallStateChanged();
            mCallStateNotifier.onCallTerminated(call);
        }
    }

    private void onCallDestroy(Call call) {
        CallNode node = getPendingCallNode(call.getNativeCallId());

        if (node != null) {
            mPendingCallNodes.remove(node);
            if (!call.isOnPreIncoming()) {
                doCallStateChanged();
                mCallStateNotifier.onCallDestroyed(call);
            }
            logi("onCallDestroyed :: pendingCalls=" + mPendingCallNodes.size());
            return;
        }

        node = getCallNode(call.getNativeCallId());

        if (node != null) {
            mCallNodes.remove(node);
            doCallStateChanged();
            mCallStateNotifier.onCallDestroyed(call);
            logi("onCallDestroyed :: activeCalls=" + mCallNodes.size());
            return;
        }

        log("onCallDestroyed :: call not found - call=" +
                Long.toHexString(call.getNativeCallId()) +
                ", activeCalls=" + mCallNodes.size() +
                ", pendingCalls=" + mPendingCallNodes.size());
    }

    private void onCallIncomingReceived(Call call) {
        CallNode node = getPendingCallNode(call.getNativeCallId());

        if (node != null) {
            mPendingCallNodes.remove(node);
            mCallNodes.add(node);
            node.setCallState(CallTracker.CALL_STATE_RINGING);
            doCallStateChanged();
            mCallStateNotifier.onCallCreated(call);
        }
    }

    private void postAndRunTask(Runnable task) {
        mContext.getExecutor().execute(task);
    }

    /**
     * @return the IMS call state - IMS (VoLTE or VT) call state (IDLE / OFFHOOK)
     */
    private int updateCallState() {
        int voipCallState = MtcStateUtils.STATE_INACTIVE;
        int vtCallState = MtcStateUtils.STATE_INACTIVE;

        // Checks if the pending call is present
        for (CallNode node : mPendingCallNodes) {
            if (node.mCall == null) {
                continue;
            }

            int callType = node.mCall.getCallType();

            if (!MtcCallUtils.hasVideo(callType)) {
                voipCallState = MtcStateUtils.STATE_ACTIVE;
            } else {
                vtCallState = MtcStateUtils.STATE_ACTIVE;
            }
        }

        for (CallNode node : mCallNodes) {
            if (node.mCall == null) {
                continue;
            }

            int callType = node.mCall.getCallType();

            if (!MtcCallUtils.hasVideo(callType)) {
                voipCallState = MtcStateUtils.STATE_ACTIVE;
            } else {
                vtCallState = MtcStateUtils.STATE_ACTIVE;
            }
        }

        int callState = CallTracker.CALL_STATE_IDLE;

        if ((voipCallState == MtcStateUtils.STATE_ACTIVE)
                || (vtCallState == MtcStateUtils.STATE_ACTIVE)) {
            callState = CallTracker.CALL_STATE_OFFHOOK;
        }

        updateCallStateInternal(voipCallState, vtCallState);

        return callState;
    }

    private void updateCallStateInternal(int voiceCallState, int videoCallState) {
        if (updateCallStateForVoLte(voiceCallState) || updateCallStateForVt(videoCallState)) {
            Context context = mContext.getContext();
            int slotId = mContext.getSlotId();

            if ((mCallStateVoLte == MtcStateUtils.STATE_ACTIVE)
                    || (mCallStateVt == MtcStateUtils.STATE_ACTIVE)) {
                MtcStateUtils.updateCallState(context, slotId, MtcStateUtils.STATE_ACTIVE);
            } else {
                MtcStateUtils.updateCallState(context, slotId, MtcStateUtils.STATE_INACTIVE);
            }
        }
    }

    private boolean updateCallStateForVoLte(int state) {
        if (mCallStateVoLte == state) {
            // Call state is not changed; ignore it.
            return false;
        }

        if ((state != MtcStateUtils.STATE_ACTIVE) && (state != MtcStateUtils.STATE_INACTIVE)) {
            // Invalid state value; ignore it
            return false;
        }

        mCallStateVoLte = state;

        return true;
    }

    private boolean updateCallStateForVt(int state) {
        if (mCallStateVt == state) {
            // Call state is not changed; ignore it.
            return false;
        }

        if ((state != MtcStateUtils.STATE_ACTIVE) && (state != MtcStateUtils.STATE_INACTIVE)) {
            // Invalid state value; ignore it
            return false;
        }

        mCallStateVt = state;

        return true;
    }

    private void updateTelephonyCallState() {
        int callState = CallTracker.CALL_STATE_IDLE;
        String incomingNumber = "";

        if (!mPendingCallNodes.isEmpty()) {
            CallNode node = mPendingCallNodes.get(mPendingCallNodes.size() - 1);

            if (node != null) {
                callState = CallTracker.CALL_STATE_RINGING;
                incomingNumber = (node.mCall != null) ? node.mCall.getCallerId() : "";
            }
        }

        if ((callState == CallTracker.CALL_STATE_IDLE) && !mCallNodes.isEmpty()) {
            CallNode node = mCallNodes.get(mCallNodes.size() - 1);

            if (node != null) {
                callState = node.getCallState();

                if (callState == CallTracker.CALL_STATE_RINGING) {
                    incomingNumber = (node.mCall != null) ? node.mCall.getCallerId() : "";
                }
            }
        }

        if (callState == CallTracker.CALL_STATE_RINGBACK) {
            // Adjust the call state for Android Framework
            callState = CallTracker.CALL_STATE_OFFHOOK;
        }
    }

    private boolean isImsRegisteredOnWifi() {
        boolean isNetworkTypeWifi = false;
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(mContext.getPhoneId());
        ImsRegistrationTracker regTracker = (serviceRecord != null)
                ? serviceRecord.getRegistrationTracker() : null;

        if ((regTracker != null) && (regTracker.isCallRegistered())) {
            IAosRegistrationListener.NetworkType regNetworkType =
                    regTracker.getRegisteredNetworkType();

            if (regNetworkType == IAosRegistrationListener.NetworkType.IWLAN) {
                isNetworkTypeWifi = true;
            }
        }

        return isNetworkTypeWifi;
    }

    private void updateConnectedCallOnWifi(final Call call, int state, String dbgString) {

        if (call != null) {
            if (call.getCallExtraBoolean(Call.EXTRA_E_CALL, false)
                    && !call.getCallExtraBoolean(Call.EXTRA_WIFI_E_CALL, false)) {
                return;
            }
        }

        boolean callStateUpdateRequired = false;

        if (state == 0) {
            if (!hasCalls()) {
                callStateUpdateRequired = true;
            } else if (!hasAtLeastOneEstablishedCall()) {
                callStateUpdateRequired = true;
            }
        } else {
            if (isImsRegisteredOnWifi()) {
                callStateUpdateRequired = true;
            }
        }

        if (callStateUpdateRequired) {
            logi("updateConnectedCallOnWifi :: state=" + state + ", " + dbgString);

            ImsStateStore.getCallState(mContext.getPhoneId()).setConnectedCallOnWifi(state);
        }
    }

    private static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }

    private static class CallNode {
        private Call mCall;
        private int mCallState = CallTracker.CALL_STATE_IDLE;

        public CallNode(Call call) {
            mCall = call;
        }

        public int getCallState() {
            return mCallState;
        }

        public void setCallState(int state) {
            mCallState = state;
        }
    }

    private class MtcCallTracker implements CallTracker {
        @Override
        public int getActiveCalls() {
            return getActiveCallCount();
        }

        @Override
        public void updateCallState(Object call, int event, Object extraInfo) {

            // If the call is null, do not handle any operations...
            if (call == null) {
                return;
            }

            if (!(call instanceof Call)) {
                return;
            }

            switch (event) {
                case CALL_EVENT_CREATE:
                    onCallCreate((Call) call);
                    break;
                case CALL_EVENT_ESTABLISHING:
                    onCallEstablishing((Call) call);
                    break;
                case CALL_EVENT_RINGING:
                    onCallRinging((Call) call);
                    break;
                case CALL_EVENT_ACCEPT:
                    onCallAccept((Call) call);
                    break;
                case CALL_EVENT_ESTABLISHED:
                    onCallEstablished((Call) call);
                    break;
                case CALL_EVENT_UPDATED:
                    onCallUpdated((Call) call);
                    break;
                case CALL_EVENT_TERMINATING:
                    onCallTerminating((Call) call);
                    break;
                case CALL_EVENT_TERMINATED:
                    onCallTerminated((Call) call);
                    break;
                case CALL_EVENT_DESTROY:
                    onCallDestroy((Call) call);
                    break;
                case CALL_EVENT_INCOMING_RECEIVED:
                    onCallIncomingReceived((Call) call);
                    break;
                default:
                    break;
            }
        }
    }

    private class CallStateNotifier {
        private final Set<CallStateListener> mListeners
                = new CopyOnWriteArraySet<CallStateListener>();

        public CallStateNotifier() {
        }

        public void addListener(CallStateListener listener) {
            mListeners.add(listener);
        }

        public void removeListener(CallStateListener listener) {
            mListeners.remove(listener);
        }

        public void onCallCreated(final Call call) {
            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    MtcCallRegistry mtcCallRegistry =
                            MtcCallRegistry.getInstance(mContext.getSlotId());
                    mtcCallRegistry.notifyCallCreated(call);

                    for (CallStateListener l : mListeners) {
                        l.onCallCreated(call);
                    }
                }
            });
        }

        public void onCallDestroyed(final Call call) {
            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    updateConnectedCallOnWifi(call, 0, "onCallDestroyed");
                    MtcCallRegistry mtcCallRegistry =
                            MtcCallRegistry.getInstance(mContext.getSlotId());
                    mtcCallRegistry.notifyCallDestroyed(call);

                    for (CallStateListener l : mListeners) {
                        l.onCallDestroyed(call);
                    }
                }
            });
        }

        public void onCallEstablishing(final Call call) {
            if (mListeners.isEmpty()) {
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    for (CallStateListener l : mListeners) {
                        l.onCallEstablishing(call);
                    }
                }
            });
        }

        public void onCallRinging(final Call call) {
            if (mListeners.isEmpty()) {
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    for (CallStateListener l : mListeners) {
                        l.onCallRinging(call);
                    }
                }
            });
        }

        public void onCallAccepted(final Call call) {
            if (mListeners.isEmpty()) {
                updateConnectedCallOnWifi(call, 1, "onCallAccepted");
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    updateConnectedCallOnWifi(call, 1, "onCallAccepted");

                    for (CallStateListener l : mListeners) {
                        l.onCallAccepted(call);
                    }
                }
            });
        }

        public void onCallEstablished(final Call call) {
            if (mListeners.isEmpty()) {
                updateConnectedCallOnWifi(call, 1, "onCallEstablished");
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    updateConnectedCallOnWifi(call, 1, "onCallEstablished");

                    for (CallStateListener l : mListeners) {
                        l.onCallEstablished(call);
                    }
                }
            });
        }

        public void onCallUpdated(final Call call) {
            if (mListeners.isEmpty()) {
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    for (CallStateListener l : mListeners) {
                        l.onCallUpdated(call);
                    }
                }
            });
        }

        public void onCallTerminating(final Call call) {
            if (mListeners.isEmpty()) {
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    for (CallStateListener l : mListeners) {
                        l.onCallTerminating(call);
                    }
                }
            });
        }

        public void onCallTerminated(final Call call) {
            if (mListeners.isEmpty()) {
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    for (CallStateListener l : mListeners) {
                        l.onCallTerminated(call);
                    }
                }
            });
        }
    }
}
