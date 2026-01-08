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
package com.android.imsstack.imsservice.mmtel;

import static org.junit.Assert.assertEquals;

import com.android.imsstack.base.TestAppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class ImsCallConnectionIdsTest {
    private ImsCallConnectionIds mImsCallConnectionIds;
    private int mCcId = 0;
    private static final int CONNECTION_ID_1 = 1;
    private static final int CONNECTION_ID_2 = 2;

    private TestAppContext mTestAppContext;

    @Before
    public void setUp() throws Exception {
        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();
        mImsCallConnectionIds = new ImsCallConnectionIds();
    }

    @Test
    public void test_callConnectionIds() {
        mCcId = ImsCallConnectionIds.getNewId(TestAppContext.SLOT0);
        ImsCallConnectionIds.add(TestAppContext.SLOT0, mCcId);
        assertEquals(CONNECTION_ID_1, mCcId);

        int ccId = ImsCallConnectionIds.getNewId(TestAppContext.SLOT0);
        ImsCallConnectionIds.add(TestAppContext.SLOT0, ccId);
        assertEquals(CONNECTION_ID_2, ccId);

        ImsCallConnectionIds.remove(TestAppContext.SLOT0, mCcId);
        assertEquals(ImsCallConnectionIds.getNewId(TestAppContext.SLOT0), CONNECTION_ID_1);

        ImsCallConnectionIds.add(TestAppContext.SLOT1, mCcId);
        assertEquals(CONNECTION_ID_1, mCcId);

        ImsCallConnectionIds.removeAll(TestAppContext.SLOT0);
        assertEquals(ImsCallConnectionIds.getNewId(TestAppContext.SLOT0), CONNECTION_ID_1);

        ImsCallConnectionIds.removeAll(TestAppContext.SLOT1);
        assertEquals(ImsCallConnectionIds.getNewId(TestAppContext.SLOT1), CONNECTION_ID_1);
    }

    @After
    public void tearDown() throws Exception {
        mImsCallConnectionIds.removeAll(TestAppContext.SLOT0);
        mImsCallConnectionIds.removeAll(TestAppContext.SLOT1);
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }
}
