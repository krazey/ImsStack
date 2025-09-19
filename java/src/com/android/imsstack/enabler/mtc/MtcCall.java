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

import android.annotation.Nullable;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.telephony.CallQuality;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.ImsMediaSession;
import android.text.TextUtils;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.media.MediaConstants;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.jni.JniObjectId;
import com.android.imsstack.util.ImsArgs;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Set;

public class MtcCall extends Call implements ConferenceTracker {
    /**
     * Flags for call creation
     */
    // For emergency call
    public static final int FLAG_EMERGENCY = 0x00000001;
    // For conference call
    public static final int FLAG_CONFERENCE = 0x00000002;
    // For Wi-Fi emergency call
    public static final int FLAG_WIFI_EMERGENCY = 0x00000004;
    // For Video Call
    public static final int FLAG_VIDEO_CALL = 0x00000008;
    // For MO call
    public static final int FLAG_MO = 0x00000010;
    // For RTT call
    public static final int FLAG_RTT = 0x00000020;

    // @ delimiter and host part for anonymous URI
    private static final String ANONYMOUS_HOST = "@anonymous.invalid";

    /**
     * Listener interface for call operations via proxy
     */
    public static interface ProxyListener {
        public void onCallProxyHold(MtcCall call);
        public void onCallProxyResume(MtcCall call);
    }

    /**
     * Listener class for call operations
     */
    public static class Listener implements ProxyListener {
        @Override
        public void onCallProxyHold(MtcCall call) {
            // no-op
        }

        @Override
        public void onCallProxyResume(MtcCall call) {
            // no-op
        }

        public void onCallInitiating(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, int ratType) {
            // no-op
        }

