/*
 * Copyright (C) 2025 The Android Open Source Project
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

import static com.android.imsstack.enabler.media.MediaState.MEDIA_STATE_CLOSED;
import static com.android.imsstack.enabler.media.MediaState.MEDIA_STATE_IDLE;
import static com.android.imsstack.enabler.media.MediaState.MEDIA_STATE_LIVE;
import static com.android.imsstack.enabler.media.MediaState.MEDIA_STATE_OPENING;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import android.telephony.imsmedia.ImsMediaSession;
import android.testing.AndroidTestingRunner;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class MediaStateTest {

    private MediaState mMediaState;

    @Before
    public void setUp() {
        mMediaState = new MediaState(ImsMediaSession.SESSION_TYPE_AUDIO);
    }

    @Test
    public void testInitialState() {
        assertEquals(ImsMediaSession.SESSION_TYPE_AUDIO, mMediaState.getMediaType());
        assertEquals(MEDIA_STATE_IDLE, mMediaState.getMediaState());
        assertTrue(mMediaState.isIdle());
        assertFalse(mMediaState.isOpening());
        assertFalse(mMediaState.isLive());
        assertFalse(mMediaState.isClosed());
    }

    @Test
    public void testSetMediaState_opening() {
        mMediaState.setMediaState(MEDIA_STATE_OPENING);
        assertEquals(MEDIA_STATE_OPENING, mMediaState.getMediaState());
        assertFalse(mMediaState.isIdle());
        assertTrue(mMediaState.isOpening());
        assertFalse(mMediaState.isLive());
        assertFalse(mMediaState.isClosed());
    }

    @Test
    public void testSetMediaState_live() {
        mMediaState.setMediaState(MEDIA_STATE_LIVE);
        assertEquals(MEDIA_STATE_LIVE, mMediaState.getMediaState());
        assertFalse(mMediaState.isIdle());
        assertFalse(mMediaState.isOpening());
        assertTrue(mMediaState.isLive());
        assertFalse(mMediaState.isClosed());
    }

    @Test
    public void testSetMediaState_closed() {
        mMediaState.setMediaState(MEDIA_STATE_CLOSED);
        assertEquals(MEDIA_STATE_CLOSED, mMediaState.getMediaState());
        assertFalse(mMediaState.isIdle());
        assertFalse(mMediaState.isOpening());
        assertFalse(mMediaState.isLive());
        assertTrue(mMediaState.isClosed());
    }

    @Test
    public void testSetMediaState_idle() {
        mMediaState.setMediaState(MEDIA_STATE_LIVE);
        mMediaState.setMediaState(MEDIA_STATE_IDLE);
        assertEquals(MEDIA_STATE_IDLE, mMediaState.getMediaState());
        assertTrue(mMediaState.isIdle());
        assertFalse(mMediaState.isOpening());
        assertFalse(mMediaState.isLive());
        assertFalse(mMediaState.isClosed());
    }

    @Test
    public void testSetMediaType() {
        mMediaState.setMediaType(ImsMediaSession.SESSION_TYPE_VIDEO);
        assertEquals(ImsMediaSession.SESSION_TYPE_VIDEO, mMediaState.getMediaType());
    }

    @Test
    public void testConstructorWithVideoType() {
        mMediaState = new MediaState(ImsMediaSession.SESSION_TYPE_VIDEO);
        assertEquals(ImsMediaSession.SESSION_TYPE_VIDEO, mMediaState.getMediaType());
        assertEquals(MEDIA_STATE_IDLE, mMediaState.getMediaState());
    }

    @Test
    public void testSetMediaState_unknown() {
        mMediaState.setMediaState(10);
        assertEquals(10, mMediaState.getMediaState());
    }
}
