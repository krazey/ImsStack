/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20131015    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.enabler.mtc;

import android.content.Context;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.IWifiState;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.telephony.TelephonyCallStateRegistry;
import com.android.imsstack.provider.ImsStateController;
import com.android.imsstack.util.ImsLog;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

public final class MtcCallManager implements ICallStateTracker {
    private final IBaseContext mContext;
    private CallStateNotifier mCallStateNotifier = new CallStateNotifier();
    private List<CallNode> mCallNodes = new ArrayList<CallNode>();
    private List<CallNode> mPendingCallNodes = new ArrayList<CallNode>();
    private MtcCallTracker mCT = new MtcCallTracker();
    // E-call tracker
    private MtcECallStateTracker mECallStateTracker = null;
    private TelephonyCallStateRegistry mTelephonyCsRegistry = null;
    private int mCallStateVoLte = MtcStateUtils.STATE_INACTIVE;
    private int mCallStateVt = MtcStateUtils.STATE_INACTIVE;

    public MtcCallManager(IBaseContext context) {
        mContext = context;
    }

    public void dispose() {
        log("dispose");

        clear();

        mCallStateNotifier = null;
        mCT = null;
    }

    public void init() {
        log("init");

        if (MtcECallStateTracker.isFeatureSupported(mContext.getSlotId())) {
            mECallStateTracker = new MtcECallStateTracker(mContext, this);
        }

        // Initialize the call state for telephony or modem
        mTelephonyCsRegistry = new TelephonyCallStateRegistry(mContext, this);
    }

