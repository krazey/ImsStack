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
import android.telephony.CallQuality;
import android.telephony.ims.MediaThreshold;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.AnbrMode;
import android.telephony.imsmedia.AudioConfig;
import android.telephony.imsmedia.AudioSessionCallback;
import android.telephony.imsmedia.EvsParams;
import android.telephony.imsmedia.ImsAudioSession;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.MediaQualityStatus;
import android.telephony.imsmedia.MediaQualityThreshold;
import android.telephony.imsmedia.RtpReceptionStats;
import android.util.Pair;

import com.android.imsstack.core.agents.QosAgent;
import com.android.imsstack.core.agents.QosAgent.ImsQosCallback;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.IMtcMediaInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.List;

/**
 * This manages audio session by communicating between ImsStack (media enabler native)
 * and {@link ImsMediaManager}
 */
public class AudioSessionHandler extends MediaState {

    enum EDirectionType {
        DIRECTION_NONE(0),
        DIRECTION_UPLINK(1),
        DIRECTION_DOWNLINK(2);

        private final int mDirection;

        EDirectionType(int direction) {
            this.mDirection = direction;
        }

        public int getDirection() {
            return mDirection;
        }
    }

    static final int UNUSED = -1;

    /** Adaptive Multi-Rate */
    static final int CODEC_AMR = AudioConfig.CODEC_AMR;

    /** Adaptive Multi-Rate Wide Band */
    static final int CODEC_AMR_WB = AudioConfig.CODEC_AMR_WB;

    /** Enhanced Voice Services */
    static final int CODEC_EVS = AudioConfig.CODEC_EVS;

    /** G.711 A-law i.e. Pulse Code Modulation using A-law */
    static final int CODEC_PCMA = AudioConfig.CODEC_PCMA;

    /** G.711 μ-law i.e. Pulse Code Modulation using μ-law */
    static final int CODEC_PCMU = AudioConfig.CODEC_PCMU;

    static final int AUDIO_TYPE = 1;

    private final AudioSessionCallbackProxy mAudioSessionCallback;
    private ImsAudioSession mAudioSession;
    private int mAudioSessionId;
    private final ArrayList<Pair<DatagramSocket, DatagramSocket>> mRtpSocketList =
            new ArrayList<>();
    private final AudioSessionCallbackHandler mAudioSessionCallbackHandler;
    private final MediaManagerHelper mMediaManager;
    private final AudioMessageHandler mAudioMessageHandler;
    private final IBaseContext mContext;
    private QosAgent mAudioQosAgent;
    private AudioImsQosCallback mAudioImsQosCallback;
    private final Object mLock = new Object();
    private boolean mQosUpdateRequired;
    private Pair<String, Integer> mLocalAddress;
    private MediaConfig mMediaConfig;
    private int mCodecType;
    private boolean mAnbrEnabled;

    public AudioSessionHandler(IBaseContext context,
            @NonNull MediaManagerHelper mediaManager, IMtcMediaInterface mtcMediaInterface) {
        super(ImsMediaSession.SESSION_TYPE_AUDIO);
        mContext = context;
        mMediaManager = mediaManager;
        mAudioSessionCallbackHandler = new AudioSessionCallbackHandler(mtcMediaInterface);
        mAudioSessionCallback = new AudioSessionCallbackProxy();
        mAudioMessageHandler = new AudioMessageHandler(mMediaManager.getMediaLooper());
        mMediaConfig = new MediaConfig();
        createQosAgent(mContext.getSlotId());
        mAnbrEnabled = false;
        ImsLog.d("AudioSessionHandler created");
    }

    @VisibleForTesting
    public AudioSessionHandler(IBaseContext context, @NonNull MediaManagerHelper mediaManager,
            @NonNull AudioSessionCallbackHandler audioCallbackHandler,
            @NonNull ImsAudioSession audioSession, MediaConfig mediaConfig, Looper looper) {
        super(ImsMediaSession.SESSION_TYPE_AUDIO);
        mContext = context;
        mMediaManager = mediaManager;
        mAudioSessionCallbackHandler = audioCallbackHandler;
        mAudioSession = audioSession;
        mMediaConfig = mediaConfig;
        mAudioSessionCallback = new AudioSessionCallbackProxy();
        mAudioMessageHandler = new AudioMessageHandler(looper);
        mAnbrEnabled = false;
        ImsLog.d("AudioSessionHandler created");
    }

    @VisibleForTesting
    void setAudioSession(@Nullable ImsAudioSession audioSession) {
        mAudioSession = audioSession;
    }

    @VisibleForTesting
    void setAudioQosAgent(@Nullable QosAgent audioQosAgent) {
        mAudioQosAgent = audioQosAgent;
    }

    @VisibleForTesting
    void setLocalAddress(@Nullable String localAddress, int localPort) {
        mLocalAddress = new Pair<>(localAddress, localPort);
    }

    @VisibleForTesting
    void setQosUpdateRequired(boolean flag) {
        mQosUpdateRequired = flag;
    }

