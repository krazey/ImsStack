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

package com.android.imsstack.imsservice.mmtel;

import android.os.PowerManager;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.emergency.EmergencyNumber;
import android.telephony.emergency.EmergencyNumber.EmergencyCallRouting;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsCallSession;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.SrvccCall;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.PhoneStateInterface;
import com.android.imsstack.core.config.ServiceCaps;
import com.android.imsstack.enabler.mtc.CallTracker;
import com.android.imsstack.enabler.mtc.ConferenceInfoHelper;
import com.android.imsstack.enabler.mtc.IECallStateTracker;
import com.android.imsstack.enabler.mtc.IUMtcCall;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.enabler.mtc.externalcalls.ExternalCalls;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.base.IMmTelCallListener;
import com.android.imsstack.imsservice.mmtel.internal.SrvccStateTracker;
import com.android.imsstack.internal.enabler.ImsStateStore;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.internal.imsservice.MmTelMediaQualityReporter;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsWakeLock;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class ImsCallManager {
    private static final int MAX_CALL_ID = Integer.MAX_VALUE;
    protected final ConcurrentHashMap<String, ImsCallSessionImpl> mSessions =
            new ConcurrentHashMap<String, ImsCallSessionImpl>(8, 0.9f, 1);
    protected final ConcurrentHashMap<String, ImsCallSessionImpl> mPendingSessions =
            new ConcurrentHashMap<String, ImsCallSessionImpl>(4, 0.9f, 1);
    private final Object mLock = new Object();
    private final ICallContext mCallContext;
    private final MtcApp mMtcApp;
    private final IMmTelCallListener mMmTelCallListener;
    private int mCallId = 1;
    private ImsCallTracker mCT = new ImsCallTracker();
    private MtcAppCallListenerProxy mCallListener = new MtcAppCallListenerProxy();
    protected WifiCallWakeLock mWifiCallWakeLock = null;
    protected ImsCallProfile mIncomingCallInfo = null;

    public ImsCallManager(ICallContext callContext, MtcApp mtcApp, IMmTelCallListener listener) {
        mCallContext = callContext;
        mMtcApp = mtcApp;
        mMmTelCallListener = listener;

        init();
    }

    /***
     * return ImsCallSessionImpl
     */
    @VisibleForTesting
    public ImsCallSessionImpl createImsCallSession(ICallContext callContext,
            CallTracker ct, MtcCall call, String callId,
            ImsCallProfile profile, boolean isMO) {
        ImsCallSessionImpl callSession =  new ImsCallSessionImpl(
                mCallContext, ct, call, callId, profile, isMO);

        MmTelMediaQualityReporter mediaQualityReporter =
                ImsServiceRegistry.getInstance(callContext.getSlotId())
                .createMediaQualityReporter(callId);

        callSession.getMtcCall().getMediaSession()
                .setMediaQualityReporter(mediaQualityReporter);

        return callSession;
    }

    public void dispose() {
        logi("dispose :: phoneId=" + getPhoneId());

        clear();
    }

    public void init() {
        log("init");
        mMtcApp.setCallListener(mCallListener);

        if (isCallOverWifiSupported()) {
            mWifiCallWakeLock = new WifiCallWakeLock();
        }
    }

    public void clear() {
        log("clear");

        mMtcApp.setCallListener(null);

        closeAllSessions();

        removeIncomingCallInfo();

        if (mWifiCallWakeLock != null) {
            mWifiCallWakeLock.clear();
            mWifiCallWakeLock = null;
        }
    }

    public void closeAllSessions() {
        terminateAllPendingSessions();
        terminateAllSessions(0);
        removePendingSessions();
        destroyAllSessions();

        synchronized (mPendingSessions) {
            mPendingSessions.clear();
        }

        synchronized (mSessions) {
            mSessions.clear();
        }
    }

    /**
     * Creates an {@link ImsCallSessionImpl} from a given ImsCallProfile.
     *
     * @param profile The {@link ImsCallProfile} to make the {@link ImsCallSessionImpl}.
     * @return {@link ImsCallSessionImpl}
     */
    @SuppressWarnings("deprecation") // EXTRA_CALL_RAT_TYPE
    public ImsCallSessionImpl createSession(ImsCallProfile profile) {
        boolean offline = false;
        boolean ussi = false;
        @EmergencyCallRouting int emergencyRouting =
                EmergencyNumber.EMERGENCY_CALL_ROUTING_UNKNOWN;

        int sessionAttributes = MtcCall.FLAG_MO;

        if (profile.getServiceType() == ImsCallProfile.SERVICE_TYPE_EMERGENCY) {
            sessionAttributes |= MtcCall.FLAG_EMERGENCY;
            emergencyRouting = ImsCallUtils.getEmergencyRoutingFromCallProfile(profile);

            // ECBM
            checkAndExitEcbm();

            if (String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN).equals(
                    ImsCallUtils.getCallExtraFromOemExtras(
                            profile, ImsCallProfile.EXTRA_CALL_RAT_TYPE, ""))) {
                sessionAttributes |=  MtcCall.FLAG_WIFI_EMERGENCY;
            }
        } else if (profile.getServiceType() == ImsCallProfile.SERVICE_TYPE_NORMAL) {
            if (profile.getCallExtraInt(ImsCallProfile.EXTRA_DIALSTRING, -1)
                    == ImsCallProfile.DIALSTRING_USSD) {
                ussi = true;
            }
        }

        if (ImsCallUtils.isVideoCall(profile.getCallType())) {
            sessionAttributes |= MtcCall.FLAG_VIDEO_CALL;
        }

        if (profile.getMediaProfile().isRttCall()) {
            sessionAttributes |= MtcCall.FLAG_RTT;
        }

        MtcCall call = mMtcApp.createMtcCallAndAttach(sessionAttributes);
        String callId = createCallId();

        if (call == null) {
            loge("Creating a MtcCall failed", null);
            // EXCEPTION_HANDLING: Call UI stuck
            return createImsCallSession(mCallContext, mCT, null, callId, profile, true);
        }

        ImsCallSessionImpl callSession = createImsCallSession(
                mCallContext, mCT, call, callId, profile, true);

        onCallCreate(callSession);
        // MO call created.
        updateImsCallState(TelephonyManager.CALL_STATE_OFFHOOK);

        // CALL_CONNECTION_ID
        int ccId = ImsCallConnectionIds.getNewId(mCallContext.getSlotId());
        callSession.setCallConnectionId(ccId);
        ImsCallConnectionIds.add(mCallContext.getSlotId(), ccId);

        if (call.isEmergencyCall()) {
            mMtcApp.openEmergencyService(call, emergencyRouting);
        } else {
            call.open(IUMtcCall.SERVICETYPE_NORMAL, IUMtcCall.EMERGENCYTYPE_NONE, offline, ussi);
        }

        return callSession;
    }

    public boolean takeSession(ImsCallSessionImpl callSession) {
        ImsCallSessionImpl newSession = null;

        synchronized (mPendingSessions) {
            log("takeSession :: callId=" + callSession.getCallId() +
                    ", pendingSessions=" + mPendingSessions.size());

            newSession = mPendingSessions.get(callSession.getCallId());

            if (newSession != null) {
                onCallCreate(newSession);
                // MT call is taken.
                updateImsCallState(TelephonyManager.CALL_STATE_OFFHOOK);

                // CALL_CONNECTION_ID
                int ccId = ImsCallConnectionIds.getNewId(mCallContext.getSlotId());
                newSession.setCallConnectionId(ccId);
                ImsCallConnectionIds.add(mCallContext.getSlotId(), ccId);

                newSession.takeCall();
            }
        }

        removeIncomingCallInfo();

        if (newSession != null) {
            removePendingSession(newSession);
            return true;
        }

        return false;
    }

    @SuppressWarnings("unchecked")
    public ImsCallSessionImpl getConferenceCall() {
        if (mSessions.isEmpty()) {
            return null;
        }

        for (Map.Entry<String, ImsCallSessionImpl> entry : mSessions.entrySet()) {
            ImsCallSessionImpl call = entry.getValue();

            if (call.getState() >= ImsCallSession.State.ESTABLISHED) {
                try {
                    String conference = call.getProperty(MtcCall.EXTRA_CONFERENCE);

                    if ((conference != null) && conference.equalsIgnoreCase("true")) {
                        return call;
                    }
                } catch (Throwable t) {
                    loge("Exception: " + t.toString(), t);
                }
            }
        }

        return null;
    }

    /**
     * Gets ImsCallSessionImpl which is in CallTracker.CALL_STATE_RINGING or
     * CallTracker.CALL_STATE_RINGBACK state.
     *
     * @return ImsCallSessionImpl
     */
    public ImsCallSessionImpl getConnectingSession() {
        synchronized (mSessions) {
            if (!mSessions.isEmpty()) {
                for (Map.Entry<String, ImsCallSessionImpl> entry : mSessions.entrySet()) {
                    MtcCall call = entry.getValue().getMtcCall();

                    if ((call != null) && !call.isInCall() && !call.isOnHold()) {
                        return entry.getValue();
                    }
                }
            }

            if (!mPendingSessions.isEmpty()) {
                for (Map.Entry<String, ImsCallSessionImpl> entry : mPendingSessions.entrySet()) {
                    MtcCall call = entry.getValue().getMtcCall();

                    if ((call != null)) {
                        return entry.getValue();
                    }
                }
            }
        }

        return null;
    }

    public ImsCallSessionImpl getActiveSession() {
        synchronized (mSessions) {
            if (mSessions.isEmpty()) {
                return null;
            }

            for (Map.Entry<String, ImsCallSessionImpl> entry : mSessions.entrySet()) {
                MtcCall call = entry.getValue().getMtcCall();

                if ((call != null) && call.isInCall() && !call.isOnHold()) {
                    return entry.getValue();
                }
            }
        }

        return null;
    }

    public ImsCallSessionImpl getHoldSession() {
        synchronized (mSessions) {
            if (mSessions.isEmpty()) {
                return null;
            }

            for (Map.Entry<String, ImsCallSessionImpl> entry : mSessions.entrySet()) {
                MtcCall call = entry.getValue().getMtcCall();

                if ((call != null) && call.isOnHold()) {
                    return entry.getValue();
                }
            }
        }

        return null;
    }

    public int getPhoneId() {
        return mCallContext.getPhoneId();
    }

    public MtcApp getMtcApp() {
        return mMtcApp;
    }

    public List<SrvccCall> getSrvccCalls() {
        List<SrvccCall> srvccCalls =  new ArrayList<SrvccCall>();
        Set<Map.Entry<String, ImsCallSessionImpl>> sessionsEntrySet = null;
        Set<Map.Entry<String, ImsCallSessionImpl>> pendingEntrySet = null;

        synchronized (mSessions) {
            if (!mSessions.isEmpty()) {
                sessionsEntrySet = mSessions.entrySet();
            }
        }

        synchronized (mPendingSessions) {
            if (!mPendingSessions.isEmpty()) {
                pendingEntrySet =  mPendingSessions.entrySet();
            }
        }

        if (sessionsEntrySet != null) {
            addSrvccCall(sessionsEntrySet, srvccCalls);
        }

        if (pendingEntrySet != null) {
            addSrvccCall(pendingEntrySet, srvccCalls);
        }

        return srvccCalls;
    }

    private void addSrvccCall(Set<Map.Entry<String, ImsCallSessionImpl>> entries,
            List<SrvccCall> srvccCalls) {
        for (Map.Entry<String, ImsCallSessionImpl> entry : entries) {
            ImsCallSessionImpl callSession = entry.getValue();

            String callId = callSession.getCallId();
            int preciseState = callSession.getPreciseState();

            log("addSrvccCall: callSession=" + callSession + ", callId="
                    + callId + ", preciseState=" + preciseState);

            srvccCalls.add(new SrvccCall(callId, preciseState,
                    callSession.getCallProfile()));
        }
    }

    private String createCallId() {
        // Call-Id: 1 ~ (Integer.MAX - 1)
        int newId = 0;
        String newCallId = "0";

        do {
            synchronized (mLock) {
                newId = mCallId;
                mCallId++;
                if (mCallId == MAX_CALL_ID) {
                    mCallId = 1;
                }
            }

            newCallId = String.valueOf(newId);
            synchronized (mSessions) {
                if (mSessions.get(newCallId) != null) {
                    log("createCallId: collision in active sessions - newId=" + newCallId);
                    newId = 0;
                }
            }

            synchronized (mPendingSessions) {
                if (mPendingSessions.get(newCallId) != null) {
                    log("createCallId: collision in pending sessions - newId=" + newCallId);
                    newId = 0;
                }
            }
        } while (newId == 0);

        return newCallId;
    }

    protected boolean isCallOverWifiSupported() {
        return ServiceCaps.isWfcEnabledByPlatform(mCallContext.getSlotId());
    }

    private void updateImsCallState(int state) {
        PhoneStateInterface phoneState = AgentFactory.getInstance().getAgent(
                PhoneStateInterface.class, mCallContext.getSlotId());
        if (phoneState != null) {
            phoneState.setImsCallState(state);
        }
    }

    private void checkAndExitEcbm() {
        // It requires to exit an ECBM if emergency call is initiated
        IECallStateTracker ecst = mCallContext.getECallStateTracker();

        if ((ecst != null) && ecst.isEcbmEntered()) {
            ecst.exitEmergencyCallbackMode(true);
        }
    }

    private void addPendingSession(ImsCallSessionImpl session) {
        synchronized (mPendingSessions) {
            removePendingSessions();
            mPendingSessions.put(session.getCallId(), session);

            log("pendingSessions=" + mPendingSessions.size() +
                    ", session=" + session.getCallId());
        }
    }

    private void removePendingSession(ImsCallSessionImpl session) {
        String key = session.getCallId();

        synchronized (mPendingSessions) {
            ImsCallSessionImpl s = mPendingSessions.remove(key);

            // Validity check
            if ((s != null) && (s != session)) {
                log("removePendingSession :: " + session +
                        " is not associated with key(" + key + ")");

                mPendingSessions.put(key, s);

                for (Map.Entry<String, ImsCallSessionImpl> entry : mPendingSessions.entrySet()) {
                    if (entry.getValue() == s) {
                        mPendingSessions.remove(entry.getKey());
                    }
                }
            }
        }
    }

    private void removeIncomingCallInfo() {
        if (mIncomingCallInfo != null) {
            log("removeIncomingCallInfo");

            mIncomingCallInfo = null;
        }
    }

    @SuppressWarnings("unchecked")
    private void removePendingSessions() {
        if (mPendingSessions.isEmpty()) {
            return;
        }

        for (Map.Entry<String, ImsCallSessionImpl> entry : mPendingSessions.entrySet()) {
            if (entry.getValue().getState() != ImsCallSession.State.NEGOTIATING) {
                mPendingSessions.remove(entry.getKey());
            }
        }
    }

    private void onCallCreate(ImsCallSessionImpl session) {
        synchronized (mSessions) {
            mSessions.put(session.getCallId(), session);

            // SRVCC_STATE_TRACKING
            if (mSessions.size() == 1) {
                SrvccStateTracker sst = (SrvccStateTracker) mCallContext.getSrvccStateTracker();

                if (sst != null) {
                    sst.start();
                }
            }
        }

        logi("onCallCreate :: sessions=" + mSessions.size() +
                ", session=" + session.getCallId());
    }

    private void onCallDestroy(ImsCallSessionImpl session) {
        String key = session.getCallId();

        synchronized (mPendingSessions) {
            if (mPendingSessions.containsKey(key)) {
                removePendingSession(session);
                return;
            }
        }

        synchronized (mSessions) {
            ImsCallSessionImpl s = mSessions.remove(key);

            // Validity check
            if ((s != null) && (s != session)) {
                log("onCallDestroy :: " + session +
                        " is not associated with key(" + key + ")");
                mSessions.put(key, s);

                for (Map.Entry<String, ImsCallSessionImpl> entry : mSessions.entrySet()) {
                    if (entry.getValue() == s) {
                        mSessions.remove(entry.getKey());
                    }
                }
            }

            // If all the sessions are destroyed,
            // then remove all the ConferenceInfo if present.
            if (mSessions.isEmpty()) {
                updateImsCallState(TelephonyManager.CALL_STATE_IDLE);
                ConferenceInfoHelper.destroyAllConferenceInfos();

                ImsGarbageCalls gc = ImsGarbageCalls.getInstance();

                if (gc.getCount(mCallContext.getSlotId()) >= ImsGarbageCalls.MAX_CALL) {
                    gc.removeAll(mCallContext.getSlotId());
                }

                // SRVCC_STATE_TRACKING
                SrvccStateTracker sst = (SrvccStateTracker) mCallContext.getSrvccStateTracker();

                if (sst != null) {
                    sst.stop();
                }

                // CALL_CONNECTION_ID
                ImsCallConnectionIds.removeAll(mCallContext.getSlotId());

                if (mWifiCallWakeLock != null) {
                    mWifiCallWakeLock.clearLock();
                }
            }
        }

        logi("onCallDestroy :: sessions=" + mSessions.size() +
                ", session=" + session.getCallId());

        updateCallProfileOnSingleCall();
    }

    private void onCallPreIncomingReceived(final ImsCallSessionImpl session) {
        if (mMmTelCallListener == null) {
            rejectAndDestroyCall(session, false);
            return;
        }

        addPendingSession(session);

        logi("onCallPreIncomingReceived :: callId=" + session.getCallId()
                + ", phoneId=" + getPhoneId());
    }

    private void onCallIncomingReceived(final ImsCallSessionImpl session) {
        if (mMmTelCallListener == null) {
            rejectAndDestroyCall(session, true);
            return;
        }

        try {
            logi("onCallIncomingReceived :: callId=" + session.getCallId()
                    + ", phoneId=" + getPhoneId());

            mMmTelCallListener.onIncomingCallReceived(session);
        } catch (Throwable t) {
            loge("onCallIncomingReceived :: exception occurred, drop the incoming call"
                    + t.getMessage(), t);
            rejectAndDestroyCall(session, true);
        }
    }

    private void onCallInfoReceived(final ImsCallProfile profile) {
        mIncomingCallInfo = profile;
        logi("onCallInfoReceived :: calltype=" + mIncomingCallInfo.getCallType());
    }

    private void postAndRunTask(Runnable task) {
        mCallContext.getExecutor().execute(task);
    }

    private void rejectAndDestroyCall(final ImsCallSessionImpl session,
            boolean isPendingSession) {
        // Reject the incoming call
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    session.reject(ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
                } catch (Throwable t) {
                    loge("Exception: " + t.toString(), t);
                }
            }
        });

        // Destroy the incoming call
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    session.close();
                    onCallDestroy(session);
                } catch (Throwable t) {
                    loge("Exception: " + t.toString(), t);
                }
            }
        });

        if (isPendingSession) {
            // Remove the pending session
            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    try {
                        session.close();
                        removePendingSession(session);
                    } catch (Throwable t) {
                        loge("Exception: " + t.toString(), t);
                    }
                }
            });
        }
    }

    @SuppressWarnings("unchecked")
    private void destroyAllSessions() {
        synchronized (mSessions) {
            if (mSessions.isEmpty()) {
                return;
            }

            for (Map.Entry<String, ImsCallSessionImpl> entry : mSessions.entrySet()) {
                onCallDestroy(entry.getValue());
            }
        }
    }

    @SuppressWarnings("unchecked")
    private void terminateAllSessions(int reason /* not used */) {
        synchronized (mSessions) {
            if (mSessions.isEmpty()) {
                return;
            }

            int count = 0;

            for (Map.Entry<String, ImsCallSessionImpl> entry : mSessions.entrySet()) {
                ImsCallSessionImpl callSession = entry.getValue();
                callSession.notifyCallTerminatedByServiceUnavailable();

                try {
                    callSession.close();
                    count++;
                } catch (Throwable t) {
                    loge("terminateAllSessions Exception" + t.getMessage(), t);
                }
            }

            log("terminateAllSessions :: closed=" + count);
        }
    }

    @SuppressWarnings("unchecked")
    private void terminateAllPendingSessions() {
        synchronized (mPendingSessions) {
            if (mPendingSessions.isEmpty()) {
                return;
            }

            int count = 0;

            for (Map.Entry<String, ImsCallSessionImpl> entry : mPendingSessions.entrySet()) {
                ImsCallSessionImpl callSession = entry.getValue();

                if (callSession.getState() == ImsCallSession.State.IDLE) {
                    try {
                        callSession.close();
                        count++;
                    } catch (Throwable t) {
                        loge("terminateAllPendingSessions exception:" + t.getMessage(), t);
                    }
                }
            }

            log("terminateAllPendingSessions :: closed=" + count);
        }
    }

    private int getActiveSessionCount() {
        synchronized (mSessions) {
            int count = mSessions.size();

            if (count == 0) {
                return 0;
            }

            int activeSessionCount = 0;

            for (Map.Entry<String, ImsCallSessionImpl> entry : mSessions.entrySet()) {
                ImsCallSessionImpl session = entry.getValue();

                if ((session != null) && session.isInCall()) {
                    activeSessionCount++;
                }
            }

            return activeSessionCount;
        }
    }

    private void updateCallProfileOnSingleCall() {
        if (!ImsCallUtils.isCallOnNativeAppsAndCountryKR(mCallContext)) {
            return;
        }

        ImsCallSessionImpl callSession = null;

        synchronized (mSessions) {
            int count = mSessions.size();

            if (count != 1) {
                return;
            }

            // When single call is remained, update the call profile
            for (Map.Entry<String, ImsCallSessionImpl> entry : mSessions.entrySet()) {
                ImsCallSessionImpl session = entry.getValue();

                if ((session != null) && session.isInCall()) {
                    callSession = session;
                    break;
                }
            }
        }

        if (callSession != null) {
            callSession.updateCallProfileByCallManager();
        }
    }

    private static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }

    private static void loge(String s, Throwable t) {
        if (t == null) {
            ImsLog.e("[GII-IMPL] " + s);
        } else {
            ImsLog.e("[GII-IMPL] " + s, t);
        }
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }

    protected class WifiCallWakeLock implements ImsStateStore.Listener {
        private int mCallState = ImsStateStore.STATE_INACTIVE;
        protected ImsWakeLock mWakeLock = null;

        @VisibleForTesting
        public WifiCallWakeLock(ImsWakeLock imsWakeLock) {
            mWakeLock = imsWakeLock;
        }

        public WifiCallWakeLock() {
            init();
        }

        public void clear() {
            ImsStateStore.getCallState(mCallContext.getPhoneId()).removeListener(this);

            clearLock();
            mWakeLock = null;
        }

        public void clearLock() {
            if (mWakeLock != null) {
                mWakeLock.release(this);
            }
        }

        public void init() {
            mCallState = ImsStateStore.STATE_INACTIVE;

            createLock();

            ImsStateStore.getCallState(mCallContext.getPhoneId()).addListener(this);
        }

        public void initStateAndLock() {
            mCallState = ImsStateStore.STATE_INACTIVE;

            clearLock();
            mWakeLock = null;

            createLock();
        }

        @Override
        public void onStateChanged() {
            final int callState = ImsStateStore.getCallState(
                    mCallContext.getPhoneId()).getConnectedCallOnWifi();

            if (mCallState != callState) {
                mCallState = callState;

                if (mWakeLock != null) {
                    if (mCallState != ImsStateStore.STATE_ACTIVE) {
                        mWakeLock.release(this);
                    } else {
                        mWakeLock.acquire(this);
                    }
                }
            }
        }

        private void createLock() {
            if (mWakeLock == null) {
                mWakeLock = new ImsWakeLock(
                        mCallContext.getContext().getSystemService(PowerManager.class),
                        WifiCallWakeLock.class.getSimpleName());
            }
        }
    }

    protected class ImsCallTracker implements CallTracker {
        @Override
        public String createCallId() {
            return ImsCallManager.this.createCallId();
        }

        @Override
        public int getActiveCalls() {
            return getActiveSessionCount();
        }

        @Override
        public void updateCallState(Object session, int event, Object extraInfo) {

            // If the session is null, do not handle any operations...
            if (session == null) {
                return;
            }

            if (!(session instanceof ImsCallSessionImpl)) {
                return;
            }

            switch (event) {
                case CALL_EVENT_CREATE:
                    onCallCreate((ImsCallSessionImpl) session);
                    break;
                case CALL_EVENT_DESTROY:
                    onCallDestroy((ImsCallSessionImpl) session);
                    break;
                case CALL_EVENT_INCOMING_RECEIVED:
                    onCallIncomingReceived((ImsCallSessionImpl) session);
                    break;
                case CALL_EVENT_ESTABLISHING: // FALL-THROUGH
                case CALL_EVENT_RINGING: // FALL-THROUGH
                case CALL_EVENT_ACCEPT: // FALL-THROUGH
                case CALL_EVENT_ESTABLISHED: // FALL-THROUGH
                case CALL_EVENT_UPDATED: // FALL-THROUGH
                case CALL_EVENT_TERMINATING: // FALL-THROUGH
                case CALL_EVENT_TERMINATED: // FALL-THROUGH
                default:
                    break;
            }
        }
    }

    protected class MtcAppCallListenerProxy extends MtcApp.CallListener {
        @Override
        public void onExternalCallStateChanged(MtcApp app, ExternalCalls externalCalls) {
            mMmTelCallListener.onImsExternalCallStateChanged(
                    externalCalls.getImsExternalCallStates());
        }

        @Override
        public void onPreIncomingCallReceived(MtcApp app, long nativeCallObject) {
            log("onPreIncomingCallReceived");
            MtcCall call = mMtcApp.getPendingCall(nativeCallObject);

            if (call == null) {
                // fatal error
                return;
            }

            ImsCallProfile profile = new ImsCallProfile();
            String callId = createCallId();

            ImsCallSessionImpl callSession = createImsCallSession(
                    mCallContext, mCT, call, callId, profile, false);

            onCallPreIncomingReceived(callSession);
        }
    }

    static {
        ImsSuppInfoUtils.init();
    }
}
