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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.media.ToneGenerator;

import com.android.imsstack.enabler.IBaseContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.List;

@RunWith(JUnit4.class)
public class DtmfToneGeneratorTest {

    // Wrapper class to allow mocking of ToneGenerator's final methods.
    public static class ToneGeneratorWrapper extends ToneGenerator {
        public ToneGeneratorWrapper(int streamType, int volume) {
            super(streamType, volume);
        }
        // Override and delegate to allow mocking.
        @Override
        public boolean startTone(int toneType, int durationMs) {
            return super.startTone(toneType, durationMs);
        }
        @Override
        public void release() {
            super.release();
        }
    }

    private AutoCloseable mMockitoSession;
    @Mock private IBaseContext mMockContext;
    @Mock private ToneGeneratorWrapper mMockToneGenerator;

    @Before
    public void setUp() throws Exception {
        // Initialize mocks.
        mMockitoSession = MockitoAnnotations.openMocks(this);
    }

    @After
    public void tearDown() throws Exception {
        // This prevents UnfinishedVerificationException in the next test's setUp.
        if (mMockitoSession != null) {
            mMockitoSession.close();
        }
    }

    @Test
    public void testPlay_CreatesGeneratorOnce() {
        DtmfToneGenerator dtmfGeneratorSpy = spy(new DtmfToneGenerator(mMockContext));

        doReturn(mMockToneGenerator).when(dtmfGeneratorSpy).createToneGenerator();

        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_1, 200);

