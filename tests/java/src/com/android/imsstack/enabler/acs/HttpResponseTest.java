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

package com.android.imsstack.enabler.acs;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.enabler.acs.impl.HttpResponse;
import com.android.imsstack.enabler.acs.impl.HttpTransaction;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;

public class HttpResponseTest {
    private static final int SLOT_ID = 0;
    private HttpResponse mHttpResponse;

    @Mock
    HttpURLConnection mHttpURLConnection;
    @Mock
    InputStream mInputStream;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mHttpResponse = new HttpResponse(SLOT_ID, mHttpURLConnection);
    }

    @After
    public void tearDown() throws Exception {
        mHttpResponse = null;
    }

    @Test
    @SmallTest
    public void testGetResponse() throws Exception {
        // IOException is occurred during HttpURLConnection#getResponseCode, checking return
        // Internal error
        doThrow(IOException.class).when(mHttpURLConnection).getResponseCode();
        assertEquals(HttpTransaction.RESULT_TYPE_INTERNAL_ERROR, mHttpResponse.getResponseCode());

        // return any value
        doReturn(200).when(mHttpURLConnection).getResponseCode();
        assertEquals(200, mHttpResponse.getResponseCode());
        verify(mHttpURLConnection, times(2))
                .getResponseCode();
    }

    @Test
    @SmallTest
    public void testGetBody() throws Exception {
        // IOException is occurred during HttpURLConnection#getInputStream, checking return null
        doThrow(IOException.class).when(mHttpURLConnection).getInputStream();
        assertNull(mHttpResponse.getBody());

        // InputStream is null, checking return null
        doReturn(null).when(mHttpURLConnection).getInputStream();
        assertNull(mHttpResponse.getBody());

        doReturn(mInputStream).when(mHttpURLConnection).getInputStream();
        doReturn(false).when(mInputStream).markSupported();
        assertNull(mHttpResponse.getBody());

        doReturn(true).when(mInputStream).markSupported();
        doReturn(-1).when(mInputStream).read();
        assertNull(mHttpResponse.getBody());
    }
}
