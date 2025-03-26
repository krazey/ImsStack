/*
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

import android.telephony.imsmedia.ImsMediaSession;

import com.android.imsstack.util.ImsLog;

/**
 * This manages state of a media session
 */
public class MediaState {

    /**
     * Session is idle and can process OpenSession request. Any other requests will be discarded.
     */
    protected static final int MEDIA_STATE_IDLE = 0;

    /**
     * OpenSession requested to ImsMedia, waiting for response. Any further requests will be in
     * queue till response is received.
     */
    protected static final int MEDIA_STATE_OPENING = 1;

    /** Session is opened, queued requests will be processed. */
    protected static final int MEDIA_STATE_LIVE = 2;

    /**
     * CloseSession requested to ImsMedia, waiting for response. Any further requests will be
     * discarded. State will be moved to idle, when response to CloseSession is received.
     */
    protected static final int MEDIA_STATE_CLOSED = 3;

    private int mMediaType = ImsMediaSession.SESSION_TYPE_AUDIO;
    private int mMediaState = MEDIA_STATE_IDLE;

    public MediaState(int mediaType) {
        mMediaType = mediaType;
    }

    protected int getMediaState() {
        return mMediaState;
    }

    protected void setMediaState(int state) {
        if (mMediaState != state) {
            ImsLog.d("Media[" + getMediaType() + "] state updated :: "
                    + mediaStateToString(mMediaState)
                    + " >> "
                    + mediaStateToString(state));
            mMediaState = state;
        }
    }

    protected int getMediaType() {
        return mMediaType;
    }

    protected void setMediaType(int mediaType) {
        mMediaType = mediaType;
    }

    protected boolean isIdle() {
        return getMediaState() == MEDIA_STATE_IDLE;
    }

    protected boolean isOpening() {
        return getMediaState() == MEDIA_STATE_OPENING;
    }

    protected boolean isLive() {
        return getMediaState() == MEDIA_STATE_LIVE;
    }

    protected boolean isClosed() {
        return getMediaState() == MEDIA_STATE_CLOSED;
    }

    private String mediaStateToString(int state) {
        switch (state) {
            case MEDIA_STATE_IDLE:
                return "IDLE";
            case MEDIA_STATE_OPENING:
                return "OPENING";
            case MEDIA_STATE_LIVE:
                return "LIVE";
            case MEDIA_STATE_CLOSED:
                return "CLOSED";
            default:
                return "__UNKNOWN__";
        }
    }
}
