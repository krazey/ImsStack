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
package com.android.imsstack.core.agents;

import android.content.Context;
import android.util.SparseArray;

import androidx.annotation.NonNull;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.imsstack.util.Log;
import com.android.internal.annotations.VisibleForTesting;

import java.io.PrintWriter;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * A factory class for managing and controlling the various agent components.
 */
public final class AgentFactory {
    private DefaultSystemCallAgent mDefaultSystemCallAgent;
    private final SparseArray<SystemCallAgent> mSystemCallAgents;
    private volatile Map<Class<?>, IAgent> mAgents = new LinkedHashMap<>();
    private volatile SparseArray<Map<Class<?>, IAgent>> mAgentsForSlot;
    private static AgentFactory sInstance = null;

    private AgentFactory() {
        int supportedSimCount = DeviceConfig.getSupportedSimCount();
        mSystemCallAgents = new SparseArray<>(supportedSimCount);
        mAgentsForSlot = new SparseArray<>(supportedSimCount);

        for (int i = 0; i < supportedSimCount; ++i) {
            mSystemCallAgents.put(i, null);
            mAgentsForSlot.put(i, new LinkedHashMap<>());
        }
    }

    public static AgentFactory getInstance() {
        if (sInstance == null) {
            sInstance = new AgentFactory();
        }

        return sInstance;
    }

    /**
     * Clears all the resources.
     */
    @VisibleForTesting
    public void clear() {
        mAgents.clear();
        mSystemCallAgents.clear();
        mAgentsForSlot.clear();
    }

    /**
     * Returns a specific {@link IAgent} object corresponding to the given class.
     *
     * @param clazz A requested class name.
     * @return An {@link IAgent} object corresponding to the given class.
     */
    @SuppressWarnings("unchecked")
    public <T extends IAgent> T getAgent(Class<T> clazz) {
        return (T) mAgents.get(clazz);
    }

    /**
     * Sets a specific {@link IAgent} object corresponding to the given class for testing.
     *
     * @param clazz A requested class name.
     * @param agent An {@link IAgent} object corresponding to the given class.
     */
    @VisibleForTesting
    @SuppressWarnings("unchecked")
    public void setAgent(Class<?> clazz, IAgent agent) {
        mAgents.put(clazz, agent);
    }

    /**
     * Returns a specific {@link IAgent} object corresponding to the given class
     * for the specified slot.
     *
     * @param clazz A requested class name.
     * @param slotId A slot id.
     * @return An {@link IAgent} object corresponding to the given class.
     */
    @SuppressWarnings("unchecked")
    public <T extends IAgent> T getAgent(Class<T> clazz, int slotId) {
        if (slotId < 0 || slotId >= mAgentsForSlot.size()) {
            return null;
        }

        Map<Class<?>, IAgent> agents = mAgentsForSlot.get(slotId);
        return (agents != null) ? (T) agents.get(clazz) : null;
    }

    /**
     * Sets a specific {@link IAgent} object corresponding to the given class
     * for the specified slot for testing.
     *
     * @param clazz A requested class name.
     * @param agent An {@link IAgent} object corresponding to the given class.
     * @param slotId A slot id.
     */
    @VisibleForTesting
    @SuppressWarnings("unchecked")
    public void setAgent(Class<?> clazz, IAgent agent, int slotId) {
        if (slotId < 0) {
            return;
        }

        Map<Class<?>, IAgent> agents = mAgentsForSlot.get(slotId);

        if (agents == null) {
            agents = new LinkedHashMap<>();
            mAgentsForSlot.put(slotId, agents);
        }

        agents.put(clazz, agent);
    }

    /**
     * Creates the default agents.
     */
    public void createAgents() {
        if (!mAgents.isEmpty()) {
            return;
        }

        Log.i(this, "createAgents");

        if (mDefaultSystemCallAgent == null) {
            mDefaultSystemCallAgent = new DefaultSystemCallAgent();
        }

        mAgents.put(PreferenceInterface.class, new PreferenceAgent());
        mAgents.put(WakeLockInterface.class, new WakeLockAgent());
        mAgents.put(TimerInterface.class, new TimerAgent());
        mAgents.put(BatteryStateInterface.class, new BatteryStateAgent());
        mAgents.put(WifiInterface.class, new WifiAgent());
        mAgents.put(ImsTrafficInterface.class, new ImsTrafficAgent());
    }

    /**
     * Destroys the default agents.
     */
    public void destroyAgents() {
        Log.i(this, "destroyAgents");

        Collection<IAgent> agentList = reverseCollection(mAgents.values());
        for (IAgent agent : agentList) {
            if (agent != null) {
                agent.cleanup();
            }
        }

        if (mDefaultSystemCallAgent != null) {
            mDefaultSystemCallAgent.destroy();
            mDefaultSystemCallAgent = null;
        }
    }

    /**
     * Initializes the default agents.
     *
     * @param context A {@link Context} object.
     */
    public void initAgents(Context context) {
        Log.i(this, "initAgents");

        if (mDefaultSystemCallAgent == null) {
            mDefaultSystemCallAgent = new DefaultSystemCallAgent();
        }

        for (IAgent agent : mAgents.values()) {
            if (agent != null) {
                agent.init(context);
            }
        }
    }