    public void clear() {
        log("clear");

        if (mECallStateTracker != null) {
            mECallStateTracker.dispose();
            mECallStateTracker = null;
        }

        if (mTelephonyCsRegistry != null) {
            mTelephonyCsRegistry.dispose();
            mTelephonyCsRegistry = null;
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

    public MtcECallStateTracker getECallStateTracker() {
        return mECallStateTracker;
    }

    // For originating call
    public synchronized void attachCall(Call call) {
        if (call == null) {
            return;
        }

        mCallNodes.add(new CallNode(call));

        mCallStateNotifier.onCallCreated(call);
    }

    /**
     * Adds pending call for incoming call.
     */
    public synchronized void attachPreIncomingCall(Call call) {
        if (call == null) {
            return;
        }

        CallNode node = new CallNode(call);
        mPendingCallNodes.add(node);
    }

    public synchronized void closeAllCalls(boolean terminate) {
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

    public CallTracker getCallTracker() {
        return mCT;
    }

    public synchronized Call getCall(long callId) {
        CallNode node = getCallNode(callId);

        if (node != null) {
            return node.mCall;
        }

        return null;
    }

    /**
     * Gets pending call for incoming call.
     */
    public synchronized Call getPendingCall(long callId) {
        CallNode node = getPendingCallNode(callId);

        return (node != null) ? node.mCall : null;
    }

    public synchronized boolean hasAtLeastOneEstablishedCall() {
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

    public synchronized boolean hasCalls() {
        return !mCallNodes.isEmpty();
    }

    public synchronized boolean hasPendingCalls() {
        return !mPendingCallNodes.isEmpty();
    }

    public synchronized void terminateAllCalls() {
        List<Call> allCalls = getAllCalls();

        // Terminates all the calls
        for (Call call : allCalls) {
            if (call != null) {
                call.terminate(0, true);
            }
        }
    }

    public synchronized int getVacantCallIndex() {
        List<Call> allCalls = getAllCalls();

        int index = 1;

        while (true) {
            boolean indexExist = false;
            for (Call call : allCalls) {
                if (call.getCallIndex() == index) {
                    indexExist = true;
                    break;
                }
            }
            if (indexExist == false) {
                break;
            }
            index++;
        }

        return index;
    }

    private void checkAndNotifyNonTelephonyCallState(Call call) {
        // Special case for VZW
        if ((mTelephonyCsRegistry != null)
                && mTelephonyCsRegistry.isCallStateRequired()
                && call.isTerminatedByCSRetry()) {
            notifyNonTelephonyCallState(TelephonyCallStateRegistry.CALL_STATE_CS_RETRY, true);
        }
    }

    private void doCallStateChanged() {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                int callState = updateCallState();
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

    private synchronized int getActiveCallCount() {
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

    /** @param callState TelephonyCallStateRegistry#CALL_STATE_XXX */
    private void notifyNonTelephonyCallState(final int callState, boolean isAsync) {
        if (isAsync) {
            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    notifyNonTelephonyCallState(callState, false);
                }
            });
        } else {
            if (mTelephonyCsRegistry != null) {
                mTelephonyCsRegistry.updateNonTelephonyCallState(callState);
            }
        }
    }

    /** @param callState CallTracker#CALL_STATE_XXX */
    private void notifyTelephonyCallState(int callState, String incomingNumber) {
        if (mTelephonyCsRegistry != null) {
            mTelephonyCsRegistry.updateCallState(callState, incomingNumber);
        }
    }

    private synchronized void onCallCreate(Call call) {
        CallNode node = new CallNode(call);

        if (call.isConference() && call.isInCall()) {
            node.setCallState(CallTracker.CALL_STATE_OFFHOOK);
        }

        mCallNodes.add(node);
        doCallStateChanged();
        mCallStateNotifier.onCallCreated(call);
    }

    private synchronized void onCallEstablishing(Call call) {
        CallNode node = getCallNode(call.getNativeCallId());

        if (node != null) {
            node.setCallState(CallTracker.CALL_STATE_RINGBACK);
            doCallStateChanged();
            mCallStateNotifier.onCallEstablishing(call);
        }
    }

    private synchronized void onCallRinging(Call call) {
        mCallStateNotifier.onCallRinging(call);
    }

    private synchronized void onCallAccept(Call call) {
        CallNode node = getCallNode(call.getNativeCallId());

        if (node != null) {
            node.setCallState(CallTracker.CALL_STATE_OFFHOOK);
            doCallStateChanged();
            mCallStateNotifier.onCallAccepted(call);
        }
    }

    private synchronized void onCallEstablished(Call call) {
        CallNode node = getCallNode(call.getNativeCallId());

        if (node != null) {
            node.setCallState(CallTracker.CALL_STATE_OFFHOOK);
        }

        doCallStateChanged();
        mCallStateNotifier.onCallEstablished(call);
    }

    private synchronized void onCallUpdated(Call call) {
        doCallStateChanged();
        mCallStateNotifier.onCallUpdated(call);
    }

    private synchronized void onCallTerminating(Call call) {
        mCallStateNotifier.onCallTerminating(call);
    }

    private synchronized void onCallTerminated(Call call) {
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
            checkAndNotifyNonTelephonyCallState(call);
            doCallStateChanged();
            mCallStateNotifier.onCallTerminated(call);
        }
    }

    private synchronized void onCallDestroy(Call call) {
        CallNode node = getPendingCallNode(call.getNativeCallId());

        if (node != null) {
            mPendingCallNodes.remove(node);
            doCallStateChanged();
            mCallStateNotifier.onCallDestroyed(call);
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

    private synchronized void onCallIncomingReceived(Call call) {
        CallNode node = getCallNode(call.getNativeCallId());

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
    private synchronized int updateCallState() {
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
        boolean voiceCallStateChanged = updateCallStateForVoLte(voiceCallState);
        boolean videoCallStateChanged = updateCallStateForVt(videoCallState);

        if (voiceCallStateChanged || videoCallStateChanged) {
            Context context = mContext.getContext();
            int slotId = mContext.getSlotId();

            if ((mCallStateVoLte == MtcStateUtils.STATE_ACTIVE)
                    || (mCallStateVt == MtcStateUtils.STATE_ACTIVE)) {
                MtcStateUtils.updateCallState(context, slotId, MtcStateUtils.STATE_ACTIVE);
            } else {
                MtcStateUtils.updateCallState(context, slotId, MtcStateUtils.STATE_INACTIVE);
            }

            if (voiceCallStateChanged) {
                MtcStateUtils.notifyCallState(context, slotId,
                        mCallStateVoLte, MtcStateUtils.SERVICE_VOIP);
            }

            if (videoCallStateChanged) {
                MtcStateUtils.notifyCallState(context, slotId,
                        mCallStateVt, MtcStateUtils.SERVICE_VT);
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

    private synchronized void updateTelephonyCallState() {
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

        //4 need to check if the termination reason is SRVCC case

        notifyTelephonyCallState(callState, incomingNumber);
    }

    private boolean isImsRegisteredOnWifi() {
        IWifiState ws = (IWifiState)AgentFactory.getAgent(AgentFactory.WIFI_STATE);

        if (!ImsGlobal.isWfcEnabled(mContext.getContext(), mContext.getSlotId())) {
            return false;
        } else if (ws != null && (!ws.isWifiConnected())) {
            return false;
        }

        int networkType = ImsStateController.RegState.getNetworkTypeForPhoneId(
                mContext.getContext().getContentResolver(), mContext.getPhoneId());

        return networkType == TelephonyManager.NETWORK_TYPE_IWLAN;
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

            ImsStateController.CallState.putConnectedCallOnWifiForPhoneId(
                    mContext.getContext().getContentResolver(), state, mContext.getPhoneId());
        }
    }

    private static void log(String s) {
        ImsLog.d("[GII-MTC] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-MTC] " + s);
    }

    private class CallNode {
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
            if (mListeners.isEmpty()) {
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    for (CallStateListener l : mListeners) {
                        l.onCallCreated(call);
                    }
                }
            });
        }

        public void onCallDestroyed(final Call call) {
            if (mListeners.isEmpty()) {
                updateConnectedCallOnWifi(call, 0, "onCallDestroyed");
                return;
            }

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    updateConnectedCallOnWifi(call, 0, "onCallDestroyed");

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