        verify(dtmfGeneratorSpy, times(1)).createToneGenerator();
        verify(mMockToneGenerator).startTone(ToneGenerator.TONE_DTMF_1, 200);
    }

    @Test
    public void testPlay_OtherDtmfTones() {
        DtmfToneGenerator dtmfGeneratorSpy = spy(new DtmfToneGenerator(mMockContext));
        doReturn(mMockToneGenerator).when(dtmfGeneratorSpy).createToneGenerator();

        // Perform multiple play actions to verify caching and sequential calls.
        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_0, 100); // Index 0
        dtmfGeneratorSpy.play(10, 150);                        // Index 1 (*)
        dtmfGeneratorSpy.play(11, 200);                        // Index 2 (#)
        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_A, 250); // Index 3

        ArgumentCaptor<Integer> toneCaptor = ArgumentCaptor.forClass(Integer.class);
        ArgumentCaptor<Integer> durationCaptor = ArgumentCaptor.forClass(Integer.class);

        // Verify that createToneGenerator was only called once (caching logic).
        verify(dtmfGeneratorSpy, times(1)).createToneGenerator();

        // Verify that startTone was called exactly 4 times with the correct parameters
        verify(mMockToneGenerator, times(4)).startTone(
                toneCaptor.capture(), durationCaptor.capture());

        List<Integer> capturedTones = toneCaptor.getAllValues();
        List<Integer> capturedDurations = durationCaptor.getAllValues();

        // Verify parameters for each specific call index.
        assertEquals(ToneGenerator.TONE_DTMF_0, (int) capturedTones.get(0));
        assertEquals(100, (int) capturedDurations.get(0));

        assertEquals(10, (int) capturedTones.get(1));
        assertEquals(150, (int) capturedDurations.get(1));

        assertEquals(11, (int) capturedTones.get(2));
        assertEquals(200, (int) capturedDurations.get(2));

        assertEquals(ToneGenerator.TONE_DTMF_A, (int) capturedTones.get(3));
        assertEquals(250, (int) capturedDurations.get(3));
    }

    @Test
    public void testPlay_InvalidDuration_UsesDefaultDuration() {
        DtmfToneGenerator dtmfGeneratorSpy = spy(new DtmfToneGenerator(mMockContext));
        doReturn(mMockToneGenerator).when(dtmfGeneratorSpy).createToneGenerator();

        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_1, 0);
        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_1, -100);

        ArgumentCaptor<Integer> toneCaptor = ArgumentCaptor.forClass(Integer.class);
        ArgumentCaptor<Integer> durationCaptor = ArgumentCaptor.forClass(Integer.class);

        // createToneGenerator should only be called once.
        verify(dtmfGeneratorSpy, times(1)).createToneGenerator();
        // startTone should be called twice with the default duration.
        verify(mMockToneGenerator, times(2)).startTone(toneCaptor.capture(),
                durationCaptor.capture());
        assertEquals(ToneGenerator.TONE_DTMF_1, toneCaptor.getAllValues().get(0).intValue());
        assertEquals(300, durationCaptor.getAllValues().get(1).intValue());
    }

    @Test
    public void testPlay_CreateGeneratorFails() {
        DtmfToneGenerator dtmfGeneratorSpy = spy(new DtmfToneGenerator(mMockContext));

        doReturn(null).when(dtmfGeneratorSpy).createToneGenerator();
        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_1, 200);

        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_2, 200);

        verify(dtmfGeneratorSpy, times(1)).createToneGenerator();
        verify(mMockToneGenerator, never()).startTone(anyInt(), anyInt());
    }

    @Test
    public void testPlay_MultiplePlays_GeneratorReused() {
        DtmfToneGenerator dtmfGeneratorSpy = spy(new DtmfToneGenerator(mMockContext));
        doReturn(mMockToneGenerator).when(dtmfGeneratorSpy).createToneGenerator();

        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_1, 100);
        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_2, 100);
        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_3, 100);

        ArgumentCaptor<Integer> toneCaptor = ArgumentCaptor.forClass(Integer.class);

        verify(dtmfGeneratorSpy, times(1)).createToneGenerator();
        verify(mMockToneGenerator, times(3)).startTone(toneCaptor.capture(), anyInt());

        List<Integer> capturedTones = toneCaptor.getAllValues();
        assertEquals(3, capturedTones.size());
        assertEquals(ToneGenerator.TONE_DTMF_1, capturedTones.get(0).intValue());
        assertEquals(ToneGenerator.TONE_DTMF_2, capturedTones.get(1).intValue());
        assertEquals(ToneGenerator.TONE_DTMF_3, capturedTones.get(2).intValue());
    }

    @Test
    public void testPlay_AfterRelease_GeneratorRecreated() {
        DtmfToneGenerator dtmfGeneratorSpy = spy(new DtmfToneGenerator(mMockContext));
        doReturn(mMockToneGenerator).when(dtmfGeneratorSpy).createToneGenerator();

        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_1, 100);
        dtmfGeneratorSpy.release();
        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_2, 100);

        ArgumentCaptor<Integer> toneCaptor = ArgumentCaptor.forClass(Integer.class);

        verify(dtmfGeneratorSpy, times(2)).createToneGenerator();
        verify(mMockToneGenerator, times(2)).startTone(toneCaptor.capture(), anyInt());
        verify(mMockToneGenerator, times(1)).release();

        List<Integer> capturedTones = toneCaptor.getAllValues();
        assertEquals(ToneGenerator.TONE_DTMF_1, capturedTones.get(0).intValue());
        assertEquals(ToneGenerator.TONE_DTMF_2, capturedTones.get(1).intValue());
    }

    @Test
    public void testPlay_ToneGeneratorThrowsException() {
        DtmfToneGenerator dtmfGeneratorSpy = spy(new DtmfToneGenerator(mMockContext));

        doThrow(new RuntimeException("Test Exception")).when(mMockToneGenerator)
                .startTone(anyInt(), anyInt());
        doReturn(mMockToneGenerator).when(dtmfGeneratorSpy).createToneGenerator();

        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_1, 200);

        verify(dtmfGeneratorSpy, times(1)).createToneGenerator();
        verify(mMockToneGenerator, times(1)).startTone(anyInt(), anyInt());

        // Subsequent calls to play should be skipped due to the failure flag.
        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_2, 200);
        verify(dtmfGeneratorSpy, times(1)).createToneGenerator(); // Still only once
        verify(mMockToneGenerator, times(1)).startTone(anyInt(), anyInt()); // Still only once
    }

    @Test
    public void testIsToneGeneratorCreationFailed() {
        DtmfToneGenerator dtmfGeneratorSpy = spy(new DtmfToneGenerator(mMockContext));
        doReturn(mMockToneGenerator).when(dtmfGeneratorSpy).createToneGenerator();

        // Initially, the flag should be false.
        assertFalse(dtmfGeneratorSpy.isToneGeneratorCreationFailed());

        doThrow(new RuntimeException("Test Exception")).when(mMockToneGenerator)
                .startTone(anyInt(), anyInt());
        dtmfGeneratorSpy.play(ToneGenerator.TONE_DTMF_1, 200);

        // The flag should now be true.
        assertTrue(dtmfGeneratorSpy.isToneGeneratorCreationFailed());
    }
}
