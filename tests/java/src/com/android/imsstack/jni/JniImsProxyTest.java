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

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import androidx.test.filters.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class JniImsProxyTest {
    private static final long NATIVE_OBJECT = 100L;
    private static final byte[] DATA = new byte[] { (byte) 0 };

    @Mock JniImsListener mListener;
    @Mock JniSystemListener mSystemListener;
    @Mock JniIms mJniIms;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        JniImsProxy.setJniIms(mJniIms);
    }

    @After
    public void tearDown() throws Exception {
        JniImsProxy.setJniIms(null);
    }

    @Test
    @SmallTest
    public void init() throws Exception {
        JniImsProxy.init();
        verify(mJniIms).init();
    }

    @Test
    @SmallTest
    public void deinit() throws Exception {
        JniImsProxy.deinit();
        verify(mJniIms).deinit();
    }

    @Test
    @SmallTest
    public void getInterface() throws Exception {
        when(mJniIms.getInterface(anyInt(), anyInt())).thenReturn(NATIVE_OBJECT);

        long nativeObject = JniImsProxy.getInterface(1, 0);

        verify(mJniIms).getInterface(eq(1), eq(0));
        assertEquals(NATIVE_OBJECT, nativeObject);
    }

    @Test
    @SmallTest
    public void releaseInterface() throws Exception {
        JniImsProxy.releaseInterface(NATIVE_OBJECT);
        verify(mJniIms).releaseInterface(eq(NATIVE_OBJECT));
    }

    @Test
    @SmallTest
    public void sendData() throws Exception {
        when(mJniIms.sendData(eq(NATIVE_OBJECT), any())).thenReturn(JniImsProxy.OK);

        int result = JniImsProxy.sendData(NATIVE_OBJECT, DATA);

        verify(mJniIms).sendData(eq(NATIVE_OBJECT), eq(DATA));
        assertEquals(JniImsProxy.OK, result);
    }

    @Test
    @SmallTest
    public void sendDataForSystem() throws Exception {
        when(mJniIms.sendDataForSystem(eq(NATIVE_OBJECT), any())).thenReturn(DATA);

        byte[] result = JniImsProxy.sendDataForSystem(NATIVE_OBJECT, DATA);

        verify(mJniIms).sendDataForSystem(eq(NATIVE_OBJECT), eq(DATA));
        assertEquals(DATA, result);
    }

    @Test
    @SmallTest
    public void sendCommand() throws Exception {
        when(mJniIms.sendCommand(anyInt(), anyInt(), any())).thenReturn(JniImsProxy.OK);

        int result = JniImsProxy.sendCommand(1, 0, DATA);

        verify(mJniIms).sendCommand(eq(1), eq(0), eq(DATA));
        assertEquals(JniImsProxy.OK, result);
    }

    @Test
    @SmallTest
    public void setListener() throws Exception {
        when(mJniIms.setListener(eq(NATIVE_OBJECT), any())).thenReturn(JniImsProxy.OK);

        int result = JniImsProxy.setListener(NATIVE_OBJECT, mListener);

        verify(mJniIms).setListener(eq(NATIVE_OBJECT), eq(mListener));
        assertEquals(JniImsProxy.OK, result);
    }

    @Test
    @SmallTest
    public void getListener() throws Exception {
        when(mJniIms.getListener(eq(NATIVE_OBJECT))).thenReturn(mListener);

        JniImsListener listener = JniImsProxy.getListener(NATIVE_OBJECT);

        verify(mJniIms).getListener(eq(NATIVE_OBJECT));
        assertEquals(listener, mListener);
    }

    @Test
    @SmallTest
    public void removeListener() throws Exception {
        when(mJniIms.removeListener(eq(NATIVE_OBJECT))).thenReturn(JniImsProxy.OK);

        int result = JniImsProxy.removeListener(NATIVE_OBJECT);

        verify(mJniIms).removeListener(eq(NATIVE_OBJECT));
        assertEquals(JniImsProxy.OK, result);
    }

    @Test
    @SmallTest
    public void setSystemListener() throws Exception {
        when(mJniIms.setSystemListener(eq(NATIVE_OBJECT), any())).thenReturn(JniImsProxy.OK);

        int result = JniImsProxy.setSystemListener(NATIVE_OBJECT, mSystemListener);

        verify(mJniIms).setSystemListener(eq(NATIVE_OBJECT), eq(mSystemListener));
        assertEquals(JniImsProxy.OK, result);
    }

    @Test
    @SmallTest
    public void getSystemListener() throws Exception {
        when(mJniIms.getSystemListener(eq(NATIVE_OBJECT))).thenReturn(mSystemListener);

        JniSystemListener listener = JniImsProxy.getSystemListener(NATIVE_OBJECT);

        verify(mJniIms).getSystemListener(eq(NATIVE_OBJECT));
        assertEquals(listener, mSystemListener);
    }

    @Test
    @SmallTest
    public void removeSystemListener() throws Exception {
        when(mJniIms.removeSystemListener(eq(NATIVE_OBJECT))).thenReturn(JniImsProxy.OK);

        int result = JniImsProxy.removeSystemListener(NATIVE_OBJECT);

        verify(mJniIms).removeSystemListener(eq(NATIVE_OBJECT));
        assertEquals(JniImsProxy.OK, result);
    }
}
