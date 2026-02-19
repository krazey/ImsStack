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
package com.android.imsstack.enabler.uce.impl.options;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.os.Parcel;
import android.util.ArraySet;

import androidx.test.filters.SmallTest;

import com.android.imsstack.enabler.uce.impl.define.UceFeatureTags;
import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.Set;

@RunWith(JUnit4.class)
public class UceOptionsResponseCallbackTest {
    private static final int SLOT_ID = 0;
    private static final int KEY = 10;
    @Mock UceJNI mJni;
    private UceOptionsResponseCallback mCallback;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void cleanUp() {
        mCallback = null;
    }

    @Test
    @SmallTest
    public void test_onRespondToCapabilityRequestWithBlocked() throws Exception {
        Set<String> ownCapabilities = new ArraySet<>();
        ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getTag());
        ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VOICE.getTag());
        boolean isBlocked = true;

        mCallback = createUceOptionsResponseCallback();

        mCallback.onRespondToCapabilityRequest(ownCapabilities, isBlocked);

        ArgumentCaptor<Parcel> captor = ArgumentCaptor.forClass(Parcel.class);
        verify(mJni, times(1)).sendMessage(eq(SLOT_ID), captor.capture());

        Parcel parcel = captor.getValue();
        parcel.setDataPosition(0);
        assertEquals(UceMessage.UCE_SEND_OPTIONS_RESP_CMD, parcel.readInt());
        assertEquals(KEY, parcel.readInt());
        assertEquals(200, parcel.readInt());
        assertEquals("", parcel.readString());
        assertEquals(0, parcel.readLong());
        verifyNoMoreInteractions(mJni);
    }

    @Test
    @SmallTest
    public void test_onRespondToCapabilityRequestWithEmptyCapabilities() throws Exception {
        Set<String> ownCapabilities = new ArraySet<>();
        boolean isBlocked = false;

        mCallback = createUceOptionsResponseCallback();

        mCallback.onRespondToCapabilityRequest(ownCapabilities, isBlocked);

        ArgumentCaptor<Parcel> captor = ArgumentCaptor.forClass(Parcel.class);
        verify(mJni, times(1)).sendMessage(eq(SLOT_ID), captor.capture());

        Parcel parcel = captor.getValue();
        parcel.setDataPosition(0);
        assertEquals(UceMessage.UCE_SEND_OPTIONS_RESP_CMD, parcel.readInt());
        assertEquals(KEY, parcel.readInt());
        assertEquals(200, parcel.readInt());
        assertEquals("", parcel.readString());
        assertEquals(0, parcel.readLong());
        verifyNoMoreInteractions(mJni);
    }

    @Test
    @SmallTest
    public void test_onRespondToCapabilityRequest() throws Exception {
        long featureTags = 0L;
        Set<String> ownCapabilities = new ArraySet<>();
        ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getTag());
        featureTags |= UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getCapa();

        ownCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VOICE.getTag());
        featureTags |= UceFeatureTags.Tags.FEATURE_TAG_IPCALL_VOICE.getCapa();

        boolean isBlocked = false;

        mCallback = createUceOptionsResponseCallback();

        mCallback.onRespondToCapabilityRequest(ownCapabilities, isBlocked);

        ArgumentCaptor<Parcel> captor = ArgumentCaptor.forClass(Parcel.class);
        verify(mJni, times(1)).sendMessage(eq(SLOT_ID), captor.capture());

        Parcel parcel = captor.getValue();
        parcel.setDataPosition(0);
        assertEquals(UceMessage.UCE_SEND_OPTIONS_RESP_CMD, parcel.readInt());
        assertEquals(KEY, parcel.readInt());
        assertEquals(200, parcel.readInt());
        assertEquals("", parcel.readString());
        assertEquals(featureTags, parcel.readLong());
        verifyNoMoreInteractions(mJni);
    }

    @Test
    @SmallTest
    public void onRespondToCapabilityRequestWithError() throws Exception {
        int code = 100;
        String reason = "TEST";

        mCallback = createUceOptionsResponseCallback();

        mCallback.onRespondToCapabilityRequestWithError(code, reason);

        ArgumentCaptor<Parcel> captor = ArgumentCaptor.forClass(Parcel.class);
        verify(mJni, times(1)).sendMessage(eq(SLOT_ID), captor.capture());

        Parcel parcel = captor.getValue();
        parcel.setDataPosition(0);
        assertEquals(UceMessage.UCE_SEND_OPTIONS_RESP_CMD, parcel.readInt());
        assertEquals(KEY, parcel.readInt());
        assertEquals(code, parcel.readInt());
        assertEquals(reason, parcel.readString());
        assertEquals(0, parcel.readLong());
        verifyNoMoreInteractions(mJni);
    }

    private UceOptionsResponseCallback createUceOptionsResponseCallback() {
        return new UceOptionsResponseCallback(KEY, SLOT_ID, mJni);
    }
}