    /**
     * Creates the agents for the specified slot.
     *
     * @param slotId A slot id.
     */
    public void createAgentsForSlot(int slotId) {
        Map<Class<?>, IAgent> agents = mAgentsForSlot.get(slotId);

        if (agents == null) {
            agents = new LinkedHashMap<>();
            mAgentsForSlot.put(slotId, agents);
        } else if (!agents.isEmpty()) {
            // Already created.
            return;
        }

        Log.i(this, "createAgentsForSlot" + slotId);

        mSystemCallAgents.put(slotId, new SystemCallAgent(slotId));

        agents.put(NativeStateInterface.class, new NativeStateAgent(slotId));
        agents.put(SimInterface.class, new SimAgent(slotId));
        agents.put(ConfigInterface.class, new ConfigAgent(slotId));
        agents.put(PhoneStateInterface.class, new PhoneStateAgent(slotId));
        agents.put(TelephonyInterface.class, new TelephonyAgent(slotId));
        agents.put(LocationInterface.class, new LocationAgent(slotId));
        agents.put(IpSecInterface.class, new IpSecAgent(slotId));
        agents.put(SubsInfoInterface.class, new SubsInfoAgent(slotId));
        agents.put(GbaInterface.class, new GbaAgent(slotId));
        agents.put(ImsRadioInterface.class, new ImsRadioAgent(slotId));
        agents.put(CellInfoInterface.class, new CellInfoAgent(slotId));
        agents.put(EmergencyStateInterface.class, new EmergencyStateAgent(slotId));
    }

    /**
     * Destroys the agents for the specified slot.
     *
     * @param slotId A slot id.
     */
    public void destroyAgentsForSlot(int slotId) {
        Log.i(this, "destroyAgentsForSlot" + slotId);

        Map<Class<?>, IAgent> agents = mAgentsForSlot.get(slotId);

        if (agents != null) {
            Collection<IAgent> agentList = reverseCollection(agents.values());
            for (IAgent agent : agentList) {
                if (agent != null) {
                    agent.cleanup();
                }
            }
        }

        SystemCallAgent sca = mSystemCallAgents.get(slotId);

        if (sca != null) {
            sca.destroy();
        }

        mSystemCallAgents.put(slotId, null);
    }

    /**
     * Initializes the agents for the specified slot.
     *
     * @param context A {@link Context} object.
     * @param slotId A slot id.
     */
    public void initAgentsForSlot(Context context, int slotId) {
        Log.i(this, "initAgentsForSlot" + slotId);

        SystemCallAgent sca = mSystemCallAgents.get(slotId);

        if (sca == null) {
            mSystemCallAgents.put(slotId, new SystemCallAgent(slotId));
        }

        Map<Class<?>, IAgent> agents = mAgentsForSlot.get(slotId);

        if (agents != null) {
            for (IAgent agent : agents.values()) {
                if (agent != null) {
                    agent.init(context);
                }
            }
        }
    }

    /**
     * Dumps this instance into a readable format for dumpsys usage.
     *
     * @param printWriter A {@link PrintWriter} object used to write the formatted logs
     */
    public void dump(@NonNull PrintWriter printWriter) {
        IndentingPrintWriter pw = new IndentingPrintWriter(printWriter, "  ");

        pw.println("Common:");
        pw.increaseIndent();

        for (IAgent agent : mAgents.values()) {
            if (agent != null) {
                agent.dump(pw);
            }
        }

        pw.decreaseIndent();

        for (int i = 0; i < mAgentsForSlot.size(); ++i) {
            int slotId = mAgentsForSlot.keyAt(i);
            pw.printf("Slot%d:\n", slotId);
            pw.increaseIndent();

            Map<Class<?>, IAgent> agents = mAgentsForSlot.get(slotId);
            for (IAgent agent : agents.values()) {
                // ConfigInterface is handled specially outside of this loop.
                if (agent == null || agent instanceof ConfigInterface) {
                    continue;
                }
                agent.dump(pw);
            }

            pw.decreaseIndent();
        }
        pw.println();

        pw.flush();
        pw.println("### Carrier Configs");

        for (int i = 0; i < mAgentsForSlot.size(); ++i) {
            int slotId = mAgentsForSlot.keyAt(i);
            ConfigInterface ci = getAgent(ConfigInterface.class, slotId);
            if (ci != null) {
                ci.dump(pw);
                pw.flush();
            }
        }

        pw.println();
        pw.println("### System Interface");
        if (mDefaultSystemCallAgent != null) {
            mDefaultSystemCallAgent.dump(pw);
        }
    }

    private static Collection<IAgent> reverseCollection(Collection<IAgent> c) {
        return c.stream().collect(
                Collectors.collectingAndThen(
                        Collectors.toList(),
                        l -> {
                                Collections.reverse(l);
                                return l;
                        }
                )
        );
    }
}
