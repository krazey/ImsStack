/*
 * Copyright (C) 2023 The Android Open Source Project
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

package com.android.imsstack.core.agents.dcm;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.TelephonyNetworkSpecifier;
import android.os.Handler;
import android.os.Message;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class DefaultNetworkCallbackTest extends ImsStackTest {
    private static final int SLOT_0 = 0;
    private int mSubId = -1;
    private DummyHandler mDummyHandler;
    private DefaultNetworkCallback mDefaultNetworkCallback = null;

    @Mock private Network mMockNetwork;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);
        AppContext.init(mContext);

        mSubId = MSimUtils.getSubId(SLOT_0);

        // create the instance to test
        mDummyHandler = new DummyHandler();
        mDefaultNetworkCallback = new DefaultNetworkCallback(SLOT_0, mDummyHandler);
    }

    @After
    public void tearDown() throws Exception {
        AppContext.deinit();
        mSubId = -1;
        mDummyHandler = null;
        mDefaultNetworkCallback = null;
        super.tearDown();
    }

    @Test
    public void testCallbackLostNetwork() throws Exception {
        // onLost callback when other SIM Cellular network is not used
        mDefaultNetworkCallback.onLost(mMockNetwork);
        processAllMessages();
        assertFalse(mDummyHandler.isOtherSimCellularAvailable());
        assertEquals(0, mDummyHandler.getInvokeCount());

        // callback that other SIM Cellular network is available
        onCapabilitiesChangedForOtherSubIdCellular();
        assertTrue(mDummyHandler.isOtherSimCellularAvailable());
        assertEquals(1, mDummyHandler.getInvokeCount());

        // calback that network is not available when other SIM Cellular network is used
        mDefaultNetworkCallback.onLost(mMockNetwork);
        processAllMessages();
        assertFalse(mDummyHandler.isOtherSimCellularAvailable());
        assertEquals(2, mDummyHandler.getInvokeCount());
    }

    @Test
    public void testCallbackForWifi() throws Exception {
        final NetworkCapabilities ncNotAvailable = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                .setNetworkSpecifier(new TelephonyNetworkSpecifier(mSubId + 1))
                .build();

        // callback for WiFi network when other SIM Cellular is not used
        mDefaultNetworkCallback.onCapabilitiesChanged(mMockNetwork, ncNotAvailable);
        processAllMessages();
        assertFalse(mDummyHandler.isOtherSimCellularAvailable());
        assertEquals(0, mDummyHandler.getInvokeCount());

        // callback that other SIM Cellular network is available
        onCapabilitiesChangedForOtherSubIdCellular();
        assertTrue(mDummyHandler.isOtherSimCellularAvailable());
        assertEquals(1, mDummyHandler.getInvokeCount());

        // callback that WiFi network is available
        mDefaultNetworkCallback.onCapabilitiesChanged(mMockNetwork, ncNotAvailable);
        processAllMessages();
        assertFalse(mDummyHandler.isOtherSimCellularAvailable());
        assertEquals(2, mDummyHandler.getInvokeCount());
    }

    @Test
    public void testCallbackWithoutSubIdInfo() throws Exception {
        final NetworkCapabilities ncNotAvailable = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .build();

        // callback that other SIM Cellular network is available
        onCapabilitiesChangedForOtherSubIdCellular();
        assertTrue(mDummyHandler.isOtherSimCellularAvailable());
        assertEquals(1, mDummyHandler.getInvokeCount());

        // callback without Subscription ID information
        mDefaultNetworkCallback.onCapabilitiesChanged(mMockNetwork, ncNotAvailable);
        processAllMessages();
        assertFalse(mDummyHandler.isOtherSimCellularAvailable());
        assertEquals(2, mDummyHandler.getInvokeCount());
    }

    @Test
    public void testCallbackWithOwnSubId() throws Exception {
        final NetworkCapabilities ncNotAvailable = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .setNetworkSpecifier(new TelephonyNetworkSpecifier(mSubId))
                .build();

        // callback that other SIM Cellular network is available
        onCapabilitiesChangedForOtherSubIdCellular();
        assertTrue(mDummyHandler.isOtherSimCellularAvailable());
        assertEquals(1, mDummyHandler.getInvokeCount());

        // callback for own Subscription ID
        mDefaultNetworkCallback.onCapabilitiesChanged(mMockNetwork, ncNotAvailable);
        processAllMessages();
        assertFalse(mDummyHandler.isOtherSimCellularAvailable());
        assertEquals(2, mDummyHandler.getInvokeCount());
    }

    private void onCapabilitiesChangedForOtherSubIdCellular() {
        final NetworkCapabilities ncAvailable = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .setNetworkSpecifier(new TelephonyNetworkSpecifier(mSubId + 1))
                .build();

        // callback that other SIM Cellular network is available
        mDefaultNetworkCallback.onCapabilitiesChanged(mMockNetwork, ncAvailable);
        processAllMessages();
    }

    private static class DummyHandler extends Handler {
        private boolean mIsOtherSimCellularAvailable = false;
        private int mInvokeCount = 0;

        DummyHandler() {
        }

        public void handleMessage(Message msg) {
            switch(msg.what) {
                case Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED:
                    boolean isCellularAvailable = (boolean) msg.obj;
                    mIsOtherSimCellularAvailable = isCellularAvailable;
                    mInvokeCount++;
                    break;
                default:
                    break;
            }
        }

        public boolean isOtherSimCellularAvailable() {
            return mIsOtherSimCellularAvailable;
        }

        public int getInvokeCount() {
            return mInvokeCount;
        }
    }
}
