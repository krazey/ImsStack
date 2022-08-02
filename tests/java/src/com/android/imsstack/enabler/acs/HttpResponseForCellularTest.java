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

import static com.android.imsstack.enabler.acs.impl.HttpTransaction.REQUEST_DONE;
import static com.android.imsstack.enabler.acs.impl.HttpTransaction.REQUEST_HTTPS;
import static com.android.imsstack.enabler.acs.impl.HttpTransaction.REQUEST_NON_CELLULAR;
import static com.android.imsstack.enabler.acs.impl.HttpTransaction.RESULT_TYPE_INTERNAL_ERROR;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.mockito.Mockito.anyLong;
import static org.mockito.Mockito.atLeast;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.os.Handler;
import android.os.Message;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.enabler.acs.impl.HttpResponse;
import com.android.imsstack.enabler.acs.impl.HttpResponseForCellular;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.List;

public class HttpResponseForCellularTest {
    private static final int SLOT_ID = 0;
    private static final String AC_DATA_PARTIAL = "<?xml version=\"1.0\"?>"
            + "<wap-provisioningdoc version=\"1.1\">"
            + "<characteristic type=\"VERS\">"
            + "<parm name=\"version\" value=\"3\"/>"
            + "<parm name=\"validity\" value=\"3600000\"/>"
            + "</characteristic>"
            + "<characteristic type=\"TOKEN\">"
            + "<parm name=\"token\" value=\"qazwsxedc12345\"/>"
            + "</characteristic>"
            + "</wap-provisioningdoc>";

    private HttpResponseForCellular mHttpResponseForCellular;

    @Mock
    Handler mHandler;
    @Mock
    HttpResponse mHttpResponse;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mHttpResponseForCellular = new HttpResponseForCellular(mHandler, SLOT_ID);

        // default : return 200
        doReturn(200).when(mHttpResponse).getResponseCode();
        doReturn(getCookies()).when(mHttpResponse).getCookies();
        doReturn(AC_DATA_PARTIAL.getBytes()).when(mHttpResponse).getBody();
    }

    @After
    public void tearDown() throws Exception {
        mHttpResponseForCellular = null;
    }

    @Test
    @SmallTest
    public void testHandle() throws Exception {
        // HttpResponse is null
        mHttpResponseForCellular.handle(null);

        // msg what: REQUEST_DONE, arg1: RESULT_TYPE_INTERNAL_ERROR
        ArgumentCaptor<Message> captor = ArgumentCaptor.forClass(Message.class);
        verify(mHandler, times(1))
                .sendMessageAtTime(captor.capture(), anyLong());
        assertEquals(REQUEST_DONE, captor.getValue().what);
        assertEquals(RESULT_TYPE_INTERNAL_ERROR, captor.getValue().arg1);
        verifyNoMoreInteractions(mHandler);

        // check getResponseCode is called
        mHttpResponseForCellular.handle(mHttpResponse);
        verify(mHttpResponse, times(2)).getResponseCode();
    }

    @Test
    @SmallTest
    public void testHandle200OkResponse() throws Exception {
        // case : 200 ok + xml body(ac data)
        mHttpResponseForCellular.handle(mHttpResponse);

        // msg what: REQUEST_DONE, arg1: 200, obj : AC_DATA_PARTIAL
        ArgumentCaptor<Message> captor = ArgumentCaptor.forClass(Message.class);
        verify(mHandler, times(1))
                .sendMessageAtTime(captor.capture(), anyLong());
        assertEquals(REQUEST_DONE, captor.getValue().what);
        assertEquals(200, captor.getValue().arg1);
        assertEquals(AC_DATA_PARTIAL.length(), ((byte[]) captor.getValue().obj).length);

        verifyNoMoreInteractions(mHandler);

        // case : 200 ok + cookie
        doReturn(null).when(mHttpResponse).getBody();
        mHttpResponseForCellular.handle(mHttpResponse);

        // msg what: REQUEST_HTTPS
        verify(mHandler, times(2))
                .sendMessageAtTime(captor.capture(), anyLong());
        assertEquals(REQUEST_HTTPS, captor.getValue().what);
        verifyNoMoreInteractions(mHandler);

        // case : 200 ok + no HTTP body
        mHttpResponseForCellular.handle(mHttpResponse);

        // msg what: REQUEST_DONE, arg1: RESULT_TYPE_INTERNAL_ERROR
        verify(mHandler, times(3))
                .sendMessageAtTime(captor.capture(), anyLong());
        assertEquals(REQUEST_DONE, captor.getValue().what);
        assertEquals(RESULT_TYPE_INTERNAL_ERROR, captor.getValue().arg1);
        assertNull(captor.getValue().obj);

        verifyNoMoreInteractions(mHandler);

        // case : 200 ok + no cookie
        doReturn(null).when(mHttpResponse).getCookies();
        mHttpResponseForCellular.handle(mHttpResponse);

        // msg what: REQUEST_DONE, arg1: RESULT_TYPE_INTERNAL_ERROR
        verify(mHandler, times(4))
                .sendMessageAtTime(captor.capture(), anyLong());
        assertEquals(REQUEST_DONE, captor.getValue().what);
        assertEquals(RESULT_TYPE_INTERNAL_ERROR, captor.getValue().arg1);

        verify(mHttpResponse, times(4)).getBody();
        verify(mHttpResponse, times(3)).getCookies();
        verifyNoMoreInteractions(mHandler);
    }

    @Test
    @SmallTest
    public void testHandle500Response() throws Exception {
        // received 500
        doReturn(500).when(mHttpResponse).getResponseCode();
        mHttpResponseForCellular.handle(mHttpResponse);

        // msg what: REQUEST_DONE, arg1: 500
        ArgumentCaptor<Message> captor = ArgumentCaptor.forClass(Message.class);
        verify(mHandler, atLeast(1))
                .sendMessageAtTime(captor.capture(), anyLong());
        assertEquals(REQUEST_DONE, captor.getValue().what);
        assertEquals(500, captor.getValue().arg1);

        verifyNoMoreInteractions(mHandler);
    }

    @Test
    @SmallTest
    public void testHandle511Response() throws Exception {
        // exist cookie header & received 511
        doReturn(511).when(mHttpResponse).getResponseCode();
        mHttpResponseForCellular.handle(mHttpResponse);
        List<String> cookies = getCookies();
        assertEquals(cookies.get(0), mHttpResponseForCellular.getCookies().get(0));
        assertEquals(cookies, mHttpResponseForCellular.getCookies());

        // msg what: REQUEST_NON_CELLULAR
        ArgumentCaptor<Message> captor = ArgumentCaptor.forClass(Message.class);
        verify(mHandler, times(1))
                .sendMessageAtTime(captor.capture(), anyLong());
        assertEquals(REQUEST_NON_CELLULAR, captor.getValue().what);

        verifyNoMoreInteractions(mHandler);

        // no cookie header & received 511
        doReturn(null).when(mHttpResponse).getCookies();
        mHttpResponseForCellular.handle(mHttpResponse);

        // msg what: REQUEST_NON_CELLULAR
        verify(mHandler, times(2))
                .sendMessageAtTime(captor.capture(), anyLong());
        assertEquals(REQUEST_NON_CELLULAR, captor.getValue().what);

        verify(mHttpResponse, times(2)).getCookies();
        verifyNoMoreInteractions(mHandler);
    }

    private List<String> getCookies() {
        List<String> cookies = new ArrayList<>();
        cookies.add("abcd");
        cookies.add("efgh");
        return cookies;
    }
}
