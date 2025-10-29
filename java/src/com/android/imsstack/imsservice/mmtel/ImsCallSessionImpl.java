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

import android.annotation.NonNull;
import android.location.Location;
import android.os.Handler;
import android.os.Message;
import android.os.Parcel;
import android.os.RemoteException;
import android.os.SystemClock;
import android.telecom.Connection.RttModifyStatus;
import android.telephony.CallQuality;
import android.telephony.PreciseCallState;
import android.telephony.TelephonyManager;
import android.telephony.emergency.EmergencyNumber;
import android.telephony.emergency.EmergencyNumber.EmergencyCallRouting;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsCallSessionListener;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsVideoCallProvider;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.telephony.imscallext.ImsCallExtManager;
import android.text.TextUtils;

import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.core.agents.Usat;
import com.android.imsstack.core.agents.UsatInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.enabler.mtc.Call;
import com.android.imsstack.enabler.mtc.CallFeature;
import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.CallTracker;
import com.android.imsstack.enabler.mtc.ConferenceInfo;
import com.android.imsstack.enabler.mtc.ConferenceInfoHelper;
import com.android.imsstack.enabler.mtc.IServiceStateTracker;
import com.android.imsstack.enabler.mtc.IUMtcCall;
import com.android.imsstack.enabler.mtc.IUMtcService;
import com.android.imsstack.enabler.mtc.IncomingMtcCall;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.enabler.mtc.MtcCallInfo;
import com.android.imsstack.enabler.mtc.MtcCallUtils;
import com.android.imsstack.enabler.mtc.MtcConference;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.enabler.mtc.SuppServiceUtils.SuppService;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.enabler.mtc.reg.MtcServiceState;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.base.ICallLocationPolicy;
import com.android.imsstack.imsservice.mmtel.base.TtyModeTracker;
import com.android.imsstack.imsservice.mmtel.call.IVideoCallSession;
import com.android.imsstack.imsservice.mmtel.internal.ConferenceProxy;
import com.android.imsstack.imsservice.mmtel.videocall.ImsVideoCallProviderFactory;
import com.android.imsstack.imsservice.mmtel.videocall.base.ImsVideoCallProviderBase;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class ImsCallSessionImpl extends ImsCallSessionImplBase {
    private static final boolean DBG = ImsLog.isDebuggable();
    private static final boolean FEATURE_CHANGE_CONFERENCE_PARTICIPANT_STATE_ON_DROP = false;
    private static final String MEDIA_GTT_MODE = "media_gtt_mode";
    private static final String PREFIX_MMI_PARTICIPANT_DROP = "callId:";
    private static final int TRANSFER_RESUME_RECOVERY_DELAY_TIME = 2000;

    /**
     * Extra key to identify that the session modification is handled by ImsCallSessionImpl.
     * Value: boolean type
     */
    private static final String EXTRA_CALL_CONTROLLED_BY_IMS = "call_controlled_by_ims";
    public static final int CF_TTY = 0;
    public static final int CF_AUDIO_HOLD_WITH_INACTIVE = 1;
    public static final int CF_VIDEO_HOLD_WITH_INACTIVE = 2;
    public static final int CF_TEXT_HOLD_WITH_INACTIVE = 3;
    public static final int CF_INCOMING_RESUME_EVENT = 4;
    public static final int CF_ONE_WAY_VIDEO_LOCAL = 5;
    public static final int CF_ONE_WAY_VIDEO_REMOTE = 6;
    public static final int CF_CONF_USER_ANONYMOUS = 7;

    private final Object mLock = new Object();
    private final ICallContext mCallContext;
    private final MtcCallListenerProxy mListenerProxy = new MtcCallListenerProxy();
    private final EmergencyCallFailureListener mEmergencyCallFailureListener =
            new EmergencyCallFailureListener();
    private final MtcConferenceListenerProxy mConferenceListenerProxy
            = new MtcConferenceListenerProxy();
    private final CallTracker mCT;
    private final String mCallId;
    private final CallDetails mCallDetails = new CallDetails();
    private ImsCallSessionCallback mCallback;
    private MtcCall mCall;
    private int mState = ImsCallSessionImplBase.State.IDLE;
    // Local call profile (local capabilities, obtained at call originating or terminating)
    private ImsCallProfile mLocalCallProfile = null;
    private ImsCallProfile mRemoteCallProfile = null;
    // Negotiated call profile (local capa. + remote capa.)
    private ImsCallProfile mCallProfile = null;
    private ImsCallProfile mProposedCallProfile = new ImsCallProfile();
    protected ImsReasonInfo mImmediateCallEndReason = null;
    private ImsReasonInfo mOperationFailReason = null;
    private int mTerminationReason = ImsReasonInfo.CODE_UNSPECIFIED;
    private ConferenceProxy mConferenceProxy = null;
    private MoPendingCall mMoPendingCall = null;
    private TtyModeListenerProxy mTtyModeListener = null;
    private MtcServiceStateListener mServiceStateListener = null;
    private final ImsVideoCallSession mVideoCallSession;
    private final ImsVideoCallProviderBase mVideoCallProvider;
    // WFC w/ geolocation {
    private LocationBasedCall mLocationBasedCall = null;
    // WFC w/ geolocation }
    private UsatBasedCall mUsatBasedCall = null;
    private String mTransferTargetNumber = null;
    protected boolean mIsEctConfirmationRequired = false;
    protected ImsCallSessionImpl mTransferRequestedSession = null;
    protected ImsCallSessionImpl mTransferTargetSession = null;
    private Map<Integer, Boolean> mCallFeatureCache = new HashMap<Integer, Boolean>();
    private CallReasonInfo mCacheCallReasonInfo = null;
    private boolean mIsE2eeCallInfoNotified = false;
    private ImsCallExtManagerProxy mCallExtManagerProxy = new ImsCallExtManagerProxy() {
        @Override
        public void reportCallInfo(int slotId, String imsCallSessionId, byte[] localSessionId,
                byte[] remoteSessionId, String remotePhoneNumber) {
            try {
                logi("reportCallInfo slotId : " + slotId);
                ImsCallExtManager.getInstance(mCallContext.getContext()).reportCallInfo(slotId,
                        imsCallSessionId, localSessionId, remoteSessionId, remotePhoneNumber);
            } catch (Exception e) {
                log("can't notify E2EE");
            }
        }
    };

    public interface ImsCallExtManagerProxy {
        /**
         * The ImsService needs to notify E2EE module with related call information for
         * RTP encryption.
         */
        void reportCallInfo(int slotId, String imsCallSessionId, byte[] localSessionId,
                byte[] remoteSessionId, String remotePhoneNumber);
    }

    public ImsCallSessionImpl(ICallContext callContext,
            CallTracker ct, MtcCall call,
            String callId, ImsCallProfile profile, boolean isMO) {
        this(callContext, ct, call, callId, profile, isMO,
                new ImsCallSessionCallback(callContext.getExecutor()), null);
    }

    @VisibleForTesting
    public ImsCallSessionImpl(ICallContext callContext, CallTracker ct, MtcCall call,
            String callId, ImsCallProfile profile, boolean isMO, ImsCallSessionCallback callBack,
            ImsVideoCallSession videoCallSession) {

        mCallContext = callContext;

        mCT = ct;
        mCall = call;
        mCallId = createCallId(callId);
        mCallback = callBack;

        initCallProfile(profile);

        if (call != null) {
            call.setListener(mListenerProxy);
            MtcCall.setListener(call, mConferenceListenerProxy);
            call.setEmergencyCallFailureListener(mEmergencyCallFailureListener);
        }

        mVideoCallSession = videoCallSession == null
                ? new ImsVideoCallSession(mCallContext, this, isMO) : videoCallSession;
        mVideoCallProvider = ImsVideoCallProviderFactory.createVideoCallProvider(
                mVideoCallSession,
                (call != null) ? call.getMediaSession() : null);

        if (isMO) {
            mCallDetails.set(CallDetails.MO);

            if (mCallProfile.getServiceType() != ImsCallProfile.SERVICE_TYPE_NORMAL) {
                mMoPendingCall = new MoPendingCall(mCallProfile.getServiceType());
            }
        }

        clearProposedCallProfile();

        // TTY_MODE
        TtyModeTracker tmt = mCallContext.getTtyModeTracker();

        if (tmt != null) {
            mTtyModeListener = new TtyModeListenerProxy();
            tmt.addListener(mTtyModeListener);
        }
    }

    @VisibleForTesting
    MtcCall.Listener getMtcCallListenerProxy() {
        return mListenerProxy;
    }

    @Override
    public void close() {
        ImsGarbageCalls garbageCalls = ImsGarbageCalls.getInstance();

        clearConferenceProxy();
        clearPendingCall();
        clearLocationBasedCall();
        clearUsatBasedCall();

        logi("close");

        if (mCall != null) {
            if (mCallDetails.is(CallDetails.IMPLICIT_TERMINATED)
                    && isImplicitTerminatedCondition()) {
                // Call is closed when onCallTerminated(...) is invoked.
                logi("Close is pending");

                mCall.detach();
                mCallDetails.set(CallDetails.CLOSE_PENDING);
                clearSpecificFeatures();

                garbageCalls.add(mCallContext.getSlotId(), this);

                mCT.updateCallState(this, CallTracker.CALL_EVENT_DESTROY, null);
                return;
            }

            /* Don't close the {@link #MtcCall} before {@link onAudioSessionClosed} is called
             */
            if (mCallDetails.is(CallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END)) {
                logi("Don't close MtcCall before audio session closed");
                mCallDetails.set(CallDetails.CLOSE_PENDING);
                return;
            }

            // Terminate the call if the call is not in TERMINATED state.
            int state = getState();

            if (!mCallDetails.is(CallDetails.MO)
                    && (state < ImsCallSessionImplBase.State.ESTABLISHED)) {
                mCall.reject(CallReasonInfo.CODE_UNSPECIFIED);
            } else if (state < ImsCallSessionImplBase.State.NEGOTIATING) {
                // To avoid timing issue
                SystemClock.sleep(10);

                state = getState();

                if ((state > ImsCallSessionImplBase.State.INITIATED)
                        && (state < ImsCallSessionImplBase.State.TERMINATING)) {
                    mCall.terminate(CallReasonInfo.CODE_UNSPECIFIED);
                }
            } else if (state < ImsCallSessionImplBase.State.TERMINATING) {
                mCall.terminate(CallReasonInfo.CODE_UNSPECIFIED);
            }

            // CALL_CONNECTION_ID
            if (isMultiparty()) {
                removeCallConnectionId();
            } else if (!mCallDetails.is(CallDetails.MERGED)) {
                ImsCallConnectionIds.remove(mCallContext.getSlotId(), getCallConnectionId());
            }

            mCall.setListener(null);
            mCall.setEmergencyCallFailureListener(null);
            mCall.close();
            mCall = null;
        }

        setState(ImsCallSessionImplBase.State.TERMINATED);

        // Usually, it's to close the foreground session when this session is a conference.
        mConferenceListenerProxy.closeSession();
        mVideoCallProvider.close();

        clearSpecificFeatures();

        if (garbageCalls.contains(this)) {
            garbageCalls.remove(this);
        } else {
            mCT.updateCallState(this, CallTracker.CALL_EVENT_DESTROY, null);
        }

        log("close callId=" + getCallId() + " host=" + isMultiparty() +
                " participant=" + mCallDetails.is(CallDetails.MERGED));

        mCallback.setListener(null);
    }

    @Override
    public String getCallId() {
        return mCallId;
    }

    @Override
    public ImsCallProfile getCallProfile() {
        return mCallProfile;
    }

    @Override
    public ImsCallProfile getLocalCallProfile() {
        return mLocalCallProfile;
    }

    @Override
    public ImsCallProfile getRemoteCallProfile() {
        return mRemoteCallProfile;
    }

    @Override
    public String getProperty(String name) {
        if (name == null) {
            return null;
        }

        if (mCall == null) {
            return null;
        }

        // Order: Boolean/Int/String properties
        if (Call.isCallExtraBoolean(name)) {
            return String.valueOf(mCall.getCallExtraBoolean(name, false));
        } else if (Call.isCallExtraInt(name)) {
            return String.valueOf(mCall.getCallExtraInt(name, -1));
        } else {
            return mCall.getCallExtra(name, null);
        }
    }

    @Override
    public int getState() {
        return mState;
    }

    @Override
    public boolean isInCall() {
        return (mCall != null) ? mCall.isInCall() : false;
    }

    @Override
    public void setListener(ImsCallSessionListener listener) {
        log("setListener : " + listener);

        synchronized (mLock) {
            mCallback.setListener(listener);
        }

        // Notify the supplementary service for call forwarding if present.
        if ((listener != null)
                && !mCallDetails.is(CallDetails.MO)
                && (getState() == ImsCallSessionImplBase.State.NEGOTIATING)) {
            notifySuppServiceForForwardedCall(false);
        }
    }

    @Override
    public void setMute(boolean muted) {
        // no-op
    }

    @Override
    public void start(String callee, ImsCallProfile profile) {
        if (mCall == null) {
            // EXCEPTION_HANDLING: Call UI stuck
            int code = CallReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE;
            int extraCode = 0;

            if (mCallProfile.getCallExtraBoolean(ImsCallProfile.EXTRA_EMERGENCY_CALL, false)
                    || (mCallProfile.getServiceType() == ImsCallProfile.SERVICE_TYPE_EMERGENCY)) {
                code = CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
                extraCode = CallReasonInfo.EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;
            }

            loge("start :: No native session - code=" + code + ", extraCode=" + extraCode);

            notifyCallStartFailed(ImsCallUtils.createReasonInfo(
                    code, extraCode, "No native session", ImsCallUtils.FLAG_REASON_INFO_ALL));
            return;
        }

        ImsCallApp callApp = (ImsCallApp) mCallContext.getApp();
        MtcApp mtcApp = callApp.getCallManager().getMtcApp();
        if (mtcApp.isOutgoingCallBarringActivated(ImsCallUtils.getCallTypeFromProfile(
                profile.getCallType(), profile.getMediaProfile().isRttCall()), callee)) {
            notifyCallStartFailed(ImsCallUtils.createReasonInfo(
                    CallReasonInfo.CODE_CALL_BARRED, 0, "", ImsCallUtils.FLAG_REASON_INFO_CODE));
            return;
        }

        mCallDetails.set(CallDetails.TELEPHONY_LISTENING);

        if (mCall.isEmergencyCall()) {

            @EmergencyCallRouting
            int emergencyRouting = ImsCallUtils.maybeUpdateEmergencyRouting(
                    mCallContext, ImsCallUtils.getEmergencyRoutingFromCallProfile(profile), callee,
                    mtcApp.getMtcEmergencyServiceManager().getNetworkCountryIso());
            mtcApp.openEmergencyService(mCall, emergencyRouting);
        }

        // Handles an emergency call as a normal call
        boolean emergencyCall = false;

        if (mCallProfile.getCallExtraBoolean(ImsCallProfile.EXTRA_EMERGENCY_CALL, false)) {
            emergencyCall = true;
            profile.setCallExtraBoolean(ImsCallProfile.EXTRA_EMERGENCY_CALL, true);
        }

        // Checks if the originating call is determined by USAT
        if ((mCallProfile.getServiceType() != ImsCallProfile.SERVICE_TYPE_EMERGENCY)
                && !emergencyCall
                && isUsatBasedCallRequired()) {
            mUsatBasedCall = new UsatBasedCall();
            mUsatBasedCall.start(callee);
            log("USAT based call is started");
            return;
        }

        // OFFLINE_DIALING or E_CALL
        if (startMoPendingCall(callee, profile)) {
            return;
        }

        // WFC w/ geolocation
        ICallLocationPolicy clp = mCallContext.getCallLocationPolicy();

        if ((clp != null) && clp.isLocationRequired(callee, profile)) {
            mLocationBasedCall = new LocationBasedCall();
            mLocationBasedCall.start(callee);
            log("Location based call :: waits for the location updates");
            return;
        }

        startInternal(callee, profile);
    }

    @Override
    public void startConference(String[] participants, ImsCallProfile profile) {
        // FLEXIBILITY :: only one participant, so it is handled as a normal call
        // 150402, This check routine will not be used for KDDI requirement.
        //if ((participants != null) && (participants.length == 1)) {
        //    start(participants[0], profile);
        //    return;
        //}
        if (mCall == null) {
            // EXCEPTION_HANDLING: Call UI stuck
            loge("startConference :: No native session");

            notifyCallStartFailed(ImsCallUtils.createReasonInfo(
                    CallReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE,
                    0, "No native session", ImsCallUtils.FLAG_REASON_INFO_ALL));
            return;
        }

        mCallDetails.set(CallDetails.TELEPHONY_LISTENING);

        mCall.startConference(ImsCallUtils.getCallTypeFromProfile(
                profile.getCallType(), profile.getMediaProfile().isRttCall()),
                MtcCallUtils.createUsersInfo(participants),
                ImsCallMediaUtils.createMediaInfoFromMediaProfile(profile.getMediaProfile()),
                ImsCallUtils.createSuppInfoFromCallProfile(mCallContext, profile, null, ""));

        setState(ImsCallSessionImplBase.State.NEGOTIATING);

        ConferenceInfo ci = ConferenceInfoHelper.getConferenceInfo(mCall.getCallId());

        if ((ci != null) && (participants != null)) {
            for (int i = 0; i < participants.length; ++i) {
                ci.addUserForInterimStatus(0, participants[i],
                        ConferenceInfo.User.STATUS_PENDING, 0, 0);
            }

            if (participants.length > 0) {
                // Notify the status changed for some users
                notifyCallSessionConferenceStateUpdated();
            }
        }
    }

    @Override
    public void accept(int callType, ImsStreamMediaProfile profile) {
        int state = getState();

        if ((state != ImsCallSessionImplBase.State.NEGOTIATING)
                && (state != ImsCallSessionImplBase.State.RENEGOTIATING)) {
            // FIXME: notify the event result - Illegal state
            loge("accept :: Illegal state; callId=" + getCallId() +
                    ", state=" + ImsCallSessionImplBase.State.toString(state));
            return;
        }

        // FIXME: If the media profile is not matched with the call type,
        // it needs to be re-formed properly based on the call type.
        MediaInfo mediaInfo;

        if (ImsCallMediaUtils.isDefaultMediaProfile(profile)) {
            // FIXME: check the network type
            int audioCaps = mCallContext.getMediaCapabilities(callType, ICallContext.MEDIA_AUDIO);
            int videoCaps = mCallContext.getMediaCapabilities(callType, ICallContext.MEDIA_VIDEO);

            log("accept :: audioCaps=" + audioCaps + ", videoCaps=" + videoCaps
                    + ", mediaProfile" + mCallProfile.getMediaProfile());

            // Application expects that the incoming media profile reflects
            // the local media profile in the IMS signaling level.
            // For example,
            //    - "recvonly" by remote end: "sendonly"
            //    - "sendonly" by remote end: "recvonly"

            if (state == ImsCallSessionImplBase.State.RENEGOTIATING) {
                mediaInfo = ImsCallMediaUtils.createMediaInfoForCallAccept(
                        mProposedCallProfile, callType, audioCaps, videoCaps);
            } else {
                mediaInfo = ImsCallMediaUtils.createMediaInfoForCallAccept(
                        mCallProfile, callType, audioCaps, videoCaps);
            }
        } else {
            mediaInfo = ImsCallMediaUtils.createMediaInfoFromMediaProfile(profile);
        }

        boolean isRttCall = false;
        if (profile != null) {
            isRttCall = profile.isRttCall();
        }

        setRttGttInfo(mediaInfo, isRttCall);
        mCall.accept(ImsCallUtils.getCallTypeFromProfile(callType, isRttCall), mediaInfo);

        // Skip a state, ImsCallSessionImplBase.State.ESTABLISHING and REESTABLISHING
        setState(ImsCallSessionImplBase.State.ESTABLISHED);
        clearProposedCallProfile();
    }

    @Override
    public void reject(int reason) {
        int state = getState();

        // If the reject method is invoked in ESTABLISHED state,
        // we assume that it is an intention of the user, and terminate this call.
        if (state == ImsCallSessionImplBase.State.ESTABLISHED) {
            logi("reject >> terminate");
            try {
                terminate(reason);
            } catch (Throwable t) {
                loge("reject :: " + t.toString());
            }
            return;
        }

        if ((state != ImsCallSessionImplBase.State.NEGOTIATING)
                && (state != ImsCallSessionImplBase.State.RENEGOTIATING)) {
            // FIXME: notify the event result - Illegal state
            loge("reject :: Illegal state; callId=" + getCallId() +
                    ", state=" + ImsCallSessionImplBase.State.toString(state));

            // We can consider that if there is an immediate call end reason,
            // it was cancelled by the remote end before alerting the user.
            if (mImmediateCallEndReason != null) {
                log("Call end reason :: " + mImmediateCallEndReason);
                notifyCallStartFailed(mImmediateCallEndReason);
            } else if ((state == ImsCallSessionImplBase.State.TERMINATED)
                    && !mCallDetails.is(CallDetails.CALL_END_FINISHED)) {
                waitOrNotifyCallTerminated(ImsReasonInfo.CODE_USER_TERMINATED,
                        ImsReasonInfo.CODE_UNSPECIFIED,
                        ImsCallUtils.REASON_CALL_DISCONNECTED_BY_USER);
            }
            return;
        }

        if (state == ImsCallSessionImplBase.State.RENEGOTIATING) {
            mCall.reject(CallReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION);
            setState(ImsCallSessionImplBase.State.ESTABLISHED);
        } else {
            mCall.reject(ImsCallUtils.getRejectCallReasonInfoCodeFromImsReasonInfo(reason));
            setState(ImsCallSessionImplBase.State.TERMINATED);
        }

        clearProposedCallProfile();
    }

    @Override
    public void transfer(@NonNull String number, boolean isConfirmationRequired) {
       // FIXME : This is for blind/assured Call Transfer need to confirm than Call App support this

       /** UE-B -> Transferor , UE-A -> Transferee  UE-C -> Transfer Target (number)
        * a) blind transfer: the transferor wants to perform the transfer without any further
        *    ````action on the transfer operation; or
        * b) assured transfer: the transferor wants to have a Confirmation on the transfer operation
        * ```````progress with the possibility to retrieve the communication with the transferee;
        *
        * if  isConfirmationRequired  = false-> its blind transfer
        * if  isConfirmationRequired  = true -> its assured transfer
        */

        log("Blind/Assured CallTransfer :: callId=" + getCallId());

        if (mCallDetails.is(CallDetails.ON_ECT)) {
            log("ECT is already in progress.");
            return;
        }

        mIsEctConfirmationRequired = isConfirmationRequired;
        mTransferTargetNumber = number;

        mTransferRequestedSession = this;

        if (mCall.isOnHold()) {
            mCall.transfer(mTransferTargetNumber);
        } else if (!mCallDetails.is(CallDetails.ON_HOLDING)) {
            mCall.hold(MtcCallUtils.createHoldMedia(
                    mCall.getCallInfo(), mCall.getMediaInfo(),
                    isCallFeatureSupported(CF_VIDEO_HOLD_WITH_INACTIVE),
                    isCallFeatureSupported(CF_TEXT_HOLD_WITH_INACTIVE)));
        }

        mCallDetails.set(CallDetails.ON_ECT);
    }

    @Override
    public void transfer(@NonNull ImsCallSessionImplBase otherSession)  {
        log("Consultative CallTransfer :: callId=" + getCallId());

        ImsCallApp callApp = (ImsCallApp) mCallContext.getApp();
        ImsCallSessionImpl fgCallSession = callApp.getCallManager().getActiveSession();

        if (this != fgCallSession) {
            /* {@link #mTransferTargetSession} is a session which is getting hold before
             * it get transferred and transfer request will be send
             * on {@link #mTransferRequestedSession}
             */
            mTransferTargetSession = fgCallSession;
            mIsEctConfirmationRequired = true;
            fgCallSession.doConsultativeTransfer(this);
            return;
        }
        /* Per 3GPP TS 24.629 - A.2, the signalling for a consultative transfer should send the
         * REFER on the background held call with the foreground call specified as the destination.
         * FW will call {@link #transfer} on background session only.
         */
    }

    private void doConsultativeTransfer(ImsCallSessionImpl transferRequestedSession) {
        if (mCallDetails.is(CallDetails.ON_ECT)) {
            log("ECT is already in progress.");
            return;
        }
        log("Consultative CallTransfer :: callId=" + getCallId());

        mTransferRequestedSession = transferRequestedSession;

        /* If required we can pass transfer target ID or Session to native
         * Currently we can pass "null" to keep native implemation as it is for
         * Consultative transfer.
         */
        if (mCall.isOnHold()) {
            mTransferRequestedSession.mCall.transfer(null);
        } else if (!mCallDetails.is(CallDetails.ON_HOLDING)) {
            mCall.hold(MtcCallUtils.createHoldMedia(
                    mCall.getCallInfo(), mCall.getMediaInfo(),
                    isCallFeatureSupported(CF_VIDEO_HOLD_WITH_INACTIVE),
                    isCallFeatureSupported(CF_TEXT_HOLD_WITH_INACTIVE)));
        }

        /* {@link #ON_ECT} will be set for {@link #mTransferTargetSession} session only.
         */
        mCallDetails.set(CallDetails.ON_ECT);
    }

    @Override
    public void terminate(int reason) {
        int state = getState();

        if (state == ImsCallSessionImplBase.State.TERMINATING) {
            return;
        }

        setTerminationReason(reason);

        if (state == ImsCallSessionImplBase.State.TERMINATED) {
            if (!mCallDetails.is(CallDetails.CALL_END_FINISHED)) {
                if (mCallDetails.is(CallDetails.MO) && !mCallDetails.is(CallDetails.MO_STARTED)) {
                    logi("Callback-Replacement(terminate) :: "
                            + "onCallTerminated >> onCallStartFailed");

                    final ImsReasonInfo reasonInfo = ImsCallUtils.createReasonInfo(
                            ImsReasonInfo.CODE_USER_TERMINATED,
                            ImsReasonInfo.CODE_UNSPECIFIED,
                            ImsCallUtils.REASON_CALL_DISCONNECTED_BY_USER,
                            ImsCallUtils.FLAG_REASON_INFO_NONE);

                    notifyCallStartFailed(reasonInfo);
                } else {
                    waitOrNotifyCallTerminated(ImsReasonInfo.CODE_USER_TERMINATED,
                            ImsReasonInfo.CODE_UNSPECIFIED,
                            ImsCallUtils.REASON_CALL_DISCONNECTED_BY_USER);
                }
            }
            return;
        }

        if ((state < ImsCallSessionImplBase.State.NEGOTIATING)
                && (((mMoPendingCall != null) && !mMoPendingCall.isStartDone())
                    || ((mLocationBasedCall != null) && !mLocationBasedCall.isStartDone())
                    || ((mUsatBasedCall != null) && !mUsatBasedCall.isStartDone()))) {
            CallReasonInfo callReasonInfo = new CallReasonInfo(
                    CallReasonInfo.CODE_USER_TERMINATED, 0, "");
            mListenerProxy.onCallStartFailed(mCall, callReasonInfo);
            return;
        }

        // Terminate the ongoing conference call
        clearConferenceProxy();
        mCall.terminate(ImsCallUtils.getTerminateCallReasonInfoCodeFromImsReasonInfo(reason));

        if (getState() != ImsCallSessionImplBase.State.TERMINATED) {
            setState(ImsCallSessionImplBase.State.TERMINATING);
        }

        // Checks if the conference state notification is required...
    }

    @Override
    public void hold(ImsStreamMediaProfile profile) {
        int state = getState();
        int audioDirection = profile.getAudioDirection();
        int videoDirection = profile.getVideoDirection();

        if ((state == ImsCallSessionImplBase.State.TERMINATING)
                || (state == ImsCallSessionImplBase.State.TERMINATED)) {
            loge("hold :: Illegal state; callId=" + getCallId() +
                    ", state=" + ImsCallSessionImplBase.State.toString(state));

            notifyCallHoldOrResumeFailed(ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED, true);

            if ((state == ImsCallSessionImplBase.State.TERMINATED)
                    && !mCallDetails.is(CallDetails.CALL_END_FINISHED)) {
                waitOrNotifyCallTerminated(ImsReasonInfo.CODE_USER_TERMINATED,
                        ImsReasonInfo.CODE_UNSPECIFIED, "");
            }

            return;
        } else if (state == ImsCallSessionImplBase.State.RENEGOTIATING) {
            loge("hold :: Illegal state; callId=" + getCallId() +
                    ", state=" + ImsCallSessionImplBase.State.toString(state));

            notifyCallHoldOrResumeFailed(ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE, true);
            return;
        }

        // Checks if the user is held by the remote party and adjusts the direction if changes
        if (isCallFeatureSupported(CF_AUDIO_HOLD_WITH_INACTIVE) || mCall.isOnHeld()) {
            audioDirection = ImsStreamMediaProfile.DIRECTION_INACTIVE;
        } else {
            // AudioDirection : DIRECTION_INACTIVE or DIRECTION_SEND
            // No need to update the call state
        }

        // Video direction should be set to INACTIVE for video call hold.
        if (ImsCallUtils.isVideoCall(mCallProfile.getCallType())) {
            if (mCall.isOnHeld()) {
                videoDirection = ImsStreamMediaProfile.DIRECTION_INACTIVE;
            } else {
                if (isCallFeatureSupported(CF_VIDEO_HOLD_WITH_INACTIVE)) {
                    log("Video direction :: INACTIVE");
                    videoDirection = ImsStreamMediaProfile.DIRECTION_INACTIVE;
                } else if (mCall.is1WayVideo() || mCall.is1WayVideoByRemoteEnd()) {
                    log("Video direction :: INACTIVE by 1 way video");
                    videoDirection = ImsStreamMediaProfile.DIRECTION_INACTIVE;
                }
            }
        }

        mCallDetails.set(CallDetails.ON_HOLDING);

        ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile(
                profile.getAudioQuality(), audioDirection, profile.getVideoQuality(),
                videoDirection, profile.getRttMode());

        MediaInfo mediaInfo = ImsCallMediaUtils.createMediaInfoFromMediaProfile(mediaProfile);
        setRttGttInfo(mediaInfo, mCallProfile.getMediaProfile().isRttCall());

        setState(ImsCallSessionImplBase.State.RENEGOTIATING);
        mCall.hold(mediaInfo);
    }

    @Override
    public void resume(ImsStreamMediaProfile profile) {
        int state = getState();
        int audioDirection = profile.getAudioDirection();
        int videoDirection = profile.getVideoDirection();

        if ((state == ImsCallSessionImplBase.State.TERMINATING)
                || (state == ImsCallSessionImplBase.State.TERMINATED)) {
            loge("resume :: Illegal state; callId=" + getCallId() +
                    ", state=" + ImsCallSessionImplBase.State.toString(state));

            notifyCallHoldOrResumeFailed(ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED, false);

            if ((state == ImsCallSessionImplBase.State.TERMINATED)
                    && !mCallDetails.is(CallDetails.CALL_END_FINISHED)) {
                waitOrNotifyCallTerminated(ImsReasonInfo.CODE_USER_TERMINATED,
                        ImsReasonInfo.CODE_UNSPECIFIED, "");
            }

            return;
        }

        // Checks if the user is held by the remote party and adjusts the direction if changes
        if (mCall.isOnHeld()) {
            audioDirection = ImsStreamMediaProfile.DIRECTION_RECEIVE;
        } else {
            // AudioDirection : DIRECTION_SEND_RECEIVE or DIRECTION_RECEIVE
            // No need to update the call state
        }

        if (ImsCallUtils.isVideoCall(mCallProfile.getCallType())) {

            if (mCall.isOnHeld()) {
                if (isCallFeatureSupported(CF_VIDEO_HOLD_WITH_INACTIVE)) {
                    videoDirection = ImsStreamMediaProfile.DIRECTION_INACTIVE;
                } else {
                    videoDirection = ImsStreamMediaProfile.DIRECTION_RECEIVE;
                }
            } else if (mVideoCallSession != null) {
                if (!mVideoCallSession.isCameraOn()
                        && (profile.getVideoDirection()
                            == ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE)) {
                    log("Video direction :: RECEIVE by cam off");
                    videoDirection = ImsStreamMediaProfile.DIRECTION_RECEIVE;
                } else if (isCallFeatureSupported(CF_ONE_WAY_VIDEO_LOCAL)
                        && mVideoCallSession.isCameraOn() && mCall.is1WayVideo()) {
                    log("Video direction :: SEND by 1 way video");
                    videoDirection = ImsStreamMediaProfile.DIRECTION_SEND;
                } else if (isCallFeatureSupported(CF_ONE_WAY_VIDEO_REMOTE)
                        && mVideoCallSession.isCameraOn() && mCall.is1WayVideoByRemoteEnd()) {
                    log("Video direction :: RECEIVE by 1 way video by remote end");
                    videoDirection = ImsStreamMediaProfile.DIRECTION_RECEIVE;
                }
            }
        }

        mCallDetails.set(CallDetails.ON_UNHOLDING);
        ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile(
                profile.getAudioQuality(), audioDirection, profile.getVideoQuality(),
                videoDirection, profile.getRttMode());

        MediaInfo mediaInfo = ImsCallMediaUtils.createMediaInfoFromMediaProfile(mediaProfile);
        setRttGttInfo(mediaInfo, mCallProfile.getMediaProfile().isRttCall());

        setState(ImsCallSessionImplBase.State.RENEGOTIATING);
        mCall.resume(mediaInfo);
    }

    @Override
    public void merge() {
        if (isConferenceTransitionInProgress()) {
            loge("merge :: Illegal state - merge in progress; callId=" + getCallId()
                    + ", state=" + ImsCallSessionImplBase.State.toString(getState()));
            return;
        }

        // To merge the calls in a single thread
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                int state = getState();

                if ((state == ImsCallSessionImplBase.State.RENEGOTIATING)
                        || (state == ImsCallSessionImplBase.State.REESTABLISHING)) {
                    loge("merge :: Illegal state - callId=" + getCallId()
                            + ", state=" + ImsCallSessionImplBase.State.toString(state));

                    notifyCallSessionMergeFailed(
                            CallReasonInfo.CODE_UNSPECIFIED, "",
                            ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
                    return;
                }

                if (!ImsConferenceHelper.getInstance().merge(mCallContext)) {
                    notifyCallSessionMergeFailed(
                            CallReasonInfo.CODE_UNSPECIFIED, "", 0);
                }
            }
        });
    }

    @Override
    public void update(int callType, ImsStreamMediaProfile profile) {
        if (mVideoCallSession.isClearedSessionModificationInfo()) {
            ImsStreamMediaProfile existingMediaProfile = mCallProfile.getMediaProfile();

            ImsCallProfile callProfile = ImsCallUtils.createCallProfile(
                    mCallProfile.getServiceType(), ImsCallMediaUtils.getVideoCallType(profile),
                    existingMediaProfile.getAudioQuality(),
                    existingMediaProfile.getAudioDirection(),
                    existingMediaProfile.getVideoQuality(),
                    existingMediaProfile.getVideoDirection(),
                    existingMediaProfile.getRttMode());
            setCallInfo(callProfile);

            mCallback.invokeUpdated(ImsCallSessionImpl.this,
                    ImsCallUtils.getSanitizedCallProfileForVideoDirection(callProfile));
            return;
        }

        final int requestedCallType = callType;
        final ImsStreamMediaProfile requestedProfile = profile;

        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                updateInternal(requestedCallType, requestedProfile);
            }
        });
    }

    @Override
    public void extendToConference(String[] participants) {
        // FIXME: notify the event result - Illegal state
        if (participants == null) {
            return;
        }

        if (participants.length == 0) {
            // no-op
            return;
        }

        final String[] users = Arrays.copyOf(participants, participants.length);

        // To extend the call to the conference in a single thread
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                if (!ImsConferenceHelper.getInstance().extendToConference(
                        mCallContext, users)) {
                    MtcConference conference = MtcCall.getConference(mCall);
                    if (conference == null) {
                        return;
                    }

                    mConferenceListenerProxy.onCallConferenceExtendFailed(
                            conference,
                            new CallReasonInfo(CallReasonInfo.CODE_UNSPECIFIED, 0, ""));
                }
            }
        });
    }

    @Override
    public void inviteParticipants(String[] participants) {
        // FIXME: notify the event result - Illegal state
        log("inviteParticipants :: participants=" + ImsLog.hiddenString(participants));

        if ((participants == null) || (participants.length == 0)) {
            mCallback.invokeInviteParticipantsRequestFailed(this,
                    ImsCallUtils.createReasonInfo(
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT,
                        ImsReasonInfo.CODE_UNSPECIFIED, "", 0));
            return;
        }

        UsersInfo usersInfo = createUsersInfoForInvitation(participants);

        if (usersInfo == null) {
            // NOT_REACHABLE_CODE
            MtcCall.inviteParticipants(mCall,
                    MtcCallUtils.createUsersInfo(participants));
            return;
        }

        MtcCall.inviteParticipants(mCall, usersInfo);

        // Notify the status changed for some users
        notifyCallSessionConferenceStateUpdated();
    }

    @Override
    public void removeParticipants(String[] participants) {
        // FIXME: notify the event result - Illegal state
        log("removeParticipants :: participants=" + ImsLog.hiddenString(participants));

        if ((participants == null) || (participants.length == 0)) {
            mCallback.invokeRemoveParticipantsRequestFailed(this,
                    ImsCallUtils.createReasonInfo(
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT,
                        ImsReasonInfo.CODE_UNSPECIFIED, "", 0));
            return;
        }

        int ccId = 0;

        if ((participants.length == 1)
                && participants[0].startsWith(PREFIX_MMI_PARTICIPANT_DROP)) {
            if (mCall.getCallExtraBoolean(Call.EXTRA_CONFERENCE_EVENT, false)) {
                String callId = participants[0].substring(PREFIX_MMI_PARTICIPANT_DROP.length());

                try {
                    ccId = Integer.parseInt(callId);
                } catch (NumberFormatException e) {
                    log("Invalid callId=" + participants[0]);
                }
            }

            if (ccId <= 0) {
                mCallback.invokeRemoveParticipantsRequestFailed(this,
                        ImsCallUtils.createReasonInfo(
                            ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT,
                            ImsReasonInfo.CODE_UNSPECIFIED, "", 0));
                return;
            }
        }

        UsersInfo usersInfo = createUsersInfoForDrop(participants, ccId);

        if (usersInfo == null) {
            // NOT_REACHABLE_CODE
            // Ignore undetected participants...
            //MtcCall.removeParticipants(mCall,
            //        MtcCallUtils.createUsersInfo(participants));
            return;
        }

        MtcCall.removeParticipants(mCall, usersInfo);

        // Notify the status changed for some users
        notifyCallSessionConferenceStateUpdated();
    }

    @Override
    public void sendDtmf(char c, Message result) {
        if (ImsCallMediaUtils.isDtmfEvent(c)) {
            mCall.sendDtmf(c);
        } else {
            loge("sendDtmf :: not supported - dtmf=" + c);
        }

        // P-OS: Messenger object (replyTo) is introduced
        if ((result != null) && (result.replyTo != null)) {
            try {
                result.replyTo.send(result);
            } catch (RemoteException e) {
                loge("sendDtmf :: reply error - " + e.toString());
            }
        }
    }

    @Override
    public void startDtmf(char c) {
        // Same as sendDtmf(char, Message)
        sendDtmf(c, null);
    }

    @Override
    public void stopDtmf() {
        // No actions in Ims implementation.
    }

    @Override
    public void sendUssd(String ussdMessage) {
        // FIXME: notify the event result - Illegal state

        mCall.sendUssd(ussdMessage);
    }

    @Override
    public ImsVideoCallProvider getImsVideoCallProvider() {
        return mVideoCallProvider;
    }

    @Override
    public boolean isMultiparty() {
        boolean isMultiparty = (mCallProfile != null) ?
                mCallProfile.getCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE) : false;

        return isMultiparty;
    }

    @Override
    public void sendRttModifyRequest(ImsCallProfile toProfile) {
        if (toProfile == null) {
            mCallback.invokeRttModifyResponseReceived(this,
                    RttModifyStatus.SESSION_MODIFY_REQUEST_INVALID);
            return;
        }

        if (getState() != ImsCallSessionImplBase.State.ESTABLISHED) {
            loge("sendRttModifyRequest :: Illegal state; callId=" + getCallId() +
                    ", state=" + ImsCallSessionImplBase.State.toString(getState()));
            mCallback.invokeRttModifyResponseReceived(this,
                    RttModifyStatus.SESSION_MODIFY_REQUEST_FAIL);
            return;
        }

        boolean isRttOn = toProfile.getMediaProfile().isRttCall();

        if (isRttOn == mCallProfile.getMediaProfile().isRttCall()) {
            log("sendRttModifyRequest :: No need to modify RTT (already in mode)");
            mCallback.invokeRttModifyResponseReceived(this,
                    RttModifyStatus.SESSION_MODIFY_REQUEST_SUCCESS);
            return;
        }

        if (isRttOn) {
            mCallDetails.set(CallDetails.RTT_TURNING_ON);
        } else {
            mCallDetails.set(CallDetails.RTT_TURNING_OFF);
        }

        MediaInfo mediaInfo =
                ImsCallMediaUtils.createMediaInfoFromMediaProfile(mCallProfile.getMediaProfile());
        setRttInfo(mediaInfo, isRttOn);

        setState(ImsCallSessionImplBase.State.RENEGOTIATING);
        mCall.update(
                ImsCallUtils.getCallTypeFromProfile(mCallProfile.getCallType(), isRttOn),
                        mediaInfo);
    }

    @Override
    public void sendRttModifyResponse(boolean isRttOn) {
        if (getState() != ImsCallSessionImplBase.State.RENEGOTIATING) {
            loge("sendRttModifyResponse :: Illegal state; callId=" + getCallId() +
                    ", state=" + ImsCallSessionImplBase.State.toString(getState()));
            return;
        }

        MediaInfo mediaInfo = ImsCallMediaUtils.createMediaInfoFromMediaProfile(
                mProposedCallProfile.getMediaProfile());

        setRttInfo(mediaInfo, isRttOn);

        mCall.accept(
                ImsCallUtils.getCallTypeFromProfile(mCallProfile.getCallType(), isRttOn),
                        mediaInfo);

        setState(ImsCallSessionImplBase.State.ESTABLISHED);
        clearProposedCallProfile();
    }

    @Override
    public void sendRttMessage(String rttMessage) {
        if (mCall == null) {
            loge("sendRttMessage :: session is null");
            return;
        }
        if (!mCallProfile.getMediaProfile().isRttCall()) {
            loge("sendRttMessage :: not RTT call. ignore");
            return;
        }

        mCall.sendRttMessage(rttMessage);
    }

    @Override
    public void sendRtpHeaderExtensions(@NonNull Set<RtpHeaderExtension> rtpHeaderExtensions) {
        if (mCall == null) {
            loge("sendRtpHeaderExtensions :: session is null");
            return;
        }

        mCall.sendRtpHeaderExtensions(rtpHeaderExtensions);
    }

    /**
     * Deliver the bitrate for the indicated media type, direction and bitrate to the upper layer.
     *
     * @param mediaType MediaType is used to identify media stream such as audio or video.
     * @param direction Direction of this packet stream (e.g. uplink or downlink).
     * @param bitsPerSecond This value is the bitrate received from the NW through the Recommended
     *        bitrate MAC Control Element message and ImsStack converts this value from MAC bitrate
     *        to audio/video codec bitrate (defined in TS26.114).
     * @hide
     */
    @Override
    public void callSessionNotifyAnbr(int mediaType, int direction, int bitsPerSecond) {
        if (mCall == null) {
            loge("callSessionNotifyAnbr :: session is null");
            return;
        }

        mCall.notifyAnbr(mediaType, direction, bitsPerSecond);
    }

    // @Override
    // @QUALCOMM_API
    public void deflect(String deflectNumber) {
        // FIXME: IMPL_REQUIRED
    }

    // @Override
    // @QUALCOMM_API
    public int getCallSubstate() {
        // FIXME: IMPL_REQUIRED
        return 0;
    }

    public int getPreciseState() {
        if (!mCall.isOnHold() && (mState == ImsCallSessionImplBase.State.ESTABLISHED)) {
            return PreciseCallState.PRECISE_CALL_STATE_ACTIVE;
        } else if (mState == ImsCallSessionImplBase.State.ESTABLISHING) {
            return PreciseCallState.PRECISE_CALL_STATE_ALERTING;
        } else if (mState == ImsCallSessionImplBase.State.TERMINATED) {
            return PreciseCallState.PRECISE_CALL_STATE_DISCONNECTED;
        } else if (mState == ImsCallSessionImplBase.State.TERMINATING) {
            return PreciseCallState.PRECISE_CALL_STATE_DISCONNECTING;
        } else if (mCall.isOnHold() || (mCallDetails.is(CallDetails.ON_HOLDING)
                && (mState == ImsCallSessionImplBase.State.RENEGOTIATING))) {
            return PreciseCallState.PRECISE_CALL_STATE_HOLDING;
        } else if (mCall.isOnPreIncoming()) {
            return PreciseCallState.PRECISE_CALL_STATE_INCOMING_SETUP;
        } else if (mCallDetails.is(CallDetails.MO)
                && ((mState != ImsCallSessionImplBase.State.INVALID)
                && (mState <= ImsCallSessionImplBase.State.NEGOTIATING))) {
            return PreciseCallState.PRECISE_CALL_STATE_DIALING;
        } else if ((mCT.getActiveCalls() > 0) && !mCallDetails.is(CallDetails.MO)
                && (mState == ImsCallSessionImplBase.State.IDLE)) {
            return PreciseCallState.PRECISE_CALL_STATE_WAITING;
        } else if (!mCallDetails.is(CallDetails.MO)
                && ((mState != ImsCallSessionImplBase.State.INVALID)
                && (mState <= ImsCallSessionImplBase.State.NEGOTIATING))) {
            return PreciseCallState.PRECISE_CALL_STATE_INCOMING;
        }

        return PreciseCallState.PRECISE_CALL_STATE_NOT_VALID;
    }

    public void alertUser() {
        if (mCall == null) {
            return;
        }

        if (mCall.isTerminatedByAutoRejectedCall()) {
            log("alertUser for Auto Rejected Call");
            int code = ImsReasonInfo.CODE_UNSPECIFIED;
            String callDisconnectCause = mCallProfile.getCallExtra(
                    ImsCallProfile.EXTRA_CALL_DISCONNECT_CAUSE, null);
            if (callDisconnectCause != null) {
                try {
                    code = Integer.parseInt(callDisconnectCause);
                } catch (NumberFormatException e) {
                    loge("Call extra is invalid");
                }
            }

            notifyCallStartFailed(ImsCallUtils.createReasonInfo(
                    code, 0, "", ImsCallUtils.FLAG_REASON_INFO_NONE));
            return;
        }

        mCall.alertUser();
    }

    /**
     * Called when the framework has been notified of an incoming call.
     *
     * If the notification is not handled by the framework, the call will be rejected with
     * {@link ImsReasonInfo#CODE_LOCAL_SERVICE_UNAVAILABLE}
     *
     * @param isHandled Indicates whether the framework has handled the incoming call notification.
     *        If false, the telephony will not invoke {@code close()}. The ImsStack must internally
     *        handle the call closing mechanism.
     */
    public void onIncomingcallNotified(boolean isHandled) {
        log("onIncomingcallNotified isHandled: " +  isHandled);
        if (isHandled) {
            mCallDetails.set(CallDetails.TELEPHONY_LISTENING);

            if (mImmediateCallEndReason != null) {
                log("Call end reason :: " + mImmediateCallEndReason);
                notifyCallStartFailed(mImmediateCallEndReason);
                return;
            }

            alertUser();
        } else {
            reject(ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
            closeInternal(ImsCallSessionImpl.this);
        }
    }

    public int getCallConnectionId() {
        return mLocalCallProfile.getCallExtraInt(Call.EXTRA_CALL_CONNECTION_ID, 0);
    }

    public void setCallConnectionId(int ccId) {
        int oldCcId = getCallConnectionId();

        if (oldCcId != ccId) {
            logi("setCallConnectionId :: callId=" + getCallId()
                    + ", callConnectionId: " + oldCcId + " >> " + ccId);

            mLocalCallProfile.setCallExtraInt(Call.EXTRA_CALL_CONNECTION_ID, ccId);
        }
    }

    public MtcCall getMtcCall() {
        return mCall;
    }

    public boolean isConferenceTransitionInProgress() {
        return (mConferenceProxy != null);
    }

    public void notifyCallTerminatedByServiceUnavailable() {
        if (getState() == ImsCallSessionImplBase.State.TERMINATED) {
            return;
        }

        // Don't change the state in the moment; close() operation will do it.
        //setState(ImsCallSessionImplBase.State.TERMINATED);

        setTerminationReason(ImsReasonInfo.CODE_LOCAL_NOT_REGISTERED);

        notifyCallTerminated(
                CallReasonInfo.CODE_LOCAL_NOT_REGISTERED,
                ImsReasonInfo.CODE_UNSPECIFIED,
                ImsCallUtils.REASON_IMS_NOT_REGISTERED);
    }

    public void setConferenceProxy(ConferenceProxy confProxy) {
        if ((mConferenceProxy != null) && (confProxy == null)) {
            clearConferenceProxy();
        }

        mConferenceProxy = confProxy;

        if (mConferenceProxy != null) {
            mConferenceProxy.addListener(mListenerProxy, mConferenceListenerProxy);

            if (((mCall != null) && mCall.isConference())
                    || (isFirstCallMergeInitiator()
                        && mConferenceProxy.isConferenceForCallMerge())) {
                mCallDetails.set(CallDetails.ON_MERGING);
            }
        }
    }

    public void takeCall() {
        // It's called inside of getPendingCall(...)
        if (getState() == ImsCallSessionImplBase.State.IDLE) {
            setState(ImsCallSessionImplBase.State.NEGOTIATING);
        } else {
            log("takeCall :: state=" + getState() + ", callEndReason="
                    + ((mImmediateCallEndReason != null) ? mImmediateCallEndReason : "__null__"));
        }

        notifyCallEventForVideoCallSession(IVideoCallSession.EVENT_CALL_ALERTING);
    }

    public void updateCallProfileByCallManager() {
        if (ImsCallUtils.isCallOnNativeAppsAndCountryKR(mCallContext)) {
            int activeCalls = mCT.getActiveCalls();

            if (activeCalls == 1) {
                log("updateCallProfileByCallManager");
                if (updateCallTypeChangeCapability()
                        && getState() == ImsCallSessionImplBase.State.ESTABLISHED) {
                    mCallback.invokeUpdated(ImsCallSessionImpl.this,
                            ImsCallUtils.getSanitizedCallProfileForVideoDirection(mCallProfile));
                }
            }
        }
    }

    @VisibleForTesting
    public CallDetails getCallDetails() {
        return mCallDetails;
    }

    protected Handler getCallHandler() {
        return mCallContext.getCallHandler();
    }

    @VisibleForTesting
    protected MtcCallListenerProxy getCallListenerProxy() {
        return mListenerProxy;
    }

    @VisibleForTesting
    protected MtcCall.IEmergencyCallFailureListener getEmergencyCallFailureListener() {
        return mEmergencyCallFailureListener;
    }

    @VisibleForTesting
    public boolean isCacheCallReasonInfoNull() {
        return mCacheCallReasonInfo == null;
    }

    protected boolean isCallFeatureSupported(int feature) {
        if (mCallFeatureCache.containsKey(feature)) {
            return mCallFeatureCache.get(feature);
        } else {
            boolean isFeatureSupported = getFeatureSupportedStatus(feature);
            mCallFeatureCache.put(feature, isFeatureSupported);
            return isFeatureSupported;
        }
    }

    private void waitOrNotifyCallTerminated(int code, int extraCode, String extraMessage) {
        if (!mCallDetails.is(CallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END)) {
            notifyCallTerminated(code, extraCode, extraMessage);
        } else {
            mCacheCallReasonInfo = new CallReasonInfo(code, extraCode, extraMessage);
        }
    }

    private boolean getFeatureSupportedStatus(int feature) {
        int slotId = mCallContext.getSlotId();
        switch (feature) {
            case CF_TTY:
                return CallFeature.isTtySupported(slotId);
            case CF_AUDIO_HOLD_WITH_INACTIVE:
                return CallFeature.isCallHoldUsingInactive(slotId);
            case CF_VIDEO_HOLD_WITH_INACTIVE:
                return CallFeature.isVideoDirectionInactiveOnVideoCallHold(slotId);
            case CF_TEXT_HOLD_WITH_INACTIVE:
                return CallFeature.isTextDirectionInactiveOnRttCallHold(slotId);
            case CF_INCOMING_RESUME_EVENT:
                return CallFeature.isIncomingResumeEventSupported(slotId);
            case CF_ONE_WAY_VIDEO_LOCAL:
                return CallFeature.isOneWayVideoCallByLocalEndSupported(slotId);
            case CF_ONE_WAY_VIDEO_REMOTE:
                return CallFeature.isOneWayVideoCallByRemoteEndSupported(slotId);
            case CF_CONF_USER_ANONYMOUS:
                return CallFeature.isNotifyConfStateWhenAnonymousUser(slotId);
            default:
                return false;
        }
    }

    private void clearConferenceProxy() {
        if (mConferenceProxy != null) {
            mConferenceProxy.removeListener(mListenerProxy, mConferenceListenerProxy);
            mConferenceProxy = null;

            // Recover the listener for this session
            if ((mCall != null) && (getState() != ImsCallSessionImplBase.State.TERMINATED)) {
                if (!mCall.isTerminated()) {
                    mCall.setListener(mListenerProxy);
                    MtcCall.setListener(mCall, mConferenceListenerProxy);
                } else {
                    postAndRunTask(new Runnable() {
                        @Override
                        public void run() {
                            if (mCall != null) {
                                mCall.setListener(mListenerProxy);
                                MtcCall.setListener(mCall, mConferenceListenerProxy);
                            }
                        }
                    });
                }
            }
        }
    }

    private void clearSpecificFeatures() {
        // TTY_MODE
        if (mTtyModeListener != null) {
            TtyModeTracker tmt = mCallContext.getTtyModeTracker();

            if (tmt != null) {
                tmt.removeListener(mTtyModeListener);
            }

            mTtyModeListener = null;
        }
    }

    private void clearLocationBasedCall() {
        if (mLocationBasedCall != null) {
            mLocationBasedCall.dispose();
            mLocationBasedCall = null;
        }
    }

    private void clearPendingCall() {
        if (mMoPendingCall != null) {
            mMoPendingCall.dispose();
            mMoPendingCall = null;
        }
    }

    private void clearUsatBasedCall() {
        if (mUsatBasedCall != null) {
            mUsatBasedCall.dispose();
            mUsatBasedCall = null;
        }
    }

    private void closeInternal(final ImsCallSessionImpl session) {
        if (DBG) {
            log("closeInternal :: session=" + session);
        }

        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (session != null) {
                        /* When {@link #close} the {@link #ImsCallSessionImpl} internally will clear
                         * the {@link #WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END} so it will not
                         * wait for {#link #onAudioSessionClosed}
                         */
                        mCallDetails.clear(CallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END);
                        session.close();
                    }
                } catch (Throwable t) {
                    loge("closeInternal");
                }
            }
        });
    }

    private void closeMtcCall(final MtcCall call) {
        if (DBG) {
            log("closeMtcCall :: call=" + call);
        }

        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                try {
                    if (call != null) {
                        call.setListener(null);
                        call.close();
                    }
                } catch (Throwable t) {
                    log("closeMtcCall");
                }
            }
        });
    }

    private String createCallId(String callId) {
        if (!TextUtils.isEmpty(callId)) {
            return callId;
        }

        return String.valueOf(this.hashCode());
    }

    private ImsConferenceState createConferenceState(String ccid) {
        // usersInfo will not be used in the moment.
        ConferenceInfo ci = ConferenceInfoHelper.getConferenceInfo(ccid);

        if ((ci == null) || !ci.hasActiveUser()) {
            log("createConferenceState :: no active users");
            return null;
        }

        List<ConferenceInfo.User> confUsers = ci.getUsers();
        ImsConferenceState confState = getImsConferenceState();

        for (int i = 0; i < confUsers.size(); ++i) {
            ConferenceInfo.User user = confUsers.get(i);

            if (!user.isReportable()) {
                // This user doesn't have a reportable state,
                // so it will not be passed to the application.
                continue;
            }

            String key = user.getUuid();
            String endpoint = key;

            if (TextUtils.isEmpty(endpoint)) {
                endpoint = user.getEndpoint();
            }

            confState.mParticipants.put(key,
                    ImsCallUtils.createConferenceParticipant(
                        user.getId(), endpoint,
                        user.getDisplayText(), user.getStatus(),
                        user.getSIPStatusCode(),
                        user.getDisconnectedCause()));

            if (DBG) {
                log("ConferenceState(add) :: " + user);
            }
        }

        return confState;
    }

    @VisibleForTesting
    protected @NonNull ImsConferenceState getImsConferenceState() {
        Parcel parcel = Parcel.obtain();
        try {
            parcel.writeInt(0);
            parcel.setDataPosition(0);
            return ImsConferenceState.CREATOR.createFromParcel(parcel);
        } finally {
            parcel.recycle();
        }
    }

    private UsersInfo createUsersInfoForInvitation(String[] participants) {
        ConferenceInfo ci = ConferenceInfoHelper.getConferenceInfo(mCall.getCallId());

        if (ci == null) {
            return null;
        }

        UsersInfo usersInfo = new UsersInfo();

        for (int i = 0; i < participants.length; ++i) {
            // KEY: <endpoint, user>
            ConferenceInfo.User user = ci.getUserForEntity(participants[i], participants[i]);

            if (user == null) {
                user = ci.getUser(null, participants[i]);
            }

            if (user != null) {
                // Depends on the MTC enabler's behavior:
                // re-invitation case
                MtcCallUtils.addUser(usersInfo, 0L, participants[i]);

                user.initForInterimEvent(participants[i], participants[i], participants[i],
                        "", ConferenceInfo.User.STATUS_PENDING);
                user.setSIPStatusCode(0);
            } else {
                log("User does not exist; participant=" + ImsLog.hiddenString(participants[i]));

                MtcCallUtils.addUser(usersInfo, 0L, participants[i]);

                ci.addUserForInterimStatus(0, participants[i],
                        ConferenceInfo.User.STATUS_PENDING, 0, 0);
            }
        }

        return usersInfo;
    }

    private UsersInfo createUsersInfoForDrop(String[] participants, int ccId) {
        ConferenceInfo ci = ConferenceInfoHelper.getConferenceInfo(mCall.getCallId());

        if (ci == null) {
            return null;
        }

        int dropCount = (ccId > 0) ? 1 : participants.length;
        UsersInfo usersInfo = new UsersInfo();

        for (int i = 0; i < dropCount; ++i) {
            // KEY: <endpoint, user>
            ConferenceInfo.User user = null;

            if (ccId > 0) {
                user = ci.getUserForCallConnectionId(ccId);
            } else {
                user = ci.getUserForEntity(participants[i], participants[i]);
            }

            if (user == null) {
                user = ci.getUser(null, participants[i]);
            }

            if (user != null) {
                // Depends on the MTC enabler's behavior.
                // TODO: _CONFERENCE_CALL_CONNECTION_ID_
                long callId = Long.parseLong(user.getCallId());
                // MTC enabler expects to set the same value
                // which has been updated by the event for conference participants' change.
                String idForNative = user.getIdForNative();

                if (TextUtils.isEmpty(idForNative)) {
                    idForNative = user.getId();
                }

                if (user.isInterim()) {
                    MtcCallUtils.addUser(usersInfo, callId, idForNative);
                } else {
                    MtcCallUtils.addUser(usersInfo, callId,
                        idForNative, user.getUser(), user.getEndpoint());
                }

                if (FEATURE_CHANGE_CONFERENCE_PARTICIPANT_STATE_ON_DROP) {
                    user.updateStatus(ConferenceInfo.User.STATUS_DISCONNECTING);
                }
            } else {
                // NOT_REACHABLE_CODE
                log("User does not exist; participant=" + ImsLog.hiddenString(participants[i]));

                // Ignore undetected participants...
                //MtcCallUtils.addUser(usersInfo, 0L, participants[i]);
            }
        }

        return usersInfo;
    }

    private ImsCallSessionImpl createNewCallSession(MtcCall call, ImsCallProfile profile) {
        String callId = mCT.createCallId();
        return new ImsCallSessionImpl(mCallContext, mCT, call, callId, profile, true);
    }

    private int getTerminationReason(int reason) {
        return (mTerminationReason != ImsReasonInfo.CODE_UNSPECIFIED) ?
                mTerminationReason : reason;
    }

    private MtcCall getMtcCall(long callId) {
        return (MtcCall)((callId == 0) ? null : mCallContext.getMtcCall(callId));
    }

    private void initCallProfile(ImsCallProfile profile) {
        mLocalCallProfile = profile;
        mCallProfile = ImsCallUtils.cloneCallProfile(profile);
        // FIXME: how to handle this call profile?
        mRemoteCallProfile = new ImsCallProfile(profile.getServiceType(), profile.getCallType());
    }

    private boolean isConferenceExtended(CallInfo si) {
        return !isMultiparty() && MtcCallInfo.isConference(si);
    }

    private boolean isFirstCallMergeInitiator() {
        return (mCall != null)
                && (mConferenceProxy != null)
                && mConferenceProxy.isInitialConferenceExtension()
                && mConferenceProxy.isConferenceExtensionRequestor(mCall);
    }

    private boolean isImplicitTerminatedCondition() {
        return (mCallDetails.is(CallDetails.MERGED)
                || mCallDetails.is(CallDetails.MERGED_N_DETACHED));
    }

    private boolean isTerminationReasonPresent() {
        return (mTerminationReason != ImsReasonInfo.CODE_UNSPECIFIED);
    }

    private boolean isUsatBasedCallRequired() {
        UsatInterface usat = mCallContext.getUsatInterface();
        return (usat != null) ? usat.isServiceAvailable(Usat.SERVICE_CALL_CONTROL) : false;
    }

    private void notifyCallEventForVideoCallSession(final int event) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                mVideoCallSession.notifyCallEvent(event);
            }
        });
    }

    private void notifyCallHoldOrResumeFailed(int code, boolean isHold) {
        int preferredCode = 0;

        if ((getTerminationReason(0) != 0) || mCall.isTerminated()) {
            preferredCode = ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED;
        }

        ImsReasonInfo reasonInfo = ImsCallUtils.createReasonInfo(
                code, ImsReasonInfo.CODE_UNSPECIFIED,
                isHold ? "Hold Failed" : "Resume Failed",
                ImsCallUtils.FLAG_REASON_INFO_ALL, preferredCode);

        if (isHold) {
            mCallback.invokeHoldFailed(this, reasonInfo);
        } else {
            mCallback.invokeResumeFailed(this, reasonInfo);
        }
    }

    private void notifyCallSessionConferenceStateUpdated() {
        notifyCallSessionConferenceStateUpdated(mCall.getCallId());
    }

    private void notifyCallSessionConferenceStateUpdated(String ccid) {
        if (ccid == null) {
            ccid = mCall.getCallId();
        }

        final ImsConferenceState confState = createConferenceState(ccid);

        if (confState == null) {
            log("notifyCallSessionConferenceStateUpdated :: no active users");
            return;
        }

        mCallback.invokeConferenceStateUpdated(this, confState);

        ConferenceInfo ci = ConferenceInfoHelper.getConferenceInfo(ccid);

        if (ci != null) {
            ci.notifyUserStatusIfChanged();
        }
    }

    private void notifyCallSessionMergeFailed(int reason,
            String message, int preferredCode) {
        if (!mCallback.hasListener()) {
            return;
        }

        ImsReasonInfo reasonInfo = ImsCallUtils.createReasonInfo(
                reason, ImsReasonInfo.CODE_UNSPECIFIED, message,
                ImsCallUtils.FLAG_REASON_INFO_ALL, preferredCode);

        mCallback.invokeMergeFailed(this, reasonInfo);
    }

    private void notifyCallSessionMultipartyStateChanged(boolean state) {
        if (state) {
            mCallback.invokeMultipartyStateChanged(this, true);

            // Toast pop-up for multi-party call
            mCallback.invokeSuppServiceReceived(this,
                    ImsSuppServiceUtils.MT.getMultipartyCall());
        }
    }

    private boolean notifyCallStartFailedIfAlreadyTerminated() {
        if (getState() == ImsCallSessionImplBase.State.TERMINATED) {
            boolean callStartFailedNotification = false;

            synchronized (mLock) {
                callStartFailedNotification = ((mImmediateCallEndReason != null)
                        && !mCallDetails.is(CallDetails.CALL_END_FINISHED));
            }

            if (callStartFailedNotification) {
                notifyCallStartFailed(mImmediateCallEndReason);
                return true;
            }
        }

        return false;
    }

    /**
     * To avoid the timing issue when incoming call is immediately terminated by the remote end.
     */
    private void notifyCallStartFailed(final ImsReasonInfo reasonInfo) {
        boolean callbackReplacementRequired = false;

        if (mCallDetails.is(CallDetails.MO)) {
            if (mCallDetails.is(CallDetails.MO_PROGRESSING)) {
                callbackReplacementRequired = true;
            }

            if (ImsCallUtils.isCsSilentRedialRequired(reasonInfo)) {
                /* Call StarFailed callback to re-dial the call via CS
                 * {@link #invokeStartFailed} will called only in case of
                 * {@link #CODE_LOCAL_CALL_CS_RETRY_REQUIRED}. For other cases will call
                 * {@link #invokeTerminated}
                 */
                callbackReplacementRequired = false;
            }
        } else {
            callbackReplacementRequired = true;
        }

        if (callbackReplacementRequired) {
            notifyCallTerminated(reasonInfo);
            return;
        }

        mCallDetails.set(CallDetails.CALL_END_FINISHED);

        mCallback.invokeStartFailed(this, reasonInfo);
        /* When {@link #invokeStartFailed} is called, from framework side {@link #close} will
            * not get called so {@link #MtcCall} for start failed session will not close.
            * This will close the {@link #MtcCall} when call session start failed.
        */
        closeInternal(ImsCallSessionImpl.this);
    }

    private void notifyCallTerminated(int reason, int extraCode, String message) {
        ImsReasonInfo reasonInfo = ImsCallUtils.createReasonInfo(
                getTerminationReason(reason), extraCode, message,
                isTerminationReasonPresent() ?
                ImsCallUtils.FLAG_REASON_INFO_NONE : ImsCallUtils.FLAG_REASON_INFO_ALL);

        notifyCallTerminated(reasonInfo);
    }

    private void notifyCallTerminated(final ImsReasonInfo reasonInfo) {
        if (mCallDetails.is(CallDetails.CALL_END_FINISHED)) {
            return;
        }

        setTerminationReason(reasonInfo.getCode());

        mCallDetails.clear(CallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END);
        mCallDetails.set(CallDetails.CALL_END_FINISHED);

        // the Telephony doesn't require a termination notification for this reason.
        if (reasonInfo.getCode() == CallReasonInfo.CODE_LOCAL_CALL_VCC_ON_PROGRESSING) {
            return;
        }

        mCallback.invokeTerminated(this, reasonInfo);
    }

    @VisibleForTesting
    protected void closeSessionWithDelay(final ImsCallSessionImpl session, long delay) {
        if ((session == null) && !mCallDetails.is(CallDetails.CLOSE_PENDING)) {
            return;
        }

        logi("closeSessionWithDelay ::" + delay);

        if (delay <= 0) {
            session.close();
            mCallDetails.clear(CallDetails.CLOSE_PENDING);
        } else {
            getCallHandler().postDelayed(() -> {
                if (session.getMtcCall() != null) {
                    session.close();
                    mCallDetails.clear(CallDetails.CLOSE_PENDING);
                }
            }, delay);
        }
    }

    private void notifySuppServiceForForwardedCall(final boolean isMO) {
        if (mCall == null) {
            return;
        }

        final int cdivCause = mCall.getCallExtraInt(
                Call.EXTRA_CDIV_CAUSE, 0);

        if (cdivCause <= 0) {
            return;
        }

        final String cdivHistory = mCall.getCallExtra(
                Call.EXTRA_CDIV_HISTORY, null);

        if (TextUtils.isEmpty(cdivHistory)) {
            return;
        }

        logi("Forwarded call :: number=" + ImsLog.hiddenString(cdivHistory)
                + ", cause=" + cdivCause);

        getCallHandler().postDelayed(new Runnable() {
            @Override
            public void run() {
                try {
                    if (isMO) {
                        mCallback.invokeSuppServiceReceived(ImsCallSessionImpl.this,
                            ImsSuppServiceUtils.MO.getCallForwarded());
                    } else {
                        mCallback.invokeSuppServiceReceived(ImsCallSessionImpl.this,
                            ImsSuppServiceUtils.MT.getForwardedCall(cdivCause, cdivHistory, null));
                    }
                } catch (Throwable t) {
                    loge("invokeSuppServiceReceived :: " + t.toString());
                }
            }
        }, 100);
    }

    private void postAndRunTask(Runnable task) {
        mCallContext.getExecutor().execute(task);
    }

    private void rejectSessionUpdateAsync(final MtcCall call,
            final int reason, final String dbgLog) {
        postAndRunTask(new Runnable() {
            @Override
            public void run() {
                log(dbgLog);
                call.reject((reason != 0) ? reason :
                        CallReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION);
            }
        });
    }

    private void removeCallConnectionId() {
        ConferenceInfo ci = ConferenceInfoHelper.getConferenceInfo(mCall.getCallId());

        if (ci == null) {
            log("ConferenceInfo is null; " + mCall);
            return;
        }

        List<ConferenceInfo.User> confUsers = ci.getUsers();

        for (ConferenceInfo.User user : confUsers) {
            if (!ConferenceInfo.User.STATUS_DISCONNECTED.equals(user.getStatus())) {
                ImsCallConnectionIds.remove(mCallContext.getSlotId(), user.getCallConnectionId());
            }
        }
    }

    private void removeConferenceUsersOnDisconnected() {

        // If any users are removed, then it will be notified to the application.
        if (ConferenceInfoHelper.removeConferenceUsersByStatus(
                mCall.getCallId(),
                ConferenceInfo.User.STATUS_DISCONNECTED) > 0) {
            notifyCallSessionConferenceStateUpdated();
        }
    }

    private void setCallInfo(ImsCallProfile profile) {
        if (mCallProfile == null) {
            return;
        }

        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_EMERGENCY_CALL,
                mCallProfile.getCallExtraBoolean(ImsCallProfile.EXTRA_EMERGENCY_CALL));
        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE,
                mCallProfile.getCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE));
        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE,
                mCallProfile.getCallExtraBoolean(ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE));

        // Call RAT type : NR/ LTE / IWLAN / UNKNOWN
        ImsCallUtils.setCallExtraIfPresent(mCallProfile,
                ImsCallProfile.EXTRA_CALL_NETWORK_TYPE, ImsCallUtils.VAR_TYPE_INT, profile);

        // Calling number verification status
        int verStat = mCallProfile.getCallerNumberVerificationStatus();
        if (verStat != -1) {
            profile.setCallerNumberVerificationStatus(verStat);
        }

        // Operator specific
        // SKT
        profile.setCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE_AVAIL,
                mCall.getCallExtraBoolean(Call.EXTRA_CONFERENCE_AVAIL, false));

        ImsCallUtils.setCallExtraIfPresent(mCallProfile,
                ImsCallProfile.EXTRA_OIR, ImsCallUtils.VAR_TYPE_INT, profile);
        ImsCallUtils.setCallExtraIfPresent(mCallProfile,
                ImsCallProfile.EXTRA_CNAP, ImsCallUtils.VAR_TYPE_INT, profile);

        ImsCallUtils.setCallExtraIfPresent(mCallProfile,
                ImsCallProfile.EXTRA_OI, ImsCallUtils.VAR_TYPE_STRING, profile);
        ImsCallUtils.setCallExtraIfPresent(mCallProfile,
                ImsCallProfile.EXTRA_CNA, ImsCallUtils.VAR_TYPE_STRING, profile);

        // ATT
        ImsCallUtils.setCallExtraIfPresent(mCallProfile,
                ImsCallUtils.EXTRA_CDIV_CAUSE, ImsCallUtils.VAR_TYPE_INT, profile);
    }

    private void setRttGttInfo(MediaInfo mi, boolean isRttOn) {
        if (isRttOn) {
            setRttInfo(mi, isRttOn);
            return;
        }

        if (isCallFeatureSupported(CF_TTY)) {
            setGttInfo(mi);
        }
    }

    private void setGttInfo(MediaInfo mi) {
        if (!isCallFeatureSupported(CF_TTY) || mCall.isConference()) {
            return;
        }

        if (!ImsCallUtils.isVoiceCall(mCallProfile.getCallType())) {
            // FIXME: should we consider downgrade case?
            return;
        }

        TtyModeTracker tmt = mCallContext.getTtyModeTracker();
        int ttyMode = (tmt != null) ? tmt.getTtyMode() : TtyModeTracker.getTtyModeOff();
        int gttMode = MediaInfo.GTTMODE_INVALID;
        int gttDirection = MediaInfo.DIRECTION_INVALID;

        if (TtyModeTracker.isTtyModeOff(ttyMode)) {
            mLocalCallProfile.setCallExtraInt(MEDIA_GTT_MODE, MediaInfo.GTTMODE_INVALID);

            if (getState() == ImsCallSessionImplBase.State.RENEGOTIATING) {
                gttMode = mProposedCallProfile.getCallExtraInt(
                        MEDIA_GTT_MODE, MediaInfo.GTTMODE_INVALID);
                gttDirection = mProposedCallProfile.getCallExtraInt(
                        ImsCallMediaUtils.MEDIA_TEXT_DIRECTION, MediaInfo.DIRECTION_INVALID);
            } else {
                gttMode = mCall.getMediaInfo().gttMode;
                gttDirection = mCall.getMediaInfo().textDir;
            }

            if (!MtcCallUtils.isGttEnabled(gttMode)) {
                return;
            }
        } else {
            gttMode = ImsCallMediaUtils.getTtyModeFromTelecomToMediaInfo(ttyMode);
            gttDirection = ImsCallMediaUtils.getDirectionFromGTTMode(gttMode);

            mLocalCallProfile.setCallExtraInt(MEDIA_GTT_MODE, gttMode);
        }

        if (mCallDetails.is(CallDetails.ON_HOLDING)) {
            ImsCallMediaUtils.setGttInfo(mi, MediaInfo.DIRECTION_INACTIVE, gttMode);
        } else if (mCallDetails.is(CallDetails.ON_UNHOLDING)) {
            ImsCallMediaUtils.setGttInfo(mi, MediaInfo.DIRECTION_SEND_RECEIVE, gttMode);
        } else {
            // TTY mode is turned on during voice call
            ImsCallMediaUtils.setGttInfo(mi, gttDirection, gttMode);
        }
    }

    private void setRttInfo(MediaInfo mi, boolean isRttOn) {
        if (mCall.isConference()) {
            return;
        }

        int rttDirection = MediaInfo.DIRECTION_SEND_RECEIVE;

        if (!isRttOn) {
            rttDirection = MediaInfo.DIRECTION_INVALID;
        } else {
            if (mCallDetails.is(CallDetails.ON_HOLDING)) {
                if (isCallFeatureSupported(CF_TEXT_HOLD_WITH_INACTIVE) || mCall.isOnHeld()) {
                    rttDirection = MediaInfo.DIRECTION_INACTIVE;
                } else {
                    rttDirection = MediaInfo.DIRECTION_SEND;
                }
            }
            if (mCallDetails.is(CallDetails.ON_UNHOLDING)) {
                if (mCall.isOnHeld()) {
                    if (isCallFeatureSupported(CF_TEXT_HOLD_WITH_INACTIVE)) {
                        rttDirection = MediaInfo.DIRECTION_INACTIVE;
                    } else {
                        rttDirection = MediaInfo.DIRECTION_RECEIVE;
                    }
                }
            }
        }

        log("setRttInfo isRttOn=" + (isRttOn ? "Y" : "N") + ", rttDirection=" + rttDirection);
        ImsCallMediaUtils.setRttInfo(mi, rttDirection, isRttOn);
    }

    @VisibleForTesting
    protected void setTerminationReason(int reason) {
        if (mTerminationReason != ImsReasonInfo.CODE_UNSPECIFIED) {
            return;
        }

        mTerminationReason = reason;

        logi("TerminationReason=" + mTerminationReason);
    }

    protected void setState(int state) {
        if (mState != state) {
            logi("ImsCallSession :: " + mState + " >> " + state);

            mState = state;
        }
    }

    private void startInternal(String callee, ImsCallProfile profile) {
        if (mCall == null) {
            return;
        }

        // FEATURE_CALL_PULL
        String actualCallee = null;
        boolean isRttOn = profile.getMediaProfile().isRttCall();

        MediaInfo mi = ImsCallMediaUtils.createMediaInfoFromMediaProfile(profile.getMediaProfile());

        setRttGttInfo(mi, isRttOn);

        int state = getState();

        if ((state == ImsCallSessionImplBase.State.IDLE)
                || (state == ImsCallSessionImplBase.State.INITIATED)) {
            setState(ImsCallSessionImplBase.State.NEGOTIATING);

            ImsCallApp callApp = (ImsCallApp) mCallContext.getApp();
            mCall.start(
                    ImsCallUtils.getCallTypeFromProfile(profile.getCallType(), isRttOn),
                    callee, actualCallee, mi,
                    ImsCallUtils.createSuppInfoFromCallProfile(mCallContext, profile, callee,
                            callApp.getCallManager().getMtcApp().getMtcEmergencyServiceManager()
                            .getNetworkCountryIso()));

            notifyCallEventForVideoCallSession(IVideoCallSession.EVENT_CALL_INITIATING);
        } else if (state == ImsCallSessionImplBase.State.TERMINATED) {
            notifyCallStartFailedIfAlreadyTerminated();
        }
    }

    private boolean startMoPendingCall(String callee, ImsCallProfile profile) {
        if (mMoPendingCall == null) {
            return false;
        }

        IServiceStateTracker sst = mCallContext.getServiceStateTracker();
        boolean offlineDialing = false;
        boolean emergencyCall = false;

        if (profile.getServiceType() == ImsCallProfile.SERVICE_TYPE_NONE) {
            int callType = ImsCallUtils.getCallTypeFromProfile(
                    profile.getCallType(), profile.getMediaProfile().isRttCall());

            if ((callType == IUMtcCall.CALLTYPE_VOIP || callType == IUMtcCall.CALLTYPE_RTT)
                    && !sst.isServiceRegistered(IUMtcService.SERVICE_VOIP)) {
                mMoPendingCall.start(callee);
                offlineDialing = true;
            } else if ((callType == IUMtcCall.CALLTYPE_VT
                    || callType == IUMtcCall.CALLTYPE_VIDEO_RTT)
                    && !sst.isServiceRegistered(IUMtcService.SERVICE_VT)) {
                mMoPendingCall.start(callee);
                offlineDialing = true;
            }
        } else {
            if (!sst.isServiceRegistered(IUMtcService.SERVICE_EMERGENCY)) {
                mMoPendingCall.start(callee);
                emergencyCall = true;
            }
        }

        if (offlineDialing || emergencyCall) {
            logi("MoPendingCall :: starts - offline="
                    + offlineDialing + ", e-call=" + emergencyCall);
            return true;
        }

        return false;
    }

    private void updateInternal(int callType, ImsStreamMediaProfile profile) {
        if (getState() != ImsCallSessionImplBase.State.ESTABLISHED) {
            // FIXME: event notification
            loge("updateInternal :: Illegal state; callId=" + getCallId() +
                    ", state=" + ImsCallSessionImplBase.State.toString(getState()));

            if (mVideoCallSession.isSessionModificationInProgress()) {
                mVideoCallSession.receiveSessionModifyResponse(
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE, null);
            }
            return;
        } else if (isConferenceTransitionInProgress()) {
            loge("updateInternal :: Illegal state - merge in progress; callId=" + getCallId()
                    + ", state=" + ImsCallSessionImplBase.State.toString(getState()));

            if (mVideoCallSession.isSessionModificationInProgress()) {
                mVideoCallSession.receiveSessionModifyResponse(
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE, null);
            }
            return;
        }

        MediaInfo mediaInfo = ImsCallMediaUtils.createMediaInfoFromMediaProfile(profile);

        setRttGttInfo(mediaInfo, mCallProfile.getMediaProfile().isRttCall());

        setState(ImsCallSessionImplBase.State.RENEGOTIATING);

        mCall.update(ImsCallUtils.getCallTypeFromProfile(
                callType, mCallProfile.getMediaProfile().isRttCall()),
                mediaInfo);

        // FIXME: Stores the proposed media profile?
    }

    private void updateCallExtraForHDVoice(ImsCallProfile profile, MediaInfo mediaInfo) {
        boolean hdVoice = false;
        boolean uhdVoice = false;

        if ((mediaInfo != null) && MtcCallUtils.isAudioEvsCategory(mediaInfo.audioQuality)) {
            hdVoice = MtcCallUtils.isAudioHDQuality(mediaInfo.audioQuality);
            uhdVoice = MtcCallUtils.isAudioUHDQuality(mediaInfo.audioQuality);
        } else {
            hdVoice = ImsCallMediaUtils.isAudioHDQuality(
                    profile.getMediaProfile().getAudioQuality());
            uhdVoice = ImsCallMediaUtils.isAudioUHDQuality(
                    profile.getMediaProfile().getAudioQuality());
        }

        if ("LGU".equals(ImsPrivateProperties.getSimOperator(mCallContext.getSlotId()))) {
            // All the EVS qualities should be an UHD voice based on LGU+ requirement.
            if (((mediaInfo != null) && MtcCallUtils.isAudioEvsCategory(mediaInfo.audioQuality))
                    || ImsCallMediaUtils.isAudioEvsCategory(
                            profile.getMediaProfile().getAudioQuality())) {
                if (!uhdVoice) {
                    log("EVS :: none or hd >> uhd");

                    hdVoice = false;
                    uhdVoice = true;
                }
            }
        }

        if (hdVoice || uhdVoice) {
            mRemoteCallProfile.setCallRestrictCause(ImsCallProfile.CALL_RESTRICT_CAUSE_NONE);
        } else {
            mRemoteCallProfile.setCallRestrictCause(ImsCallProfile.CALL_RESTRICT_CAUSE_HD);
        }

        // Update media information for local call profile
        updateMediaProfile(mLocalCallProfile, mCallProfile);
    }

    private void updateCallProfile(CallInfo callInfo, MediaInfo mediaInfo) {
        log("updateCallProfile");

        if (callInfo != null) {
            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mCallProfile, callInfo);
        }

        if (mediaInfo != null) {
            ImsCallMediaUtils.updateCallProfileFromMediaInfo(
                    mCallContext, mCallProfile, mediaInfo);

            updateCallExtraForHDVoice(mCallProfile, mediaInfo);
        }

        updateCallTypeChangeCapability();
    }

    private boolean updateCallTypeChangeCapability() {
        int callType = mCallProfile.getCallType();
        // Google-Native: enable call switch capability from voice to video
        // Q-OS: enable call switch capability from voice to video
        if (mCallProfile.getCallExtraBoolean(ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE, false)) {
            callType = ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE;
        }

        if (ImsCallUtils.isCallOnNativeAppsAndCountryKR(mCallContext)) {
            if (mCT.getActiveCalls() > 1) {
                logi("CallTypeChangeCapability :: multiple calls");
                callType = mCallProfile.getCallType();
            } else {
                if (mCall.isOnHeld() || mCall.isOnHold()) {
                    logi("CallTypeChangeCapability :: on-hold call");
                    callType = mCallProfile.getCallType();
                }
            }
        }

        int oldLocalCallType = mLocalCallProfile.getCallType();
        mLocalCallProfile.updateCallType(new ImsCallProfile(mLocalCallProfile.getServiceType(),
                callType));
        mRemoteCallProfile.updateCallType(new ImsCallProfile(mRemoteCallProfile.getServiceType(),
                callType));

        if (oldLocalCallType != callType) {
            logi("CallTypeChangeCapability is updated - " + oldLocalCallType + " >> " + callType);
            return true;
        }

        return false;
    }

    private void updateLocalTtyMode() {
        if (!isCallFeatureSupported(CF_TTY)) {
            return;
        }

        if (mCallDetails.is(CallDetails.TTY_MODE_CHANGED)) {
            mCallDetails.clear(CallDetails.TTY_MODE_CHANGED);
            log("updateLocalTtyMode :: TTY_MODE_CHANGED is cleared");
        }

        if (!ImsCallUtils.isVoiceCall(mCallProfile.getCallType())) {
            return;
        }

        if (getState() != ImsCallSessionImplBase.State.ESTABLISHED) {
            return;
        }

        // Checks if the local TTY mode and the settings
        // , and if it's different, then update the current session implicitly.
        int localGttMode = mLocalCallProfile.getCallExtraInt(
                MEDIA_GTT_MODE, MediaInfo.GTTMODE_INVALID);
        int negotiatedGttMode = mCallProfile.getCallExtraInt(
                MEDIA_GTT_MODE, MediaInfo.GTTMODE_INVALID);

        // TTY mode: OFF => ON
        if (!MtcCallUtils.isGttEnabled(localGttMode)
                && !MtcCallUtils.isGttEnabled(negotiatedGttMode)) {
            TtyModeTracker tmt = mCallContext.getTtyModeTracker();
            int ttyMode = (tmt != null) ?
                    tmt.getTtyMode() : TtyModeTracker.getTtyModeOff();

            if (!TtyModeTracker.isTtyModeOff(ttyMode)) {
                try {
                    update(mCallProfile.getCallType(), mCallProfile.getMediaProfile());
                } catch (Throwable t) {
                    loge("updateLocalTtyMode :: " + t.toString());
                }
            }
        }
    }

    private boolean isRttChanged(MediaInfo mi) {
        return (mCallProfile.getMediaProfile().isRttCall()
                != MtcCallUtils.isGttEnabled(mi.gttMode));
    }

    private static void updateMediaProfile(
            ImsCallProfile toProfile, ImsCallProfile fromProfile) {
        if (toProfile == null || fromProfile == null) {
            return;
        }

        if (toProfile.getMediaProfile() == null || fromProfile.getMediaProfile() == null) {
            return;
        }

        toProfile.getMediaProfile().copyFrom(new ImsStreamMediaProfile(
                fromProfile.getMediaProfile().getAudioQuality(),
                toProfile.getMediaProfile().getAudioDirection(),
                fromProfile.getMediaProfile().getVideoQuality(),
                toProfile.getMediaProfile().getVideoDirection(),
                fromProfile.getMediaProfile().getRttMode()));
        ImsCallMediaUtils.updateCallProfileForAudioCodecAttributes(toProfile, fromProfile);
    }

    private static boolean shouldUpdateVideoCapabilityBeforeModifyRequest(
            ImsCallProfile originalProfile, ImsCallProfile proposedProfile) {
        return ImsCallUtils.isVideoCall(proposedProfile.getCallType())
                && !originalProfile.getCallExtraBoolean(
                ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE, false)
                && proposedProfile.getCallExtraBoolean(
                ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE, false);
    }

    private static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }

    private static void loge(String s) {
        ImsLog.e("[GII-IMPL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }

    /**
     * This class describes the detail information of this call.
     */
    @VisibleForTesting
    protected static final class CallDetails {
        public static final int NONE = 0x00000000;
        /**
         * MO call
         */
        public static final int MO = 0x00000001;
        /**
         * The peer session of call merge has this information.
         */
        public static final int MERGED = 0x00000002;
        /**
         * The foreground session is replaced to the conference session
         * after the call merge is completed at the first call merge.
         */
        public static final int MERGED_N_DETACHED = 0x00000004;
        /**
         * If the call merge is completed, then the session is transited to TERMINATED state,
         * but it can't be closed because it is kept in the native layer until termination of
         * the session by the network.
         * If it is set, it means that the session is managed as the terminated session
         * virtually. And, if the session is really terminated by the native, it will be cleared.
         */
        public static final int IMPLICIT_TERMINATED = 0x00000008;
        /**
         * Indicates that the session should be closed after completing the conference operation
         * (call merge or conference extension) regardless of the result.
         * If the session is terminated during conference, it will be set.
         */
        public static final int SESSION_TERMINATED_ON_CONFERENCE = 0x00000010;
        /**
         * It's for a foreground session which is replaced to the conference session
         * after the call merge is completed at the first call merge.
         * If the session is not terminated yet, this attribute is set
         * and it is closed in the callback of terminated event.
         */
        public static final int CLOSE_PENDING = 0x00000020;
        /**
         * Indicates that the MO call is started (in-call state).
         * It's to distinguish between start-failed and terminated event.
         */
        public static final int MO_STARTED = 0x00000040;
        /**
         * Indicates that ECT is in progress.
         */
        public static final int ON_ECT = 0x00000080;
        /**
         * Indicates that HOLD is done by internal operation.
         */
        public static final int IMPLICIT_ON_HOLD = 0x00000100;
        /**
         * Indicates that MO call session is transited to PROGRESSING.
         */
        public static final int MO_PROGRESSING = 0x00000400;

        /**
         * Indicates that session is waiting for the media session to close.
         */
        public static final int WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END = 0x00001000;

        /**
         * To keep the state for each ongoing operations
         */
        public static final int ON_HOLDING = 0x01000000;
        public static final int ON_UNHOLDING = 0x02000000;
        public static final int ON_MERGING = 0x04000000;
        /**
         * Keeps the TTY mode changed.
         * It should be updated when the session is in ESTABLISHED state.
         */
        public static final int TTY_MODE_CHANGED = 0x08000000;
        /**
         * Indicates that RTT is on updating (turning ON/OFF)
         * It should be updated when the session is in ESTABLISHED state.
         */
        public static final int RTT_TURNING_ON = 0x10000000;
        public static final int RTT_TURNING_OFF = 0x20000000;

        /**
         * Indicates that the Telephony is ready to receive callbacks.
         */
        public static final int TELEPHONY_LISTENING = 0x40000000;
        /**
         * Indicates that the call end event is passed to the framework or not.
         */
        public static final int CALL_END_FINISHED = 0x80000000;

        private int mDetails = NONE;

        public CallDetails() {
        }

        public boolean is(int attribute) {
            return (mDetails & attribute) != 0;
        }

        public void clear(int attribute) {
            mDetails &= (~attribute);
        }

        public void set(int attribute) {
            mDetails |= attribute;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();

            sb.append("[ CallDetails: mo=");
            sb.append(is(MO) ? "Y" : "N");
            sb.append(", merged=");
            sb.append(is(MERGED) ? "Y" : "N");
            sb.append(", mergedAndDetached=");
            sb.append(is(MERGED_N_DETACHED) ? "Y" : "N");
            sb.append(", implicitTerminated=");
            sb.append(is(IMPLICIT_TERMINATED) ? "Y" : "N");
            sb.append(", terminatedOnConference=");
            sb.append(is(SESSION_TERMINATED_ON_CONFERENCE) ? "Y" : "N");
            sb.append(", closePending=");
            sb.append(is(CLOSE_PENDING) ? "Y" : "N");
            sb.append(", callEndFinished=");
            sb.append(is(CALL_END_FINISHED) ? "Y" : "N");
            sb.append(", 0x");
            sb.append(Integer.toHexString(mDetails));
            sb.append(" ]");

            return sb.toString();
        }
    }

    private class MoPendingCall extends Handler {
        private static final int EVENT_EMERGENCY_SERVICE_STATE_CHANGED = 101;
        private static final int EVENT_SERVICE_STATE_CHANGED = 102;
        private static final int EVENT_IMS_REG_WAITING_TIMER_EXPIRED = 103;

        private static final int RESULT_NOK = (-1);
        private static final int RESULT_OK = 0;
        private static final int RESULT_IGNORE = 1;

        // 20 seconds
        private static final int IMS_REG_WAITING_TIME = 20 * 1000;

        private int mServiceType = ImsCallProfile.SERVICE_TYPE_EMERGENCY;
        private String mCallee = null;
        private boolean mStartDone = false;
        private boolean mImsRegWaitingTimerRequired = false;

        public MoPendingCall(int serviceType) {
            super(mCallContext.getCallLooper());

            mServiceType = serviceType;

            if ((mServiceType == ImsCallProfile.SERVICE_TYPE_EMERGENCY)
                    && "KR".equals(ImsPrivateProperties.getSimCountry(mCallContext.getSlotId()))) {
                mImsRegWaitingTimerRequired = true;
            }
        }

        public void dispose() {
            if (mServiceStateListener != null) {
                IServiceStateTracker sst = mCallContext.getServiceStateTracker();
                sst.removeListener(mServiceStateListener);
                mServiceStateListener = null;
            }

            stopImsRegWaitingTimer();
        }

        public boolean isIdle() {
            return (mCallee == null);
        }

        public boolean isStartDone() {
            synchronized (mLock) {
                return mStartDone;
            }
        }

        public void start(String callee) {
            mCallee = callee;
            setStartDone(false);

            if (mServiceStateListener == null) {
                IServiceStateTracker sst = mCallContext.getServiceStateTracker();
                mServiceStateListener = new MtcServiceStateListener();
                sst.addListener(mServiceStateListener);
            }

            if (mImsRegWaitingTimerRequired) {
                sendEmptyMessageDelayed(
                        EVENT_IMS_REG_WAITING_TIMER_EXPIRED, IMS_REG_WAITING_TIME);
            }

            notifyCallStartFailedIfAlreadyTerminated();
        }

        @Override
        public void handleMessage(Message msg) {
            log("MoPendingCall :: msg=" + msg.what);

            switch (msg.what) {
                case EVENT_SERVICE_STATE_CHANGED: {
                    if (mServiceType != ImsCallProfile.SERVICE_TYPE_NORMAL) {
                        logi("MoPendingCall :: Service type is not matching, ignore");
                        break;
                    }
                    MtcServiceState ss = (MtcServiceState) msg.obj;
                    logi("MoPendingCall :: " + ss);

                    int result = startCall(ss);

                    if (result == RESULT_NOK) {
                        notifyPendingCallStartFailed();
                    }

                    if (result != RESULT_IGNORE) {
                        dispose();
                    }
                    break;
                }

                case EVENT_EMERGENCY_SERVICE_STATE_CHANGED: {
                    if (mServiceType != ImsCallProfile.SERVICE_TYPE_EMERGENCY) {
                        logi("MoPendingCall :: Service type is not matching, ignore");
                        break;
                    }
                    MtcServiceState ss = (MtcServiceState) msg.obj;
                    logi("MoPendingCall :: " + ss);

                    int result = startEmergencyCall(ss);

                    if (result == RESULT_NOK) {
                        notifyPendingECallStartFailed(ss);
                    }

                    if (result != RESULT_IGNORE) {
                        dispose();
                    }
                    break;
                }

                case EVENT_IMS_REG_WAITING_TIMER_EXPIRED: {
                    logi("MoPendingCall :: IMS-REG waiting timer expired");
                    notifyPendingCallStartFailed();
                    dispose();
                    break;
                }

                default:
                    break;
            }
        }

        private int startCall(MtcServiceState ss) {
            if (ss == null) {
                return RESULT_NOK;
            }

            if (ss.mServiceType == IUMtcService.SERVICE_OPENING) {
                stopImsRegWaitingTimer();
                return RESULT_IGNORE;
            } else if ((ss.mServiceType == IUMtcService.SERVICE_UC)
                    || ((ss.mServiceType == IUMtcService.SERVICE_VT)
                        && ImsCallUtils.isVideoCall(mCallProfile.getCallType()))
                    || ((ss.mServiceType == IUMtcService.SERVICE_VOIP)
                        && ImsCallUtils.isVoiceCall(mCallProfile.getCallType()))) {
                stopImsRegWaitingTimer();

                if (getState() == ImsCallSessionImplBase.State.TERMINATED) {
                    log("Call is already terminated by user");
                    return RESULT_OK;
                }

                startInternal(mCallee, mCallProfile);
                setStartDone(true);
                return RESULT_OK;
            }

            return RESULT_NOK;
        }

        private int startEmergencyCall(MtcServiceState ss) {
            if (ss == null) {
                return RESULT_NOK;
            }

            if (ss.mExtraState == IUMtcService.ES_OPENED) {
                stopImsRegWaitingTimer();

                if (getState() == ImsCallSessionImplBase.State.TERMINATED) {
                    log("Call is already terminated by user");
                    return RESULT_OK;
                }

                ICallLocationPolicy clp = mCallContext.getCallLocationPolicy();

                if ((clp != null) && clp.isLocationRequired(mCallee, mCallProfile)) {
                    mLocationBasedCall = new LocationBasedCall();
                    mLocationBasedCall.start(mCallee);
                    log("Location based E-Call :: waits for the location updates");
                } else {
                    startInternal(mCallee, mCallProfile);
                    setStartDone(true);
                }

                return RESULT_OK;
            } else if (ss.mExtraState == IUMtcService.ES_OPENING) {
                stopImsRegWaitingTimer();
                return RESULT_IGNORE;
            }

            return RESULT_NOK;
        }

        private void notifyPendingCallStartFailed() {
            // Check whether call is already terminated or not, and
            // notify call start failed using the existing terminated reason if it's terminated
            if (notifyCallStartFailedIfAlreadyTerminated()) {
                return;
            }

            // FIXME: video call scenario
            if ((mCall != null) && (mListenerProxy != null)
                    && (getState() != ImsCallSessionImplBase.State.TERMINATED)) {
                // ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE
                int code = CallReasonInfo.CODE_REJECT_INTERNAL_ERROR;
                int extraCode = 0;
                int slotId = mCallContext.getSlotId();

                if (mServiceType == ImsCallProfile.SERVICE_TYPE_EMERGENCY) {
                    code = CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
                    extraCode = CallReasonInfo.EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;

                    // TODO : need to modify this
                    // after emergency domain selection policy is decided.
                    /*if ("VZW") {
                        code = IUMtcCall.Fail_Reason.FAIL_REASON_SESSION_RETRY_RAT;
                    }*/
                } else {
                    String operator = ImsPrivateProperties.getSimOperator(slotId);
                    if (ImsCallUtils.isVideoCall(mCallProfile.getCallType())
                            && ("SKT".equals(operator) || "KT".equals(operator))) {
                        code = CallReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
                        extraCode = CallReasonInfo.EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;
                    }
                }

                mListenerProxy.onCallStartFailed(mCall, new CallReasonInfo(code, extraCode, ""));
            }
        }

        private void notifyPendingECallStartFailed(MtcServiceState ss) {
            // Check whether call is already terminated or not, and
            // notify call start failed using the existing terminated reason if it's terminated
            if (notifyCallStartFailedIfAlreadyTerminated()) {
                return;
            }

            if ((mCall != null) && (mListenerProxy != null)
                    && (getState() != ImsCallSessionImplBase.State.TERMINATED)) {
                int code = CallReasonInfo.CODE_REJECT_INTERNAL_ERROR;
                int extraCode = 0;

                if (ss.mExtraState == IUMtcService.ES_UNAVAILABLE) {
                    switch (ss.mReason) {
                        case IUMtcService.ES_UNAVAILABLE_REASON_DATA_PERMANENTLY_FAILED:
                            code = CallReasonInfo.CODE_EMERGENCY_TEMP_FAILURE;
                            break;
                        case IUMtcService.ES_UNAVAILABLE_REASON_NETWORK_ATTACH_REJECTED:
                            code = CallReasonInfo.CODE_EMERGENCY_PERM_FAILURE;
                            break;
                        default:
                            code = CallReasonInfo.CODE_LOCAL_NOT_REGISTERED;
                            break;
                    }
                }

                mListenerProxy.onCallStartFailed(mCall, new CallReasonInfo(code, extraCode, ""));
            }
        }

        private void setStartDone(boolean done) {
            synchronized (mLock) {
                mStartDone = done;
            }
        }

        private void stopImsRegWaitingTimer() {
            if (mImsRegWaitingTimerRequired) {
                removeMessages(EVENT_IMS_REG_WAITING_TIMER_EXPIRED);
            }
        }
    }

    private class LocationBasedCall implements ImsLocationHelper.Listener {
        private ImsLocationHelper mLocationHelper = null;
        private String mCallee = null;
        private boolean mStartDone;
        private boolean mDisposed = false;

        public LocationBasedCall() {
        }

        public void dispose() {
            mDisposed = true;

            if (mLocationHelper != null) {
                mLocationHelper.dispose();
                mLocationHelper = null;
            }
        }

        public boolean isIdle() {
            return (mCallee == null);
        }

        public boolean isStartDone() {
            synchronized (mLock) {
                return mStartDone;
            }
        }

        public void start(String callee) {
            log("start");

            mCallee = callee;

            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    startLocationUpdate();
                }
            });

            notifyCallStartFailedIfAlreadyTerminated();
        }

        @Override
        public void onLocationUpdateTimeout() {
            onLocationUpdated();
        }

        @Override
        public void onLocationUpdated() {
            if (isStartDone()) {
                logi("Call is already started");
                return;
            }

            Location location = null;

            if (mLocationHelper != null) {
                location = mLocationHelper.getCurrentLocation();

                if (location == null) {
                    location = mLocationHelper.getLastKnownLocationInfo();
                }

                if (location == null) {
                    location = mLocationHelper.getCachedLocation();
                }
            }

            stopLocationUpdate();

            if (location == null) {
                // No Geolocation
                logi("No valid location");
            }

            mCallProfile.setCallExtraBoolean(ImsSuppInfoUtils.EXTRA_GEOLOCATION,
                    (location != null) ? true : false);

            if (checkImsRegistrationAndNotifyError()) {
                return;
            }

            if (getState() == ImsCallSessionImplBase.State.TERMINATED) {
                log("Call is already terminated by user");
                return;
            }

            startInternal(mCallee, mCallProfile);

            synchronized (mLock) {
                mStartDone = true;
            }
        }

        private boolean checkImsRegistrationAndNotifyError() {
            if (!mCallContext.getApp().isConnected(ImsCallProfile.SERVICE_TYPE_NORMAL, 0)) {
                logi("IMS is not registered");
                notifyCallStartFailed(
                        CallReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE, 0);
                return true;
            }

            return false;
        }

        private void stopLocationUpdate() {
            if (mLocationHelper != null) {
                mLocationHelper.stopLocationUpdates();
            }
        }

        private void startLocationUpdate() {
            if (mDisposed) {
                return;
            }

            if (checkImsRegistrationAndNotifyError()) {
                return;
            }

            mLocationHelper = new ImsLocationHelper(mCallContext,
                    this, mCallContext.getCallLooper());

            ICallLocationPolicy clp = mCallContext.getCallLocationPolicy();

            if (clp != null) {
                mLocationHelper.setValidityOption(clp.isPositionInfoRequired(),
                        clp.getValidityPeriod());

                mLocationHelper.startLocationUpdates(
                        clp.getWaitingTimeForLocationFix());
            }
        }

        private void notifyCallStartFailed(int code, int extraCode) {
            if ((mCall != null) && (mListenerProxy != null)) {
                CallReasonInfo callReasonInfo = new CallReasonInfo(code, extraCode, "");
                mListenerProxy.onCallStartFailed(mCall, callReasonInfo);
            }
        }
    }

    private class UsatBasedCall implements Usat.Listener {
        private boolean mStartDone;
        private Usat.CallControlCommand mCcCmd = null;

        public UsatBasedCall() {
        }

        @Override
        public void onCommandResponse(Usat.CommandResponse response) {
            Usat.CallControlCommandResponse cmdRes = (Usat.CallControlCommandResponse) response;
            Usat.CallControlCommand cmd = (Usat.CallControlCommand) cmdRes.getCommand();

            synchronized (mLock) {
                if (!cmd.equals(mCcCmd)) {
                    loge("Command mismatched - " + cmd);
                    return;
                }
            }

            log("onCommandResponse :: cmd=" + cmd + ", result=" + cmdRes.getResult()
                    + ", dialedString=" + ImsLog.hiddenString(cmdRes.getDialedString()));

            if (cmdRes.getResult() == Usat.RESULT_NOT_ALLOWED) {
                notifyCallStartFailed(ImsReasonInfo.CODE_UNSPECIFIED);
                return;
            }

            String dialedString = cmd.getDialedString();

            if (cmdRes.getResult() == Usat.RESULT_ALLOWED_WITH_MODIFICATION) {
                dialedString = cmdRes.getDialedString();
                int reasonCode = getReasonCodeForUsatCallControlType(cmd.getCcType(),
                        cmdRes.getCcType(), cmd.getMediaType(), dialedString);

                if (reasonCode == ImsReasonInfo.CODE_UNSPECIFIED) {
                    reasonCode = getReasonCodeForUsatMediaType(
                            cmd.getMediaType(), cmdRes.getMediaType());
                }

                if (reasonCode != ImsReasonInfo.CODE_UNSPECIFIED) {
                    notifyCallStartFailed(reasonCode);
                    return;
                }

                if (TextUtils.isEmpty(dialedString)) {
                    // Use the original dialed string if this is not present.
                    dialedString = cmd.getDialedString();
                } else {
                    mCallProfile.setCallExtra(ImsCallProfile.EXTRA_OI, dialedString);
                    // This will be used in onCallStarted callback.
                    mRemoteCallProfile.setCallExtraInt(ImsCallProfile.EXTRA_OIR,
                            ImsCallProfile.OIR_PRESENTATION_NOT_RESTRICTED);

                    mCallback.invokeUpdated(ImsCallSessionImpl.this,
                            ImsCallUtils.getSanitizedCallProfileForVideoDirection(mCallProfile));
                }
            }

            // OFFLINE_DIALING
            if (!startMoPendingCall(dialedString, mCallProfile)) {
                // Normal call
                startInternal(dialedString, mCallProfile);
            }

            synchronized (mLock) {
                mStartDone = true;
                mCcCmd = null;
            }
        }

        public void dispose() {
            synchronized (mLock) {
                if (mCcCmd != null) {
                    UsatInterface usat = mCallContext.getUsatInterface();

                    if (usat != null) {
                        usat.cancelCommand(mCcCmd);
                    }

                    mCcCmd = null;
                }
            }
        }

        public boolean isIdle() {
            synchronized (mLock) {
                return (mCcCmd == null);
            }
        }

        public boolean isStartDone() {
            synchronized (mLock) {
                return mStartDone;
            }
        }

        public void start(String callee) {
            log("start");

            UsatInterface usat = mCallContext.getUsatInterface();

            if (usat == null) {
                // OFFLINE_DIALING
                if (!startMoPendingCall(callee, mCallProfile)) {
                    // Normal call
                    startInternal(callee, mCallProfile);
                }
                return;
            }

            int networkType = mCallProfile.getCallExtraInt(
                    ImsCallProfile.EXTRA_CALL_NETWORK_TYPE,
                    TelephonyManager.NETWORK_TYPE_LTE);
            int mediaType = ImsCallUtils.isVoiceCall(mCallProfile.getCallType())
                    ? Usat.MEDIA_TYPE_VOICE : Usat.MEDIA_TYPE_VIDEO;
            int ccType = Usat.CALL_CONTROL_TYPE_MO_CALL;
            int dialString = mCallProfile.getCallExtraInt(ImsCallProfile.EXTRA_DIALSTRING,
                    ImsCallProfile.DIALSTRING_NORMAL);

            if (dialString == ImsCallProfile.DIALSTRING_USSD) {
                ccType = Usat.CALL_CONTROL_TYPE_USSD;
            }

            mCcCmd = usat.createCallControlCommand(
                    ccType, callee, networkType, mediaType, this);

            usat.sendCommand(mCcCmd);
        }

        private void notifyCallStartFailed(int specificReasonCode) {
            if ((mCall != null) && (mListenerProxy != null)) {
                if (specificReasonCode != ImsReasonInfo.CODE_UNSPECIFIED) {
                    setTerminationReason(specificReasonCode);
                }

                CallReasonInfo callReasonInfo = new CallReasonInfo(
                        CallReasonInfo.CODE_UNSPECIFIED, 0, "");

                mListenerProxy.onCallStartFailed(mCall, callReasonInfo);
            }

            synchronized (mLock) {
                mCcCmd = null;
            }
        }
    }

    private static int getReasonCodeForUsatCallControlType(int oldCcType, int newCcType,
            int mediaType, String dialString) {
        if (oldCcType == Usat.CALL_CONTROL_TYPE_MO_CALL) {
            if (mediaType == Usat.MEDIA_TYPE_VIDEO) {
                if (newCcType == Usat.CALL_CONTROL_TYPE_SS) {
                    return ImsReasonInfo.CODE_DIAL_VIDEO_MODIFIED_TO_SS;
                } else if (newCcType == Usat.CALL_CONTROL_TYPE_USSD) {
                    return ImsReasonInfo.CODE_DIAL_VIDEO_MODIFIED_TO_USSD;
                } else if (newCcType == Usat.CALL_CONTROL_TYPE_MO_CALL
                        && ImsUtils.hasWildValueForDialString(dialString)) {
                    return ImsReasonInfo.CODE_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO;
                }
            } else {
                if (newCcType == Usat.CALL_CONTROL_TYPE_SS) {
                    return ImsReasonInfo.CODE_DIAL_MODIFIED_TO_SS;
                } else if (newCcType == Usat.CALL_CONTROL_TYPE_USSD) {
                    return ImsReasonInfo.CODE_DIAL_MODIFIED_TO_USSD;
                } else if (newCcType == Usat.CALL_CONTROL_TYPE_MO_CALL
                        && ImsUtils.hasWildValueForDialString(dialString)) {
                    return ImsReasonInfo.CODE_DIAL_MODIFIED_TO_DIAL;
                }
            }
        } else if (oldCcType == Usat.CALL_CONTROL_TYPE_SS) {
            // Originally, this was dialed as USSD, so it can be handled properly
            // for all the call control types except for the exceptional case.
            if ((newCcType == Usat.CALL_CONTROL_TYPE_SS
                    || newCcType == Usat.CALL_CONTROL_TYPE_MO_CALL)
                            && ImsUtils.hasWildValueForDialString(dialString)) {
                return ImsReasonInfo.CODE_DIAL_MODIFIED_TO_DIAL;
            }
        } else if (oldCcType == Usat.CALL_CONTROL_TYPE_USSD) {
            if (newCcType == Usat.CALL_CONTROL_TYPE_SS) {
                return ImsReasonInfo.CODE_DIAL_MODIFIED_TO_SS;
            } else if (newCcType == Usat.CALL_CONTROL_TYPE_MO_CALL
                    && ImsUtils.hasWildValueForDialString(dialString)) {
                return ImsReasonInfo.CODE_DIAL_MODIFIED_TO_DIAL;
            }
        }

        return ImsReasonInfo.CODE_UNSPECIFIED;
    }

    private static int getReasonCodeForUsatMediaType(int oldMediaType, int newMediaType) {
        if (oldMediaType == Usat.MEDIA_TYPE_VOICE
                && newMediaType == Usat.MEDIA_TYPE_VIDEO) {
            return ImsReasonInfo.CODE_DIAL_MODIFIED_TO_DIAL_VIDEO;
        } else if (oldMediaType == Usat.MEDIA_TYPE_VIDEO
                && newMediaType == Usat.MEDIA_TYPE_VOICE) {
            return ImsReasonInfo.CODE_DIAL_VIDEO_MODIFIED_TO_DIAL;
        }

        return ImsReasonInfo.CODE_UNSPECIFIED;
    }

    private class TtyModeListenerProxy implements TtyModeTracker.Listener {
        @Override
        public void onTtyModeChanged(int newTtyMode) {
            log("onTtyModeChanged :: ttyMode=" + newTtyMode);

            if (!TtyModeTracker.isTtyModeOff(newTtyMode)) {
                if (!ImsCallUtils.isVoiceCall(mCallProfile.getCallType())) {
                    return;
                }

                if (mCall == null) {
                    return;
                }

                MediaInfo mi = mCall.getMediaInfo();

                if (MtcCallUtils.isGttEnabled(mi.gttMode)) {
                    log("TTY mode is already enabled");
                    return;
                }

                int state = getState();

                if ((state == ImsCallSessionImplBase.State.NEGOTIATING)
                        || (state == ImsCallSessionImplBase.State.ESTABLISHING)
                        || (state == ImsCallSessionImplBase.State.RENEGOTIATING)
                        || (state == ImsCallSessionImplBase.State.REESTABLISHING)) {
                    // Postpone the session update if it can't be handled in the moment.
                    mCallDetails.set(CallDetails.TTY_MODE_CHANGED);
                    log("Session update for TTY mode changed is postponed");
                } else {
                    updateLocalTtyMode();
                }
            }
        }
    }

    private class MtcServiceStateListener implements IServiceStateTracker.Listener {
        @Override
        public void onEmergencyServiceStateChanged(MtcServiceState serviceState) {
            Message.obtain(mMoPendingCall, MoPendingCall.EVENT_EMERGENCY_SERVICE_STATE_CHANGED,
                    serviceState).sendToTarget();
        }

        @Override
        public void onNormalServiceStateChanged(MtcServiceState serviceState) {
            Message.obtain(mMoPendingCall, MoPendingCall.EVENT_SERVICE_STATE_CHANGED,
                    serviceState).sendToTarget();
        }
    }

    private int getApnType(ImsCallProfile profile) {
        if (profile.getServiceType() == ImsCallProfile.SERVICE_TYPE_EMERGENCY) {
            //To-Do:- Need to find the way Emergency call Over VoWiFi
            if (!ImsCallUtils.isEmergencyPdnUsedForEmergencyCallViaWfc(mCallContext)) {
                return EApnType.IMS.getType();
            }
            return EApnType.EMERGENCY.getType();
        } else {
            return EApnType.IMS.getType();
        }
    }

    protected class EmergencyCallFailureListener implements MtcCall.IEmergencyCallFailureListener {
        @Override
        public boolean onEmergencyCallFailedByAlreadyOpenedServiceClosed() {
            if (mMoPendingCall == null) {
                log("There is no MoPendingCall.");
                return false;
            }

            if (mCall == null) {
                log("There is no MtcCall.");
                return false;
            }

            IServiceStateTracker sst = mCallContext.getServiceStateTracker();
            if (sst.isServiceRegistered(IUMtcService.SERVICE_EMERGENCY)) {
                log("The emergency service is already registered.");
                return false;
            }

            ImsCallApp callApp = (ImsCallApp) mCallContext.getApp();
            callApp.getCallManager().getMtcApp().openEmergencyService(
                    mCall, EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY);
            setState(ImsCallSessionImplBase.State.IDLE);
            mMoPendingCall.start(mCall.getRemoteNumber());

            return true;
        }
    }

    protected class MtcCallListenerProxy extends MtcCall.Listener {
        @Override
        public void onCallProxyHold(MtcCall call) {
            if (!call.equals(mCall)) {
                return;
            }

            log("onCallProxyHold :: call=" + call);

            //setState(ImsCallSessionImplBase.State.RENEGOTIATING);
        }

        @Override
        public void onCallProxyResume(MtcCall call) {
            if (!call.equals(mCall)) {
                return;
            }

            log("onCallProxyResume :: call=" + call);

            //setState(ImsCallSessionImplBase.State.RENEGOTIATING);
        }

        @Override
        public void onCallInitiating(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, int ratType) {
            if (!call.equals(mCall)) {
                return;
            }

            mCallProfile.setCallExtraInt(ImsCallProfile.EXTRA_CALL_NETWORK_TYPE, ratType);

            ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                    mCallContext, callInfo, mediaInfo);
            setCallInfo(profile);

            mCallback.invokeInitiating(ImsCallSessionImpl.this, profile);
        }

        @Override
        public void onCallProgressing(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            if (mCallDetails.is(CallDetails.MO)) {
                mCallDetails.set(CallDetails.MO_PROGRESSING);
            }

            setState(ImsCallSessionImplBase.State.ESTABLISHING);

            ImsCallUtils.updateCallProfileOnSessionProgressing(
                    mCallContext, mCallProfile, callInfo, suppInfo);
            ImsCallMediaUtils.updateCallProfileFromMediaInfo(
                    mCallContext, mCallProfile, mediaInfo);
            updateCallExtraForHDVoice(mCallProfile, mediaInfo);

            // FIXME: MTC_NEW_IF - it should be changed to ImsCallProfile
            mCallback.invokeProgressing(ImsCallSessionImpl.this,
                    ImsCallMediaUtils.createMediaProfileFromMediaInfo(mediaInfo));

            // Check and notify the application that call is forwarded
            notifySuppServiceForForwardedCall(true);

            if (MtcCallUtils.isCallWaitingEnabled(suppInfo)) {
                mCallback.invokeSuppServiceReceived(ImsCallSessionImpl.this,
                        ImsSuppServiceUtils.MO.getCallIsWaiting());
            }

            notifyE2eeCallInfo(suppInfo, mCall.getRemoteNumber());
        }

        @Override
        public void onCallStarted(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo,
                SuppInfo suppInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            setState(ImsCallSessionImplBase.State.ESTABLISHED);

            // FIXME : how to remove the existing values?
            ImsCallUtils.updateCallProfileForEmergency(mCallProfile, callInfo);
            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mCallProfile, callInfo);
            ImsCallUtils.updateCallProfileOnSessionStarted(mCallProfile, suppInfo);
            ImsCallUtils.updateCallProfileFromSuppInfoExtension(
                    mCallContext, mCallProfile, suppInfo);
            ImsCallMediaUtils.updateCallProfileFromMediaInfo(
                    mCallContext, mCallProfile, mediaInfo);
            updateCallExtraForHDVoice(mCallProfile, mediaInfo);

            // IMS-LAMPLITE: enable call switch capability from voice to video
            updateCallProfile(null, null);

            // CALL_CONNECTION_ID
            int ccId = getCallConnectionId();

            if (ccId > 0) {
                mCall.setCallConnectionId(ccId);
            }

            // MO call & OIR is overridden by USAT call control
            SuppService ss = suppInfo.getService(SuppInfo.SUPP_TYPE_TIP);
            if (mCallDetails.is(CallDetails.MO)
                    && (ss != null) && (ss.intValue == SuppInfo.TIP_NONE)) {
                ImsCallUtils.setCallExtraIfPresent(mRemoteCallProfile,
                        ImsCallProfile.EXTRA_OIR, ImsCallUtils.VAR_TYPE_INT, mCallProfile);
            }

            ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                    mCallContext, callInfo, mediaInfo);

            setCallInfo(profile);

            mCallback.invokeStarted(ImsCallSessionImpl.this, profile);

            // Handle TTY mode related operations
            onTtyModeReceived(mediaInfo.gttMode, true);
            updateLocalTtyMode();

            if (mCallDetails.is(CallDetails.MO)) {
                mCallDetails.set(CallDetails.MO_STARTED);
            }

            notifyCallEventForVideoCallSession(IVideoCallSession.EVENT_CALL_ESTABLISHED);

            notifyE2eeCallInfo(suppInfo, mCall.getRemoteNumber());

            // FIXME: If the call setup failure is not a re-dial case,
            // we need to call the callSessionTerminated() to adapt the original Android Framework.
        }

        @Override
        public void onCallStartFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            if (mCallDetails.is(CallDetails.CALL_END_FINISHED)) {
                log("Ignore the duplicated start-failed event");
                return;
            }

            if (call.isOnPreIncoming()) {
                log("closeInternal on ON_PRE_INCOMING");
                setState(ImsCallSessionImplBase.State.TERMINATED);
                closeInternal(ImsCallSessionImpl.this);
                return;
            }

            ImsCallUtils.refineCallReasonInfoForCode(mCallContext, mCallProfile, callReasonInfo);

            if (!mCallDetails.is(CallDetails.TELEPHONY_LISTENING)) {
                mImmediateCallEndReason = ImsCallUtils.createReasonInfo(
                        callReasonInfo, ImsCallUtils.FLAG_REASON_INFO_ALL);
                setState(ImsCallSessionImplBase.State.TERMINATED);
                return;
            }

            if (ImsCallUtils.isAlternateEmergencyCall(callReasonInfo)) {
                ImsCallUtils.setSosUrnFromCallReasonInfo(callReasonInfo, mCallProfile);
                mCallback.invokeUpdated(ImsCallSessionImpl.this,
                        ImsCallUtils.getSanitizedCallProfileForVideoDirection(mCallProfile));
                mCallDetails.clear(CallDetails.MO_PROGRESSING);
            }

            setState(ImsCallSessionImplBase.State.TERMINATED);

            final ImsReasonInfo reasonInfo = ImsCallUtils.createReasonInfo(
                    getTerminationReason(callReasonInfo.mCode),
                    isTerminationReasonPresent() ?
                    ImsReasonInfo.CODE_UNSPECIFIED : callReasonInfo.mExtraCode,
                    callReasonInfo.mExtraMessage,
                    isTerminationReasonPresent()
                    ? ImsCallUtils.FLAG_REASON_INFO_NONE : ImsCallUtils.FLAG_REASON_INFO_ALL, 0);

            notifyCallStartFailed(reasonInfo);
            if (MtcCallUtils.isOutgoingCallsBarred(callReasonInfo)) {
                mCallback.invokeSuppServiceReceived(ImsCallSessionImpl.this,
                        ImsSuppServiceUtils.MO.getOutgoingCallsBarred());
            }

            notifyCallEventForVideoCallSession(IVideoCallSession.EVENT_CALL_TERMINATED);
        }

        @Override
        public void onCallTerminated(MtcCall call, CallReasonInfo callReasonInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            log("onCallTerminated(), call=" + ImsCallSessionImpl.this);

            if (mCallDetails.is(CallDetails.CLOSE_PENDING)
                    && mCallDetails.is(CallDetails.IMPLICIT_TERMINATED)) {
                mCallDetails.clear(CallDetails.CLOSE_PENDING);
                mCallDetails.clear(CallDetails.IMPLICIT_TERMINATED);
                setState(ImsCallSessionImplBase.State.TERMINATED);
                log("closeInternal on CLOSE_PENDING");
                closeInternal(ImsCallSessionImpl.this);
                return;
            }

            mCallDetails.clear(CallDetails.IMPLICIT_TERMINATED);

            // FIXME : close MtcCall according to the reason (call merge); ignore the terminated
            if (checkAndHandleConferenceOnCallTerminated(callReasonInfo)) {
                return;
            }

            checkAndHandleTransferOnCallTerminated(callReasonInfo);

            if (getState() == ImsCallSessionImplBase.State.TERMINATED) {
                log("Call is already terminated; call=" + ImsCallSessionImpl.this);
                return;
            }

            if (mCallDetails.is(CallDetails.MO) && !mCallDetails.is(CallDetails.MO_STARTED)) {
                logi("Callback-Replacement :: onCallTerminated >> onCallStartFailed");
                onCallStartFailed(call, callReasonInfo);
                return;
            }

            setState(ImsCallSessionImplBase.State.TERMINATED);
            checkAndNotifyCallOperationFailureOnCallTerminated(call, callReasonInfo);

            String ccid = mCall.getCallId();

            if (mCallDetails.is(CallDetails.CALL_END_FINISHED)) {
                log("Ignore the duplicated terminated event");
            } else if (mCallDetails.is(CallDetails.ON_MERGING)) {
            /* When one of the conference participant is terminated {@link #onCallMergeFailed}
             * is called that time we should not wait for {@link #onAudioSessionClosed} and
             * immediately send {@link #invokeTerminated} so FW will {@link #close} the new
             * conference session.
             */
                notifyCallTerminated(callReasonInfo.mCode, callReasonInfo.mExtraCode,
                        callReasonInfo.mExtraMessage);
            } else {
                waitOrNotifyCallTerminated(callReasonInfo.mCode, callReasonInfo.mExtraCode,
                        callReasonInfo.mExtraMessage);
            }

            // Notify all the users that the conference call is terminated.
            if (isMultiparty()) {
                ConferenceInfoHelper.updateAndNotifyDisconnectedForAllConferenceUsers(ccid);
            }

            notifyCallEventForVideoCallSession(IVideoCallSession.EVENT_CALL_TERMINATED);
        }

        @Override
        public void onCallHeld(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            if (mCallDetails.is(CallDetails.ON_ECT)) {
                // Hold operation is triggered by ECT
                postAndRunTask(new Runnable() {
                    @Override
                    public void run() {
                        MtcCall transferRequestedCall = mTransferRequestedSession.mCall;
                        logi("transfer :: " + transferRequestedCall);
                        if (mTransferTargetNumber != null) {
                            transferRequestedCall.transfer(mTransferTargetNumber);
                        } else {
                            // Here we can pass the Session received from FW
                            transferRequestedCall.transfer(null);
                        }
                    }
                });

                if (!mCallDetails.is(CallDetails.ON_HOLDING)) {
                    mCallDetails.set(CallDetails.IMPLICIT_ON_HOLD);
                    return;
                }
            }

            int oldState = getState();

            if ((oldState == ImsCallSessionImplBase.State.RENEGOTIATING)
                    || (oldState == ImsCallSessionImplBase.State.REESTABLISHING)) {
                setState(ImsCallSessionImplBase.State.ESTABLISHED);
            }

            boolean invokeUpdatedRequired = false;

            if (isRttChanged(mediaInfo)) {
                invokeUpdatedRequired = true;
            }

            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mCallProfile, callInfo);
            ImsCallMediaUtils.updateCallProfileFromMediaInfo(
                    mCallContext, mCallProfile, mediaInfo);
            updateCallExtraForHDVoice(mCallProfile, mediaInfo);

            if (isConferenceTransitionInProgress()) {
                // Don't invoke the callback method if the call command is in progress
                return;
            }

            mCallDetails.clear(CallDetails.ON_HOLDING);

            if (ImsCallUtils.isCallOnNativeAppsAndCountryKR(mCallContext)) {
                updateCallTypeChangeCapability();
            }

            ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                    mCallContext, callInfo, mediaInfo);

            setCallInfo(profile);

            mCallback.invokeHeld(ImsCallSessionImpl.this, profile);

            if (invokeUpdatedRequired) {
                mCallback.invokeUpdated(ImsCallSessionImpl.this, profile);
            }
        }

        @Override
        public void onCallHoldFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            if (mCallDetails.is(CallDetails.ON_ECT)) {
                // Hold operation is triggered by ECT
                mCallDetails.clear(CallDetails.ON_ECT);
                mCallDetails.clear(CallDetails.IMPLICIT_ON_HOLD);

                final ImsReasonInfo reasonInfo = ImsCallUtils.createReasonInfo(
                        callReasonInfo.mCode, ImsReasonInfo.CODE_UNSPECIFIED,
                        callReasonInfo.mExtraMessage, ImsCallUtils.FLAG_REASON_INFO_ALL, 0);

                if (!mCallDetails.is(CallDetails.ON_HOLDING)) {
                    if (!mTransferRequestedSession.mIsEctConfirmationRequired) {
                        log("CallTransfer: Confirmation not required");
                    } else {
                        mTransferRequestedSession.mCallback.invokeCallSessionTransferFailed(
                                mTransferRequestedSession, reasonInfo);
                    }
                    return;
                } else {
                    if (!mTransferRequestedSession.mIsEctConfirmationRequired) {
                        log("CallTransfer: Confirmation not required");
                        clearTransferRequestedSessionEctDetails();
                    } else {
                        postAndRunTask(new Runnable() {
                            @Override
                            public void run() {
                                mTransferRequestedSession.mCallback.invokeCallSessionTransferFailed(
                                        mTransferRequestedSession, reasonInfo);
                                clearTransferRequestedSessionEctDetails();
                            }
                        });
                    }
                }
            }

            int oldState = getState();

            if ((oldState == ImsCallSessionImplBase.State.RENEGOTIATING)
                    || (oldState == ImsCallSessionImplBase.State.REESTABLISHING)) {
                setState(ImsCallSessionImplBase.State.ESTABLISHED);
            }

            if (isConferenceTransitionInProgress()) {
                // Don't invoke the callback method if the call command is in progress
                return;
            }

            mCallDetails.clear(CallDetails.ON_HOLDING);

            int preferredCode = 0;

            if (getTerminationReason(0) == ImsReasonInfo.CODE_USER_TERMINATED) {
                preferredCode = ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED;
            } else if ((getTerminationReason(0) != 0) || mCall.isTerminated()) {
                preferredCode = ImsReasonInfo.CODE_SUPP_SVC_FAILED;
            }

            ImsReasonInfo reasonInfo = ImsCallUtils.createReasonInfo(
                    callReasonInfo.mCode, ImsReasonInfo.CODE_UNSPECIFIED,
                    callReasonInfo.mExtraMessage, ImsCallUtils.FLAG_REASON_INFO_ALL, preferredCode);

            mCallback.invokeHoldFailed(ImsCallSessionImpl.this, reasonInfo);
        }

        @Override
        public void onCallHoldReceived(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            boolean isOnHeld = false;
            boolean isLocalHoldToneEnforced = MtcCallUtils.isLocalHoldToneEnforced(suppInfo);

            if (isLocalHoldToneEnforced) {
                // To avoid a multiple toast pop-up for call hold
                int direction = mCallProfile.getMediaProfile().getAudioDirection();

                if (direction == ImsStreamMediaProfile.DIRECTION_INACTIVE) {
                    isLocalHoldToneEnforced = false;
                } else if (direction == ImsStreamMediaProfile.DIRECTION_RECEIVE) {
                    if (mediaInfo.audioDir == MediaInfo.DIRECTION_INACTIVE) {
                        isLocalHoldToneEnforced = false;
                    } else if (mediaInfo.audioDir == MediaInfo.DIRECTION_RECEIVE) {
                        isOnHeld = true;
                    }
                }
            }

            boolean invokeUpdatedRequired = false;

            if (isRttChanged(mediaInfo)) {
                invokeUpdatedRequired = true;
            }

            boolean conferenceExtendedByRemote = isConferenceExtended(callInfo);
            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mCallProfile, callInfo);
            ImsCallMediaUtils.updateCallProfileFromMediaInfo(
                    mCallContext, mCallProfile, mediaInfo);
            updateCallExtraForHDVoice(mCallProfile, mediaInfo);

            if (ImsCallUtils.isCallOnNativeAppsAndCountryKR(mCallContext)) {
                if (updateCallTypeChangeCapability()) {
                    invokeUpdatedRequired = true;
                }
            }

            ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                    mCallContext, callInfo, mediaInfo);

            int audioDirection = profile.getMediaProfile().getAudioDirection();

            setCallInfo(profile);

            // __IMS_TEST_MODE__
            if (com.android.imsstack.test.ImsTestMode.getInstance().getTestMode(
                    mCallContext.getSlotId()).isLocalHoldToneEnabled()) {
                log("LocalHoldTone :: testmode");
                audioDirection = ImsStreamMediaProfile.DIRECTION_INACTIVE;
            }

            if (isLocalHoldToneEnforced) {
                logi("LocalHoldTone :: enforced");
                audioDirection = ImsStreamMediaProfile.DIRECTION_INACTIVE;
            }

            ImsStreamMediaProfile tMediaProfile = profile.getMediaProfile();
            ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile(
                    tMediaProfile.getAudioQuality(), audioDirection,
                    tMediaProfile.getVideoQuality(), tMediaProfile.getVideoDirection(),
                    tMediaProfile.getRttMode());

            profile.getMediaProfile().copyFrom(mediaProfile);
            mCallback.invokeHoldReceived(ImsCallSessionImpl.this, profile);

            // Toast pop-up for call hold
            if (!isOnHeld) {
                mCallback.invokeSuppServiceReceived(ImsCallSessionImpl.this,
                        ImsSuppServiceUtils.MT.getCallOnHold());
            }

            if (invokeUpdatedRequired) {
                mCallback.invokeUpdated(ImsCallSessionImpl.this, profile);
            }

            notifyCallSessionMultipartyStateChanged(conferenceExtendedByRemote);

            // FIXME: checks if audio direction needs to be restored
            // when local hold tone is enforced.
        }

        @Override
        public void onCallResumed(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            if (mCallDetails.is(CallDetails.ON_ECT)
                    && mCallDetails.is(CallDetails.IMPLICIT_ON_HOLD)) {
                mCallDetails.clear(CallDetails.ON_ECT);
                mCallDetails.clear(CallDetails.IMPLICIT_ON_HOLD);

                if (mOperationFailReason != null) {
                    if (!mTransferRequestedSession.mIsEctConfirmationRequired) {
                        log("CallTransfer: Confirmation not required");
                    } else {
                        mTransferRequestedSession.mCallback.invokeCallSessionTransferFailed(
                                mTransferRequestedSession, mOperationFailReason);
                    }

                    mOperationFailReason = null;
                }

                clearTransferRequestedSessionEctDetails();
                return;
            }

            // This callback can be invoked when UpdateResume is received
            // and accepted during the voice / video call.
            if (isCallFeatureSupported(CF_INCOMING_RESUME_EVENT)
                    && !mCallDetails.is(CallDetails.ON_UNHOLDING)) {
                logi("Ignore onCallResumed() by IncomingResume");
                // FIXME: is this call required?
                // onCallUpdated(call, callInfo, mediaInfo, suppInfo);
                return;
            }

            int oldState = getState();

            if ((oldState == ImsCallSessionImplBase.State.RENEGOTIATING)
                    || (oldState == ImsCallSessionImplBase.State.REESTABLISHING)) {
                setState(ImsCallSessionImplBase.State.ESTABLISHED);
            }

            boolean invokeUpdatedRequired = false;

            if (isRttChanged(mediaInfo)) {
                invokeUpdatedRequired = true;
            }

            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mCallProfile, callInfo);
            ImsCallMediaUtils.updateCallProfileFromMediaInfo(
                    mCallContext, mCallProfile, mediaInfo);
            updateCallExtraForHDVoice(mCallProfile, mediaInfo);

            if (isConferenceTransitionInProgress()) {
                // Don't invoke the callback method if the call command is in progress
                return;
            }

            mCallDetails.clear(CallDetails.ON_UNHOLDING);

            if (ImsCallUtils.isCallOnNativeAppsAndCountryKR(mCallContext)) {
                updateCallTypeChangeCapability();
            }

            ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                    mCallContext, callInfo, mediaInfo);

            setCallInfo(profile);

            mCallback.invokeResumed(ImsCallSessionImpl.this, profile);

            if (invokeUpdatedRequired) {
                mCallback.invokeUpdated(ImsCallSessionImpl.this, profile);
            }
        }

        @Override
        public void onCallResumeFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            if (mCallDetails.is(CallDetails.ON_ECT)
                    && mCallDetails.is(CallDetails.IMPLICIT_ON_HOLD)) {
                // TODO(b/451717299): mTransferRequestedSession.mCallback.
                // invokeCallSessionTransferFailed is required here?
                getCallHandler().postDelayed(
                        () -> mCall.resume(MtcCallUtils.createUnholdMedia(
                            mCall.getCallInfo(), mCall.getMediaInfo(),
                            isCallFeatureSupported(CF_VIDEO_HOLD_WITH_INACTIVE))),
                            TRANSFER_RESUME_RECOVERY_DELAY_TIME);
                return;
            }

            int oldState = getState();

            if ((oldState == ImsCallSessionImplBase.State.RENEGOTIATING)
                    || (oldState == ImsCallSessionImplBase.State.REESTABLISHING)) {
                setState(ImsCallSessionImplBase.State.ESTABLISHED);
            }

            if (isConferenceTransitionInProgress()) {
                // Don't invoke the callback method if the call command is in progress
                return;
            }

            mCallDetails.clear(CallDetails.ON_UNHOLDING);

            int preferredCode = 0;

            if (getTerminationReason(0) == ImsReasonInfo.CODE_USER_TERMINATED) {
                preferredCode = ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED;
            } else if ((getTerminationReason(0) != 0) || mCall.isTerminated()) {
                preferredCode = ImsReasonInfo.CODE_SUPP_SVC_FAILED;
            }

            ImsReasonInfo reasonInfo = ImsCallUtils.createReasonInfo(
                    callReasonInfo.mCode, ImsReasonInfo.CODE_UNSPECIFIED,
                    callReasonInfo.mExtraMessage, ImsCallUtils.FLAG_REASON_INFO_ALL, preferredCode);

            mCallback.invokeResumeFailed(ImsCallSessionImpl.this, reasonInfo);
        }

        @Override
        public void onCallResumeReceived(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            boolean invokeUpdatedRequired = false;

            if (isRttChanged(mediaInfo)) {
                invokeUpdatedRequired = true;
            }

            boolean conferenceExtendedByRemote = isConferenceExtended(callInfo);
            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mCallProfile, callInfo);
            ImsCallMediaUtils.updateCallProfileFromMediaInfo(
                    mCallContext, mCallProfile, mediaInfo);
            updateCallExtraForHDVoice(mCallProfile, mediaInfo);

            if (ImsCallUtils.isCallOnNativeAppsAndCountryKR(mCallContext)) {
                if (updateCallTypeChangeCapability()) {
                    invokeUpdatedRequired = true;
                }
            }

            ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                    mCallContext, callInfo, mediaInfo);

            setCallInfo(profile);

            mCallback.invokeResumeReceived(ImsCallSessionImpl.this, profile);

            // Toast pop-up for call resume
            mCallback.invokeSuppServiceReceived(ImsCallSessionImpl.this,
                    ImsSuppServiceUtils.MT.getCallRetrieved());

            if (invokeUpdatedRequired) {
                mCallback.invokeUpdated(ImsCallSessionImpl.this, profile);
            }

            notifyCallSessionMultipartyStateChanged(conferenceExtendedByRemote);
        }

        @Override
        public void onCallAutoUpdated(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            boolean conferenceExtendedByRemote = isConferenceExtended(callInfo);
            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mCallProfile, callInfo);
            ImsCallMediaUtils.updateCallProfileFromMediaInfo(
                    mCallContext, mCallProfile, mediaInfo);
            updateCallExtraForHDVoice(mCallProfile, mediaInfo);

            ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                    mCallContext, callInfo, mediaInfo);

            setCallInfo(profile);

            // FIXME: need to verify it.
            mCallback.invokeUpdated(ImsCallSessionImpl.this, profile);

            notifyCallSessionMultipartyStateChanged(conferenceExtendedByRemote);

            // Handle TTY mode related operations
            onTtyModeReceived(mediaInfo.gttMode, false);
            updateLocalTtyMode();
        }

        @Override
        public void onCallUpdated(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            // VIDEO_CALL_HOLD
            if (ImsCallUtils.isVideoCall(mCallProfile.getCallType())) {
                if (mCallDetails.is(CallDetails.ON_HOLDING)) {
                    onCallHeld(call, callInfo, mediaInfo, suppInfo);
                    return;
                } else if (mCallDetails.is(CallDetails.ON_UNHOLDING)) {
                    onCallResumed(call, callInfo, mediaInfo, suppInfo);
                    return;
                }
            }

            boolean conferenceExtendedByRemote = isConferenceExtended(callInfo);
            onRttChanged(MtcCallUtils.isGttEnabled(mediaInfo.gttMode),
                    MtcCallUtils.isGttEnabled(mCall.getMediaInfo().gttMode));

            int oldState = getState();

            if ((oldState == ImsCallSessionImplBase.State.RENEGOTIATING)
                    || (oldState == ImsCallSessionImplBase.State.REESTABLISHING)) {
                setState(ImsCallSessionImplBase.State.ESTABLISHED);
            }

            updateCallProfile(callInfo, mediaInfo);

            if ((oldState == ImsCallSessionImplBase.State.RENEGOTIATING)
                    && mVideoCallSession.isSessionModificationInProgress()) {
                mVideoCallSession.receiveSessionModifyResponse(-1, mediaInfo);
            } else if (mVideoCallSession.isSessionModificationFinalizing()) {
                // Case: accepts the incoming session modification
                mVideoCallSession.finalizeSessionModification();
            }

            clearProposedCallProfile();
            ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                    mCallContext, callInfo, mediaInfo);

            setCallInfo(profile);

            mCallback.invokeUpdated(ImsCallSessionImpl.this, profile);

            notifyCallSessionMultipartyStateChanged(conferenceExtendedByRemote);

            // Handle TTY mode related operations
            onTtyModeReceived(mediaInfo.gttMode, false);
            updateLocalTtyMode();
        }

        @Override
        public void onCallUpdateFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            // VIDEO_CALL_HOLD
            if (ImsCallUtils.isVideoCall(mCallProfile.getCallType())) {
                if (mCallDetails.is(CallDetails.ON_HOLDING)) {
                    onCallHoldFailed(call, callReasonInfo);
                    return;
                } else if (mCallDetails.is(CallDetails.ON_UNHOLDING)) {
                    onCallResumeFailed(call, callReasonInfo);
                    return;
                }
            }

            // Result of RTT_MODIFY_REQUEST
            if (mCallDetails.is(CallDetails.RTT_TURNING_ON)) {
                mCallDetails.clear(CallDetails.RTT_TURNING_ON);
                log("onCallUpdateFailed :: RTT_TURNING_ON is cleared");

                mCallback.invokeRttModifyResponseReceived(ImsCallSessionImpl.this,
                        RttModifyStatus.SESSION_MODIFY_REQUEST_FAIL);
            } else if (mCallDetails.is(CallDetails.RTT_TURNING_OFF)) {
                mCallDetails.clear(CallDetails.RTT_TURNING_OFF);
                log("onCallUpdateFailed :: RTT_TURNING_OFF is cleared");

                mCallback.invokeRttModifyResponseReceived(ImsCallSessionImpl.this,
                        RttModifyStatus.SESSION_MODIFY_REQUEST_FAIL);
            }

            int oldState = getState();

            if ((oldState == ImsCallSessionImplBase.State.RENEGOTIATING)
                    || (oldState == ImsCallSessionImplBase.State.REESTABLISHING)) {
                setState(ImsCallSessionImplBase.State.ESTABLISHED);
            }

            if ((oldState == ImsCallSessionImplBase.State.RENEGOTIATING)
                    && mVideoCallSession.isSessionModificationInProgress()) {
                mVideoCallSession.receiveSessionModifyResponse(
                        (callReasonInfo.mExtraCode == 603) ? ImsReasonInfo.CODE_SIP_USER_REJECTED :
                            ImsCallUtils.getReasonFromMTC(callReasonInfo.mCode),
                        null);
                clearProposedCallProfile();
                return;
            } else if (mVideoCallSession.isSessionModificationFinalizing()) {
                // Case: rejects the incoming session modification
                mVideoCallSession.finalizeSessionModification();
                clearProposedCallProfile();
                return;
            }

            clearProposedCallProfile();

            if (mLocalCallProfile.getCallExtraBoolean(EXTRA_CALL_CONTROLLED_BY_IMS, false)) {
                // Ignore this event because it's not triggered by the IMS framework.
                log("Ignore this event because it is automatically rejected by IMS");
                mLocalCallProfile.setCallExtraBoolean(EXTRA_CALL_CONTROLLED_BY_IMS, false);
                return;
            }

            ImsReasonInfo reasonInfo = ImsCallUtils.createReasonInfo(
                    callReasonInfo.mCode, ImsReasonInfo.CODE_UNSPECIFIED,
                    callReasonInfo.mExtraMessage, ImsCallUtils.FLAG_REASON_INFO_ALL);

            mCallback.invokeUpdateFailed(ImsCallSessionImpl.this, reasonInfo);
        }

        @Override
        public void onCallUpdateReceived(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            // FIXME: Check the race condition
            int slotId = mCallContext.getSlotId();
            log("onCallUpdateReceived :: slotId" + slotId);

            // VIDEO_CALL_HOLD
            if (ImsCallUtils.isVideoCall(mCallProfile.getCallType())) {
                if (!call.isOnHeld()
                        && MtcCallUtils.isHoldMediaOnVideoCallByRemoteEnd(mediaInfo,
                            isCallFeatureSupported(CF_VIDEO_HOLD_WITH_INACTIVE))) {
                    onVideoCallHoldReceived(call, callInfo, mediaInfo, suppInfo);
                    return;
                } else if (call.isOnHeld()
                        && MtcCallUtils.isUnholdMediaOnVideoCallByRemoteEnd(mediaInfo)) {
                    onVideoCallResumeReceived(call, callInfo, mediaInfo, suppInfo);
                    return;
                }
            }

            // GLARE_CONDITION: between call mode changes
            if (mVideoCallSession.isSessionModificationInProgress()) {
                rejectSessionUpdateAsync(call,
                        CallReasonInfo.CODE_REJECT_ONGOING_CALL_UPGRADE,
                        "onCallUpdateReceived :: SessionModification-InProgress");
                return;
            }

            setState(ImsCallSessionImplBase.State.RENEGOTIATING);

            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mProposedCallProfile, callInfo);
            ImsCallMediaUtils.updateCallProfileFromMediaInfo(
                    mCallContext, mProposedCallProfile, mediaInfo);
            updateCallExtraForHDVoice(mProposedCallProfile, mediaInfo);

            if (isCallFeatureSupported(CF_TTY)) {
                mProposedCallProfile.setCallExtraInt(
                        ImsCallMediaUtils.MEDIA_TEXT_DIRECTION, mediaInfo.textDir);
                mProposedCallProfile.setCallExtraInt(MEDIA_GTT_MODE, mediaInfo.gttMode);
            }

            if (ImsCallUtils.isCallTypeChanged(mCallProfile, callInfo)) {
                if (checkAndRejectSessionModificationRequest()) {
                    return;
                }

                if (shouldUpdateVideoCapabilityBeforeModifyRequest(
                        mCallProfile, mProposedCallProfile)) {
                    mCallProfile.setCallExtraBoolean(
                            ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE, true);
                    updateCallTypeChangeCapability();
                    mCallback.invokeUpdated(ImsCallSessionImpl.this,
                            ImsCallUtils.getSanitizedCallProfileForVideoDirection(mCallProfile));

                    log("onCallUpdateReceived :: delay the video upgrade"
                            + "to sync video capability with the call framework");
                    getCallHandler().postDelayed(
                            () -> mVideoCallSession.receiveSessionModifyRequest(
                            ImsVideoCallSession.MODIFICATION_CALL_TYPE, mediaInfo), 100);
                } else {
                    mVideoCallSession.receiveSessionModifyRequest(
                                ImsVideoCallSession.MODIFICATION_CALL_TYPE, mediaInfo);
                }

                return;
            } else if (ImsCallUtils.isVideoProfileChanged(mCallProfile, callInfo, mediaInfo)) {
                if (checkAndRejectSessionModificationRequest()) {
                    return;
                }

                mVideoCallSession.receiveSessionModifyRequest(
                        ImsVideoCallSession.MODIFICATION_VIDEO_PROFILE, mediaInfo);
                return;
            }

            // FIXME: Consider the priority of RTT Upgrade vs. VT Upgrade
            if (MtcCallUtils.isGttEnabled(mediaInfo.gttMode)
                    && !MtcCallUtils.isGttEnabled(mCall.getMediaInfo().gttMode)) {
                log("onCallUpdateReceived :: RTT upgrade request");
                mCallback.invokeRttModifyRequestReceived(
                        ImsCallSessionImpl.this, mProposedCallProfile);
                return;
            } else if (!MtcCallUtils.isGttEnabled(mediaInfo.gttMode)
                    && MtcCallUtils.isGttEnabled(mCall.getMediaInfo().gttMode)) {
                log("onCallUpdateReceived :: RTT downgrade request");
                postAndRunTask(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            logi("Voice call is automatically accepted (RTT)");
                            sendRttModifyResponse(false);
                            mCallback.invokeUpdated(ImsCallSessionImpl.this,
                                    ImsCallUtils.getSanitizedCallProfileForVideoDirection(
                                            mCallProfile));
                        } catch (Throwable t) {
                            loge("onCallUpdateReceived: " + t.toString());
                        }
                    }
                });
                return;
            }

            ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                    mCallContext, callInfo, mediaInfo);

            updateCallExtraForHDVoice(profile, mediaInfo);

            mCallback.invokeUpdateReceived(ImsCallSessionImpl.this, profile);
        }

        @Override
        public void onCallUpdateResumeReceived(MtcCall call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            // VIDEO_CALL_RESUME
            if (call.isOnHeld()) {
                if (ImsCallUtils.isVoiceCall(mCallProfile.getCallType())) {
                    onVoiceCallResumeReceived(call, callInfo, mediaInfo, suppInfo);
                    return;
                } else if (ImsCallUtils.isVideoCall(mCallProfile.getCallType())) {
                    onVideoCallResumeReceived(call, callInfo, mediaInfo, suppInfo);
                    return;
                }
            }

            rejectSessionUpdateAsync(call, CallReasonInfo.CODE_REJECT_ONGOING_CALL_UPGRADE,
                    "onCallUpdateResumeReceived");
        }

        @Override
        public void onCallIncomingReceived(
                MtcCall call, IncomingMtcCall incomingCall, int ratType) {
            if (!call.equals(mCall)) {
                return;
            }

            ImsStreamMediaProfile tMediaprofile = mLocalCallProfile.getMediaProfile();

            int videoQuality = tMediaprofile.getVideoQuality();


            ImsCallProfile profile = ImsCallUtils.createCallProfileFromIncomingCallInfo(
                    mCallContext, incomingCall);
            profile.setCallExtraInt(ImsCallProfile.EXTRA_CALL_NETWORK_TYPE, ratType);
            initCallProfile(profile);

            // For Google native behavior
            // Set local call profile as local capability for audio quality
            int audioQuality = mCallContext.getMediaCapabilities(
                    mCallProfile.getCallType(), ICallContext.MEDIA_AUDIO);

            if (ImsCallUtils.isVideoCall(mCallProfile.getCallType())) {
                videoQuality = mCallContext.getMediaCapabilities(
                        mCallProfile.getCallType(), ICallContext.MEDIA_VIDEO);
            }

            mLocalCallProfile.getMediaProfile().copyFrom(new ImsStreamMediaProfile(audioQuality,
                    tMediaprofile.getAudioDirection(), videoQuality,
                    tMediaprofile.getVideoDirection(), tMediaprofile.getRttMode()));

            mCT.updateCallState(ImsCallSessionImpl.this,
                    CallTracker.CALL_EVENT_INCOMING_RECEIVED, null);

            notifyE2eeCallInfo(incomingCall.suppInfo, mCall.getRemoteNumber());
        }

        @Override
        public void onCallRatChanged(int ratType) {
            log("onCallRatChanged ratType=" + ratType);
            if (!mCallProfile.getCallExtras().containsKey(ImsCallProfile.EXTRA_CALL_NETWORK_TYPE)) {
                return;
            }

            mCallProfile.setCallExtraInt(ImsCallProfile.EXTRA_CALL_NETWORK_TYPE, ratType);
            mCallback.invokeUpdated(ImsCallSessionImpl.this,
                    ImsCallUtils.getSanitizedCallProfileForVideoDirection(mCallProfile));
        }

        @Override
        public void onCallTransferred(MtcCall call) {
            if (!call.equals(mCall)) {
                return;
            }
            log("onCallTransferred callId=" + getCallId());

            mTransferTargetSession.mCallDetails.clear(CallDetails.ON_ECT);
            mTransferTargetSession.mCallDetails.clear(CallDetails.IMPLICIT_ON_HOLD);

            if (!mIsEctConfirmationRequired) {
                log("CallTransfer: Confirmation not required");
            } else {
                mCallback.invokeCallSessionTransferred(ImsCallSessionImpl.this);
            }

            clearTransferTargetSessionEctDetails();
        }

        @Override
        public void onCallTransferFailed(MtcCall call, CallReasonInfo callReasonInfo) {
            if (!call.equals(mCall)) {
                return;
            }
            log("onCallTransferFailed callId=" + getCallId());

            ImsReasonInfo reasonInfo = ImsCallUtils.createReasonInfo(
                    callReasonInfo.mCode, ImsReasonInfo.CODE_UNSPECIFIED,
                    callReasonInfo.mExtraMessage, ImsCallUtils.FLAG_REASON_INFO_ALL);

            if (mTransferTargetSession.mCallDetails.is(CallDetails.ON_ECT)) {
                MtcCall transferTargetCall = mTransferTargetSession.mCall;
                if (transferTargetCall.isOnHold()
                        && mTransferTargetSession.mCallDetails.is(CallDetails.IMPLICIT_ON_HOLD)) {
                    mOperationFailReason = reasonInfo;
                    transferTargetCall.resume(MtcCallUtils.createUnholdMedia(
                            transferTargetCall.getCallInfo(), transferTargetCall.getMediaInfo(),
                            isCallFeatureSupported(CF_VIDEO_HOLD_WITH_INACTIVE)));
                }
            }

            if (!mIsEctConfirmationRequired) {
                log("CallTransfer: Confirmation not required");
            } else {
                mCallback.invokeCallSessionTransferFailed(ImsCallSessionImpl.this, reasonInfo);
            }

            clearTransferTargetSessionEctDetails();
        }

        @Override
        public void onCallTransferReceived(MtcCall call, MtcCall newCall, CallInfo callInfo,
                MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.equals(mCall)) {
                return;
            }

            if (newCall != null) {
                newCall.setListener(mListenerProxy);
                MtcCall.setListener(newCall, mConferenceListenerProxy);

                mCall.setListener(null);
                MtcCall.setListener(mCall, null);

                mVideoCallProvider.updateMediaSession(newCall.getMediaSession());

                final MtcCall oldCall = mCall;
                mCall = newCall;

                oldCall.setListener(null);
            }

            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mCallProfile, callInfo);
            ImsCallMediaUtils.updateCallProfileFromMediaInfo(
                    mCallContext, mCallProfile, mediaInfo);
            updateCallExtraForHDVoice(mCallProfile, mediaInfo);

            ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                    mCallContext, callInfo, mediaInfo);

            setCallInfo(profile);

            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mLocalCallProfile, callInfo);
            updateCallExtraForHDVoice(mLocalCallProfile, null);

            mCallback.invokeUpdated(ImsCallSessionImpl.this, profile);

            mCallback.invokeSuppServiceReceived(ImsCallSessionImpl.this,
                    ImsSuppServiceUtils.MT.getForwardedCall(0, null, null));
        }

        @Override
        public void onCallInfoUpdated(MtcCall call,
                int type, String strValue, int intValue, boolean booleanValue) {
            if (!call.equals(mCall)) {
                return;
            }

            log("onCallInfoUpdated :: type=" + type + ", strValue=" + strValue
                    + ", intValue=" + intValue + ", boolValue=" + booleanValue);

            if (type == IUMtcCall.INFO_TYPE_USSD) {
                mCallback.invokeUssdMessageReceived(
                        ImsCallSessionImpl.this, intValue, strValue);
            } else if ((type == IUMtcCall.INFO_TYPE_MEDIA_VIDEO_LOWEST_BIT_RATE)
                    || (type == IUMtcCall.INFO_TYPE_MEDIA_VIDEO_NO_DATA)) {
                if (mVideoCallSession != null) {
                    mVideoCallSession.handleCallSessionEvent(type);
                }
            } else if (type == IUMtcCall.INFO_TYPE_MEDIA_DTMF_RECEIVED) {
                char dtmf = strValue.charAt(0);
                mCallback.invokeDtmfReceived(ImsCallSessionImpl.this, dtmf);
            }
        }

        @Override
        public void onCallRttMessageReceived(MtcCall call, String data) {
            if (!call.equals(mCall)) {
                return;
            }

            if (!mCallProfile.getMediaProfile().isRttCall()) {
                loge("onCallRttMessageReceived :: not RTT call. ignore");
                return;
            }

            mCallback.invokeRttMessageReceived(ImsCallSessionImpl.this, data);
        }

        @Override
        public void onCallRttAudioIndication(MtcCall call,
                boolean status) {
            if (!call.equals(mCall)) {
                return;
            }
            if (!mCallProfile.getMediaProfile().isRttCall()) {
                loge("onCallRttAudioIndication :: not RTT call. ignore");
                return;
            }

            ImsStreamMediaProfile mediaProfile = mCallProfile.getMediaProfile();
            ImsStreamMediaProfile profile = new ImsStreamMediaProfile(
                    mediaProfile.getAudioQuality(), mediaProfile.getAudioDirection(),
                    mediaProfile.getVideoQuality(), mediaProfile.getVideoDirection(),
                    mediaProfile.getRttMode());

            profile.setReceivingRttAudio(status);

            mCallback.invokeRttAudioIndicatorChanged(ImsCallSessionImpl.this, profile);
        }

        @Override
        public void onAudioSessionOpened(MtcCall call) {
            if (!call.equals(mCall)) {
                return;
            }

            logi("onAudioSessionOpened");
            mCallDetails.set(CallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END);
        }

        @Override
        public void onAudioSessionClosed(MtcCall call) {
            if (!call.equals(mCall)) {
                return;
            }

            logi("onAudioSessionClosed");
            if (mCallDetails.is(CallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END)) {
                if ((getState() == ImsCallSessionImplBase.State.TERMINATED)
                        && !mCallDetails.is(CallDetails.CALL_END_FINISHED)
                        && !isCacheCallReasonInfoNull()) {
                    notifyCallTerminated(mCacheCallReasonInfo.mCode,
                            mCacheCallReasonInfo.mExtraCode,
                            mCacheCallReasonInfo.mExtraMessage);
                }
                mCallDetails.clear(CallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END);

                if (mCallDetails.is(CallDetails.CLOSE_PENDING)
                        && !mCallDetails.is(CallDetails.IMPLICIT_TERMINATED)) {
                    closeSessionWithDelay(ImsCallSessionImpl.this, 500);
                }
            }

            if (isConferenceTransitionInProgress() && isImplicitTerminatedCondition()) {
                clearConferenceProxy();
                mConferenceListenerProxy.closeMtcCallIfSessionTerminatedOnConference(mCall);
            }
        }

        @Override
        public void onCallQualityChanged(MtcCall call, CallQuality callQuality) {
            if (!call.equals(mCall)) {
                return;
            }

            logi("onCallQualityChanged");
            /* After SRVCC success case don't invoke callQualityChanged as ImsCallSession
             * will be closed from framework side.
             */
            if (!mCallDetails.is(CallDetails.CLOSE_PENDING)) {
                mCallback.invokeCallQualityChanged(callQuality);
            }
        }

        @Override
        public void onCallRtpHeaderExtensionsReceived(MtcCall call,
                Set<RtpHeaderExtension> extensions) {
            if (!call.equals(mCall)) {
                return;
            }

            logi("onCallRtpHeaderExtensionsReceived");
            mCallback.invokeRtpHeaderExtensionsReceived(extensions);
        }

        @Override
        public void onTriggerAnbrQueryReceived(MtcCall call, int mediaType, int direction,
                int bitsPerSecond) {
            if (!call.equals(mCall)) {
                return;
            }

            logi("onTriggerAnbrQueryReceived");
            mCallback.invokeSendAnbrQuery(mediaType, direction, bitsPerSecond);
        }

        @Override
        public void onNotifyIncomingDtmfReceived(MtcCall call, int numDtmfDigit) {
            if (!call.equals(mCall)) {
                return;
            }

            logi("onNotifyIncomingDtmfReceived");
            mCallback.invokeDtmfReceived(
                    ImsCallSessionImpl.this, ImsCallUtils.convertIntToDtmfDigit(numDtmfDigit));
        }

        private void clearTransferRequestedSessionEctDetails() {
            if (mTransferRequestedSession != null) {
                mTransferRequestedSession.mTransferTargetNumber = null;
                mTransferRequestedSession.mIsEctConfirmationRequired = false;
                mTransferRequestedSession.mTransferTargetSession = null;
                mTransferRequestedSession = null;
            }
        }

        private void clearTransferTargetSessionEctDetails() {
            if (mTransferTargetSession != null) {
                mIsEctConfirmationRequired = false;
                mTransferTargetNumber = null;
                mTransferTargetSession.mTransferRequestedSession = null;
                mTransferTargetSession = null;
            }
        }

        private void onVideoCallHoldReceived(final MtcCall call,
                final CallInfo callInfo, final MediaInfo mediaInfo, final SuppInfo suppInfo) {
            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    logi("onVideoCallHoldReceived");
                    call.accept(MtcCallInfo.getCallType(callInfo), mediaInfo);
                    onCallHoldReceived(call, callInfo, mediaInfo, suppInfo);
                }
            });
        }

        private void onVideoCallResumeReceived(final MtcCall call,
                final CallInfo callInfo, final MediaInfo mediaInfo, final SuppInfo suppInfo) {
            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    logi("onVideoCallResumeReceived");

                    if (call.isOnHold()) {
                        // Accept the resume request without any changes
                        log("onVideoCallResumeReceived :: on-hold");
                    } else if ((mVideoCallSession != null)
                            && !mVideoCallSession.isCameraOn()) {
                        if (mediaInfo.videoDir == MediaInfo.DIRECTION_SEND_RECEIVE) {
                            if (mVideoCallSession.isMultitaskingState()) {
                                log("onVideoCallResumeReceived :: inactive");
                                mediaInfo.videoDir = MediaInfo.DIRECTION_INACTIVE;
                            } else {
                                log("onVideoCallResumeReceived :: recvonly");
                                mediaInfo.videoDir = MediaInfo.DIRECTION_RECEIVE;
                            }
                        } else if (mediaInfo.videoDir == MediaInfo.DIRECTION_SEND) {
                            log("onVideoCallResumeReceived :: call type is changed");
                            MtcCallInfo.setCallType(callInfo, IUMtcCall.CALLTYPE_VOIP);
                            mediaInfo.videoDir = MediaInfo.DIRECTION_SEND_RECEIVE;
                        }
                    } else if ((mVideoCallSession != null)
                            && mVideoCallSession.isMultitaskingState()) {
                        log("onVideoCallResumeReceived :: inactive by multitasking");
                        mediaInfo.videoDir = MediaInfo.DIRECTION_INACTIVE;
                    }

                    call.accept(MtcCallInfo.getCallType(callInfo), mediaInfo);

                    if (MtcCallInfo.getCallType(callInfo) == IUMtcCall.CALLTYPE_VOIP) {
                        mediaInfo.videoQuality = MediaInfo.VIDEO_QUALITY_NONE;
                        mediaInfo.videoDir = MediaInfo.DIRECTION_INVALID;
                    }

                    onCallResumeReceived(call, callInfo, mediaInfo, suppInfo);
                }
            });
        }

        private void onVoiceCallResumeReceived(final MtcCall call,
                final CallInfo callInfo, final MediaInfo mediaInfo, final SuppInfo suppInfo) {
            postAndRunTask(new Runnable() {
                @Override
                public void run() {
                    logi("onVoiceCallResumeReceived");

                    if (call.isOnHold()) {
                        if (isCallFeatureSupported(CF_AUDIO_HOLD_WITH_INACTIVE)) {
                            mediaInfo.audioDir = MediaInfo.DIRECTION_INACTIVE;
                        } else if (mediaInfo.audioDir == MediaInfo.DIRECTION_SEND_RECEIVE) {
                            mediaInfo.audioDir = MediaInfo.DIRECTION_SEND;
                        } else if (mediaInfo.audioDir == MediaInfo.DIRECTION_RECEIVE) {
                            mediaInfo.audioDir = MediaInfo.DIRECTION_INACTIVE;
                        }
                    }

                    call.accept(MtcCallInfo.getCallType(callInfo), mediaInfo);
                    onCallResumeReceived(call, callInfo, mediaInfo, suppInfo);
                }
            });
        }

        private void onTtyModeReceived(int gttMode, boolean onCallStarted) {
            if (!isCallFeatureSupported(CF_TTY)) {
                return;
            }

            if (onCallStarted) {
                if (!MtcCallUtils.isGttEnabled(gttMode)) {
                    // GTT mode is not enabled
                    return;
                }
            } else {
                int oldGttMode = mCallProfile.getCallExtraInt(
                        MEDIA_GTT_MODE, MediaInfo.GTTMODE_INVALID);

                if (oldGttMode == gttMode) {
                    // GTT mode is not updated
                    return;
                }
            }

            mCallProfile.setCallExtraInt(MEDIA_GTT_MODE, gttMode);

            logi("onTtyModeReceived :: mode=" + gttMode);

            mCallback.invokeTtyModeReceived(ImsCallSessionImpl.this,
                ImsCallMediaUtils.getTtyModeFromMediaInfoToTelecom(gttMode));
        }

        private void onRttChanged(boolean isRttOn, boolean oldRttOn) {
            log("onRttChanged :: isRttOn=" + isRttOn + "oldRttOn=" + oldRttOn);

            if (mCallDetails.is(CallDetails.RTT_TURNING_ON)) {
                mCallDetails.clear(CallDetails.RTT_TURNING_ON);
                log("onRttChanged :: RTT_TURNING_ON is cleared");

                if (isRttOn) {
                    mCallback.invokeRttModifyResponseReceived(ImsCallSessionImpl.this,
                        RttModifyStatus.SESSION_MODIFY_REQUEST_SUCCESS);
                } else {
                    mCallback.invokeRttModifyResponseReceived(ImsCallSessionImpl.this,
                        RttModifyStatus.SESSION_MODIFY_REQUEST_FAIL);
                }
            } else if (mCallDetails.is(CallDetails.RTT_TURNING_OFF)) {
                mCallDetails.clear(CallDetails.RTT_TURNING_OFF);
                log("onRttChanged :: RTT_TURNING_OFF is cleared");

                if (!isRttOn) {
                    mCallback.invokeRttModifyResponseReceived(ImsCallSessionImpl.this,
                        RttModifyStatus.SESSION_MODIFY_REQUEST_SUCCESS);
                } else {
                    mCallback.invokeRttModifyResponseReceived(ImsCallSessionImpl.this,
                        RttModifyStatus.SESSION_MODIFY_REQUEST_FAIL);
                }
            }

        }

        private boolean checkAndHandleConferenceOnCallTerminated(CallReasonInfo callReasonInfo) {
            if (MtcCallUtils.isCallTerminatedByJoiningConference(callReasonInfo.mCode)
                    && (mTerminationReason == ImsReasonInfo.CODE_UNSPECIFIED)) {
                logi("CALL_MERGE :: ignore the TERMINATED event");

                setState(ImsCallSessionImplBase.State.TERMINATED);

                if (!isConferenceTransitionInProgress()) {
                    // TODO: _CONFERENCE_CALL_CONNECTION_ID_
                    ConferenceInfoHelper.setListenerForConferenceUser(
                            mCall.getCallConnectionId() + "", mCall.getConferenceUserId(),
                            null);

                    notifyCallTerminated(callReasonInfo.mCode, callReasonInfo.mExtraCode,
                            callReasonInfo.mExtraMessage);
                    closeMtcCall(mCall);
                } else {
                    // After receiving the result of call merge,
                    // the session will be closed.
                    mCallDetails.set(CallDetails.SESSION_TERMINATED_ON_CONFERENCE);
                }
                return true;
            } else if (MtcCallUtils.isCallTerminatedByJoiningConference(callReasonInfo.mExtraCode)
                    && ((mTerminationReason == ImsReasonInfo.CODE_USER_TERMINATED_BY_REMOTE)
                        || (mTerminationReason == ImsReasonInfo
                            .CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE))) {
                // Rollback the call state to ESTABLISHED to handle the call terminated
                logi("CALL_MERGE :: Call state will be restored to ESTABLISHED");
                setState(ImsCallSessionImplBase.State.ESTABLISHED);

                // Overwrite the existing reason code from the call end reason
                mTerminationReason = ImsCallUtils.getReasonFromMTC(callReasonInfo.mCode);
            }

            return false;
        }

        private void checkAndHandleTransferOnCallTerminated(CallReasonInfo callReasonInfo) {
            if (MtcCallUtils.isCallTerminatedByCallForward(callReasonInfo)) {
                if (mTerminationReason == ImsReasonInfo.CODE_UNSPECIFIED) {
                    //FIXME: Need to add proper reason for ECT terminated.
                    // may be CODE_USER_TERMINATED_BY_ECT
                    setTerminationReason(ImsReasonInfo.CODE_USER_TERMINATED);
                }

                logi("Call terminated by ECT");
                /* Wait for {@link #onAudioSessionClosed} for transfer target session.
                 * Once it receives then will initiate {@link #callSessionTerminated}
                 * With that FW will call {@link #close} and MtcCall will be closed.
                 */
            }

            mCallDetails.clear(CallDetails.ON_ECT);
            mCallDetails.clear(CallDetails.IMPLICIT_ON_HOLD);
            mTransferTargetNumber = null;
            mIsEctConfirmationRequired = false;
            mTransferRequestedSession = null;
            mTransferTargetSession = null;
        }

        private void checkAndNotifyCallOperationFailureOnCallTerminated(
                final MtcCall call, CallReasonInfo callReasonInfo) {
            if (mCallDetails.is(CallDetails.ON_HOLDING)) {
                onCallHoldFailed(call, callReasonInfo);
            } else if (mCallDetails.is(CallDetails.ON_UNHOLDING)) {
                onCallResumeFailed(call, callReasonInfo);
            } else if (mCallDetails.is(CallDetails.ON_MERGING)) {
                if (isConferenceTransitionInProgress()) {
                    return;
                }

                log("Foreground call is terminated by user during call merge");

                mCallDetails.clear(CallDetails.ON_MERGING);

                ImsConferenceHelper ich = ImsConferenceHelper.getInstance();
                ImsCallSessionImpl transientConfSession
                        = (ImsCallSessionImpl)ich.getTransientConferenceSession();
                ich.setTransientConferenceSession(null);

                ConferenceInfoHelper.removeConferenceUser(
                        mCall.getCallId(), mCall.getConferenceUserId());

                notifyCallSessionMergeFailed(callReasonInfo.mCode, callReasonInfo.mExtraMessage,
                        0);

                // CASE: initial merge failure
                if (transientConfSession != null) {
                    transientConfSession.closeInternal(transientConfSession);
                } else {
                    ImsCallSessionImpl bgSession = (ImsCallSessionImpl)ich.getBackgroundSession();

                    if ((bgSession != null) && (ImsCallSessionImpl.this != bgSession)) {
                        MtcConference conference = MtcCall.getConference(bgSession.mCall);

                        if (conference != null) {
                            bgSession.mConferenceListenerProxy.onCallMergeFailed(
                                    conference, callReasonInfo);
                        }
                    }
                }
            }
        }

        private boolean checkAndRejectSessionModificationRequest() {
            if (ImsCallUtils.isCallOnNativeAppsAndCountryKR(mCallContext)) {
                if (mCT.getActiveCalls() > 1) {
                    logi("SessionModificationRequest :: rejected by multiple calls");

                    try {
                        reject(ImsReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION);
                        mLocalCallProfile.setCallExtraBoolean(EXTRA_CALL_CONTROLLED_BY_IMS, true);
                        return true;
                    } catch (Throwable t) {
                        loge(t.toString());
                    }
                }
            }

            return false;
        }
    }

    private void clearProposedCallProfile() {
        mProposedCallProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO);
        mProposedCallProfile.setCallerNumberVerificationStatus(
                ImsCallProfile.VERIFICATION_STATUS_NOT_VERIFIED);
    }

    private void notifyE2eeCallInfo(SuppInfo suppInfo, String remoteNumber) {
        if (mIsE2eeCallInfoNotified) {
            return;
        }

        log("notifyE2eeCallInfo");
        SuppService suppSessionId = suppInfo.getService(SuppInfo.SUPP_TYPE_SESSION_ID);
        String sessionId = suppSessionId == null ? "" : suppSessionId.strValue;
        if (sessionId.isEmpty()) {
            return;
        }

        mCallExtManagerProxy.reportCallInfo(mCallContext.getSlotId(), getCallId(),
                sessionId.getBytes(), sessionId.getBytes(), remoteNumber);
        mIsE2eeCallInfoNotified = true;
    }

    @VisibleForTesting
    public void setImsCallExtManagerProxy(ImsCallExtManagerProxy proxy) {
        mCallExtManagerProxy = proxy;
    }

    private class MtcConferenceListenerProxy extends MtcConference.Listener
            implements ConferenceInfo.User.Listener {
        private int mSlotId = 0;
        private boolean mIsConferenceHost = false;
        // If this session is a conference session, this variable grabs
        // the foreground session that was used in an initial merge.
        // Otherwise, it's usually null.
        private ImsCallSessionImpl mCallSession = null;

        public void closeSession() {
            if (mCallSession != null) {
                mCallSession.closeInternal(mCallSession);
                mCallSession = null;
            }
        }

        public void setConferenceAttributes(int slotId,
                boolean isConferenceHost, ImsCallSessionImpl callSession) {
            mSlotId = slotId;
            mIsConferenceHost = isConferenceHost;
            mCallSession = callSession;
        }

        @Override
        public void onConferenceUserStatusUpdated(ConferenceInfo.User user) {
            if (DBG) {
                log("onConferenceUserStatusUpdated: " + user);
            }

            if (ConferenceInfo.User.STATUS_DISCONNECTED.equals(user.getStatus())) {
                ImsCallConnectionIds.remove(mSlotId, user.getCallConnectionId());

                if (mIsConferenceHost) {
                    // CALL_CONNECTION_ID
                    log("Peer participant of conference host dropped");
                    setCallConnectionId(0);
                    /* When foreground session is disconnected by user, close it with delay while
                     * {@link#onCallMerged} is in progress. So when {@link#invokeMergeComplete}
                     * is called followed by {@link#notifyCallSessionConferenceStateUpdated}
                     * foreground session callback will be available to update it to framework.
                     */
                    mCallSession.mCallDetails.set(CallDetails.CLOSE_PENDING);
                    mCallSession.closeSessionWithDelay(mCallSession, 500);
                }
            }
        }

        @Override
        public void onCallProxyMerge(MtcConference confCall,
                MtcConference hostCall, MtcConference peerCall) {
            if (!hostCall.isSameCall(MtcCall.getConference(mCall))) {
                return;
            }

            MtcCall confCallL = (MtcCall)confCall.getParent();
            MtcCall hostCallL = (MtcCall)hostCall.getParent();
            MtcCall peerCallL = (MtcCall)peerCall.getParent();

            // no-op
            log("onCallProxyMerge :: host=" + hostCallL + ", peer=" + peerCallL);

            if (!hostCallL.isConference()) {
                // Adjust a conference user id based on the initialized id.
                ConferenceInfoHelper.setAnonymousId(1);
                hostCallL.updateConferenceUserId(null);
                peerCallL.updateConferenceUserId(null);

                // After the first merge, the foreground call will be closed.
                ConferenceInfoHelper.addConferenceUser(confCallL.getCallId(),
                        hostCallL.getConferenceUserId(),
                        hostCallL.getCallExtraInt(Call.EXTRA_CALL_CONNECTION_ID, 0));
                ConferenceInfoHelper.addConferenceUser(confCallL.getCallId(),
                        peerCallL.getConferenceUserId(),
                        peerCallL.getCallExtraInt(Call.EXTRA_CALL_CONNECTION_ID, 0));
            } else {
                // If a participant rejoins after being removed, ensures data associated with
                // previous presence is deleted before re-adding the participant.
                // This prevents duplicate user entries, even if the participant's callId changes.
                if (ConferenceInfoHelper.isConferenceUserRemovable(
                        peerCallL.getConferenceUserId())) {
                    ConferenceInfoHelper.removeConferenceUser(
                            null, peerCallL.getConferenceUserId());
                }

                ConferenceInfoHelper.addConferenceUser(hostCallL.getCallId(),
                        peerCallL.getConferenceUserId(),
                        peerCallL.getCallExtraInt(Call.EXTRA_CALL_CONNECTION_ID, 0));
            }

            // In case of call merge, all the users are in "connected" state initially.
            ConferenceInfo ci = ConferenceInfoHelper.getConferenceInfo(confCallL.getCallId());

            // We need to check if DIALING_IN is proper in this moment.
            if ((ci != null) && !hostCallL.isConference()) {
                ci.updateUser(hostCallL.getCallId(),
                        hostCallL.getConferenceUserId(),
                        ConferenceInfo.User.STATUS_DIALING_IN);
            }

            if (ci != null) {
                ci.updateUser(peerCallL.getCallId(),
                        peerCallL.getConferenceUserId(),
                        ConferenceInfo.User.STATUS_DIALING_IN);
            }

            if (isCallFeatureSupported(CF_CONF_USER_ANONYMOUS) && !hostCallL.isConference()) {
                String hostConfUserId = hostCallL.getConferenceUserId();
                String peerConfUserId = peerCallL.getConferenceUserId();

                if (hostConfUserId != null
                        && hostConfUserId.startsWith(Call.ANONYMOUS)
                        && peerConfUserId != null
                        && peerConfUserId.startsWith(Call.ANONYMOUS)) {
                    log("notifyCallSessionConferenceStateUpdated - vzw");
                    notifyCallSessionConferenceStateUpdated(confCallL.getCallId());
                }
            }
        }

        @Override
        public void onCallProxyExtendToConference(MtcConference confCall,
                MtcConference hostCall, String[] participants) {
            if (!hostCall.isSameCall(MtcCall.getConference(mCall))) {
                return;
            }

            MtcCall confCallL = (MtcCall)confCall.getParent();
            MtcCall hostCallL = (MtcCall)hostCall.getParent();

            log("onCallProxyExtendToConference :: host=" + hostCallL);

            //setState(ImsCallSessionImplBase.State.RENEGOTIATING);

            // FIXME: Creates the interim event states
            ConferenceInfoHelper.addConferenceUser(confCallL.getCallId(),
                    hostCallL.getConferenceUserId(), 0);

            ConferenceInfo ci = ConferenceInfoHelper.getConferenceInfo(confCallL.getCallId());

            if (ci != null) {
                ci.updateUser(hostCallL.getCallId(),
                        hostCallL.getConferenceUserId(),
                        ConferenceInfo.User.STATUS_CONNECTED);

                if (participants != null) {
                    for (int i = 0; i < participants.length; ++i) {
                        ci.addUserForInterimStatus(0, participants[i],
                                ConferenceInfo.User.STATUS_PENDING, 0, 0);
                    }
                }
            }
        }

        @Override
        public void onCallMergeStarted(MtcConference call, MtcConference confCall,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.isSameCall(MtcCall.getConference(mCall))) {
                return;
            }

            if (confCall == null) {
                return;
            }

            logi("onCallMergeStarted :: " + mCall);

            MtcCall confCallL = (MtcCall)confCall.getParent();

            // This indicates a call profile from the foreground call call
            ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                    mCallContext, callInfo, mediaInfo);
            ImsCallSessionImpl confCallSession = createNewCallSession(confCallL, profile);

            // Update the conference call identifier
            //ConferenceInfoHelper.replaceImmatureCcid(
            //        mCall.getCallId(), confCallL.getCallId());

            confCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHING);
            mCT.updateCallState(confCallSession, CallTracker.CALL_EVENT_CREATE, null);

            // FIXME: which profile should be passed to the current call?
            setCallInfo(profile);

            // Store the transient conference call to handle the result of call merge
            ImsConferenceHelper ich = ImsConferenceHelper.getInstance();
            ich.setTransientConferenceSession(confCallSession);

            mCallback.invokeMergeStarted(ImsCallSessionImpl.this, confCallSession, profile);

            // Notify the interim event state
            // M-OS: No needs to be updated in here because MergeStarted is not used.
            // notifyCallSessionConferenceStateUpdated(confCallL.getCallId());
        }

        @Override
        public void onCallMerged(MtcConference call,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo, UsersInfo usersInfo) {
            if (!call.isSameCall(MtcCall.getConference(mCall))) {
                return;
            }

            logi("onCallMerged :: " + mCall);

            ImsCallSessionImpl transientConfSession = getTransientConferenceSession();

            updateCallDetailsForNonConferenceSession(transientConfSession, callInfo, mediaInfo);

            UsersInfo conferenceParticipants = null;

            if (mCall.isConference() || isFirstCallMergeInitiator()) {
                conferenceParticipants = mConferenceProxy.getConferenceParticipants();
            }

            mCallDetails.clear(CallDetails.ON_MERGING);

            /* {@link #onCallQualityChanged} and {@link #onAudioSessionClosed} called when
             * {@link #onCallMerged} is in progress for foreground session. That time these Media
             * callbacks are in queue of postAndRun thread of {@link MergeProxy} and it will not be
             * delivered until {@link #onCallMerged} finishes. So when postAndRun tries to send to
             * lw.mListener that time this listener will set to null as it is cleared
             * in {@link #clearConferenceProxy}. To avoid this delay the
             * {@link ConferenceProxy#removeListener} so {@link #onAudioSessionClosed}
             * for foreground session can be delivered from {@link MergeProxy}.
             */
            if (!mCallDetails.is(CallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END)
                    || mCall.isConference()) {
                clearConferenceProxy();
            }

            int state = getState();

            if ((state != ImsCallSessionImplBase.State.TERMINATING)
                    && (state != ImsCallSessionImplBase.State.TERMINATED)) {
                setState(ImsCallSessionImplBase.State.ESTABLISHED);

                // If the call is a conference focus,
                // we do not transient this call state.
                if (!mCall.isConference()) {
                    mCallDetails.set(CallDetails.IMPLICIT_TERMINATED);
                    setState(ImsCallSessionImplBase.State.TERMINATED);
                }
            }

            // MOS: Invoke the merge completion callback only for the merge host.
            if (transientConfSession != null) {
                // O-MR1 change: It needs to be updated before call merge completed event.
                updateConferenceSession(ImsCallSessionImpl.this, transientConfSession,
                        usersInfo, conferenceParticipants);

                logi("Conference participants notified in initial merge");
                notifyCallSessionConferenceStateUpdated(
                        transientConfSession.getMtcCall().getCallId());
                mCallback.invokeMergeComplete(ImsCallSessionImpl.this, transientConfSession);
            } else if (mCall.isConference()) {
                updateCallProfile(callInfo, mediaInfo);
                mCallback.invokeMergeComplete(ImsCallSessionImpl.this, null);

                // Update the conference's call profile after call merged
                // , and update user's status if present.
                updateConferenceSession(ImsCallSessionImpl.this, transientConfSession,
                        usersInfo, conferenceParticipants);
            }

            closeMtcCallIfSessionTerminatedOnConference(mCall);
        }

        @Override
        public void onCallMergeFailed(MtcConference call, CallReasonInfo callReasonInfo) {
            if (!call.isSameCall(MtcCall.getConference(mCall))) {
                return;
            }

            logi("onCallMergeFailed :: " + mCall);

            ImsCallSessionImpl transientConfSession = null;
            boolean notifyMergeFailed = false;

            if (isFirstCallMergeInitiator()) {
                ImsConferenceHelper ich = ImsConferenceHelper.getInstance();
                transientConfSession = (ImsCallSessionImpl)ich.getTransientConferenceSession();
                ich.setTransientConferenceSession(null);
                notifyMergeFailed = true;
            } else if (mCall.isConference()) {
                notifyMergeFailed = true;
            }

            ConferenceInfoHelper.removeConferenceUser(
                    mCall.getCallId(), mCall.getConferenceUserId());

            mCallDetails.clear(CallDetails.ON_MERGING);
            /* When conference participant get terminated while merge, after
             * {@link #onCallMergeFailed} will get {@link #onCallMerged} and ImsStack will notify
             * both {@link #invokeMergeFailed} and {@link #invokeMergeComplete} to framework
             */
            clearConferenceProxy();

            // Update conference state because any update may be done previously.
            if (mCall.isConference()) {
                notifyCallSessionConferenceStateUpdated();
            }

            if (notifyMergeFailed) {
                notifyCallSessionMergeFailed(callReasonInfo.mCode, callReasonInfo.mExtraMessage, 0);
            }

            // FIXME: is it required?
            closeMtcCallIfSessionTerminatedOnConference(mCall);

            // CASE: initial merge failure
            if (transientConfSession != null) {
                transientConfSession.closeInternal(transientConfSession);
            }

            // If a call is joined to the conference and call merge is failed,
            // then the already joined call should be notified for call-terminated.
            int reason = getTerminationReason(ImsReasonInfo.CODE_UNSPECIFIED);

            if (!mCall.isConference()
                    && (getState() == ImsCallSessionImplBase.State.TERMINATED)
                    && ((reason == ImsReasonInfo.CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE)
                        || (reason == ImsReasonInfo.CODE_USER_TERMINATED_BY_REMOTE))
                    && !mCallDetails.is(CallDetails.CALL_END_FINISHED)) {
                waitOrNotifyCallTerminated(ImsReasonInfo.CODE_USER_TERMINATED_BY_REMOTE,
                        ImsReasonInfo.CODE_UNSPECIFIED,
                        ImsCallUtils.REASON_CALL_DISCONNECTED_BY_MERGE_FAILED);
            }
        }

        @Override
        public void onCallConferenceExtended(MtcConference call, long confCallId,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.isSameCall(MtcCall.getConference(mCall))) {
                return;
            }

            clearConferenceProxy();

            setState(ImsCallSessionImplBase.State.ESTABLISHED);

            MtcCall confCall = getMtcCall(confCallId);

            if (confCall != null) {
                confCall.setListener(mListenerProxy);
                MtcCall.setListener(confCall, mConferenceListenerProxy);

                mCall.setListener(null);
                MtcCall.setListener(mCall, null);

                mVideoCallProvider.updateMediaSession(confCall.getMediaSession());

                final MtcCall oldCall = mCall;
                mCall = confCall;

                oldCall.setListener(null);

                if (!closeMtcCallIfSessionTerminatedOnConference(oldCall)) {
                    postAndRunTask(new Runnable() {
                        @Override
                        public void run() {
                            oldCall.terminate(
                                    CallReasonInfo.CODE_UNSPECIFIED);
                        }
                    });
                }
            }

            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mCallProfile, callInfo);
            ImsCallMediaUtils.updateCallProfileFromMediaInfo(
                    mCallContext, mCallProfile, mediaInfo);
            updateCallExtraForHDVoice(mCallProfile, mediaInfo);

            ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                    mCallContext, callInfo, mediaInfo);

            setCallInfo(profile);

            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mLocalCallProfile, callInfo);
            updateCallExtraForHDVoice(mLocalCallProfile, null);

            mCallback.invokeConferenceExtended(ImsCallSessionImpl.this, null, profile);
        }

        @Override
        public void onCallConferenceExtendFailed(MtcConference call,
                CallReasonInfo callReasonInfo) {
            if (!call.isSameCall(MtcCall.getConference(mCall))) {
                return;
            }

            // All the user's state will be automatically removed in the application layer
            // by the notification of conference extension failure.

            clearConferenceProxy();

            if (mCallDetails.is(CallDetails.SESSION_TERMINATED_ON_CONFERENCE)) {
                mCallDetails.clear(CallDetails.SESSION_TERMINATED_ON_CONFERENCE);
                waitOrNotifyCallTerminated(ImsReasonInfo.CODE_USER_TERMINATED_BY_REMOTE,
                        ImsReasonInfo.CODE_UNSPECIFIED,
                        ImsCallUtils.REASON_CALL_DISCONNECTED_BY_MERGE_FAILED);
            } else {
                setState(ImsCallSessionImplBase.State.ESTABLISHED);

                ImsReasonInfo reasonInfo = ImsCallUtils.createReasonInfo(
                        callReasonInfo.mCode, ImsReasonInfo.CODE_UNSPECIFIED,
                        callReasonInfo.mExtraMessage, ImsCallUtils.FLAG_REASON_INFO_ALL);

                mCallback.invokeConferenceExtendFailed(ImsCallSessionImpl.this, reasonInfo);
                // Empty conference state notification
                mCallback.invokeConferenceStateUpdated(ImsCallSessionImpl.this,
                        getImsConferenceState());
            }
        }

        @Override
        public void onCallConferenceExtendReceived(MtcConference call, long confCallId,
                CallInfo callInfo, MediaInfo mediaInfo, SuppInfo suppInfo) {
            if (!call.isSameCall(MtcCall.getConference(mCall))) {
                return;
            }

            MtcCall confCall = getMtcCall(confCallId);

            if (confCall != null) {
                confCall.setListener(mListenerProxy);
                MtcCall.setListener(confCall, mConferenceListenerProxy);

                mCall.setListener(null);
                MtcCall.setListener(mCall, null);

                mVideoCallProvider.updateMediaSession(confCall.getMediaSession());

                final MtcCall oldCall = mCall;
                mCall = confCall;

                oldCall.setListener(null);

                postAndRunTask(new Runnable() {
                    @Override
                    public void run() {
                        oldCall.terminate(CallReasonInfo.CODE_UNSPECIFIED);
                    }
                });
            }

            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mCallProfile, callInfo);
            ImsCallMediaUtils.updateCallProfileFromMediaInfo(
                    mCallContext, mCallProfile, mediaInfo);
            updateCallExtraForHDVoice(mCallProfile, mediaInfo);

            ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                    mCallContext, callInfo, mediaInfo);

            setCallInfo(profile);

            ImsCallUtils.updateCallProfileFromCallInfo(
                    mCallContext, mLocalCallProfile, callInfo);
            updateCallExtraForHDVoice(mLocalCallProfile, null);

            mCallback.invokeConferenceExtendReceived(ImsCallSessionImpl.this, null, profile);

            notifyCallSessionMultipartyStateChanged(true);
        }

        @Override
        public void onCallInviteParticipantsRequestDelivered(MtcConference call) {
            if (!call.isSameCall(MtcCall.getConference(mCall))) {
                return;
            }

            mCallback.invokeInviteParticipantsRequestDelivered(ImsCallSessionImpl.this);

            // Notify the status changed for some users if the status of users is changed
            //notifyCallSessionConferenceStateUpdated();
        }

        @Override
        public void onCallInviteParticipantsRequestFailed(MtcConference call,
                CallReasonInfo callReasonInfo) {
            if (!call.isSameCall(MtcCall.getConference(mCall))) {
                return;
            }

            ImsReasonInfo reasonInfo = ImsCallUtils.createReasonInfo(
                    callReasonInfo.mCode, ImsReasonInfo.CODE_UNSPECIFIED,
                    callReasonInfo.mExtraMessage, ImsCallUtils.FLAG_REASON_INFO_ALL);

            mCallback.invokeInviteParticipantsRequestFailed(ImsCallSessionImpl.this, reasonInfo);

            // Notify the status changed for some users
            // FIXME: For LGU+, "connect-fail" ?
            ConferenceInfoHelper.removeConferenceUsersOnInvitationFailed(mCall.getCallId());
            notifyCallSessionConferenceStateUpdated();
        }

        @Override
        public void onCallRemoveParticipantsRequestDelivered(MtcConference call) {
            if (!call.isSameCall(MtcCall.getConference(mCall))) {
                return;
            }

            mCallback.invokeRemoveParticipantsRequestDelivered(ImsCallSessionImpl.this);

            // Notify the status changed for some users
            ConferenceInfoHelper.updateConferenceUsersOnRemoval(mCall.getCallId());
            notifyCallSessionConferenceStateUpdated();
            removeConferenceUsersOnDisconnected();
        }

        @Override
        public void onCallRemoveParticipantsRequestFailed(MtcConference call,
                CallReasonInfo callReasonInfo) {
            if (!call.isSameCall(MtcCall.getConference(mCall))) {
                return;
            }

            ImsReasonInfo reasonInfo = ImsCallUtils.createReasonInfo(
                    callReasonInfo.mCode, ImsReasonInfo.CODE_UNSPECIFIED,
                    callReasonInfo.mExtraMessage, ImsCallUtils.FLAG_REASON_INFO_ALL);

            mCallback.invokeRemoveParticipantsRequestFailed(ImsCallSessionImpl.this, reasonInfo);

            // Notify the status changed for some users
            ConferenceInfoHelper.updateConferenceUsersOnRemoval(mCall.getCallId());
            notifyCallSessionConferenceStateUpdated();
            removeConferenceUsersOnDisconnected();
        }

        @Override
        public void onCallConferenceStateUpdated(MtcConference call,
                UsersInfo usersInfo) {
            if (!call.isSameCall(MtcCall.getConference(mCall))) {
                return;
            }

            updateConferenceState(call, usersInfo, false, true);
        }

        private boolean closeMtcCallIfSessionTerminatedOnConference(MtcCall call) {
            if (mCallDetails.is(CallDetails.SESSION_TERMINATED_ON_CONFERENCE)
                    && !mCallDetails.is(CallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END)) {
                closeMtcCall(call);
                mCallDetails.clear(CallDetails.SESSION_TERMINATED_ON_CONFERENCE);
                return true;
            }

            return false;
        }

        private ImsCallSessionImpl getTransientConferenceSession() {
            if (!mCall.isConference()) {
                if (isFirstCallMergeInitiator()) {
                    ImsConferenceHelper ich = ImsConferenceHelper.getInstance();
                    return (ImsCallSessionImpl)ich.getTransientConferenceSession();
                }
            }

            return null;
        }

        private void updateCallDetailsForNonConferenceSession(
                ImsCallSessionImpl transientConfSession, CallInfo callInfo, MediaInfo mediaInfo) {
            if (!mCall.isConference()) {
                if (!isFirstCallMergeInitiator()) {
                    mCallDetails.set(CallDetails.MERGED);

                    // TODO: _CONFERENCE_CALL_CONNECTION_ID_
                    ConferenceInfoHelper.setListenerForConferenceUser(
                            mCall.getCallConnectionId() + "", mCall.getConferenceUserId(), this);
                    mConferenceListenerProxy.setConferenceAttributes(
                            mCallContext.getSlotId(), false, null);
                } else {
                    if (transientConfSession != null) {
                        transientConfSession.mCallDetails.set(CallDetails.MO_STARTED);
                        transientConfSession.mCallDetails.set(
                                CallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END);
                        transientConfSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);

                        ImsCallProfile profile = ImsCallUtils.createCallProfileFromCallInfo(
                                mCallContext, callInfo, mediaInfo);

                        updateCallExtraForHDVoice(profile, mediaInfo);
                        ImsCallUtils.setCallExtraIfPresent(mCallProfile,
                                ImsCallProfile.EXTRA_CALL_NETWORK_TYPE, ImsCallUtils.VAR_TYPE_INT,
                                profile);

                        // Update the session & media profiles
                        transientConfSession.initCallProfile(profile);
                        transientConfSession.updateCallExtraForHDVoice(
                                transientConfSession.mCallProfile, mediaInfo);

                        // CALL_CONNECTION_ID
                        transientConfSession.setCallConnectionId(getCallConnectionId());
                        setCallConnectionId(0);

                        // CALL_CONNECTION_ID
                        // TODO: _CONFERENCE_CALL_CONNECTION_ID_
                        ConferenceInfoHelper.setListenerForConferenceUser(
                                mCall.getCallConnectionId() + "", mCall.getConferenceUserId(),
                                transientConfSession.mConferenceListenerProxy);
                        transientConfSession.mConferenceListenerProxy.setConferenceAttributes(
                                mCallContext.getSlotId(), true, ImsCallSessionImpl.this);
                    }

                    ImsConferenceHelper ich = ImsConferenceHelper.getInstance();
                    ich.setTransientConferenceSession(null);

                    // Do not set the listener to monitor the user status change.
                    // This call session will be replaced to the conference call session
                    // by the ImsCall.
                    mCallDetails.set(CallDetails.MERGED_N_DETACHED);
                }
            }
        }

        private void updateConferenceSession(ImsCallSessionImpl confCall,
                ImsCallSessionImpl transientConfSession, UsersInfo usersInfo,
                UsersInfo cachedUsersInfo) {
            MtcConference conference = null;
            MtcConferenceListenerProxy listenerProxy = null;
            boolean notifyToApp = true;

            if (transientConfSession != null) {
                /* O-MR1 change: oneway for IImsCallSessionListener */
                notifyToApp = false;
                /*
                transientConfSession.mCallback.invokeUpdated(
                    transientConfSession, transientConfSession.mCallProfile);
                */

                listenerProxy = transientConfSession.mConferenceListenerProxy;
                conference = MtcCall.getConference(transientConfSession.mCall);
            } else if ((confCall.mCall != null) && confCall.mCall.isConference()) {
                listenerProxy = confCall.mConferenceListenerProxy;
                conference = MtcCall.getConference(confCall.mCall);
            }

            if ((listenerProxy != null) && (conference != null)) {
                Call call = conference.getParent();
                boolean updateEventStateOnly = true;

                if (call != null) {
                    updateEventStateOnly = call.getCallExtraBoolean(
                            Call.EXTRA_CONFERENCE_EVENT, false);

                    // FIXME: enforce it to false
                    updateEventStateOnly = false;
                }

                listenerProxy.updateConferenceState(conference,
                        usersInfo, updateEventStateOnly, notifyToApp);

                if (cachedUsersInfo != null) {
                    logi("Cached conference participants updated");
                    listenerProxy.updateConferenceState(conference,
                            cachedUsersInfo, updateEventStateOnly, notifyToApp);
                }
            }
        }

        private void updateConferenceState(MtcConference call,
                UsersInfo usersInfo, boolean updateEventStateOnly, boolean notifyToApp) {
            if (usersInfo == null) {
                return;
            }

            log("updateConferenceState :: call=" + call);

            int userCount = usersInfo.Users.size();

            if (userCount == 0) {
                log("No users in UsersInfo");
                return;
            }

            // FIXME: Is it required to update the conference state changed event?
            ConferenceInfo ci = ConferenceInfoHelper.getConferenceInfo(mCall.getCallId());

            if (ci == null) {
                loge("ConferenceInfo is null; " + mCall);
                return;
            }

            boolean isOneUserAtLeastUpdated = false;

            for (int i = 0; i < userCount; ++i) {
                UsersInfo.User user = usersInfo.Users.get(i);
                String callId = Call.getCallId(getMtcCall(user.callID), user.callID);

                if (updateEventStateOnly) {
                    if (ConferenceInfo.isInterimUser(
                            callId, user.target, user.userEntity, user.epEntity)) {
                        // Do not update the user if user's state is created by event package.
                        continue;
                    }
                }

                if (ConferenceInfoHelper.updateConferenceUser(ci,
                        callId, user.target, user.userEntity, user.epEntity, user.displayName,
                        ImsCallUtils.getStringFromUserStatus(user.status),
                        MtcCallUtils.getSIPStatusCodeFromUserStatusCode(user.statusCode),
                        ImsCallUtils.getDisconnectedCauseFromUserStatus(user.status))) {
                    isOneUserAtLeastUpdated = true;
                }
            }

            if (isOneUserAtLeastUpdated && notifyToApp) {
                notifyCallSessionConferenceStateUpdated();
                removeConferenceUsersOnDisconnected();
            }
        }
    }
}

