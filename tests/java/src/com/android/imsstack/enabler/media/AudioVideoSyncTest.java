/**
 * Copyright (C) 2024 The Android Open Source Project
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

import static org.junit.Assert.assertTrue;

import android.telephony.imsmedia.RtpReceptionStats;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class AudioVideoSyncTest {

    private AudioVideoSync mAVSync;
    private RtpReceptionStats mAudioStats;
    private RtpReceptionStats mVideoStats;

    @Before
    public void setUp() throws Exception {
        mAVSync = new AudioVideoSync();
        mAudioStats = new RtpReceptionStats.Builder()
                .setRtpTimestamp(10)
                .setRtcpSrTimestamp(20)
                .setRtcpSrNtpTimestamp(200)
                .setJitterBufferMs(200)
                .setRoundTripTimeMs(20)
                .build();
        mVideoStats = new RtpReceptionStats.Builder()
                .setRtpTimestamp(10)
                .setRtcpSrTimestamp(20)
                .setRtcpSrNtpTimestamp(200)
                .setJitterBufferMs(100)
                .setRoundTripTimeMs(20)
                .build();
    }

    @After
    public void tearDown() throws Exception {
        mAVSync = null;
    }

    @Test
    public void testDefaultInstance() {
        assertTrue(mAVSync.getVideoDelay() == 0);
    }

    @Test
    public void testValidParameters() {
        mAVSync.setAudioClockRate(16);
        mAVSync.setVideoClockRate(90);
        mAVSync.setAudioStats(mAudioStats);
        mAVSync.setVideoStats(mVideoStats);
        assertTrue(mAVSync.getVideoDelay() == 110);
    }

    @Test
    public void testValidParametersWithTrimOff() {
        mAVSync.setAudioClockRate(16);
        mAVSync.setVideoClockRate(90);
        mAVSync.setAudioStats(mAudioStats);
        RtpReceptionStats videoStats = new RtpReceptionStats.Builder()
                .setRtpTimestamp(10)
                .setRtcpSrTimestamp(20)
                .setRtcpSrNtpTimestamp(200)
                .setJitterBufferMs(95)
                .setRoundTripTimeMs(20)
                .build();
        mAVSync.setVideoStats(videoStats);
        assertTrue(mAVSync.getVideoDelay() == 110);
    }

    @Test
    public void testInvalidClockRate() {
        mAVSync.setAudioClockRate(0);
        mAVSync.setVideoClockRate(90);
        mAVSync.setAudioStats(mAudioStats);
        mAVSync.setVideoStats(mVideoStats);
        assertTrue(mAVSync.getVideoDelay() == 0);

        mAVSync.setAudioClockRate(16);
        mAVSync.setVideoClockRate(0);
        assertTrue(mAVSync.getVideoDelay() == 0);
    }

    @Test
    public void testAudioReceptionStatsNull() {
        mAVSync.setAudioClockRate(16);
        mAVSync.setVideoClockRate(90);
        mAVSync.setVideoStats(mVideoStats);
        assertTrue(mAVSync.getVideoDelay() == 0);
    }

    @Test
    public void testVideoReceptionStatsNull() {
        mAVSync.setAudioClockRate(16);
        mAVSync.setVideoClockRate(90);
        mAVSync.setAudioStats(mAudioStats);
        assertTrue(mAVSync.getVideoDelay() == 0);
    }

    @Test
    public void testAudioReceptionStatsNotUpdated() {
        mAVSync.setAudioClockRate(16);
        mAVSync.setVideoClockRate(90);
        mAVSync.setAudioStats(mAudioStats);
        mAVSync.setVideoStats(mVideoStats);
        assertTrue(mAVSync.getVideoDelay() == 110);
        RtpReceptionStats audioStats = new RtpReceptionStats.Builder()
                .setRtpTimestamp(10)
                .setRtcpSrTimestamp(20)
                .setRtcpSrNtpTimestamp(200)
                .setJitterBufferMs(95)
                .setRoundTripTimeMs(20)
                .build();
        mAVSync.setAudioStats(audioStats);
        RtpReceptionStats videoStats = new RtpReceptionStats.Builder()
                .setRtpTimestamp(100)
                .setRtcpSrTimestamp(20)
                .setRtcpSrNtpTimestamp(200)
                .setJitterBufferMs(200)
                .setRoundTripTimeMs(20)
                .build();
        mAVSync.setVideoStats(videoStats);
        assertTrue(mAVSync.getVideoDelay() == 0);
    }

    @Test
    public void testVideoReceptionStatsNotUpdated() {
        mAVSync.setAudioClockRate(16);
        mAVSync.setVideoClockRate(90);
        mAVSync.setAudioStats(mAudioStats);
        mAVSync.setVideoStats(mVideoStats);
        assertTrue(mAVSync.getVideoDelay() == 110);
        RtpReceptionStats audioStats = new RtpReceptionStats.Builder()
                .setRtpTimestamp(100)
                .setRtcpSrTimestamp(20)
                .setRtcpSrNtpTimestamp(200)
                .setJitterBufferMs(95)
                .setRoundTripTimeMs(20)
                .build();
        mAVSync.setAudioStats(audioStats);
        RtpReceptionStats videoStats = new RtpReceptionStats.Builder()
                .setRtpTimestamp(10)
                .setRtcpSrTimestamp(20)
                .setRtcpSrNtpTimestamp(200)
                .setJitterBufferMs(200)
                .setRoundTripTimeMs(20)
                .build();
        mAVSync.setVideoStats(videoStats);
        assertTrue(mAVSync.getVideoDelay() == 0);
    }

    @Test
    public void testVideoDelayOverMaximum() {
        mAVSync.setAudioClockRate(16);
        mAVSync.setVideoClockRate(90);
        RtpReceptionStats audioStats = new RtpReceptionStats.Builder()
                .setRtpTimestamp(100)
                .setRtcpSrTimestamp(20)
                .setRtcpSrNtpTimestamp(200)
                .setJitterBufferMs(1000)
                .setRoundTripTimeMs(20)
                .build();
        mAVSync.setAudioStats(audioStats);
        mAVSync.setVideoStats(mVideoStats);
        assertTrue(mAVSync.getVideoDelay() == 400);
    }
}

