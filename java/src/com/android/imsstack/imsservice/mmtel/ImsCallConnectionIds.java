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

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.util.ImsLog;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public final class ImsCallConnectionIds {
    /**
     * 3GPP TS 22.030
     * "X" is the numbering (starting with 1) of the call given by the sequence of setting up
     * or receiving the calls (active, held or waiting) as seen by the served subscriber.
     * Calls hold their number until they are released.
     * New calls take the lowest available number.
     */
    private static Map<Integer, List<Integer>> sCallConnectionIdTable;

    public static int getNewId(int slotId) {
        List<Integer> callConnectionIds = getConnectionIds(slotId);

        if (callConnectionIds == null) {
            ImsLog.d("[ISIL] No call connection ids");
            return 1;
        }

        if (callConnectionIds.isEmpty()) {
            return 1;
        }

        for (int i = 0; i < callConnectionIds.size(); i++) {
            Integer ccId = callConnectionIds.get(i);

            if (!ccId.equals(Integer.valueOf(i + 1))) {
                return (i + 1);
            }
        }

        Integer ccId = callConnectionIds.get(callConnectionIds.size() - 1);

        return (ccId.intValue() + 1);
    }

    public static void add(int slotId, int ccId) {
        if (ccId > 0) {
            List<Integer> callConnectionIds = getConnectionIds(slotId);

            if (callConnectionIds == null) {
                ImsLog.d("[ISIL] No call connection ids");
                return;
            }

            callConnectionIds.add(Integer.valueOf(ccId));
            Collections.sort(callConnectionIds);

            if (ImsLog.isDebuggable()) {
                displayCallConnectionIds(callConnectionIds, "add");
            }
        }
    }

    public static void remove(int slotId, int ccId) {
        if (ccId > 0) {
            List<Integer> callConnectionIds = getConnectionIds(slotId);

            if (callConnectionIds == null) {
                ImsLog.d("[ISIL] No call connection ids");
                return;
            }

            callConnectionIds.remove(Integer.valueOf(ccId));

            if (ImsLog.isDebuggable()) {
                displayCallConnectionIds(callConnectionIds, "remove");
            }
        }
    }

    public static void removeAll(int slotId) {
        List<Integer> callConnectionIds = getConnectionIds(slotId);

        if (callConnectionIds == null) {
            ImsLog.d("[ISIL] No call connection ids");
            return;
        }

        if (!callConnectionIds.isEmpty()) {
            displayCallConnectionIds(callConnectionIds, "removeAll");
            callConnectionIds.clear();
        }
    }

    private static List<Integer> getConnectionIds(int slotId) {
        if ((slotId < 0) || (slotId >= sCallConnectionIdTable.size())) {
            return null;
        }

        return sCallConnectionIdTable.get(slotId);
    }

    private static void displayCallConnectionIds(List<Integer> ids, String tag) {
        ImsLog.d("[ISIL] ImsCallConnectionIds :: "
                + tag + " - " + ids.toString());
    }

    static {
        sCallConnectionIdTable = new HashMap<Integer, List<Integer>>();
        int supportedSimCount = DeviceConfig.getSupportedSimCount();

        for (int i = 0; i < supportedSimCount; i++) {
            sCallConnectionIdTable.put(i, new ArrayList<Integer>());
        }
    }
}
