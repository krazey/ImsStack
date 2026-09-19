/*
 * Copyright (C) 2026 The Android Open Source Project
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
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;

import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.core.agents.dcmif.DcConstants;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ApnNetworkReplacementTest {
    private final Network mOld = new Network(100);
    private final Network mNew = new Network(101);
    private final List<Integer> mEvents = new ArrayList<>();
    private final Map<Network, LinkProperties> mProperties = new HashMap<>();
    private Apn.ImsNetworkCallback mCallback;
    private TestableLooper mLooper;
    private Handler mHandler;

    @Before
    public void setUp() {
        mLooper = TestableLooper.get(this);
        mHandler = new Handler(Looper.myLooper()) {
            @Override
            public void handleMessage(Message message) {
                mEvents.add(message.what);
            }
        };
        mCallback = new Apn.ImsNetworkCallback(DcConstants.TYPE_IMS, mHandler) {
            @Override
            protected void cacheLinkProperties(Network network) {
                mCachedLinkProperties = mProperties.get(network);
            }
        };
        mProperties.put(mOld, properties("ipsec0", "192.0.2.10/24"));
        mProperties.put(mNew, properties("rmnet_data1", "192.0.2.20/24"));
    }

    private LinkProperties properties(String iface, String address) {
        LinkProperties lp = new LinkProperties();
        lp.setInterfaceName(iface);
        lp.addLinkAddress(new LinkAddress(address));
        return lp;
    }

    private void replaceNetwork() {
        mCallback.onAvailable(mOld);
        mCallback.onAvailable(mNew);
        mLooper.processAllMessages();
        // Preserve the registration restart required by a genuinely new Network.
        assertEquals(List.of(Apn.EVENT_NETWORK_AVAILABLE, Apn.EVENT_NETWORK_LOST,
                Apn.EVENT_NETWORK_AVAILABLE), mEvents);
        mEvents.clear();
    }

    @Test
    public void lateLossDoesNotClearReplacementNetwork() {
        replaceNetwork();
        mCallback.onLost(mOld);
        mLooper.processAllMessages();
        assertSame(mNew, mCallback.getCachedNetwork());
        assertSame(mProperties.get(mNew), mCallback.mCachedLinkProperties);
        assertTrue(mEvents.isEmpty());
    }

    @Test
    public void staleLinkPropertiesCannotOverwriteReplacementInterface() {
        replaceNetwork();
        mCallback.onLinkPropertiesChanged(mNew, mProperties.get(mNew));
        mLooper.processAllMessages();
        mEvents.clear();
        mCallback.onLinkPropertiesChanged(mOld, mProperties.get(mOld));
        mLooper.processAllMessages();
        assertEquals("rmnet_data1", mCallback.getActiveIfaceName());
        assertSame(mProperties.get(mNew), mCallback.mCachedLinkProperties);
        assertTrue(mEvents.isEmpty());
    }

    @Test
    public void staleCapabilitiesDoNotRevertPendingReplacement() {
        mCallback.onAvailable(mOld);
        mProperties.remove(mNew);
        mCallback.onAvailable(mNew);
        mLooper.processAllMessages();
        mEvents.clear();
        assertTrue(mCallback.mIsPendingOnAvailable);
        mCallback.onCapabilitiesChanged(mOld, new NetworkCapabilities());
        mLooper.processAllMessages();
        assertSame(mNew, mCallback.getCachedNetwork());
        assertTrue(mCallback.mIsPendingOnAvailable);
        assertTrue(mEvents.isEmpty());

        mProperties.put(mNew, properties("rmnet_data1", "192.0.2.20/24"));
        mCallback.onCapabilitiesChanged(mNew, new NetworkCapabilities());
        mLooper.processAllMessages();
        assertFalse(mCallback.mIsPendingOnAvailable);
        assertEquals(List.of(Apn.EVENT_NETWORK_AVAILABLE,
                Apn.EVENT_NETWORK_CAPABILITIES_CHANGED), mEvents);
    }

    @Test
    public void staleLosingCallbackDoesNotReachTheApn() {
        replaceNetwork();
        mCallback.onLosing(mOld, 1000);
        mLooper.processAllMessages();
        assertTrue(mEvents.isEmpty());
        mCallback.onLosing(mNew, 1000);
        mLooper.processAllMessages();
        assertEquals(List.of(Apn.EVENT_NETWORK_LOSING), mEvents);
    }

    @Test
    public void actualLossStillNotifiesAndClearsPendingAvailability() {
        replaceNetwork();
        mCallback.mIsPendingOnAvailable = true;
        mCallback.mActiveIfaceName = "rmnet_data1";
        mCallback.onLost(mNew);
        mLooper.processAllMessages();
        assertNull(mCallback.getCachedNetwork());
        assertNull(mCallback.mCachedLinkProperties);
        assertNull(mCallback.getActiveIfaceName());
        assertFalse(mCallback.mIsPendingOnAvailable);
        assertEquals(List.of(Apn.EVENT_NETWORK_LOST), mEvents);
    }
}
