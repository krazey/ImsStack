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

package com.android.imsstack.enabler.mtc;

import android.os.Parcel;
import android.telephony.CallQuality;
import android.telephony.ims.RtpHeaderExtension;
import android.view.Surface;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.media.IMediaListener;
import com.android.imsstack.enabler.media.MediaFactory;
import com.android.imsstack.enabler.media.MediaSession;
import com.android.imsstack.util.ImsLog;

import java.util.Iterator;
import java.util.Set;

/**
 * class to handle calls to or from ImsMedia, ImsService, videoprovider
 * and invoke jni methods to libimsstack
 */
public class MtcMediaSession implements IMtcMediaVideoCallProvider, IMtcMediaInterface {

    /**
     * Listener to send callback message from audio session to call
     */
    public static class AudioListener {
        /**
         * Called when the audio session opened
         */
        public void onAudioSessionOpened() {
            // no-op
        }

        /**
         * Called when the audio session close
         */
        public void onAudioSessionClosed() {
            // no-op
        }

        /**
         * Called when the audio call quality changed message received
         */
        public void onCallQualityChanged(CallQuality callQuality) {
            // no-op
        }

        /**
         * Called when the RTP header extension data received
         */
        public void onRtpHeaderExtensionsReceived(Set<RtpHeaderExtension> extensions) {
            // no-op
        }
    }

    /**
     * Listener to send callback message from video session to video provider
     */
    public static class VideoListener {
        public void onMediaSessionDataUsageChanged(MtcMediaSession session,
                long dataSize) {
            // no-op
        }

        /**
         * Called when the media information is changed in the media session.
         */
        public void onMediaSessionMediaInfoChanged(MtcMediaSession session,
                int mediaInfo, int intParam, String strParam) {
            // no-op
        }

        /**
         * Called when the first video packet is received from the remote endpoint.
         */
        public void onMediaSessionPeerFirstVideoReceived(MtcMediaSession session) {
            // no-op
        }

        /**
         * Called when the peer dimension is changed
         */
        public void onMediaSessionPeerDimensionsChanged(MtcMediaSession session,
                final int width, final int height) {
            // no-op
        }
    }

    /**
     * Listener to send callback message from text session to call
     */
    public static class TextListener {
        /**
         * Called when Rx RTT text is received from peer.
         */
        public void onRttMessageReceived(MtcMediaSession session, String data) {
            // no-op
        }

        /**
         * Called when Rx audio receiving is started/stopped while on-going RTT session
         */
        public void onRttAudioIndication(MtcMediaSession session, boolean status) {
            // no-op
        }
    }

    public static class MediaInfoEvent {
        private int mType = 0;
        private int mIntParam = 0;
        private String mStringParam = "";

        public MediaInfoEvent(int type, int intParam, String stringParam) {
            mType = type;
            mIntParam = intParam;
            mStringParam = stringParam;
        }

        @Override
        public String toString() {
            return "[ MediaInfoEvent: type=" + mType
                    + ", intP=" + mIntParam
                    + ", stringP=" + mStringParam + " ]";
        }

        public void dispose() {
            mType = 0;
            mIntParam = 0;
            mStringParam = "";
        }

        public int getType() {
            return mType;
        }

        public int getIntParam() {
            return mIntParam;
        }

        public String getStringParam() {
            return mStringParam;
        }

        public boolean isValidEvent() {
            return mType != 0;
        }

        public void setParams(int intParam, String stringParam) {
            mIntParam = intParam;
            mStringParam = stringParam;
        }
    }

    /**
     * This surface variables will be accessed by the native layer.
     * It just grabs the object to be enable to access the surface.
     */
    private static Surface mPreviewSurface = null;
    private static Surface mDisplaySurface = null;

    private final Object mLock = new Object();
    private Call mCall;
    private MtcMediaSession.AudioListener mAudioListener = null;
    private MtcMediaSession.VideoListener mVideoListener = null;
    private MtcMediaSession.TextListener mTextListener = null;
    private IMediaListener mMediaListener = null;
    private MediaInfoEvent mMediaInfoEvent = null;
    public int mPrevOrientation = 0;
    private MediaSession mMediaSession = null;
    private boolean mIsVideoSessionOpened = false;
    private boolean mIsPendingSelectCamera = false;
    private int mCamera = -1;

    public MtcMediaSession(IBaseContext context, Call call) {
        mCall = call;
        mMediaSession = MediaFactory.createMediaSession(context, this);
        mIsVideoSessionOpened = false;
        mIsPendingSelectCamera = false;
    }

    public void dispose() {
        if (!isCallValid() || !isMediaSessionValid()) {
            return;
        }

        synchronized (mLock) {
            mCall = null;
            mMediaSession = null;
            mAudioListener = null;
            mVideoListener = null;
            mTextListener = null;
            mMediaListener = null;
            mMediaInfoEvent = null;
        }
    }

