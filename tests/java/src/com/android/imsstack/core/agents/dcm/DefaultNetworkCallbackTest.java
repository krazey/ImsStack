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

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.TelephonyNetworkSpecifier;
import android.os.Handler;
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
    public void testLostNetworkWhileCrossSimIsNotUsed() throws Exception {
        mDefaultNetworkCallback.mIsOtherSimCellularAvailable = false;

        mDefaultNetworkCallback.onLost(mMockNetwork);

        assertFalse(mDefaultNetworkCallback.mIsOtherSimCellularAvailable);
        assertFalse(mDummyHandler.hasMessages(Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED));
    }

    @Test
    public void testLostNetworkWhileCrossSimIsUsed() throws Exception {
        mDefaultNetworkCallback.mIsOtherSimCellularAvailable = true;

        mDefaultNetworkCallback.onLost(mMockNetwork);

        assertFalse(mDefaultNetworkCallback.mIsOtherSimCellularAvailable);
        assertTrue(mDummyHandler.hasMessages(Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED));
    }

    @Test
    public void testCapabilitiesChangedForWifiWhileCrossSimIsNotUsed() throws Exception {
        final NetworkCapabilities nc = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                .setNetworkSpecifier(new TelephonyNetworkSpecifier(mSubId + 1))
                .build();
        mDefaultNetworkCallback.mIsOtherSimCellularAvailable = false;

        mDefaultNetworkCallback.onCapabilitiesChanged(mMockNetwork, nc);

        assertFalse(mDefaultNetworkCallback.mIsOtherSimCellularAvailable);
        assertFalse(mDummyHandler.hasMessages(Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED));
    }

    @Test
    public void testCapabilitiesChangedForWifiWhileCrossSimIsUsed() throws Exception {
        final NetworkCapabilities nc = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                .setNetworkSpecifier(new TelephonyNetworkSpecifier(mSubId + 1))
                .build();
        mDefaultNetworkCallback.mIsOtherSimCellularAvailable = true;

        mDefaultNetworkCallback.onCapabilitiesChanged(mMockNetwork, nc);

        assertFalse(mDefaultNetworkCallback.mIsOtherSimCellularAvailable);
        assertTrue(mDummyHandler.hasMessages(Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED));
    }

    @Test
    public void testCapabilitiesChangedForCellularWhileCrossSimIsNotUsed() throws Exception {
        final NetworkCapabilities nc = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .setNetworkSpecifier(new TelephonyNetworkSpecifier(mSubId + 1))
                .build();
        mDefaultNetworkCallback.mIsOtherSimCellularAvailable = false;

        mDefaultNetworkCallback.onCapabilitiesChanged(mMockNetwork, nc);

        assertTrue(mDefaultNetworkCallback.mIsOtherSimCellularAvailable);
        assertTrue(mDummyHandler.hasMessages(Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED));
    }

    @Test
    public void testCapabilitiesChangedForCellularWhileCrossSimIsUsed() throws Exception {
        final NetworkCapabilities nc = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .setNetworkSpecifier(new TelephonyNetworkSpecifier(mSubId + 1))
                .build();
        mDefaultNetworkCallback.mIsOtherSimCellularAvailable = true;

        mDefaultNetworkCallback.onCapabilitiesChanged(mMockNetwork, nc);

        assertTrue(mDefaultNetworkCallback.mIsOtherSimCellularAvailable);
        assertFalse(mDummyHandler.hasMessages(Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED));
    }

    @Test
    public void testCapabilitiesChangedForCellularWithoutSubId() throws Exception {
        final NetworkCapabilities nc = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .build();
        mDefaultNetworkCallback.mIsOtherSimCellularAvailable = false;

        mDefaultNetworkCallback.onCapabilitiesChanged(mMockNetwork, nc);

        assertFalse(mDefaultNetworkCallback.mIsOtherSimCellularAvailable);
        assertFalse(mDummyHandler.hasMessages(Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED));
    }

    @Test
    public void testCapabilitiesChangedForCellularWithSameSubId() throws Exception {
        final NetworkCapabilities nc = new NetworkCapabilities.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                .setNetworkSpecifier(new TelephonyNetworkSpecifier(mSubId))
                .build();
        mDefaultNetworkCallback.mIsOtherSimCellularAvailable = false;

        mDefaultNetworkCallback.onCapabilitiesChanged(mMockNetwork, nc);

        assertFalse(mDefaultNetworkCallback.mIsOtherSimCellularAvailable);
        assertFalse(mDummyHandler.hasMessages(Apn.EVENT_DEFAULT_NETWORK_STATUS_CHANGED));
    }

    private static class DummyHandler extends Handler {
        DummyHandler() {
        }
    }
}
