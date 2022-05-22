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
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.AudioConfig;
import android.telephony.imsmedia.AudioSessionCallback;
import android.telephony.imsmedia.ImsAudioSession;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.MediaQualityThreshold;
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
public class AudioSessionHandler  {

    static final int UNUSED = -1;

    private final AudioSessionCallbackProxy mAudioSessionCallback;
    private ImsAudioSession mAudioSession;
    private int mAudioSessionId;
    private Pair<DatagramSocket, DatagramSocket> mRtpSocket;
    private final AudioSessionCallbackHandler mAudioSessionCallbackHandler;
    private final MediaManagerHelper mMediaManager;
    private final AudioMessageHandler mAudioMessageHandler;
    private final IBaseContext mContext;
    private QosAgent mAudioQosAgent;
    private AudioImsQosCallback mAudioImsQosCallback;
    private InetSocketAddress mRemoteAddress;

    public AudioSessionHandler(IBaseContext context,
            @NonNull MediaManagerHelper mediaManager, IMtcMediaInterface mtcMediaInterface) {
        mContext = context;
        mMediaManager = mediaManager;
        mAudioSessionCallbackHandler = new AudioSessionCallbackHandler(mtcMediaInterface);
        mAudioSessionCallback = new AudioSessionCallbackProxy();
        mAudioMessageHandler = new AudioMessageHandler(mMediaManager.getMediaLooper());
        createQosAgent(mContext.getSlotId());
        ImsLog.d("AudioSessionHandler created");
    }

