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
import static org.mockito.Mockito.anyLong;
import static org.mockito.Mockito.atLeast;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.verify;

import android.os.Handler;
import android.os.Message;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.enabler.acs.impl.HttpResponse;
import com.android.imsstack.enabler.acs.impl.HttpResponseForCellular;
import com.android.imsstack.enabler.acs.impl.HttpTransaction;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

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
    HttpURLConnection mHttpURLConnection;
    @Mock
    Handler mHandler;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mHttpResponseForCellular = new HttpResponseForCellular(mHandler, SLOT_ID,
                mHttpURLConnection);
    }

    @After
    public void tearDown() throws Exception {
        mHttpResponseForCellular = null;
    }

    @Test
    @SmallTest
    public void testHandle200OkResponse() throws Exception {
        // received 200 ok + cookie
        setCookies(200);
        HttpResponse response = new HttpResponse(SLOT_ID, mHttpURLConnection);

        mHttpResponseForCellular.handle(response);
        List<String> cookies = getCookies();
        assertEquals(cookies.get(0), mHttpResponseForCellular.getCookies().get(0));
        assertEquals(cookies, mHttpResponseForCellular.getCookies());

        // check send next progress message
        ArgumentCaptor<Message> messageArgumentCaptor = ArgumentCaptor.forClass(Message.class);
        verify(mHandler, atLeast(1))
                .sendMessageAtTime(messageArgumentCaptor.capture(), anyLong());
        assertEquals(HttpTransaction.REQUEST_HTTPS, messageArgumentCaptor.getValue().what);

        // received 200 ok + xml body(ac data)
        byte[] acData = AC_DATA_PARTIAL.getBytes();
        InputStream inputStream = new ByteArrayInputStream(acData);
        doReturn(inputStream).when(mHttpURLConnection).getInputStream();

        mHttpResponseForCellular.handle(response);
        verify(mHandler, atLeast(1))
                .sendMessageAtTime(messageArgumentCaptor.capture(), anyLong());
        assertEquals(HttpTransaction.REQUEST_DONE, messageArgumentCaptor.getValue().what);
        assertEquals(acData.length, ((byte[]) messageArgumentCaptor.getValue().obj).length);
    }

    private void setCookies(int responseCode) throws Exception {
        doReturn(responseCode).when(mHttpURLConnection).getResponseCode();
        List<String> cookies = getCookies();
        Map<String, List<String>> headers = new HashMap<>();
        headers.put("Set-Cookie", cookies);
        doReturn(headers).when(mHttpURLConnection).getHeaderFields();
    }

    private List<String> getCookies() {
        List<String> cookies = new ArrayList<>();
        cookies.add("abcd");
        cookies.add("efgh");
        return cookies;
    }
}
