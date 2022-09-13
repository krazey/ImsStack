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
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.ImsVideoSession;
import android.telephony.imsmedia.MediaQualityThreshold;
import android.telephony.imsmedia.VideoConfig;
import android.telephony.imsmedia.VideoSessionCallback;

import com.android.imsstack.enabler.mtc.IMtcMediaInterface;
import com.android.imsstack.enabler.mtc.IMtcMediaVideoCallProvider;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.net.DatagramSocket;
import java.util.List;

/**
 * This manages video session by communicating between ImsStack (media enabler native)
 * and {@link ImsMediaManager}
 */
public class VideoSessionHandler {

    static final int UNUSED = -1;

    private final VideoSessionCallbackProxy mVideoSessionCallback;
    private ImsVideoSession mVideoSession;
    private int mVideoSessionId;
    private DatagramSocket mRtpSocket, mRtcpSocket;
    private final VideoSessionCallbackHandler mVideoSessionCallbackHandler;
    private final MediaManagerHelper mMediaManager;
    private IMtcMediaVideoCallProvider mMtcMediaVideoCallProvider;
    private final VideoMessageHandler mVideoMessageHandler;
    private boolean mPreviewSurfaceSet, mDisplaySurfaceSet;
    private String mLocalIpAddress;
    private int mLocalPortNumber;

    public VideoSessionHandler(
            @NonNull MediaManagerHelper mediaManager, IMtcMediaInterface mtcMediaInterface,
            IMtcMediaVideoCallProvider mtcMediaVideoCallProvider) {
        mMediaManager = mediaManager;
        mMtcMediaVideoCallProvider = mtcMediaVideoCallProvider;
        mVideoSessionCallbackHandler = new VideoSessionCallbackHandler(mtcMediaInterface);
        mVideoSessionCallback = new VideoSessionCallbackProxy();
        mVideoMessageHandler = new VideoMessageHandler(mMediaManager.getMediaLooper());
        ImsLog.d("VideoSessionHandler created");
    }

