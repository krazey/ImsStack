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
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.os.Parcel;
import android.test.suitebuilder.annotation.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.Arrays;

@RunWith(JUnit4.class)
public class JniImsTest {
    private static final long NATIVE_OBJECT = 100L;
    private static final long SYSTEM_NATIVE_OBJECT = 200L;

    @Mock JniImsListener mListener;
    @Mock JniSystemListener mSystemListener;
    private JniIms mJniIms;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mJniIms = new JniIms();
    }

    @After
    public void tearDown() throws Exception {
        mJniIms = null;
    }

    @Test
    @SmallTest
    public void setListener() throws Exception {
        assertEquals(JniIms.ERROR, mJniIms.setListener(0, mListener));
        assertEquals(JniIms.ERROR, mJniIms.setListener(NATIVE_OBJECT, null));
        assertEquals(JniIms.OK, mJniIms.setListener(NATIVE_OBJECT, mListener));
        assertEquals(mListener, mJniIms.getListener(NATIVE_OBJECT));
    }

    @Test
    @SmallTest
    public void getListener() throws Exception {
        assertNull(mJniIms.getListener(0));
        assertNull(mJniIms.getListener(NATIVE_OBJECT));

        mJniIms.setListener(NATIVE_OBJECT, mListener);

        assertEquals(mListener, mJniIms.getListener(NATIVE_OBJECT));

        mJniIms.removeListener(NATIVE_OBJECT);

        assertNull(mJniIms.getListener(NATIVE_OBJECT));
    }

    @Test
    @SmallTest
    public void removeListener() throws Exception {
        assertEquals(JniIms.ERROR, mJniIms.removeListener(0));
        assertEquals(JniIms.OK, mJniIms.removeListener(NATIVE_OBJECT));

        mJniIms.setListener(NATIVE_OBJECT, mListener);

        assertEquals(JniIms.OK, mJniIms.removeListener(NATIVE_OBJECT));
        assertNull(mJniIms.getListener(NATIVE_OBJECT));
    }

    @Test
    @SmallTest
    public void setSystemListener() throws Exception {
        assertEquals(JniIms.ERROR, mJniIms.setSystemListener(0, mSystemListener));
        assertEquals(JniIms.ERROR, mJniIms.setSystemListener(SYSTEM_NATIVE_OBJECT, null));
        assertEquals(JniIms.OK, mJniIms.setSystemListener(SYSTEM_NATIVE_OBJECT, mSystemListener));
        assertEquals(mSystemListener, mJniIms.getSystemListener(SYSTEM_NATIVE_OBJECT));
    }

    @Test
    @SmallTest
    public void getSystemListener() throws Exception {
        assertNull(mJniIms.getSystemListener(0));
        assertNull(mJniIms.getSystemListener(SYSTEM_NATIVE_OBJECT));

        mJniIms.setSystemListener(SYSTEM_NATIVE_OBJECT, mSystemListener);

        assertEquals(mSystemListener, mJniIms.getSystemListener(SYSTEM_NATIVE_OBJECT));

        mJniIms.removeSystemListener(SYSTEM_NATIVE_OBJECT);

        assertNull(mJniIms.getSystemListener(SYSTEM_NATIVE_OBJECT));
    }

    @Test
    @SmallTest
    public void removeSystemListener() throws Exception {
        assertEquals(JniIms.ERROR, mJniIms.removeSystemListener(0));
        assertEquals(JniIms.OK, mJniIms.removeSystemListener(SYSTEM_NATIVE_OBJECT));

        mJniIms.setSystemListener(SYSTEM_NATIVE_OBJECT, mSystemListener);

        assertEquals(JniIms.OK, mJniIms.removeSystemListener(SYSTEM_NATIVE_OBJECT));
        assertNull(mJniIms.getSystemListener(SYSTEM_NATIVE_OBJECT));
    }

    @Test
    @SmallTest
    public void sendDataToJava() throws Exception {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(1);
        parcel.writeInt(0);
        byte[] data = parcel.marshall();
        parcel.recycle();

        assertEquals(JniIms.ERROR_NO_LISTENER, JniIms.sendDataToJava(NATIVE_OBJECT, data));

        mJniIms.setListener(NATIVE_OBJECT, mListener);
        JniIms.sendDataToJava(NATIVE_OBJECT, data);

        verify(mListener).onMessage(any(Parcel.class));

        mJniIms.removeListener(NATIVE_OBJECT);
        JniIms.sendDataToJava(NATIVE_OBJECT, data);

        verifyNoMoreInteractions(mListener);
    }

    @Test
    @SmallTest
    public void sendDataToJavaForSystem() throws Exception {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(1);
        parcel.writeInt(0);
        byte[] data = parcel.marshall();
        parcel.recycle();

        byte[] result = JniIms.sendDataToJavaForSystem(SYSTEM_NATIVE_OBJECT, data, null);
        assertTrue(Arrays.equals(result, new byte[] {(byte) 0}));

        mJniIms.setSystemListener(SYSTEM_NATIVE_OBJECT, mSystemListener);
        JniIms.sendDataToJavaForSystem(SYSTEM_NATIVE_OBJECT, data, null);

        verify(mSystemListener).onMessage(any(Parcel.class), any());

        mJniIms.removeSystemListener(SYSTEM_NATIVE_OBJECT);
        JniIms.sendDataToJavaForSystem(SYSTEM_NATIVE_OBJECT, data, null);

        verifyNoMoreInteractions(mSystemListener);
    }
}
