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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.telephony.TelephonyManager;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.ims.ImsManager;
import com.android.imsstack.ContextFixture;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class ImsUtilsTest {
    private static final int SLOT0 = 0;

    private Context mContext;

    @Before
    public void setup() {
        mContext = new ContextFixture().getTestDouble();
        TelephonyManager tm = mContext.getSystemService(TelephonyManager.class);
        when(tm.getActiveModemCount()).thenReturn(1);
        when(tm.getSupportedModemCount()).thenReturn(1);
        AppContext.init(mContext);
    }

    @After
    public  void tearDown() {
        ImsUtils.clear();
        AppContext.deinit();
        mContext = null;
    }

    @Test
    @SmallTest
    public void testUpdateImsManager() {
        assertNotNull(ImsUtils.getImsManager(SLOT0));
        assertNotNull(ImsUtils.getImsManager(2));

        ImsUtils.init();

        assertNotNull(ImsUtils.getImsManager(SLOT0));

        ImsUtils.updateImsManager();

        assertNotNull(ImsUtils.getImsManager(SLOT0));
    }

    @Test
    @SmallTest
    public void testGetDeviceConfigurations() {
        when(mContext.getResources().getBoolean(anyInt())).thenReturn(true);

        assertEquals(true, ImsUtils.isVoLteEnabledByDevice(mContext, SLOT0));
        assertEquals(true, ImsUtils.isVtEnabledByDevice(mContext, SLOT0));
        assertEquals(true, ImsUtils.isWfcEnabledByDevice(mContext, SLOT0));
    }

    @Test
    @SmallTest
    public void testGetPlatformConfigurations() {
        ImsManager imsManager = Mockito.mock(ImsManager.class);
        ImsUtils.setImsManager(SLOT0, imsManager);

        when(imsManager.isVolteEnabledByPlatform()).thenReturn(false);
        when(imsManager.isVtEnabledByPlatform()).thenReturn(false);
        when(imsManager.isWfcEnabledByPlatform()).thenReturn(false);

        assertFalse(ImsUtils.isVoLteEnabledByPlatform(mContext, SLOT0));
        assertFalse(ImsUtils.isVtEnabledByPlatform(mContext, SLOT0));
        assertFalse(ImsUtils.isWfcEnabledByPlatform(mContext, SLOT0));

        when(imsManager.isVolteEnabledByPlatform()).thenReturn(true);
        when(imsManager.isVtEnabledByPlatform()).thenReturn(true);
        when(imsManager.isWfcEnabledByPlatform()).thenReturn(true);

        assertTrue(ImsUtils.isVoLteEnabledByPlatform(mContext, SLOT0));
        assertTrue(ImsUtils.isVtEnabledByPlatform(mContext, SLOT0));
        assertTrue(ImsUtils.isWfcEnabledByPlatform(mContext, SLOT0));
    }

    @Test
    @SmallTest
    public void testSetServiceCapsToLocalStorage() {
        ImsUtils.setServiceCapsToLocalStorage(SLOT0, null);
        ImsUtils.ServiceCaps serviceCaps = ImsUtils.getServiceCapsFromLocalStorage(SLOT0);

        assertNull(serviceCaps);

        ImsUtils.ServiceCaps serviceCapsForAllEnabled = new ImsUtils.ServiceCaps(true, true, true);
        ImsUtils.init();
        ImsUtils.setServiceCapsToLocalStorage(-1, serviceCapsForAllEnabled);
        serviceCaps = ImsUtils.getServiceCapsFromLocalStorage(-1);

        assertNull(serviceCaps);

        serviceCaps = ImsUtils.getServiceCapsFromLocalStorage(SLOT0);

        assertNotNull(serviceCaps);
        assertFalse(serviceCaps.isVoLteEnabled());
        assertFalse(serviceCaps.isVtEnabled());
        assertFalse(serviceCaps.isWfcEnabled());
        assertNotNull(serviceCaps.toString());

        ImsUtils.setServiceCapsToLocalStorage(SLOT0, serviceCapsForAllEnabled);
        serviceCaps = ImsUtils.getServiceCapsFromLocalStorage(SLOT0);

        assertNotNull(serviceCaps);
        assertTrue(serviceCaps.isVoLteEnabled());
        assertTrue(serviceCaps.isVtEnabled());
        assertTrue(serviceCaps.isWfcEnabled());
        assertNotNull(serviceCaps.toString());
    }
}

