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

/**
 * This manages text session by communicating between ImsStack and {@link ImsMediaManager}
 */
public class TextSessionHandler extends MediaState {

    static final int UNUSED = -1;

    private final TextSessionCallbackProxy mTextSessionCallback;
    private ImsTextSession mTextSession;
    private int mTextSessionId;
    private Pair<DatagramSocket, DatagramSocket> mRtpSocket;
    private final TextSessionCallbackHandler mTextSessionCallbackHandler;
    private final MediaManagerHelper mMediaManager;
    private final TextMessageHandler mTextMessageHandler;
    private final IBaseContext mContext;
    private QosAgent mTextQosAgent;
    private TextImsQosCallback mTextImsQosCallback;
    private final Object mLock = new Object();

    public TextSessionHandler(IBaseContext context,
            @NonNull MediaManagerHelper mediaManager, IMtcMediaInterface mtcMediaInterface) {
        super(ImsMediaSession.SESSION_TYPE_RTT);
        mContext = context;
        mMediaManager = mediaManager;
        mTextSessionCallbackHandler = new TextSessionCallbackHandler(mtcMediaInterface);
        mTextSessionCallback = new TextSessionCallbackProxy();
        mTextMessageHandler = new TextMessageHandler(mMediaManager.getMediaLooper());
        createQosAgent(mContext.getSlotId());
        ImsLog.d("TextSessionHandler created");
    }

    @VisibleForTesting
    public TextSessionHandler(IBaseContext context, @NonNull MediaManagerHelper mediaManager,
            @NonNull TextSessionCallbackHandler textCallbackHandler,
            @NonNull ImsTextSession textSession, Looper looper) {
        super(ImsMediaSession.SESSION_TYPE_RTT);
        mContext = context;
        mMediaManager = mediaManager;
        mTextSessionCallbackHandler = textCallbackHandler;
        mTextSession = textSession;
        mTextSessionCallback = new TextSessionCallbackProxy();
        mTextMessageHandler = new TextMessageHandler(looper);
        ImsLog.d("TextSessionHandler created");
    }

    @VisibleForTesting
    void setTextSession(@Nullable ImsTextSession textSession) {
        mTextSession = textSession;
    }

    @VisibleForTesting
    void setTextQosAgent(@Nullable QosAgent textQosAgent) {
        mTextQosAgent = textQosAgent;
    }

