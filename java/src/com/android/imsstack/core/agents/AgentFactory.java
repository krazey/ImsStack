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
import android.util.ArrayMap;
import android.util.SparseArray;

import com.android.imsstack.util.MSimUtils;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This class provides the APIs to manage and control Multi-SIM state.
 */
public final class AgentFactory {
    // agents without slot id
    public static final int SUBSCRIPTION = 1;
    public static final int BATTERY_STATE = 2;

    // agents with slot id
    public static final int CELL_INFO = 11;
    public static final int PHONE_CALL_DB = 12;

    private static final int AGENT_END = (PHONE_CALL_DB + 1);
    private static final int AGENT_MAX = AGENT_END;

    private static Map<Integer, IAgent> sAgents =
            new HashMap<Integer, IAgent>(AGENT_MAX);

    private static Map<Integer, HashMap<Integer, IAgent>> sAgentSlots =
            new HashMap<Integer, HashMap<Integer, IAgent>>(MSimUtils.getSupportedSimCount());

    private final Object mLock = new Object();
    private DefaultSystemCallAgent mDefaultSystemCallAgent;
    private final ArrayMap<Class<?>, IAgent> mAgents = new ArrayMap<>();
    private final SparseArray<SystemCallAgent> mSystemCallAgents;
    private final SparseArray<ArrayMap<Class<?>, IAgent>> mAgentsForSlot;
    private static AgentFactory sInstance;

