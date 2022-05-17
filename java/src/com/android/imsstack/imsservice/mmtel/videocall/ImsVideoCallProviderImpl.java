/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20150329    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.imsservice.mmtel.videocall;

import android.os.Handler;
import android.telecom.Connection;
import android.telecom.VideoProfile;
import android.telephony.ims.ImsStreamMediaProfile;
import android.util.Size;
import android.view.Surface;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.enabler.mtc.IUMtcMedia;
import com.android.imsstack.enabler.mtc.MtcCallUtils;
import com.android.imsstack.enabler.mtc.MtcMediaSession;
import com.android.imsstack.imsservice.mmtel.ImsCallUtils;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.call.IVideoCallSession;
import com.android.imsstack.imsservice.mmtel.util.VideoDimension;
import com.android.imsstack.imsservice.mmtel.videocall.base.ImsCamera;
import com.android.imsstack.imsservice.mmtel.videocall.base.ImsVideoCallProviderBase;
import com.android.imsstack.imsservice.mmtel.videocall.base.VideoCallUtils;
import com.android.imsstack.util.ImsConstants;

/** IMS extended interface implementation */
public class ImsVideoCallProviderImpl extends ImsVideoCallProviderBase {
    // milli-seconds
    private static final int WAIT_TIME_FOR_UI_CLEARING = 4000;
    private static final int WAIT_TIME_FOR_UI_NOTIFICATION = 400;
    private final Object mLock = new Object();
    private Surface mCachedPreviewSurface;
    private Surface mCachedDisplaySurface;
    private String mCameraId = null;
    private Runnable mStopPreviewRunner = null;
    private Runnable mMultitaskingRunner = null;
    private boolean mMultitaskingStarted = false;
    private int mSessionModificationType = IVideoCallSession.MODIFICATION_NONE;
    private boolean mFirstPeerDisplayOrientationChanged = false;
    private boolean mPreviewStartedAtOnce = false;

    public ImsVideoCallProviderImpl(IVideoCallSession callSession,
            MtcMediaSession mediaSession) {
        super(callSession, mediaSession);
    }

    @Override
    public void onSetPreviewSurface(Surface surface) {
        super.onSetPreviewSurface(surface);

        if (isVideoCallFor3rdPartyDialer()) {
            synchronized (mLock) {
                if (getCallState() < CALL_STATE_ESTABLISHED) {
                    mCachedPreviewSurface = surface;
                }

                if ((surface != null) && !mPreviewStartedAtOnce) {
                    if (canPreviewStarted()) {
                        mPreviewStartedAtOnce = startPreview(surface);
                    } else {
                        logi("Ignore start preview because call is already connected");
                    }
                }
            }
        }
    }

    @Override
    public void onSetDisplaySurface(Surface surface) {
        super.onSetDisplaySurface(surface);

        if (isVideoCallFor3rdPartyDialer()) {
            synchronized (mLock) {
                if (getCallState() == CALL_STATE_IDLE) {
                    mCachedDisplaySurface = surface;
                }
            }
        }
    }

    @Override
    public void onSetCamera(String cameraId) {
        logi("onSetCamera :: id=" + cameraId);

        if (isVideoCallFor3rdPartyDialer()) {
            if (cameraId == null) {
                mCameraId = null;
                // Start multitasking except for camera-off scenario.
                // The interval is to check whether this situation is triggered
                // by turning off the camera.
                startMultitaskingDelayed(200);

                synchronized (mLock) {
                    abortAndRunDelayedStopPreview(true);
                }
                return;
            } else {
                // Stop multitasking.
                stopMultitasking();
            }
        }

        super.onSetCamera(cameraId);

        if (cameraId != null) {
            int camId = getCameraIdInt(cameraId);

            if (camId >= ImsCamera.CAMERA_REAR) {
                mCameraId = cameraId;
            }

            if (isVideoCallFor3rdPartyDialer()) {
                if (!mPreviewStartedAtOnce
                        && (mCachedPreviewSurface != null)
                        && canPreviewStarted()) {
                    mPreviewStartedAtOnce = startPreview(mCachedPreviewSurface);
                }
            }
        }
    }

