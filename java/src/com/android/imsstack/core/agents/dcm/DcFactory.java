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

import android.content.Context;

import com.android.imsstack.core.agents.dcmif.IDc;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This class provides the APIs to manage and control Multi-SIM state.
 */
public final class DcFactory {
    /** DC object identifier. */
    public static final int UTIL = 1;
    public static final int SETTING = 2;
    public static final int NETWORK_WATCHER = 3;
    public static final int APN = 4;
    public static final int MAX = (APN + 1);

    private static Map<Integer, HashMap<Integer, IDc>> sDcSlots =
            new HashMap<Integer, HashMap<Integer, IDc>>(MSimUtils.getSupportedSimCount());

    public DcFactory() {
    }

    /**
     * Get the object of Data Connection Agent
     *
     * @param type Type of object to acquire.
     * @param slotId The slot-id which will be used.
     * @return The object of Data Connection Agent
     */
    public static IDc getDc(int type, int slotId) {
        HashMap<Integer, IDc> dcs = getObjects(slotId);

        if (dcs != null) {
            IDc dc = dcs.get(type);
            if (dc != null) {
                return dc;
            }
        }

        return null;
    }

    /**
     * Create the object of Data Connection Agent
     *
     * @param context Context
     * @param slotId The slot-id which will be used.
     */
    public static void createDc(Context context, int slotId) {
        HashMap<Integer, IDc> agents = getObjects(slotId);

        if (agents != null) {
            return;
        }

        agents = new HashMap<Integer, IDc>(MAX);

        agents.put(UTIL, new DcUtils(slotId));
        agents.put(SETTING, new DcSettings(slotId));
        agents.put(NETWORK_WATCHER, new DcNetWatcher(slotId));
        agents.put(APN, new DcApn(slotId));

        setObjects(slotId, agents);
    }

    /**
     * Clean up each of Data Connection Agent
     *
     * @param slotId The slot-id which will be used.
     */
    public static void cleanUpDc(int slotId) {
        HashMap<Integer, IDc> dcs = getObjects(slotId);

        if (dcs == null) {
            return;
        }

        List<IDc> dcList = new ArrayList<IDc>();
        dcList.add(dcs.get(UTIL));
        dcList.add(dcs.get(SETTING));
        dcList.add(dcs.get(NETWORK_WATCHER));
        dcList.add(dcs.get(APN));

        for (int i = 0; i < dcList.size(); i++) {
            IDc dc = dcList.get(i);
            if (dc != null) {
                dc.cleanup();
            }
        }
    }

    /**
     * Initialize each of Data Connection Agent
     *
     * @param context Context
     * @param slotId The slot-id which will be used.
     */
    public static void initDc(Context context, int slotId) {
        HashMap<Integer, IDc> dcs = getObjects(slotId);

        if (dcs == null) {
            return;
        }

        List<IDc> dcList = new ArrayList<IDc>();
        dcList.add(dcs.get(UTIL));
        dcList.add(dcs.get(SETTING));
        dcList.add(dcs.get(NETWORK_WATCHER));
        dcList.add(dcs.get(APN));

        for (int i = 0; i < dcList.size(); i++) {
            IDc dc = dcList.get(i);
            if (dc != null) {
                dc.init(context);
            }
        }
    }

    /**
     * Get the object include Data Connection Agents for each slot id
     */
    @VisibleForTesting
    public static synchronized HashMap<Integer, IDc> getObjects(int slotId) {
        return sDcSlots.get(slotId);
    }

    /**
     * Set the object include Data Connection Agents for each slot id
     */
    @VisibleForTesting
    public static synchronized void setObjects(int slotId, HashMap<Integer, IDc> objects) {
        sDcSlots.put(slotId, objects);
    }
}
