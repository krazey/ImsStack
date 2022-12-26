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

package com.android.imsstack.enabler.mtc;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class ConferenceInfoManagerTest {
    private String mCcid = "mCcid";

    private ConferenceInfoManager mTestConferenceInfoManager;

    @Before
    public void setUp() throws Exception {
        mTestConferenceInfoManager = ConferenceInfoManager.getInstance();
    }

    @After
    public void tearDown() throws Exception {
        mTestConferenceInfoManager.destroyAllConferenceInfos();
    }

    @Test
    public void testConferenceInfoManager() {
        mTestConferenceInfoManager.createConferenceInfo("");

        assertFalse(mTestConferenceInfoManager.hasConferenceInfo());

        mTestConferenceInfoManager.createConferenceInfo(mCcid);

        assertTrue(mTestConferenceInfoManager.hasConferenceInfo());
        assertNotNull(mTestConferenceInfoManager.getConferenceInfo(mCcid));

        mTestConferenceInfoManager.destroyConferenceInfo(mCcid);

        assertFalse(mTestConferenceInfoManager.hasConferenceInfo());
        assertNull(mTestConferenceInfoManager.getConferenceInfo(mCcid));

        mTestConferenceInfoManager.createConferenceInfo(mCcid);
        mTestConferenceInfoManager.createConferenceInfo(mCcid + "1");

        mTestConferenceInfoManager.destroyAllConferenceInfos();

        assertFalse(mTestConferenceInfoManager.hasConferenceInfo());
        assertNull(mTestConferenceInfoManager.getConferenceInfo(mCcid));
        assertNull(mTestConferenceInfoManager.getConferenceInfo(mCcid + "1"));

        mTestConferenceInfoManager.createConferenceInfo(mCcid);
        assertTrue(ConferenceInfoHelper.addConferenceUser(mCcid, mCcid + "1", 1));

        assertTrue(mTestConferenceInfoManager.hasConferenceInfo());
        assertNotNull(mTestConferenceInfoManager.getConferenceInfoByUser(
                String.valueOf(1), mCcid + "1"));
    }
}