    @Override
    public void onRequestCameraCapabilities() {
        if (isVideoCallFor3rdPartyDialer()) {
            if (mCameraId == null) {
                log("No camera selected");
                return;
            }

            VideoDimension vd = getVideoDimensionForPreview();

            ImsCamera camera = new ImsCamera(mCameraId);
            float maxZoom = camera.getMaxZoom();
            boolean zoomSupported = camera.isZoomSupported(maxZoom);
            Size size = camera.getPreviewSize();

            int width = (vd != null) ? vd.getWidth() : size.getWidth();
            int height = (vd != null) ? vd.getHeight() : size.getHeight();

            logi("ImsCamera :: previewSize=" + size
                    + ", maxZoom=" + maxZoom
                    + ", zoomSupported=" + zoomSupported
                    + ", " + ((vd != null) ? vd.toString() : "(null)"));

            VideoProfile.CameraCapabilities cc
                    = new VideoProfile.CameraCapabilities(width, height, zoomSupported, maxZoom);

            changeCameraCapabilities(cc);
            return;
        }

        super.onRequestCameraCapabilities();
    }

    @Override
    public void receiveSessionModifyRequest(VideoProfile videoProfile) {
        IVideoCallSession callSession = getVideoCallSession();
        boolean callTypeChangeRequest = (callSession != null)
                && (callSession.getSessionModificationType()
                    == IVideoCallSession.MODIFICATION_CALL_TYPE);
        boolean isAudioOnly = (videoProfile != null)
                && videoProfile.isAudioOnly(videoProfile.getVideoState());

        if (isDynamicVideoQualitySupportedOnSessionModification()) {
            // Notify video resolution first before passing video upgrade request
            if (callTypeChangeRequest && !isAudioOnly) {
                notifyPeerDimensionsChanged(callSession.getProposedStreamMediaProfile());
            }
        }

        boolean notificationDelayRequired = false;

        if (isVideoCallFor3rdPartyDialer()) {
            if (callTypeChangeRequest) {
                if (isAudioOnly) {
                    if (!isTPhone()) {
                        // Auto acceptance for voice call downgrade
                        Handler h = getCallHandler();

                        h.post(new Runnable() {
                            @Override
                            public void run() {
                                logi("Voice call is automatically accepted");
                                onSendSessionModifyResponse(
                                        new VideoProfile(VideoProfile.STATE_AUDIO_ONLY));
                            }
                        });
                        return;
                    }
                } else {
                    logi("Prepare preview for video call upgrade request (RX)");

                    synchronized (mLock) {
                        // If stopPreview requires some milli-seconds to wait for closing camera,
                        // so, the session modification request is handled with some delay
                        // to preserve startPreview.
                        notificationDelayRequired = abortAndRunDelayedStopPreview(true);

                        setCallState(CALL_STATE_VIDEO_UPGRADE_REQUESTED);
                        mPreviewStartedAtOnce = false;
                    }
                }
            }
        }

        if (notificationDelayRequired) {
            final VideoProfile receivedProfile = videoProfile;
            Handler h = getCallHandler();

            // UI notification is delayed to preserve startPreview
            // when video call upgrade request is received.
            // It waits for average 400ms.
            h.postDelayed(new Runnable() {
                @Override
                public void run() {
                    logi("receiveSessionModifyRequest :: delayed");
                    ImsVideoCallProviderImpl.super.receiveSessionModifyRequest(receivedProfile);
                }
            }, WAIT_TIME_FOR_UI_NOTIFICATION);
            return;
        }

        super.receiveSessionModifyRequest(videoProfile);
    }

    @Override
    public void receiveSessionModifyResponse(
            int status, VideoProfile requestedProfile, VideoProfile responseProfile) {
        if (isDynamicVideoQualitySupportedOnSessionModification()) {
            IVideoCallSession callSession = getVideoCallSession();

            // Notify video resolution first before passing video upgrade result
            if ((callSession != null)
                    && (status == Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS)
                    && (mSessionModificationType == IVideoCallSession.MODIFICATION_CALL_TYPE)
                    && (responseProfile != null)
                    && !responseProfile.isAudioOnly(responseProfile.getVideoState())) {
                notifyPeerDimensionsChanged(callSession.getStreamMediaProfile());
            }
        }

        super.receiveSessionModifyResponse(status, requestedProfile, responseProfile);

        mSessionModificationType = IVideoCallSession.MODIFICATION_NONE;

        synchronized (mLock) {
            if (isVideoCallFor3rdPartyDialer()) {
                if ((getCallState() == CALL_STATE_VIDEO_UPGRADE_REQUESTED)
                        && ((responseProfile == null)
                                || VideoProfile.isAudioOnly(responseProfile.getVideoState()))) {
                    // Session modification is rejected by local endpoint.
                    if (status == Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS) {
                        stopPreview();
                    } else {
                        abortAndRunDelayedStopPreview(false);

                        mStopPreviewRunner = new Runnable() {
                            @Override
                            public void run() {
                                logi("stopPreviewDelayed");
                                stopPreview();

                                synchronized (mLock) {
                                    mStopPreviewRunner = null;
                                }
                            }
                        };

                        Handler h = getCallHandler();
                        h.postDelayed(mStopPreviewRunner, WAIT_TIME_FOR_UI_CLEARING);
                    }
                }
            }

            setCallState(CALL_STATE_ESTABLISHED);
            mPreviewStartedAtOnce = true;
        }
    }

