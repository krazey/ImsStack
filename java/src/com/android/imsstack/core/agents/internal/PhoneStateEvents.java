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

import com.android.imsstack.core.agents.IPhoneStateNotifier;
import com.android.imsstack.core.agents.ImsPhoneStateListener;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;

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

    // IMS extended events
    public static final int LISTEN_PCSCF_ADDRESS_INFO = 0x00000001;

    private static final int MAX_EVENTS = 8;
    private static final int MAX_IMS_EVENTS = 1;

    private static ArrayList<Integer> sPSEvents;
    private static ArrayList<Integer> sImsEvents;

    private final Object mLock = new Object();
    private final HashMap<Integer, HashSet<IPhoneStateNotifier>> mEventRefs
            = new HashMap<Integer, HashSet<IPhoneStateNotifier>>(MAX_EVENTS, 0.9f);
    private final HashMap<Integer, HashSet<IPhoneStateNotifier>> mImsEventRefs
            = new HashMap<Integer, HashSet<IPhoneStateNotifier>>(MAX_IMS_EVENTS, 0.9f);
    private int mEvents = DEFAULT_EVENTS;
    private int mImsEvents = 0;

    public PhoneStateEvents() {
        for (int i = 0; i < sPSEvents.size(); ++i) {
            Integer event = sPSEvents.get(i);

            if (event != null) {
                mEventRefs.put(event.intValue(), new HashSet<IPhoneStateNotifier>());
            }
        }

        for (int i = 0; i < sImsEvents.size(); ++i) {
            Integer event = sImsEvents.get(i);

            if (event != null) {
                mImsEventRefs.put(event.intValue(), new HashSet<IPhoneStateNotifier>());
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
            for (int i = 0; i < sPSEvents.size(); ++i) {
                Integer event = sPSEvents.get(i);

                if (event != null) {
                    setOrRemoveEvent(mEventRefs, events, event.intValue(), notifier);
                }
            }
        }

        int oldEvents = getEvents();

        synchronized (mLock) {
            mEvents = calculateEvents(sPSEvents, mEventRefs);

            // Default events are always included
            mEvents |= DEFAULT_EVENTS;
        }

        return oldEvents != getEvents();
    }

    public int getImsEvents() {
        synchronized (mLock) {
            return mImsEvents;
        }
    }

    public boolean updateImsEvents(int events, IPhoneStateNotifier notifier) {
        if (notifier == null) {
            return false;
        }

        synchronized (mLock) {
            for (int i = 0; i < sImsEvents.size(); ++i) {
                Integer event = sImsEvents.get(i);

                if (event != null) {
                    setOrRemoveEvent(mImsEventRefs, events, event.intValue(), notifier);
                }
            }
        }

        int oldImsEvents = getImsEvents();

        synchronized (mLock) {
            mImsEvents = calculateEvents(sImsEvents, mImsEventRefs);
        }

        return oldImsEvents != getImsEvents();
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

    public static int getImsEventsFromImsPhoneState(int events) {
        int imsEvents = 0;

        if (isSet(events, ImsPhoneStateListener.LISTEN_PCSCF_ADDRESS_INFO)) {
            imsEvents |= LISTEN_PCSCF_ADDRESS_INFO;
        }

        return imsEvents;
    }

    private static int calculateEvents(ArrayList<Integer> eventList,
            HashMap<Integer, HashSet<IPhoneStateNotifier>> eventRefs) {
        int events = 0;

        for (int i = 0; i < eventList.size(); ++i) {
            Integer event = eventList.get(i);

            if (event != null) {
                events |= checkAndGetEvent(eventRefs, event.intValue());
            }
        }

        return events;
    }

    private static int checkAndGetEvent(HashMap<Integer, HashSet<IPhoneStateNotifier>> eventRefs,
            int event) {
        HashSet<IPhoneStateNotifier> ref = eventRefs.get(event);

        if (ref != null && !ref.isEmpty()) {
            return event;
        }

        return 0;
    }

    private static void removeEvent(HashMap<Integer, HashSet<IPhoneStateNotifier>> eventRefs,
            int event, IPhoneStateNotifier notifier) {
        HashSet<IPhoneStateNotifier> ref = eventRefs.get(event);

        if (ref != null) {
            ref.remove(notifier);
        }
    }

    private static void setEvent(HashMap<Integer, HashSet<IPhoneStateNotifier>> eventRefs,
            int event, IPhoneStateNotifier notifier) {
        HashSet<IPhoneStateNotifier> ref = eventRefs.get(event);

        if (ref != null) {
            ref.add(notifier);
        }
    }

    private static void setOrRemoveEvent(HashMap<Integer, HashSet<IPhoneStateNotifier>> eventRefs,
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

    static {
        sPSEvents = new ArrayList<Integer>(MAX_EVENTS);
        sPSEvents.add(LISTEN_SERVICE_STATE);
        sPSEvents.add(LISTEN_CALL_STATE);
        sPSEvents.add(LISTEN_PRECISE_CALL_STATE);
        sPSEvents.add(LISTEN_SRVCC_STATE_CHANGED);
        sPSEvents.add(LISTEN_CELL_INFO);
        sPSEvents.add(LISTEN_SIGNAL_STRENGTHS);
        sPSEvents.add(LISTEN_PRECISE_DATA_CONNECTION_STATE);
        sPSEvents.add(LISTEN_BARRING_INFO);

        // IMS extended events
        sImsEvents = new ArrayList<Integer>(MAX_IMS_EVENTS);
        sImsEvents.add(LISTEN_PCSCF_ADDRESS_INFO);
    }
}