    @VisibleForTesting
    public VideoSessionHandler(
            @NonNull MediaManagerHelper mediaManager,
            IMtcMediaVideoCallProvider mtcMediaVideoCallProvider,
            @NonNull VideoSessionCallbackHandler videoCallbackHandler,
            @NonNull ImsVideoSession videoSession) {
        mMediaManager = mediaManager;
        mMtcMediaVideoCallProvider = mtcMediaVideoCallProvider;
        mVideoSessionCallbackHandler = videoCallbackHandler;
        mVideoSession = videoSession;
        mVideoSessionCallback = new VideoSessionCallbackProxy();
        mVideoMessageHandler = new VideoMessageHandler(Looper.getMainLooper());
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

    @VisibleForTesting
    VideoMessageHandler getVideoMessageHandler() {
        return mVideoMessageHandler;
    }

    /** Video session message Handler */
    class VideoMessageHandler extends Handler {

        VideoMessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            ImsLog.v("msg.what= " + msg.what);
            switch (msg.what) {
                case MediaConstants.REQUEST_OPEN_SESSION:
                {
                    handleVideoOpenSession((VideoConfig) msg.obj);
                }
                    break;

                case MediaConstants.REQUEST_CLOSE_SESSION:
                {
                    handleVideoCloseSession();
                }
                    break;

                case MediaConstants.REQUEST_MODIFY_SESSION:
                {
                    handleVideoModifySession((VideoConfig) msg.obj);
                }
                    break;

                case MediaConstants.REQUEST_SET_PREVIEW_SURFACE:
                {
                    handleSetPreviewSurface();
                }
                    break;

                case MediaConstants.REQUEST_SET_DISPLAY_SURFACE:
                {
                    handleSetDisplaySurface();
                }
                    break;

                case MediaConstants.REQUEST_SET_MEDIA_QUALITY:
                {
                    handleVideoSetMediaQualityThreshold((MediaQualityThreshold) msg.obj);
                }
                    break;

                case MediaConstants.REQUEST_VIDEO_DATA_USAGE:
                {
                    handleVideoDataUsageRequest();
                }
                    break;

                case MediaConstants.RESPONSE_OPEN_SESSION:
                {
                    handleVideoOpenSessionResponse((ImsMediaSession) msg.obj, msg.arg1);
                }
                    break;

                case MediaConstants.RESPONSE_SESSION_CLOSED:
                {
                    handleVideoSessionClosed();
                }
                    break;

                case MediaConstants.RESPONSE_SESSION_CHANGED:
                {
                    handleVideoSessionChanged(msg.arg1);
                }
                    break;

                case MediaConstants.RESPONSE_MODIFY_SESSION:
                {
                    handleVideoModifySessionResponse((VideoConfig) msg.obj, msg.arg1);
                }
                    break;

                case MediaConstants.NOTIFY_FIRST_PACKET:
                {
                    handleVideoFirstMediaPacketReceived((VideoConfig) msg.obj);
                }
                    break;

                case MediaConstants.NOTIFY_PEER_DIMENSION_CHANGED:
                {
                    handlePeerDimensionChanged(msg.arg1, msg.arg2);
                }
                    break;

                case MediaConstants.NOTIFY_MEDIA_INACTIVITY:
                {
                    handleVideoMediaInactivityNotification(msg.arg1);
                }
                    break;

                case MediaConstants.NOTIFY_PACKET_LOSS:
                {
                    handleVideoPacketLossNotification(msg.arg1);
                }
                    break;

                case MediaConstants.NOTIFY_VIDEO_DATA_USAGE:
                {
                    handleVideoDataUsageNotification((long) msg.obj);
                }
                    break;

                case MediaConstants.NOTIFY_MEDIA_DETACH:
                {
                    handleVideoDisconnection();
                }
                    break;

                default:
                {
                    ImsLog.e("Invalid RequestType");
                }
                    break;
            }
        }
    }

    private class VideoSessionCallbackProxy extends VideoSessionCallback {

        @Override
        public void onOpenSessionSuccess(ImsMediaSession session) {

            Message.obtain(mVideoMessageHandler, MediaConstants.RESPONSE_OPEN_SESSION,
                    ImsMediaSession.RESULT_SUCCESS, UNUSED, session).sendToTarget();
        }

        @Override
        public void onOpenSessionFailure(final @ImsMediaSession.SessionOperationResult int error) {
            ImsLog.d("error=" + error);

            Message.obtain(mVideoMessageHandler, MediaConstants.RESPONSE_OPEN_SESSION,
                    error, UNUSED, null).sendToTarget();
        }

        @Override
        public void onSessionClosed() {
            ImsLog.d("onSessionClosed for SessionId[" + getVideoSessionId() + "]");

            Message.obtain(mVideoMessageHandler, MediaConstants.RESPONSE_SESSION_CLOSED)
                    .sendToTarget();
        }

        @Override
        public void onSessionChanged(final @ImsMediaSession.SessionState int state) {
            ImsLog.d("state=" + state);

            Message.obtain(mVideoMessageHandler, MediaConstants.RESPONSE_SESSION_CHANGED,
                    state, UNUSED).sendToTarget();
        }

        @Override
        public void onModifySessionResponse(final VideoConfig videoConfig,
                final @ImsMediaSession.SessionOperationResult int result) {
            ImsLog.d("result=" + result);

            Message.obtain(mVideoMessageHandler, MediaConstants.RESPONSE_MODIFY_SESSION,
                    result, UNUSED, videoConfig).sendToTarget();
        }

        @Override
        public void onFirstMediaPacketReceived(final VideoConfig videoConfig) {
            ImsLog.d("FirstMediaPacketReceived for SessionId[" + getVideoSessionId() + "]");

            Message.obtain(mVideoMessageHandler, MediaConstants.NOTIFY_FIRST_PACKET,
                    videoConfig).sendToTarget();
        }

