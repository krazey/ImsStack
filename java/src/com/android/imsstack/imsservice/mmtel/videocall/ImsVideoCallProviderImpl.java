/**
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

package com.android.imsstack.imsservice.mmtel.videocall;

import android.os.Handler;
import android.os.Looper;
import android.telecom.Connection;
import android.telecom.VideoProfile;
import android.telephony.ims.ImsStreamMediaProfile;
import android.util.Size;

import com.android.imsstack.enabler.mtc.CallFeature;
import com.android.imsstack.enabler.mtc.MtcCallUtils;
import com.android.imsstack.enabler.mtc.MtcMediaSession;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.call.IVideoCallSession;
import com.android.imsstack.imsservice.mmtel.util.VideoDimension;
import com.android.imsstack.imsservice.mmtel.videocall.base.ImsCamera;
import com.android.imsstack.imsservice.mmtel.videocall.base.ImsVideoCallProviderBase;
import com.android.imsstack.imsservice.mmtel.videocall.base.VideoCallUtils;

/** IMS extended interface implementation */
public class ImsVideoCallProviderImpl extends ImsVideoCallProviderBase {
    // milli-seconds
    private static final int WAIT_TIME_FOR_UI_NOTIFICATION = 400;
    private final Object mLock = new Object();
    private String mCameraId = null;
    private int mSessionModificationType = IVideoCallSession.MODIFICATION_NONE;
    private boolean mFirstPeerDisplayOrientationChanged = false;

    public ImsVideoCallProviderImpl(IVideoCallSession callSession,
            MtcMediaSession mediaSession) {
        super(callSession, mediaSession);
    }

    @Override
    public void onSetCamera(String cameraId) {
        if (mMediaSession == null) {
            return;
        }
        mCameraId = cameraId;
        if (cameraId == null) {
            mMediaSession.selectCamera(CAMERA_ID_NONE);
        } else {
            int camId = getCameraIdInt(cameraId);
            if (camId >= ImsCamera.CAMERA_REAR) {
                mMediaSession.selectCamera(camId);
            }
        }
    }

    @Override
    public void onRequestCameraCapabilities() {
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

        VideoProfile.CameraCapabilities cc = new VideoProfile.CameraCapabilities(width, height,
                zoomSupported, maxZoom);

        changeCameraCapabilities(cc);
        return;
    }

    @Override
    public void receiveSessionModifyRequest(VideoProfile videoProfile) {
        IVideoCallSession callSession = getVideoCallSession();
        boolean callTypeChangeRequest = (callSession != null)
                && (callSession.getSessionModificationType()
                        == IVideoCallSession.MODIFICATION_CALL_TYPE);
        boolean isAudioOnly = (videoProfile != null)
                && VideoProfile.isAudioOnly(videoProfile.getVideoState());

        if (isDynamicVideoQualitySupportedOnSessionModification()) {
            // Notify video resolution first before passing video upgrade request
            if (callTypeChangeRequest && !isAudioOnly) {
                notifyPeerDimensionsChanged(callSession.getProposedStreamMediaProfile());
            }
        }

        boolean notificationDelayRequired = false;

        if (callTypeChangeRequest) {
            if (isAudioOnly) {
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
            } else {
                logi("Prepare preview for video call upgrade request (RX)");
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
                    && !VideoProfile.isAudioOnly(responseProfile.getVideoState())) {
                notifyPeerDimensionsChanged(callSession.getStreamMediaProfile());
            }
        }

        super.receiveSessionModifyResponse(status, requestedProfile, responseProfile);

        mSessionModificationType = IVideoCallSession.MODIFICATION_NONE;

        synchronized (mLock) {
            setCallState(CALL_STATE_ESTABLISHED);
        }
    }

