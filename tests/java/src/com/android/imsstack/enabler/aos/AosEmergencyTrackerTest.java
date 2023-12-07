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

package com.android.imsstack.enabler.aos;

import static com.android.imsstack.base.TestAppContext.SLOT0;

import static org.mockito.Mockito.verify;

import com.android.imsstack.enabler.aos.IAosInfo.EmcCallbackMode;
import com.android.imsstack.enabler.aos.IAosInfo.EmcCallbackModeType;
import com.android.imsstack.enabler.aos.service.AosService;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class AosEmergencyTrackerTest {
    private AosEmergencyTracker mAosEmcTracker;

    @Mock private AosService mMockAosService;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        AosFactory.getInstance().mAosServices.put(SLOT0, mMockAosService);
        mAosEmcTracker = new AosEmergencyTracker(SLOT0);
    }

    @After
    public void cleanUp() throws Exception {
        AosFactory.getInstance().mAosServices.remove(SLOT0);
    }

    @Test
    public void notifyEmcCallStart() {
        mAosEmcTracker.updateEmcCallbackMode(
                EmcCallbackModeType.CALL, EmcCallbackMode.START, 10L);

        verify(mMockAosService).notifyEmcCallbackModeChanged(
                EmcCallbackModeType.CALL, EmcCallbackMode.START, 10L);
    }

    @Test
    public void notifyEmcCallStopByEmc() {
        mAosEmcTracker.updateEmcCallbackMode(
                EmcCallbackModeType.CALL, EmcCallbackMode.STOP_BY_EMC, 0L);

        verify(mMockAosService).notifyEmcCallbackModeChanged(
                EmcCallbackModeType.CALL, EmcCallbackMode.STOP_BY_EMC, 0L);
    }
}
