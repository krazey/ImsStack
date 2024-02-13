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

import android.telecom.Connection;
import android.telecom.VideoProfile;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsCallSession;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsVideoCallProvider;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.widget.Toast;

import com.android.imsstack.R;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.call.IVideoCallSession;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

public final class ImsVideoCallSession implements IVideoCallSession {
    private static final int UPDATE_STATE_IDLE = 0;
    private static final int UPDATE_STATE_SENT = 1;
    private static final int UPDATE_STATE_RECEIVED = 2;
    private static final int UPDATE_STATE_FINALIZING = 3;

    private final ICallContext mCallContext;
    private final ImsCallSessionImplBase mCallSession;
    private final boolean mMoCall;
    private ImsVideoCallProvider mVideoCallProvider;
    private EventListener mEventListener;
    // It's used to handle an incoming call type switching request
    private ImsStreamMediaProfile mProposalMediaProfile;
    // Old video profile which is used before the device requests the session modification
    private VideoProfile mProfileBeforeRequest;
    private VideoProfile mProposalProfile;
    private int mModificationType = MODIFICATION_NONE;
    private int mUpdateState = UPDATE_STATE_IDLE;
    private int mCameraSetting = CAMERA_ON;
    private int mMultitaskingState = MULTITASKING_NONE;
    private boolean mIsPausedByRemote = false;

    public ImsVideoCallSession(ICallContext callContext,
            ImsCallSessionImplBase callSession, boolean isMO) {
        mCallContext = callContext;
        mCallSession = callSession;
        mMoCall = isMO;
    }

    @Override
    public ICallContext getCallContext() {
        return mCallContext;
    }

    @Override
    public int getCallType() {
        final ImsCallProfile callProfile = getCallProfile();
        return (callProfile != null) ? callProfile.getCallType() : ImsCallProfile.CALL_TYPE_VOICE;
    }

    @Override
    public int getSessionModificationType() {
        synchronized (this) {
            return mModificationType;
        }
    }

    @Override
    public ImsStreamMediaProfile getProposedStreamMediaProfile() {
        synchronized (this) {
            return mProposalMediaProfile;
        }
    }

    @Override
    public ImsStreamMediaProfile getStreamMediaProfile() {
        final ImsCallProfile callProfile = getCallProfile();
        return (callProfile != null) ? callProfile.getMediaProfile() : null;
    }

    @Override
    public boolean isMoCall() {
        return mMoCall;
    }

    @Override
    public boolean isVrbtEnabled() {
        return false;
    }

    @Override
    public void sendSessionModifyRequest(VideoProfile fromProfile, VideoProfile toProfile) {
        if (isSessionModificationInProgress()
                || isSessionModificationFinalizing()) {
            // FIXME: exception handling
            log("SessionModification-InProgress: "
                    + ImsCallMediaUtils.toString(toProfile));

            mVideoCallProvider.receiveSessionModifyResponse(
                    Connection.VideoProvider.SESSION_MODIFY_REQUEST_INVALID,
                    null, fromProfile);
            return;
        }

        logi("SessionModification-Request(SEND) :: "
                + ImsCallMediaUtils.toString(toProfile));

        if (checkAndRejectVideoCallControl(fromProfile, toProfile)) {
            mVideoCallProvider.receiveSessionModifyResponse(
                    Connection.VideoProvider.SESSION_MODIFY_REQUEST_INVALID,
                    toProfile, fromProfile);

            Toast.makeText(mCallContext.getContext(), mCallContext.getContext().getString(
                    R.string.turning_off_camera_not_supported), Toast.LENGTH_SHORT).show();

            if (mEventListener != null) {
                mEventListener.onSessionModificationAbortedByCameraOff();
            }
            return;
        }

        VideoProfile proposalProfile = ImsCallMediaUtils.cloneVideoProfile(toProfile);

        setProposalProfile(proposalProfile);

        if (proposalProfile == null) {
            log("Invalid request profile");
            mVideoCallProvider.receiveSessionModifyResponse(
                    Connection.VideoProvider.SESSION_MODIFY_REQUEST_INVALID,
                    null, fromProfile);
            return;
        }

        ImsCallProfile callProfile = getCallProfile();
        ImsStreamMediaProfile mediaProfile = createProposalMedia(proposalProfile, true);
        int callType = (callProfile != null) ?
                callProfile.getCallType() : ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO;

        if (ImsCallUtils.isVoiceCall(callType)
                && (mediaProfile.getVideoQuality() != ImsStreamMediaProfile.VIDEO_QUALITY_NONE)) {
            callType = ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE;
            setSessionModificationType(MODIFICATION_CALL_TYPE);
        } else if (ImsCallUtils.isVideoCall(callType)
                && (mediaProfile.getVideoQuality() == ImsStreamMediaProfile.VIDEO_QUALITY_NONE)) {
            callType = ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO;
            setSessionModificationType(MODIFICATION_CALL_TYPE);
        } else {
            // Changes the video profile only
            setSessionModificationType(MODIFICATION_VIDEO_PROFILE);
            // FIXME : Is operator checked required.?
            if (handleVideoProfileUpdate(fromProfile, toProfile, callProfile)) {
                return;
            }
            mediaProfile = createProposalMedia(getProposalProfile(), true);
        }

        try {
            setUpdateState(UPDATE_STATE_SENT);
            mCallSession.update(callType, mediaProfile);
        } catch (Throwable t) {
            loge(t.toString(), t);
        }
    }

