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
package com.android.imsstack.enabler.media;

import android.annotation.NonNull;
import android.annotation.Nullable;
import android.os.Parcel;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.ImsVideoSession;
import android.telephony.imsmedia.MediaQualityThreshold;
import android.telephony.imsmedia.VideoConfig;
import android.telephony.imsmedia.VideoSessionCallback;
import android.view.Surface;

import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.net.DatagramSocket;
import java.util.List;

/**
 * This manages video session by communicating between ImsStack (media enabler native)
 * and {@link ImsMediaManager}
 */
public class VideoSessionHandler {

    private final VideoSessionCallbackProxy mVideoSessionCallback;
    private ImsVideoSession mVideoSession;
    private int mVideoSessionId;
    private DatagramSocket mRtpSocket, mRtcpSocket;
    private final VideoSessionCallbackHandler mVideoSessionCallbackHandler;
    private final MediaManagerHelper mMediaManager;

    public VideoSessionHandler(
            @NonNull MediaSession mediaSession, @NonNull MediaManagerHelper mediaManager) {
        mMediaManager = mediaManager;
        mVideoSessionCallbackHandler = new VideoSessionCallbackHandler(mediaSession);
        mVideoSessionCallback = new VideoSessionCallbackProxy();
        ImsLog.d("VideoSessionHandler created");
    }

    @VisibleForTesting
    public VideoSessionHandler(
            @NonNull MediaManagerHelper mediaManager,
            @NonNull VideoSessionCallbackHandler videoCallbackHandler,
            @NonNull ImsVideoSession videoSession) {
        mMediaManager = mediaManager;
        mVideoSessionCallbackHandler = videoCallbackHandler;
        mVideoSession = videoSession;
        mVideoSessionCallback = new VideoSessionCallbackProxy();
        ImsLog.d("VideoSessionHandler created");
    }

    @VisibleForTesting
    void setVideoSession(@Nullable ImsVideoSession videoSession) {
        mVideoSession = videoSession;
    }

    @VisibleForTesting
    VideoSessionCallback getVideoSessionCallback() {
        return mVideoSessionCallback;
    }

    @VisibleForTesting
    int getVideoSessionId() {
        return mVideoSessionId;
    }

    private ImsVideoSession getVideoSession() {
        return mVideoSession;
    }

    /**
     * Handles requests received from Media Native
     *
     * @param requestType type of the request
     * @param parcel parcel received from Media Native
     */
    public void onImsMediaVideoMessage(final int requestType, Parcel parcel) {
        ImsLog.v("requestType= " + requestType);

        switch (requestType) {
            /** Requests (ImsStack -> ImsMedia) */
            case MediaConstants.REQUEST_OPEN_SESSION:
            {
                String localIpAddress = parcel.readString();
                int localPortNumber = parcel.readInt();
                ImsLog.v("localIpAddress= " + localIpAddress
                                + " localPortNumber= " + localPortNumber);

                handleVideoOpenSession(localIpAddress, localPortNumber, null);
            }
                break;

            case MediaConstants.REQUEST_CLOSE_SESSION:
            {
                handleVideoCloseSession();
            }
                break;

            case MediaConstants.REQUEST_MODIFY_SESSION:
            {
                VideoConfig videoConfig = VideoConfig.CREATOR.createFromParcel(parcel);

                handleVideoModifySession(videoConfig);
            }
                break;

            case MediaConstants.REQUEST_SET_PREVIEW_SURFACE:
            {
                Surface surface = Surface.CREATOR.createFromParcel(parcel);

                handleSetPreviewSurface(surface);
            }
                break;

            case MediaConstants.REQUEST_SET_DISPLAY_SURFACE:
            {
                Surface surface = Surface.CREATOR.createFromParcel(parcel);

                handleSetDisplaySurface(surface);
            }
                break;

            case MediaConstants.REQUEST_SET_MEDIA_QUALITY:
            {
                MediaQualityThreshold threshold =
                        MediaQualityThreshold.CREATOR.createFromParcel(parcel);
                ImsLog.v("onVideoSetMediaQualityThreshold: " + threshold.toString());

                handleVideoSetMediaQualityThreshold(threshold);
            }
                break;

            case MediaConstants.REQUEST_VIDEO_DATA_USAGE:
            {
                handleVideoDataUsageRequest();
            }
                break;

            default:
            {
                ImsLog.e("Invalid RequestType");
            }
                break;
        }
    }

    private void handleVideoOpenSession(
            String localIpAddress, int localPortNumber, VideoConfig videoConfig) {

        if (mVideoSession == null) {
            if (mMediaManager.isImsMediaConnected()) {
                // TODO_MEDIA : ImsQOSManager to be used
                mRtpSocket = MediaSocket.createDatagramSocket(localIpAddress, localPortNumber);
                mRtcpSocket =
                        MediaSocket.createDatagramSocket(localIpAddress, (localPortNumber + 1));

                if (mRtpSocket == null || mRtcpSocket == null) {
                    ImsLog.e("socket creation failed");
                    if (mVideoSessionCallbackHandler != null) {
                        mVideoSessionCallbackHandler.openSessionResponse(
                                ImsMediaSession.RESULT_PORT_UNAVAILABLE);
                    }
                    return;
                }

                mMediaManager.openSession(mRtpSocket, mRtcpSocket,
                        ImsMediaSession.SESSION_TYPE_VIDEO, videoConfig, mVideoSessionCallback);
            } else {
                ImsLog.d("ImsMediaManager is not ready");
                if (mVideoSessionCallbackHandler != null) {
                    mVideoSessionCallbackHandler.openSessionResponse(
                            ImsMediaSession.RESULT_NOT_READY);
                }
            }
        } else {
            ImsLog.d("Video Session is already created: SessionId=" + mVideoSession.getSessionId());
            if (mVideoSessionCallbackHandler != null) {
                mVideoSessionCallbackHandler.openSessionResponse(
                        ImsMediaSession.RESULT_NOT_SUPPORTED);
            }
        }
    }