        public void onCallProgressing(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        public void onCallStarted(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        /**
        * Called when the call setup is failed.
        *
        * @param call the object of this {@code MtcCall}
        * @param callReasonInfo detailed reason of the call setup failure
        */
        public void onCallStartFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            // no-op
        }

        /**
        * Called when the call is terminated.
        *
        * @param call the object of this {@code MtcCall}
        * @param callReasonInfo detailed reason of the call termination
        */
        public void onCallTerminated(MtcCall call, CallReasonInfo callReasonInfo) {
            // no-op
        }

        public void onCallHeld(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        /**
        * Called when the call hold is failed.
        *
        * @param call the object of this {@code MtcCall}
        * @param callReasonInfo detailed reason of the call hold failure
        */
        public void onCallHoldFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            // no-op
        }

        public void onCallHoldReceived(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        public void onCallResumed(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        /**
        * Called when the call resume is failed.
        *
        * @param call the object of this {@code MtcCall}
        * @param callReasonInfo detailed reason of the call resume failure
        */
        public void onCallResumeFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            // no-op
        }

        public void onCallResumeReceived(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        public void onCallAutoUpdated(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        public void onCallUpdated(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        /**
        * Called when the call update is failed.
        *
        * @param call the object of this {@code MtcCall}
        * @param callReasonInfo detailed reason of the call update failure
        */
        public void onCallUpdateFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            // no-op
        }

        public void onCallUpdateReceived(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        public void onCallUpdateResumeReceived(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        /**
         * When incoming call received, notify ImsCallManager.
         */
        public void onCallIncomingReceived(
                MtcCall call, IncomingMtcCall incomingCall, int ratType) {
            // no-op
        }

        /**
         * called when network is changed.
         */
        public void onCallRatChanged(int ratType) {
            // no-op
        }

        public void onCallTransferred(MtcCall call) {
            // no-op
        }

        /**
        * Called when the call transfer is failed.
        *
        * @param call the object of this {@code MtcCall}
        * @param callReasonInfo detailed reason of the call transfer failure
        */
        public void onCallTransferFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            // no-op
        }

        public void onCallTransferReceived(MtcCall call, MtcCall newCall, CallInfo callInfo,
                MediaInfo mediaInfo, SuppInfo suppInfo) {
            // no-op
        }

        /**
        * Called when the call push is completed.
        *
        * @param call the object of this {@code MtcCall}
        */
        public void onCallPushRequestCompleted(MtcCall call) {
            // no-op
        }

        /**
        * Called when the call push is failed.
        *
        * @param call the object of this {@code MtcCall}
        * @param callReasonInfo detailed reason of the call push failure
        */
        public void onCallPushRequestFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            // no-op
        }

        public void onCallInfoUpdated(MtcCall call, int type,
                String strValue, int intValue, boolean booleanValue) {
            // no-op
        }

        public void onCallRttMessageReceived(MtcCall call, String data) {
            // no-op
        }

        public void onCallRttAudioIndication(MtcCall call, boolean status) {
            // no-op
        }

        /**
         * Called when the AudioSession is opened
         */
        public void onAudioSessionOpened(MtcCall call) {
            // no-op
        }

        /**
         * Called when the AudioSession is closed
         */
        public void onAudioSessionClosed(MtcCall call) {
            // no-op
        }
        /**
        * Called when the call quality changed report is received
        *
        * @param call the object of this {@code MtcCall}
        * @param quality the call quality report object
        */
        public void onCallQualityChanged(MtcCall call, CallQuality quality) {
            // no-op
        }

        /**
         * Called when the RTP header extension data is received
         * @param extensions the RTP header extension data
         */
        public void onCallRtpHeaderExtensionsReceived(MtcCall call,
                Set<RtpHeaderExtension> extensions) {
            // no-op
        }

        /**
        * Trigger Anbr query to discuss with the network whether the current media bitrate
        * can be changed after receiving cmr.
        *
        * @param call the object of this {@code MtcCall}
        * @param mediaType is used to identify media stream such as audio or video.
        * @param direction of this packet stream (e.g. uplink or downlink).
        * @param bitsPerSecond This value is the bitrate requested by the other party UE
        *        through RTP CMR, RTCPAPP or TMMBR, and ImsStack converts this value
        *        to the MAC bitrate (defined in TS36.321, range: 0 ~ 8000 kbit/s).
        */
        public void onTriggerAnbrQueryReceived(MtcCall call, int mediaType, int direction,
                int bitsPerSecond) {
            // no-op
        }

        /**
         * A notification is sent when an incoming audio dtmf is received.
         * @param call the object of this {@code MtcCall}
         * @param numDtmfDigit Incoming audio dtmf digit
         */
        public void onNotifyIncomingDtmfReceived(MtcCall call, int numDtmfDigit) {
            // no-op
        }
    }

    /**
     * A listener interface for handling emergency call failures.
     *
     * This listener is designed to be implemented by a module
     * that needs to react to a specific type of emergency call failure,
     * such as when an already-opened service unexpectedly closes.
     */
    public interface IEmergencyCallFailureListener {
        /**
         * Called when an emergency call fails because the already-opened service was closed.
         *
         * This method signals to the listener that the failure was due to a loss of a
         * pre-existing emergency registration. Upon this event, the listener is expected
         * to perform a recovery action, such as re-opening the emergency service and
         * initiating a retry.
         * @return true if the service reopen process was started successfully, false otherwise.
         */
        boolean onEmergencyCallFailedByAlreadyOpenedServiceClosed();
    }

    /**
     * Listener interface for audio session callback
     */
    protected class AudioSessionListener extends MtcMediaSession.AudioListener {
        @Override
        public void onAudioSessionOpened() {
            Message.obtain(mHandler, MSG_AUDIO_SESSION_OPENED).sendToTarget();
        }

        @Override
        public void onAudioSessionClosed() {
            Message.obtain(mHandler, MSG_AUDIO_SESSION_CLOSED).sendToTarget();
        }

        @Override
        public void onCallQualityChanged(CallQuality callQuality) {
            Message.obtain(mHandler, MSG_AUDIO_QUALITY_CHANGED, callQuality).sendToTarget();
        }

        @Override
        public void onRtpHeaderExtensionsReceived(Set<RtpHeaderExtension> extensions) {
            Message.obtain(mHandler, MSG_AUDIO_RTP_EXTENSION_RECEIVED, extensions).sendToTarget();
        }

        @Override
        public void onTriggerAnbrQueryReceived(int mediaType, int direction, int bitsPerSecond) {
            Message.obtain(mHandler, MSG_AUDIO_TRIGGER_ANBR_QUERY_RECEIVED, mediaType, direction,
                    bitsPerSecond).sendToTarget();
        }

        @Override
        public void onNotifyIncomingDtmfReceived(int dtmfDigit) {
            Message.obtain(mHandler, MSG_AUDIO_INCOMING_DTMF_RECEIVED, dtmfDigit).sendToTarget();
        }
    }

    /**
     * Listener interface for RTT call
     */
    protected class TextSessionListener extends MtcMediaSession.TextListener {
        @Override
        public void onRttMessageReceived(MtcMediaSession session, String data) {
            log("onRttMessageReceived");

            if (session != mMediaSession) {
                return;
            }

            if (mListener == null) {
                log("onRttMessageReceived :: Listener is null");
                return;
            }

            mListener.onCallRttMessageReceived(MtcCall.this, data);
        }

        @Override
        public void onRttAudioIndication(MtcMediaSession session, boolean status) {
            log("onRttAudioIndication");

            if (session != mMediaSession) {
                return;
            }

            if (mListener == null) {
                log("onRttAudioIndication :: Listener is null");
                return;
            }

            mListener.onCallRttAudioIndication(MtcCall.this, status);
        }
    }

    private static final boolean DBG = ImsLog.isDebuggable();
    private static final int ONE_WAY_VIDEO_NONE = 0;
    private static final int ONE_WAY_VIDEO_BY_LOCAL_END = 1;
    private static final int ONE_WAY_VIDEO_BY_REMOTE_END = 2;

    /**
     * Internal messages
     * Requests from application : 101 ~
     * Requests from native : 201 ~
     * Requests from MediaEnabler : 301 ~
     */
    /** Args: Parcel */
    private static final int MSG_SEND_REQUEST = 101;
    /** Args: MtcCall */
    private static final int MSG_CLOSE = 102;
    /** Args: Long(nativeCallId) */
    private static final int MSG_CLEAR_INTERFACE = 103;

    /** Args: Parcel */
    private static final int MSG_MESSAGE_RECEIVED = 201;
    /** Args: CallReasonInfo */
    private static final int MSG_CALL_START_FAILED = 202;
    private static final int MSG_CALL_UPDATE_FAILED = 203;
    private static final int MSG_CALL_HOLD_FAILED = 204;
    private static final int MSG_CALL_RESUME_FAILED = 205;
    private static final int MSG_CALL_TERMINATED = 206;

    /** From MediaEnabler */
    private static final int MSG_AUDIO_SESSION_OPENED = 301;
    private static final int MSG_AUDIO_SESSION_CLOSED = 302;
    private static final int MSG_AUDIO_QUALITY_CHANGED = 303;
    private static final int MSG_AUDIO_RTP_EXTENSION_RECEIVED = 304;
    private static final int MSG_AUDIO_TRIGGER_ANBR_QUERY_RECEIVED = 305;
    private static final int MSG_AUDIO_INCOMING_DTMF_RECEIVED = 306;

    private final MessageHandler mHandler;
    private final JNIImsListenerProxy mNativeListener = new JNIImsListenerProxy();
    private final CallTracker mCT;
    private final MtcConference mConference;
    protected MtcMediaSession mMediaSession;
    private MtcCall.Listener mListener = null;
    private MtcCall.IEmergencyCallFailureListener mEmergencyCallFailureListener = null;
    protected MtcCall.AudioSessionListener mAudioListener = null;
    protected MtcCall.TextSessionListener mTextListener = null;
    private MtcCallInfo mCallInfo = null;
    private MediaInfo mMediaInfo = null;
    private CallReasonInfo mTerminationReason = null;
    private int mOldCallTypeOnUpdateAccept = IUMtcCall.CALLTYPE_VOIP;
    /** It will be controlled when audio is in sendrecv & video direction is only changed */
    private int mVideoState = ONE_WAY_VIDEO_NONE;
    private MtcJniProxy mMtcJniProxy;
    private boolean mUsingAlreadyOpenedEmergencyService = false;

    boolean mJniCreated;

    public MtcCall(IBaseContext context, CallTracker ct, int callAttributes,
            int index, String logTag) {
        super(context, index, logTag);

        mMtcJniProxy = MtcJniProxy.getInstance();
        mCT = ct;
        mHandler = new MessageHandler(mContext.getCallLooper());

        if ((callAttributes & FLAG_MO) != 0) {
            setDetails(Details.MO, true);
        } else {
            setDetails(Details.ON_PRE_INCOMING, true);
        }

        if ((callAttributes & FLAG_EMERGENCY) != 0) {
            setCallExtraBoolean(EXTRA_E_CALL, true);
        } else {
            createNativeCallObject();
        }

        if ((callAttributes & FLAG_CONFERENCE) != 0) {
            setCallExtraBoolean(EXTRA_CONFERENCE, true);
        }

        if ((callAttributes & FLAG_WIFI_EMERGENCY) != 0) {
            setCallExtraBoolean(EXTRA_WIFI_E_CALL, true);
        }

        if ((callAttributes & FLAG_VIDEO_CALL) != 0 && (callAttributes & FLAG_RTT) != 0) {
            setCallType(IUMtcCall.CALLTYPE_VIDEO_RTT);
        } else if ((callAttributes & FLAG_VIDEO_CALL) != 0) {
            setCallType(IUMtcCall.CALLTYPE_VT);
        } else if ((callAttributes & FLAG_RTT) != 0) {
            setCallType(IUMtcCall.CALLTYPE_RTT);
        }

        mCallInfo = new MtcCallInfo(
                    IUMtcCall.SERVICETYPE_NORMAL,
                    getCallType(),
                    isConference());

        mConference = new MtcConference(mContext.getCallLooper(), this, this);

        // ConferenceInfo: to manage the participants in the conference call
        if (isConference()) {
            ConferenceInfoHelper.createConferenceInfo(getCallId());
        }

        initMedia();

        logi(toString());
    }

    @VisibleForTesting
    public MtcCall(IBaseContext context, CallTracker ct,
            int index, String logTag, Looper looper, MtcConference mtcConference,
            MtcMediaSession mtcMediaSession, MtcJniProxy mtcJniProxy, CallInfo callInfo,
            MediaInfo mediaInfo) {
        super(context, index, logTag);

        mCT = ct;
        mHandler = new MessageHandler(looper);
        mConference = mtcConference;
        mMediaSession = mtcMediaSession;
        mAudioListener = new AudioSessionListener();
        mTextListener = new TextSessionListener();
        mMtcJniProxy = mtcJniProxy;
        mCallInfo = new MtcCallInfo(callInfo);
        mMediaInfo = mediaInfo;
    }

    @VisibleForTesting
    MtcMediaSession.AudioListener getAudioListener() {
        return mAudioListener;
    }

    @VisibleForTesting
    MtcMediaSession.TextListener getTextListener() {
        return mTextListener;
    }

    @Override
    public void close() {
        if (mHandler.hasMessages(MSG_CLOSE)) {
            return;
        }

        if (mHandler.hasMessages(MSG_SEND_REQUEST)) {
            closeInternal(MtcCall.this);
            return;
        }

        long nativeCallId = 0L;
        String callId = null;

        synchronized (this) {
            if (!isCallValid() && !isTerminatedByAutoRejectedCall()) {
                if (isEmergencyCall() && !mJniCreated) {
                    logi("close :: emergency call has been terminated before Native is ready");
                } else {
                    logi("close :: already closed");
                    return;
                }
            }

            log(toString());

            nativeCallId = getNativeCallId();
            callId = getCallId();

            super.close();
        }

        logi("close :: object=" + nativeCallId);

        setCallState(CallTracker.CALL_STATE_IDLE);

        // ConferenceInfo: to manage the participants in the conference call
        ConferenceInfoHelper.destroyConferenceInfo(callId);

        mConference.dispose();

        mAudioListener = null;
        mTextListener = null;

        requestCloseSessionToImsMediaSession();

        mMediaSession.setAudioListener(null);
        mMediaSession.setTextListener(null);
        mMediaSession.dispose();

        Message.obtain(mHandler, MSG_CLEAR_INTERFACE,
                Long.valueOf(nativeCallId)).sendToTarget();

        if (!hasDetails(Details.DETACHED)) {
            mCT.updateCallState(this, CallTracker.CALL_EVENT_DESTROY, null);
        }
    }

    @Override
    public void terminate(int reason, boolean immediateCallback) {
        logi("terminate :: call=" + Long.toHexString(getNativeCallId()) +
                ", reason=" + reason + ", immediateCallback=" + immediateCallback);

        if (!isCallValid()) {
            log("Call is already closed");
            return;
        }

        clearHoldState();
        setCallState(CallTracker.CALL_STATE_IDLE);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcCall.TERMINATE);
        parcel.writeInt(reason);

        sendRequest(parcel);

        if (immediateCallback && (mListener != null)) {
            CallReasonInfo callReasonInfo = new CallReasonInfo(
                    CallReasonInfo.CODE_USER_TERMINATED, 0, "");

            Message.obtain(mHandler, MSG_CALL_TERMINATED, callReasonInfo).sendToTarget();
        }

        mCT.updateCallState(this, CallTracker.CALL_EVENT_TERMINATING, null);
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("[ MtcCall: id=");
        sb.append(Long.toHexString(getNativeCallId()));
        sb.append(", state=");
        sb.append(callStateToString(getCallState()));
        sb.append(", index=");
        sb.append(getCallIndex());
        sb.append(", connectionid=");
        sb.append(getCallConnectionId() + "");

        if (mCallInfo == null) {
            sb.append(", { callInfo=null }");
        } else {
            sb.append(", { service=");
            sb.append(mCallInfo.getServiceType());
            sb.append(", call=");
            sb.append(mCallInfo.getCallType());
            sb.append(", conf=");
            sb.append(mCallInfo.isConference() ? "Y" : "N");
            sb.append(isConference() ? "-Y" : "-N");
            sb.append(", conf-avail=");
            sb.append(mCallInfo.isConferenceAvailable() ? "Y" : "N");
            sb.append(", conf-event=");
            sb.append(mCallInfo.isConferenceEventSupported() ? "Y" : "N");
            sb.append(" ");
            sb.append(mCallDetails);
            sb.append(" }");
        }

        if (mMediaInfo == null) {
            sb.append(", { mediaInfo=null }");
        } else {
            sb.append(", { AQ=");
            sb.append(mMediaInfo.AQuality);
            sb.append(", AD=");
            sb.append(mMediaInfo.ADir);
            sb.append(", VQ=");
            sb.append(mMediaInfo.VQuality);
            sb.append(", VD=");
            sb.append(mMediaInfo.VDir);
            sb.append(", TD=");
            sb.append(mMediaInfo.TDir);
            sb.append(", GTT=");
            sb.append(mMediaInfo.GTTMode);
            sb.append(" }");
        }

        sb.append(" ]");

        return sb.toString();
    }

    @Override
    public void updateConferenceState(Object conference,
            int event, ImsArgs args) {
        switch (event) {
        case EVENT_MERGE: // FALL-THROUGH
        case EVENT_EXTEND_TO_CONFERENCE: // FALL-THROUGH
        case EVENT_INVITE_PARTICIPANTS: // FALL-THROUGH
        case EVENT_REMOVE_PARTICIPANTS: // FALL-THROUGH
        case EVENT_DELETE_PARTICIPANTS:
            break;

        case EVENT_EXTENDED: {
                CallInfo callInfo = (CallInfo) args.mArg1;
                MediaInfo mediaInfo = (MediaInfo) args.mArg2;
                SuppInfo suppInfo = (SuppInfo) args.mArg3;
                long jniConfCallId = args.mLongArg;

                logi("EVENT_EXTENDED :: call="
                        + Long.toHexString(getNativeCallId()));

                setCallState(CallTracker.CALL_STATE_OFFHOOK);

                // Update the call/media info.
                updateCallParameters(callInfo, mediaInfo, suppInfo);

                if (jniConfCallId == 0) {
                    setAdhocGroup();
                    // ConferenceInfo: to manage the participants in the conference call
                    ConferenceInfoHelper.createConferenceInfo(getCallId());
                } else {
                    MtcCall confCall = createAndSetMtcCallForConference(
                            jniConfCallId, callInfo, mediaInfo, suppInfo);

                    mCT.updateCallState(confCall, CallTracker.CALL_EVENT_CREATE, null);
                }
                break;
        }

        case EVENT_EXTEND_RECEIVED: {
                CallInfo callInfo = (CallInfo) args.mArg1;
                MediaInfo mediaInfo = (MediaInfo) args.mArg2;
                SuppInfo suppInfo = (SuppInfo) args.mArg3;
                long jniConfCallId = args.mLongArg;

                logi("EVENT_EXTEND_RECEIVED :: call="
                        + Long.toHexString(getNativeCallId()));

                // Update the call/media info.
                updateCallParameters(callInfo, mediaInfo, suppInfo);

                if (jniConfCallId == 0) {
                    // ConferenceInfo: to manage the participants in the conference call
                    ConferenceInfoHelper.createConferenceInfo(getCallId());
                } else {
                    MtcCall confCall = createAndSetMtcCallForConference(
                            jniConfCallId, callInfo, mediaInfo, suppInfo);

                    mCT.updateCallState(confCall, CallTracker.CALL_EVENT_CREATE, null);
                }
                break;
        }

        case EVENT_MERGED: {
            CallInfo callInfo = (CallInfo)args.mArg1;
            MediaInfo mediaInfo = (MediaInfo)args.mArg2;
            SuppInfo suppInfo = (SuppInfo)args.mArg3;

            logi("EVENT_MERGED :: call="
                    + Long.toHexString(getNativeCallId()));

            int oldCallState = getCallState();

            setCallState(CallTracker.CALL_STATE_OFFHOOK);

            // Update the call/media info.
            updateCallParameters(callInfo, mediaInfo, suppInfo);

            if (DBG && (oldCallState == CallTracker.CALL_STATE_IDLE)) {
                log("MERGED :: " + this);
            }

            mCT.updateCallState(this, CallTracker.CALL_EVENT_ESTABLISHED, null);
            break;
        }

        case EVENT_INVITE_PARTICIPANTS_REQUEST_DELIVERED: // FALL-THROUGH
        case EVENT_REMOVE_PARTICIPANTS_REQUEST_DELIVERED: // FALL-THROUGH
        case EVENT_EXTEND_FAILED: // FALL-THROUGH
        case EVENT_MERGE_FAILED: // FALL-THROUGH
        case EVENT_INVITE_PARTICIPANTS_REQUEST_FAILED: // FALL-THROUGH
        case EVENT_REMOVE_PARTICIPANTS_REQUEST_FAILED: // FALL-THROUGH
        case EVENT_DELETE_PARTICIPANTS_REQUEST_COMPLETED: // FALL-THROUGH
        default:
            break;
        }

        if (args != null) {
            args.recycle();
        }
    }

    /**
     * Creates interface between Java and Native for call.
     */
    public void createNativeCallObject() {
        long nativeCallObject = mMtcJniProxy.getJniInterfaceAndSetListener(
                mContext.getSlotId(), JniObjectId.MTC_CALL, mNativeListener);
        super.updateNativeCallObject(nativeCallObject);

        mJniCreated = true;
    }

    public void detach() {
        if (!isCallValid()) {
            return;
        }

        if (DBG) {
            log("detach() :: " + this);
        }

        setCallState(CallTracker.CALL_STATE_IDLE);
        setDetails(Details.DETACHED, true);

        mCT.updateCallState(this, CallTracker.CALL_EVENT_DESTROY, null);
    }

    public MtcConference getConferenceInterface() {
        return mConference;
    }

    public MediaInfo getMediaInfo() {
        return mMediaInfo;
    }

    public CallInfo getCallInfo() {
        return mCallInfo.getCallInfo();
    }

    public CallReasonInfo getTerminationReason() {
        return mTerminationReason;
    }

    public boolean is1WayVideo() {
        return (mVideoState & ONE_WAY_VIDEO_BY_LOCAL_END) != 0;
    }

    public boolean is1WayVideoByRemoteEnd() {
        return (mVideoState & ONE_WAY_VIDEO_BY_REMOTE_END) != 0;
    }

    public void updateCallExtras(IncomingMtcCall incomingCall) {
        if (incomingCall == null) {
            return;
        }

        setCallType(MtcCallInfo.getCallType(incomingCall.callInfo));

        // FIXME: is this required?
        updateCallParameters(incomingCall.callInfo,
                incomingCall.mediaInfo, incomingCall.suppInfo);
        updateCallExtraForEmergency(incomingCall.callInfo);
        updateAutoRejectedCall(incomingCall.isAutoRejectedCall());

        // Boolean extra information
        setCallExtraBoolean(EXTRA_VMS, false);
        setCallExtraBoolean(EXTRA_CONFERENCE_AVAIL, false);

        // Integer extra information
        setCallExtraInt(EXTRA_OIR, incomingCall.OIPType);
        setCallExtraInt(EXTRA_CNAP, incomingCall.OIPType);

        // String extra information
        setCallExtra(EXTRA_OI, incomingCall.callerPartyNum);

        // Store the callee information as the conference user identity
        updateConferenceUserId(incomingCall.callerPartyNum);
    }

    public void setListener(MtcCall.Listener listener) {
        logi("setListener :: " + ((listener != null) ? getCallId() : "(null)"));

        mListener = listener;

        if ((mListener != null) && isTerminated()) {
            if (!isOnceInCall()) {
                notifyStartFailed();
            } else {
                Message.obtain(mHandler, MSG_CALL_TERMINATED,
                        getOrCreateTerminationReason()).sendToTarget();
            }
        }
    }

    /**
     * Sets the listener to be notified when an emergency call fails.
     */
    public void setEmergencyCallFailureListener(
            @Nullable MtcCall.IEmergencyCallFailureListener listener) {
        mEmergencyCallFailureListener = listener;
    }

    /**
     * Creates an outgoing call before it starts.
     */
    public void open(int serviceType, int emergencyType, boolean offline, boolean ussi,
            boolean usingAlreadyOpenedEmergencyService) {
        mUsingAlreadyOpenedEmergencyService = usingAlreadyOpenedEmergencyService;
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcCall.OPEN);
        parcel.writeInt(serviceType);
        parcel.writeInt(getCallType());
        parcel.writeInt(emergencyType);
        parcel.writeInt(offline ? 1 : 0);
        parcel.writeInt(ussi ? 1 : 0);

        parcel.writeString(getLogTag());

        sendRequest(parcel);
    }

    public void start(int callType, String callee, String actualCallee,
            MediaInfo mediaInfo, SuppInfo suppInfo) {
        logi("start :: callType=" + callType
                + ", callee=" + ImsLog.hiddenString(callee));

        // Store the call type of this call
        setCallType(callType);
        setRemoteNumber(callee);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcCall.START);
        parcel.writeInt(callType);
        parcel.writeString(callee);

        if (DBG) {
            log(MtcCallUtils.toString(mediaInfo) + ", " + MtcCallUtils.toString(suppInfo));
        }

        mediaInfo.writeToParcel(parcel, 1);
        suppInfo.writeToParcel(parcel, 1);

        parcel.writeString(getLogTag());

        // Update the call/media info.
        mCallInfo.setCallType(callType);
        MtcCallUtils.copyMediaInfo(mediaInfo, mMediaInfo);

        sendRequest(parcel);

        // Store the callee information as the conference user identity
        if (!TextUtils.isEmpty(actualCallee)) {
            setCallExtra(EXTRA_TI_ORIGIN, actualCallee);
            updateConferenceUserId(actualCallee);
        } else {
            setCallExtra(EXTRA_TI_ORIGIN, callee);
            updateConferenceUserId(callee);
        }

        mCT.updateCallState(this, CallTracker.CALL_EVENT_ESTABLISHING, null);
    }

