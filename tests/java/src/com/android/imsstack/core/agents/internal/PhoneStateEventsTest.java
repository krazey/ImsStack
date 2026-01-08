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

package com.android.imsstack.core.agents.internal;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import androidx.test.filters.SmallTest;

import com.android.imsstack.core.agents.IPhoneStateNotifier;
import com.android.imsstack.core.agents.ImsPhoneStateListener;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class PhoneStateEventsTest {
    @Mock private IPhoneStateNotifier mPsNotifier;

    private PhoneStateEvents mPsEvents;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mPsEvents = new PhoneStateEvents();
    }

    @After
    public void tearDown() throws Exception {
        mPsNotifier = null;
        mPsEvents = null;
    }

    @Test
    @SmallTest
    public void testInit() {
        assertEquals(PhoneStateEvents.DEFAULT_EVENTS, mPsEvents.getEvents());
        assertTrue(PhoneStateEvents.isEventSet(PhoneStateEvents.DEFAULT_EVENTS,
                ImsPhoneStateListener.LISTEN_SERVICE_STATE));
        assertFalse(PhoneStateEvents.isEventSet(PhoneStateEvents.DEFAULT_EVENTS,
                ImsPhoneStateListener.LISTEN_CELL_INFO));
    }

    @Test
    @SmallTest
    public void testUpdateEvents() {
        // Start listening for a new event.
        boolean updated = mPsEvents.updateEvents(
                ImsPhoneStateListener.LISTEN_CELL_INFO, mPsNotifier);

        assertTrue(updated);
        assertEquals(PhoneStateEvents.DEFAULT_EVENTS | ImsPhoneStateListener.LISTEN_CELL_INFO,
                mPsEvents.getEvents());

        // Stop listening for previously registered event.
        updated = mPsEvents.updateEvents(ImsPhoneStateListener.LISTEN_NONE, mPsNotifier);

        assertTrue(updated);
        assertEquals(PhoneStateEvents.DEFAULT_EVENTS, mPsEvents.getEvents());

        // Start listening for a default event.
        updated = mPsEvents.updateEvents(ImsPhoneStateListener.LISTEN_SERVICE_STATE, mPsNotifier);

        assertFalse(updated);
        assertEquals(PhoneStateEvents.DEFAULT_EVENTS, mPsEvents.getEvents());

        // Stop listening for a default event.
        updated = mPsEvents.updateEvents(ImsPhoneStateListener.LISTEN_NONE, mPsNotifier);

        assertFalse(updated);
        assertEquals(PhoneStateEvents.DEFAULT_EVENTS, mPsEvents.getEvents());

        // IPhoneStateNotifier is null.
        updated = mPsEvents.updateEvents(ImsPhoneStateListener.LISTEN_SERVICE_STATE, null);

        assertFalse(updated);
    }
}
