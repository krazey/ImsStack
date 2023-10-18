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
package com.android.imsstack.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import android.test.suitebuilder.annotation.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class MapIntLongTest {
    private static final int TEST_KEY1 = 1;
    private static final int TEST_KEY2 = 2;
    private static final long TEST_VALUE1 = 10000;
    private static final long TEST_VALUE2 = 20000;

    private MapIntLong mMapIntLong;

    @Before
    public void setUp() throws Exception {
        mMapIntLong = new MapIntLong();
    }

    @After
    public void tearDown() throws Exception {
        mMapIntLong = null;
    }

    @Test
    @SmallTest
    public void add() throws Exception {
        // No values
        assertEquals(MapIntLong.INVALID_KEY, mMapIntLong.getKey(TEST_VALUE1));
        assertEquals(MapIntLong.INVALID_KEY, mMapIntLong.getKey(TEST_VALUE2));
        assertEquals(MapIntLong.INVALID_VALUE, mMapIntLong.getValue(TEST_KEY1));
        assertEquals(MapIntLong.INVALID_VALUE, mMapIntLong.getValue(TEST_KEY2));

        // Add new ones
        mMapIntLong.add(TEST_KEY1, TEST_VALUE1);
        mMapIntLong.add(TEST_KEY2, TEST_VALUE2);

        assertEquals(TEST_KEY1, mMapIntLong.getKey(TEST_VALUE1));
        assertEquals(TEST_KEY2, mMapIntLong.getKey(TEST_VALUE2));
        assertEquals(TEST_VALUE1, mMapIntLong.getValue(TEST_KEY1));
        assertEquals(TEST_VALUE2, mMapIntLong.getValue(TEST_KEY2));
    }

    @Test
    @SmallTest
    public void contains() throws Exception {
        // No values
        assertFalse(mMapIntLong.contains(TEST_KEY1));
        assertFalse(mMapIntLong.contains(TEST_KEY2));

        // Add new ones
        mMapIntLong.add(TEST_KEY1, TEST_VALUE1);
        mMapIntLong.add(TEST_KEY2, TEST_VALUE2);

        assertTrue(mMapIntLong.contains(TEST_KEY1));
        assertTrue(mMapIntLong.contains(TEST_KEY2));

        // Remove all
        mMapIntLong.removeAll();

        assertFalse(mMapIntLong.contains(TEST_KEY1));
        assertFalse(mMapIntLong.contains(TEST_KEY2));
    }

    @Test
    @SmallTest
    public void getNewKey() throws Exception {
        int newKey = mMapIntLong.getNewKey();

        mMapIntLong.add(newKey, TEST_VALUE2);

        assertEquals(newKey, mMapIntLong.getKey(TEST_VALUE2));
    }

    @Test
    @SmallTest
    public void remove() throws Exception {
        mMapIntLong.add(TEST_KEY1, TEST_VALUE1);
        mMapIntLong.add(TEST_KEY2, TEST_VALUE2);

        assertTrue(mMapIntLong.contains(TEST_KEY1));
        assertTrue(mMapIntLong.contains(TEST_KEY2));

        mMapIntLong.remove(TEST_KEY1);

        assertFalse(mMapIntLong.contains(TEST_KEY1));
        assertTrue(mMapIntLong.contains(TEST_KEY2));

        mMapIntLong.remove(TEST_KEY2);

        assertFalse(mMapIntLong.contains(TEST_KEY1));
        assertFalse(mMapIntLong.contains(TEST_KEY2));
    }

    @Test
    @SmallTest
    public void removeAll() throws Exception {
        mMapIntLong.add(TEST_KEY1, TEST_VALUE1);
        mMapIntLong.add(TEST_KEY2, TEST_VALUE2);

        assertTrue(mMapIntLong.contains(TEST_KEY1));
        assertTrue(mMapIntLong.contains(TEST_KEY2));

        mMapIntLong.removeAll();

        assertFalse(mMapIntLong.contains(TEST_KEY1));
        assertFalse(mMapIntLong.contains(TEST_KEY2));
    }

    @Test
    @SmallTest
    public void removeValue() throws Exception {
        mMapIntLong.add(TEST_KEY1, TEST_VALUE1);
        mMapIntLong.add(TEST_KEY2, TEST_VALUE2);

        assertTrue(mMapIntLong.contains(TEST_KEY1));
        assertTrue(mMapIntLong.contains(TEST_KEY2));

        mMapIntLong.removeValue(TEST_VALUE1);

        assertFalse(mMapIntLong.contains(TEST_KEY1));
        assertTrue(mMapIntLong.contains(TEST_KEY2));

        mMapIntLong.removeValue(TEST_VALUE2);

        assertFalse(mMapIntLong.contains(TEST_KEY1));
        assertFalse(mMapIntLong.contains(TEST_KEY2));
    }
}
