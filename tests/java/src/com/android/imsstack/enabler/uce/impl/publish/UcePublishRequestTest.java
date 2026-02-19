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
package com.android.imsstack.enabler.uce.impl.publish;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.os.Parcel;

import androidx.test.filters.SmallTest;

import com.android.imsstack.core.agents.PreferenceInterface;
import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;
import com.android.imsstack.enabler.uce.interf.PublishResponse;
import com.android.imsstack.enabler.uce.interf.UceApiConstant;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class UcePublishRequestTest {
    private static final int SLOT_ID = 0;
    private static final int KEY = 10;
    private static final boolean USED_EXPIRED_ETAG = true;
    private static final boolean NOT_USED_EXPIRED_ETAG = false;
    @Mock PublishResponse publishCb;
    @Mock UceJNI jni;
    @Mock PreferenceInterface mPf;

    private UcePublishRequest mRequest;
    private UcePublishRequest mResponseWithEtag;
    private UcePublishRequest mResponseWithoutEtag;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void cleanUp() {
        mRequest = null;
        mResponseWithEtag = null;
        mResponseWithoutEtag = null;
    }

    @Test
    @SmallTest
    public void test_create() throws Exception {
        String eTag = "ABCDFEG";
        mResponseWithoutEtag = createUcePublishRequestEmptyPf(eTag);
        assertEquals(eTag, mResponseWithoutEtag.getEtag());
        assertEquals(KEY, mResponseWithoutEtag.getKey());

        String savedEtag = "savedEtag";
        doReturn(savedEtag).when(mPf).getString(any(), eq(SLOT_ID));
        mResponseWithEtag = createUcePublishRequestPf(eTag);
        assertEquals(savedEtag, mResponseWithEtag.getEtag());
    }

    @Test
    @SmallTest
    public void test_sendRequestWithEmptyPidf() throws Exception {
        String pidfXml = "";
        long capability = 150;

        mRequest = createUcePublishRequest("");
        // verify that send command error if the pidfxml is empty
        mRequest.setRequestInfo(pidfXml, false, capability);
        mRequest.sendRequest();
        verify(publishCb).onCommandError(eq(UceApiConstant.COMMAND_CODE_INVALID_PARAM));
        verifyNoMoreInteractions(jni);
    }

    @Test
    @SmallTest
    public void test_sendRequestWithEmptyEtag() throws Exception {
        String pidfXml = "test pidf";
        long capability = 150;

        // verify that the input data be passed to the JNI
        ArgumentCaptor<Parcel> captor = ArgumentCaptor.forClass(Parcel.class);

        mRequest = createUcePublishRequest("");
        mRequest.setRequestInfo(pidfXml, false, capability);
        mRequest.sendRequest();
        verify(jni, times(1)).sendMessage(eq(SLOT_ID), captor.capture());

        Parcel parcel = captor.getValue();
        parcel.setDataPosition(0);
        assertEquals(UceMessage.UCE_SEND_PUBLISH_CMD, parcel.readInt());
        assertEquals(KEY, parcel.readInt());
        assertEquals(pidfXml, parcel.readString()); // pidf xml
        assertEquals(0, parcel.readInt()); // extend
        assertEquals(capability, parcel.readLong()); // capability
        assertEquals(0, parcel.readInt()); // no etag

        verifyNoMoreInteractions(jni);
    }

    @Test
    @SmallTest
    public void test_sendRequest() throws Exception {
        String pidfXml = "test pidf";
        String eTag = "ABC123ADB";
        long capability = 150;

        // verify that the input data be passed to the JNI
        ArgumentCaptor<Parcel> captor = ArgumentCaptor.forClass(Parcel.class);

        mRequest = createUcePublishRequest(eTag);
        mRequest.setRequestInfo(pidfXml, true, capability);
        mRequest.sendRequest();
        verify(jni, times(1)).sendMessage(eq(SLOT_ID), captor.capture());

        Parcel parcel = captor.getValue();
        parcel.setDataPosition(0);
        assertEquals(UceMessage.UCE_SEND_PUBLISH_CMD, parcel.readInt());
        assertEquals(KEY, parcel.readInt());
        assertEquals(pidfXml, parcel.readString()); // pidf xml
        assertEquals(1, parcel.readInt()); // extend
        assertEquals(capability, parcel.readLong()); // capability
        assertEquals(1, parcel.readInt()); // etag
        assertEquals(eTag, parcel.readString()); // etag value

        verifyNoMoreInteractions(jni);
    }

    @Test
    @SmallTest
    public void test_informResponse() throws Exception {
        int responseCode = 200;
        String reason = "OK";
        int reasonHdrCause = 0;
        String reasonHdrText = "test reason text";
        String eTag = "";

        mResponseWithEtag = createUcePublishRequest("");
        mResponseWithEtag.informNetworkResponse(responseCode, reason, reasonHdrCause,
                reasonHdrText, eTag);

        verify(publishCb, times(1)).onNetworkResponse(eq(responseCode), eq(reason));

        reasonHdrCause = 10;
        eTag = "ABC";
        mResponseWithoutEtag = createUcePublishRequest(eTag);
        mResponseWithoutEtag.informNetworkResponse(responseCode, reason, reasonHdrCause,
                reasonHdrText, eTag);
        verify(publishCb, times(1)).onNetworkResponse(eq(responseCode), eq(reason),
                eq(reasonHdrCause), eq(reasonHdrText));
    }

    @Test
    @SmallTest
    public void test_informCommandCode() throws Exception {
        int commandCode = 200;

        mRequest = createUcePublishRequest("");
        mRequest.informCommandError(commandCode);

        verify(publishCb, times(1)).onCommandError(eq(commandCode));
    }

    private UcePublishRequest createUcePublishRequest(String eTag) {
        return new UcePublishRequest(publishCb, SLOT_ID, KEY,
                NOT_USED_EXPIRED_ETAG, jni, eTag, mPf);
    }

    private UcePublishRequest createUcePublishRequestEmptyPf(String eTag) {
        return new UcePublishRequest(publishCb, SLOT_ID, KEY,
                USED_EXPIRED_ETAG, jni, eTag, null);
    }

    private UcePublishRequest createUcePublishRequestPf(String eTag) {
        return new UcePublishRequest(publishCb, SLOT_ID, KEY,
                USED_EXPIRED_ETAG, jni, eTag, mPf);
    }
}
