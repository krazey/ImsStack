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

package com.android.imsstack.imsservice.mmtel.internal;

import android.telephony.CallQuality;

import com.android.imsstack.enabler.mtc.CallFeature;
import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.enabler.mtc.MtcCallUtils;
import com.android.imsstack.enabler.mtc.MtcConference;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.internal.annotations.VisibleForTesting;

public class MergeProxy extends ConferenceProxy {
    private static final int FLAG_RECOVER_FOREGROUND_CALL = 0x01;
    private static final int FLAG_RECOVER_BACKGROUND_CALL = 0x02;

    private MtcCall mForegroundCall;
    private MtcCall mBackgroundCall;
    private MtcCallListenerProxy mListenerProxy = new MtcCallListenerProxy();
    private MtcConferenceListenerProxy mConferenceListenerProxy = new MtcConferenceListenerProxy();
    private int mCallRecoveryRequired = 0;
    private CallReasonInfo mMergeFailedReason = null;

    public MergeProxy(ICallContext callContext,
            MtcCall fgCall, MtcCall bgCall) {
        super(callContext);

        mForegroundCall = fgCall;
        mBackgroundCall = bgCall;

        if (fgCall.isConference()) {
            setConferenceCall(fgCall);
        } else if (bgCall.isConference()) {
            setConferenceCall(bgCall);
        } else {
            setInitialConferenceExtension(true);
        }

        log("MergeProxy :: fg=" + fgCall + ", bg=" + bgCall);
    }

    @VisibleForTesting
    public MtcCallListenerProxy getMtcCallListener() {
        return mListenerProxy;
    }

    @VisibleForTesting
    public MtcConferenceListenerProxy getMtcConferenceListener() {
        return mConferenceListenerProxy;
    }

    @Override
    public void abort() {
        int state = getState();

        if (isInitialConferenceExtension()
                && (getConferenceCall() != null)
                && (state != STATE_MERGED)) {
            terminateConferenceSession();
            closeConferenceSession();
        }

        super.abort();
    }

    @Override
    public void dispose() {
        mForegroundCall = null;
        mBackgroundCall = null;
        super.dispose();
    }

    @Override
    public boolean isConferenceExtensionRequestor(MtcCall c) {
        return (mForegroundCall != null) && mForegroundCall.equals(c);
    }

    @Override
    public boolean isConferenceForCallMerge() {
        return (mBackgroundCall != null);
    }

    @Override
    protected boolean startInternal(boolean holdRequired) {
        MtcCall confCall = getConferenceCall();

        if (confCall == null) {
            return false;
        }

        if (isInitialConferenceExtension()) {
            confCall.setListener(mListenerProxy);
            MtcCall.setListener(confCall, mConferenceListenerProxy);
        }

        mForegroundCall.setListener(mListenerProxy);
        MtcCall.setListener(mForegroundCall, mConferenceListenerProxy);

        mBackgroundCall.setListener(mListenerProxy);
        MtcCall.setListener(mBackgroundCall, mConferenceListenerProxy);

        if (isInitialConferenceExtension() && holdRequired) {
            // hold -> merge
            setState(STATE_HOLDING);
            executeHold(mForegroundCall);
        } else if (confCall.isOnHold()
                 && !CallFeature.isCallMergeableOnConferenceOnHold(getCallContext().getSlotId())) {
            // hold -> resume -> merge
            setState(STATE_SWAP_HOLDING);
            executeHold(mForegroundCall);
        } else {
            setState(STATE_MERGE_WAITING);
            executeMerge();
        }

        return true;
    }

    private UsersInfo createUsersInfo() {
        MtcCall confCall = getConferenceCall();
        UsersInfo usersInfo = new UsersInfo();

        if ((confCall == null)
                || (mForegroundCall == null)
                || (mBackgroundCall == null)) {
            return usersInfo;
        }

        if (isInitialConferenceExtension()) {
            MtcCallUtils.addUser(usersInfo, mForegroundCall.getCallConnectionId(), "");
            MtcCallUtils.addUser(usersInfo, mBackgroundCall.getCallConnectionId(), "");
        } else {
            if (confCall.equals(mForegroundCall)) {
                MtcCallUtils.addUser(usersInfo, mBackgroundCall.getCallConnectionId(), "");
            } else {
                MtcCallUtils.addUser(usersInfo, mForegroundCall.getCallConnectionId(), "");
            }
        }

        return usersInfo;
    }