        @Override
        public void onPeerDimensionChanged(final int width, final int height) {
            ImsLog.d("Peer Dimensions Changed width[" + width + "] height[" + height + "]");

            Message.obtain(mVideoMessageHandler, MediaConstants.NOTIFY_PEER_DIMENSION_CHANGED,
                    width, height).sendToTarget();
        }

        @Override
        public void onHeaderExtensionReceived(final List<RtpHeaderExtension> extensions) {
            ImsLog.d("onHeaderExtensionReceived");

            // TODO_MEDIA : to be deleted when API is removed from interface
        }

        @Override
        public void notifyMediaInactivity(final @ImsMediaSession.PacketType int packetType) {
            ImsLog.d("packetType=" + packetType);

            Message.obtain(mVideoMessageHandler, MediaConstants.NOTIFY_MEDIA_INACTIVITY,
                    packetType, UNUSED).sendToTarget();
        }

        @Override
        public void notifyPacketLoss(final int packetLossPercentage) {
            ImsLog.d("packetLossPercentage=" + packetLossPercentage);

            Message.obtain(mVideoMessageHandler, MediaConstants.NOTIFY_PACKET_LOSS,
                    packetLossPercentage, UNUSED).sendToTarget();
        }

        @Override
        public void notifyVideoDataUsage(final long bytes) {
            ImsLog.d("Video Data Usage in bytes: " + bytes);

            Message.obtain(mVideoMessageHandler, MediaConstants.NOTIFY_VIDEO_DATA_USAGE,
                    bytes).sendToTarget();

        }
    }

