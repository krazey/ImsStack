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
    private String mCameraId = null;
    private int mSessionModificationType = IVideoCallSession.MODIFICATION_NONE;

    public ImsVideoCallProviderImpl(IVideoCallSession callSession,
            MtcMediaSession mediaSession) {
        super(callSession, mediaSession);
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

        if (callTypeChangeRequest) {
            if (isAudioOnly) {
                logi("Voice call is automatically accepted");
                sendSessionModifyResponse(new VideoProfile(VideoProfile.STATE_AUDIO_ONLY));
                return;
            } else {
                logi("Prepare preview for video call upgrade request (RX)");
            }
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

        setCallState(CALL_STATE_ESTABLISHED);
    }

    /**
     * Provides the call data usage to the telephony framework.
     *
     * @param dataSize The data usage of the video call in bytes.
     */
    public void changeCallDataUsage(long dataSize) {
        log("changeCallDataUsage :: " + dataSize);
        super.changeCallDataUsage(dataSize);
    }

    @Override
    protected void setCamera(String cameraId) {
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

        mCallSession.onSetCamera(cameraId);
    }

    @Override
    protected void sendSessionModifyRequest(
            VideoProfile fromProfile, VideoProfile toProfile) {
        super.sendSessionModifyRequest(fromProfile, toProfile);

        IVideoCallSession callSession = getVideoCallSession();

        if (callSession != null) {
            mSessionModificationType = callSession.getSessionModificationType();

            if (mSessionModificationType == IVideoCallSession.MODIFICATION_CALL_TYPE
                    && !VideoProfile.isAudioOnly(toProfile.getVideoState())) {
                logi("Prepare preview for video call upgrade request (TX)");
                setCallState(CALL_STATE_VIDEO_UPGRADE_REQUESTED);
            }
        }
    }

    @Override
    protected void sendSessionModifyResponse(VideoProfile responseProfile) {
        super.sendSessionModifyResponse(responseProfile);
        setCallState(CALL_STATE_ESTABLISHED);
    }

    @Override
    protected void requestCameraCapabilities() {
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

        if (camera.isLandscape() && width < height) {
            int tmp = width;
            width = height;
            height = tmp;
        }

        logi("ImsCamera :: previewSize=" + width + "x" + height
                + ", maxZoom=" + maxZoom
                + ", zoomSupported=" + zoomSupported);

        VideoProfile.CameraCapabilities cc = new VideoProfile.CameraCapabilities(width, height,
                zoomSupported, maxZoom);

        changeCameraCapabilities(cc);
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
            } else if (intParam == MtcCallUtils.MEDIA_CVO_ENABLED) {
            }
        }
    }

    @Override
    protected void handleMediaSessionPeerFirstVideoReceived() {
        handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_RX_RESUME);
    }

    @Override
    protected void handleMediaSessionPeerDimensionsChanged(final int width, final int height) {
        if ((width > 0) && (height > 0)) {
            super.changePeerDimensions(width, height);
        }
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
}
