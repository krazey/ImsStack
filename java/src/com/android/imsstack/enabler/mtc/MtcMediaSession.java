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

import android.content.Context;
import android.os.Parcel;
import android.view.Surface;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.media.IMediaListener;
import com.android.imsstack.enabler.media.MediaConstants;
import com.android.imsstack.enabler.media.MediaFactory;
import com.android.imsstack.enabler.media.MediaSession;
import com.android.imsstack.jni.JNIIms;
import com.android.imsstack.util.ImsLog;

/**
 * class to handles call from videoprovider and invoke jni methods to libimsstack
 */
public class MtcMediaSession implements MediaSession.IMediaSurfaceHandler {
    /**
     * Listener for events relating to an MTC media session.
     * <p>
     * Many of these events are also received by {@link MtcMediaSession.Listener}.
     * </p>
     */
    public static class Listener {
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
                final int videoResolution) {
            // no-op
        }
    }

    public static class RttListener {
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
     * Display orientation.
     */
    public final static int ORIENTATION_0 = 0;
    public final static int ORIENTATION_90 = 1;
    public final static int ORIENTATION_270 = 2;
    public final static int ORIENTATION_180 = 3;

    private static final int STATE_NONE = 0x0;
    private static final int STATE_AUDIO_STARTED = 0x1;
    private static final int STATE_AUDIO_PAUSED = 0x2;
    private static final int STATE_MEDIA_STARTED = 0x4;

    /**
     * This surface variables will be accessed by the native layer.
     * It just grabs the object to be enable to access the surface.
     */
    private static Surface mPreviewSurface = null;
    private static Surface mDisplaySurface = null;

    private final Object mLock = new Object();
    private Context mContext;
    private Call mCall;
    private int mState = STATE_NONE;
    private MtcMediaSession.Listener mListener = null;
    private MtcMediaSession.RttListener mRttListener = null;
    private IMediaListener mMediaListener = null;
    private MediaInfoEvent mMediaInfoEvent = null;
    public int mPrevOrientation = 0;
    private final MediaSession mMediaSession;

    public MtcMediaSession(IBaseContext context, Call call) {
        mContext = context.getContext();
        mCall = call;
        mMediaSession = MediaFactory.createMediaSession(context, this);
    }

    public void dispose() {
        if (!isMediaSessionValid()) {
            return;
        }

        synchronized (mLock) {
            mCall = null;
            mListener = null;
            mRttListener = null;
            mMediaListener = null;
            mMediaInfoEvent = null;
        }
    }

    public void setListener(MtcMediaSession.Listener listener) {
        synchronized (mLock) {
            mListener = listener;
        }
    }

    public void setRttListener(MtcMediaSession.RttListener listener) {
        synchronized (mLock) {
            mRttListener = listener;
        }
    }

    public void setMediaListener(IMediaListener listener) {
        synchronized (mLock) {
            mMediaListener = listener;
        }
    }

    /**
     * Gets the call-id to identify the current call media.
     */
    public long getCallId() {
        return isMediaSessionValid() ? mCall.getNativeCallId() : 0;
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
     * CAMERA
     * Selects the camera type for video call.
     *
     * @param camera the camera type
     *               {@link ImsCamera#CAMERA_REAR}
     *               {@link ImsCamera#CAMERA_FRONT}
     */
    public void selectCamera(int camera) {
        log("selectCamera :: camera=" + camera);

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
     * Sets the orientation of the specified display.
     *
     * @param display     the display type
     *                    {@link ImsVideoCallProviderBase#DISPLAY_NEAR}
     *                    {@link ImsVideoCallProviderBase#DISPLAY_FAR}
     *                    {@link ImsVideoCallProviderBase#DISPLAY_NEAR_N_FAR}
     * @param orientation the orientation of the specified display
     *                    {@link MtcMediaSession#ORIENTATION_0}
     *                    {@link MtcMediaSession#ORIENTATION_90}
     *                    {@link MtcMediaSession#ORIENTATION_270}
     *                    {@link MtcMediaSession#ORIENTATION_180}
     */
    public void setDisplayOrientation(int display, int orientation) {
        log("setDisplayOrientation :: display=" + display +
                ", orientation=" + orientation);
        mPrevOrientation = orientation;
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcMedia.CHANGE_ORIENTATION_CMD);
        parcel.writeInt(display);
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
        Listener listener = null;

        synchronized (mLock) {
            listener = mListener;
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

    public void onMessage(Parcel parcel) {
        int msg = parcel.readInt();

        if (isMessageForImsMediaManager(msg)) {
            parcel.setDataPosition(0);
            onImsMediaManagerMessage(parcel);
            return;
        }

        Listener listener = null;

        synchronized (mLock) {
            listener = mListener;
        }

        if (listener == null) {
            log("No listener: " + this);
            return;
        }

        switch (msg) {
            case IUMtcMedia.RTT_TEXT_RECEIVED_IND: {
                String str = parcel.readString();

                log("RTT_TEXT_RECEIVED_IND :: text=" + str);

                if (mRttListener != null) {
                    mRttListener.onRttMessageReceived(this, str);
                }
            }
                break;

            case IUMtcMedia.RTT_AUDIO_INDICATION_IND: {
                boolean status = (parcel.readInt()) > 0;

                log("RTT_AUDIO_INDICATION_IND :: status=" + status);

                if (mRttListener != null) {
                    mRttListener.onRttAudioIndication(this, status);
                }
            }
                break;

            default:
                break;
        }
    }

    public static boolean isMessageForMediaSession(int msg) {
        return (((msg > IUMtcMedia.IMS_MSG_BASE_MEDIA)
                && (msg < (IUMtcMedia.IMS_MSG_BASE_MEDIA + 100)))
                || (isMessageForImsMediaManager(msg)));
    }

    private void onImsMediaManagerMessage(Parcel parcel) {

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

    private static boolean isMessageForImsMediaManager(int msg) {
        return (msg > MediaConstants.IMSMEDIA_REQUEST)
                && (msg < MediaConstants.IMSMEDIA_MAX);
    }

    private boolean isCallSessionTerminated() {
        if (!isMediaSessionValid()) {
            return true;
        }

        synchronized (mLock) {
            return (mCall.isTerminated()
                    || (mCall.getCallState() == CallTracker.CALL_STATE_IDLE));
        }
    }

    private boolean isMediaSessionValid() {
        synchronized (mLock) {
            return mCall != null;
        }
    }

    public void sendRttMessage(String data) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.RTT_TEXT_SEND_CMD);
        parcel.writeString(data);

        sendRequest(parcel);
    }

    public void sendRequest(Parcel parcel) {
        long sessionId = 0;

        synchronized (mLock) {
            sessionId = (mCall != null) ? mCall.getNativeCallId() : 0;
        }

        if (sessionId == 0) {
            parcel.recycle();
            parcel = null;

            ImsLog.e("[GII-MTC] Media session is already closed");
            return;
        }

        byte[] data = parcel.marshall();
        parcel.recycle();
        parcel = null;

        JNIIms.sendData(sessionId, data);
    }

    private static void log(String s) {
        ImsLog.d("[GII-MTC] " + s);
    }
}
