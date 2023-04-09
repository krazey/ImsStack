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

import android.telephony.ims.feature.MmTelFeature;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class MmTelMediaQualityReporterTest {
    private static final String CALL_ID = "1";

    @Mock MmTelMediaRegistry mMockMmTelMediaRegistry;
    @Mock MmTelFeature mMockMmTelFeature;
    private MmTelMediaQualityReporter mMmTelMediaQualityReporter;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mMmTelMediaQualityReporter = new MmTelMediaQualityReporter(mMockMmTelMediaRegistry,
                mMockMmTelFeature, CALL_ID);
    }

    @After
    public void tearDown() throws Exception {
        mMmTelMediaQualityReporter = null;
    }

    @Test
    public void testMmTelMediaQualityReporter() {
        // Verify {@link MmTelMediaQualityReporter#getMediaRegistry}
        assertEquals(mMockMmTelMediaRegistry, mMmTelMediaQualityReporter.getMediaRegistry());
    }
}