    @VisibleForTesting
    void setRtpSocket(@Nullable Pair<DatagramSocket, DatagramSocket> rtpSocket) {
        synchronized (mRtpSocketList) {
            mRtpSocketList.clear();
            mRtpSocketList.add(rtpSocket);
        }
    }

    @VisibleForTesting
    void setAudioAnbrEnabled(boolean anbrEnabled) {
        mAnbrEnabled = anbrEnabled;
    }

    @VisibleForTesting
    void setCodecType(int codecType) {
        mCodecType = codecType;
    }

    @VisibleForTesting
    AudioSessionCallback getAudioSessionCallback() {
        return mAudioSessionCallback;
    }

    @VisibleForTesting
    int getAudioSessionId() {
        return mAudioSessionId;
    }

    @VisibleForTesting
    AudioMessageHandler getAudioMessageHandler() {
        return mAudioMessageHandler;
    }

    @VisibleForTesting
    boolean getAudioAnbrEnabled() {
        return mAnbrEnabled;
    }

    private boolean isWaitRequired(int requestType) {
        return (requestType != MediaConstants.REQUEST_OPEN_SESSION
                && requestType != MediaConstants.RESPONSE_OPEN_SESSION
                && requestType != MediaConstants.REQUEST_QOS
                && requestType != MediaConstants.NOTIFY_MEDIA_DETACH);
    }

    /** Audio session message Handler */
    @SuppressWarnings({"unchecked", "WaitNotInLoop"})
    class AudioMessageHandler extends Handler {

        AudioMessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            ImsLog.d("messageType = " + msg.what);

            // Till open session response is received, handling other commands has to wait
            try {
                synchronized (mLock) {
                    if (mAudioSession == null && isWaitRequired(msg.what)) {
                        ImsLog.d(Thread.currentThread().getName()
                                + " is waiting for Audio openSession response");
                        mLock.wait(MediaConstants.RESPONSE_WAIT_TIMEOUT);
                        if (mAudioSession == null) {
                            ImsLog.e("Audio openSession response timeout");
                            handleOpenSessionResponse(null, ImsMediaSession.RESULT_NOT_READY);
                            return;
                        }
                        ImsLog.d(Thread.currentThread().getName()
                                + " received Audio openSession response");
                    }
                }
            } catch (InterruptedException ie) {
                ImsLog.e("unexpectedly interrupted while waiting" + ie.getMessage());
            }

