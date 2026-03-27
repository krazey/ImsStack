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

import java.util.Arrays;

public class ConferenceExtensionProxy extends ConferenceProxy {
    private MtcCall mForegroundCall;
    private String[] mParticipants;
    private MtcCallListenerProxy mListenerProxy = new MtcCallListenerProxy();
    private MtcConferenceListenerProxy mConferenceListenerProxy = new MtcConferenceListenerProxy();

    public ConferenceExtensionProxy(ICallContext callContext,
            MtcCall fgCall, String[] participants) {
        super(callContext);

        mForegroundCall = fgCall;
        mParticipants = Arrays.copyOf(participants, participants.length);
        setInitialConferenceExtension(true);

        log("ConferenceExtensionProxy :: fg=" + fgCall
                + ", participants=" + dbgLog(participants));
    }

    @Override
    public void abort() {
        int state = getState();

        if (isInitialConferenceExtension()
                && (getConferenceCall() != null)
                && (state != STATE_CONFERENCE_EXTENDED)) {
            closeConferenceSession();
        }

        super.abort();
    }

    @Override
    public void dispose() {
        mForegroundCall = null;
        super.dispose();
    }

    @Override
    public boolean isConferenceExtensionRequestor(MtcCall c) {
        return (mForegroundCall != null) && mForegroundCall.equals(c);
    }

    @Override
    protected boolean startInternal(boolean holdRequired) {
        MtcCall confCall = getConferenceCall();

        if (confCall == null) {
            return false;
        }

        confCall.setListener(mListenerProxy);
        MtcCall.setListener(confCall, mConferenceListenerProxy);

        mForegroundCall.setListener(mListenerProxy);
        MtcCall.setListener(mForegroundCall, mConferenceListenerProxy);

        if (holdRequired) {
            setState(STATE_HOLDING);
            executeHold(mForegroundCall);
        } else {
            setState(STATE_CONFERENCE_EXTENDING);
            executeConferenceExtension();
        }

        return true;
    }

    @VisibleForTesting
    public MtcCallListenerProxy getMtcCallListener() {
        return mListenerProxy;
    }

    @VisibleForTesting
    public MtcConferenceListenerProxy getMtcConferenceListener() {
        return mConferenceListenerProxy;
    }

    private UsersInfo createUsersInfo() {
        MtcCall confCall = getConferenceCall();
        UsersInfo usersInfo = new UsersInfo();

        if ((confCall == null) || (mForegroundCall == null)) {
            return usersInfo;
        }

        MtcCallUtils.addUser(usersInfo, mForegroundCall.getNativeCallId(), "");

        if (mParticipants != null) {
            for (int i = 0; i < mParticipants.length; ++i) {
                MtcCallUtils.addUser(usersInfo, 0L, mParticipants[i]);
            }
        }

        return usersInfo;
    }

    private void executeConferenceExtension() {
        MtcCall confCall = getConferenceCall();

        if (confCall == null) {
            return;
        }

        MtcCall.extendToConference(confCall, createUsersInfo());

        for (ListenerWrapper lw : mListeners) {
            if (lw.mListener != null) {
                lw.mConferenceListener.onCallProxyExtendToConference(
                        MtcCall.getConference(confCall),
                        MtcCall.getConference(mForegroundCall),
                        mParticipants);
            }
        }
    }

    private void notifyConferenceFailed() {
        notifySessionConferenceExtendFailed(createUnspecifiedCallReasonInfo());
    }

    private void notifySessionConferenceExtended(final CallInfo callInfo,
            final MediaInfo mediaInfo, final SuppInfo suppInfo) {
        try {
            MtcCall confCall = getConferenceCall();

            if (confCall == null) {
                return;
            }

            for (ListenerWrapper lw : mListeners) {
                if (lw.mConferenceListener != null) {
                    lw.mConferenceListener.onCallConferenceExtended(
                            MtcCall.getConference(mForegroundCall),
                            confCall.getNativeCallId(),
                            callInfo, mediaInfo, suppInfo);
                }
            }
        } catch (Throwable t) {
            loge("notifySessionConferenceExtended", t);
        }
    }

    private void notifySessionConferenceExtendFailed(final CallReasonInfo callReasonInfo) {
        try {
            for (ListenerWrapper lw : mListeners) {
                if (lw.mConferenceListener != null) {
                    lw.mConferenceListener.onCallConferenceExtendFailed(
                            MtcCall.getConference(mForegroundCall), callReasonInfo);
                }
            }
        } catch (Throwable t) {
            loge("notifySessionConferenceExtendFailed", t);
        }
    }

    private class MtcCallListenerProxy extends MtcCall.Listener {
        @Override
        public void onCallTerminated(MtcCall call, CallReasonInfo callReasonInfo) {
            if (call.equals(getConferenceCall())) {
                notifyConferenceFailed();
            } else {
                notifySessionTerminated(call, callReasonInfo);

                int state = getState();

                if (state == STATE_HOLDING) {
                    // Merge or conference extension is not triggered in HOLDING state
                    notifyConferenceFailed();
                } else {
                    // If foreground or background call is terminated,
                    // then we need to restore the 1-to-1 call at most.
                    if (!MtcCallUtils.isCallTerminatedByJoiningConference(callReasonInfo.mCode)) {
                        if (state == STATE_CONFERENCE_EXTENDING) {
                            notifyConferenceFailed();
                        }
                    }
                }
            }
        }

        @Override
        public void onCallHeld(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.equals(mForegroundCall)) {
                return;
            }

            if (getState() == STATE_HOLDING) {
                setState(STATE_CONFERENCE_EXTENDING);
                executeConferenceExtension();
            }
        }

        @Override
        public void onCallHoldFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            if (!call.equals(mForegroundCall)) {
                return;
            }

            notifySessionConferenceExtendFailed(callReasonInfo);
        }

        @Override
        public void onCallHoldReceived(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
        }

        @Override
        public void onCallResumeReceived(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
        }
    }

    private class MtcConferenceListenerProxy extends MtcConference.Listener {
        @Override
        public void onCallConferenceExtended(MtcConference confCall,
                long confCallId/* not used in here */,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!confCall.isSameCall(MtcCall.getConference(getConferenceCall()))) {
                return;
            }

            setState(STATE_CONFERENCE_EXTENDED);
            notifySessionConferenceExtended(callInfo, mediaInfo, suppInfo);
        }

        @Override
        public void onCallConferenceExtendFailed(MtcConference confCall,
                CallReasonInfo callReasonInfo) {
            if (!confCall.isSameCall(MtcCall.getConference(getConferenceCall()))) {
                return;
            }

            notifySessionConferenceExtendFailed(callReasonInfo);
        }
    }
}
