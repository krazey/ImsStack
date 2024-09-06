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

package com.android.imsstack.test.menu;

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SLOT1;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import android.app.Instrumentation;
import android.content.Intent;
import android.preference.Preference;
import android.widget.AdapterView;
import android.widget.ListView;

import androidx.test.filters.LargeTest;
import androidx.test.filters.SmallTest;
import androidx.test.platform.app.InstrumentationRegistry;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.carrier.CarrierInfo;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ImsConfigMenuTest {
    private Instrumentation mInstrumentation;
    private ImsConfigMenu mImsConfigMenu;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        mInstrumentation = InstrumentationRegistry.getInstrumentation();
        DeviceConfig.setSimCount(2, 2);
        CarrierInfo.clear();
    }

    @After
    public void tearDown() throws Exception {
        mImsConfigMenu = null;
        mInstrumentation = null;
        CarrierInfo.clear();
        DeviceConfig.setSimCount(1, 1);
    }

    @Test
    @LargeTest
    public void testOnCreate() {
        setUpActivity(false);

        ListView listView = mImsConfigMenu.getListView();
        AdapterView.OnItemClickListener listener = listView.getOnItemClickListener();

        assertNotNull(listener);

        Preference menu;
        // SIM1
        final int positionSim1 = 1;
        mInstrumentation.runOnMainSync(() -> {
            listener.onItemClick(listView, listView, positionSim1, 1L);
        });

        menu = mImsConfigMenu.findPreference(ImsConfigMenu.CARRIER_CONFIG_MENU);

        assertNotNull(menu);
        assertEquals(SLOT0, menu.getIntent().getIntExtra(MSimUtils.EXTRA_KEY_SLOT_ID, -1));

        menu = mImsConfigMenu.findPreference(ImsConfigMenu.TEST_CONFIG_MENU);

        assertNotNull(menu);
        assertEquals(SLOT0, menu.getIntent().getIntExtra(MSimUtils.EXTRA_KEY_SLOT_ID, -1));

        // SIM2
        final int positionSim2 = 2;
        mInstrumentation.runOnMainSync(() -> {
            listener.onItemClick(listView, listView, positionSim2, 1L);
        });

        menu = mImsConfigMenu.findPreference(ImsConfigMenu.CARRIER_CONFIG_MENU);

        assertNotNull(menu);
        assertEquals(SLOT1, menu.getIntent().getIntExtra(MSimUtils.EXTRA_KEY_SLOT_ID, -1));

        menu = mImsConfigMenu.findPreference(ImsConfigMenu.TEST_CONFIG_MENU);

        assertNotNull(menu);
        assertEquals(SLOT1, menu.getIntent().getIntExtra(MSimUtils.EXTRA_KEY_SLOT_ID, -1));
    }

    @Test
    @SmallTest
    public void testIsValidFragment() {
        setUpActivity(true);
        assertTrue(mImsConfigMenu.isValidFragment("test"));
        assertFalse(mImsConfigMenu.isValidFragment(null));
    }

    private void setUpActivity(boolean isSmallTest) {
        Intent intent = new Intent(mInstrumentation.getTargetContext(), ImsConfigMenu.class);

        if (isSmallTest) {
            mInstrumentation.runOnMainSync(() -> {
                try {
                    mImsConfigMenu = (ImsConfigMenu) mInstrumentation.newActivity(
                            getClass().getClassLoader(), ImsConfigMenu.class.getName(), intent);
                } catch (Exception e) {
                    throw new RuntimeException(e); // nothing to do
                }
            });
        } else {
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mImsConfigMenu = (ImsConfigMenu) mInstrumentation.startActivitySync(intent);
        }

        assertNotNull(mImsConfigMenu);
    }
}
