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

package com.android.imsstack.imsservice.mmtel.videocall.base;

import android.net.Uri;
import android.telecom.VideoProfile;
import android.telephony.ims.ImsVideoCallProvider;
import android.view.Surface;

import androidx.annotation.NonNull;

import com.android.imsstack.enabler.mtc.MtcMediaSession;
import com.android.imsstack.imsservice.mmtel.call.IVideoCallSession;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.concurrent.Executor;

public class ImsVideoCallProviderBase extends ImsVideoCallProvider
        implements IVideoCallSession.EventListener {
    public static final int CALL_STATE_IDLE = 0;
    // Originating call is started
    public static final int CALL_STATE_INITIATING = 1;
    // Incoming call is notified to framework and user alert will be done soon.
    public static final int CALL_STATE_ALERTING = 2;
    // Call establishment is completed
    public static final int CALL_STATE_ESTABLISHED = 3;
    // Call is terminated
    public static final int CALL_STATE_TERMINATED = 4;
    // Video call upgrade request is in progress
    public static final int CALL_STATE_VIDEO_UPGRADE_REQUESTED = 5;

    protected static final int CAMERA_ID_NONE = -1;

    /*
     * Display type for video call
     */
    protected final static int DISPLAY_NEAR = 0;
    protected final static int DISPLAY_FAR = 1;
    protected final static int DISPLAY_NEAR_N_FAR = 2;

    /*
     * Display size
     */
    protected final static int DISPLAY_SIZE_NORMAL = 0;
    protected final static int DISPLAY_SIZE_FULL = 1;

    protected final IVideoCallSession mCallSession;
    private MtcMediaSessionListenerProxy mListenerProxy = new MtcMediaSessionListenerProxy();
    private int mCallState = CALL_STATE_IDLE;
    protected MtcMediaSession mMediaSession = null;

    protected final Executor mExecutor;

    public ImsVideoCallProviderBase(@NonNull IVideoCallSession callSession,
            MtcMediaSession mediaSession) {
        mCallSession = callSession;
        mMediaSession = mediaSession;
        mExecutor = mCallSession.getCallContext().getExecutor();

        if (mediaSession != null) {
            mediaSession.setVideoListener(mListenerProxy);
        }

        callSession.setVideoCallProvider(this);
        callSession.setEventListener(this);
    }

    @Override
    public final void onSetCamera(String cameraId) {
        mExecutor.execute(() -> setCamera(cameraId));
    }

    @Override
    public final void onSetPreviewSurface(Surface surface) {
        mExecutor.execute(() -> setPreviewSurface(surface));
    }

    @Override
    public final void onSetDisplaySurface(Surface surface) {
        mExecutor.execute(() -> setDisplaySurface(surface));
    }

    @Override
    public final void onSetDeviceOrientation(int rotation) {
        mExecutor.execute(() -> setDeviceOrientation(rotation));
    }

    @Override
    public final void onSetZoom(float value) {
        mExecutor.execute(() -> setZoom(value));
    }

    @Override
    public final void onSendSessionModifyRequest(VideoProfile from, VideoProfile to) {
        mExecutor.execute(() -> sendSessionModifyRequest(from, to));
    }

    @Override
    public final void onSendSessionModifyResponse(VideoProfile responseProfile) {
        mExecutor.execute(() -> sendSessionModifyResponse(responseProfile));
    }

    @Override
    public final void onRequestCameraCapabilities() {
        mExecutor.execute(() -> requestCameraCapabilities());
    }

    @Override
    public final void onRequestCallDataUsage() {
        mExecutor.execute(() -> requestCallDataUsage());
    }

    @Override
    public final void onSetPauseImage(Uri uri) {
        mExecutor.execute(() -> setPauseImage(uri));
    }

    protected void setCamera(String cameraId) {}

    protected void setPreviewSurface(Surface surface) {
        if (mMediaSession == null) {
            // Exception handling
            return;
        }

        mMediaSession.setPreviewSurface(surface);
    }

    protected void setDisplaySurface(Surface surface) {
        if (mMediaSession == null) {
            // Exception handling
            return;
        }

        mMediaSession.setDisplaySurface(surface);
    }

    protected void setDeviceOrientation(int rotation) {
        if (mMediaSession == null) {
            return;
        }

        logi("setDeviceOrientation :: rotation=" + rotation);

        mMediaSession.setDeviceOrientation(getQuartile(rotation));
    }

    protected void setZoom(float value) {
        if (mMediaSession == null) {
            return;
        }

        Float f = Float.valueOf(value);

        mMediaSession.setCameraZoom(f.intValue());
    }

    protected void sendSessionModifyRequest(
            VideoProfile fromProfile, VideoProfile toProfile) {
        mCallSession.sendSessionModifyRequest(fromProfile, toProfile);
    }

    protected void sendSessionModifyResponse(VideoProfile responseProfile) {
        mCallSession.sendSessionModifyResponse(responseProfile);
    }

    protected void requestCameraCapabilities() {}

    protected void requestCallDataUsage() {
        log("requestCallDataUsage");

        if (mMediaSession == null) {
            return;
        }

        mMediaSession.requestCallDataUsage();
    }

    protected void setPauseImage(Uri uri) {}

    @Override
    public void onCallEvent(int event) {
        log("VideoCallProvider :: callEvent=" + event);
        handleCallEvent(event);
    }

    public void updateMediaSession(MtcMediaSession mediaSession) {
        if (mMediaSession != null) {
            mMediaSession.setVideoListener(null);
            mMediaSession = null;
        }

        mMediaSession = mediaSession;

        if (mMediaSession != null) {
            mMediaSession.setVideoListener(mListenerProxy);
        }
    }

    public void close() {
        mCallSession.setEventListener(null);

        if (mMediaSession != null) {
            mMediaSession.setVideoListener(null);
            mMediaSession = null;
        }
    }

    @VisibleForTesting
    public int getCallState() {
        return mCallState;
    }

    protected final MtcMediaSession getMediaSession() {
        return mMediaSession;
    }

    protected final IVideoCallSession getVideoCallSession() {
        return mCallSession;
    }

    protected void setCallState(int state) {
        if (mCallState != state) {
            logi("VideoCallProvider :: callState - " + mCallState + " >> " + state);
            mCallState = state;
        }
    }

    protected void handleCallEvent(int event) {
        switch (event) {
            case IVideoCallSession.EVENT_CALL_IDLE:
                setCallState(CALL_STATE_IDLE);
                break;
            case IVideoCallSession.EVENT_CALL_INITIATING:
                setCallState(CALL_STATE_INITIATING);
                break;
            case IVideoCallSession.EVENT_CALL_ALERTING:
                setCallState(CALL_STATE_ALERTING);
                break;
            case IVideoCallSession.EVENT_CALL_ESTABLISHED:
                setCallState(CALL_STATE_ESTABLISHED);
                break;
            case IVideoCallSession.EVENT_CALL_TERMINATED:
                setCallState(CALL_STATE_TERMINATED);
                break;
            default:
                // no-op
                break;
        }
    }

    protected void handleMediaSessionDataUsageChanged(long dataSize) {
        log("changeCallDataUsage dataSize : " + dataSize);
        changeCallDataUsage(dataSize);
    }

    protected void handleMediaSessionMediaInfoChanged(
            int mediaInfo, int intParam, String strParam) {
        // no-op
    }

    protected void handleMediaSessionPeerFirstVideoReceived() {
        // no-op
    }

    protected void handleMediaSessionPeerDimensionsChanged(final int width, final int height) {
        changePeerDimensions(width, height);
    }

    protected static boolean checkRadius(int angle, int criteria, int threshold) {
        int limitLeft = criteria - threshold;
        int limitRight = criteria + threshold;

        return (angle >= limitLeft) && (angle < limitRight);
    }

    protected static int getQuartile(int angle) {
        int orientation = 0;

        // This is for the reference devices.
        final int orientationThreshold = 45;

        if (checkRadius(angle, 90, orientationThreshold)) {
            orientation = 90;
        } else if (checkRadius(angle, 180, orientationThreshold)) {
            orientation = 180;
        } else if (checkRadius(angle, 270, orientationThreshold)) {
            orientation = 270;
        }

        return orientation;
    }

    protected static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }

    protected static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }

    private class MtcMediaSessionListenerProxy extends MtcMediaSession.VideoListener {
        @Override
        public void onMediaSessionDataUsageChanged(MtcMediaSession session,
                long dataSize) {
            mExecutor.execute(() -> handleMediaSessionDataUsageChanged(dataSize));
        }

        @Override
        public void onMediaSessionMediaInfoChanged(MtcMediaSession session,
                int mediaInfo, int intParam, String strParam) {
            mExecutor.execute(() -> handleMediaSessionMediaInfoChanged(
                    mediaInfo, intParam, strParam));
        }

        @Override
        public void onMediaSessionPeerFirstVideoReceived(MtcMediaSession session) {
            mExecutor.execute(() -> handleMediaSessionPeerFirstVideoReceived());
        }

        @Override
        public void onMediaSessionPeerDimensionsChanged(MtcMediaSession session,
                final int width, final int height) {
            mExecutor.execute(() -> handleMediaSessionPeerDimensionsChanged(width, height));
        }
    }
}
