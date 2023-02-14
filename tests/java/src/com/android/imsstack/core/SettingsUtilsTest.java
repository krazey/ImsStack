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

package com.android.imsstack.core;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;

import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class SettingsUtilsTest {
    private Context mContext;
    private final int mSlotId = 1;
    public static final int VALUE_OFF = 0;

    @Before
    public void setUp() throws Exception {
        mContext = mock(Context.class);
        AppContext.init(mContext);
    }

    @After
    public void tearDown() throws Exception {
        mContext = null;
        AppContext.deinit();
    }

    @Test
    public void getDefaultWfcImsEnabledTest() {
        assertEquals(VALUE_OFF, SettingsUtils.getDefaultWfcImsEnabled(mSlotId));
    }

    @Test
    public void getWFCImsModeTest() {
        assertEquals(SettingsUtils.WfcMode.WIFI_PREFERRED,
                SettingsUtils.getWFCImsMode(mContext, mSlotId));
    }

    @Test
    public void getDefaultWfcImsRoamingEnabledTest() {
        assertEquals(VALUE_OFF, SettingsUtils.getDefaultWfcImsRoamingEnabled(mSlotId));
    }

    @Test
    public void isWFCImsEnabledTest() {
        assertEquals(false, SettingsUtils.isWFCImsEnabled(mContext, mSlotId));
        assertEquals(false, SettingsUtils.isWFCImsEnabled(null, mSlotId));
    }

    @Test
    public void isWfcImsRoamingEnabledTest() {
        assertEquals(false, SettingsUtils.isWfcImsRoamingEnabled(mContext, mSlotId));
    }

    @Test public void isVtImsEnabledTest() {
        assertEquals(true, SettingsUtils.isVtImsEnabled(mContext, mSlotId));
    }
    @Test
    public void isWFCImsRoamingEnabledTest() {
        assertEquals(false, SettingsUtils.isWFCImsRoamingEnabled(mContext, mSlotId));
        assertEquals(false, SettingsUtils.isWFCImsRoamingEnabled(null, mSlotId));
    }

    @Test
    public void getWfcImsRoamingModeTest() {
        assertEquals(SettingsUtils.WfcMode.WIFI_PREFERRED,
                SettingsUtils.getWfcImsRoamingMode(mContext, mSlotId));
    }

    @Test
    public void registerObserverForSystemTest() {
        ContentResolver resolver = mock(ContentResolver.class);
        ContentObserver observer = mock(ContentObserver.class);
        SettingsUtils.registerObserverForSystem(null, null, observer);

        SettingsUtils.unregisterObserver(resolver, observer);
        verify(resolver).unregisterContentObserver(observer);
    }

}