    public boolean handleVideoProfileUpdate(VideoProfile fromProfile,
            VideoProfile toProfile, ImsCallProfile callProfile) {
        ImsStreamMediaProfile mediaProfile = callProfile.getMediaProfile();
        int audioDirection = mediaProfile.getAudioDirection();
        int videoDirection = mediaProfile.getVideoDirection();
        int fromVideoState = fromProfile.getVideoState();
        int toVideoState = toProfile.getVideoState();
        boolean isTurnOffCameraRequest = isTurnOffCameraRequest(fromVideoState, toVideoState);
        boolean isTurnOnCameraRequest = isTurnOnCameraRequest(fromVideoState, toVideoState);

        log("handleVideoProfileUpdate :: " + "AudioDirection:" + audioDirection
                + ", VideoDirection:" + videoDirection + ", FromVideoState:" + fromVideoState
                + ", ToVideoState:" + toVideoState + ", IsCameraTurnOffRequest:"
                + isTurnOffCameraRequest + ", IsCameraTurnOnRequest:" + isTurnOnCameraRequest);

        if (isTurnOnCameraRequest) {
            setCameraSetting(CAMERA_ON);
        } else if (isTurnOffCameraRequest) {
            setCameraSetting(CAMERA_OFF);
        }

        /* Media direction will be `inactive` in below cases
         * 1. Device is held by remote.
         * 2. Device is in pause state when MT device goes in background.
         * In above both cases Device must not send any SIP signaling when
         * 1. Device turns camera on/off
         * 2. Device goes to background/foreground
         */
        if ((isTurnOffCameraRequest || isTurnOnCameraRequest
                || isPauseOrResumeRequest(fromVideoState, toVideoState))
                    && (isHeldByRemote(audioDirection, videoDirection) || isPausedByRemote())) {
            log("Ignore camera on/off when video direction is inactive");
            /* No SIP signaling, accept the request with
             * (@link Connection.VideoProvide#SESSION_MODIFY_REQUEST_SUCCESS}
             * Dialer UI changes when camera on/off button press.
             */
            clearSessionModificationInfo();
            finalizeSessionModification();
            mVideoCallProvider.receiveSessionModifyResponse(
                    Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS, toProfile, toProfile);
            return true;
        } else if (isResumeRequest(fromVideoState, toVideoState) && !isCameraOn()) {
            /* When camera is off and device comes to foreground from background,
             * video direction must be recvonly.
             */
            log("come to foreground with camera off");
            VideoProfile profile = new VideoProfile(VideoProfile.STATE_RX_ENABLED,
                    toProfile.getQuality());
            setProposalProfile(profile);
        }

        return false;
    }

    private void setPausedByRemote(boolean pausedByRemote) {
        mIsPausedByRemote = pausedByRemote;
    }

    private boolean isPausedByRemote() {
        return mIsPausedByRemote;
    }

    private static boolean isPauseOrResumeRequest(int from, int to) {
        return isPauseRequest(from, to) || isResumeRequest(from, to);
    }

    private static boolean isHeldByRemote(int audioDirection, int videoDirection) {
        return (audioDirection == ImsStreamMediaProfile.DIRECTION_RECEIVE
                && videoDirection == ImsStreamMediaProfile.DIRECTION_INACTIVE);
    }

