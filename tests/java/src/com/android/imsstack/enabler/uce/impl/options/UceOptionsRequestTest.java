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
import com.android.imsstack.enabler.uce.impl.utils.UceUtils;
import com.android.imsstack.enabler.uce.interf.OptionsResponse;
import com.android.imsstack.enabler.uce.interf.UceApiConstant;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.List;
import java.util.Set;

@RunWith(JUnit4.class)
public class UceOptionsRequestTest {
    private static final int SLOT_ID = 0;
    private static final int KEY = 10;
    @Mock OptionsResponse optionsCb;
    @Mock UceJNI jni;
    private UceOptionsRequest mRequest;

    @Before
    public void setUp(){
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void cleanUp() {
        mRequest = null;
    }

    @Test
    @SmallTest
    public void test_sendRequestWithEmptyUri() throws Exception {
        String remoteUri = "";
        Set<String> myCapabilities = new ArraySet<>();

        mRequest = createUceOptionsRequest();

        // verify that send command error if the remote uri is empty
        mRequest.sendRequest(remoteUri, myCapabilities);
        verify(optionsCb).onCommandError(eq(UceApiConstant.COMMAND_CODE_INVALID_PARAM));
        verifyNoMoreInteractions(optionsCb);
    }

    @Test
    @SmallTest
    public void test_sendRequestWithEmptyCapabilities() throws Exception {
        String remoteUri = "test";
        Set<String> myCapabilities = new ArraySet<>();

        mRequest = createUceOptionsRequest();

        // verify that send command error if the myCapabilities is empty
        mRequest.sendRequest(remoteUri, myCapabilities);
        verify(optionsCb).onCommandError(eq(UceApiConstant.COMMAND_CODE_INVALID_PARAM));
        verifyNoMoreInteractions(optionsCb);
    }

    @Test
    @SmallTest
    public void test_normalSendRequest() throws Exception {
        String remoteUri = "test";
        Set<String> myCapabilities = new ArraySet<>();
        myCapabilities.add(UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getTag());

        mRequest = createUceOptionsRequest();

        mRequest.sendRequest(remoteUri, myCapabilities);

        ArgumentCaptor<Parcel> captor = ArgumentCaptor.forClass(Parcel.class);

        verify(jni, times(1)).sendMessage(eq(SLOT_ID), captor.capture());

        Parcel parcel = captor.getValue();
        parcel.setDataPosition(0);
        assertEquals(parcel.readInt(), UceMessage.UCE_SEND_OPTIONS_CMD);
        assertEquals(parcel.readInt(), KEY);
        assertEquals(parcel.readString(), remoteUri);
        assertEquals(parcel.readLong(), UceUtils.getCapabilities(myCapabilities));

        verifyNoMoreInteractions(jni);
    }

    @Test
    @SmallTest
    public void test_informResponse() throws Exception {
        int responseCode = 200;
        String reason = "OK";
        Set<String> capabilities = new ArraySet<>();
        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getTag());
        capabilities.add(UceFeatureTags.Tags.FEATURE_TAG_CPM_CHAT.getTag());
        long capability = UceUtils.getCapabilities(capabilities);

        mRequest = createUceOptionsRequest();
        mRequest.informNetworkResponse(responseCode, reason, capability);

        ArgumentCaptor<List> captor = ArgumentCaptor.forClass(List.class);

        verify(optionsCb, times(1)).onNetworkResponse(eq(responseCode), eq(reason),
                captor.capture());
        List<String> data = captor.getValue();
        assertEquals(data.size(), 2);
        assertEquals(data.get(0), UceFeatureTags.Tags.FEATURE_TAG_PRESENCE.getTag());
        assertEquals(data.get(1), UceFeatureTags.Tags.FEATURE_TAG_CPM_CHAT.getTag());
    }

    @Test
    @SmallTest
    public void informCommandError() throws Exception {
        int commandError = 6;

        mRequest = createUceOptionsRequest();
        mRequest.informCommandError(commandError);

        verify(optionsCb, times(1)).onCommandError(eq(commandError));
    }

    private UceOptionsRequest createUceOptionsRequest() {
        return new UceOptionsRequest(optionsCb, SLOT_ID, KEY, jni);
    }
}
