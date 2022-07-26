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
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.ImsTextSession;
import android.telephony.imsmedia.MediaQualityThreshold;
import android.telephony.imsmedia.TextConfig;
import android.telephony.imsmedia.TextSessionCallback;

import com.android.imsstack.enabler.mtc.IMtcMediaInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.net.DatagramSocket;

/**
 * This manages text session by communicating between ImsStack (media enabler native)
 * and {@link ImsMediaManager}
 */
public class TextSessionHandler  {

    static final int UNUSED = -1;

    private final TextSessionCallbackProxy mTextSessionCallback;
    private ImsTextSession mTextSession;
    private int mTextSessionId;
    private DatagramSocket mRtpSocket, mRtcpSocket;
    private final TextSessionCallbackHandler mTextSessionCallbackHandler;
    private final MediaManagerHelper mMediaManager;
    private final TextMessageHandler mTextMessageHandler;

    public TextSessionHandler(
            @NonNull MediaManagerHelper mediaManager, IMtcMediaInterface mtcMediaInterface) {
        mMediaManager = mediaManager;
        mTextSessionCallbackHandler = new TextSessionCallbackHandler(mtcMediaInterface);
        mTextSessionCallback = new TextSessionCallbackProxy();
        mTextMessageHandler = new TextMessageHandler(mMediaManager.getMediaLooper());
        ImsLog.d("TextSessionHandler created");
    }

    @VisibleForTesting
    public TextSessionHandler(@NonNull MediaManagerHelper mediaManager,
            @NonNull TextSessionCallbackHandler textCallbackHandler,
            @NonNull ImsTextSession textSession) {
        mMediaManager = mediaManager;
        mTextSessionCallbackHandler = textCallbackHandler;
        mTextSession = textSession;
        mTextSessionCallback = new TextSessionCallbackProxy();
        mTextMessageHandler = new TextMessageHandler(Looper.getMainLooper());
        ImsLog.d("TextSessionHandler created");
    }

    @VisibleForTesting
    void setTextSession(@Nullable ImsTextSession textSession) {
        mTextSession = textSession;
    }

    @VisibleForTesting
    TextSessionCallback getTextSessionCallback() {
        return mTextSessionCallback;
    }

    @VisibleForTesting
    int getTextSessionId() {
        return mTextSessionId;
    }

    @VisibleForTesting
    TextMessageHandler getTextMessageHandler() {
        return mTextMessageHandler;
    }

    /** Text session message Handler */
    class TextMessageHandler extends Handler {

        TextMessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            ImsLog.v("msg.what= " + msg.what);
            switch (msg.what) {
                case MediaConstants.REQUEST_OPEN_SESSION:
                {
                    handleTextOpenSession((String) msg.obj, msg.arg1);
                }
                    break;

                case MediaConstants.REQUEST_CLOSE_SESSION:
                {
                    handleTextCloseSession();
                }
                    break;

                case MediaConstants.REQUEST_MODIFY_SESSION:
                {
                    handleTextModifySession((TextConfig) msg.obj);
                }
                    break;

                case MediaConstants.REQUEST_SEND_RTT:
                {
                    handleSendRtt((String) msg.obj);
                }
                    break;

                case MediaConstants.REQUEST_SET_MEDIA_QUALITY:
                {
                    handleTextSetMediaQualityThreshold((MediaQualityThreshold) msg.obj);
                }
                    break;

                case MediaConstants.RESPONSE_OPEN_SESSION:
                {
                    handleOpenSessionResponse((ImsMediaSession) msg.obj, msg.arg1);
                }
                    break;

                case MediaConstants.RESPONSE_SESSION_CHANGED:
                {
                    handleSessionChanged(msg.arg1);
                }
                    break;

                case MediaConstants.RESPONSE_MODIFY_SESSION:
                {
                    handleModifySessionResponse((TextConfig) msg.obj, msg.arg1);
                }
                    break;

                case MediaConstants.NOTIFY_RTT_RECEIVED:
                {
                    handleRttReceived((String) msg.obj);
                }
                    break;

                case MediaConstants.NOTIFY_MEDIA_INACTIVITY:
                {
                    handleMediaInactivityNotification(msg.arg1);
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

    private class TextSessionCallbackProxy extends TextSessionCallback {

        @Override
        public void onOpenSessionSuccess(ImsMediaSession session) {
            Message.obtain(mTextMessageHandler, MediaConstants.RESPONSE_OPEN_SESSION,
                    ImsMediaSession.RESULT_SUCCESS, UNUSED, session).sendToTarget();
        }

        @Override
        public void onOpenSessionFailure(final @ImsMediaSession.SessionOperationResult int error) {
            ImsLog.d("error=" + error);

            Message.obtain(mTextMessageHandler, MediaConstants.RESPONSE_OPEN_SESSION,
                    error, UNUSED, null).sendToTarget();
        }

        @Override
        public void onSessionChanged(final @ImsMediaSession.SessionState int state) {
            ImsLog.d("state=" + state);

            Message.obtain(mTextMessageHandler, MediaConstants.RESPONSE_SESSION_CHANGED,
                    state, UNUSED).sendToTarget();
        }

        @Override
        public void onModifySessionResponse(final TextConfig textConfig,
                final @ImsMediaSession.SessionOperationResult int result) {
            ImsLog.d("result=" + result);

            Message.obtain(mTextMessageHandler, MediaConstants.RESPONSE_MODIFY_SESSION,
                    result, UNUSED, textConfig).sendToTarget();
        }

        @Override
        public void onRttReceived(final String text) {
            ImsLog.d("onRttReceived for SessionId[" + getTextSessionId() + "]");

            Message.obtain(mTextMessageHandler, MediaConstants.NOTIFY_RTT_RECEIVED,
                    text).sendToTarget();
        }

        @Override
        public void notifyMediaInactivity(final @ImsMediaSession.PacketType int packetType) {
            ImsLog.d("packetType=" + packetType);

            Message.obtain(mTextMessageHandler, MediaConstants.NOTIFY_MEDIA_INACTIVITY,
                    packetType, UNUSED).sendToTarget();
        }
    }

    /**
     * Handles Text Session requests received from Media Native
     *
     * @param requestType type of the request
     * @param parcel parcel received from Media Native
     */
    public void onImsMediaTextMessage(final int requestType, Parcel parcel) {
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
                        mTextMessageHandler, requestType, localPortNumber, UNUSED, localIpAddress)
                        .sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_CLOSE_SESSION:
            {
                Message.obtain(mTextMessageHandler, requestType).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_MODIFY_SESSION:
            {
                TextConfig textConfig = TextConfig.CREATOR.createFromParcel(parcel);
                Message.obtain(mTextMessageHandler, requestType, textConfig).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_SEND_RTT:
            {
                String text = parcel.readString();
                ImsLog.v("rtt message = " + text);

                Message.obtain(mTextMessageHandler, requestType, text).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_SET_MEDIA_QUALITY:
            {
                MediaQualityThreshold threshold =
                        MediaQualityThreshold.CREATOR.createFromParcel(parcel);
                ImsLog.v("onTextSetMediaQualityThreshold: " + threshold.toString());

                Message.obtain(mTextMessageHandler, requestType, threshold).sendToTarget();
            }
                break;

            default:
            {
                ImsLog.e("Invalid RequestType");
            }
                break;
        }
    }

    private void handleTextOpenSession(String localIpAddress, int localPortNumber) {

        if (mTextSession == null) {
            if (mMediaManager.isImsMediaConnected()) {
                // TODO_MEDIA : ImsQOSManager to be used
                mRtpSocket = MediaSocket.createDatagramSocket(localIpAddress,
                    localPortNumber);
                mRtcpSocket = MediaSocket.createDatagramSocket(localIpAddress,
                    (localPortNumber + 1));

                if (mRtpSocket == null || mRtcpSocket == null) {
                    ImsLog.e("socket creation failed");
                    if (mTextSessionCallbackHandler != null) {
                        mTextSessionCallbackHandler.openSessionResponse(
                                ImsMediaSession.RESULT_PORT_UNAVAILABLE);
                    }
                    return;
                }

                mMediaManager.openSession(mRtpSocket, mRtcpSocket,
                        ImsMediaSession.SESSION_TYPE_RTT, null, mTextSessionCallback);
            } else {
                ImsLog.d("ImsMediaManager is not ready");
                if (mTextSessionCallbackHandler != null) {
                    mTextSessionCallbackHandler.openSessionResponse(
                            ImsMediaSession.RESULT_NOT_READY);
                }
            }
        } else {
            ImsLog.d("Text Session is already created: SessionId="
                    + mTextSession.getSessionId());
            if (mTextSessionCallbackHandler != null) {
                mTextSessionCallbackHandler.openSessionResponse(
                        ImsMediaSession.RESULT_NOT_SUPPORTED);
            }
        }
    }

    private void handleTextCloseSession() {
        if (mTextSession != null) {
            mMediaManager.closeSession(mTextSession);
            closeSockets();
            mTextSession = null;
        }
    }

    private void closeSockets() {
        // TODO_MEDIA: ImsQOSManager to be used
        MediaSocket.closeDatagramSocket(mRtpSocket);
        mRtpSocket = null;
        MediaSocket.closeDatagramSocket(mRtcpSocket);
        mRtcpSocket = null;
    }

    private void handleTextModifySession(TextConfig textConfig) {
        if (mTextSession != null) {
            mTextSession.modifySession(textConfig);
        }
    }

    private void handleSendRtt(String text) {
        if (mTextSession != null) {
            mTextSession.sendRtt(text);
        }
    }

    private void handleTextSetMediaQualityThreshold(MediaQualityThreshold threshold) {
        if (mTextSession != null) {
            mTextSession.setMediaQualityThreshold(threshold);
        }
    }

    private void handleOpenSessionResponse(ImsMediaSession session, int result) {
        if (result == ImsMediaSession.RESULT_SUCCESS) {
            if (session == null) {
                ImsLog.e("Text Session is not created");
                if (mTextSessionCallbackHandler != null) {
                    mTextSessionCallbackHandler.openSessionResponse(
                            ImsMediaSession.RESULT_NO_MEMORY);
                }
                return;
            }

            mTextSession = (ImsTextSession) session;
            mTextSessionId = mTextSession.getSessionId();
            ImsLog.d("Text Session created: SessionId=" + mTextSessionId);
        }

        if (mTextSessionCallbackHandler != null) {
            mTextSessionCallbackHandler.openSessionResponse(result);
        }
    }

    private void handleSessionChanged(int state) {
        if (mTextSessionCallbackHandler != null) {
            mTextSessionCallbackHandler.sessionChanged(state);
        }
    }

    private void handleModifySessionResponse(final TextConfig textConfig, final int result) {
        if (mTextSessionCallbackHandler != null) {
            mTextSessionCallbackHandler.modifySessionResponse(textConfig, result);
        }
    }

    private void handleRttReceived(final String text) {
        if (mTextSessionCallbackHandler != null) {
            mTextSessionCallbackHandler.onRttReceived(text);
        }
    }

    private void handleMediaInactivityNotification(final int packetType) {
        if (mTextSessionCallbackHandler != null) {
            mTextSessionCallbackHandler.onNotifyMediaInactivity(packetType);
        }
    }
}
