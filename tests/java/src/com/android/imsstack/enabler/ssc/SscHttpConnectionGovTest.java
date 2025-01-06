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

package com.android.imsstack.enabler.ssc;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telephony.CarrierConfigManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.HashMap;
import java.util.Map;

@RunWith(JUnit4.class)
public class SscHttpConnectionGovTest {
    private static final int SLOT_0 = 0;
    private static final int DEFAULT_HTTP_TRANSACTION_TIMEOUT_MS = 30 * 1000;

    ISscHttpConnectionGov mSscHttConnectionGov;

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private SscHttpConnection mMockSscHttpConnection;
    @Mock private Map<Integer, ISscHttpConnection> mMockSscHttpConnections;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        SscConfig.setConfigInterface(SLOT_0, mMockConfigInterface);

        mSscHttConnectionGov = SscHttpConnectionGov.getInstance();
        replaceInstance(SscHttpConnectionGov.class, "sSscHttpConnections", null,
                mMockSscHttpConnections);
    }

    @After
    public void teardown() {
        replaceInstance(SscHttpConnectionGov.class, "sSscHttpConnections", null,
                new HashMap<Integer, ISscHttpConnection>());
        SscConfig.clear(SLOT_0);
    }

    @Test
    @SmallTest
    public void open_withTcp() {
        when(mMockCarrierConfig.getInt(CarrierConfigManager.ImsSs.KEY_UT_TRANSPORT_TYPE_INT))
                .thenReturn(CarrierConfigManager.Ims.PREFERRED_TRANSPORT_TCP);

        mSscHttConnectionGov.open(SLOT_0, EApnType.XCAP);

        verify(mMockSscHttpConnections).put(eq(SLOT_0), any(SscHttpConnection.class));
    }

    @Test
    @SmallTest
    public void open_withTls() {
        when(mMockCarrierConfig.getInt(CarrierConfigManager.ImsSs.KEY_UT_TRANSPORT_TYPE_INT))
                .thenReturn(CarrierConfigManager.Ims.PREFERRED_TRANSPORT_TLS);

        mSscHttConnectionGov.open(SLOT_0, EApnType.XCAP);

        verify(mMockSscHttpConnections).put(eq(SLOT_0), any(SscHttpsConnection.class));
    }

    @Test
    @SmallTest
    public void open_closeConnectionBeforeRemoving() {
        when(mMockCarrierConfig.getInt(CarrierConfigManager.ImsSs.KEY_UT_TRANSPORT_TYPE_INT))
                .thenReturn(CarrierConfigManager.Ims.PREFERRED_TRANSPORT_TCP);
        when(mMockSscHttpConnections.get(SLOT_0)).thenReturn(mMockSscHttpConnection);

        mSscHttConnectionGov.open(SLOT_0, EApnType.XCAP);

        verify(mMockSscHttpConnection).close();
        verify(mMockSscHttpConnections).put(eq(SLOT_0), any(SscHttpConnection.class));
    }

    @Test
    @SmallTest
    public void close_normal() {
        when(mMockSscHttpConnections.get(SLOT_0)).thenReturn(mMockSscHttpConnection);

        mSscHttConnectionGov.close(SLOT_0);

        verify(mMockSscHttpConnection).close();
        verify(mMockSscHttpConnections).remove(SLOT_0);
    }

    @Test
    @SmallTest
    public void close_whenNotOpened() {
        when(mMockSscHttpConnections.get(SLOT_0)).thenReturn(null);

        mSscHttConnectionGov.close(SLOT_0);

        verify(mMockSscHttpConnection, never()).close();
        verify(mMockSscHttpConnections, never()).remove(SLOT_0);
    }

    @Test
    @SmallTest
    public void sendRequest_normal() {
        String requestUri = "xcap.root.uri";
        String xui = "3gpp.test.net";
        when(mMockSscHttpConnections.get(SLOT_0)).thenReturn(mMockSscHttpConnection);

        mSscHttConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET, requestUri,
                xui, null, DEFAULT_HTTP_TRANSACTION_TIMEOUT_MS);

        verify(mMockSscHttpConnection)
                .sendRequest(ISscHttpConnection.HTTP_REQUEST_GET, requestUri, xui, null,
                        DEFAULT_HTTP_TRANSACTION_TIMEOUT_MS);
    }

    @Test
    @SmallTest
    public void sendRequest_whenNotOpened() {
        String requestUri = "xcap.root.uri";
        String xui = "3gpp.test.net";
        when(mMockSscHttpConnections.get(SLOT_0)).thenReturn(null);

        mSscHttConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET, requestUri,
                xui, null, DEFAULT_HTTP_TRANSACTION_TIMEOUT_MS);

        verify(mMockSscHttpConnection, never())
                .sendRequest(ISscHttpConnection.HTTP_REQUEST_GET, requestUri, xui, null,
                        DEFAULT_HTTP_TRANSACTION_TIMEOUT_MS);
    }

    @Test
    @SmallTest
    public void getInputStream_normal() {
        when(mMockSscHttpConnections.get(SLOT_0)).thenReturn(mMockSscHttpConnection);

        mSscHttConnectionGov.getInputStream(SLOT_0);

        verify(mMockSscHttpConnection).getInputStream();
    }

    @Test
    @SmallTest
    public void getInputStream_whenNotOpened() {
        when(mMockSscHttpConnections.get(SLOT_0)).thenReturn(null);

        mSscHttConnectionGov.getInputStream(SLOT_0);

        verify(mMockSscHttpConnection, never()).getInputStream();
    }

    private synchronized void replaceInstance(final Class c, final String instanceName,
            final Object obj, final Object newValue) {
        try {
            java.lang.reflect.Field field = c.getDeclaredField(instanceName);
            field.setAccessible(true);
            field.set(obj, newValue);
        } catch (Exception e) {
            org.junit.Assert.fail(e.toString());
        }
    }
}