            switch (msg.what) {
                case MediaConstants.REQUEST_OPEN_SESSION:
                {
                    handleAudioOpenSession((String) msg.obj, msg.arg1);
                }
                    break;

                case MediaConstants.REQUEST_CLOSE_SESSION:
                {
                    handleAudioCloseSession();
                }
                    break;

                case MediaConstants.REQUEST_MODIFY_SESSION:
                {
                    handleAudioModifySession((AudioConfig) msg.obj);
                }
                    break;

                case MediaConstants.REQUEST_QOS:
                {
                    handleAudioQos((String) msg.obj, msg.arg1);
                }
                    break;

                case MediaConstants.REQUEST_ADD_CONFIG:
                {
                    handleAudioAddConfig((AudioConfig) msg.obj);
                }
                    break;

                case MediaConstants.REQUEST_DELETE_CONFIG:
                {
                    handleAudioDeleteConfig((AudioConfig) msg.obj);
                }
                    break;

                case MediaConstants.REQUEST_CONFIRM_CONFIG:
                {
                    handleAudioConfirmConfig((AudioConfig) msg.obj);
                }
                    break;

                case MediaConstants.REQUEST_SEND_DTMF:
                {
                    handleAudioSendDtmf((char) msg.obj, msg.arg1);
                }
                    break;

                case MediaConstants.REQUEST_SET_MEDIA_QUALITY:
                {
                    handleAudioSetMediaQualityThreshold(
                            (MediaQualityThreshold) msg.obj, msg.arg1 == 1);
                }
                    break;

                case MediaConstants.REQUEST_SET_QNS_MEDIA_THRESHOLD:
                {
                    handleAudioSetQnsMediaThreshold((MediaThreshold) msg.obj);
                }
                    break;

                case MediaConstants.REQUEST_HEADER_EXTENSION:
                {
                    handleAudioSendHeaderExtension((ArrayList<RtpHeaderExtension>) msg.obj);
                }
                    break;

                case MediaConstants.REQUEST_UPDATE_ANBR_ENABLED_CONFIG:
                {
                    handleAudioUpdateAnbrEnabledConfig((boolean) msg.obj);
                }
                    break;

                case MediaConstants.NOTIFY_ANBR_RECEIVED:
                {
                    handleAudioAnbrReceived((int) msg.obj, msg.arg1, msg.arg2);
                }
                    break;

                case MediaConstants.RESPONSE_OPEN_SESSION:
                {
                    handleOpenSessionResponse((ImsMediaSession) msg.obj, msg.arg1);
                }
                    break;

                case MediaConstants.RESPONSE_SESSION_CLOSED:
                {
                    handleSessionClosed();
                }
                    break;

                case MediaConstants.RESPONSE_MODIFY_SESSION:
                {
                    handleModifySessionResponse((AudioConfig) msg.obj, msg.arg1);
                }
                    break;

                case MediaConstants.RESPONSE_ADD_CONFIG:
                {
                    handleAddConfigResponse((AudioConfig) msg.obj, msg.arg1);
                }
                    break;

                case MediaConstants.RESPONSE_CONFIRM_CONFIG:
                {
                    handleConfirmConfigResponse((AudioConfig) msg.obj, msg.arg1);
                }
                    break;

                case MediaConstants.NOTIFY_FIRST_PACKET:
                {
                    handleFirstMediaPacketReceived((AudioConfig) msg.obj);
                }
                    break;

                case MediaConstants.NOTIFY_HEADER_EXTENSION:
                {
                    handleHeaderExtensionReceived((List<RtpHeaderExtension>) msg.obj);
                }
                    break;

                case MediaConstants.NOTIFY_MEDIA_QUALITY_STATUS:
                {
                    handleMediaQualityStatusNotification((MediaQualityStatus) msg.obj);
                }
                    break;

                case MediaConstants.NOTIFY_CALL_QUALITY_CHANGE:
                {
                    handleCallQualityChanged((CallQuality) msg.obj);
                }
                    break;

                case MediaConstants.NOTIFY_MEDIA_DETACH:
                {
                    handleAudioDisconnection();
                }
                    break;

                case MediaConstants.TRIGGER_ANBR_QUERY:
                {
                    handleTriggerAnbrQuery((AudioConfig) msg.obj);
                }
                    break;

                case MediaConstants.NOTIFY_RTP_RECEPTION_STATS:
                    handleNotifyRtpReceptionStats((RtpReceptionStats) msg.obj);
                    break;

                case MediaConstants.NOTIFY_DTMF_RECEIVED:
                    handleNotifyIncomingDtmfReceived((int) msg.obj, msg.arg1);
                    break;

                default:
                {
                    ImsLog.e("Invalid RequestType");
                }
                    break;
            }
        }
    }

    private class AudioSessionCallbackProxy extends AudioSessionCallback {

        @Override
        public void onOpenSessionSuccess(ImsMediaSession session) {
            // Release the wait as OpenSession Response is received
            synchronized (mLock) {
                mAudioSession = (ImsAudioSession) session;
                mLock.notifyAll();
            }
            Message.obtain(mAudioMessageHandler, MediaConstants.RESPONSE_OPEN_SESSION,
                    ImsMediaSession.RESULT_SUCCESS, UNUSED, session).sendToTarget();
        }

        @Override
        public void onOpenSessionFailure(final @ImsMediaSession.SessionOperationResult int error) {
            ImsLog.d("error=" + error);
            // Release the wait as OpenSession Response is received
            synchronized (mLock) {
                mLock.notifyAll();
            }
            Message.obtain(mAudioMessageHandler, MediaConstants.RESPONSE_OPEN_SESSION,
                    error, UNUSED, null).sendToTarget();
        }

        @Override
        public void onSessionClosed() {
            ImsLog.d("onSessionClosed for SessionId[" + getAudioSessionId() + "]");

            Message.obtain(mAudioMessageHandler, MediaConstants.RESPONSE_SESSION_CLOSED)
                    .sendToTarget();
        }

        @Override
        public void onModifySessionResponse(final AudioConfig audioConfig,
                final @ImsMediaSession.SessionOperationResult int result) {
            ImsLog.d("result=" + result);

            Message.obtain(mAudioMessageHandler, MediaConstants.RESPONSE_MODIFY_SESSION,
                    result, UNUSED, audioConfig).sendToTarget();
        }

        @Override
        public void onAddConfigResponse(final AudioConfig audioConfig,
                final @ImsMediaSession.SessionOperationResult int result) {
            ImsLog.d("result=" + result);

            Message.obtain(mAudioMessageHandler, MediaConstants.RESPONSE_ADD_CONFIG,
                    result, UNUSED, audioConfig).sendToTarget();
        }

        @Override
        public void onConfirmConfigResponse(final AudioConfig audioConfig,
                final @ImsMediaSession.SessionOperationResult int result) {
            ImsLog.d("result=" + result);

            Message.obtain(mAudioMessageHandler, MediaConstants.RESPONSE_CONFIRM_CONFIG,
                    result, UNUSED, audioConfig).sendToTarget();
        }

        @Override
        public void onFirstMediaPacketReceived(final AudioConfig audioConfig) {
            ImsLog.d("FirstMediaPacketReceived for SessionId[" + getAudioSessionId() + "]");

            Message.obtain(mAudioMessageHandler, MediaConstants.NOTIFY_FIRST_PACKET, audioConfig)
                    .sendToTarget();
        }

        @Override
        public void onHeaderExtensionReceived(final List<RtpHeaderExtension> extensions) {
            ImsLog.d("onHeaderExtensionReceived");

            Message.obtain(mAudioMessageHandler, MediaConstants.NOTIFY_HEADER_EXTENSION,
                    extensions).sendToTarget();
        }

        @Override
        public void notifyMediaQualityStatus(final MediaQualityStatus qualityStatus) {
            ImsLog.d("notifyMediaQualityStatus: " + qualityStatus.toString());
            Message.obtain(mAudioMessageHandler, MediaConstants.NOTIFY_MEDIA_QUALITY_STATUS,
                qualityStatus).sendToTarget();
        }

        @Override
        public void onCallQualityChanged(@NonNull final CallQuality callQuality) {
            ImsLog.d("Media Quality Changed: " + callQuality.toString());

            Message.obtain(mAudioMessageHandler, MediaConstants.NOTIFY_CALL_QUALITY_CHANGE,
                    callQuality).sendToTarget();
        }

        @Override
        public void triggerAnbrQuery(final AudioConfig audioConfig) {
            ImsLog.d("triggerAnbrQuery for SessionId[" + getAudioSessionId() + "]");

            Message.obtain(mAudioMessageHandler, MediaConstants.TRIGGER_ANBR_QUERY, audioConfig)
                    .sendToTarget();
        }

        @Override
        public void notifyRtpReceptionStats(final RtpReceptionStats stats) {
            ImsLog.d("notifyRtpReceptionStats: stats=" + stats);

            Message.obtain(mAudioMessageHandler, MediaConstants.NOTIFY_RTP_RECEPTION_STATS, stats)
                    .sendToTarget();
        }

        @Override
        public void onDtmfReceived(final char dtmfDigit, final int durationMs) {
            ImsLog.d("onDtmfReceived: digit=" + ((int) dtmfDigit) + ", duration=" + durationMs);

            Message.obtain(mAudioMessageHandler, MediaConstants.NOTIFY_DTMF_RECEIVED,
                    durationMs, UNUSED, (int) dtmfDigit).sendToTarget();
        }
    }

    /** Implements Interface to receive callbacks when the QoS is connected or disconnected. */
    private class AudioImsQosCallback implements ImsQosCallback {

        @Override
        public void onNotifyQosConnectionAvailable(InetSocketAddress remoteAddress) {
            ImsLog.i("ImsQosCallback - connected");

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.onNotifyQosInfo(
                        remoteAddress.getAddress().getHostAddress(),
                        remoteAddress.getPort(), true);
            }
        }

        @Override
        public void onNotifyQosConnectionLost(InetSocketAddress remoteAddress) {
            ImsLog.i("ImsQosCallback - disconnected");

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.onNotifyQosInfo(
                        remoteAddress.getAddress().getHostAddress(),
                        remoteAddress.getPort(), false);
            }
        }
    }

    /**
     * Handles Audio Session requests received from Media Native
     *
     * @param requestType type of the request
     * @param parcel parcel received from Media Native
     */
    public void onImsMediaAudioMessage(final int requestType, Parcel parcel) {
        ImsLog.d("requestType= " + requestType);

        switch (requestType) {
            /** Requests (ImsStack -> ImsMedia) */
            case MediaConstants.REQUEST_OPEN_SESSION:
            {
                setMediaState(MEDIA_STATE_OPENING);
                String localIpAddress = parcel.readString();
                int localPortNumber = parcel.readInt();
                ImsLog.d("localIpAddress= " + localIpAddress
                        + " localPortNumber= " + localPortNumber);

                Message.obtain(
                        mAudioMessageHandler, requestType, localPortNumber, UNUSED, localIpAddress)
                        .sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_CLOSE_SESSION:
            case MediaConstants.NOTIFY_MEDIA_DETACH:
            {
                Message.obtain(mAudioMessageHandler, requestType).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_MODIFY_SESSION:
            case MediaConstants.REQUEST_ADD_CONFIG:
            case MediaConstants.REQUEST_DELETE_CONFIG:
            case MediaConstants.REQUEST_CONFIRM_CONFIG:
            {
                AudioConfig audioConfig = AudioConfig.CREATOR.createFromParcel(parcel);
                Message.obtain(mAudioMessageHandler, requestType, audioConfig).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_QOS:
            {
                String remoteIpAddress = parcel.readString();
                int remotePortNumber = parcel.readInt();
                ImsLog.d("remoteIpAddress= " + remoteIpAddress
                        + " remotePortNumber= " + remotePortNumber);

                Message.obtain(
                        mAudioMessageHandler, requestType, remotePortNumber, UNUSED,
                        remoteIpAddress).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_SEND_DTMF:
            {
                char dtmfDigit = (char)parcel.readByte();
                int duration = parcel.readInt();
                ImsLog.d("dtmfDigit=" + dtmfDigit + ", duration=" + duration);

                Message.obtain(mAudioMessageHandler, requestType, duration, UNUSED, dtmfDigit)
                        .sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_SET_MEDIA_QUALITY:
            {
                MediaQualityThreshold threshold =
                    MediaQualityThreshold.CREATOR.createFromParcel(parcel);
                Boolean needFwkTimer = parcel.readBoolean();
                ImsLog.d("MediaQualityThreshold: " + threshold.toString() + ", needFwkTimer="
                        + needFwkTimer);

                Message.obtain(mAudioMessageHandler, requestType, needFwkTimer ? 1 : 0, UNUSED,
                    threshold).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_SET_QNS_MEDIA_THRESHOLD:
            {
                MediaThreshold mediaThreshold = MediaThreshold.CREATOR.createFromParcel(parcel);
                ImsLog.d("MediaThreshold: " + mediaThreshold.toString());

                Message.obtain(mAudioMessageHandler, requestType, mediaThreshold).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_HEADER_EXTENSION:
            {
                int rtpExtensionsListSize = parcel.readInt();
                ArrayList<RtpHeaderExtension> rtpExtensions = new ArrayList<RtpHeaderExtension>();
                if (rtpExtensionsListSize!=0) {
                    for (int i = 0; i < rtpExtensionsListSize; ++i) {
                        rtpExtensions.add(RtpHeaderExtension.CREATOR.createFromParcel(parcel));
                    }
                }

                Message.obtain(mAudioMessageHandler, requestType, rtpExtensions).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_UPDATE_ANBR_ENABLED_CONFIG:
            {
                Boolean anbrEnabled = parcel.readBoolean();
                ImsLog.d("anbr config=" + anbrEnabled);

                Message.obtain(mAudioMessageHandler, requestType, anbrEnabled).sendToTarget();
            }
                break;

            case MediaConstants.NOTIFY_ANBR_RECEIVED:
            {
                int mediaType = parcel.readInt();
                int direction = parcel.readInt();
                int bitrate = parcel.readInt();
                ImsLog.d("media type=" + mediaType + " direction=" + direction
                        + " bitrate=" + bitrate);
                Message.obtain(mAudioMessageHandler, requestType, direction, bitrate, mediaType)
                        .sendToTarget();
            }
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
        if (mAudioQosAgent == null) {
            mAudioQosAgent = new QosAgent(slotId);
        }
        if (mAudioImsQosCallback == null) {
            mAudioImsQosCallback = new AudioImsQosCallback();
        }
        mAudioQosAgent.setCallback(mAudioImsQosCallback);
    }

    /**
     * Gets the sampling rate of the rtp timestamp
     */
    public int getSamplingRateKHz() {
        if (mCodecType == CODEC_AMR) {
            return 8;
        }
        return 16;
    }

    private void handleAudioOpenSession(String localIpAddress, int localPortNumber) {

        mCodecType = UNUSED;

        if (mAudioSession == null) {
            if (mMediaManager.isImsMediaConnected()) {

                Pair<DatagramSocket, DatagramSocket> rtpSocket =
                        mAudioQosAgent.createQosConnection(localIpAddress, localPortNumber);
                synchronized (mRtpSocketList) {
                    mRtpSocketList.clear();
                    mRtpSocketList.add(rtpSocket);
                }
                setQosUpdateRequired(true);
                setLocalAddress(localIpAddress, localPortNumber);

                if (rtpSocket == null) {
                    ImsLog.e("rtp socket creation failed");
                    if (mAudioSessionCallbackHandler != null) {
                        mAudioSessionCallbackHandler.openSessionResponse(
                                ImsMediaSession.RESULT_PORT_UNAVAILABLE);
                    }
                    setMediaState(MEDIA_STATE_IDLE);
                    return;
                } else if (rtpSocket.first == null || rtpSocket.second == null) {
                    ImsLog.e("rtp socket creation failed");
                    closeSockets();
                    if (mAudioSessionCallbackHandler != null) {
                        mAudioSessionCallbackHandler.openSessionResponse(
                                ImsMediaSession.RESULT_PORT_UNAVAILABLE);
                    }
                    setMediaState(MEDIA_STATE_IDLE);
                    return;
                }

                mMediaManager.openSession(rtpSocket.first, rtpSocket.second,
                        ImsMediaSession.SESSION_TYPE_AUDIO, null, mAudioSessionCallback);
                setMediaState(MEDIA_STATE_OPENING);
            } else {
                ImsLog.d("ImsMediaManager is not ready");
                if (mAudioSessionCallbackHandler != null) {
                    mAudioSessionCallbackHandler.openSessionResponse(
                            ImsMediaSession.RESULT_NOT_READY);
                }
                setMediaState(MEDIA_STATE_IDLE);
            }
        } else {
            ImsLog.w("Audio Session is already created: SessionId="
                + mAudioSession.getSessionId());
            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.openSessionResponse(
                        ImsMediaSession.RESULT_NOT_SUPPORTED);
            }
        }
    }

    private void handleAudioCloseSession() {
        if (mAudioSession != null) {
            mMediaManager.closeSession(mAudioSession);
            setMediaState(MEDIA_STATE_CLOSED);

            mCodecType = UNUSED;
        }
    }

    private void closeSockets() {
        synchronized (mRtpSocketList) {
            for (Pair<DatagramSocket, DatagramSocket> rtpSocket : mRtpSocketList) {
                mAudioQosAgent.destroyQosConnection(rtpSocket.first, rtpSocket.second);
            }
            mRtpSocketList.clear();
        }
    }

    private void handleAudioModifySession(AudioConfig audioConfig) {
        if (mAudioSession != null) {
            mCodecType = audioConfig.getCodecType();
            mAudioSession.modifySession(audioConfig);
            mMediaConfig.updateRtpConfig(audioConfig);
        } else {
            handleModifySessionResponse(audioConfig, ImsMediaSession.RESULT_NOT_READY);
        }
    }

    private void handleAudioQos(String remoteIpAddress, int remotePortNumber) {
        if (isOpening() || isLive()) {
            synchronized (mRtpSocketList) {
                if (remoteIpAddress != null && remotePortNumber != 0 && mLocalAddress != null) {
                    if (mQosUpdateRequired) {
                        updateQosConnection(remoteIpAddress, remotePortNumber, false);
                        setQosUpdateRequired(false);
                    } else if (isNewRemoteAddress(remoteIpAddress, remotePortNumber)) {
                        updateQosConnection(remoteIpAddress, remotePortNumber, true);
                    }
                }
            }
        }
        else {
            ImsLog.d("OpenSession was not successful or session is already closed, state : "
                    + getMediaState());
        }
    }

    private void updateQosConnection(
            String remoteIpAddress, int remotePortNumber, boolean isNewRemote) {
        if (!mRtpSocketList.isEmpty()) {
            Pair<DatagramSocket, DatagramSocket> rtpSocket = mRtpSocketList.get(0);
            if (rtpSocket != null) {
                mAudioQosAgent.updateQosConnection(rtpSocket.first,
                        rtpSocket.second, remoteIpAddress, remotePortNumber, isNewRemote);
                ImsLog.d("Updated QoS Connection for remoteIpAddress= " + remoteIpAddress
                        + " remotePortNumber= " + remotePortNumber
                        + " new Remote received= " + isNewRemote);
            }
        } else {
            ImsLog.d("Rtp socket list is empty");
        }
    }

    private void handleAudioAddConfig(AudioConfig audioConfig) {
        if (mAudioSession != null) {
            // TODO : rtpSocket has to be sent via addConfig
            mCodecType = audioConfig.getCodecType();
            ImsLog.d("handleAudioAddConfig: codec type: " + mCodecType);

            mAudioSession.addConfig(audioConfig);
            mMediaConfig.updateRtpConfig(audioConfig);
        } else {
            handleAddConfigResponse(audioConfig, ImsMediaSession.RESULT_NOT_READY);
        }
    }

    private void handleAudioDeleteConfig(AudioConfig audioConfig) {
        if (mAudioSession != null) {
            InetSocketAddress remoteRtpAddress = audioConfig.getRemoteRtpAddress();
            if (remoteRtpAddress != null) {
                InetAddress remoteInetAddress = remoteRtpAddress.getAddress();
                int remotePort = remoteRtpAddress.getPort();
                Pair<DatagramSocket, DatagramSocket> rtpSocket =
                        getRtpSocketFromList(remoteInetAddress.getHostAddress(), remotePort);
                if (rtpSocket != null) {
                    mAudioQosAgent.destroyQosConnection(rtpSocket.first, rtpSocket.second);
                    synchronized (mRtpSocketList) {
                        mRtpSocketList.remove(rtpSocket);
                    }
                }
            }

            mAudioSession.deleteConfig(audioConfig);
        }
    }

    private void handleAudioConfirmConfig(AudioConfig audioConfig) {
        if (mAudioSession != null) {
            InetSocketAddress remoteRtpAddress = audioConfig.getRemoteRtpAddress();
            if (remoteRtpAddress != null) {
                InetAddress remoteInetAddress = remoteRtpAddress.getAddress();
                if (remoteInetAddress != null) {
                    String confirmedAddress = remoteInetAddress.getHostAddress();
                    int confirmedPort = remoteRtpAddress.getPort();
                    Pair<DatagramSocket, DatagramSocket> confirmedRtpSocket =
                            getRtpSocketFromList(confirmedAddress, confirmedPort);
                    synchronized (mRtpSocketList) {
                        for (Pair<DatagramSocket, DatagramSocket> rtpSocket : mRtpSocketList) {
                            InetSocketAddress remoteSocketAddress =
                                    (InetSocketAddress) (rtpSocket.first).getRemoteSocketAddress();
                            if (confirmedAddress != null
                                    && (!confirmedAddress.equals(
                                            remoteSocketAddress.getAddress().getHostAddress())
                                    || confirmedPort != remoteSocketAddress.getPort())) {
                                mAudioQosAgent.destroyQosConnection(rtpSocket.first,
                                        rtpSocket.second);
                                ImsLog.d("destroyed QoS Connection for remoteIpAddress= "
                                        + remoteSocketAddress.getAddress().getHostAddress()
                                        + " remotePortNumber= " + remoteSocketAddress.getPort());
                            }
                        }
                        mRtpSocketList.clear();
                        mRtpSocketList.add(confirmedRtpSocket);
                        ImsLog.d("rtpSocketList has [%d] sockets available"
                                + mRtpSocketList.size());
                    }
                    mCodecType = audioConfig.getCodecType();
                    ImsLog.d("handleAudioConfirmConfig: codec type: " + mCodecType);

                    mAudioSession.confirmConfig(audioConfig);
                    mMediaConfig.updateRtpConfig(audioConfig);
                } else {
                    handleConfirmConfigResponse(audioConfig, ImsMediaSession.RESULT_INVALID_PARAM);
                }
            } else {
                handleConfirmConfigResponse(audioConfig, ImsMediaSession.RESULT_INVALID_PARAM);
            }
        }
    }

    private void handleAudioSendDtmf(char dtmfDigit, int duration) {
        if (mAudioSession != null) {
            mAudioSession.sendDtmf(dtmfDigit, duration);
        }
    }

    private void handleAudioSetMediaQualityThreshold(
            MediaQualityThreshold mediaThreshold, Boolean needFwkTimer) {
        mMediaConfig.updateMediaQualityThreshold(mediaThreshold, needFwkTimer);
        setAudioQualityThreshold();
    }

    private void handleAudioSetQnsMediaThreshold(MediaThreshold mediaThreshold) {
        mMediaConfig.updateMediaQualityThreshold(mediaThreshold);
        setAudioQualityThreshold();
    }

    private void setAudioQualityThreshold() {
        if (mAudioSession != null) {
            MediaQualityThreshold mediaQualityThreshold = mMediaConfig.getMediaQualityThreshold();
            ImsLog.d("setMediaQualityThreshold: " + mediaQualityThreshold.toString());
            mAudioSession.setMediaQualityThreshold(mediaQualityThreshold);

            final int reportingIntervalMillis = 3000;
            ImsLog.d("requestRtpReceptionStats - intervalMs=" + reportingIntervalMillis);
            mAudioSession.requestRtpReceptionStats(reportingIntervalMillis);
        }
    }

    private void handleAudioSendHeaderExtension(List<RtpHeaderExtension> extensions) {
        if (mAudioSession != null) {
            mAudioSession.sendHeaderExtension(extensions);
        }
    }

    private void handleAudioUpdateAnbrEnabledConfig(Boolean anbrEnabled) {
        if (mAudioSession != null) {
            mAnbrEnabled = anbrEnabled;
        }
    }

    private void handleAudioAnbrReceived(int mediayType, int direction, int bitsPerSecond) {
        ImsLog.d("handleAudioAnbrReceived: bitsPerSecond= " + bitsPerSecond);
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.notifyAnbrReceived(mediayType, direction, bitsPerSecond);
        }
    }

    private void handleOpenSessionResponse(ImsMediaSession session, int result) {
        if (result == ImsMediaSession.RESULT_SUCCESS) {
            if (session == null) {
                ImsLog.e("Audio Session is not created");
                if (mAudioSessionCallbackHandler != null) {
                    mAudioSessionCallbackHandler.openSessionResponse(
                            ImsMediaSession.RESULT_NO_MEMORY);
                }
                setMediaState(MEDIA_STATE_IDLE);
                return ;
            }

            mAudioSessionId = mAudioSession.getSessionId();
            setMediaState(MEDIA_STATE_LIVE);
            ImsLog.d("Audio Session created: SessionId=" + mAudioSessionId);
        } else {
            setMediaState(MEDIA_STATE_IDLE);
        }

        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.openSessionResponse(result);
            mMediaConfig.updateMediaQualityThreshold(
                    mAudioSessionCallbackHandler.getMediaThreshold(
                            ImsMediaSession.SESSION_TYPE_AUDIO));
        }
    }

    private void handleSessionClosed() {
        setMediaState(MEDIA_STATE_IDLE);
        closeSockets();
        mAudioSession = null;
        mAudioSessionId = 0;
        mAudioMessageHandler.removeCallbacksAndMessages(null);
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.closeSessionResponse();
        }
    }

    private void handleAudioDisconnection() {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.nofityMediaDetach();
        }
        handleSessionClosed();
    }

    private void handleModifySessionResponse(final AudioConfig audioConfig, final int result) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.modifySessionResponse(audioConfig, result);
        }
    }

    private void handleAddConfigResponse(final AudioConfig audioConfig, final int result) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.addConfigResponse(audioConfig, result);
        }
    }

    private void handleConfirmConfigResponse(final AudioConfig audioConfig, final int result) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.confirmConfigResponse(audioConfig, result);
        }
    }

    private void handleFirstMediaPacketReceived(final AudioConfig audioConfig) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.firstMediaPacketReceived(audioConfig);
        }
    }

    private void handleHeaderExtensionReceived(final List<RtpHeaderExtension> extensions) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.headerExtensionReceived(extensions);
        }
    }

    private void handleMediaQualityStatusNotification(final MediaQualityStatus qualityStatus) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.onNotifyMediaQualityStatus(
                    mMediaConfig.getRtpConfig().getAccessNetwork(), qualityStatus);
        }
    }

    private void handleCallQualityChanged(final CallQuality callQuality) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.callQualityChanged(callQuality);
        }
    }

    private void handleTriggerAnbrQuery(final AudioConfig audioConfig) {
        if (!mAnbrEnabled) {
            ImsLog.d("Anbr feature is disabled");
            return;
        }

        if (mAudioSessionCallbackHandler != null) {
            AnbrMode anbrMode = audioConfig.getAnbrMode();
            int anbrDirection = -1;
            int bitrate = -1;

            ImsLog.d("handleTriggerAnbrQuery: ul=" + anbrMode.getAnbrUplinkCodecMode() + " dl="
                    + anbrMode.getAnbrDownlinkCodecMode());

            if (anbrMode.getAnbrUplinkCodecMode() > 0) {
                anbrDirection = EDirectionType.DIRECTION_UPLINK.getDirection();
                bitrate = convertCodecModeToBitrate(mCodecType, anbrMode.getAnbrUplinkCodecMode());

            } else if (anbrMode.getAnbrDownlinkCodecMode() > 0) {
                anbrDirection = EDirectionType.DIRECTION_DOWNLINK.getDirection();
                bitrate =
                        convertCodecModeToBitrate(mCodecType, anbrMode.getAnbrDownlinkCodecMode());
            } else {
                ImsLog.d("handleTriggerAnbrQuery: invalid codec mode ");
            }

            ImsLog.d("handleTriggerAnbrQuery: dir= " + anbrDirection + " bitrate= " + bitrate);
            mAudioSessionCallbackHandler.triggerAnbrQuery(AUDIO_TYPE, anbrDirection, bitrate);
        } else {
            ImsLog.d("Enter Anbr Callback is null");
        }
    }

    @VisibleForTesting
    int convertCodecModeToBitrate(int codecType, int codecMode) {
        int convertedBitrate = -1;

        if (codecType == CODEC_EVS) {
            convertedBitrate =
            switch (codecMode) {
                case EvsParams.EVS_MODE_9 -> 5900; // 5.9 kbps (or 2.8)
                case EvsParams.EVS_MODE_10 -> 7200; // 7.2kbps
                case EvsParams.EVS_MODE_11 -> 8000; // 8.0kbps
                case EvsParams.EVS_MODE_12 -> 9600; // 9.6kbps
                case EvsParams.EVS_MODE_13 -> 13200; // 13.2kbps
                case EvsParams.EVS_MODE_14 -> 16400; // 16.4kbps
                case EvsParams.EVS_MODE_15 -> 24400; // 24.4kbps
                case EvsParams.EVS_MODE_16 -> 32000; // 32.0kbps
                case EvsParams.EVS_MODE_17 -> 48000; // 48.0kbps
                case EvsParams.EVS_MODE_18 -> 64000; // 64.0kbps
                case EvsParams.EVS_MODE_19 -> 96000; // 96.0kbps
                case EvsParams.EVS_MODE_20 -> 128000; // 128.0kbps
                default -> { // default value
                    ImsLog.d("convertCodecModeToBitrate: Error - set to 13.2kbps");
                    yield 13200;
                }
            };
        }
        ImsLog.d("convertedBitrate= " + convertedBitrate);
        return convertedBitrate;
    }

    private void handleNotifyRtpReceptionStats(final RtpReceptionStats stats) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.onNotifyRtpReceptionStats(stats);
        }
    }

    private void handleNotifyIncomingDtmfReceived(final int dtmfDigit, final int durationMs) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.onNotifyIncomingDtmfReceived(dtmfDigit, durationMs);
        }
    }

    private boolean isNewRemoteAddress(String remoteIpAddress, int remotePortNumber) {
        synchronized (mRtpSocketList) {
            if (remoteIpAddress != null) {
                for (Pair<DatagramSocket, DatagramSocket> rtpSocket : mRtpSocketList) {
                    InetSocketAddress remoteSocketAddress =
                            (InetSocketAddress) (rtpSocket.first).getRemoteSocketAddress();
                    if (remoteSocketAddress != null) {
                        InetAddress remoteInetAddress = remoteSocketAddress.getAddress();
                        if (remoteInetAddress != null
                                && remoteIpAddress.equals(remoteInetAddress.getHostAddress())
                                && remotePortNumber == remoteSocketAddress.getPort()) {
                            return false;
                        }
                    }
                }
            }
        }
        return true;
    }

    private Pair<DatagramSocket, DatagramSocket> getRtpSocketFromList(
            String remoteIpAddress, int remotePortNumber) {
        synchronized (mRtpSocketList) {
            if (remoteIpAddress != null) {
                for (Pair<DatagramSocket, DatagramSocket> rtpSocket : mRtpSocketList) {
                    InetSocketAddress remoteSocketAddress =
                            (InetSocketAddress) (rtpSocket.first).getRemoteSocketAddress();
                    if (remoteSocketAddress != null) {
                        InetAddress remoteInetAddress = remoteSocketAddress.getAddress();
                        if (remoteInetAddress != null
                                && remoteIpAddress.equals(remoteInetAddress.getHostAddress())
                                && remotePortNumber == remoteSocketAddress.getPort()) {
                            return rtpSocket;
                        }
                    }
                }
            }
        }
        return null;
    }
}
