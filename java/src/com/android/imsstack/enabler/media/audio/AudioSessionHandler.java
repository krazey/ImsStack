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
import android.telephony.CallQuality;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.imsmedia.AudioConfig;
import android.telephony.imsmedia.AudioSessionCallback;
import android.telephony.imsmedia.ImsAudioSession;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.MediaQualityThreshold;

import com.android.imsstack.enabler.media.AudioSessionCallbackHandler;
import com.android.imsstack.enabler.media.MediaConstants;
import com.android.imsstack.enabler.media.MediaManagerHelper;
import com.android.imsstack.enabler.media.MediaSession;
import com.android.imsstack.enabler.media.MediaSocket;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.net.DatagramSocket;
import java.util.ArrayList;
import java.util.List;

/**
 * This manages audio session by communicating between ImsStack (media enabler native)
 * and {@link ImsMediaManager}
 */
public class AudioSessionHandler  {

    private final AudioSessionCallbackProxy mAudioSessionCallback;
    private ImsAudioSession mAudioSession;
    private int mAudioSessionId;
    private DatagramSocket mRtpSocket, mRtcpSocket;
    private final AudioSessionCallbackHandler mAudioSessionCallbackHandler;
    private final MediaManagerHelper mMediaManager;

    public AudioSessionHandler(@NonNull MediaSession mediaSession,
                                @NonNull MediaManagerHelper mediaManager) {
        mMediaManager = mediaManager;
        mAudioSessionCallbackHandler = new AudioSessionCallbackHandler(mediaSession);
        mAudioSessionCallback = new AudioSessionCallbackProxy();
        ImsLog.d("AudioSessionHandler created");
    }

    @VisibleForTesting
    public AudioSessionHandler(@NonNull MediaManagerHelper mediaManager,
        @NonNull AudioSessionCallbackHandler audioCallbackHandler,
        @NonNull ImsAudioSession audioSession) {
        mMediaManager = mediaManager;
        mAudioSessionCallbackHandler = audioCallbackHandler;
        mAudioSession = audioSession;
        mAudioSessionCallback = new AudioSessionCallbackProxy();
        ImsLog.d("AudioSessionHandler created");
    }

    @VisibleForTesting
    void setAudioSession(@Nullable ImsAudioSession audioSession) {
        mAudioSession = audioSession;
    }

    @VisibleForTesting
    AudioSessionCallback getAudioSessionCallback() {
        return mAudioSessionCallback;
    }

    @VisibleForTesting
    int getAudioSessionId() {
        return mAudioSessionId;
    }

    private ImsAudioSession getAudioSession() {
        return mAudioSession;
    }