    @VisibleForTesting
    void setRtpSocket(@Nullable Pair<DatagramSocket, DatagramSocket> rtpSocket) {
        mRtpSocket = rtpSocket;
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

    private boolean isWaitRequired(int requestType) {
        return (requestType != MediaConstants.REQUEST_OPEN_SESSION
                && requestType != MediaConstants.RESPONSE_OPEN_SESSION
                && requestType != MediaConstants.REQUEST_QOS
                && requestType != MediaConstants.NOTIFY_MEDIA_DETACH);
    }

    /** Text session message Handler */
    class TextMessageHandler extends Handler {

        TextMessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            ImsLog.d("messageType = " + msg.what);

            // Till open session response is received, handling other commands has to wait
            try {
                synchronized (mLock) {
                    if (mTextSession == null && isWaitRequired(msg.what)) {
                        ImsLog.d(Thread.currentThread().getName()
                                + " is waiting for Text openSession response");
                        mLock.wait(MediaConstants.RESPONSE_WAIT_TIMEOUT);
                        if (mTextSession == null) {
                            ImsLog.e("Text openSession response timeout");
                            handleOpenSessionResponse(null, ImsMediaSession.RESULT_NOT_READY);
                            return;
                        }
                        ImsLog.d(Thread.currentThread().getName()
                                + " received Text openSession response");
                    }
                }
            } catch (InterruptedException ie) {
                ImsLog.e("unexpectedly interrupted while waiting" + ie.getMessage());
            }

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

                case MediaConstants.REQUEST_QOS:
                {
                    handleTextQos((String) msg.obj, msg.arg1);
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

                case MediaConstants.RESPONSE_SESSION_CLOSED:
                case MediaConstants.NOTIFY_MEDIA_DETACH:
                {
                    handleTextSessionClosed();
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
            // Release the wait as OpenSession Response is received
            synchronized (mLock) {
                mTextSession = (ImsTextSession) session;
                mLock.notifyAll();
            }
            Message.obtain(mTextMessageHandler, MediaConstants.RESPONSE_OPEN_SESSION,
                    ImsMediaSession.RESULT_SUCCESS, UNUSED, session).sendToTarget();
        }

        @Override
        public void onOpenSessionFailure(final @ImsMediaSession.SessionOperationResult int error) {
            ImsLog.d("error=" + error);
            // Release the wait as OpenSession Response is received
            synchronized (mLock) {
                mLock.notifyAll();
            }
            Message.obtain(mTextMessageHandler, MediaConstants.RESPONSE_OPEN_SESSION,
                    error, UNUSED, null).sendToTarget();
        }

        @Override
        public void onSessionClosed() {
            ImsLog.d("onSessionClosed for SessionId[" + getTextSessionId() + "]");

            Message.obtain(mTextMessageHandler, MediaConstants.RESPONSE_SESSION_CLOSED)
                    .sendToTarget();
        }

        @Override
        public void onModifySessionResponse(final TextConfig textConfig,
                final @ImsMediaSession.SessionOperationResult int result) {
            ImsLog.d("result=" + result);

            Message.obtain(mTextMessageHandler, MediaConstants.RESPONSE_MODIFY_SESSION,
                    result, UNUSED, textConfig).sendToTarget();
        }

        @Override
        public void onRttReceived(final String rttMessage) {
            ImsLog.d("onRttReceived for SessionId[" + getTextSessionId() + "]");

            Message.obtain(mTextMessageHandler, MediaConstants.NOTIFY_RTT_RECEIVED,
                rttMessage).sendToTarget();
        }

        @Override
        public void notifyMediaInactivity(final @ImsMediaSession.PacketType int packetType) {
            ImsLog.d("packetType=" + packetType);

            Message.obtain(mTextMessageHandler, MediaConstants.NOTIFY_MEDIA_INACTIVITY,
                    packetType, UNUSED).sendToTarget();
        }
    }

    /** Implements Interface to receive callbacks when the QoS is connected or disconnected. */
    private class TextImsQosCallback implements ImsQosCallback {

        @Override
        public void onNotifyQosConnectionAvailable(InetSocketAddress remoteAddress) {
            ImsLog.i("ImsQosCallback - connected");

            if (mTextSessionCallbackHandler != null) {
                mTextSessionCallbackHandler.onNotifyQosInfo(
                        remoteAddress.getAddress().getHostAddress(),
                        remoteAddress.getPort(), true);
            }
        }

        @Override
        public void onNotifyQosConnectionLost(InetSocketAddress remoteAddress) {
            ImsLog.i("ImsQosCallback - disconnected");

            if (mTextSessionCallbackHandler != null) {
                mTextSessionCallbackHandler.onNotifyQosInfo(
                        remoteAddress.getAddress().getHostAddress(),
                        remoteAddress.getPort(), false);
            }
        }
    }

    /**
     * Handles Text Session requests received from Media Native
     *
     * @param requestType type of the request
     * @param parcel parcel received from Media Native
     */
    public void onImsMediaTextMessage(final int requestType, Parcel parcel) {
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
                        mTextMessageHandler, requestType, localPortNumber, UNUSED, localIpAddress)
                        .sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_CLOSE_SESSION:
            case MediaConstants.NOTIFY_MEDIA_DETACH:
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

            case MediaConstants.REQUEST_QOS:
            {
                String remoteIpAddress = parcel.readString();
                int remotePortNumber = parcel.readInt();
                ImsLog.d("remoteIpAddress= " + remoteIpAddress
                        + " remotePortNumber= " + remotePortNumber);

                Message.obtain(
                        mTextMessageHandler, requestType, remotePortNumber, UNUSED,
                        remoteIpAddress).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_SEND_RTT:
            {
                String rttMessage = parcel.readString();
                ImsLog.d("rtt message");

                Message.obtain(mTextMessageHandler, requestType, rttMessage).sendToTarget();
            }
                break;

            case MediaConstants.REQUEST_SET_MEDIA_QUALITY:
            {
                MediaQualityThreshold threshold =
                        MediaQualityThreshold.CREATOR.createFromParcel(parcel);
                ImsLog.d("onTextSetMediaQualityThreshold: " + threshold.toString());

                Message.obtain(mTextMessageHandler, requestType, threshold).sendToTarget();
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
        if (mTextQosAgent == null) {
            mTextQosAgent = new QosAgent(slotId);
        }
        if (mTextImsQosCallback == null) {
            mTextImsQosCallback = new TextImsQosCallback();
        }
        mTextQosAgent.setCallback(mTextImsQosCallback);
    }

    private void handleTextOpenSession(String localIpAddress, int localPortNumber) {

        if (mTextSession == null) {
            if (mMediaManager.isImsMediaConnected()) {

                mRtpSocket = mTextQosAgent.createQosConnection(localIpAddress, localPortNumber);

                if (mRtpSocket == null) {
                    ImsLog.e("rtp socket creation failed");
                    if (mTextSessionCallbackHandler != null) {
                        mTextSessionCallbackHandler.openSessionResponse(
                                ImsMediaSession.RESULT_PORT_UNAVAILABLE);
                    }
                    setMediaState(MEDIA_STATE_IDLE);
                    return;
                } else if (mRtpSocket.first == null || mRtpSocket.second == null) {
                    ImsLog.e("rtp socket creation failed");
                    closeSockets();
                    if (mTextSessionCallbackHandler != null) {
                        mTextSessionCallbackHandler.openSessionResponse(
                                ImsMediaSession.RESULT_PORT_UNAVAILABLE);
                    }
                    setMediaState(MEDIA_STATE_IDLE);
                    return;
                }

                mMediaManager.openSession(mRtpSocket.first, mRtpSocket.second,
                        ImsMediaSession.SESSION_TYPE_RTT, null, mTextSessionCallback);
                setMediaState(MEDIA_STATE_OPENING);
            } else {
                ImsLog.d("ImsMediaManager is not ready");
                if (mTextSessionCallbackHandler != null) {
                    mTextSessionCallbackHandler.openSessionResponse(
                            ImsMediaSession.RESULT_NOT_READY);
                }
                setMediaState(MEDIA_STATE_IDLE);
            }
        } else {
            ImsLog.w("Text Session is already created: SessionId=" + mTextSession.getSessionId());
            if (mTextSessionCallbackHandler != null) {
                mTextSessionCallbackHandler.openSessionResponse(
                        ImsMediaSession.RESULT_NOT_SUPPORTED);
            }
        }
    }

    private void handleTextCloseSession() {
        if (mTextSession != null) {
            mMediaManager.closeSession(mTextSession);
            setMediaState(MEDIA_STATE_CLOSED);
        }
    }

    private void closeSockets() {
        if (mRtpSocket != null) {
            mTextQosAgent.destroyQosConnection(mRtpSocket.first, mRtpSocket.second);
        }
    }

    private void handleTextModifySession(TextConfig textConfig) {
        if (mTextSession != null) {
            mTextSession.modifySession(textConfig);
        } else {
            handleModifySessionResponse(textConfig, ImsMediaSession.RESULT_NOT_READY);
        }
    }

    private void handleTextQos(String remoteIpAddress, int remotePortNumber)  {
        if (remoteIpAddress != null && remotePortNumber != 0
                && isNewRemoteAddress(remoteIpAddress, remotePortNumber)) {
            mTextQosAgent.updateQosConnection(mRtpSocket.first, mRtpSocket.second,
                    remoteIpAddress, remotePortNumber, false);
            ImsLog.d("Updated QoS Connection for remoteIpAddress= " + remoteIpAddress
                    + " remotePortNumber= " + remotePortNumber);
        }
    }

    private void handleSendRtt(String rttMessage) {
        if (mTextSession != null) {
            mTextSession.sendRtt(rttMessage);
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
                setMediaState(MEDIA_STATE_IDLE);
                return;
            }

            mTextSessionId = mTextSession.getSessionId();
            setMediaState(MEDIA_STATE_LIVE);
            ImsLog.d("Text Session created: SessionId=" + mTextSessionId);
        } else {
            setMediaState(MEDIA_STATE_IDLE);
        }

        if (mTextSessionCallbackHandler != null) {
            mTextSessionCallbackHandler.openSessionResponse(result);
        }
    }

    private void handleTextSessionClosed() {
        closeSockets();
        setMediaState(MEDIA_STATE_IDLE);
        mTextSession = null;
        mTextSessionId = 0;
        mTextMessageHandler.removeCallbacksAndMessages(null);
    }

    private void handleModifySessionResponse(final TextConfig textConfig, final int result) {
        if (mTextSessionCallbackHandler != null && mTextSession != null) {
            mTextSessionCallbackHandler.modifySessionResponse(textConfig, result);
        }
    }

    private void handleRttReceived(final String rttMessage) {
        if (mTextSessionCallbackHandler != null && mTextSession != null) {
            mTextSessionCallbackHandler.onRttReceived(rttMessage);
        }
    }

    private void handleMediaInactivityNotification(final int packetType) {
        if (mTextSessionCallbackHandler != null && mTextSession != null) {
            mTextSessionCallbackHandler.onNotifyMediaInactivity(packetType);
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
