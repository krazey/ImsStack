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
import static org.mockito.Mockito.verify;

import android.content.Context;

import com.android.imsstack.core.agents.dcmif.IDc;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.HashMap;

@RunWith(JUnit4.class)
public class DcFactoryTest {
    private static final int SLOT_0 = 0;
    private DcFactory mDcFactory;

    @Mock Context mMockContext;
    @Mock DcUtils mMockDcUtils;
    @Mock DcSettings mMockDcSettings;
    @Mock DcNetWatcher mMockDcNetWatcher;
    @Mock DcApn mMockDcApn;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        // create the instance to test
        mDcFactory = new DcFactory();

    }

    @After
    public void tearDown() throws Exception {
        mDcFactory.setObjects(SLOT_0, null);

        if (mDcFactory != null) {
            mDcFactory = null;
        }
    }

    @Test
    public void testGetDc() throws Exception {
        createMockDcs(SLOT_0);

        assertEquals(mDcFactory.getDc(DcFactory.UTIL, SLOT_0), mMockDcUtils);
        assertEquals(mDcFactory.getDc(DcFactory.SETTING, SLOT_0), mMockDcSettings);
        assertEquals(mDcFactory.getDc(DcFactory.NETWORK_WATCHER, SLOT_0), mMockDcNetWatcher);
        assertEquals(mDcFactory.getDc(DcFactory.APN, SLOT_0), mMockDcApn);
    }

    @Test
    public void testCreateDc() throws Exception {
        mDcFactory.createDc(mMockContext, SLOT_0);

        HashMap<Integer, IDc> agents = mDcFactory.getObjects(SLOT_0);

        assertNotNull(agents);
        assertNotNull(agents.get(DcFactory.UTIL));
        assertNotNull(agents.get(DcFactory.SETTING));
        assertNotNull(agents.get(DcFactory.NETWORK_WATCHER));
        assertNotNull(agents.get(DcFactory.APN));
    }

    @Test
    public void testCleanUpDc() throws Exception {
        createMockDcs(SLOT_0);
        mDcFactory.cleanUpDc(SLOT_0);

        verify(mMockDcUtils).cleanup();
        verify(mMockDcSettings).cleanup();
        verify(mMockDcNetWatcher).cleanup();
        verify(mMockDcApn).cleanup();
    }

    @Test
    public void testInitDc() throws Exception {
        createMockDcs(SLOT_0);
        mDcFactory.initDc(mMockContext, SLOT_0);

        verify(mMockDcUtils).init(mMockContext);
        verify(mMockDcSettings).init(mMockContext);
        verify(mMockDcNetWatcher).init(mMockContext);
        verify(mMockDcApn).init(mMockContext);
    }

    private void createMockDcs(int slotId) {
        HashMap<Integer, IDc> agents = mDcFactory.getObjects(slotId);

        if (agents != null) {
            return;
        }

        agents = new HashMap<Integer, IDc>(DcFactory.MAX);

        agents.put(DcFactory.UTIL, mMockDcUtils);
        agents.put(DcFactory.SETTING, mMockDcSettings);
        agents.put(DcFactory.NETWORK_WATCHER, mMockDcNetWatcher);
        agents.put(DcFactory.APN, mMockDcApn);

        mDcFactory.setObjects(slotId, agents);
    }
}