    /**
     * Handles Video Session requests received from Media Native
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
                mLocalIpAddress = parcel.readString();
                mLocalPortNumber = parcel.readInt();
                VideoConfig videoConfig = VideoConfig.CREATOR.createFromParcel(parcel);
                ImsLog.v("localIpAddress= " + mLocalIpAddress
                        + " localPortNumber= " + mLocalPortNumber);

                Message.obtain(mVideoMessageHandler, requestType, videoConfig).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_CLOSE_SESSION:
            case MediaConstants.REQUEST_SET_PREVIEW_SURFACE:
            case MediaConstants.REQUEST_SET_DISPLAY_SURFACE:
            case MediaConstants.REQUEST_VIDEO_DATA_USAGE:
            case MediaConstants.NOTIFY_MEDIA_DETACH:
            {
                Message.obtain(mVideoMessageHandler, requestType).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_MODIFY_SESSION:
            {
                VideoConfig videoConfig = VideoConfig.CREATOR.createFromParcel(parcel);
                Message.obtain(mVideoMessageHandler, requestType, videoConfig).sendToTarget();
            }
                break;


            case MediaConstants.REQUEST_SET_MEDIA_QUALITY:
            {
                MediaQualityThreshold threshold =
                        MediaQualityThreshold.CREATOR.createFromParcel(parcel);
                ImsLog.v("onVideoSetMediaQualityThreshold: " + threshold.toString());

                Message.obtain(mVideoMessageHandler, requestType, threshold).sendToTarget();
            }
                break;

            default:
            {
                ImsLog.e("Invalid RequestType");
            }
                break;
        }
    }

    private void handleVideoOpenSession(VideoConfig videoConfig) {

        if (mVideoSession == null) {
            if (mMediaManager.isImsMediaConnected()) {
                // TODO_MEDIA : ImsQOSManager to be used
                mRtpSocket = MediaSocket.createDatagramSocket(mLocalIpAddress, mLocalPortNumber);
                mRtcpSocket =
                        MediaSocket.createDatagramSocket(mLocalIpAddress, (mLocalPortNumber + 1));

                if (mRtpSocket == null || mRtcpSocket == null) {
                    ImsLog.e("socket creation failed");
                    if (mVideoSessionCallbackHandler != null) {
                        mVideoSessionCallbackHandler.openSessionResponse(
                                ImsMediaSession.RESULT_PORT_UNAVAILABLE);
                    }
                    return;
                }

                mPreviewSurfaceSet = true;
                mDisplaySurfaceSet = true;
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
        }
    }

    private void handleVideoDisconnection() {
        if (mVideoSession != null) {
            closeSockets();
            mVideoSession = null;
        }
    }

    private void closeSockets() {
        // TODO_MEDIA: ImsQOSManager to be used
        MediaSocket.closeDatagramSocket(mRtpSocket);
        mRtpSocket = null;
        MediaSocket.closeDatagramSocket(mRtcpSocket);
        mRtcpSocket = null;
    }

    private void handleVideoModifySession(VideoConfig videoConfig) {
        if (mVideoSession != null) {
            mVideoSession.modifySession(videoConfig);
        }
    }

    private void handleSetPreviewSurface() {
        if (mVideoSession != null && mMtcMediaVideoCallProvider != null) {
            mVideoSession.setPreviewSurface(mMtcMediaVideoCallProvider.getPreviewSurface());
            mPreviewSurfaceSet = true;
        } else {
            mPreviewSurfaceSet = false;
        }
    }

    private void handleSetDisplaySurface() {
        if (mVideoSession != null && mMtcMediaVideoCallProvider != null) {
            mVideoSession.setDisplaySurface(mMtcMediaVideoCallProvider.getDisplaySurface());
            mDisplaySurfaceSet = true;
        } else {
            mDisplaySurfaceSet = false;
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

    private void handleVideoOpenSessionResponse(ImsMediaSession session, int result) {
        if (result == ImsMediaSession.RESULT_SUCCESS) {
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
            if (!mPreviewSurfaceSet) {
                Message.obtain(mVideoMessageHandler, MediaConstants.REQUEST_SET_PREVIEW_SURFACE)
                        .sendToTarget();
            }

            if (!mDisplaySurfaceSet) {
                Message.obtain(mVideoMessageHandler, MediaConstants.REQUEST_SET_DISPLAY_SURFACE)
                        .sendToTarget();
            }
        }

        if (mVideoSessionCallbackHandler != null) {
            mVideoSessionCallbackHandler.openSessionResponse(result);
        }
    }

    private void handleVideoSessionClosed() {
        closeSockets();
        mVideoSession = null;
        mVideoSessionId = 0;
    }

    private void handleVideoSessionChanged(int state) {
        if (mVideoSessionCallbackHandler != null) {
            mVideoSessionCallbackHandler.sessionChanged(state);
        }
    }

    private void handleVideoModifySessionResponse(final VideoConfig videoConfig, final int result) {
        if (mVideoSessionCallbackHandler != null) {
            mVideoSessionCallbackHandler.modifySessionResponse(videoConfig, result);
        }
    }

    private void handleVideoFirstMediaPacketReceived(final VideoConfig videoConfig) {
        if (mVideoSessionCallbackHandler != null) {
            mVideoSessionCallbackHandler.firstMediaPacketReceived(videoConfig);
        }
    }

    private void handlePeerDimensionChanged(final int width, final int height) {
        if (mMtcMediaVideoCallProvider != null) {
            mMtcMediaVideoCallProvider.peerDimensionChanged(width, height);
        }
    }

    private void handleVideoMediaInactivityNotification(final int packetType) {
        if (mVideoSessionCallbackHandler != null) {
            mVideoSessionCallbackHandler.onNotifyMediaInactivity(packetType);
        }
    }

    private void handleVideoPacketLossNotification(final int packetLossPercentage) {
        if (mVideoSessionCallbackHandler != null) {
            mVideoSessionCallbackHandler.onNotifyPacketLoss(packetLossPercentage);
        }
    }

    private void handleVideoDataUsageNotification(final long bytes) {
        if (mVideoSessionCallbackHandler != null) {
            mVideoSessionCallbackHandler.onNotifyVideoDataUsage(bytes);
        }
    }
}
