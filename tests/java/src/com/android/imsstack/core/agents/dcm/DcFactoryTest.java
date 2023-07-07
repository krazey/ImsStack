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

package com.android.imsstack.core.agents.dcm;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;

import android.content.Context;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcSettings;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.util.MSimUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class DcFactoryTest {
    private static final int SLOT_0 = 0;
    private static final int SLOT_1 = 1;

    @Mock private Context mContext;
    @Mock private DcUtils mDcUtils;
    @Mock private DcSettings mDcSettings;
    @Mock private DcNetWatcher mDcNetWatcher;
    @Mock private DcApn mDcApn;
    @Mock private DcApn mDcApn2;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void tearDown() throws Exception {
        DcFactory.clear(SLOT_0);
    }

    @Test
    @SmallTest
    public void testGetDcAgent() {
        assertNull(DcFactory.getDcAgent(IDcUtils.class, SLOT_0));
        assertNull(DcFactory.getDcAgent(IDcSettings.class, SLOT_0));
        assertNull(DcFactory.getDcAgent(IDcNetWatcher.class, SLOT_0));
        assertNull(DcFactory.getDcAgent(IDcApn.class, SLOT_0));

        setUpDcAgents();

        assertEquals(mDcUtils, DcFactory.getDcAgent(IDcUtils.class, SLOT_0));
        assertEquals(mDcSettings, DcFactory.getDcAgent(IDcSettings.class, SLOT_0));
        assertEquals(mDcNetWatcher, DcFactory.getDcAgent(IDcNetWatcher.class, SLOT_0));
        assertEquals(mDcApn, DcFactory.getDcAgent(IDcApn.class, SLOT_0));

        // Exception handling.
        assertNull(DcFactory.getDcAgent(IDcUtils.class, SLOT_1));
        assertNull(DcFactory.getDcAgent(IDcSettings.class, MSimUtils.INVALID_SUB_ID));

        DcFactory.setDcAgent(IDcApn.class, mDcApn2, MSimUtils.INVALID_SUB_ID);

        // Expect: not changed
        assertEquals(mDcApn, DcFactory.getDcAgent(IDcApn.class, SLOT_0));
    }

    @Test
    @SmallTest
    public void testCreateDcAgent() {
        // To guarantee that the container of DC agents is empty.
        DcFactory.clear(SLOT_0);
        DcFactory.createDcAgents(SLOT_0);

        assertNotNull(DcFactory.getDcAgent(IDcUtils.class, SLOT_0));
        assertNotNull(DcFactory.getDcAgent(IDcSettings.class, SLOT_0));
        assertNotNull(DcFactory.getDcAgent(IDcNetWatcher.class, SLOT_0));
        assertNotNull(DcFactory.getDcAgent(IDcApn.class, SLOT_0));
    }

    @Test
    @SmallTest
    public void testCreateDcAgentWhenAlreadyPresent() {
        setUpDcAgents();
        // Mocked object will be kept because the agents are already present.
        DcFactory.createDcAgents(SLOT_0);

        assertEquals(mDcUtils, DcFactory.getDcAgent(IDcUtils.class, SLOT_0));
        assertEquals(mDcSettings, DcFactory.getDcAgent(IDcSettings.class, SLOT_0));
        assertEquals(mDcNetWatcher, DcFactory.getDcAgent(IDcNetWatcher.class, SLOT_0));
        assertEquals(mDcApn, DcFactory.getDcAgent(IDcApn.class, SLOT_0));
    }

    @Test
    @SmallTest
    public void testInitDcAgent() throws Exception {
        setUpDcAgents();
        DcFactory.initDcAgents(mContext, SLOT_0);

        verify(mDcUtils).init(eq(mContext));
        verify(mDcSettings).init(eq(mContext));
        verify(mDcNetWatcher).init(eq(mContext));
        verify(mDcApn).init(eq(mContext));
    }

    @Test
    @SmallTest
    public void testCleanUpDc() throws Exception {
        setUpDcAgents();
        DcFactory.cleanUpDcAgents(SLOT_0);

        verify(mDcUtils).cleanup();
        verify(mDcSettings).cleanup();
        verify(mDcNetWatcher).cleanup();
        verify(mDcApn).cleanup();
    }

    private void setUpDcAgents() {
        DcFactory.setDcAgent(IDcUtils.class, mDcUtils, SLOT_0);
        DcFactory.setDcAgent(IDcSettings.class, mDcSettings, SLOT_0);
        DcFactory.setDcAgent(IDcNetWatcher.class, mDcNetWatcher, SLOT_0);
        DcFactory.setDcAgent(IDcApn.class, mDcApn, SLOT_0);
    }
}