    /**
     * Determines if this request includes turning the camera off (i.e. turning off transmission).
     *
     * @param from the from video state.
     * @param to the to video state.
     * @return {@code true} if the state change disables the user's camera.
     */
    private static boolean isTurnOffCameraRequest(int from, int to) {
        if (!isPauseRequest(from, to)) {
            return VideoProfile.isTransmissionEnabled(from)
                    && !VideoProfile.isTransmissionEnabled(to);
        }

        return false;
    }

    /**
     * Determines if this request includes turning the camera on (i.e. turning on transmission).
     *
     * @param from the from video state.
     * @param to the to video state.
     * @return {@code true} if the state change enables the user's camera.
     */
    private static boolean isTurnOnCameraRequest(int from, int to) {
        if (!isResumeRequest(from, to)) {
            return !VideoProfile.isTransmissionEnabled(from)
                    && VideoProfile.isTransmissionEnabled(to);
        }

        return false;
    }

    /**
     * Determines if a session modify request represents a request to resume the video.
     *
     * @param from The from video state.
     * @param to The to video state.
     * @return {@code true} if a resume was requested, {@code false} otherwise.
     */
    private static boolean isResumeRequest(int from, int to) {
        return VideoProfile.isPaused(from) && !VideoProfile.isPaused(to);
    }

    /**
     * Determines if a session modify request represents a request to pause the video.
     *
     * @param from The from video state.
     * @param to The to video state.
     * @return {@code true} if a pause was requested.
     */
    private static boolean isPauseRequest(int from, int to) {
        return !VideoProfile.isPaused(from) && VideoProfile.isPaused(to);
    }

    @Override
    public void sendSessionModifyResponse(VideoProfile responseProfile) {
        int modificationType = getSessionModificationType();
        ImsCallProfile callProfile = getCallProfile();

        if (callProfile == null) {
            // FIXME:
            rejectSessionModification(ImsReasonInfo.CODE_USER_DECLINE);
            clearSessionModificationInfo();

            if (modificationType == MODIFICATION_CALL_TYPE) {
                sendSessionModifyRejectCompleted();
            }
            return;
        }

        logi("SessionModification-Response(SEND) :: "
                + ImsCallMediaUtils.toString(responseProfile));

        if (responseProfile == null) {
            rejectSessionModification(ImsReasonInfo.CODE_USER_DECLINE);
            clearSessionModificationInfo();

            if (modificationType == MODIFICATION_CALL_TYPE) {
                sendSessionModifyRejectCompleted();
            }
            return;
        }

        ImsStreamMediaProfile mediaProfile = createProposalMedia(responseProfile, false);
        int callType = callProfile.getCallType();

        if (ImsCallUtils.isVoiceCall(callType)
                && (mediaProfile.getVideoQuality() != ImsStreamMediaProfile.VIDEO_QUALITY_NONE)) {
            callType = ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE;
        } else if (ImsCallUtils.isVideoCall(callType)
                && (mediaProfile.getVideoQuality() == ImsStreamMediaProfile.VIDEO_QUALITY_NONE)) {
            callType = ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO;
        }

        boolean callTypeChanged = false;

        if (modificationType == MODIFICATION_CALL_TYPE) {
            callTypeChanged = ImsCallUtils.isCallTypeChanged(callProfile.getCallType(), callType);
            if (callTypeChanged) {
                acceptSessionModification(callType, mediaProfile);
            } else {
                rejectSessionModification(ImsReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION);
            }
        } else if (modificationType == MODIFICATION_VIDEO_PROFILE) {
            // FIXME: how to identify if the response video profile is for accept?

            /* When remote device goes in background local device will be in paused state.
             * Update the device paused state by remote.
             */
            setPausedByRemote(mediaProfile.getAudioDirection()
                    == ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE
                    && VideoProfile.isPaused(responseProfile.getVideoState()));
            acceptSessionModification(callType, mediaProfile);
        } else {
            acceptSessionModification(callType, mediaProfile);
        }

        clearSessionModificationInfo();

        if ((modificationType == MODIFICATION_CALL_TYPE) && !callTypeChanged) {
            sendSessionModifyRejectCompleted();
        }
    }

    @Override
    public void setCameraSetting(int setting) {
        // FIXME: is operator check required?
        if (mCameraSetting != setting) {
            logi("setCameraSetting :: " + mCameraSetting + " >> " + setting);
            mCameraSetting = setting;
        }
    }

    @Override
    public void setMultitaskingState(int state) {
        // FIXME: is operator check required?
        if (mMultitaskingState != state) {
            logi("setMultitaskingState :: " + mMultitaskingState + " >> " + state);
            mMultitaskingState = state;
        }
    }

