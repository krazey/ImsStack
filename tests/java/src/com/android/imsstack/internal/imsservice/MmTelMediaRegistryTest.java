/*
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
package com.android.imsstack.internal.imsservice;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.telephony.ims.MediaQualityStatus;
import android.telephony.ims.MediaThreshold;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class MmTelMediaRegistryTest {
    private MmTelMediaRegistry mMmTelMediaRegistry;
    private int mSessionType;
    private MediaThreshold mMediaThreshold;
    boolean mIsListenerCalled = false;

    @Before
    public void setUp() throws Exception {
        mMmTelMediaRegistry = new MmTelMediaRegistry();
    }

    @After
    public void tearDown() throws Exception {
        mMmTelMediaRegistry = null;
        mMediaThreshold = null;
        mSessionType = 0;
        mIsListenerCalled = false;
    }

    @Test
    public void testMmTelMediaRegistry() {
        mSessionType = MediaQualityStatus.MEDIA_SESSION_TYPE_AUDIO;
        MmTelMediaRegistry.Listener mockListener = Mockito.mock(MmTelMediaRegistry.Listener.class);

        /* Verify {@link MediaThreshold} is set and {@link #notifyMediaThresholdChanged} is called
         * when listener is added.
         */
        int[] packetLossRate = {1, 20};
        int[] jitterMillies = {20};
        long[] inactivityTimeMillies = {100, 7000};
        mMediaThreshold = new MediaThreshold.Builder()
                .setThresholdsRtpPacketLossRate(packetLossRate)
                .setThresholdsRtpJitterMillis(jitterMillies)
                .setThresholdsRtpInactivityTimeMillis(inactivityTimeMillies)
                .build();

        mMmTelMediaRegistry.addListener(mockListener);
        mMmTelMediaRegistry.setMediaThreshold(mSessionType, mMediaThreshold);
        assertEquals(mMediaThreshold, mMmTelMediaRegistry.getMediaThreshold(mSessionType));
        verify(mockListener).onMediaThresholdChanged(eq(mSessionType), eq(mMediaThreshold));

        /* Verify {@link MediaThreshold} is not set and {@link #notifyMediaThresholdChanged} is
         * called as @link MediaThreshold} is not changed for same session type.
         */
        mMmTelMediaRegistry.setMediaThreshold(mSessionType, mMediaThreshold);
        assertEquals(mMediaThreshold, mMmTelMediaRegistry.getMediaThreshold(mSessionType));
        verifyNoMoreInteractions(mockListener);

        // Add {@link MediaThreshold} for video session type.
        mSessionType = MediaQualityStatus.MEDIA_SESSION_TYPE_VIDEO;
        mMmTelMediaRegistry.setMediaThreshold(mSessionType, mMediaThreshold);
        assertEquals(mMediaThreshold, mMmTelMediaRegistry.getMediaThreshold(mSessionType));
        verify(mockListener).onMediaThresholdChanged(eq(mSessionType), eq(mMediaThreshold));

        // Verify {@link clearMediaThreshold}
        mMediaThreshold = null;
        mMmTelMediaRegistry.setMediaThreshold(mSessionType, null);
        assertEquals(mMediaThreshold, mMmTelMediaRegistry.getMediaThreshold(mSessionType));
        verify(mockListener).onMediaThresholdChanged(eq(mSessionType), eq(null));

        /* Verify {@link MediaThreshold} is set and {@link #notifyMediaThresholdChanged}
         * is not called when listener is removed.
         */
        mMmTelMediaRegistry.removeListener(mockListener);
        mMmTelMediaRegistry.setMediaThreshold(mSessionType, null);
        assertNull(mMmTelMediaRegistry.getMediaThreshold(mSessionType));
        verifyNoMoreInteractions(mockListener);
    }
}