    public void onImsMediaAudioMessage(final int requestType, Parcel parcel) {
        ImsLog.v("requestType= " + requestType);

        switch (requestType) {
            /**
             * Requests (ImsStack -> ImsMedia)
             */
            case MediaConstants.REQUEST_OPEN_SESSION:
            {
                String localIpAddress = parcel.readString();
                int localPortNumber = parcel.readInt();
                ImsLog.v("localIpAddress= " + localIpAddress
                        + " localPortNumber= " + localPortNumber);

                handleAudioOpenSession(localIpAddress, localPortNumber, null);
            }
                break;

            case MediaConstants.REQUEST_CLOSE_SESSION:
            {
                handleAudioCloseSession();
            }
                break;

            case MediaConstants.REQUEST_MODIFY_SESSION:
            {
                AudioConfig audioConfig = AudioConfig.CREATOR.createFromParcel(parcel);

                handleAudioModifySession(audioConfig);
            }
                break;

            case MediaConstants.REQUEST_ADD_CONFIG:
            {
                AudioConfig audioConfig = AudioConfig.CREATOR.createFromParcel(parcel);

                handleAudioAddConfig(audioConfig);
            }
                break;

            case MediaConstants.REQUEST_DELETE_CONFIG:
            {
                AudioConfig audioConfig = AudioConfig.CREATOR.createFromParcel(parcel);

                handleAudioDeleteConfig(audioConfig);
            }
                break;

            case MediaConstants.REQUEST_CONFIRM_CONFIG:
            {
                AudioConfig audioConfig = AudioConfig.CREATOR.createFromParcel(parcel);

                handleAudioConfirmConfig(audioConfig);
            }
                break;

            case MediaConstants.REQUEST_SEND_DTMF:
            {
                char dtmfDigit = (char)parcel.readByte();
                int duration = parcel.readInt();
                ImsLog.v("dtmfDigit=" + dtmfDigit + ", duration=" +duration);

                handleAudioSendDtmf(dtmfDigit, duration);
            }
                break;

            case MediaConstants.REQUEST_SET_MEDIA_QUALITY:
            {
                MediaQualityThreshold threshold =
                    MediaQualityThreshold.CREATOR.createFromParcel(parcel);
                ImsLog.v("onAudioSetMediaQualityThreshold: " + threshold.toString());

                handleAudioSetMediaQualityThreshold(threshold);
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

                handleAudioSendHeaderExtension(rtpExtensions);
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

    private void handleAudioOpenSession(String localIpAddress, int localPortNumber,
        AudioConfig audioConfig) {

        if(mAudioSession == null) {
            if (mMediaManager.isImsMediaConnected()) {
                //TODO: ImsQOSManager to be used
                mRtpSocket = MediaSocket.createDatagramSocket(localIpAddress,
                    localPortNumber);
                mRtcpSocket = MediaSocket.createDatagramSocket(localIpAddress,
                    (localPortNumber+1));

                if (mRtpSocket == null || mRtcpSocket == null) {
                    ImsLog.e("socket creation failed");
                    if (mAudioSessionCallbackHandler != null) {
                        mAudioSessionCallbackHandler.openSessionResponse(
                            ImsMediaSession.RESULT_PORT_UNAVAILABLE);
                    }
                    return;
                }

                mMediaManager.openSession(mRtpSocket, mRtcpSocket,
                    ImsMediaSession.SESSION_TYPE_AUDIO,
                    audioConfig, mAudioSessionCallback);
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
            closeSockets();
            mAudioSession = null;
        }
    }

    private void handleAudioDisconnection() {
        if (mAudioSession != null) {
            closeSockets();
            mAudioSession = null;
        }
    }

    private void closeSockets() {
        //TODO: ImsQOSManager to be used
        MediaSocket.closeDatagramSocket(mRtpSocket);
        mRtpSocket = null;
        MediaSocket.closeDatagramSocket(mRtcpSocket);
        mRtcpSocket = null;
    }

    private void handleAudioModifySession(AudioConfig audioConfig) {
        if (mAudioSession != null) {
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

    private class AudioSessionCallbackProxy extends AudioSessionCallback {

        @Override
        public void onOpenSessionSuccess(ImsMediaSession session) {

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

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.openSessionResponse(ImsMediaSession.RESULT_SUCCESS);
            }
        }

        @Override
        public void onOpenSessionFailure(final @ImsMediaSession.SessionOperationResult int error) {
            ImsLog.d("error=" + error);

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.openSessionResponse(error);
            }
        }

        @Override
        public void onSessionChanged(final @ImsMediaSession.SessionState int state) {
            ImsLog.d("state=" + state);

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.sessionChanged(state);
            }
        }

        @Override
        public void onModifySessionResponse(final AudioConfig audioConfig,
                final @ImsMediaSession.SessionOperationResult int result) {
            ImsLog.d("result=" + result);

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.modifySessionResponse(audioConfig, result);
            }
        }

        @Override
        public void onAddConfigResponse(final AudioConfig audioConfig,
                final @ImsMediaSession.SessionOperationResult int result) {
            ImsLog.d("result=" + result);

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.addConfigResponse(audioConfig, result);
            }
        }

        @Override
        public void onConfirmConfigResponse(final AudioConfig audioConfig,
                final @ImsMediaSession.SessionOperationResult int result) {
            ImsLog.d("result=" + result);

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.confirmConfigResponse(audioConfig, result);
            }
        }

        @Override
        public void onFirstMediaPacketReceived(final AudioConfig audioConfig) {
            ImsLog.d("FirstMediaPacketReceived for SessionId[" + getAudioSessionId() + "]");

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.firstMediaPacketReceived(audioConfig);
            }
        }

        @Override
        public void onHeaderExtensionReceived(final List<RtpHeaderExtension> extensions) {
            ImsLog.d("onHeaderExtensionReceived");

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.headerExtensionReceived(extensions);
            }
        }

        @Override
        public void notifyMediaInactivity(final @ImsMediaSession.PacketType int packetType) {
            ImsLog.d("packetType=" + packetType);

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.onNotifyMediaInactivity(packetType);
            }
        }

        @Override
        public void notifyPacketLoss(final int packetLossPercentage) {
            ImsLog.d("packetLossPercentage=" + packetLossPercentage);

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.onNotifyPacketLoss(packetLossPercentage);
            }
        }

        @Override
        public void notifyJitter(final int jitter) {
            ImsLog.d("jitter=" + jitter);

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.onNotifyJitter(jitter);
            }
        }

        @Override
        public void onMediaQualityChanged(@NonNull final CallQuality callQuality) {
            ImsLog.v("Media Quality Changed: " + callQuality.toString());

            if (mAudioSessionCallbackHandler != null) {
                mAudioSessionCallbackHandler.mediaQualityChanged(callQuality);
            }
        }
    }
}