    @Override
    public void setVideoCallProvider(ImsVideoCallProvider provider) {
        mVideoCallProvider = provider;
    }

    @Override
    public void setEventListener(EventListener listener) {
        mEventListener = listener;
    }

    public void finalizeSessionModification() {
        setUpdateState(UPDATE_STATE_IDLE);
    }

    public void handleCallSessionEvent(int mediaEvent) {
        // IUMtcCall.INFO_TYPE_MEDIA_VIDEO_LOWEST_BIT_RATE
        // IUMtcCall.INFO_TYPE_MEDIA_VIDEO_NO_DATA
        logi("handleCallSessionEvent: ignored - mediaEvent=" + mediaEvent);
    }

    public boolean isCameraOn() {
        return (mCameraSetting == CAMERA_ON);
    }

    public boolean isMultitaskingState() {
        return (mMultitaskingState != MULTITASKING_NONE);
    }

    public boolean isSessionModificationFinalizing() {
        synchronized (this) {
            return (mUpdateState == UPDATE_STATE_FINALIZING);
        }
    }

    public boolean isSessionModificationInProgress() {
        synchronized (this) {
            return (mUpdateState == UPDATE_STATE_SENT)
                    || (mUpdateState == UPDATE_STATE_RECEIVED);
        }
    }

    public void notifyCallEvent(int event) {
        if (mEventListener != null) {
            mEventListener.onCallEvent(event);
        }
    }

    public void receiveSessionModifyRequest(int modificationType, MediaInfo mi) {
        VideoProfile requestedProfile = ImsCallMediaUtils.createVideoProfileFromMediaInfo(mi);

        // To identify that audio/video quality information is changed.
        setProposedStreamMediaProfile(ImsCallMediaUtils.createMediaProfileFromMediaInfo(mi));
        setProposalProfile(ImsCallMediaUtils.cloneVideoProfile(requestedProfile));
        setSessionModificationType(modificationType);
        setUpdateState(UPDATE_STATE_RECEIVED);

        logi("SessionModification-Request(RECV) :: "
                + ImsCallMediaUtils.toString(requestedProfile));

        mVideoCallProvider.receiveSessionModifyRequest(requestedProfile);
    }

    public void receiveSessionModifyResponse(int reasonInfoCode, MediaInfo mi) {
        VideoProfile responseProfile
                = (mi != null) ? ImsCallMediaUtils.createVideoProfileFromMediaInfo(mi) : null;
        VideoProfile requestedProfile = ImsCallMediaUtils.cloneVideoProfile(getProposalProfile());
        int status = Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS;
        int modificationType = getSessionModificationType();

        if ((modificationType == MODIFICATION_CALL_TYPE)
                && !isCallTypeChanged(requestedProfile, responseProfile)
                && (reasonInfoCode < 0)) {
            log("SessionModification :: Call type is not changed");
            status = Connection.VideoProvider.SESSION_MODIFY_REQUEST_FAIL;
        }

        if ((reasonInfoCode < 0)
                && (requestedProfile != null) && (responseProfile != null)
                && (requestedProfile.getVideoState() != responseProfile.getVideoState())) {
            status = Connection.VideoProvider.SESSION_MODIFY_REQUEST_REJECTED_BY_REMOTE;
        }

        clearSessionModificationInfo();
        finalizeSessionModification();

        if (reasonInfoCode >= ImsReasonInfo.CODE_UNSPECIFIED) {
            status = Connection.VideoProvider.SESSION_MODIFY_REQUEST_FAIL;

            if (reasonInfoCode == ImsReasonInfo.CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE) {
                status = Connection.VideoProvider.SESSION_MODIFY_REQUEST_TIMED_OUT;
            } else if (reasonInfoCode == ImsReasonInfo.CODE_LOCAL_ILLEGAL_ARGUMENT) {
                status = Connection.VideoProvider.SESSION_MODIFY_REQUEST_INVALID;
            } else if (reasonInfoCode == ImsReasonInfo.CODE_SIP_USER_REJECTED) {
                status = Connection.VideoProvider.SESSION_MODIFY_REQUEST_REJECTED_BY_REMOTE;
            }
        }

        /** FIXME: Is it correct to set the old profile when the session modification is failed?
        if ((mProfileBeforeRequest != null)
                && (status != Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS)) {
            // responseProfile = mProfileBeforeRequest;
        } */

        logi("SessionModification-Response(RECV) :: errorCode="
                + reasonInfoCode + ", status=" + status
                + ", request=" + ImsCallMediaUtils.toString(requestedProfile)
                + ", response=" + ImsCallMediaUtils.toString(responseProfile));

        mVideoCallProvider.receiveSessionModifyResponse(
                status, requestedProfile, responseProfile);
    }

