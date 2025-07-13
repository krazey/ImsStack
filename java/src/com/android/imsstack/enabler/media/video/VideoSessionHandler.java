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
import android.telephony.imsmedia.RtpReceptionStats;
import android.telephony.imsmedia.VideoConfig;
import android.telephony.imsmedia.VideoSessionCallback;
import android.util.Pair;

import com.android.imsstack.core.agents.QosAgent;
import com.android.imsstack.core.agents.QosAgent.ImsQosCallback;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.IMtcMediaInterface;
import com.android.imsstack.enabler.mtc.IMtcMediaVideoCallProvider;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.util.List;

/**
 * This manages video session by communicating between ImsStack (media enabler native)
 * and {@link ImsMediaManager}
 */
public class VideoSessionHandler extends MediaState {

    static final int UNUSED = -1;

    private final VideoSessionCallbackProxy mVideoSessionCallback;
    private ImsVideoSession mVideoSession;
    private int mVideoSessionId;
    private Pair<DatagramSocket, DatagramSocket> mRtpSocket;
    private final VideoSessionCallbackHandler mVideoSessionCallbackHandler;
    private final MediaManagerHelper mMediaManager;
    private IMtcMediaVideoCallProvider mMtcMediaVideoCallProvider;
    private final VideoMessageHandler mVideoMessageHandler;
    private boolean mPreviewSurfaceSet, mDisplaySurfaceSet;
    private String mLocalIpAddress;
    private int mLocalPortNumber;
    private final IBaseContext mContext;
    private QosAgent mVideoQosAgent;
    private VideoImsQosCallback mVideoImsQosCallback;
    private final Object mLock = new Object();

    public VideoSessionHandler(IBaseContext context,
            @NonNull MediaManagerHelper mediaManager, IMtcMediaInterface mtcMediaInterface,
            IMtcMediaVideoCallProvider mtcMediaVideoCallProvider) {
        super(ImsMediaSession.SESSION_TYPE_VIDEO);
        mContext = context;
        mMediaManager = mediaManager;
        mMtcMediaVideoCallProvider = mtcMediaVideoCallProvider;
        mVideoSessionCallbackHandler = new VideoSessionCallbackHandler(mtcMediaInterface);
        mVideoSessionCallback = new VideoSessionCallbackProxy();
        mVideoMessageHandler = new VideoMessageHandler(mMediaManager.getMediaLooper());
        createQosAgent(mContext.getSlotId());
        ImsLog.d("VideoSessionHandler created");
    }

    @VisibleForTesting
    public VideoSessionHandler(IBaseContext context, @NonNull MediaManagerHelper mediaManager,
            IMtcMediaVideoCallProvider mtcMediaVideoCallProvider,
            @NonNull VideoSessionCallbackHandler videoCallbackHandler,
            @NonNull ImsVideoSession videoSession, Looper looper, @Nullable QosAgent qosAgent) {
        super(ImsMediaSession.SESSION_TYPE_VIDEO);
        mContext = context;
        mMediaManager = mediaManager;
        mMtcMediaVideoCallProvider = mtcMediaVideoCallProvider;
        mVideoSessionCallbackHandler = videoCallbackHandler;
        mVideoSession = videoSession;
        mVideoQosAgent = qosAgent;
        mVideoSessionCallback = new VideoSessionCallbackProxy();
        mVideoMessageHandler = new VideoMessageHandler(looper);
        createQosAgent(mContext.getSlotId());
        ImsLog.d("VideoSessionHandler created");
    }

    @VisibleForTesting
    void setVideoSession(@Nullable ImsVideoSession videoSession) {
        mVideoSession = videoSession;
    }

    @VisibleForTesting
    ImsVideoSession getVideoSession() {
        return mVideoSession;
    }

    @VisibleForTesting
    void setVideoQosAgent(@Nullable QosAgent videoQosAgent) {
        mVideoQosAgent = videoQosAgent;
    }