    @Override
    public void onSendSessionModifyRequest(VideoProfile fromProfile, VideoProfile toProfile) {
        super.onSendSessionModifyRequest(fromProfile, toProfile);

        IVideoCallSession callSession = getVideoCallSession();

        if (callSession != null) {
            mSessionModificationType = callSession.getSessionModificationType();

            if (isVideoCallFor3rdPartyDialer()
                    && (mSessionModificationType == IVideoCallSession.MODIFICATION_CALL_TYPE)
                    && !toProfile.isAudioOnly(toProfile.getVideoState())) {
                logi("Prepare preview for video call upgrade request (TX)");

                synchronized (mLock) {
                    setCallState(CALL_STATE_VIDEO_UPGRADE_REQUESTED);
                    mPreviewStartedAtOnce = false;
                }
            }
        }
    }

    @Override
    public void onSendSessionModifyResponse(VideoProfile responseProfile) {
        super.onSendSessionModifyResponse(responseProfile);

        synchronized (mLock) {
            if (isVideoCallFor3rdPartyDialer()) {
                if ((getCallState() == CALL_STATE_VIDEO_UPGRADE_REQUESTED)
                        && ((responseProfile == null)
                                || VideoProfile.isAudioOnly(responseProfile.getVideoState()))) {
                    stopPreview();
                }
            }

            setCallState(CALL_STATE_ESTABLISHED);
            mPreviewStartedAtOnce = true;
        }
    }

    public void changeCallDataUsage(long dataSize) {

        log("changeCallDataUsage :: " + dataSize);

        super.changeCallDataUsage(dataSize);
    }

    @Override
    public void close() {
        stopMultitasking();

        super.close();

        synchronized (mLock) {
            mCachedPreviewSurface = null;
            mCachedDisplaySurface = null;

            abortAndRunDelayedStopPreview(false);
        }
    }

    @Override
    public void onSessionModificationAbortedByCameraOff() {
        stopMultitasking();

        super.onSessionModificationAbortedByCameraOff();
    }

    @Override
    protected void handleCallEvent(int event) {
        synchronized (mLock) {
            super.handleCallEvent(event);

            int callState = getCallState();

            if (callState == CALL_STATE_INITIATING) {
                boolean logging = false;

                if (mCachedDisplaySurface != null) {
                    super.onSetDisplaySurface(mCachedDisplaySurface);
                    mCachedDisplaySurface = null;
                    logging = true;
                }

                if (mCachedPreviewSurface != null) {
                    super.onSetPreviewSurface(mCachedPreviewSurface);

                    if (!mPreviewStartedAtOnce) {
                        mPreviewStartedAtOnce = startPreview(mCachedPreviewSurface);
                    }

                    if (mPreviewStartedAtOnce) {
                        mCachedPreviewSurface = null;
                    }

                    logging = true;
                }

                if (logging) {
                    logi("Set surface by call initiation");
                }
            } else if (callState == IVideoCallSession.EVENT_CALL_ESTABLISHED) {
                mPreviewStartedAtOnce = true;
                mCachedPreviewSurface = null;
                mCachedDisplaySurface = null;
            }
        }
    }

    @Override
    protected void handleMediaSessionStarted() {
        enforcePeerDimensionsChanged();
        super.handleMediaSessionStarted();
    }