    @VisibleForTesting
    public AudioSessionHandler(IBaseContext context, @NonNull MediaManagerHelper mediaManager,
            @NonNull AudioSessionCallbackHandler audioCallbackHandler,
            @NonNull ImsAudioSession audioSession) {
        mContext = context;
        mMediaManager = mediaManager;
        mAudioSessionCallbackHandler = audioCallbackHandler;
        mAudioSession = audioSession;
        mAudioSessionCallback = new AudioSessionCallbackProxy();
        mAudioMessageHandler = new AudioMessageHandler(Looper.getMainLooper());
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
    void setRtpSocket(@Nullable Pair<DatagramSocket, DatagramSocket> rtpSocket) {
        mRtpSocket = rtpSocket;
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

    /** Audio session message Handler */
    class AudioMessageHandler extends Handler {

        AudioMessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            ImsLog.v("msg.what= " + msg.what);
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
                    handleAudioSetMediaQualityThreshold((MediaQualityThreshold) msg.obj);
                }
                    break;

                case MediaConstants.REQUEST_HEADER_EXTENSION:
                {
                    handleAudioSendHeaderExtension((ArrayList<RtpHeaderExtension>) msg.obj);
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

                case MediaConstants.RESPONSE_SESSION_CHANGED:
                {
                    handleSessionChanged(msg.arg1);
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

                case MediaConstants.NOTIFY_MEDIA_INACTIVITY:
                {
                    handleMediaInactivityNotification(msg.arg1);
                }
                    break;

                case MediaConstants.NOTIFY_PACKET_LOSS:
                {
                    handlePacketLossNotification(msg.arg1);
                }
                    break;

                case MediaConstants.NOTIFY_JITTER:
                {
                    handleJitterNotification(msg.arg1);
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
            Message.obtain(mAudioMessageHandler, MediaConstants.RESPONSE_OPEN_SESSION,
                    ImsMediaSession.RESULT_SUCCESS, UNUSED, session).sendToTarget();
        }

        @Override
        public void onOpenSessionFailure(final @ImsMediaSession.SessionOperationResult int error) {
            ImsLog.d("error=" + error);

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
        public void onSessionChanged(final @ImsMediaSession.SessionState int state) {
            ImsLog.d("state=" + state);

            Message.obtain(mAudioMessageHandler, MediaConstants.RESPONSE_SESSION_CHANGED,
                    state, UNUSED).sendToTarget();
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

            Message.obtain(mAudioMessageHandler, MediaConstants.NOTIFY_FIRST_PACKET,
                    audioConfig).sendToTarget();
        }

        @Override
        public void onHeaderExtensionReceived(final List<RtpHeaderExtension> extensions) {
            ImsLog.d("onHeaderExtensionReceived");

            Message.obtain(mAudioMessageHandler, MediaConstants.NOTIFY_HEADER_EXTENSION,
                    extensions).sendToTarget();
        }

        @Override
        public void notifyMediaInactivity(final @ImsMediaSession.PacketType int packetType) {
            ImsLog.d("packetType=" + packetType);

            Message.obtain(mAudioMessageHandler, MediaConstants.NOTIFY_MEDIA_INACTIVITY,
                    packetType, UNUSED).sendToTarget();
        }

        @Override
        public void notifyPacketLoss(final int packetLossPercentage) {
            ImsLog.d("packetLossPercentage=" + packetLossPercentage);

            Message.obtain(mAudioMessageHandler, MediaConstants.NOTIFY_PACKET_LOSS,
                    packetLossPercentage, UNUSED).sendToTarget();
        }

        @Override
        public void notifyJitter(final int jitter) {
            ImsLog.d("jitter=" + jitter);

            Message.obtain(mAudioMessageHandler, MediaConstants.NOTIFY_JITTER, jitter, UNUSED)
                    .sendToTarget();
        }

        @Override
        public void onCallQualityChanged(@NonNull final CallQuality callQuality) {
            ImsLog.v("Media Quality Changed: " + callQuality.toString());

            Message.obtain(mAudioMessageHandler, MediaConstants.NOTIFY_CALL_QUALITY_CHANGE,
                    callQuality).sendToTarget();
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
        ImsLog.v("requestType= " + requestType);

        switch (requestType) {
            /** Requests (ImsStack -> ImsMedia) */
            case MediaConstants.REQUEST_OPEN_SESSION:
            {
                String localIpAddress = parcel.readString();
                int localPortNumber = parcel.readInt();
                ImsLog.v("localIpAddress= " + localIpAddress
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

            case MediaConstants.REQUEST_SEND_DTMF:
            {
                char dtmfDigit = (char)parcel.readByte();
                int duration = parcel.readInt();
                ImsLog.v("dtmfDigit=" + dtmfDigit + ", duration=" +duration);

                Message.obtain(mAudioMessageHandler, requestType, duration, UNUSED, dtmfDigit)
                        .sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_SET_MEDIA_QUALITY:
            {
                MediaQualityThreshold threshold =
                    MediaQualityThreshold.CREATOR.createFromParcel(parcel);
                ImsLog.v("onAudioSetMediaQualityThreshold: " + threshold.toString());

                Message.obtain(mAudioMessageHandler, requestType, threshold).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_HEADER_EXTENSION:
            {
                int rtpExtensionsListSize = parcel.readInt();
                ArrayList rtpExtensions = new ArrayList<RtpHeaderExtension>();
                if (rtpExtensionsListSize!=0) {
                    for (int i = 0; i < rtpExtensionsListSize; ++i) {
                        rtpExtensions.add(RtpHeaderExtension.CREATOR.createFromParcel(parcel));
                    }
                }

                Message.obtain(mAudioMessageHandler, requestType, rtpExtensions).sendToTarget();
            }
                break;

            default:
            {
                ImsLog.e("Invalid RequestType");
            }
            break;
        }
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

    private void handleAudioOpenSession(String localIpAddress, int localPortNumber) {

        if(mAudioSession == null) {
            if (mMediaManager.isImsMediaConnected()) {

                mRtpSocket = mAudioQosAgent.createQosConnection(localIpAddress, localPortNumber);

                if (mRtpSocket == null) {
                    ImsLog.e("rtp socket creation failed");
                    if (mAudioSessionCallbackHandler != null) {
                        mAudioSessionCallbackHandler.openSessionResponse(
                            ImsMediaSession.RESULT_PORT_UNAVAILABLE);
                    }
                    return;
                } else if (mRtpSocket.first == null || mRtpSocket.second == null) {
                    ImsLog.e("rtp socket creation failed");
                    closeSockets();
                    if (mAudioSessionCallbackHandler != null) {
                        mAudioSessionCallbackHandler.openSessionResponse(
                                ImsMediaSession.RESULT_PORT_UNAVAILABLE);
                    }
                    return;
                }

                mMediaManager.openSession(mRtpSocket.first, mRtpSocket.second,
                        ImsMediaSession.SESSION_TYPE_AUDIO, null, mAudioSessionCallback);
            }
            else {
                ImsLog.d("ImsMediaManager is not ready");
                if (mAudioSessionCallbackHandler != null) {
                    mAudioSessionCallbackHandler.openSessionResponse(
                            ImsMediaSession.RESULT_NOT_READY);
                }
            }
        }
        else {
            ImsLog.d("Audio Session is already created: SessionId="
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
        }
    }

    private void handleAudioDisconnection() {
        if (mAudioSession != null) {
            closeSockets();
            mAudioSession = null;
            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.nofityMediaDetach();
            }
        }
    }

    private void closeSockets() {
        if (mRtpSocket != null) {
            mAudioQosAgent.destroyQosConnection(mRtpSocket.first, mRtpSocket.second);
        }
    }

    private void handleAudioModifySession(AudioConfig audioConfig) {
        if (mAudioSession != null) {
            InetSocketAddress remoteRtpAddress = audioConfig.getRemoteRtpAddress();
            if (remoteRtpAddress != null) {
                InetAddress remoteInetAddress = remoteRtpAddress.getAddress();
                int remotePort = remoteRtpAddress.getPort();
                if (remoteInetAddress != null && remotePort != 0
                        && !remoteRtpAddress.equals(mRemoteAddress)) {
                    mAudioQosAgent.updateQosConnection(mRtpSocket.first, mRtpSocket.second,
                            remoteInetAddress.getHostAddress(), remotePort);
                    mRemoteAddress = remoteRtpAddress;
                }
            }
            mAudioSession.modifySession(audioConfig);
        }
    }

    private void handleAudioAddConfig(AudioConfig audioConfig) {
        if (mAudioSession != null) {
            mAudioSession.addConfig(audioConfig);
        }
    }

    private void handleAudioDeleteConfig(AudioConfig audioConfig) {
        if (mAudioSession != null) {
            mAudioSession.deleteConfig(audioConfig);
        }
    }

    private void handleAudioConfirmConfig(AudioConfig audioConfig) {
        if (mAudioSession != null) {
            mAudioSession.confirmConfig(audioConfig);
        }
    }

    private void handleAudioSendDtmf(char dtmfDigit, int duration) {
        if (mAudioSession != null) {
            mAudioSession.sendDtmf(dtmfDigit, duration);
        }
    }

    private void handleAudioSetMediaQualityThreshold(MediaQualityThreshold threshold) {
        if (mAudioSession != null) {
            mAudioSession.setMediaQualityThreshold(threshold);
        }
    }

    private void handleAudioSendHeaderExtension(List<RtpHeaderExtension> extensions) {
        if (mAudioSession != null) {
            mAudioSession.sendHeaderExtension(extensions);
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
                return ;
            }

            mAudioSession = (ImsAudioSession) session;
            mAudioSessionId = mAudioSession.getSessionId();
            ImsLog.d("Audio Session created: SessionId=" + mAudioSessionId);
        }

        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.openSessionResponse(result);
        }
    }

    private void handleSessionClosed() {
        closeSockets();
        mAudioSession = null;
        mAudioSessionId = 0;
    }

    private void handleSessionChanged(int state) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.sessionChanged(state);
        }
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

    private void handleMediaInactivityNotification(final int packetType) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.onNotifyMediaInactivity(packetType);
        }
    }

    private void handlePacketLossNotification(final int packetLossPercentage) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.onNotifyPacketLoss(packetLossPercentage);
        }
    }

    private void handleJitterNotification(final int jitter) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.onNotifyJitter(jitter);
        }
    }

    private void handleCallQualityChanged(final CallQuality callQuality) {
        if (mAudioSessionCallbackHandler != null) {
            mAudioSessionCallbackHandler.callQualityChanged(callQuality);
        }
    }
}
