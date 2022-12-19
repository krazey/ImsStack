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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.when;

import android.telephony.ims.stub.ImsCallSessionImplBase;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class ImsGarbageCallsTest {
    ImsGarbageCalls mImsGarbageCalls;
    private String mCallId;
    private int mSlotId;

    @Before
    public void setUp() throws Exception {
        mImsGarbageCalls = ImsGarbageCalls.getInstance();
        mCallId = "1";
        mSlotId = 0;
    }

    @After
    public void tearDown() throws Exception {
        mImsGarbageCalls.removeAll(0);
        mImsGarbageCalls.removeAll(1);
        mSlotId = 0;
    }

    @Test
    public void testAdd() {
        ImsCallSessionImplBase callSession = Mockito.mock(ImsCallSessionImplBase.class);
        mImsGarbageCalls.add(mSlotId, null);
        when(callSession.getCallId()).thenReturn(mCallId);
        assertFalse(mImsGarbageCalls.contains(callSession));

        mImsGarbageCalls.add(mSlotId, callSession);
        assertTrue(mImsGarbageCalls.contains(callSession));

        mImsGarbageCalls.add(mSlotId, callSession);
        assertEquals(mImsGarbageCalls.getCount(mSlotId), 1);

        ImsCallSessionImplBase callSession1 = Mockito.mock(ImsCallSessionImplBase.class);
        mCallId = "2";
        mSlotId = mSlotId++;
        mImsGarbageCalls.add(mSlotId, null);
        when(callSession1.getCallId()).thenReturn(mCallId);
        assertFalse(mImsGarbageCalls.contains(callSession1));

        mImsGarbageCalls.add(mSlotId, callSession1);
        assertTrue(mImsGarbageCalls.contains(callSession1));

        mImsGarbageCalls.add(mSlotId, callSession1);
        assertEquals(mImsGarbageCalls.getCount(mSlotId), 2);
        mImsGarbageCalls.removeAll(mSlotId);
    }

    @Test
    public void testRemove() {
        ImsCallSessionImplBase callSession = Mockito.mock(ImsCallSessionImplBase.class);
        mImsGarbageCalls.add(mSlotId, callSession);

        ImsCallSessionImplBase callSession1 = Mockito.mock(ImsCallSessionImplBase.class);
        when(callSession1.getCallId()).thenReturn("2");
        mImsGarbageCalls.add(mSlotId, callSession1);
        assertEquals(mImsGarbageCalls.getCount(mSlotId), 2);

        when(callSession.getCallId()).thenReturn("1");
        mImsGarbageCalls.remove(callSession);
        assertEquals(mImsGarbageCalls.getCount(mSlotId), 1);
    }

    @Test
    public void testRemoveAll() {
        ImsCallSessionImplBase callSession = Mockito.mock(ImsCallSessionImplBase.class);
        mImsGarbageCalls.add(mSlotId, callSession);

        ImsCallSessionImplBase callSession1 = Mockito.mock(ImsCallSessionImplBase.class);
        when(callSession1.getCallId()).thenReturn("2");
        mImsGarbageCalls.add(mSlotId, callSession1);
        assertEquals(mImsGarbageCalls.getCount(mSlotId), 2);

        int slotId = 1;
        mImsGarbageCalls.add(slotId, callSession1);
        when(callSession1.getCallId()).thenReturn("3");
        mImsGarbageCalls.add(slotId, callSession1);

        mImsGarbageCalls.removeAll(slotId);
        assertEquals(mImsGarbageCalls.getCount(slotId), 0);

        mImsGarbageCalls.removeAll(mSlotId);
        assertEquals(mImsGarbageCalls.getCount(mSlotId), 0);
    }
}
