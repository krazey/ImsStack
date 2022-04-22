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
import com.android.imsstack.enabler.media.AudioSessionHandler;
import com.android.imsstack.enabler.media.IMediaListener;
import com.android.imsstack.enabler.media.MediaManagerHelper;
import com.android.imsstack.enabler.mtc.MtcMediaSession;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;
import java.util.concurrent.Executor;

/**
 * This manages Media Sessions
 */
public class MediaSession {

    private MtcMediaSession mMtcMediaSession;
    private final IBaseContext mContext;
    private AudioSessionHandler mAudioSessionHandler;
    private final MediaListener mMediaListener;
    private final MediaManagerHelper mMediaManager;

    public MediaSession(IBaseContext context, MtcMediaSession mtcMediaSession) {
        ImsLog.v("MediaSession created");
        mContext = context;
        mMediaManager = new MediaManagerHelper(this);
        createAudioSession();
        mMediaListener = new MediaListener();
        initMtcMediaSession(mtcMediaSession);
    }

    @VisibleForTesting
    public MediaSession(IBaseContext context, MtcMediaSession mtcMediaSession,
        ImsMediaManager imsMediaManager, Executor executor) {
        mContext = context;
        mMediaManager = new MediaManagerHelper(context.getContext(), this,
                imsMediaManager, executor);
        mMediaListener = new MediaListener();
        initMtcMediaSession(mtcMediaSession);
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

    private void createAudioSession() {
        if (mAudioSessionHandler == null) {
            mAudioSessionHandler = new AudioSessionHandler(this, mMediaManager);
        }
    }

    private void initMtcMediaSession(MtcMediaSession mtcMediaSession) {
        mMtcMediaSession = mtcMediaSession;
        if (mtcMediaSession != null) {
            mtcMediaSession.setMediaListener(mMediaListener);
        }
    }

    /**
     * Sends response to media enabler native
     */
    public void sendRequest(Parcel parcel) {
        if (mMtcMediaSession != null) {
            mMtcMediaSession.sendRequest(parcel);
        }
    }

    /**
     * Returns slotId
     */
    public int getSlotId() {
        return mContext.getSlotId();
    }

    private class MediaListener implements IMediaListener {

        @Override
        public void onMediaMessage(Parcel parcel) {
            final int requestType = parcel.readInt();
            final int sessionType = parcel.readInt();
            ImsLog.v("requestType=" + requestType + ", sessionType=" +sessionType);

            switch (sessionType) {
            case ImsMediaSession.SESSION_TYPE_AUDIO:
                {
                    createAudioSession();
                    if (mAudioSessionHandler != null) {
                        mAudioSessionHandler.onImsMediaAudioMessage(requestType, parcel);
                    }
                }
                break;

            case ImsMediaSession.SESSION_TYPE_VIDEO:
                {
                    ImsLog.v("SESSION_TYPE_VIDEO :: TODO");
                }
                break;

            case ImsMediaSession.SESSION_TYPE_RTT:
                {
                    ImsLog.v("SESSION_TYPE_RTT :: TODO");
                }
                break;

            default:
                {
                    ImsLog.e("Invalid SessionType");
                }
                break;
            }
        }
    }
}