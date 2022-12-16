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
package com.android.imsstack.core.agents.internal;

import android.util.ArraySet;
import android.util.SparseArray;

import com.android.imsstack.core.agents.IPhoneStateNotifier;
import com.android.imsstack.core.agents.ImsPhoneStateListener;

import java.util.List;

public final class PhoneStateEvents {
    public static final int LISTEN_NONE = 0;
    public static final int LISTEN_SERVICE_STATE = 0x00000001;
    public static final int LISTEN_CALL_STATE = 0x00000020;
    public static final int LISTEN_SIGNAL_STRENGTHS = 0x00000100;
    public static final int LISTEN_CELL_INFO = 0x00000400;
    public static final int LISTEN_PRECISE_CALL_STATE = 0x00000800;
    public static final int LISTEN_SRVCC_STATE_CHANGED = 0x00004000;
    public static final int LISTEN_PRECISE_DATA_CONNECTION_STATE = 0x00001000;
    public static final int LISTEN_BARRING_INFO = 0x00002000;

    public static final int DEFAULT_EVENTS = (LISTEN_SERVICE_STATE | LISTEN_CALL_STATE
            | LISTEN_BARRING_INFO);

    private static final List<Integer> PS_EVENTS = List.of(
            LISTEN_SERVICE_STATE,
            LISTEN_CALL_STATE,
            LISTEN_PRECISE_CALL_STATE,
            LISTEN_SRVCC_STATE_CHANGED,
            LISTEN_CELL_INFO,
            LISTEN_SIGNAL_STRENGTHS,
            LISTEN_PRECISE_DATA_CONNECTION_STATE,
            LISTEN_BARRING_INFO);
    private final Object mLock = new Object();
    private final SparseArray<ArraySet<IPhoneStateNotifier>> mEventRefs = new SparseArray<>(8);
    private int mEvents = DEFAULT_EVENTS;

    public PhoneStateEvents() {
        for (int i = 0; i < PS_EVENTS.size(); ++i) {
            Integer event = PS_EVENTS.get(i);

            if (event != null) {
                mEventRefs.put(event.intValue(), new ArraySet<IPhoneStateNotifier>());
            }
        }
    }

    public int getEvents() {
        synchronized (mLock) {
            return mEvents;
        }
    }

    public boolean updateEvents(int events, IPhoneStateNotifier notifier) {
        if (notifier == null) {
            return false;
        }

        synchronized (mLock) {
            for (int i = 0; i < PS_EVENTS.size(); ++i) {
                Integer event = PS_EVENTS.get(i);

                if (event != null) {
                    setOrRemoveEvent(mEventRefs, events, event.intValue(), notifier);
                }
            }
        }

        int oldEvents = getEvents();

        synchronized (mLock) {
            mEvents = calculateEvents(PS_EVENTS, mEventRefs);

            // Default events are always included
            mEvents |= DEFAULT_EVENTS;
        }

        return oldEvents != getEvents();
    }

    public static int getEventsFromImsPhoneState(int events) {
        int pslEvents = LISTEN_NONE;

        if (isSet(events, ImsPhoneStateListener.LISTEN_SERVICE_STATE)) {
            pslEvents |= LISTEN_SERVICE_STATE;
        }

        if (isSet(events, ImsPhoneStateListener.LISTEN_CALL_STATE)) {
            pslEvents |= LISTEN_CALL_STATE;
        }

        if (isSet(events, ImsPhoneStateListener.LISTEN_PRECISE_CALL_STATE)) {
            pslEvents |= LISTEN_PRECISE_CALL_STATE;
        }

        if (isSet(events, ImsPhoneStateListener.LISTEN_SRVCC_STATE)) {
            pslEvents |= LISTEN_SRVCC_STATE_CHANGED;
        }

        if (isSet(events, ImsPhoneStateListener.LISTEN_CELL_INFO)) {
            pslEvents |= LISTEN_CELL_INFO;
        }

        if (isSet(events, ImsPhoneStateListener.LISTEN_SIGNAL_STRENGTHS)) {
            pslEvents |= LISTEN_SIGNAL_STRENGTHS;
        }

        if (isSet(events, ImsPhoneStateListener.LISTEN_PRECISE_DATA_CONNECTION_STATE)) {
            pslEvents |= LISTEN_PRECISE_DATA_CONNECTION_STATE;
        }

        if (isSet(events, ImsPhoneStateListener.LISTEN_BARRING_INFO)) {
            pslEvents |= LISTEN_BARRING_INFO;
        }

        return pslEvents;
    }

    private static int calculateEvents(List<Integer> eventList,
            SparseArray<ArraySet<IPhoneStateNotifier>> eventRefs) {
        int events = 0;

        for (int i = 0; i < eventList.size(); ++i) {
            Integer event = eventList.get(i);

            if (event != null) {
                events |= checkAndGetEvent(eventRefs, event.intValue());
            }
        }

        return events;
    }

    private static int checkAndGetEvent(SparseArray<ArraySet<IPhoneStateNotifier>> eventRefs,
            int event) {
        ArraySet<IPhoneStateNotifier> ref = eventRefs.get(event);

        if (ref != null && !ref.isEmpty()) {
            return event;
        }

        return 0;
    }

    private static void removeEvent(SparseArray<ArraySet<IPhoneStateNotifier>> eventRefs,
            int event, IPhoneStateNotifier notifier) {
        ArraySet<IPhoneStateNotifier> ref = eventRefs.get(event);

        if (ref != null) {
            ref.remove(notifier);
        }
    }

    private static void setEvent(SparseArray<ArraySet<IPhoneStateNotifier>> eventRefs,
            int event, IPhoneStateNotifier notifier) {
        ArraySet<IPhoneStateNotifier> ref = eventRefs.get(event);

        if (ref != null) {
            ref.add(notifier);
        }
    }

    private static void setOrRemoveEvent(SparseArray<ArraySet<IPhoneStateNotifier>> eventRefs,
                int events, int event, IPhoneStateNotifier notifier) {
        if (isSet(events, event)) {
            setEvent(eventRefs, event, notifier);
        } else {
            removeEvent(eventRefs, event, notifier);
        }
    }

    private static boolean isSet(int events, int event) {
        return (events & event) != 0;
    }
}