    public void startConference(int callType, UsersInfo usersInfo,
            MediaInfo mediaInfo, SuppInfo suppInfo) {
        log("startConference :: callType=" + callType);

        setCallType(callType);

        setCallExtraBoolean(EXTRA_CONFERENCE, true);
        // ConferenceInfo: to manage the participants in the conference call
        ConferenceInfoHelper.createConferenceInfo(getCallId());

        Parcel parcel = Parcel.obtain();

        // For exception handling, it is removed from here (150418)
        //if (usersInfo.getSize() <= 0) {
        //    loge("No participants for conference call setup");
        //    return;
        //}

        parcel.writeInt(IUMtcCall.STARTCONF);
        parcel.writeInt(callType);

        if (DBG) {
            log(MtcCallUtils.toString(mediaInfo) + ", " + MtcCallUtils.toString(usersInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));
        }

        mediaInfo.writeToParcel(parcel, 1);
        suppInfo.writeToParcel(parcel, 1);
        usersInfo.writeToParcel(parcel, 1);

        // Update the call/media info.
        mCallInfo.setCallType(callType);
        mCallInfo.setConference(true);
        mCallInfo.setConferenceAvailable(false);
        MtcCallUtils.copyMediaInfo(mediaInfo, mMediaInfo);

        sendRequest(parcel);

        mCT.updateCallState(this, CallTracker.CALL_EVENT_ESTABLISHING, null);
    }

    /**
     * need to notify the Native to process remain works for incoming call after
     * receiving pre-incoming event completed.
     */
    public void attach(long nativeCallKey) {
        log("attach for incoming");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcCall.ATTACH);
        parcel.writeLong(nativeCallKey);

        sendRequest(parcel);
    }

    public void alertUser() {
        log("alertUser");

        setCallState(CallTracker.CALL_STATE_RINGING);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcCall.USER_ALERT);
        parcel.writeString(getLogTag());

        sendRequest(parcel);

