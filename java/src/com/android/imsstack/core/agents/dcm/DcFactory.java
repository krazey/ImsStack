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
import android.util.SparseArray;

import com.android.imsstack.core.agents.dcmif.IDc;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcSettings;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.util.Log;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * A factory class for managing and accessing the data connection related agents.
 */
public final class DcFactory {
    private static volatile SparseArray<Map<Class<?>, IDc>> sDcAgents = new SparseArray<>(2);

    /**
     * Clears all the resources for the specified slot.
     *
     * @param slotId A slot id.
     */
    @VisibleForTesting
    public static void clear(int slotId) {
        sDcAgents.remove(slotId);
    }

    /**
     * Returns a specific {@link IDc} object corresponding to the given class
     * for the specified slot.
     *
     * @param clazz A requested class name.
     * @param slotId A slot id.
     * @return An {@link IDc} object corresponding to the given class.
     */
    @SuppressWarnings("unchecked")
    public static <T extends IDc> T getDcAgent(Class<T> clazz, int slotId) {
        Map<Class<?>, IDc> agents = sDcAgents.get(slotId);
        return (agents != null) ? (T) agents.get(clazz) : null;
    }

    /**
     * Sets a specific {@link IDc} object corresponding to the given class
     * for the specified slot for testing.
     *
     * @param clazz A requested class name.
     * @param agent An {@link IDc} object corresponding to the given class.
     * @param slotId A slot id.
     */
    @VisibleForTesting
    @SuppressWarnings("unchecked")
    public static void setDcAgent(Class<?> clazz, IDc agent, int slotId) {
        if (slotId < 0) {
            return;
        }

        Map<Class<?>, IDc> agents = sDcAgents.get(slotId);

        if (agents == null) {
            agents = new LinkedHashMap<>();
            sDcAgents.put(slotId, agents);
        }

        agents.put(clazz, agent);
    }

    /**
     * Creates the data connection related agents for the specified slot.
     *
     * @param slotId A slot id.
     */
    public static void createDcAgents(int slotId) {
        Map<Class<?>, IDc> agents = sDcAgents.get(slotId);

        if (agents == null) {
            agents = new LinkedHashMap<>();
            sDcAgents.put(slotId, agents);
        } else if (!agents.isEmpty()) {
            // Already created.
            return;
        }

        Log.i(DcFactory.class, "createDcAgents: slot" + slotId);

        agents.put(IDcUtils.class, new DcUtils(slotId));
        agents.put(IDcSettings.class, new DcSettings(slotId));
        agents.put(IDcNetWatcher.class, new DcNetWatcher(slotId));
        agents.put(IDcApn.class, new DcApn(slotId));
    }

    /**
     * Initiaizes the data connection related agents for the specified slot.
     *
     * @param context The {@link Context} object to initialize.
     * @param slotId A slot id.
     */
    public static void initDcAgents(Context context, int slotId) {
        Log.i(DcFactory.class, "initDcAgents: slot" + slotId);

        Map<Class<?>, IDc> agents = sDcAgents.get(slotId);

        if (agents != null) {
            for (IDc dc : agents.values()) {
                if (dc != null) {
                    dc.init(context);
                }
            }
        }
    }

    /**
     * Cleans up the data connection related agents for the specified slot.
     *
     * @param slotId A slot id.
     */
    public static void cleanUpDcAgents(int slotId) {
        Log.i(DcFactory.class, "cleanUpDcAgents: slot" + slotId);

        Map<Class<?>, IDc> agents = sDcAgents.get(slotId);

        if (agents != null) {
            Collection<IDc> values = reverseCollection(agents.values());
            for (IDc dc : values) {
                if (dc != null) {
                    dc.cleanup();
                }
            }
        }
    }

    private static Collection<IDc> reverseCollection(Collection<IDc> c) {
        return c.stream().collect(
                Collectors.collectingAndThen(
                        Collectors.toList(),
                        l -> {
                            Collections.reverse(l);
                            return l;
                        }
                ));
    }

    private DcFactory() {}
}