    @VisibleForTesting
    public void setStateAndType(int updateState, int modificationType) {
        mUpdateState = updateState;
        mModificationType = modificationType;
    }

    @VisibleForTesting
    public boolean isClearedSessionModificationInfo() {
        if ((mProfileBeforeRequest == null) && (mProposalProfile == null)
                && (mProposalMediaProfile == null) && (mModificationType == MODIFICATION_NONE)) {
            return true;
        }
        return false;
    }

    private void sendSessionModifyRejectCompleted() {
        logi("SessionModification-RejectCompleted");

        // Notify that video upgrade request is completely rejected.
        mVideoCallProvider.receiveSessionModifyResponse(
                Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS,
                null, new VideoProfile(VideoProfile.STATE_AUDIO_ONLY));
    }

    private void clearSessionModificationInfo() {
        mProfileBeforeRequest = null;

        setProposalProfile(null);
        setSessionModificationType(MODIFICATION_NONE);
        setProposedStreamMediaProfile(null);
    }

    private boolean checkAndRejectVideoCallControl(
            VideoProfile fromProfile, VideoProfile toProfile) {
        if (!ImsCallUtils.isCallOnNativeAppsAndCountryKR(mCallContext)) {
            return false;
        }

        int fromState = (fromProfile != null) ? fromProfile.getVideoState()
                : VideoProfile.STATE_BIDIRECTIONAL;
        int toState = (toProfile != null) ? toProfile.getVideoState()
                : VideoProfile.STATE_BIDIRECTIONAL;

        if (VideoProfile.isBidirectional(fromState)) {
            if ((!VideoProfile.isBidirectional(toState)
                    && (VideoProfile.isTransmissionEnabled(toState)
                        || VideoProfile.isReceptionEnabled(toState)))
                    || VideoProfile.isPaused(toState)) {
                log("checkAndRejectVideoCallControl :: rejected - from="
                        + ImsCallMediaUtils.toString(fromProfile)
                        + ", to=" + ImsCallMediaUtils.toString(toProfile));
                return true;
            }
        }

        return false;
    }

    private ImsStreamMediaProfile createProposalMedia(VideoProfile profile,
            boolean sessionModificationRequest) {
        ImsCallProfile callProfile = getCallProfile();

        if (callProfile == null) {
            return new ImsStreamMediaProfile(ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                    ImsStreamMediaProfile.DIRECTION_INVALID,
                    ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                    ImsStreamMediaProfile.DIRECTION_INVALID,
                    ImsStreamMediaProfile.RTT_MODE_DISABLED);
        }

        ImsStreamMediaProfile mediaProfile = callProfile.getMediaProfile();
        ImsStreamMediaProfile proposalMediaProfile = getProposedStreamMediaProfile();
        int audioQuality = mediaProfile.getAudioQuality();
        int videoQuality = mediaProfile.getVideoQuality();
        int videoDirection = mediaProfile.getVideoDirection();

        // Overwrites the audio/video quality information from the proposed media profile
        if (!sessionModificationRequest && (proposalMediaProfile != null)) {
            if (mediaProfile.getAudioQuality() != proposalMediaProfile.getAudioQuality()) {
                log("MediaProfile :: audioQuality - " + mediaProfile.getAudioQuality()
                        + " >> " + proposalMediaProfile.getAudioQuality());
                audioQuality = proposalMediaProfile.getAudioQuality();
            }

            if (mediaProfile.getVideoQuality() != proposalMediaProfile.getVideoQuality()) {
                log("MediaProfile :: videoQuality - " + mediaProfile.getVideoQuality()
                        + " >> " + proposalMediaProfile.getVideoQuality());
                videoQuality = proposalMediaProfile.getVideoQuality();
            }
        }

        int direction = ImsCallMediaUtils.getDirectionFromVideoProfileForMediaInfo(
                profile.getVideoState());

        if (direction == MediaInfo.DIRECTION_INVALID) {
            // Audio only
            videoDirection = ImsStreamMediaProfile.DIRECTION_INVALID;
            videoQuality = ImsStreamMediaProfile.VIDEO_QUALITY_NONE;
        } else {
            videoDirection
                    = ImsCallMediaUtils.getDirectionFromMediaInfoForMediaProfile(direction);

            // Keep the current preferred media quality

            // If the current call is a voice call and the device receives the upgrade operation,
            // then we need to set the video quality to the maximum level.
            if (videoQuality == ImsStreamMediaProfile.VIDEO_QUALITY_NONE) {
                videoQuality = ImsStreamMediaProfile.VIDEO_QUALITY_VGA_PORTRAIT;
            }
        }

        return new ImsStreamMediaProfile(audioQuality,
                mediaProfile.getAudioDirection(), videoQuality, videoDirection,
                mediaProfile.getRttMode());
    }