    @Override
    public void onSendSessionModifyRequest(VideoProfile fromProfile, VideoProfile toProfile) {
        super.onSendSessionModifyRequest(fromProfile, toProfile);

        IVideoCallSession callSession = getVideoCallSession();

        if (callSession != null) {
            mSessionModificationType = callSession.getSessionModificationType();

            if (mSessionModificationType == IVideoCallSession.MODIFICATION_CALL_TYPE
                    && !VideoProfile.isAudioOnly(toProfile.getVideoState())) {
                logi("Prepare preview for video call upgrade request (TX)");
                synchronized (mLock) {
                    setCallState(CALL_STATE_VIDEO_UPGRADE_REQUESTED);
                }
            }
        }
    }

    @Override
    public void onSendSessionModifyResponse(VideoProfile responseProfile) {
        super.onSendSessionModifyResponse(responseProfile);
        synchronized (mLock) {
            setCallState(CALL_STATE_ESTABLISHED);
        }
    }

    public void changeCallDataUsage(long dataSize) {

        log("changeCallDataUsage :: " + dataSize);

        super.changeCallDataUsage(dataSize);
    }

    @Override
    protected void handleMediaSessionStarted() {
        enforcePeerDimensionsChanged();
        super.handleMediaSessionStarted();
    }

    @Override
    protected void handleMediaSessionMediaInfoChanged(
            int mediaInfo, int intParam, String strParam) {
        if (mediaInfo == MtcCallUtils.INFO_TYPE_MEDIA_VIDEO_NO_DATA) {
            handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_RX_PAUSE);
        } else if (mediaInfo == MtcCallUtils.INFO_TYPE_MEDIA_VIDEO_DATA_RECEIVED) {
            handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_RX_RESUME);
        } else if (mediaInfo == MtcCallUtils.INFO_TYPE_MEDIA_CVO_CAPABILITY) {
            if (intParam == MtcCallUtils.MEDIA_CVO_DISABLED) {
                // TODO: how to handle this?
            } else if (intParam == MtcCallUtils.MEDIA_CVO_ENABLED) {
                // TODO: how to handle this?
            }
        }
        return;
    }

    @Override
    protected void handleMediaSessionPeerFirstVideoReceived() {
        handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_RX_RESUME);
        return;
    }

    @Override
    protected void handleMediaSessionPeerDisplayOrientationChanged(final int orientation) {
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

    @Override
    protected void handleMediaSessionPeerDimensionsChanged(final int width, final int height) {
        if ((width > 0) && (height > 0)) {
            setCurrentVideoDimension(width, height);
            super.changePeerDimensions(width, height);
        }
        return;
    }

    private boolean isDynamicVideoQualitySupportedOnSessionModification() {
        IVideoCallSession callSession = getVideoCallSession();
        return CallFeature.isDynamicVideoQualitySupported(
                    callSession.getCallContext().getSlotId());
    }

    private Handler getCallHandler() {
        ICallContext callContext = getVideoCallSession().getCallContext();
        return (callContext != null) ? callContext.getCallHandler()
                : new Handler(Looper.myLooper());
    }

    private static int getCameraIdInt(String cameraId) {
        try {
            int camId = Integer.parseInt(cameraId);

            if (camId >= ImsCamera.CAMERA_REAR) {
                return camId;
            }
        } catch (NumberFormatException e) {
            logi("Invalid cameraId=" + cameraId);
        }

        return (-1);
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
            log("super.changePeerDimensions :: " + vd);
            super.changePeerDimensions(vd.getWidth(), vd.getHeight());
        }
    }

    private void enforcePeerDimensionsChanged() {
        IVideoCallSession callSession = getVideoCallSession();
        boolean callbackRequired = false;
        ImsStreamMediaProfile mediaProfile = callSession.getStreamMediaProfile();

        if (mediaProfile != null) {
            int videoQuality = VideoCallUtils.getVideoQualityFromMediaProfileForMediaInfo(
                    mediaProfile.getVideoQuality());

            logi("Enforce super.changePeerDimensions on media started; videoQuality="
                    + videoQuality);

            VideoDimension vd = VideoCallUtils.getVideoDimension(videoQuality);

            if (vd != null) {
                setCurrentVideoDimension(vd.getWidth(), vd.getHeight());
                super.changePeerDimensions(vd.getWidth(), vd.getHeight());
            }
        }
    }
}
