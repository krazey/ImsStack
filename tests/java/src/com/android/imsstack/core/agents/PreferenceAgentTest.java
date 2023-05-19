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
package com.android.imsstack.core.agents;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.TestApplication;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class PreferenceAgentTest {
    private static final String TEST_FILE = "test_prefs";
    private static final String TEST_KEY1 = "test-key1";
    private static final String TEST_KEY2 = "test-key2";
    private static final String TEST_VALUE1 = "test-value1";
    private static final String TEST_VALUE2 = "test-value2";
    private static final int SLOT0 = 0;

    private PreferenceAgent mPreferenceAgent;

    @Before
    public void setUp() throws Exception {
        AppContext.init(TestApplication.getAppContext());

        mPreferenceAgent = new PreferenceAgent();
        mPreferenceAgent.init(TestApplication.getAppContext());
    }

    @After
    public void tearDown() throws Exception {
        if (mPreferenceAgent != null) {
            mPreferenceAgent.remove(TEST_KEY1, SLOT0);
            mPreferenceAgent.remove(TEST_KEY2, SLOT0);
            mPreferenceAgent.cleanup();
            mPreferenceAgent = null;
        }

        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void testPreferenceForDefaultFile() {
        assertNull(mPreferenceAgent.getString(TEST_KEY1, SLOT0));
        assertTrue(mPreferenceAgent.putString(TEST_KEY1, TEST_VALUE1, SLOT0));
        assertEquals(TEST_VALUE1, mPreferenceAgent.getString(TEST_KEY1, SLOT0));

        mPreferenceAgent.remove(TEST_KEY1, SLOT0);
        assertNull(mPreferenceAgent.getString(TEST_KEY1, SLOT0));

        // Exceptional cases.
        assertNull(mPreferenceAgent.getString(null, SLOT0));
        assertFalse(mPreferenceAgent.putString(null, TEST_VALUE1, SLOT0));
        assertFalse(mPreferenceAgent.putString(TEST_KEY1, null, SLOT0));
        mPreferenceAgent.remove(null, SLOT0);
    }

    @Test
    @SmallTest
    public void testPreferenceForSpecificFile() {
        assertNull(mPreferenceAgent.getString(TEST_FILE, TEST_KEY2, SLOT0));
        assertTrue(mPreferenceAgent.putString(TEST_FILE, TEST_KEY2, TEST_VALUE2, SLOT0));
        assertEquals(TEST_VALUE2, mPreferenceAgent.getString(TEST_FILE, TEST_KEY2, SLOT0));

        mPreferenceAgent.remove(TEST_FILE, TEST_KEY2, SLOT0);
        assertNull(mPreferenceAgent.getString(TEST_FILE, TEST_KEY2, SLOT0));

        // Exceptional cases.
        assertNull(mPreferenceAgent.getString(TEST_FILE, null, SLOT0));
        assertFalse(mPreferenceAgent.putString(TEST_FILE, null, TEST_VALUE2, SLOT0));
        assertFalse(mPreferenceAgent.putString(TEST_FILE, TEST_KEY2, null, SLOT0));
        mPreferenceAgent.remove(TEST_FILE, null, SLOT0);

        assertTrue(mPreferenceAgent.putString(null, TEST_KEY1, TEST_VALUE1, SLOT0));
        assertEquals(TEST_VALUE1, mPreferenceAgent.getString(null, TEST_KEY1, SLOT0));
    }
}
