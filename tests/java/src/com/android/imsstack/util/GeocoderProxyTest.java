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

package com.android.imsstack.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import android.content.Context;
import android.location.Address;
import android.location.Location;
import android.os.Handler;
import android.os.Looper;
import android.test.suitebuilder.annotation.SmallTest;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class GeocoderProxyTest {
    private static final String UNKNOWN_COUNTRY = "ZZ";
    private GeocoderProxy mGeocoderProxy;
    private Context mContext;

    @Before
    public void setup() {
        mContext = Mockito.mock(Context.class);
        mGeocoderProxy = new GeocoderProxy(mContext, new Handler(Looper.myLooper()));
    }

    @After
    public void tearDown() {
        mGeocoderProxy = null;
    }

    @Test
    @SmallTest
    public void getAddresses() {
        assertNull(mGeocoderProxy.getAddresses());
    }

    @Test
    @SmallTest
    public void getAddress() {
        assertNull(mGeocoderProxy.getAddress());
    }

    @Test
    @SmallTest
    public void getAddressFromLocation() {
        Address address = GeocoderProxy.getAddressFromLocation(mContext, null);
        assertNull(address);

        Location location = new Location("US");
        location.setLatitude(37D);
        location.setLatitude(-122D);
        address = GeocoderProxy.getAddressFromLocation(mContext, location);
        assertNull(address);
    }

    @Test
    @SmallTest
    public void getCountryFromLocation() {
        assertEquals(UNKNOWN_COUNTRY, GeocoderProxy.getCountryFromLocation(mContext, null));
    }
}
