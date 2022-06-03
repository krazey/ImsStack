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
    private final MediaListener mMediaListener;
    private final MediaManagerHelper mMediaManager;

    /**
     * Called by the ImsMediaManagerCallback when the ImsMedia service is connected.
     */
    @Override
    public void onMediaConnected() {
        ImsLog.v("handle ImsMedia Connected");
    }

    /**
     * Called by the ImsMediaManagerCallback when the ImsMedia service is disconnected.
     */
    @Override
    public void onMediaDisconnected() {
        ImsLog.v("Handle ImsMedia disconnection");
        if (mAudioSessionHandler != null) {
            mAudioSessionHandler.onImsMediaAudioMessage(MediaConstants.NOTIFY_MEDIA_DETACH, null);
        }

        if (mVideoSessionHandler != null) {
            mVideoSessionHandler.onImsMediaVideoMessage(MediaConstants.NOTIFY_MEDIA_DETACH, null);
        }
    }

    public MediaSession(IBaseContext context, MtcMediaSession mtcMediaSession) {
        ImsLog.v("MediaSession created");
        mContext = context;
        mMtcMediaSession = mtcMediaSession;
        mMediaManager = new MediaManagerHelper(this);
        createAudioSession();
        mMediaListener = new MediaListener();
        setMtcMediaListener(mtcMediaSession);
    }

    @VisibleForTesting
    public MediaSession(IBaseContext context, MtcMediaSession mtcMediaSession,
            ImsMediaManager imsMediaManager, Executor executor) {
        mContext = context;
        mMtcMediaSession = mtcMediaSession;
        mMediaManager = new MediaManagerHelper(context.getContext(), this,
                imsMediaManager, executor);
        mMediaListener = new MediaListener();
        setMtcMediaListener(mtcMediaSession);
        ImsLog.v("MediaSession created");
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

    private void createAudioSession() {
        if (mAudioSessionHandler == null) {
            mAudioSessionHandler = new AudioSessionHandler(mMediaManager, mMtcMediaSession);
        }
    }

    private void createVideoSession() {
        if (mVideoSessionHandler == null) {
            mVideoSessionHandler = new VideoSessionHandler(
                    mMediaManager, mMtcMediaSession, mMtcMediaSession);
        }
    }

    private void setMtcMediaListener(IMtcMediaInterface mtcMediaInterface) {
        if (mtcMediaInterface != null) {
            mtcMediaInterface.setMediaListener(mMediaListener);
        }
    }

    /** Returns slotId */
    public int getSlotId() {
        return mContext.getSlotId();
    }

    private class MediaListener implements IMediaListener {

        @Override
        public void onMediaMessage(Parcel parcel) {
            final int requestType = parcel.readInt();
            final int sessionType = parcel.readInt();
            ImsLog.v("requestType=" + requestType + ", sessionType=" + sessionType);

            switch (sessionType) {
                case ImsMediaSession.SESSION_TYPE_AUDIO: {
                    createAudioSession();
                    if (mAudioSessionHandler != null) {
                        mAudioSessionHandler.onImsMediaAudioMessage(requestType, parcel);
                    }
                }
                    break;

                case ImsMediaSession.SESSION_TYPE_VIDEO: {
                    createVideoSession();
                    if (mVideoSessionHandler != null) {
                        mVideoSessionHandler.onImsMediaVideoMessage(requestType, parcel);
                    }
                }
                    break;

                case ImsMediaSession.SESSION_TYPE_RTT: {
                    ImsLog.v("SESSION_TYPE_RTT :: TODO");
                }
                    break;

                default: {
                    ImsLog.e("Invalid SessionType");
                }
                    break;
            }
        }
    }
}
