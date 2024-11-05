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

import com.android.imsstack.enabler.mtc.CallFeature;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.IUMtcCall;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.enabler.mtc.MtcCallUtils;
import com.android.imsstack.enabler.mtc.MtcConference;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.util.ImsLog;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

public class ConferenceProxy {
    public static interface DisposalCallback {
        public void onConferenceProxyDisposed(ConferenceProxy confProxy);
    }

    protected static class ListenerWrapper {
        public final MtcCall.Listener mListener;
        public final MtcConference.Listener mConferenceListener;

        public ListenerWrapper(MtcCall.Listener listener,
                MtcConference.Listener conferenceListener) {
            mListener = listener;
            mConferenceListener = conferenceListener;
        }

        @Override
        public boolean equals(Object o) {
            if (!(o instanceof ListenerWrapper)) {
                return false;
            }

            ListenerWrapper lw = (ListenerWrapper)o;

            return (mListener != null)
                && mListener.equals(lw.mListener)
                && (mConferenceListener != null)
                && mConferenceListener.equals(lw.mConferenceListener);
        }

        @Override
        public int hashCode() {
            int code = 17;

            code = 31 * code + ((mListener == null) ? 0 : 1);
            code = 31 * code + ((mConferenceListener == null) ? 0 : 1);

            if (mListener != null) {
                code += mListener.hashCode();
            }

            if (mConferenceListener != null) {
                code += mConferenceListener.hashCode();
            }

            return code;
        }
    }

    public static final int STATE_IDLE = 0;
    public static final int STATE_HOLDING = 1;
    public static final int STATE_SWAP_HOLDING = 2;
    public static final int STATE_UNHOLDING = 3;
    public static final int STATE_MERGE_WAITING = 4;
    public static final int STATE_MERGED = 5;
    public static final int STATE_CONFERENCE_EXTENDING = 6;
    public static final int STATE_CONFERENCE_EXTENDED = 7;

    protected final Set<ListenerWrapper> mListeners = new CopyOnWriteArraySet<ListenerWrapper>();

    private final ICallContext mCallContext;
    private ConferenceProxy.DisposalCallback mDisposalCallback;
    private MtcCall mConfCall = null;
    private boolean mInitialConferenceExtension = false;
    private int mState = STATE_IDLE;
    private UsersInfo mConferenceParticipants = null;

    public ConferenceProxy(ICallContext callContext) {
        mCallContext = callContext;
    }

    public void abort() {
        // no-op
    }

    public void dispose() {
        abort();
        setConferenceCall(null);
    }

    public UsersInfo getConferenceParticipants() {
        return mConferenceParticipants;
    }

    public MtcCall getConferenceCall() {
        return mConfCall;
    }

    public int getState() {
        return mState;
    }

    public boolean isConferenceExtensionRequestor(MtcCall c) {
        return false;
    }

    public boolean isConferenceForCallMerge() {
        return false;
    }

    /**
     * Checks if the call is initially extended to the conference call.
     * e.g. initial call merge, initial conference extension(SKT)
     */
    public boolean isInitialConferenceExtension() {
        return mInitialConferenceExtension;
    }

    public final void addListener(MtcCall.Listener listener,
            MtcConference.Listener conferenceListener) {
        mListeners.add(new ListenerWrapper(listener, conferenceListener));
    }

    public final boolean hasListeners() {
        return !mListeners.isEmpty();
    }

    public final void removeListener(MtcCall.Listener listener,
            MtcConference.Listener conferenceListener) {
        mListeners.remove(new ListenerWrapper(listener, conferenceListener));

        if (!hasListeners()) {
            postAndRun(new Runnable() {
                @Override
                public void run() {
                    if (mDisposalCallback != null) {
                        mDisposalCallback.onConferenceProxyDisposed(ConferenceProxy.this);
                    }
                }
            });
        }
    }

    public final void setDisposalCallback(ConferenceProxy.DisposalCallback callback) {
        mDisposalCallback = callback;
    }

    public boolean start(MtcApp app, boolean holdRequired) {
        if (mConfCall == null) {
            mConfCall = createConferenceCall(app);

            if (mConfCall == null) {
                return false;
            }
        }

        return startInternal(holdRequired);
    }

    protected final ICallContext getCallContext() {
        return mCallContext;
    }

    protected void closeConferenceSession() {
        if (mConfCall != null) {
            mConfCall.close();
            mConfCall = null;
        }
    }

    /**
     * gets the video direction when video call is on hold
     */
    public boolean isVideoDirectionInactiveOnVideoCallHold() {
        boolean videoDirection = CallFeature.isVideoDirectionInactiveOnVideoCallHold(
                getCallContext().getSlotId());
        return videoDirection;
    }

