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
package com.android.imsstack.jni;

import static com.android.imsstack.base.TestAppContext.SLOT0;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class NativeCommandsTest {
    @Mock private JniIms mJniIms;

    private Context mContext;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContext = new ContextFixture().getTestDouble();
        when(mContext.getResources().getBoolean(anyInt())).thenReturn(true);
        JniImsProxy.setJniIms(mJniIms);
    }

    @After
    public void tearDown() throws Exception {
        JniImsProxy.setJniIms(null);
        mContext = null;
    }

    @Test
    @SmallTest
    public void setDeviceConfig() {
        NativeCommands.setDeviceConfig(mContext);
        verify(mJniIms).sendCommand(eq(NativeCommands.CMD_SET_DEVICE_CONFIG), anyInt(), any());
    }

    @Test
    @SmallTest
    public void startEnabler() {
        NativeCommands.startEnabler(SLOT0);
        verify(mJniIms).sendCommand(eq(NativeCommands.CMD_START_ENABLER), eq(SLOT0), any());
    }

    @Test
    @SmallTest
    public void stopEnabler() {
        NativeCommands.stopEnabler(SLOT0);
        verify(mJniIms).sendCommand(eq(NativeCommands.CMD_STOP_ENABLER), eq(SLOT0), any());
    }
}