    @Override
    protected void handleMediaSessionMediaInfoChanged(
            int mediaInfo, int intParam, String strParam) {
        if (isVideoCallFor3rdPartyDialer()) {
            if (mediaInfo == MtcCallUtils.INFO_TYPE_MEDIA_VIDEO_NO_DATA) {
                handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_RX_PAUSE);
            } else if (mediaInfo == MtcCallUtils.INFO_TYPE_MEDIA_VIDEO_DATA_RECEIVED) {
                handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_RX_RESUME);
            }
            return;
        }

        if (mediaInfo == MtcCallUtils.INFO_TYPE_MEDIA_CVO_CAPABILITY) {
            if (intParam == MtcCallUtils.MEDIA_CVO_DISABLED) {
                // TODO: how to handle this?
            } else if (intParam == MtcCallUtils.MEDIA_CVO_ENABLED) {
                // TODO: how to handle this?
            }
            return;
        }

        super.handleMediaSessionMediaInfoChanged(mediaInfo, intParam, strParam);
    }

    @Override
    protected void handleMediaSessionPeerFirstVideoReceived() {
        if (isVideoCallFor3rdPartyDialer()) {
            handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_RX_RESUME);
            return;
        }

        super.handleMediaSessionPeerFirstVideoReceived();
    }

    @Override
    protected void handleMediaSessionPeerDisplayOrientationChanged(final int orientation) {
        if (isVideoCallFor3rdPartyDialer()) {
            boolean enforceUpdate = false;

            if (!mFirstPeerDisplayOrientationChanged) {
                mFirstPeerDisplayOrientationChanged = true;
                enforceUpdate = true;
                handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_RX_RESUME);
            }

            int changedOrientation = VideoCallUtils.getOrientationFromMtcMediaSession(orientation);

            if (getCurrentVideoDimension() != null) {
                updateReversedPeerDimensionFromVideoDimension(changedOrientation, enforceUpdate);
            } else {
                updateReversedPeerDimensionFromMediaProfile(changedOrientation, enforceUpdate);
            }
            return;
        }

        super.handleMediaSessionPeerDisplayOrientationChanged(orientation);
    }

    @Override
    protected void handleMediaSessionPeerDimensionsChanged(final int videoResolution) {
        if (isVideoCallFor3rdPartyDialer()) {
            int width = getVideoWidth(videoResolution);
            int height = getVideoHeight(videoResolution);

            if ((width > 0) && (height > 0)) {
                setCurrentVideoDimension(width, height);
                changePeerDimensions(width, height);
            }
            return;
        }

        super.handleMediaSessionPeerDimensionsChanged(videoResolution);
    }

    @Override
    protected void handleMediaSessionSelectCameraCompleted(final int result) {
        if (isVideoCallFor3rdPartyDialer()) {
            if (result == IUMtcMedia.Reason.REASON_NOERROR) {
                handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_CAMERA_READY);
            } else {
                handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_CAMERA_FAILURE);
            }
            return;
        }

        super.handleMediaSessionSelectCameraCompleted(result);
    }

    @Override
    protected void handleMediaSessionStartBackgroundCompleted(final int result) {
        if (isVideoCallFor3rdPartyDialer()) {
            if (result == IUMtcMedia.Reason.REASON_NOERROR) {
                handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_RX_PAUSE);
            } else {
                handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_CAMERA_FAILURE);
            }
            return;
        }

        super.handleMediaSessionStartBackgroundCompleted(result);
    }

    @Override
    protected void handleMediaSessionStopBackgroundCompleted(final int result) {
        if (isVideoCallFor3rdPartyDialer()) {
            if (result == IUMtcMedia.Reason.REASON_NOERROR) {
                handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_RX_RESUME);
            } else {
                handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_CAMERA_FAILURE);
            }
            return;
        }

        super.handleMediaSessionStopBackgroundCompleted(result);
    }

    private boolean abortAndRunDelayedStopPreview(boolean doPendingStopPreview) {
        if (mStopPreviewRunner != null) {
            Handler h = getCallHandler();
            boolean hasPendingOperation = h.hasCallbacks(mStopPreviewRunner);

            if (hasPendingOperation) {
                h.removeCallbacks(mStopPreviewRunner);
            }

            mStopPreviewRunner = null;

            if (hasPendingOperation && doPendingStopPreview) {
                log("Run pending stopPreview");
                stopPreview();
                return true;
            }
        }

        return false;
    }

    private boolean canPreviewStarted() {
        int callState = getCallState();

        if (callState == CALL_STATE_INITIATING
                || callState == CALL_STATE_VIDEO_UPGRADE_REQUESTED) {
            return true;
        } else if (callState == CALL_STATE_ALERTING
                && mCachedDisplaySurface == null) {
            // MT call
            // In the first video call, it should be checked
            // whether preview start needs or not because of camera permission of call UI.
            return true;
        }

        return false;
    }

    private boolean isDynamicVideoQualitySupportedOnSessionModification() {
        IVideoCallSession callSession = getVideoCallSession();
        return ImsGlobal.isOperator(callSession.getCallContext().getSlotId(), "RJIL");
    }

    private Handler getCallHandler() {
        ICallContext callContext = getVideoCallSession().getCallContext();
        return (callContext != null) ? callContext.getCallHandler()
                : ImsGlobal.getInstance().getCallHandler();
    }

    private VideoDimension getVideoDimension(ImsStreamMediaProfile mediaProfile,
            boolean reversedDimension) {
        if (mediaProfile != null) {
            int videoQuality = VideoCallUtils.getVideoQualityFromMediaProfileForMediaInfo(
                    mediaProfile.getVideoQuality());

            if (reversedDimension) {
                return VideoCallUtils.getReversedVideoDimension(videoQuality);
            } else {
                return VideoCallUtils.getVideoDimension(videoQuality);
            }
        }

        return null;
    }

    private VideoDimension getVideoDimensionForPreview() {
        IVideoCallSession callSession = getVideoCallSession();
        ImsStreamMediaProfile mediaProfile = callSession.getStreamMediaProfile();
        return getVideoDimension(mediaProfile, true);
    }

    private void notifyPeerDimensionsChanged(ImsStreamMediaProfile mediaProfile) {
        VideoDimension vd = getVideoDimension(mediaProfile, false);

        if (vd != null) {
            log("changePeerDimensions :: " + vd);
            changePeerDimensions(vd.getWidth(), vd.getHeight());
        }
    }

    private void startMultitaskingDelayed(long millis) {
        if (!ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            return;
        }

        IVideoCallSession callSession = getVideoCallSession();
        Handler h = getCallHandler();

        synchronized (mLock) {
            if (mMultitaskingRunner != null) {
                h.removeCallbacks(mMultitaskingRunner);
                mMultitaskingRunner = null;
            }

            if (ImsCallUtils.isVoiceCall(callSession.getCallType())) {
                // On call switch case, do not start multitasking
                return;
            }

            mMultitaskingRunner = new Runnable() {
                @Override
                public void run() {
                    IVideoCallSession callSession = getVideoCallSession();

                    if (ImsCallUtils.isVoiceCall(callSession.getCallType())) {
                        // On call switch case, do not start multitasking
                        return;
                    }

                    logi("Starting multitasking...");

                    synchronized (mLock) {
                        mMultitaskingStarted = true;
                        onStartBackground();
                    }
                }
            };

            h.postDelayed(mMultitaskingRunner, millis);
        }
    }

    private void stopMultitasking() {
        synchronized (mLock) {
            if (mMultitaskingRunner != null) {
                Handler h = getCallHandler();
                h.removeCallbacks(mMultitaskingRunner);
                mMultitaskingRunner = null;
            }

            if (mMultitaskingStarted) {
                logi("Stopping multitasking...");
                mMultitaskingStarted = false;
                onStopBackground();
            }
        }
    }

    private boolean startPreview(Surface surface) {
        MtcMediaSession mediaSession = getMediaSession();

        if (mediaSession != null) {
            if (mCameraId != null) {
                mediaSession.startPreviewCamera(surface);
                return true;
            }
        }

        return false;
    }

    private void stopPreview() {
        MtcMediaSession mediaSession = getMediaSession();

        if (mediaSession != null) {
            mediaSession.stopPreview();
        }
    }

    private void enforcePeerDimensionsChanged() {
        IVideoCallSession callSession = getVideoCallSession();
        boolean callbackRequired = false;

        if (isVideoCallFor3rdPartyDialer()) {
            callbackRequired = true;
        } else if ((getCallState() < IVideoCallSession.EVENT_CALL_ESTABLISHED)
                && callSession.isVrbtEnabled()) {
            callbackRequired = true;
        }

        if (callbackRequired) {
            ImsStreamMediaProfile mediaProfile = callSession.getStreamMediaProfile();

            if (mediaProfile != null) {
                int videoQuality = VideoCallUtils.getVideoQualityFromMediaProfileForMediaInfo(
                        mediaProfile.getVideoQuality());

                logi("Enforce changePeerDimensions on media started; videoQuality="
                        + videoQuality);

                VideoDimension vd = VideoCallUtils.getVideoDimension(videoQuality);

                if (vd != null) {
                    setCurrentVideoDimension(vd.getWidth(), vd.getHeight());
                    changePeerDimensions(vd.getWidth(), vd.getHeight());
                }
            }
        }
    }
}