    /**
     * Set the IMediaListener to send callback
     */
    public void setMediaListener(IMediaListener listener) {
        synchronized (mLock) {
            mMediaListener = listener;
        }
    }

    /**
     * Set the AudioListener to send callback from audio session
     */
    public void setAudioListener(MtcMediaSession.AudioListener listener) {
        synchronized (mLock) {
            log("setAudioListener :: listener=" + listener);
            mAudioListener = listener;
        }
    }

    /**
     * Set the VideoListener to send callback from video session
     */
    public void setVideoListener(MtcMediaSession.VideoListener listener) {
        synchronized (mLock) {
            log("setVideoListener :: listener=" + listener);
            mVideoListener = listener;
        }
    }

    /**
     * Set the TextListener to send callback from text session
     */
    public void setTextListener(MtcMediaSession.TextListener listener) {
        synchronized (mLock) {
            log("setTextListener :: listener=" + listener);
            mTextListener = listener;
        }
    }

    /**
     * Gets the call-id to identify the current call media.
     */
    public long getCallId() {
        return isCallValid() ? mCall.getNativeCallId() : 0;
    }

    /**
     * Sends dtmf to media session
     * @param code a character of dtmf code
     */
    public void sendDtmf(char code) {
        log("sendDtmf :: code=" + code);
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcMedia.SEND_DTMF);
        parcel.writeByte((byte) code);
        sendRequest(parcel);
    }

    public void setPreviewSurface(Surface surface) {
        log("setPreviewSurface :: surface=" + surface);

        Parcel parcel = Parcel.obtain();

        mPreviewSurface = surface;

        /**
         * The argument will be passed the following orders:
         * surface-type
         */
        parcel.writeInt(IUMtcMedia.SETSURFACE_CMD);
        parcel.writeInt(IUMtcMedia.ParamValue.SURFACE_NEAR);

        if (surface != null) {
            sendRequest(parcel);
        }
    }

    public void setDisplaySurface(Surface surface) {
        log("setDisplaySurface :: surface=" + surface);

        Parcel parcel = Parcel.obtain();

        mDisplaySurface = surface;

        /**
         * The argument will be passed the following orders:
         * surface-type
         */
        parcel.writeInt(IUMtcMedia.SETSURFACE_CMD);
        parcel.writeInt(IUMtcMedia.ParamValue.SURFACE_FAR);

        if (surface != null) {
            sendRequest(parcel);
        }
    }

    public Surface getPreviewSurface() {
        return mPreviewSurface;
    }

    public Surface getDisplaySurface() {
        return mDisplaySurface;
    }

    /**
     * Notifies when the received video frame resolution is different with the current resolution.
     * @param width width of resolution changed.
     * @param height height of resolution changed.
     */
    public void peerDimensionChanged(final int width, final int height) {
        log("peerDimensionChanged width[" + width + "] height[" + height + "]");

        if (mVideoListener != null) {
            mVideoListener.onMediaSessionPeerDimensionsChanged(this, width, height);
        }
    }

    /**
     * Notifies when the remote party has sent text message via RTT
     * @param rttMessage String containing the received characters.
     */
    public void rttMessageReceived(final String rttMessage) {
        log("rttMessageReceived");
        if (mTextListener != null) {
            mTextListener.onRttMessageReceived(this, rttMessage);
        }
    }

    @Override
    public void audioSessionOpened() {
        log("audioSessionOpened");
        if (mAudioListener != null) {
            mAudioListener.onAudioSessionOpened();
        }
    }

    @Override
    public void audioSessionClosed() {
        log("audioSessionClosed");
        if (mAudioListener != null) {
            mAudioListener.onAudioSessionClosed();
        }
    }

    @Override
    public void callQualityChanged(CallQuality callQuality) {
        log("callQualityChanged");
        if (mAudioListener != null) {
            mAudioListener.onCallQualityChanged(callQuality);
        }
    }

    /**
     * Notifies when the remote party has sent RTP header extension data
     * @param extensions the RTP header extension data
     */
    public void rtpHeaderExtensionsReceived(Set<RtpHeaderExtension> extensions) {
        log("rtpHeaderExtensionsReceived");
        if (mAudioListener != null) {
            mAudioListener.onRtpHeaderExtensionsReceived(extensions);
        }
    }

    @Override
    public void videoSessionOpened() {
        log("videoSessionOpened");
        mIsVideoSessionOpened = true;
        if (mIsPendingSelectCamera) {
            Parcel parcel = Parcel.obtain();
            parcel.writeInt(IUMtcMedia.SELECT_CAMERA_CMD);
            parcel.writeInt(mCamera);
            sendRequest(parcel);
            mIsPendingSelectCamera = false;
        }
    }

    // TODO MEDIA : RTT audio indicator to be implemented

    /**
     * CAMERA
     * Selects the camera type for video call.
     *
     * @param camera the camera type
     *               {@link ImsCamera#CAMERA_REAR}
     *               {@link ImsCamera#CAMERA_FRONT}
     */
    public void selectCamera(int camera) {
        log("selectCamera :: camera=" + camera);

        if (!mIsVideoSessionOpened) {
            mCamera = camera;
            mIsPendingSelectCamera = true;
            return;
        }

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcMedia.SELECT_CAMERA_CMD);
        parcel.writeInt(camera);
        sendRequest(parcel);
    }

    /**
     * CAMERA
     * Sets the zoom property of the camera.
     *
     * @param zoom the zoom (0 ~ 10)
     */
    public void setCameraZoom(int zoom) {
        log("setCameraZoom :: zoom=" + zoom);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.CHANGE_CAMERA_ZOOM_CMD);
        parcel.writeInt(zoom);

        sendRequest(parcel);
    }

    /**
     * DISPLAY
     * Sets the orientation of the device.
     *
     * @param orientation the orientation of degree observed 0, 90, 180, 270
     */
    public void setDeviceOrientation(int orientation) {
        log("setDeviceOrientation :: orientation=" + orientation);
        mPrevOrientation = orientation;
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcMedia.CHANGE_ORIENTATION_CMD);
        parcel.writeInt(orientation);
        sendRequest(parcel);
    }

    /**
     * Releases the audio resources.
     */
    public void requestCallDataUsage() {
        log("requestCallDataUsage");
        // TODO: add implementation
    }

    public void notifyMediaInfoChanged(int mediaInfo, int intParam, String strParam) {
        VideoListener listener = null;

        synchronized (mLock) {
            listener = mVideoListener;
        }

        if (listener != null) {
            listener.onMediaSessionMediaInfoChanged(this, mediaInfo, intParam, strParam);
        }

        synchronized (mLock) {
            if ((mCall != null) && mCall.isConference()
                    && (mediaInfo == MtcCallUtils.INFO_TYPE_MEDIA_CVO_CAPABILITY)) {
                if (mMediaInfoEvent == null) {
                    log("CVO-Cap :: cache media-info event");
                    mMediaInfoEvent = new MediaInfoEvent(mediaInfo, intParam, strParam);
                } else if (mMediaInfoEvent.isValidEvent()) {
                    log("CVO-Cap :: cache media-info event parameters");
                    mMediaInfoEvent.setParams(intParam, strParam);
                }
            }
        }
    }

    public static boolean isMessageForMediaSession(int msg) {
        return ((msg > IUMtcMedia.IMS_MSG_BASE_MEDIA)
                && (msg < IUMtcMedia.IMS_MSG_MAX_MEDIA));
    }

    /**
     * Called when Message for Media session is received
     */
    public void onMessage(Parcel parcel) {

        IMediaListener listener = null;

        synchronized (mLock) {
            listener = mMediaListener;
        }

        if (listener == null) {
            log("No listener: " + this);
            return;
        }

        listener.onMediaMessage(parcel);
    }

    private boolean isCallValid() {
        synchronized (mLock) {
            return mCall != null;
        }
    }

    private boolean isMediaSessionValid() {
        synchronized (mLock) {
            return mMediaSession != null;
        }
    }

    /**
     * Sends Rtt message to Media session
     *
     * @param rttMessage String containing the characters to be sent.
     */
    public void sendRttMessage(final String rttMessage) {
        log("sendRttMessage");
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcMedia.SEND_RTT_CMD);
        parcel.writeInt(IUMtcMedia.SESSION_TYPE_RTT);
        parcel.writeString(rttMessage);
        parcel.setDataPosition(0);

        onMessage(parcel);
    }

    /**
     * Sends the Rtp header extensions to Media session
     *
     * @param rtpHeaderExtensions The header extensions to be included in the next RTP header.
     */
    public void sendRtpHeaderExtensions(Set<RtpHeaderExtension> rtpHeaderExtensions) {
        log("sendRtpHeaderExtensions");
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcMedia.SEND_HEADER_EXTENSION);
        parcel.writeInt(IUMtcMedia.SESSION_TYPE_AUDIO);
        parcel.writeInt(rtpHeaderExtensions.size());

        Iterator<RtpHeaderExtension> extensions = rtpHeaderExtensions.iterator();
        while (extensions.hasNext()) {
            RtpHeaderExtension rtpExt = extensions.next();
            rtpExt.writeToParcel(parcel, 1);
        }
        parcel.setDataPosition(0);

        onMessage(parcel);
    }

    public void sendRequest(Parcel parcel) {
        long sessionId = 0;

        synchronized (mLock) {
            sessionId = getCallId();
        }

        if (sessionId == 0) {
            parcel.recycle();
            parcel = null;

            ImsLog.e("[GII-MTC] Media session is already closed");
            return;
        }

        MtcJniProxy.getInstance().sendDataToNative(sessionId, parcel);
    }

    private static void log(String s) {
        ImsLog.d("[GII-MTC] " + s);
    }
}