    @VisibleForTesting
    void setRtpSocket(@Nullable Pair<DatagramSocket, DatagramSocket> rtpSocket) {
        mRtpSocket = rtpSocket;
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
    VideoImsQosCallback getVideoImsQosCallback() {
        return mVideoImsQosCallback;
    }

    @VisibleForTesting
    boolean isPreviewSurfaceSet() {
        return mPreviewSurfaceSet;
    }

    @VisibleForTesting
    boolean isDisplaySurfaceSet() {
        return mDisplaySurfaceSet;
    }

    /** Video session message Handler */
    class VideoMessageHandler extends Handler {

        VideoMessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            ImsLog.d("messageType = " + msg.what);

            if (isClosed() && MediaSessionUtils.isDiscardRequired(msg.what)) {
                ImsLog.w("Session=" + getVideoSessionId() + " is closing, discard request: "
                        + msg.what);
                return;
            }

            // Till open session response is received, handling other commands has to wait
            try {
                synchronized (mLock) {
                    if (mVideoSession == null && MediaSessionUtils.isWaitRequired(msg.what)) {
                        ImsLog.d(Thread.currentThread().getName()
                                + " is waiting for Video openSession response");
                        mLock.wait(MediaConstants.RESPONSE_WAIT_TIMEOUT);
                        if (mVideoSession == null) {
                            ImsLog.e("Video openSession response timeout");
                            handleVideoOpenSessionResponse(null, ImsMediaSession.RESULT_NOT_READY);
                            return;
                        }
                        ImsLog.d(Thread.currentThread().getName()
                                + " received Video openSession response");
                    }
                }
            } catch (InterruptedException ie) {
                ImsLog.e("unexpectedly interrupted while waiting" + ie.getMessage());
            }

            switch (msg.what) {
                case MediaConstants.REQUEST_OPEN_SESSION:
                    handleVideoOpenSession(null);
                    break;
                case MediaConstants.REQUEST_CLOSE_SESSION:
                    handleVideoCloseSession();
                    break;
                case MediaConstants.REQUEST_MODIFY_SESSION:
                    handleVideoModifySession((VideoConfig) msg.obj);
                    break;
                case MediaConstants.REQUEST_QOS:
                    handleVideoQos((String) msg.obj, msg.arg1);
                    break;
                case MediaConstants.REQUEST_SET_PREVIEW_SURFACE:
                    handleSetPreviewSurface();
                    break;
                case MediaConstants.REQUEST_SET_DISPLAY_SURFACE:
                    handleSetDisplaySurface();
                    break;
                case MediaConstants.REQUEST_SET_MEDIA_QUALITY:
                    handleVideoSetMediaQualityThreshold((MediaQualityThreshold) msg.obj);
                    break;
                case MediaConstants.REQUEST_VIDEO_DATA_USAGE:
                    handleVideoDataUsageRequest();
                    break;
                case MediaConstants.REQUEST_ADJUST_DELAY:
                    handleAdjustDelay((int) msg.obj);
                    break;
                case MediaConstants.RESPONSE_OPEN_SESSION:
                    handleVideoOpenSessionResponse((ImsMediaSession) msg.obj, msg.arg1);
                    break;
                case MediaConstants.RESPONSE_SESSION_CLOSED:
                case MediaConstants.NOTIFY_MEDIA_DETACH:
                    handleVideoSessionClosed();
                    break;
                case MediaConstants.RESPONSE_MODIFY_SESSION:
                    handleVideoModifySessionResponse((VideoConfig) msg.obj, msg.arg1);
                    break;
                case MediaConstants.NOTIFY_FIRST_PACKET:
                    handleVideoFirstMediaPacketReceived((VideoConfig) msg.obj);
                    break;
                case MediaConstants.NOTIFY_PEER_DIMENSION_CHANGED:
                    handlePeerDimensionChanged(msg.arg1, msg.arg2);
                    break;
                case MediaConstants.NOTIFY_MEDIA_INACTIVITY:
                    handleVideoMediaInactivityNotification(msg.arg1);
                    break;
                case MediaConstants.NOTIFY_BITRATE:
                    handleVideoBitrateNotification(msg.arg1);
                    break;
                case MediaConstants.NOTIFY_VIDEO_DATA_USAGE:
                    handleVideoDataUsageNotification((long) msg.obj);
                    break;
                case MediaConstants.NOTIFY_RTP_RECEPTION_STATS:
                    handleNotifyRtpReceptionStats((RtpReceptionStats) msg.obj);
                    break;
                default:
                    ImsLog.e("Invalid RequestType");
                    break;
            }
        }
    }

    private class VideoSessionCallbackProxy extends VideoSessionCallback {

        @Override
        public void onOpenSessionSuccess(ImsMediaSession session) {
            // Release the wait as OpenSession Response is received
            synchronized (mLock) {
                mVideoSession = (ImsVideoSession) session;
                mLock.notifyAll();
            }
            Message.obtain(mVideoMessageHandler, MediaConstants.RESPONSE_OPEN_SESSION,
                    ImsMediaSession.RESULT_SUCCESS, UNUSED, session).sendToTarget();
        }