    private void handleVideoCloseSession() {

        if (mVideoSession != null) {
            mMediaManager.closeSession(mVideoSession);
            mVideoSession = null;
        }
        // TODO_MEDIA : ImsQOSManager to be used
        MediaSocket.closeDatagramSocket(mRtpSocket);
        MediaSocket.closeDatagramSocket(mRtcpSocket);
    }

    private void handleVideoModifySession(VideoConfig videoConfig) {
        if (mVideoSession != null) {
            mVideoSession.modifySession(videoConfig);
        }
    }

    private void handleSetPreviewSurface(Surface surface) {
        if (mVideoSession != null) {
            mVideoSession.setPreviewSurface(surface);
        }
    }

    private void handleSetDisplaySurface(Surface surface) {
        if (mVideoSession != null) {
            mVideoSession.setDisplaySurface(surface);
        }
    }

    private void handleVideoSetMediaQualityThreshold(MediaQualityThreshold threshold) {
        if (mVideoSession != null) {
            mVideoSession.setMediaQualityThreshold(threshold);
        }
    }

    private void handleVideoDataUsageRequest() {
        if (mVideoSession != null) {
            mVideoSession.requestVideoDataUsage();
        }
    }

    private class VideoSessionCallbackProxy extends VideoSessionCallback {

        @Override
        public void onOpenSessionSuccess(ImsMediaSession session) {

            if (session == null) {
                ImsLog.e("Video Session is not created");
                if (mVideoSessionCallbackHandler != null) {
                    mVideoSessionCallbackHandler.openSessionResponse(
                            ImsMediaSession.RESULT_NO_MEMORY);
                }
                return;
            }

            mVideoSession = (ImsVideoSession) session;
            mVideoSessionId = mVideoSession.getSessionId();
            ImsLog.d("Video Session created: SessionId=" + mVideoSessionId);

            if (mVideoSessionCallbackHandler != null) {
                mVideoSessionCallbackHandler.openSessionResponse(ImsMediaSession.RESULT_SUCCESS);
            }
        }

        @Override
        public void onOpenSessionFailure(final @ImsMediaSession.SessionOperationResult int error) {
            ImsLog.d("error=" + error);

            if (mVideoSessionCallbackHandler != null) {
                mVideoSessionCallbackHandler.openSessionResponse(error);
            }
        }

        @Override
        public void onSessionChanged(final @ImsMediaSession.SessionState int state) {
            ImsLog.d("state=" + state);

            if (mVideoSessionCallbackHandler != null) {
                mVideoSessionCallbackHandler.sessionChanged(state);
            }
        }

        @Override
        public void onModifySessionResponse(
                final VideoConfig videoConfig,
                final @ImsMediaSession.SessionOperationResult int result) {
            ImsLog.d("result=" + result);

            if (mVideoSessionCallbackHandler != null) {
                mVideoSessionCallbackHandler.modifySessionResponse(videoConfig, result);
            }
        }

        @Override
        public void onFirstMediaPacketReceived(final VideoConfig videoConfig) {
            ImsLog.d("FirstMediaPacketReceived for SessionId[" + getVideoSessionId() + "]");

            if (mVideoSessionCallbackHandler != null) {
                mVideoSessionCallbackHandler.firstMediaPacketReceived(videoConfig);
            }
        }

        @Override
        public void onPeerDimensionChanged(final int width, final int height) {
            ImsLog.d("Peer Dimensions Changed width[" + width + "] height[" + height + "]");

            if (mVideoSessionCallbackHandler != null) {
                mVideoSessionCallbackHandler.peerDimensionChanged(width, height);
            }
        }

        @Override
        public void onHeaderExtensionReceived(final List<RtpHeaderExtension> extensions) {
            ImsLog.d("onHeaderExtensionReceived");

            // TODO_MEDIA : to be deleted when API is removed from interface
        }

        @Override
        public void notifyMediaInactivity(final @ImsMediaSession.PacketType int packetType) {
            ImsLog.d("packetType=" + packetType);

            if (mVideoSessionCallbackHandler != null) {
                mVideoSessionCallbackHandler.onNotifyMediaInactivity(packetType);
            }
        }

        @Override
        public void notifyPacketLoss(final int packetLossPercentage) {
            ImsLog.d("packetLossPercentage=" + packetLossPercentage);

            if (mVideoSessionCallbackHandler != null) {
                mVideoSessionCallbackHandler.onNotifyPacketLoss(packetLossPercentage);
            }
        }

        @Override
        public void notifyVideoDataUsage(final long bytes) {
            ImsLog.v("Video Data Usage in bytes: " + bytes);

            if (mVideoSessionCallbackHandler != null) {
                mVideoSessionCallbackHandler.onNotifyVideoDataUsage(bytes);
            }
        }
    }
}