        mCT.updateCallState(this, CallTracker.CALL_EVENT_RINGING, null);
    }

    public void accept(int callType, MediaInfo mediaInfo) {
        logi("accept :: callType=" + callType
                + " " + ((mediaInfo != null) ? MtcCallUtils.toString(mediaInfo) : ""));

        // RACE_CONDITION (Incoming call & CANCEL)
        if (isTerminated() && !isOnceInCall()) {
            notifyStartFailed();
            return;
        }

        // Set the call type when user answers the call
        int oldCallType = getCallType();
        checkAndSetCallType(callType);

        int oldUpdateState = getUpdateState();
        Parcel parcel = Parcel.obtain();

        if (!isInCall()) {
            parcel.writeInt(IUMtcCall.ACCEPT);
        } else if (oldUpdateState == UPDATE_STATE_RESUME_RECEIVED) {
            parcel.writeInt(IUMtcCall.ACCEPT_RESUME);
            mOldCallTypeOnUpdateAccept = oldCallType;
        } else {
            parcel.writeInt(IUMtcCall.ACCEPT_UPDATE);
            mOldCallTypeOnUpdateAccept = oldCallType;
        }

        parcel.writeInt(callType);

        // Update the call/media info.
        mCallInfo.setCallType(callType);

        if (mediaInfo != null) {
            mediaInfo.writeToParcel(parcel, 1);

            MtcCallUtils.copyMediaInfo(mediaInfo, mMediaInfo);
        }

        if (isInCall()) {
            if (oldUpdateState == UPDATE_STATE_RESUME_RECEIVED) {
                setUpdateState(UPDATE_STATE_RESUME_ACCEPTED);
            } else {
                setUpdateState(UPDATE_STATE_ACCEPTED);
            }
        }

        parcel.writeString(getLogTag());

        sendRequest(parcel);

        setCallState(CallTracker.CALL_STATE_OFFHOOK);

        mCT.updateCallState(this, CallTracker.CALL_EVENT_ACCEPT, null);
    }

    public void reject(int reason) {
        logi("reject :: reason=" + reason);

        // RACE_CONDITION (Incoming call & CANCEL)
        if (isTerminated() && !isOnceInCall()) {
            notifyStartFailed();
            return;
        }

        Parcel parcel = Parcel.obtain();

        if (!isInCall()) {
            parcel.writeInt(IUMtcCall.REJECT);
        } else if (getUpdateState() == UPDATE_STATE_RESUME_RECEIVED) {
            parcel.writeInt(IUMtcCall.REJECT_RESUME);
        } else {
            parcel.writeInt(IUMtcCall.REJECT_UPDATE);
        }

        parcel.writeInt(reason);

        if (isInCall()) {
            setUpdateState(UPDATE_STATE_REJECTED);
        }

        sendRequest(parcel);
    }

    public void terminate(int reason) {
        logi("terminate() :: call=" + Long.toHexString(getNativeCallId()) +
                ", reason=" + reason);

        if (!isCallValid()) {
            log("Call is already closed");
            return;
        }

        clearHoldState();
        setCallState(CallTracker.CALL_STATE_IDLE);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcCall.TERMINATE);
        parcel.writeInt(reason);

        sendRequest(parcel);

        mCT.updateCallState(this, CallTracker.CALL_EVENT_TERMINATING, null);
    }

    public void hold(MediaInfo mediaInfo) {
        log("hold");

        if (isTerminated()) {
            Message.obtain(mHandler, MSG_CALL_HOLD_FAILED,
                    getOrCreateTerminationReason()).sendToTarget();
            return;
        }

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcCall.HOLD);

        if (DBG) {
            log(MtcCallUtils.toString(mediaInfo));
        }

        mediaInfo.writeToParcel(parcel, 1);

        sendRequest(parcel);
    }

    public void resume(MediaInfo mediaInfo) {
        log("resume");

        if (isTerminated()) {
            Message.obtain(mHandler, MSG_CALL_RESUME_FAILED,
                    getOrCreateTerminationReason()).sendToTarget();
            return;
        }

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcCall.RESUME);

        if (DBG) {
            log(MtcCallUtils.toString(mediaInfo));
        }

        mediaInfo.writeToParcel(parcel, 1);

        sendRequest(parcel);
    }

    public void update(int callType, MediaInfo mediaInfo) {
        logi("update :: callType=" + callType);

        if (DBG) {
            log(MtcCallUtils.toString(mediaInfo));
        }

        if (isTerminated()) {
            Message.obtain(mHandler, MSG_CALL_UPDATE_FAILED,
                    getOrCreateTerminationReason()).sendToTarget();
            return;
        } else if (getUpdateState() == UPDATE_STATE_RECEIVED) {
            // GLARE_CONDITION: between call mode changes
            CallReasonInfo callReasonInfo = new CallReasonInfo(
                    CallReasonInfo.CODE_REJECT_ONGOING_CALL_UPGRADE, 0, "");
            Message.obtain(mHandler, MSG_CALL_UPDATE_FAILED, callReasonInfo).sendToTarget();
            return;
        }

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcCall.UPDATE);
        parcel.writeInt(callType);

        mediaInfo.writeToParcel(parcel, 1);

        setUpdateState(UPDATE_STATE_SENT);

        sendRequest(parcel);
    }

    public void transfer(String target) {
        boolean isTargetEmpty = TextUtils.isEmpty(target);

        log("transfer :: target="
                + (isTargetEmpty ? "null" : ImsLog.hiddenString(target)));

        Parcel parcel = Parcel.obtain();

        if (isTargetEmpty) {
            parcel.writeInt(IUMtcCall.REQUEST_ECT);
        } else {
            parcel.writeInt(IUMtcCall.REQUEST_ECT_BLIND);
            parcel.writeString(target);
        }

        sendRequest(parcel);
    }

    public void requestCallPush(String targetDevice) {
        log("requestCallPush");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcCall.REQUEST_CALL_PUSH);
        parcel.writeString(targetDevice);

        sendRequest(parcel);
    }

    public void cancelCallPush() {
        log("cancelCallPush");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcCall.CANCEL_CALL_PUSH);

        sendRequest(parcel);
    }

    /**
     * Sends dtmf character
     * @param code a character of dtmf code
     */
    public void sendDtmf(char code) {
        log("sendDtmf :: code=" + code);

        if (mMediaSession != null) {
            mMediaSession.sendDtmf(code);
        }
    }

    public void sendRttMessage(String data) {
        if (mMediaSession != null) {
            mMediaSession.sendRttMessage(data);
        }
    }

    /**
     * Requests that {@code rtpHeaderExtensions} are sent as a header extension with the next
     * RTP packet sent by the ImsMedia.
     * @param rtpHeaderExtensions The header extensions to be included in the next RTP header.
     */
    public void sendRtpHeaderExtensions(Set<RtpHeaderExtension> rtpHeaderExtensions) {
        if (mMediaSession != null) {
            mMediaSession.sendRtpHeaderExtensions(rtpHeaderExtensions);
        }
    }

    /**
     * Deliver the bitrate for the indicated media type, direction and bitrate to the upper layer.
     *
     * @param mediaType MediaType is used to identify media stream such as audio or video.
     * @param direction Direction of this packet stream (e.g. uplink or downlink).
     * @param bitsPerSecond This value is the bitrate received from the NW through the Recommended
     *        bitrate MAC Control Element message and ImsStack converts this value from MAC bitrate
     *        to audio/video codec bitrate (defined in TS26.114).
     */
    public void notifyAnbr(int mediaType, int direction, int bitsPerSecond) {
        logi("notifyAnbr - call mediaSession");

        if (mMediaSession != null) {
            mMediaSession.notifyAnbr(mediaType, direction, bitsPerSecond);
        }
    }

    public void sendUssd(String ussdMessage) {
        log("sendUssd :: ussdMessage=" + ussdMessage);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcCall.SEND_USSD);
        parcel.writeString(ussdMessage);

        sendRequest(parcel);
    }

    public void updateConferenceUserId(String confUid) {
        int anonymousId = 0;

        if (TextUtils.isEmpty(confUid)) {
            anonymousId = ConferenceInfoHelper.getAnonymousId();
            confUid = ANONYMOUS + anonymousId + ANONYMOUS_HOST;
        } else {
            // Do percent-encoding if remote end's number contains '#'
            confUid = confUid.replace("#", "%23");
        }

        String oldConfUid = getCallExtra(EXTRA_CONFERENCE_USER_ID, "");

        if (TextUtils.isEmpty(oldConfUid)) {
            setCallExtra(EXTRA_CONFERENCE_USER_ID, confUid);

            if (anonymousId > 0) {
                ConferenceInfoHelper.setAnonymousId(anonymousId + 1);
            }
        } else if (oldConfUid.length() >= 9) {
            String prefix = oldConfUid.substring(0, 9);

            if (ANONYMOUS.equalsIgnoreCase(prefix)) {
                setCallExtra(EXTRA_CONFERENCE_USER_ID, confUid);

                if (anonymousId > 0) {
                    ConferenceInfoHelper.setAnonymousId(anonymousId + 1);
                }
            }
        }
    }

    public MtcMediaSession getMediaSession() {
        // FIXME: define the media session
        return mMediaSession;
    }

    /**
     * Invokes fake event like there is an incoming call to notify auto missed call.
     *
     * @param incomingCall the object that has information of an incoming call.
     */
    public void invokeIncomingCallReceivedForAutoRejecting(IncomingMtcCall incomingCall) {
        log("invokeIncomingCallReceivedForAutoRejecting");
        mNativeListener.onIncomingCallReceived(incomingCall, 0);
    }

    private boolean checkAndHandleMediaMessage(int msg, Parcel parcel) {
        if (MtcMediaSession.isMessageForMediaSession(msg)) {
            if (mMediaSession != null) {
                parcel.setDataPosition(0);
                mMediaSession.onMessage(parcel);
            } else {
                loge("MtcMediaSession is null");
            }
            return true;
        }

        // Message is not for media session.
        return false;
    }

    protected void initMedia() {
        mMediaInfo = new MediaInfo(
                MediaInfo.AUDIO_QUALITY_AMR_WB,
                MediaInfo.VIDEO_QUALITY_NONE,
                MediaInfo.DIRECTION_SEND_RECEIVE,
                MediaInfo.DIRECTION_INVALID,
                MediaInfo.DIRECTION_INVALID,
                MediaInfo.GTTMODE_INVALID);

        mMediaSession = new MtcMediaSession(mContext, this);

        mAudioListener = new AudioSessionListener();
        mMediaSession.setAudioListener(mAudioListener);

        mTextListener = new TextSessionListener();
        mMediaSession.setTextListener(mTextListener);
    }

    protected MtcCall createAndSetMtcCallForConference(
            long jniConfCallId, CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
        MtcCall confCall = new MtcCall(mContext, mCT, isEmergencyCall()
                ? (FLAG_EMERGENCY | FLAG_CONFERENCE) : FLAG_CONFERENCE, -1, "EX");

        confCall.updateNativeCallObject(jniConfCallId);
        confCall.setAdhocGroup();
        confCall.setCallType(getCallType());
        confCall.setCallState(CallTracker.CALL_STATE_OFFHOOK);
        // FIXME: Is it a correct information for the new call?
        confCall.updateCallParameters(callInfo, mediaInfo, suppInfo);

        return confCall;
    }

    private boolean checkAndSetCallType(int callType) {
        if (getCallType() != callType) {
            logi("CallTypeChanged :: " +
                    getCallType() + " >> " + callType);
            setCallType(callType);

            // FIXME: how to restore the conversation state?
            clearHoldState();
            return true;
        }

        return false;
    }

    private void clearHoldState() {
        if (isOnHeld()) {
            log("clearHoldState :: on-held");
            setOnHeld(false);
        }

        if (isOnHold()) {
            log("clearHoldState :: on-hold");
            setOnHold(false);
        }
    }

    private CallReasonInfo getOrCreateTerminationReason() {
        return (mTerminationReason != null) ? new CallReasonInfo(mTerminationReason) :
                new CallReasonInfo(CallReasonInfo.CODE_USER_TERMINATED_BY_REMOTE, 0, "");
    }

    private void notifyStartFailed() {
        logi("notifyStartFailed");

        if (getCallState() != CallTracker.CALL_STATE_IDLE) {
            setCallState(CallTracker.CALL_STATE_IDLE);
            mCT.updateCallState(this, CallTracker.CALL_EVENT_TERMINATED, null);
        }

        CallReasonInfo callReasonInfo = (mTerminationReason != null)
                ? new CallReasonInfo(mTerminationReason) :
                new CallReasonInfo(CallReasonInfo.CODE_SIP_REQUEST_CANCELLED, 0, "");

        Message.obtain(mHandler, MSG_CALL_START_FAILED, callReasonInfo).sendToTarget();
    }

    private void removeAllCallExtrasFromSuppInfo() {
        // It removes the call extra parameters which are related to SuppInfo.
        removeCallExtra(EXTRA_MMC);
        removeCallExtra(EXTRA_GTT);
        removeCallExtra(EXTRA_VMS);
        removeCallExtra(EXTRA_CDIV_CAUSE);
        removeCallExtra(EXTRA_CNA);
        removeCallExtra(EXTRA_CDIV_HISTORY);
    }

    private void sendRequest(Parcel parcel) {
        if (!isCallValid()) {
            parcel.recycle();
            loge("Call is already closed");
            return;
        }

        Message.obtain(mHandler, MSG_SEND_REQUEST, parcel).sendToTarget();
    }

    private void setHoldStateAsLocalEnd(MediaInfo mediaInfo) {
        int callType = getCallType();

        if (!MtcCallUtils.hasVideo(callType)) {
            if (MtcCallUtils.isHoldMediaOnVoiceCall(mediaInfo)) {
                setOnHold(true);
            }
        } else {
            boolean isVideoDirectionInactiveOnVideoCallHold
                    = CallFeature.isVideoDirectionInactiveOnVideoCallHold(
                        mContext.getSlotId());

            if (MtcCallUtils.isHoldMediaOnVideoCall(mediaInfo,
                    isVideoDirectionInactiveOnVideoCallHold)) {
                setOnHold(true);
            } else if (isOnHold() && MtcCallUtils.isUnholdMediaOnVideoCall(mediaInfo,
                    isVideoDirectionInactiveOnVideoCallHold)) {
                setOnHold(false);
            }
        }
    }

    private void setHoldStateAsRemoteEnd(MediaInfo mediaInfo) {
        int callType = getCallType();

        if (!MtcCallUtils.hasVideo(callType)) {
            if (MtcCallUtils.isHoldMediaOnVoiceCallByRemoteEnd(mediaInfo)) {
                setOnHeld(true);
            }
        } else {
            boolean isVideoDirectionInactiveOnVideoCallHold
                    = CallFeature.isVideoDirectionInactiveOnVideoCallHold(
                        mContext.getSlotId());

            if (MtcCallUtils.isHoldMediaOnVideoCallByRemoteEnd(mediaInfo,
                    isVideoDirectionInactiveOnVideoCallHold)) {
                setOnHeld(true);
            } else if (isOnHeld()
                    && MtcCallUtils.isUnholdMediaOnVideoCallByRemoteEnd(mediaInfo)) {
                setOnHeld(false);
            }
        }
    }

    private void setHoldStateOnCallStarted(MediaInfo mediaInfo) {
        int callType = getCallType();

        if (!MtcCallUtils.hasVideo(callType)) {
            if (MtcCallUtils.isHoldMediaOnVoiceCall(mediaInfo)) {
                setOnHold(true);
            }

            if (MtcCallUtils.isHoldMediaOnVoiceCallByRemoteEnd(mediaInfo)) {
                setOnHeld(true);
            }
        } else {
            boolean isVideoDirectionInactiveOnVideoCallHold
                    = CallFeature.isVideoDirectionInactiveOnVideoCallHold(
                        mContext.getSlotId());

            if (MtcCallUtils.isHoldMediaOnVideoCall(mediaInfo,
                    isVideoDirectionInactiveOnVideoCallHold)) {
                setOnHold(true);
            }

            if (MtcCallUtils.isHoldMediaOnVideoCallByRemoteEnd(mediaInfo,
                    isVideoDirectionInactiveOnVideoCallHold)) {
                setOnHeld(true);
            }
        }
    }

    private void setVideoHoldState(MediaInfo mediaInfo) {
        if (!MtcCallUtils.hasVideo(getCallType())) {
            return;
        }

        int updateState = getUpdateState();

        if (updateState == UPDATE_STATE_ACCEPTED) {
            setHoldStateAsRemoteEnd(mediaInfo);
        } else if (updateState == UPDATE_STATE_SENT) {
            setHoldStateAsLocalEnd(mediaInfo);
        }
    }

    private void setVideoState(MediaInfo mediaInfo) {
        int oldVideoState = mVideoState;
        int callType = getCallType();

        if (!MtcCallUtils.hasVideo(callType)) {
            mVideoState = ONE_WAY_VIDEO_NONE;
        } else {
            if (!isOnHold() && !isOnHeld()) {
                if (MtcCallUtils.is1WayVideo(mediaInfo)) {
                    mVideoState |= ONE_WAY_VIDEO_BY_LOCAL_END;
                    mVideoState &= (~ONE_WAY_VIDEO_BY_REMOTE_END);
                } else if (MtcCallUtils.is1WayVideoByRemoteEnd(mediaInfo)) {
                    mVideoState &= (~ONE_WAY_VIDEO_BY_LOCAL_END);
                    mVideoState |= ONE_WAY_VIDEO_BY_REMOTE_END;
                } else {
                    mVideoState = ONE_WAY_VIDEO_NONE;
                }
            }
        }

        if (oldVideoState != mVideoState) {
            logi("VideoState :: " + oldVideoState + " >> " + mVideoState);
        }
    }

    private void updateCallExtraForEmergency(CallInfo si) {
        setCallExtraBoolean(EXTRA_E_CALL,
                (MtcCallInfo.getServiceType(si) == IUMtcCall.SERVICETYPE_EMERGENCY) ?
                    true : false);
    }

    private void updateCallExtraFromSuppInfo(SuppInfo si) {
        removeAllCallExtrasFromSuppInfo();

        for (SuppServiceUtils.SuppService ss : si.getServices()) {
            if (MtcCallUtils.isSuppInfoBoolean(ss.type)) {
                String key = SuppServiceUtils.getKey(ss.type);

                if (key != null) {
                    setCallExtraBoolean(key, ss.boolValue);
                }
            } else if (MtcCallUtils.isSuppInfoInt(ss.type)) {
                String key = SuppServiceUtils.getKey(ss.type);

                if (key != null) {
                    setCallExtraInt(key, ss.intValue);
                }
            } else if (MtcCallUtils.isSuppInfoString(ss.type)) {
                String key = SuppServiceUtils.getKey(ss.type);

                if (key != null) {
                    setCallExtra(key, ss.strValue);
                }
            }
        }
    }

    private void updateCallParameters(
            CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
        mCallInfo.copyFrom(callInfo);
        MtcCallUtils.copyMediaInfo(mediaInfo, mMediaInfo);

        // Call extras
        setCallExtraBoolean(EXTRA_CONFERENCE, mCallInfo.isConference());
        setCallExtraBoolean(EXTRA_CALL_MODE_CHANGEABLE, isVideoCapable(callInfo));
        setCallExtraBoolean(EXTRA_CONFERENCE_EVENT, mCallInfo.isConferenceEventSupported());
        setCallExtraBoolean(EXTRA_RTT_AVAIL, mCallInfo.isRttCapable());
        setCallExtra(EXTRA_USSD, mCallInfo.isUssi() ? "true" : "false");

        updateCallExtraFromSuppInfo(suppInfo);
    }

    private void updateAutoRejectedCall(boolean autoRejectedCall) {
        setDetails(Details.TERMINATED_BY_AUTO_REJECTED_CALL, autoRejectedCall);
    }

    private void requestCloseSessionToImsMediaSession() {
        Parcel parcelAudio = Parcel.obtain();
        parcelAudio.writeInt(MediaConstants.REQUEST_CLOSE_SESSION);
        parcelAudio.writeInt(ImsMediaSession.SESSION_TYPE_AUDIO);
        parcelAudio.setDataPosition(0);
        mMediaSession.onMessage(parcelAudio);
        parcelAudio.recycle();

        Parcel parcelVideo = Parcel.obtain();
        parcelVideo.writeInt(MediaConstants.REQUEST_CLOSE_SESSION);
        parcelVideo.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        parcelVideo.setDataPosition(0);
        mMediaSession.onMessage(parcelVideo);
        parcelVideo.recycle();

        Parcel parcelText = Parcel.obtain();
        parcelText.writeInt(MediaConstants.REQUEST_CLOSE_SESSION);
        parcelText.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        parcelText.setDataPosition(0);
        mMediaSession.onMessage(parcelText);
        parcelText.recycle();
    }

    private CallReasonInfo updateCallReasonInfoOnInvalidCallKey(
            final CallReasonInfo callReasonInfo) {
        if (callReasonInfo.mCode != CallReasonInfo.CODE_LOCAL_INTERNAL_ERROR) {
            return callReasonInfo;
        }
        if (callReasonInfo.mExtraCode
                != CallReasonInfo.EXTRA_CODE_INTERNAL_ERROR_INVALID_CALL_KEY) {
            return callReasonInfo;
        }
        return new CallReasonInfo(CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED,
                isEmergencyCall() ? CallReasonInfo.EXTRA_CODE_CALL_RETRY_EMERGENCY
                : CallReasonInfo.EXTRA_CODE_CALL_RETRY_SILENT_REDIAL, "");
    }

    private boolean isCallFailedByAlreadyOpenedServiceClosed(
            final CallReasonInfo callReasonInfo) {
        if (!mUsingAlreadyOpenedEmergencyService) {
            return false;
        }

        if (callReasonInfo.mCode == CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED
                && CallReasonInfo.EXTRA_MESSAGE_AOS_DISCONNECTED.equals(
                        callReasonInfo.mExtraMessage)) {
            return true;
        }

        if (callReasonInfo.mCode == CallReasonInfo.CODE_LOCAL_INTERNAL_ERROR
                && callReasonInfo.mExtraCode
                        == CallReasonInfo.EXTRA_CODE_INTERNAL_ERROR_INVALID_CALL_KEY) {
            return true;
        }

        return false;
    }

    private static void closeInternal(final MtcCall call) {
        if (call == null) {
            return;
        }

        Message.obtain(call.mHandler, MSG_CLOSE, call).sendToTarget();
    }

    private static void rejectInternal(final MtcCall call,
            final int rejectType, final int reason) {
        if (call == null) {
            return;
        }

        call.logi("rejectInternal :: reason=" + reason);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(rejectType);
        parcel.writeInt(reason);

        call.sendRequest(parcel);
    }

    private static boolean isVideoCapable(CallInfo callInfo) {
        return MtcCallInfo.isVideoCapable(callInfo);
    }

    private void log(String s) {
        ImsLog.d("[GII-MTC][" + getLogTag() + "] " + s);
    }

    private void loge(String s) {
        ImsLog.e("[GII-MTC][" + getLogTag() + "] " + s);
    }

    private void logi(String s) {
        ImsLog.i("[GII-MTC][" + getLogTag() + "] " + s);
    }

    /**
     * Handles the requests from the application, and the messages from the native layer.
     * All the JNI operations will be handled in a single thread("ImsCallHandlerThread").
     */
    private class MessageHandler extends Handler {
        public MessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_SEND_REQUEST: {
                    log("MtcCall :: sendRequest");

                    Parcel parcel = (Parcel)msg.obj;

                    mMtcJniProxy.sendDataToNative(getNativeCallId(), parcel);
                    break;
                }
                case MSG_CLOSE: {
                    MtcCall call = (MtcCall)msg.obj;

                    if (call != null) {
                        call.close();
                    }
                    break;
                }
                case MSG_CLEAR_INTERFACE: {
                    Long nativeCallId = (Long)msg.obj;

                    if (nativeCallId != null) {
                        logi("clearInterface=" + nativeCallId);
                        mMtcJniProxy.releaseJniInterfaceAndrRemoveListener(
                                nativeCallId.longValue(), mNativeListener);
                    }
                    break;
                }
                case MSG_MESSAGE_RECEIVED: {
                    Parcel parcel = (Parcel)msg.obj;

                    if (parcel == null) {
                        break;
                    }

                    mNativeListener.handleMessage(parcel);

                    parcel.recycle();
                    break;
                }
                default:
                    handleMessageForListener(msg);
                    break;
            }
        }

        @SuppressWarnings("unchecked")
        private void handleMessageForListener(Message msg) {
            Listener listener = mListener;

            log("handleMessageForListener :: msg=" + msg.what + ", listener=" + listener);

            if (listener == null) {
                return;
            }

            switch (msg.what) {
                case MSG_CALL_START_FAILED: {
                    listener.onCallStartFailed(MtcCall.this, (CallReasonInfo) msg.obj);
                    break;
                }
                case MSG_CALL_UPDATE_FAILED: {
                    listener.onCallUpdateFailed(MtcCall.this, (CallReasonInfo) msg.obj);
                    break;
                }
                case MSG_CALL_HOLD_FAILED: {
                    listener.onCallHoldFailed(MtcCall.this, (CallReasonInfo) msg.obj);
                    break;
                }
                case MSG_CALL_RESUME_FAILED: {
                    listener.onCallResumeFailed(MtcCall.this, (CallReasonInfo) msg.obj);
                    break;
                }
                case MSG_CALL_TERMINATED: {
                    listener.onCallTerminated(MtcCall.this, (CallReasonInfo) msg.obj);
                    break;
                }
                case MSG_AUDIO_SESSION_OPENED: {
                    listener.onAudioSessionOpened(MtcCall.this);
                    break;
                }
                case MSG_AUDIO_SESSION_CLOSED: {
                    listener.onAudioSessionClosed(MtcCall.this);
                    break;
                }
                case MSG_AUDIO_QUALITY_CHANGED: {
                    listener.onCallQualityChanged(MtcCall.this, (CallQuality) msg.obj);
                    break;
                }
                case MSG_AUDIO_RTP_EXTENSION_RECEIVED: {
                    listener.onCallRtpHeaderExtensionsReceived(
                            MtcCall.this, (Set<RtpHeaderExtension>) msg.obj);
                    break;
                }
                case MSG_AUDIO_TRIGGER_ANBR_QUERY_RECEIVED: {
                    listener.onTriggerAnbrQueryReceived(MtcCall.this, msg.arg1, msg.arg2,
                            (int) msg.obj);
                    break;
                }
                case MSG_AUDIO_INCOMING_DTMF_RECEIVED: {
                    listener.onNotifyIncomingDtmfReceived(MtcCall.this, (int) msg.obj);
                    break;
                }
                default:
                    // no-op
                    break;
            }
        }
    }

    private class JNIImsListenerProxy implements JniImsListener {
        @Override
        public void onMessage(Parcel parcel) {
            byte[] data = parcel.marshall();

            if (data == null) {
                return;
            }

            Parcel localParcel = Parcel.obtain();
            localParcel.unmarshall(data, 0, data.length);
            localParcel.setDataPosition(0);

            Message.obtain(mHandler, MSG_MESSAGE_RECEIVED, localParcel).sendToTarget();
        }

        public void handleMessage(Parcel parcel) {
            int msg = parcel.readInt();
            boolean isTerminatedEvent = false;

            // LogFilter compatibility: Mtc-MSG
            logi("MtcCall::Mtc-MSG=" + msg);

            // Set the flag when the call establishment is failed or call is terminated
            if ((msg == IUMtcCall.START_FAILED) || (msg == IUMtcCall.TERMINATED)) {
                log("Call is terminated - " + Long.toHexString(getNativeCallId()));
                isTerminatedEvent = true;
                setCallTerminated();
            }

            // Checks if the message is for media session or not
            if (checkAndHandleMediaMessage(msg, parcel)) {
                return;
            }

            // Checks if the listener is alive
            if ((mListener == null) && isTerminated()) {
                loge("Listener is null & terminated");
                handleStartFailedOrTerminated(msg, parcel);
                return;
            } else if (!isTerminatedEvent && isTerminated()) {
                // Don't handle any events from now.
                log("Call is already terminated");
            }

            switch (msg) {
                case IUMtcCall.INITIATING: {
                    CallInfo callInfo = new CallInfo(parcel);
                    MediaInfo mediaInfo = new MediaInfo(parcel);
                    int ratType = parcel.readInt();

                    onInitiating(callInfo, mediaInfo, ratType);
                    break;
                }
                case IUMtcCall.PROGRESSING: {
                    CallInfo callInfo = new CallInfo(parcel);
                    MediaInfo mediaInfo = new MediaInfo(parcel);
                    SuppInfo suppInfo = new SuppInfo(parcel);

                    onProgressing(callInfo, mediaInfo, suppInfo);
                    break;
                }
                case IUMtcCall.STARTED: {
                    CallInfo callInfo = new CallInfo(parcel);
                    MediaInfo mediaInfo = new MediaInfo(parcel);
                    SuppInfo suppInfo = new SuppInfo(parcel);

                    onStarted(callInfo, mediaInfo, suppInfo);
                    break;
                }
                case IUMtcCall.START_FAILED: {
                    CallReasonInfo callReasonInfo = new CallReasonInfo(parcel);

                    onStartFailed(callReasonInfo);
                    break;
                }
                case IUMtcCall.TERMINATED: {
                    CallReasonInfo callReasonInfo = new CallReasonInfo(parcel);

                    onTerminated(callReasonInfo);
                    break;
                }
                case IUMtcCall.HELD: {
                    CallInfo callInfo = new CallInfo(parcel);
                    MediaInfo mediaInfo = new MediaInfo(parcel);
                    SuppInfo suppInfo = new SuppInfo(parcel);

                    onHeld(callInfo, mediaInfo, suppInfo);
                    break;
                }
                case IUMtcCall.HOLD_FAILED: {
                    CallReasonInfo callReasonInfo = new CallReasonInfo(parcel);

                    onHoldFailed(callReasonInfo);
                    break;
                }
                case IUMtcCall.HELD_BY: {
                    CallInfo callInfo = new CallInfo(parcel);
                    MediaInfo mediaInfo = new MediaInfo(parcel);
                    SuppInfo suppInfo = new SuppInfo(parcel);

                    onHeldBy(callInfo, mediaInfo, suppInfo);
                    break;
                }
                case IUMtcCall.RESUMED: {
                    CallInfo callInfo = new CallInfo(parcel);
                    MediaInfo mediaInfo = new MediaInfo(parcel);
                    SuppInfo suppInfo = new SuppInfo(parcel);

                    onResumed(callInfo, mediaInfo, suppInfo);
                    break;
                }
                case IUMtcCall.RESUME_FAILED: {
                    CallReasonInfo callReasonInfo = new CallReasonInfo(parcel);

                    onResumeFailed(callReasonInfo);
                    break;
                }
                case IUMtcCall.RESUMED_BY: {
                    CallInfo callInfo = new CallInfo(parcel);
                    MediaInfo mediaInfo = new MediaInfo(parcel);
                    SuppInfo suppInfo = new SuppInfo(parcel);

                    onResumedBy(callInfo, mediaInfo, suppInfo);
                    break;
                }
                case IUMtcCall.UPDATED_BY: {
                    CallInfo callInfo = new CallInfo(parcel);
                    MediaInfo mediaInfo = new MediaInfo(parcel);
                    SuppInfo suppInfo = new SuppInfo(parcel);

                    onUpdatedBy(callInfo, mediaInfo, suppInfo);
                    break;
                }
                case IUMtcCall.UPDATED: {
                    CallInfo callInfo = new CallInfo(parcel);
                    MediaInfo mediaInfo = new MediaInfo(parcel);
                    SuppInfo suppInfo = new SuppInfo(parcel);

                    onUpdated(callInfo, mediaInfo, suppInfo);
                    break;
                }
                case IUMtcCall.UPDATE_FAILED: {
                    CallReasonInfo callReasonInfo = new CallReasonInfo(parcel);

                    onUpdateFailed(callReasonInfo);
                    break;
                }
                case IUMtcCall.INCOMING_UPDATE: {
                    CallInfo callInfo = new CallInfo(parcel);
                    MediaInfo mediaInfo = new MediaInfo(parcel);
                    SuppInfo suppInfo = new SuppInfo(parcel);

                    onIncomingUpdate(callInfo, mediaInfo, suppInfo);
                    break;
                }
                case IUMtcCall.INCOMING_RESUME: {
                    CallInfo callInfo = new CallInfo(parcel);
                    MediaInfo mediaInfo = new MediaInfo(parcel);
                    SuppInfo suppInfo = new SuppInfo(parcel);

                    onIncomingResume(callInfo, mediaInfo, suppInfo);
                    break;
                }
                case IUMtcCall.INCOMING_CALL_RECEIVED: {
                    IncomingMtcCall incomingCall = new IncomingMtcCall(parcel);
                    int ratType = parcel.readInt();
                    onIncomingCallReceived(incomingCall, ratType);
                    break;
                }
                case IUMtcCall.NETWORK_CHANGED: {
                    onRatChanged(parcel.readInt());
                    break;
                }
                case IUMtcCall.ECT_COMPLETED: {
                    int result = parcel.readInt();
                    CallReasonInfo callReasonInfo = (result == 0)
                            ? new CallReasonInfo(parcel) : null;

                    onEctCompleted(result, callReasonInfo);
                    break;
                }
                case IUMtcCall.REPLACED_BY: {
                    CallInfo callInfo = new CallInfo(parcel);
                    MediaInfo mediaInfo = new MediaInfo(parcel);
                    SuppInfo suppInfo = new SuppInfo(parcel);
                    long newNativeCallId = parcel.readLong();
                    int typeForReplacedBy = parcel.readInt();

                    onReplacedBy(callInfo, mediaInfo, suppInfo, newNativeCallId, typeForReplacedBy);
                    break;
                }
                case IUMtcCall.CALL_PUSH_COMPLETED: {
                    int result = parcel.readInt();
                    CallReasonInfo callReasonInfo = (result == 0)
                            ? new CallReasonInfo(parcel) : null;

                    onCallPushCompleted(result, callReasonInfo);
                    break;
                }
                case IUMtcCall.NOTIFY_INFO: {
                    int type = parcel.readInt();
                    String strValue = parcel.readString();
                    int intValue = parcel.readInt();
                    boolean boolValue = (parcel.readInt() == 1);

                    onNotifyInfo(type, strValue, intValue, boolValue);
                    break;
                }
                default:
                    if (MtcConference.isMessageForConference(msg)) {
                        try {
                            mConference.handleMessage(msg, parcel);
                        } catch (Throwable t) {
                            loge("MtcConference#handleMessage :: " + t.toString());
                            t.printStackTrace();
                        }
                    }
                    break;
            }
        }

        private void handleStartFailedOrTerminated(int msg, Parcel parcel) {
            if ((msg == IUMtcCall.START_FAILED) || (msg == IUMtcCall.TERMINATED)) {
                CallReasonInfo callReasonInfo = new CallReasonInfo(parcel);

                if (msg == IUMtcCall.START_FAILED) {
                    if (MtcCallUtils.isCallTerminatedByCSRetry(callReasonInfo.mCode)) {
                        setDetails(Details.TERMINATED_BY_CS_RETRY, true);
                    }
                }

                // To handle the glare condition or wait for the media resource release
                mTerminationReason = new CallReasonInfo(callReasonInfo);
            }

            setCallState(CallTracker.CALL_STATE_IDLE);

            mCT.updateCallState(MtcCall.this, CallTracker.CALL_EVENT_TERMINATED, null);

            // In this case, the application doesn't receive any notification,
            // so MtcCall needs to be closed in here.
            closeInternal(MtcCall.this);
        }

        private void onInitiating(CallInfo callInfo, MediaInfo mediaInfo, int ratType) {
            logi("INITIATING");

            if (mListener != null) {
                mListener.onCallInitiating(MtcCall.this, callInfo, mediaInfo, ratType);
            }
        }

        private void onProgressing(CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            logi("PROGRESSING :: " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));

            // Update the call/media info.
            updateCallParameters(callInfo, mediaInfo, suppInfo);

            setCallState(CallTracker.CALL_STATE_RINGBACK);

            if (mListener != null) {
                mListener.onCallProgressing(
                        MtcCall.this, callInfo, mediaInfo, suppInfo);
            }
        }

        private void onStarted(CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            logi("STARTED :: " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));

            setCallState(CallTracker.CALL_STATE_OFFHOOK);

            // Update the call/media info.
            updateCallParameters(callInfo, mediaInfo, suppInfo);
            setCallExtraBoolean(EXTRA_CONFERENCE_AVAIL,
                    MtcCallInfo.isConferenceAvailable(callInfo));

            checkAndSetCallType(MtcCallInfo.getCallType(callInfo));

            // ConferenceInfo: to manage the participants in the conference call
            if (isConference()) {
                ConferenceInfoHelper.createConferenceInfo(getCallId());
            }

            setVideoState(mediaInfo);

            mCT.updateCallState(MtcCall.this, CallTracker.CALL_EVENT_ESTABLISHED, null);

            if (mListener != null) {
                mListener.onCallStarted(
                        MtcCall.this, callInfo, mediaInfo, suppInfo);
            }

            setHoldStateOnCallStarted(mediaInfo);

            if (DBG) {
                log("onStarted :: " + MtcCall.this);
            }
        }

        private void onStartFailed(CallReasonInfo callReasonInfo) {
            logi("START_FAILED :: " + MtcCallUtils.toString(callReasonInfo));

            if (isCallFailedByAlreadyOpenedServiceClosed(callReasonInfo)) {
                if (mEmergencyCallFailureListener != null && mEmergencyCallFailureListener
                        .onEmergencyCallFailedByAlreadyOpenedServiceClosed()) {
                    mUsingAlreadyOpenedEmergencyService = false;
                    Message.obtain(mHandler, MSG_CLEAR_INTERFACE,
                            Long.valueOf(getNativeCallId())).sendToTarget();
                    updateNativeCallObject(0);
                    return;
                }
            }

            callReasonInfo = updateCallReasonInfoOnInvalidCallKey(callReasonInfo);

            setCallState(CallTracker.CALL_STATE_IDLE);

            if (MtcCallUtils.isCallTerminatedByCSRetry(callReasonInfo.mCode)) {
                setDetails(Details.TERMINATED_BY_CS_RETRY, true);
            }

            // To handle the glare condition or wait for the media resource release
            mTerminationReason = new CallReasonInfo(callReasonInfo);

            if (!isOnPreIncoming()) {
                mCT.updateCallState(MtcCall.this, CallTracker.CALL_EVENT_TERMINATED, null);
            }

            if (mListener != null) {
                mListener.onCallStartFailed(MtcCall.this, callReasonInfo);
            }
        }

        private void onTerminated(CallReasonInfo callReasonInfo) {
            logi("TERMINATED :: " + MtcCallUtils.toString(callReasonInfo));

            clearHoldState();
            setCallState(CallTracker.CALL_STATE_IDLE);

            // To handle the glare condition or wait for the media resource release
            mTerminationReason = new CallReasonInfo(callReasonInfo);

            mCT.updateCallState(MtcCall.this, CallTracker.CALL_EVENT_TERMINATED, null);

            // SYNC_WITH_MEDIA_CLOSE
            if (mListener != null) {
                mListener.onCallTerminated(MtcCall.this, callReasonInfo);
            }
        }

        private void onHeld(CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            logi("HELD :: " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));

            setOnHold(true);

            // Update the call/media info.
            updateCallParameters(callInfo, mediaInfo, suppInfo);

            mCT.updateCallState(MtcCall.this, CallTracker.CALL_EVENT_UPDATED, null);

            if (mListener != null) {
                mListener.onCallHeld(
                        MtcCall.this, callInfo, mediaInfo, suppInfo);
            }
        }

        private void onHoldFailed(CallReasonInfo callReasonInfo) {
            logi("HOLD_FAILED :: " + MtcCallUtils.toString(callReasonInfo));

            if (mListener != null) {
                mListener.onCallHoldFailed(MtcCall.this, callReasonInfo);
            }
        }

        private void onHeldBy(CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            logi("HELD_BY :: " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));

            setOnHeld(true);

            // Update the call/media info.
            updateCallParameters(callInfo, mediaInfo, suppInfo);

            mCT.updateCallState(MtcCall.this, CallTracker.CALL_EVENT_UPDATED, null);

            if (mListener != null) {
                mListener.onCallHoldReceived(
                        MtcCall.this, callInfo, mediaInfo, suppInfo);
            }
        }

        private void onResumed(CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            logi("RESUMED :: " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));

            // Update the call/media info.
            updateCallParameters(callInfo, mediaInfo, suppInfo);

            if (getUpdateState() == UPDATE_STATE_RESUME_ACCEPTED) {
                setOnHeld(false);
            } else {
                setOnHold(false);
            }

            setUpdateState(UPDATE_STATE_IDLE);

            mCT.updateCallState(MtcCall.this, CallTracker.CALL_EVENT_UPDATED, null);

            if (mListener != null) {
                mListener.onCallResumed(
                        MtcCall.this, callInfo, mediaInfo, suppInfo);
            }
        }

        private void onResumeFailed(CallReasonInfo callReasonInfo) {
            logi("RESUME_FAILED :: " + MtcCallUtils.toString(callReasonInfo));

            if (mListener != null) {
                mListener.onCallResumeFailed(MtcCall.this, callReasonInfo);
            }
        }

        private void onResumedBy(CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            logi("RESUMED_BY :: " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));

            setOnHeld(false);

            // Update the call/media info.
            updateCallParameters(callInfo, mediaInfo, suppInfo);

            mCT.updateCallState(MtcCall.this, CallTracker.CALL_EVENT_UPDATED, null);

            if (mListener != null) {
                mListener.onCallResumeReceived(
                        MtcCall.this, callInfo, mediaInfo, suppInfo);
            }
        }

        private void onUpdatedBy(CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            logi("UPDATED_BY :: " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));

            // Update the call/media info.
            updateCallParameters(callInfo, mediaInfo, suppInfo);

            checkAndSetCallType(MtcCallInfo.getCallType(callInfo));

            logi("HoldState :: isOnHold=" + isOnHold() + ", isOnHeld=" + isOnHeld());

            mCT.updateCallState(MtcCall.this, CallTracker.CALL_EVENT_UPDATED, null);

            if (mListener != null) {
                mListener.onCallAutoUpdated(
                        MtcCall.this, callInfo, mediaInfo, suppInfo);
            }

            if (DBG) {
                log("onUpdatedBy :: " + MtcCall.this);
            }
        }

        private void onUpdated(CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            logi("UPDATED :: " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));

            // Update the call/media info.
            updateCallParameters(callInfo, mediaInfo, suppInfo);
            setCallExtraBoolean(EXTRA_CONFERENCE_AVAIL,
                    MtcCallInfo.isConferenceAvailable(callInfo));

            int callType = MtcCallInfo.getCallType(callInfo);

            if (!checkAndSetCallType(callType)) {
                if ((getUpdateState() == UPDATE_STATE_ACCEPTED)
                        && (mOldCallTypeOnUpdateAccept != callType)) {
                    // Call type is changed by the remote end.

                    // Recover HOLD and/or HELD state based on the current media info.
                    if (!MtcCallUtils.hasVideo(callType)) {
                        if (!isOnHold()
                                && MtcCallUtils.isHoldMediaOnVoiceCall(mediaInfo)) {
                            log("Voice call is on HOLD");
                            setOnHold(true);
                        }

                        if (!isOnHeld()
                                && MtcCallUtils.isHoldMediaOnVoiceCallByRemoteEnd(mediaInfo)) {
                            log("Voice call is on HELD");
                            setOnHeld(true);
                        }
                    }
                } else {
                    setVideoHoldState(mediaInfo);
                }
            }

            setVideoState(mediaInfo);

            setUpdateState(UPDATE_STATE_IDLE);

            mCT.updateCallState(MtcCall.this, CallTracker.CALL_EVENT_UPDATED, null);

            if (mListener != null) {
                mListener.onCallUpdated(
                        MtcCall.this, callInfo, mediaInfo, suppInfo);
            }

            if (DBG) {
                log("onUpdated :: " + MtcCall.this);
            }
        }

        private void onUpdateFailed(CallReasonInfo callReasonInfo) {
            logi("UPDATE_FAILED :: " + MtcCallUtils.toString(callReasonInfo));

            setUpdateState(UPDATE_STATE_IDLE);

            if (mListener != null) {
                mListener.onCallUpdateFailed(MtcCall.this, callReasonInfo);
            }
        }

        private void onIncomingUpdate(CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            logi("INCOMING_UPDATE :: updateState=" + updateStateToString(getUpdateState())
                    + ", " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));

            if (getUpdateState() == UPDATE_STATE_SENT) {
                rejectInternal(MtcCall.this, IUMtcCall.REJECT_UPDATE,
                        CallReasonInfo.CODE_REJECT_ONGOING_CALL_UPGRADE);
                return;
            }

            setUpdateState(UPDATE_STATE_RECEIVED);

            if (mListener != null) {
                mListener.onCallUpdateReceived(
                        MtcCall.this, callInfo, mediaInfo, suppInfo);
            }
        }

        private void onIncomingResume(CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            logi("INCOMING_RESUME :: updateState=" + updateStateToString(getUpdateState())
                    + ", " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));

            setUpdateState(UPDATE_STATE_RESUME_RECEIVED);

            if (mListener != null) {
                mListener.onCallUpdateResumeReceived(
                        MtcCall.this, callInfo, mediaInfo, suppInfo);
            }
        }

        private void onIncomingCallReceived(IncomingMtcCall incomingCall, int ratType) {
            logi("INCOMING_CALL_RECEIVED");
            setDetails(Details.ON_PRE_INCOMING, false);
            setRemoteNumber(incomingCall.callerPartyNum);
            updateCallExtras(incomingCall);

            mCT.updateCallState(
                    MtcCall.this, CallTracker.CALL_EVENT_INCOMING_RECEIVED, null);

            if (mListener != null) {
                mListener.onCallIncomingReceived(MtcCall.this, incomingCall, ratType);
            }
        }

        private void onRatChanged(int ratType) {
            if (mListener != null) {
                mListener.onCallRatChanged(ratType);
            }
        }

        private void onEctCompleted(int result, CallReasonInfo callReasonInfo) {
            logi("ECT_COMPLETED :: result=" + result);

            if (mListener == null) {
                return;
            }

            if (result != 0) {
                mListener.onCallTransferred(MtcCall.this);
            } else {
                mListener.onCallTransferFailed(MtcCall.this, callReasonInfo);
            }
        }

        private void onReplacedBy(CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo,
                long newNativeCallId, int typeForReplacedBy) {
            logi("REPLACED_BY :: newCallId=" + Long.toHexString(newNativeCallId)
                    + ", " + MtcCallUtils.toString(callInfo)
                    + ", " + MtcCallUtils.toString(mediaInfo)
                    + ", " + MtcCallUtils.toString(suppInfo));

            if (mListener == null) {
                // FIXME: close Mtc call if present
                return;
            }

            MtcCall newCall = null;

            if (newNativeCallId == 0) {
                updateCallParameters(callInfo, mediaInfo, suppInfo);
            } else {
                newCall = new MtcCall(mContext, mCT, 0, -1, "RB");

                newCall.updateNativeCallObject(newNativeCallId);
                newCall.setCallType(MtcCallInfo.getCallType(callInfo));
                newCall.setCallState(CallTracker.CALL_STATE_OFFHOOK);
                // FIXME: Is it a correct information for the new call?
                newCall.updateCallParameters(callInfo, mediaInfo, suppInfo);
                mCT.updateCallState(newCall, CallTracker.CALL_EVENT_CREATE, null);
            }

            if (typeForReplacedBy == IUMtcCall.REPLACED_BY_TYPE_ECT) {
                mListener.onCallTransferReceived(MtcCall.this, newCall, callInfo, mediaInfo,
                        suppInfo);
            } else {
                // FIXME: close new Mtc call ?
            }
        }

        private void onCallPushCompleted(int result, CallReasonInfo callReasonInfo) {
            logi("CALL_PUSH_COMPLETED :: result=" + result);

            if (mListener == null) {
                return;
            }

            if (result != 0) {
                mListener.onCallPushRequestCompleted(MtcCall.this);
            } else {
                mListener.onCallPushRequestFailed(MtcCall.this, callReasonInfo);
            }
        }

        private void onNotifyInfo(int type, String strValue, int intValue, boolean boolValue) {
            logi("NOTIFY_INFO :: type=" + type + ", strV=" + strValue
                    + ", intV=" + intValue + ", boolV=" + boolValue);

            if (MtcCallUtils.isInfoTypeForMediaSession(type)) {
                mMediaSession.notifyMediaInfoChanged(type, intValue, strValue);
            } else {
                if (mListener != null) {
                    mListener.onCallInfoUpdated(
                            MtcCall.this, type, strValue, intValue, boolValue);
                }
            }
        }
    }

    // MTC_CONFERENCE_WRAPPER {
    public static MtcConference getConference(MtcCall s) {
        return (s != null) ? s.getConferenceInterface() : null;
    }

    public static void extendToConference(MtcCall s, UsersInfo users) {
        MtcConference c = getConference(s);

        if (c != null) {
            c.extendToConference(users);
        }
    }

    public static void merge(MtcCall s, UsersInfo users) {
        MtcConference c = getConference(s);

        if (c != null) {
            c.merge(users);
        }
    }

    public static void inviteParticipants(MtcCall s, UsersInfo users) {
        MtcConference c = getConference(s);

        if (c != null) {
            c.inviteParticipants(users);
        }
    }

    public static void removeParticipants(MtcCall s, UsersInfo users) {
        MtcConference c = getConference(s);

        if (c != null) {
            c.removeParticipants(users);
        }
    }

    public static void deleteParticipants(MtcCall s, UsersInfo users) {
        MtcConference c = getConference(s);

        if (c != null) {
            c.deleteParticipants(users);
        }
    }

    public static void setListener(MtcCall s, MtcConference.Listener listener) {
        MtcConference c = getConference(s);

        if (c != null) {
            c.setListener(listener);
        }
    }
    // MTC_CONFERENCE_WRAPPER }

    static {
        SuppServiceUtils.addKey(SuppInfo.SUPP_TYPE_CDIV_CAUSE, EXTRA_CDIV_CAUSE);
        SuppServiceUtils.addValueType(SuppInfo.SUPP_TYPE_CDIV_CAUSE, SuppServiceUtils.TYPE_INT);

        SuppServiceUtils.addKey(SuppInfo.SUPP_TYPE_CNAP, EXTRA_CNA);
        SuppServiceUtils.addValueType(SuppInfo.SUPP_TYPE_CNAP, SuppServiceUtils.TYPE_STRING);
        SuppServiceUtils.addKey(SuppInfo.SUPP_TYPE_CDIV_HISTORY, EXTRA_CDIV_HISTORY);
        SuppServiceUtils.addValueType(
                SuppInfo.SUPP_TYPE_CDIV_HISTORY, SuppServiceUtils.TYPE_STRING);

        // The below things are not managed for extra call information
        SuppServiceUtils.addValueType(SuppInfo.SUPP_TYPE_CW, SuppServiceUtils.TYPE_BOOLEAN);
        SuppServiceUtils.addValueType(SuppInfo.SUPP_TYPE_ENFORCE_LT, SuppServiceUtils.TYPE_BOOLEAN);

        SuppServiceUtils.addValueType(SuppInfo.SUPP_TYPE_CALLERID, SuppServiceUtils.TYPE_INT);
        SuppServiceUtils.addValueType(
                SuppInfo.SUPP_TYPE_CALLING_NUM_VERIFICATION, SuppServiceUtils.TYPE_INT);
    }
}
