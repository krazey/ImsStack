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

import android.os.Parcel;
import android.telephony.imsmedia.ImsMediaManager;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.RtpReceptionStats;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.IMtcMediaInterface;
import com.android.imsstack.enabler.mtc.MtcMediaSession;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.concurrent.Executor;

/** This manages Media Sessions */
public class MediaSession implements IMediaConnectionObserver {

    private MtcMediaSession mMtcMediaSession;
    private final IBaseContext mContext;
    private AudioSessionHandler mAudioSessionHandler;
    private VideoSessionHandler mVideoSessionHandler;
    private TextSessionHandler mTextSessionHandler;
    private final MediaListener mMediaListener;
    private final MediaManagerHelper mMediaManager;
    private AudioVideoSync mAvSync;

    /**
     * Called by the ImsMediaManagerCallback when the ImsMedia service is connected.
     */
    @Override
    public void onMediaConnected() {
        ImsLog.d("handle ImsMedia Connected");
    }

    /**
     * Called by the ImsMediaManagerCallback when the ImsMedia service is disconnected.
     */
    @Override
    public void onMediaDisconnected() {
        ImsLog.d("Handle ImsMedia disconnection");
        if (mAudioSessionHandler != null) {
            mAudioSessionHandler.onImsMediaAudioMessage(MediaConstants.NOTIFY_MEDIA_DETACH, null);
        }

        if (mVideoSessionHandler != null) {
            mVideoSessionHandler.onImsMediaVideoMessage(MediaConstants.NOTIFY_MEDIA_DETACH, null);
        }

        if (mTextSessionHandler != null) {
            mTextSessionHandler.onImsMediaTextMessage(MediaConstants.NOTIFY_MEDIA_DETACH, null);
        }
    }

    public MediaSession(IBaseContext context, MtcMediaSession mtcMediaSession) {
        ImsLog.d("MediaSession created");
        mContext = context;
        mMtcMediaSession = mtcMediaSession;
        mMediaManager = new MediaManagerHelper(this);
        createAudioSession();
        mMediaListener = new MediaListener();
        setMtcMediaListener(mtcMediaSession);
        mAvSync = null;
    }

    /**
     * Set the RtpReceptionStats from the ImsMedia to AudioVideoSync to compare the delay for the AV
     * synchronization
     *
     * @param type The media type of the session
     * @param stats The RtpReceptionStats of the session
     */
    public void notifyRtpReceptionStats(int type, RtpReceptionStats stats) {
        if (mAvSync != null) {
            switch (type) {
                default:
                    break;
                case ImsMediaSession.SESSION_TYPE_AUDIO:
                    mAvSync.setAudioClockRate(mAudioSessionHandler.getSamplingRateKHz());
                    mAvSync.setAudioStats(stats);
                    break;
                case ImsMediaSession.SESSION_TYPE_VIDEO:
                    mAvSync.setVideoClockRate(mVideoSessionHandler.getSamplingRateKHz());
                    mAvSync.setVideoStats(stats);
                    adjustDelay(ImsMediaSession.SESSION_TYPE_VIDEO, mAvSync.getVideoDelay());
                    break;
            }
        }
    }

    /**
     * request Video call data usage to the ImsMedia
     */

    public void requestCallDataUsage() {
        ImsLog.v("requestCallDataUsage");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.REQUEST_VIDEO_DATA_USAGE);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_VIDEO);
        parcel.setDataPosition(0);
        mMediaListener.onMediaMessage(parcel);
    }

    @VisibleForTesting
    public MediaSession(IBaseContext context, MtcMediaSession mtcMediaSession,
            ImsMediaManager imsMediaManager, Executor executor) {
        mContext = context;
        mMtcMediaSession = mtcMediaSession;
        mMediaManager =
                new MediaManagerHelper(context.getContext(), this, imsMediaManager, executor);
        mMediaListener = new MediaListener();
        setMtcMediaListener(mtcMediaSession);
        ImsLog.d("MediaSession created");
    }

    @VisibleForTesting
    MediaManagerHelper getMediaManager() {
        return mMediaManager;
    }

    @VisibleForTesting
    IMediaListener getMediaListenerProxy() {
        return mMediaListener;
    }

    @VisibleForTesting
    void setAudioSessionHandler(AudioSessionHandler audioSessionHandler) {
        mAudioSessionHandler = audioSessionHandler;
    }

    @VisibleForTesting
    void setVideoSessionHandler(VideoSessionHandler videoSessionHandler) {
        mVideoSessionHandler = videoSessionHandler;
    }

    @VisibleForTesting
    void setTextSessionHandler(TextSessionHandler textSessionHandler) {
        mTextSessionHandler = textSessionHandler;
    }

    private void createAudioSession() {
        if (mAudioSessionHandler == null) {
            mAudioSessionHandler =
                    new AudioSessionHandler(mContext, mMediaManager, mMtcMediaSession);
        }
    }

    private void createVideoSession() {
        if (mVideoSessionHandler == null) {
            mVideoSessionHandler = new VideoSessionHandler(mContext, mMediaManager,
                    mMtcMediaSession, mMtcMediaSession);
        }
        if (mAvSync == null) {
            mAvSync = new AudioVideoSync();
        }
    }

    private void createTextSession() {
        if (mTextSessionHandler == null) {
            mTextSessionHandler =
                    new TextSessionHandler(mContext, mMediaManager, mMtcMediaSession);
        }
    }

    private void setMtcMediaListener(IMtcMediaInterface mtcMediaInterface) {
        if (mtcMediaInterface != null) {
            mtcMediaInterface.setMediaListener(mMediaListener);
        }
    }

    private void adjustDelay(int type, int delay) {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.REQUEST_ADJUST_DELAY);
        parcel.writeInt(type);
        parcel.writeInt(delay);
        parcel.setDataPosition(0);
        mMediaListener.onMediaMessage(parcel);
    }

    private class MediaListener implements IMediaListener {

        @Override
        public void onMediaMessage(Parcel parcel) {
            final int requestType = parcel.readInt();
            final int sessionType = parcel.readInt();
            ImsLog.d("requestType=" + requestType + ", sessionType=" + sessionType);

            switch (sessionType) {
                case ImsMediaSession.SESSION_TYPE_AUDIO: {
                    createAudioSession();
                    if (mAudioSessionHandler != null
                                && mAudioSessionHandler.isValidRequest(requestType)) {
                        mAudioSessionHandler.onImsMediaAudioMessage(requestType, parcel);
                    }
                }
                    break;

                case ImsMediaSession.SESSION_TYPE_VIDEO: {
                    if (requestType == MediaConstants.REQUEST_OPEN_SESSION) {
                        createVideoSession();
                    }
                    if (mVideoSessionHandler != null
                            && mVideoSessionHandler.isValidRequest(requestType)) {
                        mVideoSessionHandler.onImsMediaVideoMessage(requestType, parcel);
                    }
                }
                    break;

                case ImsMediaSession.SESSION_TYPE_RTT: {
                    if (requestType == MediaConstants.REQUEST_OPEN_SESSION) {
                        createTextSession();
                    }
                    if (mTextSessionHandler != null
                            && mTextSessionHandler.isValidRequest(requestType)) {
                        mTextSessionHandler.onImsMediaTextMessage(requestType, parcel);
                    }
                }
                    break;

                default: {
                    parcel.recycle();
                    ImsLog.e("Invalid SessionType");
                }
                    break;
            }
        }
    }
}