    private AgentFactory() {
        int supportedSimCount = MSimUtils.getSupportedSimCount();
        mSystemCallAgents = new SparseArray<>(supportedSimCount);
        mAgentsForSlot = new SparseArray<>(supportedSimCount);

        for (int i = 0; i < supportedSimCount; ++i) {
            mSystemCallAgents.put(i, null);
            mAgentsForSlot.put(i, new ArrayMap<>());
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
        sAgents.clear();
        sAgentSlots.clear();
    }

    /**
     * Returns the specific agent corresponding to the given class.
     *
     * @param clazz The requested class name
     * @return A {@link IAgent} object corresponding to the given class.
     */
    @SuppressWarnings("unchecked")
    public <T extends IAgent> T getAgent(Class<T> clazz) {
        return (T) mAgents.get(clazz);
    }

    /**
     * Sets the specific agent corresponding to the given class for testing.
     *
     * @param clazz The requested class name
     * @param agent A {@link IAgent} object corresponding to the given class
     */
    @VisibleForTesting
    @SuppressWarnings("unchecked")
    public void setAgent(Class<?> clazz, IAgent agent) {
        mAgents.put(clazz, agent);
    }

    /**
     * Returns the specific agent corresponding to the given class for the specified slot.
     *
     * @param clazz The requested class name
     * @param slotId The slot-id
     * @return A {@link IAgent} object corresponding to the given class.
     */
    @SuppressWarnings("unchecked")
    public <T extends IAgent> T getAgent(Class<T> clazz, int slotId) {
        if (slotId < 0 || slotId >= mAgentsForSlot.size()) {
            return null;
        }

        ArrayMap<Class<?>, IAgent> agents = mAgentsForSlot.get(slotId);

        synchronized(mLock) {
            return (agents != null) ? (T) agents.get(clazz) : null;
        }
    }

    /**
     * Sets the specific agent corresponding to the given class for the specified slot for testing.
     *
     * @param clazz The requested class name
     * @param agent A {@link IAgent} object corresponding to the given class
     * @param slotId The slot-id
     */
    @VisibleForTesting
    @SuppressWarnings("unchecked")
    public void setAgent(Class<?> clazz, IAgent agent, int slotId) {
        if (slotId < 0) {
            return;
        }

        ArrayMap<Class<?>, IAgent> agents = mAgentsForSlot.get(slotId);

        synchronized (mLock) {
            if (agents == null) {
                agents = new ArrayMap<Class<?>, IAgent>();
                mAgentsForSlot.put(slotId, agents);
            }

            agents.put(clazz, agent);
        }
    }

    public static synchronized IAgent getAgent(int agentType) {
        return sAgents.get(agentType);
    }

    @VisibleForTesting
    public static void setDefaultAgent(int agentType, IAgent agent) {
        sAgents.put(agentType, agent);
    }

    public static synchronized IAgent getAgent(int agentType, int slotId) {
        HashMap<Integer, IAgent> agents = sAgentSlots.get(slotId);

        if (agents != null) {
            IAgent agent = agents.get(agentType);
            if (agent != null) {
                return agent;
            }
        }

        return getAgent(agentType);
    }

    public static synchronized void createAgents(Context context, int slotId) {
        HashMap<Integer, IAgent> agents = sAgentSlots.get(slotId);

        if (agents != null) {
            return;
        }

        agents = new HashMap<Integer, IAgent>(AGENT_MAX);

        agents.put(PHONE_CALL_DB, new PhoneCallDBAgent(slotId));

        // below types should be initialized and cleaned up from VoLTE package
        agents.put(CELL_INFO, new CellInfoAgent(slotId));

        sAgentSlots.put(slotId, agents);

        getInstance().createAgentsForSlot(slotId);
    }

    public static synchronized void cleanUpAgents(int slotId) {
        HashMap<Integer, IAgent> agents = sAgentSlots.get(slotId);

        if (agents == null) {
            return;
        }

        List<IAgent> agentList = new ArrayList<IAgent>();
        agentList.add(agents.get(PHONE_CALL_DB));

        for (int i = 0; i < agentList.size(); i++) {
            IAgent agent = agentList.get(i);
            if (agent != null) {
                agent.cleanup();
            }
        }

        getInstance().destroyAgentsForSlot(slotId);
    }

    public static synchronized void createDefaultAgents() {
        sAgents.put(SUBSCRIPTION, SubscriptionAgent.getInstance());
        sAgents.put(BATTERY_STATE, BatteryStateAgent.getInstance());

        getInstance().createAgents();
    }

    public static void initAgentsForMIms(Context context, int slotId) {
        HashMap<Integer, IAgent> agents = sAgentSlots.get(slotId);

        if (agents == null) {
            return;
        }

        List<IAgent> agentList = new ArrayList<IAgent>();
        agentList.add(agents.get(PHONE_CALL_DB));

        for (int i = 0; i < agentList.size(); i++) {
            IAgent agent = agentList.get(i);
            if (agent != null) {
                agent.init(context);
            }
        }

        getInstance().initAgentsForSlot(context, slotId);
    }

    public static void initDefaultAgents(Context context) {
        SubscriptionAgent.getInstance().init(context);
        BatteryStateAgent.getInstance().init(context);

        getInstance().initAgents(context);
    }

    public static void setAgentForMIms(IAgent agent, int agentType, int slotId) {
        HashMap<Integer, IAgent> agents = sAgentSlots.get(slotId);
        if (agents == null) {
            agents = new HashMap<Integer, IAgent>(AGENT_MAX);
            sAgentSlots.put(slotId, agents);
        }
        agents.put(agentType, agent);
    }

    /**
     * Creates the default agents.
     */
    public void createAgents() {
        if (mDefaultSystemCallAgent == null) {
            mDefaultSystemCallAgent = new DefaultSystemCallAgent();
        }

        mAgents.put(PreferenceInterface.class, new PreferenceAgent());
        mAgents.put(WakeLockInterface.class, new WakeLockAgent());
        mAgents.put(TimerInterface.class, new TimerAgent());
        mAgents.put(WifiInterface.class, new WifiAgent());
    }

    /**
     * Destroys the default agents.
     */
    public void destroyAgents() {
        for (int i = 0; i < mAgents.size(); ++i) {
            IAgent agent = mAgents.valueAt(i);

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
     * Initiaizes the default agents.
     *
     * @param context A {@code Context} object.
     */
    public void initAgents(Context context) {
        for (int i = 0; i < mAgents.size(); ++i) {
            IAgent agent = mAgents.valueAt(i);

            if (agent != null) {
                agent.init(context);
            }
        }
    }

    public void createAgentsForSlot(int slotId) {
        mSystemCallAgents.put(slotId, new SystemCallAgent(slotId));

        ArrayMap<Class<?>, IAgent> agents = mAgentsForSlot.get(slotId);

        if (agents != null) {
            synchronized(mLock) {
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
            }
        }
    }

    public void destroyAgentsForSlot(int slotId) {
        ArrayMap<Class<?>, IAgent> agents = mAgentsForSlot.get(slotId);

        if (agents != null) {
            synchronized(mLock) {
                for (int i = 0; i < agents.size(); ++i) {
                    IAgent agent = agents.valueAt(i);

                    if (agent != null) {
                        agent.cleanup();
                    }
                }
            }
        }

        SystemCallAgent sca = mSystemCallAgents.get(slotId);

        if (sca != null) {
            sca.destroy();
        }

        mSystemCallAgents.put(slotId, null);
    }

    public void initAgentsForSlot(Context context, int slotId) {
        SystemCallAgent sca = mSystemCallAgents.get(slotId);

        if (sca == null) {
            mSystemCallAgents.put(slotId, new SystemCallAgent(slotId));
        }

        ArrayMap<Class<?>, IAgent> agents = mAgentsForSlot.get(slotId);

        if (agents != null) {
            synchronized(mLock) {
                for (int i = 0; i < agents.size(); ++i) {
                    IAgent agent = agents.valueAt(i);

                    if (agent != null) {
                        agent.init(context);
                    }
                }
            }
        }
    }
}
