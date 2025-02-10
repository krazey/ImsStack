/**
 * Copyright (C) 2023 The Android Open Source Project
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

import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import android.telephony.ims.MediaThreshold;
import android.telephony.imsmedia.MediaQualityThreshold;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class MediaConfigTest {

    private MediaConfig mMediaConfig;

    @Before
    public void setUp() throws Exception {
        mMediaConfig = new MediaConfig();
    }

    @After
    public void tearDown() throws Exception {
        mMediaConfig = null;
    }

    @Test
    public void testResetMediaConfig() {
        MediaQualityThreshold expectedThreshold =  new MediaQualityThreshold.Builder()
                .setRtpInactivityTimerMillis(new int[0])
                .setRtcpInactivityTimerMillis(0)
                .setRtpHysteresisTimeInMillis(MediaTestUtils.RTP_HYSTERESIS_TIME)
                .setRtpPacketLossDurationMillis(MediaTestUtils.PACKET_LOSS_DURATION)
                .setRtpPacketLossRate(new int[0])
                .setRtpJitterMillis(new int[0])
                .setNotifyCurrentStatus(MediaTestUtils.NOTIFY_STATUS)
                .build();
        assertTrue(mMediaConfig.getMediaQualityThreshold().equals(expectedThreshold));
        assertNull(mMediaConfig.getRtpConfig());
    }

    @Test
    public void testUpdateMediaQualityThreshold() {
        MediaQualityThreshold mediaQualityThreshold = MediaTestUtils.createMediaQualityThreshold();
        mMediaConfig.updateMediaQualityThreshold(mediaQualityThreshold, true);
        MediaQualityThreshold expectedThreshold =  new MediaQualityThreshold.Builder()
                .setRtpInactivityTimerMillis(MediaTestUtils.RTP_TIMEOUT)
                .setRtcpInactivityTimerMillis(MediaTestUtils.RTCP_TIMEOUT)
                .setRtpHysteresisTimeInMillis(MediaTestUtils.RTP_HYSTERESIS_TIME)
                .setRtpPacketLossDurationMillis(MediaTestUtils.PACKET_LOSS_DURATION)
                .setRtpPacketLossRate(new int[0])
                .setRtpJitterMillis(new int[0])
                .setNotifyCurrentStatus(MediaTestUtils.NOTIFY_STATUS)
                .build();
        assertTrue(mMediaConfig.getMediaQualityThreshold().equals(expectedThreshold));
    }

    @Test
    public void testUpdateQnsMediaThreshold() {
        MediaThreshold mediaThreshold = MediaTestUtils.createMediaThreshold();
        mMediaConfig.updateMediaQualityThreshold(mediaThreshold);
        MediaQualityThreshold expectedThreshold =  new MediaQualityThreshold.Builder()
                .setRtpInactivityTimerMillis(new int[]{ 5000, 15000 })
                .setRtcpInactivityTimerMillis(0)
                .setRtpHysteresisTimeInMillis(MediaTestUtils.RTP_HYSTERESIS_TIME)
                .setRtpPacketLossDurationMillis(MediaTestUtils.PACKET_LOSS_DURATION)
                .setRtpPacketLossRate(MediaTestUtils.PACKET_LOSS_RATE)
                .setRtpJitterMillis(MediaTestUtils.JITTER_THRESHOLD)
                .setNotifyCurrentStatus(MediaTestUtils.NOTIFY_STATUS)
                .build();
        assertTrue(mMediaConfig.getMediaQualityThreshold().equals(expectedThreshold));
    }

    @Test
    public void testCombinedMediaQualityThreshold() {
        MediaQualityThreshold mediaQualityThreshold = MediaTestUtils.createMediaQualityThreshold();
        MediaThreshold mediaThreshold = MediaTestUtils.createMediaThreshold();
        mMediaConfig.updateMediaQualityThreshold(mediaQualityThreshold, true);
        mMediaConfig.updateMediaQualityThreshold(mediaThreshold);
        MediaQualityThreshold expectedThreshold =  new MediaQualityThreshold.Builder()
                .setRtpInactivityTimerMillis(new int[]{ 10000, 20000, 5000, 15000 })
                .setRtcpInactivityTimerMillis(MediaTestUtils.RTCP_TIMEOUT)
                .setRtpHysteresisTimeInMillis(MediaTestUtils.RTP_HYSTERESIS_TIME)
                .setRtpPacketLossDurationMillis(MediaTestUtils.PACKET_LOSS_DURATION)
                .setRtpPacketLossRate(MediaTestUtils.PACKET_LOSS_RATE)
                .setRtpJitterMillis(MediaTestUtils.JITTER_THRESHOLD)
                .setNotifyCurrentStatus(MediaTestUtils.NOTIFY_STATUS)
                .build();
        assertTrue(mMediaConfig.getMediaQualityThreshold().equals(expectedThreshold));

        // Media threshold from Native updated after threshold from frameworks
        mMediaConfig.resetMediaConfig();
        mMediaConfig.updateMediaQualityThreshold(mediaThreshold);
        mMediaConfig.updateMediaQualityThreshold(mediaQualityThreshold, true);
        assertTrue(mMediaConfig.getMediaQualityThreshold().equals(expectedThreshold));
    }

    @Test
    public void testMediaQualityThresholdwithoutQnsThreshold() {
        MediaQualityThreshold mediaQualityThreshold = MediaTestUtils.createMediaQualityThreshold();
        MediaThreshold mediaThreshold = MediaTestUtils.createMediaThreshold();
        mMediaConfig.updateMediaQualityThreshold(mediaThreshold);
        mMediaConfig.updateMediaQualityThreshold(mediaQualityThreshold, false);
        MediaQualityThreshold expectedThreshold = new MediaQualityThreshold.Builder()
                .setRtpInactivityTimerMillis(new int[]{ 10000, 20000})
                .setRtcpInactivityTimerMillis(MediaTestUtils.RTCP_TIMEOUT)
                .setRtpHysteresisTimeInMillis(MediaTestUtils.RTP_HYSTERESIS_TIME)
                .setRtpPacketLossDurationMillis(MediaTestUtils.PACKET_LOSS_DURATION)
                .setRtpPacketLossRate(MediaTestUtils.PACKET_LOSS_RATE)
                .setRtpJitterMillis(MediaTestUtils.JITTER_THRESHOLD)
                .setNotifyCurrentStatus(MediaTestUtils.NOTIFY_STATUS)
                .build();
        assertTrue(mMediaConfig.getMediaQualityThreshold().equals(expectedThreshold));
    }

    @Test
    public void testClearQnsMediaThreshold() {
        MediaQualityThreshold mediaQualityThreshold = MediaTestUtils.createMediaQualityThreshold();
        MediaThreshold mediaThreshold = MediaTestUtils.createMediaThreshold();
        mMediaConfig.updateMediaQualityThreshold(mediaThreshold);
        mMediaConfig.updateMediaQualityThreshold(mediaQualityThreshold, true);
        // clear threshold from frameworks
        mMediaConfig.updateMediaQualityThreshold((MediaThreshold) null);
        MediaQualityThreshold expectedThreshold =  new MediaQualityThreshold.Builder()
                .setRtpInactivityTimerMillis(MediaTestUtils.RTP_TIMEOUT)
                .setRtcpInactivityTimerMillis(MediaTestUtils.RTCP_TIMEOUT)
                .setRtpHysteresisTimeInMillis(MediaTestUtils.RTP_HYSTERESIS_TIME)
                .setRtpPacketLossDurationMillis(MediaTestUtils.PACKET_LOSS_DURATION)
                .setRtpPacketLossRate(new int[0])
                .setRtpJitterMillis(new int[0])
                .setNotifyCurrentStatus(MediaTestUtils.NOTIFY_STATUS)
                .build();
        assertTrue(mMediaConfig.getMediaQualityThreshold().equals(expectedThreshold));
    }
}
