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

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class ImsCallConnectionIdsTest {
    private ImsCallConnectionIds mImsCallConnectionIds;
    private int mCcId = 0;
    private static final int SLOT_0 = 0;
    private static final int SLOT_1 = 1;
    private static final int CONNECTION_ID_1 = 1;
    private static final int CONNECTION_ID_2 = 2;


    @Before
    public void setUp() throws Exception {
        mImsCallConnectionIds = new ImsCallConnectionIds();
    }

    @Test
    public void test_callConnectionIds() {
        mCcId = ImsCallConnectionIds.getNewId(SLOT_0);
        ImsCallConnectionIds.add(SLOT_0, mCcId);
        assertEquals(CONNECTION_ID_1, mCcId);

        int ccId = ImsCallConnectionIds.getNewId(SLOT_0);
        ImsCallConnectionIds.add(SLOT_0, ccId);
        assertEquals(CONNECTION_ID_2, ccId);

        ImsCallConnectionIds.remove(SLOT_0, mCcId);
        assertEquals(ImsCallConnectionIds.getNewId(SLOT_0), CONNECTION_ID_1);

        ImsCallConnectionIds.add(SLOT_1, mCcId);
        assertEquals(CONNECTION_ID_1, mCcId);

        ImsCallConnectionIds.removeAll(SLOT_0);
        assertEquals(ImsCallConnectionIds.getNewId(SLOT_0), CONNECTION_ID_1);

        ImsCallConnectionIds.removeAll(SLOT_1);
        assertEquals(ImsCallConnectionIds.getNewId(SLOT_1), CONNECTION_ID_1);
    }

    @After
    public void tearDown() throws Exception {
        mImsCallConnectionIds.removeAll(SLOT_0);
        mImsCallConnectionIds.removeAll(SLOT_1);
    }
}