    private ImsCallProfile getCallProfile() {
        try {
            return mCallSession.getCallProfile();
        } catch (Throwable t) {
            loge("CallProfile Exception:" + t.toString(), t);
        }

        return null;
    }

    private void acceptSessionModification(int callType, ImsStreamMediaProfile mediaProfile) {
        try {
            // FIXME: needs to be checked if the video property is changed
            int oldState = mCallSession.getState();

            mCallSession.accept(callType, mediaProfile);

            if (oldState != ImsCallSession.State.ESTABLISHED) {
                setUpdateState(UPDATE_STATE_FINALIZING);
            } else {
                log("acceptSessionModification :: ignored...");
            }
        } catch (Throwable t) {
            loge("acceptSessionModification Exception:" + t.toString(), t);
        }
    }

    private void rejectSessionModification(int reason) {
        try {
            int oldState = mCallSession.getState();

            mCallSession.reject(reason);

            if (oldState != ImsCallSession.State.ESTABLISHED) {
                setUpdateState(UPDATE_STATE_FINALIZING);
            } else {
                log("rejectSessionModification :: ignored...");
            }
        } catch (Throwable t) {
            loge("rejectSessionModification Exception:" + t.toString(), t);
        }
    }

    private void setProposedStreamMediaProfile(ImsStreamMediaProfile profile) {
        synchronized (this) {
            mProposalMediaProfile = profile;
        }
    }

    private void setUpdateState(int state) {
        synchronized (this) {
            if ((mUpdateState == UPDATE_STATE_IDLE)
                    && (state == UPDATE_STATE_FINALIZING)) {
                log("ImsVideoCallSession :: FINALIZING on IDLE - Ignored");
                return;
            }

            if (mUpdateState != state) {
                logi("ImsVideoCallSession :: " + updateStateToString(mUpdateState)
                        + " >> " + updateStateToString(state));

                mUpdateState = state;
            }
        }
    }

    private VideoProfile getProposalProfile() {
        synchronized (this) {
            return mProposalProfile;
        }
    }

    private void setProposalProfile(VideoProfile profile) {
        synchronized (this) {
            mProposalProfile = profile;
        }
    }

    private void setSessionModificationType(int type) {
        synchronized (this) {
            if (mModificationType != type) {
                log("ImsVideoCallSession(mod-type) :: "
                        + modificationTypeToString(mModificationType)
                        + " >> " + modificationTypeToString(type));

                mModificationType = type;
            }
        }
    }

    private static boolean isCallTypeChanged(VideoProfile request, VideoProfile response) {
        if ((request == null) || (response == null)) {
            return false;
        }

        int offeredCallType = ImsCallProfile.getCallTypeFromVideoState(request.getVideoState());
        int answeredCallType = ImsCallProfile.getCallTypeFromVideoState(response.getVideoState());

        return !ImsCallUtils.isCallTypeChanged(offeredCallType, answeredCallType);
    }

    private static String modificationTypeToString(int type) {
        switch (type) {
            case MODIFICATION_NONE:
                return "NONE";
            case MODIFICATION_CALL_TYPE:
                return "CALL_TYPE";
            case MODIFICATION_VIDEO_PROFILE:
                return "VIDEO_PROFILE";
            default:
                return "UNKNOWN";
        }
    }

    private static String updateStateToString(int state) {
        switch (state) {
            case UPDATE_STATE_IDLE:
                return "IDLE";
            case UPDATE_STATE_SENT:
                return "SENT";
            case UPDATE_STATE_RECEIVED:
                return "RECEIVED";
            case UPDATE_STATE_FINALIZING:
                return "FINALIZING";
            default:
                return "UNKNOWN";
        }
    }

    private static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }

    private void loge(String message, Throwable t) {
        ImsLog.e("[GII-IMPL] " + message, t);
    }
}