    /**
     * gets the text direction when RTT call is on Hold
     */
    public boolean isTextDirectionInactiveOnRttCallHold() {
        boolean textDirection = CallFeature.isTextDirectionInactiveOnRttCallHold(
                getCallContext().getSlotId());
        return textDirection;
    }

    protected void executeHold(final MtcCall call) {
        if (call == null) {
            return;
        }

        postAndRun(new Runnable() {
            @Override
            public void run() {
                call.hold(MtcCallUtils.createHoldMedia(
                        call.getCallInfo(),
                        call.getMediaInfo(),
                        isVideoDirectionInactiveOnVideoCallHold(),
                        isTextDirectionInactiveOnRttCallHold()));

                for (ListenerWrapper lw : mListeners) {
                    if (lw.mListener != null) {
                        lw.mListener.onCallProxyHold(call);
                    }
                }
            }
        });
    }

    protected void executeUnhold(final MtcCall call) {
        if (call == null) {
            return;
        }

        postAndRun(new Runnable() {
            @Override
            public void run() {
                call.resume(MtcCallUtils.createUnholdMedia(
                        call.getCallInfo(),
                        call.getMediaInfo(),
                        isVideoDirectionInactiveOnVideoCallHold()));

                for (ListenerWrapper lw : mListeners) {
                    if (lw.mListener != null) {
                        lw.mListener.onCallProxyResume(call);
                    }
                }
            }
        });
    }

    protected void notifySessionTerminated(final MtcCall call,
            final CallReasonInfo callReasonInfo) {
        postAndRun(new Runnable() {
            @Override
            public void run() {
                try {
                    for (ListenerWrapper lw : mListeners) {
                        if (lw.mListener != null) {
                            lw.mListener.onCallTerminated(call, callReasonInfo);
                        }
                    }
                } catch (Throwable t) {
                    loge("notifySessionTerminated", t);
                }
            }
        });
    }

    protected final void postAndRun(Runnable task) {
        mCallContext.getExecutor().execute(task);
    }

    protected final void setConferenceParticipants(UsersInfo usersInfo) {
        mConferenceParticipants = usersInfo;
    }

    protected final void setConferenceCall(MtcCall call) {
        mConfCall = call;
    }

    protected final void setInitialConferenceExtension(boolean initialConfExt) {
        mInitialConferenceExtension = initialConfExt;
    }

    protected void setState(int state) {
        if (mState != state) {
            logi("ConferenceProxy :: "
                    + stateToString(mState) + " >> " + stateToString(state));
            mState = state;
        }
    }

    protected boolean startInternal(boolean holdRequired) {
        // no-op
        return false;
    }

    protected void terminateConferenceSession() {
        if (mConfCall != null) {
            mConfCall.terminate(CallReasonInfo.CODE_USER_TERMINATED);
        }
    }

    protected static MtcCall createConferenceCall(MtcApp app) {
        if (app == null) {
            return null;
        }

        MtcCall confCall = app.createMtcCallAndAttach(MtcCall.FLAG_MO | MtcCall.FLAG_CONFERENCE);

        if (confCall != null) {
            confCall.open(IUMtcCall.SERVICETYPE_NORMAL, IUMtcCall.EMERGENCYTYPE_NONE,
                    false, false);
        }

        return confCall;
    }

    protected static CallReasonInfo createUnspecifiedCallReasonInfo() {
        return new CallReasonInfo(CallReasonInfo.CODE_UNSPECIFIED, 0, "");
    }

    protected static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }

    protected static void loge(String s, Throwable t) {
        if (t == null) {
            ImsLog.e("[GII-IMPL] " + s);
        } else {
            ImsLog.e("[GII-IMPL] " + s, t);
        }
    }

    protected static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }

    protected static String dbgLog(String[] s) {
        return ImsLog.hiddenString(s);
    }

    protected static String stateToString(int state) {
        switch (state) {
        case STATE_IDLE:
            return "IDLE";
        case STATE_HOLDING:
            return "HOLDING";
        case STATE_SWAP_HOLDING:
            return "SWAP_HOLDING";
        case STATE_UNHOLDING:
            return "UNHOLDING";
        case STATE_MERGE_WAITING:
            return "MERGE_WAITING";
        case STATE_MERGED:
            return "MERGED";
        case STATE_CONFERENCE_EXTENDING:
            return "CONFERENCE_EXTENDING";
        case STATE_CONFERENCE_EXTENDED:
            return "CONFERENCE_EXTENDED";
        default:
            return "__UNKNOWN__";
        }
    }
}
