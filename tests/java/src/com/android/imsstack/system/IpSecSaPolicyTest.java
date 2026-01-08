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

package com.android.imsstack.system;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import android.os.Parcel;

import androidx.test.filters.SmallTest;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class IpSecSaPolicyTest {
    private static final int TEST_SPI = 1;

    @Test
    @SmallTest
    public void testIpSecSaPolicy() {
        String localIp = "192.168.0.1";
        String remoteIp = "192.168.0.2";
        IpSecSaPolicy saPolicy = new IpSecSaPolicy(TEST_SPI,
                IpSecSaPolicy.DIRECTION_OUT, IpSecSaPolicy.MODE_TRANSPORT,
                localIp, remoteIp);

        assertEquals(TEST_SPI, saPolicy.getSpi());
        assertEquals(IpSecSaPolicy.DIRECTION_OUT, saPolicy.getDirection());
        assertEquals(IpSecSaPolicy.MODE_TRANSPORT, saPolicy.getMode());
        assertEquals(localIp, saPolicy.getLocalIp());
        assertEquals(remoteIp, saPolicy.getRemoteIp());
        assertNotNull(saPolicy.toString());
        assertEquals(0, saPolicy.describeContents());
    }

    @Test
    @SmallTest
    public void testCreator() {
        String localIp = "192.168.0.1";
        String remoteIp = "192.168.0.2";
        IpSecSaPolicy saPolicy = new IpSecSaPolicy(TEST_SPI,
                IpSecSaPolicy.DIRECTION_OUT, IpSecSaPolicy.MODE_TRANSPORT,
                localIp, remoteIp);
        IpSecSaPolicy newSaPolicy;
        Parcel parcel = Parcel.obtain();

        try {
            saPolicy.writeToParcel(parcel, 0);
            parcel.setDataPosition(0);
            newSaPolicy = IpSecSaPolicy.CREATOR.createFromParcel(parcel);
        } finally {
            parcel.recycle();
        }

        assertEquals(TEST_SPI, newSaPolicy.getSpi());
        assertEquals(IpSecSaPolicy.DIRECTION_OUT, newSaPolicy.getDirection());
        assertEquals(IpSecSaPolicy.MODE_TRANSPORT, newSaPolicy.getMode());
        assertEquals(localIp, newSaPolicy.getLocalIp());
        assertEquals(remoteIp, newSaPolicy.getRemoteIp());
        assertNotNull(newSaPolicy.toString());

        IpSecSaPolicy[] saPolicies = IpSecSaPolicy.CREATOR.newArray(6);

        assertEquals(6, saPolicies.length);
    }
}