    private void executeMerge() {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                MtcCall confCall = getConferenceCall();

                if (confCall == null) {
                    return;
                }

                MtcCall.merge(confCall, createUsersInfo());

                for (ListenerWrapper lw : mListeners) {
                    if (lw.mConferenceListener != null) {
                        if (confCall.equals(mBackgroundCall)) {
                            lw.mConferenceListener.onCallProxyMerge(
                                    MtcCall.getConference(confCall),
                                    MtcCall.getConference(mBackgroundCall),
                                    MtcCall.getConference(mForegroundCall));
                        } else {
                            lw.mConferenceListener.onCallProxyMerge(
                                    MtcCall.getConference(confCall),
                                    MtcCall.getConference(mForegroundCall),
                                    MtcCall.getConference(mBackgroundCall));
                        }
                    }
                }
            }
        });
    }

    private void notifyConferenceFailed() {
        notifySessionMergeFailed(createUnspecifiedCallReasonInfo());
    }

    /* Glare condition: operations for foreground call */
    private void notifySessionHoldReceived(final MtcCall call,
            final CallInfo callInfo, final MediaInfo mediaInfo, final SuppInfo suppInfo) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                try {
                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mListener != null) {
                            lw.mListener.onCallHoldReceived(
                                    call, callInfo, mediaInfo, suppInfo);
                        }
                    }
                } catch (Throwable t) {
                    loge("notifySessionHoldReceived", t);
                }
            }
        });
    }

    private void notifySessionResumeReceived(final MtcCall call,
            final CallInfo callInfo, final MediaInfo mediaInfo, final SuppInfo suppInfo) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                try {
                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mListener != null) {
                            lw.mListener.onCallResumeReceived(
                                    call, callInfo, mediaInfo, suppInfo);
                        }
                    }
                } catch (Throwable t) {
                    loge("notifySessionResumeReceived", t);
                }
            }
        });
    }

    private void notifySessionAutoUpdated(final MtcCall call,
            final CallInfo callInfo, final MediaInfo mediaInfo, final SuppInfo suppInfo) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                try {
                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mListener != null) {
                            lw.mListener.onCallAutoUpdated(
                                    call, callInfo, mediaInfo, suppInfo);
                        }
                    }
                } catch (Throwable t) {
                    loge("notifySessionAutoUpdated", t);
                }
            }
        });
    }

    private void notifySessionUpdateReceived(final MtcCall call,
            final CallInfo callInfo, final MediaInfo mediaInfo, final SuppInfo suppInfo) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                try {
                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mListener != null) {
                            lw.mListener.onCallUpdateReceived(
                                    call, callInfo, mediaInfo, suppInfo);
                        }
                    }
                } catch (Throwable t) {
                    loge("notifySessionUpdateReceived", t);
                }
            }
        });
    }

    private void notifySessionInfoUpdated(final MtcCall call, final int type,
            final String strValue, final int intValue, final boolean booleanValue) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                try {
                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mListener != null) {
                            lw.mListener.onCallInfoUpdated(
                                    call, type, strValue, intValue, booleanValue);
                        }
                    }
                } catch (Throwable t) {
                    loge("notifySessionInfoUpdated", t);
                }
            }
        });
    }

    private void notifySessionMerged(final CallInfo callInfo,
            final MediaInfo mediaInfo, final SuppInfo suppInfo, final UsersInfo usersInfo) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                try {
                    MtcCall confCall = getConferenceCall();

                    if (confCall == null) {
                        return;
                    }

                    if (confCall.equals(mBackgroundCall)) {
                        for (ListenerWrapper lw : mListeners) {
                            if (lw.mConferenceListener != null) {
                                lw.mConferenceListener.onCallMerged(
                                        MtcCall.getConference(mForegroundCall),
                                        callInfo, mediaInfo, suppInfo, usersInfo);
                            }
                        }

                        for (ListenerWrapper lw : mListeners) {
                            if (lw.mConferenceListener != null) {
                                lw.mConferenceListener.onCallMerged(
                                        MtcCall.getConference(mBackgroundCall),
                                        callInfo, mediaInfo, suppInfo, usersInfo);
                            }
                        }
                    } else {
                        for (ListenerWrapper lw : mListeners) {
                            if (lw.mConferenceListener != null) {
                                lw.mConferenceListener.onCallMerged(
                                        MtcCall.getConference(mBackgroundCall),
                                        callInfo, mediaInfo, suppInfo, usersInfo);
                            }
                        }

                        for (ListenerWrapper lw : mListeners) {
                            if (lw.mConferenceListener != null) {
                                lw.mConferenceListener.onCallMerged(
                                        MtcCall.getConference(mForegroundCall),
                                        callInfo, mediaInfo, suppInfo, usersInfo);
                            }
                        }
                    }
                } catch (Throwable t) {
                    loge("notifySessionMerged", t);
                }
            }
        });
    }

    private void notifySessionMergeFailed(final CallReasonInfo callReasonInfo) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                MtcCall call1 = mBackgroundCall;
                MtcCall call2 = mForegroundCall;

                if ((call1 != null) && call1.isConference()) {
                    call1 = mForegroundCall;
                    call2 = mBackgroundCall;
                }

                try {
                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mConferenceListener != null) {
                            lw.mConferenceListener.onCallMergeFailed(
                                    MtcCall.getConference(call1),
                                    callReasonInfo);
                        }
                    }

                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mConferenceListener != null) {
                            lw.mConferenceListener.onCallMergeFailed(
                                    MtcCall.getConference(call2),
                                    callReasonInfo);
                        }
                    }
                } catch (Throwable t) {
                    loge("notifySessionMergeFailed", t);
                }
            }
        });
    }

    private void notifySessionMergeStarted(final CallInfo callInfo,
            final MediaInfo mediaInfo, final SuppInfo suppInfo) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                try {
                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mConferenceListener != null) {
                            lw.mConferenceListener.onCallMergeStarted(
                                    MtcCall.getConference(mForegroundCall),
                                    MtcCall.getConference(getConferenceCall()),
                                    callInfo, mediaInfo, suppInfo);
                        }
                    }
                } catch (Throwable t) {
                    loge("notifySessionMergeStarted", t);
                }
            }
        });
    }

    private void notifyAudioSessionOpened(MtcCall call) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                try {
                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mListener != null) {
                            lw.mListener.onAudioSessionOpened(call);
                        }
                    }
                } catch (Throwable t) {
                    loge("notifyAudioSessionOpened", t);
                }
            }
        });
    }

    private void notifyCallQualityChanged(MtcCall call, CallQuality callQuality) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                try {
                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mListener != null) {
                            lw.mListener.onCallQualityChanged(call, callQuality);
                        }
                    }
                } catch (Throwable t) {
                    loge("notifyCallQualityChanged", t);
                }
            }
        });
    }

    private void notifyTriggerAnbrQueryReceived(MtcCall call, int mediaType, int direction,
            int bitsPerSecond) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                try {
                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mListener != null) {
                            lw.mListener.onTriggerAnbrQueryReceived(call, mediaType, direction,
                                    bitsPerSecond);
                        }
                    }
                } catch (Throwable t) {
                    loge("notifyTriggerAnbrQueryReceived", t);
                }
            }
        });
    }

    private void notifyIncomingDtmfReceived(MtcCall call, int numDtmfDigit) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                try {
                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mListener != null) {
                            lw.mListener.onNotifyIncomingDtmfReceived(call, numDtmfDigit);
                        }
                    }
                } catch (Throwable t) {
                    loge("notifyIncomingDtmfReceived", t);
                }
            }
        });
    }

    private void notifyAudioSessionClosed(MtcCall call) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                try {
                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mListener != null) {
                            lw.mListener.onAudioSessionClosed(call);
                        }
                    }
                } catch (Throwable t) {
                    loge("notifyAudioSessionClosed", t);
                }
            }
        });
    }

    @VisibleForTesting
    protected boolean isBackgroundCallRecoveryRequired() {
        return (mCallRecoveryRequired & FLAG_RECOVER_BACKGROUND_CALL) != 0;
    }

    @VisibleForTesting
    protected boolean isForegroundCallRecoveryRequired() {
        return (mCallRecoveryRequired & FLAG_RECOVER_FOREGROUND_CALL) != 0;
    }

    @VisibleForTesting
    protected void setBackgroundCallRecovery(boolean flag) {
        log("BackgroundCallRecoveryRequired=" + flag);

        if (flag) {
            mCallRecoveryRequired |= FLAG_RECOVER_BACKGROUND_CALL;
        } else {
            mCallRecoveryRequired &= (~FLAG_RECOVER_BACKGROUND_CALL);
        }
    }

    @VisibleForTesting
    protected void setForegroundCallRecovery(boolean flag) {
        log("BackgroundCallRecoveryRequired=" + flag);

        if (flag) {
            mCallRecoveryRequired |= FLAG_RECOVER_FOREGROUND_CALL;
        } else {
            mCallRecoveryRequired &= (~FLAG_RECOVER_FOREGROUND_CALL);
        }
    }

    private boolean recoverForegroundCall(CallReasonInfo callReasonInfo) {
        if (isForegroundCallRecoveryRequired()) {
            if (mForegroundCall.isOnHold()
                    && !mForegroundCall.isTerminated()
                    && (mMergeFailedReason == null)) {
                executeUnhold(mForegroundCall);

                if (callReasonInfo != null) {
                    mMergeFailedReason = callReasonInfo;
                }
                return true;
            }
        } else if (mForegroundCall.isOnHold()
                && !mForegroundCall.isTerminated()) {
            setForegroundCallRecovery(true);

            if ((mBackgroundCall != null)
                    && mBackgroundCall.isConference()
                    && !mBackgroundCall.isTerminated()
                    && isBackgroundCallRecoveryRequired()) {
                executeUnholdImmediatelyOrDelayed(mForegroundCall);
            } else {
                executeUnhold(mForegroundCall);
            }

            if (callReasonInfo != null) {
                mMergeFailedReason = callReasonInfo;
            }
            return true;
        }

        return false;
    }

    private void executeUnholdImmediatelyOrDelayed(final MtcCall call) {
        /*
        final ICallContext callContext = getCallContext();
        Handler h = callContext.getCallHandler();

        h.postDelayed(new Runnable() {
                @Override
                public void run() {
                    logi("executeUnholdImmediatelyOrDelayed - delayed");
                    executeUnhold(call);
                }
            }, 500);
        */
        executeUnhold(call);
    }

    private class MtcCallListenerProxy extends MtcCall.Listener {
        @Override
        public void onCallTerminated(MtcCall call, CallReasonInfo callReasonInfo) {
            logi("onCallTerminated");

            int state = getState();

            if (call.equals(getConferenceCall())) {
                if (!isInitialConferenceExtension()) {
                    notifySessionTerminated(call, callReasonInfo);
                }

                if ((state == STATE_MERGE_WAITING)
                        || (state == STATE_SWAP_HOLDING)) {
                    if (!isInitialConferenceExtension()
                            && call.equals(mBackgroundCall)
                            && mForegroundCall.isInCall()
                            && !mForegroundCall.isTerminated()) {
                        setForegroundCallRecovery(true);
                        mMergeFailedReason = callReasonInfo;

                        if (state == STATE_MERGE_WAITING) {
                            executeUnhold(mForegroundCall);
                        }
                        return;
                    }
                }

                notifyConferenceFailed();
            } else {
                notifySessionTerminated(call, callReasonInfo);

                if ((state == STATE_HOLDING)
                        || (state == STATE_SWAP_HOLDING)) {
                    if (call.equals(mForegroundCall)) {
                        notifyConferenceFailed();
                    } else {
                        setForegroundCallRecovery(true);
                        mMergeFailedReason = createUnspecifiedCallReasonInfo();
                    }
                } else {
                    // If foreground or background call is terminated,
                    // then we need to restore the 1-to-1 call at most.
                    if (!MtcCallUtils.isCallTerminatedByJoiningConference(callReasonInfo.mCode)) {
                        if (state == STATE_MERGE_WAITING) {
                            if (isInitialConferenceExtension()
                                    && call.equals(mBackgroundCall)
                                    && mForegroundCall.isInCall()
                                    && !mForegroundCall.isTerminated()) {
                                setForegroundCallRecovery(true);
                                return;
                            }

                            // Call was already terminated by joining the conference,
                            // but it needs to be transited to "Normal Call Terminated" state
                            // by failing the call merge operation.
                            // This SESSION_TERMINATED event will be followed
                            // by MERGE_FAILED event.
                            if (!MtcCallUtils.isCallTerminatedByJoiningConference(
                                        callReasonInfo.mExtraCode)) {
                                notifyConferenceFailed();
                            }
                        }
                    }
                }

                if (isForegroundCallRecoveryRequired()
                        && call.equals(mForegroundCall)) {
                    setForegroundCallRecovery(false);

                    // In this case, the native will invoke the call merge failed event.
                    //if (mMergeFailedReason != null) {
                    //    notifySessionMergeFailed(mMergeFailedReason);
                    //}
                }
            }
        }

        @Override
        public void onCallHeld(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.equals(mForegroundCall)) {
                // Rollback for background call
                if (call.equals(mBackgroundCall)
                        && isBackgroundCallRecoveryRequired()) {
                    if (!recoverForegroundCall(null)
                            && mForegroundCall.isTerminated()) {
                        if (mMergeFailedReason == null) {
                            mMergeFailedReason = createUnspecifiedCallReasonInfo();
                        }

                        notifySessionMergeFailed(mMergeFailedReason);
                    }

                    setBackgroundCallRecovery(false);
                }
                return;
            }

            int state = getState();

            if (state == STATE_HOLDING) {
                if (isForegroundCallRecoveryRequired()) {
                    executeUnhold(call);
                    return;
                }

                setState(STATE_MERGE_WAITING);
                executeMerge();
            } else if ((state == STATE_SWAP_HOLDING)
                    && isForegroundCallRecoveryRequired()) {
                executeUnhold(call);
            } else if (mBackgroundCall != null) {
                // Unhold the conference call
                setState(STATE_UNHOLDING);
                executeUnhold(mBackgroundCall);
            }
        }

        @Override
        public void onCallHoldFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            if (!call.equals(mForegroundCall)) {
                // Rollback for background call
                if (call.equals(mBackgroundCall)
                        && isBackgroundCallRecoveryRequired()) {
                    setBackgroundCallRecovery(false);

                    if (!recoverForegroundCall(null)
                            && mForegroundCall.isTerminated()) {
                        notifySessionMergeFailed(
                                (mMergeFailedReason != null) ? mMergeFailedReason : callReasonInfo);
                    }
                }
                return;
            }

            if (isForegroundCallRecoveryRequired()
                    && (mMergeFailedReason != null)) {
                setForegroundCallRecovery(false);
                notifySessionMergeFailed(mMergeFailedReason);
            } else {
                notifySessionMergeFailed(callReasonInfo);
            }
        }

        @Override
        public void onCallHoldReceived(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            logi("onCallHoldReceived");

            if (call.equals(mForegroundCall) || call.equals(mBackgroundCall)) {
                notifySessionHoldReceived(call, callInfo, mediaInfo, suppInfo);
            }
        }

        @Override
        public void onCallResumed(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.equals(mBackgroundCall)) {
                if (call.equals(mForegroundCall)
                        && isForegroundCallRecoveryRequired()
                        && (mMergeFailedReason != null)) {
                    // Rollback case
                    setForegroundCallRecovery(false);
                    notifySessionMergeFailed(mMergeFailedReason);
                }
                return;
            }

            if (getState() == STATE_UNHOLDING) {
                setState(STATE_MERGE_WAITING);
                executeMerge();
            }
        }

        @Override
        public void onCallResumeFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            if (!call.equals(mBackgroundCall)) {
                if (call.equals(mForegroundCall)
                        && isForegroundCallRecoveryRequired()
                        && (mMergeFailedReason != null)) {
                    // Rollback case
                    setForegroundCallRecovery(false);
                    notifySessionMergeFailed(mMergeFailedReason);
                }
                return;
            }

            if (getState() == STATE_UNHOLDING) {
                setForegroundCallRecovery(true);
                executeUnhold(mForegroundCall);
                mMergeFailedReason = callReasonInfo;
                return;
            }

            notifySessionMergeFailed(callReasonInfo);
        }

        @Override
        public void onCallResumeReceived(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            log("onCallResumeReceived");

            if (call.equals(mForegroundCall) || call.equals(mBackgroundCall)) {
                notifySessionResumeReceived(call, callInfo, mediaInfo, suppInfo);
            }
        }

        @Override
        public void onCallAutoUpdated(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            log("onCallAutoUpdated");

            if (call.equals(mForegroundCall) || call.equals(mBackgroundCall)) {
                notifySessionAutoUpdated(call, callInfo, mediaInfo, suppInfo);
            }
        }

        @Override
        public void onCallUpdateReceived(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            log("onCallUpdateReceived");

            if (call.equals(mForegroundCall)) {
                if (getState() <= STATE_SWAP_HOLDING) {
                    notifySessionUpdateReceived(call, callInfo, mediaInfo, suppInfo);
                }
            }
        }

        @Override
        public void onCallInfoUpdated(MtcCall call, int type,
                String strValue, int intValue, boolean booleanValue) {
            log("onCallInfoUpdated");

            if (call.equals(mForegroundCall) || call.equals(mBackgroundCall)) {
                notifySessionInfoUpdated(call, type, strValue, intValue, booleanValue);
            }
        }

        /* MtcCall.Listener will not be available for Conference ImsCallSessionImpl session
         * When onAudioSessionOpened is triggered from Media for Conferencesession.
         * So pass this callback to ImsCallSessionImpl via MtcConference.Listener
         * notifyAudioSessionOpenedForConference() callback.
        */
        @Override
        public void onAudioSessionOpened(MtcCall call) {
            logi("onAudioSessionOpened");
            notifyAudioSessionOpened(call);
        }

        @Override
        public void onCallQualityChanged(MtcCall call, CallQuality callQuality) {
            logi("onCallQualityChanged");

            if (call.equals(getConferenceCall())) {
                call = mForegroundCall;
            }
            notifyCallQualityChanged(call, callQuality);
        }

        @Override
        public void onTriggerAnbrQueryReceived(MtcCall call, int mediaType, int direction,
                int bitsPerSecond) {
            logi("onTriggerAnbrQueryReceived");

            if (call.equals(getConferenceCall())) {
                call = mForegroundCall;
            }
            notifyTriggerAnbrQueryReceived(call, mediaType, direction, bitsPerSecond);
        }

        @Override
        public void onNotifyIncomingDtmfReceived(MtcCall call, int numDtmfDigit) {
            logi("onNotifyIncomingDtmfReceived");

            if (call.equals(getConferenceCall())) {
                call = mForegroundCall;
            }
            notifyIncomingDtmfReceived(call, numDtmfDigit);
        }

        @Override
        public void onAudioSessionClosed(MtcCall call) {
            logi("onAudioSessionClosed");
            notifyAudioSessionClosed(call);
        }
    }

    private class MtcConferenceListenerProxy extends MtcConference.Listener {
        @Override
        public void onCallMerged(MtcConference call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo, UsersInfo usersInfo) {
            MtcCall confCall = getConferenceCall();

            if (!call.isSameCall(MtcCall.getConference(confCall))) {
                return;
            }

            setState(STATE_MERGED);

            if ((confCall != null)
                    && !confCall.equals(mForegroundCall)
                    && !confCall.equals(mBackgroundCall)) {
                // Checks if the call merge is executed to extend to the conference initially
                notifySessionMergeStarted(callInfo, mediaInfo, suppInfo);
            }
            notifySessionMerged(callInfo, mediaInfo, suppInfo, usersInfo);

            if (CallFeature.isCallMergeableOnConferenceOnHold(getCallContext().getSlotId())
                    && mBackgroundCall.isConference()) {
                executeUnhold(mBackgroundCall);
            }
        }

        @Override
        public void onCallMergeFailed(MtcConference call, CallReasonInfo callReasonInfo) {
            MtcCall confCall = getConferenceCall();

            if (!call.isSameCall(MtcCall.getConference(confCall))) {
                return;
            }

            if ((mBackgroundCall != null)
                    && mBackgroundCall.equals(getConferenceCall())
                    && !mBackgroundCall.isOnHold()
                    && !mBackgroundCall.isTerminated()) {
                setBackgroundCallRecovery(true);
                executeHold(mBackgroundCall);
                mMergeFailedReason = callReasonInfo;
                return;
            }

            if (recoverForegroundCall(callReasonInfo)) {
                return;
            }

            notifySessionMergeFailed(callReasonInfo);
        }

        @Override
        public void onCallConferenceStateUpdated(MtcConference call,
                UsersInfo usersInfo) {
            // This is to remove the race condition when the conference state is updated
            // by the conference event package notification.
            MtcCall confCall = getConferenceCall();

            if (!call.isSameCall(MtcCall.getConference(confCall))) {
                return;
            }

            log("onCallConferenceStateUpdated");

            setConferenceParticipants(usersInfo);
        }
    }
}