        @Override
        public void onOpenSessionFailure(final @ImsMediaSession.SessionOperationResult int error) {
            ImsLog.d("error=" + error);
            // Release the wait as OpenSession Response is received
            synchronized (mLock) {
                mLock.notifyAll();
            }
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
        public void onModifySessionResponse(final VideoConfig videoConfig,
                final @ImsMediaSession.SessionOperationResult int result) {
            ImsLog.d("result=" + result);

            Message.obtain(mVideoMessageHandler, MediaConstants.RESPONSE_MODIFY_SESSION,
                    result, UNUSED, videoConfig).sendToTarget();
        }

        @Override
        public void onFirstMediaPacketReceived(final VideoConfig videoConfig) {
            ImsLog.d("FirstMediaPacketReceived for SessionId[" + getVideoSessionId() + "]");

            Message.obtain(mVideoMessageHandler, MediaConstants.NOTIFY_FIRST_PACKET, videoConfig)
                    .sendToTarget();
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
        public void notifyBitrate(final int bitrate) {
            ImsLog.d("notifyBitrate=" + bitrate);

            Message.obtain(mVideoMessageHandler, MediaConstants.NOTIFY_BITRATE, bitrate, UNUSED)
                    .sendToTarget();
        }

        @Override
        public void notifyVideoDataUsage(final long bytes) {
            ImsLog.d("Video Data Usage in bytes: " + bytes);

            Message.obtain(mVideoMessageHandler, MediaConstants.NOTIFY_VIDEO_DATA_USAGE, bytes)
                    .sendToTarget();
        }

        @Override
        public void notifyRtpReceptionStats(final RtpReceptionStats stats) {
            ImsLog.d("notifyRtpReceptionStats: stats=" + stats);

            Message.obtain(mVideoMessageHandler, MediaConstants.NOTIFY_RTP_RECEPTION_STATS, stats)
                    .sendToTarget();
        }
    }

    /** Implements Interface to receive callbacks when the QoS is connected or disconnected. */
    private class VideoImsQosCallback implements ImsQosCallback {

        @Override
        public void onNotifyQosConnectionAvailable(InetSocketAddress remoteAddress) {
            ImsLog.i("ImsQosCallback - connected");

            if (mVideoSessionCallbackHandler != null) {
                mVideoSessionCallbackHandler.onNotifyQosInfo(
                        remoteAddress.getAddress().getHostAddress(), remoteAddress.getPort(), true);
            }
        }

        @Override
        public void onNotifyQosConnectionLost(InetSocketAddress remoteAddress) {
            ImsLog.i("ImsQosCallback - disconnected");

            if (mVideoSessionCallbackHandler != null) {
                mVideoSessionCallbackHandler.onNotifyQosInfo(
                        remoteAddress.getAddress().getHostAddress(),
                        remoteAddress.getPort(),
                        false);
            }
        }
    }

    /**
     * Handles Video Session requests received from Media Native
     *
     * @param requestType type of the request
     * @param parcel parcel received from Media Native
     */
    public void onImsMediaVideoMessage(final int requestType, Parcel parcel) {
        ImsLog.d("requestType= " + requestType);

        switch (requestType) {
            /** Requests (ImsStack -> ImsMedia) */
            case MediaConstants.REQUEST_OPEN_SESSION:
            {
                setMediaState(MEDIA_STATE_OPENING);
                mLocalIpAddress = parcel.readString();
                mLocalPortNumber = parcel.readInt();
                ImsLog.d("localIpAddress= " + mLocalIpAddress
                        + " localPortNumber= " + mLocalPortNumber);

                Message.obtain(mVideoMessageHandler, requestType).sendToTarget();
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

            case MediaConstants.REQUEST_QOS:
            {
                String remoteIpAddress = parcel.readString();
                int remotePortNumber = parcel.readInt();
                ImsLog.d("remoteIpAddress= " + remoteIpAddress
                        + " remotePortNumber= " + remotePortNumber);

                Message.obtain(
                        mVideoMessageHandler, requestType, remotePortNumber, UNUSED,
                        remoteIpAddress).sendToTarget();
            }
                break;
            case MediaConstants.REQUEST_SET_MEDIA_QUALITY:
            {
                MediaQualityThreshold threshold =
                        MediaQualityThreshold.CREATOR.createFromParcel(parcel);
                ImsLog.d("onVideoSetMediaQualityThreshold: " + threshold.toString());

                Message.obtain(mVideoMessageHandler, requestType, threshold).sendToTarget();
            }
                break;
            case MediaConstants.REQUEST_ADJUST_DELAY:
                Message.obtain(
                        mVideoMessageHandler, requestType, parcel.readInt()).sendToTarget();
                break;
            default:
            {
                parcel.recycle();
                ImsLog.e("Invalid RequestType");
            }
                break;
        }
    }

    /**
     * Informs whether requestType is valid or not
     *
     * @param requestType type of the request
     * @return true if requestType is valid
     */
    public boolean isValidRequest(final int requestType) {
        return ((isIdle() && (requestType == MediaConstants.REQUEST_OPEN_SESSION))
                || ((!isIdle() && (requestType != MediaConstants.REQUEST_OPEN_SESSION))
                && !isClosed()));
    }

    private void createQosAgent(int slotId) {
        if (mVideoQosAgent == null) {
            mVideoQosAgent = new QosAgent(slotId);
        }
        if (mVideoImsQosCallback == null) {
            mVideoImsQosCallback = new VideoImsQosCallback();
        }
        mVideoQosAgent.setCallback(mVideoImsQosCallback);
    }

    /**
     * Gets the sampling rate of the rtp timestamp
     */
    public int getSamplingRateKHz() {
        return 90;
    }

    private void handleVideoOpenSession(VideoConfig videoConfig) {

        if (mVideoSession == null) {
            if (mMediaManager.isImsMediaConnected()) {
                mRtpSocket = mVideoQosAgent.createQosConnection(mLocalIpAddress, mLocalPortNumber);

                if (mRtpSocket == null) {
                    ImsLog.e("rtp socket creation failed");
                    if (mVideoSessionCallbackHandler != null) {
                        mVideoSessionCallbackHandler.openSessionResponse(
                                ImsMediaSession.RESULT_PORT_UNAVAILABLE);
                    }
                    setMediaState(MEDIA_STATE_IDLE);
                    return;
                } else if (mRtpSocket.first == null || mRtpSocket.second == null) {
                    ImsLog.e("rtp socket creation failed");
                    closeSockets();
                    if (mVideoSessionCallbackHandler != null) {
                        mVideoSessionCallbackHandler.openSessionResponse(
                                ImsMediaSession.RESULT_PORT_UNAVAILABLE);
                    }
                    setMediaState(MEDIA_STATE_IDLE);
                    return;
                }

                mMediaManager.openSession(mRtpSocket.first, mRtpSocket.second,
                        ImsMediaSession.SESSION_TYPE_VIDEO, videoConfig, mVideoSessionCallback);
                setMediaState(MEDIA_STATE_OPENING);
            } else {
                ImsLog.d("ImsMediaManager is not ready");
                if (mVideoSessionCallbackHandler != null) {
                    mVideoSessionCallbackHandler.openSessionResponse(
                            ImsMediaSession.RESULT_NOT_READY);
                }
                setMediaState(MEDIA_STATE_IDLE);
            }
        } else {
            ImsLog.w("Video Session is already created: SessionId=" + mVideoSession.getSessionId());
            if (mVideoSessionCallbackHandler != null) {
                mVideoSessionCallbackHandler.openSessionResponse(
                        ImsMediaSession.RESULT_NOT_SUPPORTED);
            }
        }
    }

    private void handleVideoCloseSession() {

        if (mVideoSession != null) {
            mMediaManager.closeSession(mVideoSession);
            setMediaState(MEDIA_STATE_CLOSED);
        }
    }

    private void closeSockets() {
        if (mRtpSocket != null) {
            mVideoQosAgent.destroyQosConnection(mRtpSocket.first, mRtpSocket.second);
        }
    }

    private void handleVideoModifySession(VideoConfig videoConfig) {
        if (mVideoSession != null) {
            mVideoSession.modifySession(videoConfig);
        } else {
            handleVideoModifySessionResponse(videoConfig, ImsMediaSession.RESULT_NOT_READY);
        }
    }

    private void handleVideoQos(String remoteIpAddress, int remotePortNumber)  {
        if (remoteIpAddress != null && remotePortNumber != 0
                && isNewRemoteAddress(remoteIpAddress, remotePortNumber)) {
            mVideoQosAgent.updateQosConnection(mRtpSocket.first, mRtpSocket.second,
                    remoteIpAddress, remotePortNumber, false);
            ImsLog.d("Updated QoS Connection for remoteIpAddress= " + remoteIpAddress
                    + " remotePortNumber= " + remotePortNumber);
        }
    }

    private void handleSetPreviewSurface() {
        if (mVideoSession != null && mMtcMediaVideoCallProvider != null) {
            if (mMtcMediaVideoCallProvider.getPreviewSurface() != null) {
                mVideoSession.setPreviewSurface(mMtcMediaVideoCallProvider.getPreviewSurface());
                mPreviewSurfaceSet = true;
            } else {
                mPreviewSurfaceSet = false;
            }
        } else {
            mPreviewSurfaceSet = false;
        }
    }

    private void handleSetDisplaySurface() {
        if (mVideoSession != null && mMtcMediaVideoCallProvider != null) {
            if (mMtcMediaVideoCallProvider.getDisplaySurface() != null) {
                mVideoSession.setDisplaySurface(mMtcMediaVideoCallProvider.getDisplaySurface());
                mDisplaySurfaceSet = true;
            } else {
                mDisplaySurfaceSet = false;
            }
        } else {
            mDisplaySurfaceSet = false;
        }
    }

    private void handleVideoSetMediaQualityThreshold(MediaQualityThreshold threshold) {
        if (mVideoSession != null) {
            mVideoSession.setMediaQualityThreshold(threshold);

            final int reportingIntervalMillis = 3000;
            ImsLog.d("requestRtpReceptionStats - intervalMs=" + reportingIntervalMillis);
            mVideoSession.requestRtpReceptionStats(reportingIntervalMillis);
        }
    }

    private void handleVideoDataUsageRequest() {
        if (mVideoSession != null) {
            mVideoSession.requestVideoDataUsage();
        }
    }

    private void handleAdjustDelay(int delay) {
        if (mVideoSession != null) {
            mVideoSession.adjustDelay(delay);
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
                setMediaState(MEDIA_STATE_IDLE);
                return;
            }

            mVideoSessionId = mVideoSession.getSessionId();
            setMediaState(MEDIA_STATE_LIVE);
            ImsLog.d("Video Session created: SessionId=" + mVideoSessionId);
            if (!mPreviewSurfaceSet) {
                Message.obtain(mVideoMessageHandler, MediaConstants.REQUEST_SET_PREVIEW_SURFACE)
                        .sendToTarget();
            }

            if (!mDisplaySurfaceSet) {
                Message.obtain(mVideoMessageHandler, MediaConstants.REQUEST_SET_DISPLAY_SURFACE)
                        .sendToTarget();
            }
        } else {
            setMediaState(MEDIA_STATE_IDLE);
        }

        if (mVideoSessionCallbackHandler != null) {
            mVideoSessionCallbackHandler.openSessionResponse(result);
        }
    }

    private void handleVideoSessionClosed() {
        closeSockets();
        setMediaState(MEDIA_STATE_IDLE);
        mVideoSession = null;
        mVideoSessionId = 0;
        mVideoMessageHandler.removeCallbacksAndMessages(null);
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

    private void handleVideoBitrateNotification(final int bitrate) {
        if (mVideoSessionCallbackHandler != null) {
            mVideoSessionCallbackHandler.onNotifyBitrate(bitrate);
        }
    }

    private void handleVideoDataUsageNotification(final long bytes) {
        if (mMtcMediaVideoCallProvider != null) {
            mMtcMediaVideoCallProvider.onNotifyVideoDataUsage(bytes);
        }
    }

    private void handleNotifyRtpReceptionStats(final RtpReceptionStats stats) {
        if (mVideoSessionCallbackHandler != null) {
            mVideoSessionCallbackHandler.onNotifyRtpReceptionStats(stats);
        }
    }

    private boolean isNewRemoteAddress(String remoteIpAddress, int remotePortNumber) {
        if (remoteIpAddress != null) {
            InetSocketAddress remoteSocketAddress =
                    (InetSocketAddress) (mRtpSocket.first).getRemoteSocketAddress();
            if (remoteSocketAddress != null) {
                InetAddress remoteInetAddress = remoteSocketAddress.getAddress();
                if (remoteInetAddress != null
                        && remoteIpAddress.equals(remoteInetAddress.getHostAddress())
                        && remotePortNumber == remoteSocketAddress.getPort()) {
                    return false;
                }
            }
        }
        return true;
    }
}
