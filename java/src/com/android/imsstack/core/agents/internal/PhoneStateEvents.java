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

/**
 * A class that manages the registered phone state events and a map of its event and
 * {@link IPhoneStateNotifier} object.
 */
public final class PhoneStateEvents {
    /** A default event set for ImsStack. */
    public static final int DEFAULT_EVENTS =
            ImsPhoneStateListener.LISTEN_SERVICE_STATE
            | ImsPhoneStateListener.LISTEN_CALL_STATE
            | ImsPhoneStateListener.LISTEN_BARRING_INFO;

    private static final List<Integer> PS_EVENTS = List.of(
            ImsPhoneStateListener.LISTEN_SERVICE_STATE,
            ImsPhoneStateListener.LISTEN_CALL_STATE,
            ImsPhoneStateListener.LISTEN_PRECISE_CALL_STATE,
            ImsPhoneStateListener.LISTEN_SRVCC_STATE,
            ImsPhoneStateListener.LISTEN_CELL_INFO,
            ImsPhoneStateListener.LISTEN_SIGNAL_STRENGTHS,
            ImsPhoneStateListener.LISTEN_PRECISE_DATA_CONNECTION_STATE,
            ImsPhoneStateListener.LISTEN_BARRING_INFO);
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

    /**
     * Returns the current phone state events that are registered.
     */
    public int getEvents() {
        synchronized (mEventRefs) {
            return mEvents;
        }
    }

    /**
     * Updates the phone state events for the specified notifier.
     *
     * @param events The phone state events to be updated.
     * @param notifier The target notifier.
     * @return {@code true} if the events are updated, {@code false} otherwise.
     */
    public boolean updateEvents(int events, IPhoneStateNotifier notifier) {
        if (notifier == null) {
            return false;
        }

        boolean updated;

        synchronized (mEventRefs) {
            for (int i = 0; i < PS_EVENTS.size(); ++i) {
                Integer event = PS_EVENTS.get(i);

                if (event != null) {
                    setOrRemoveEvent(mEventRefs, events, event.intValue(), notifier);
                }
            }

            int oldEvents = mEvents;
            mEvents = calculateEvents(PS_EVENTS, mEventRefs);
            // Default events are always included
            mEvents |= DEFAULT_EVENTS;

            updated = (oldEvents != mEvents);
        }

        return updated;
    }

    /**
     * Checks whether the specified event is set or not.
     *
     * @param events The total events to be checked.
     * @param event The event to be checked.
     * @return {@code true} if the event is set, {@code false} otherwise.
     */
    public static boolean isEventSet(int events, int event) {
        return (events & event) != 0;
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
        if (isEventSet(events, event)) {
            setEvent(eventRefs, event, notifier);
        } else {
            removeEvent(eventRefs, event, notifier);
        }
    }
}
