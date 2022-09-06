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
    public static final int ALARM_TIMER = 2;
    public static final int BATTERY_STATE = 3;
    public static final int PREFERENCE = 4;
    public static final int WAKE_LOCK = 5;
    public static final int WIFI_STATE = 6;
    public static final int DEVICE = 7;

    // agents with slot id
    public static final int SHARED_STATE = 10;
    public static final int PHONE_STATE = 13;
    public static final int TELEPHONY_STATE = 14;
    public static final int TELEPHONY_SUBSCRIBER = 15;
    public static final int CELL_INFO = 17;
    public static final int PHONE_CALL_DB = 18;
    public static final int IMS_PHONE = 19;
    public static final int CALL_SETTING = 20;

    private static final int AGENT_END = (CALL_SETTING + 1);
    private static final int AGENT_MAX = AGENT_END;

    private static Map<Integer, IAgent> sAgents =
            new HashMap<Integer, IAgent>(AGENT_MAX);

    private static Map<Integer, HashMap<Integer, IAgent>> sAgentSlots =
            new HashMap<Integer, HashMap<Integer, IAgent>>(MSimUtils.getSupportedSimCount());

    private final Object mLock = new Object();
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

    public <T> T getAgent(Class<T> clazz, int slotId) {
        if (slotId < 0 || slotId >= mAgentsForSlot.size()) {
            return null;
        }

        ArrayMap<Class<?>, IAgent> agents = mAgentsForSlot.valueAt(slotId);

        synchronized(mLock) {
            return (T) agents.get(clazz);
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

    @VisibleForTesting
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

    public static synchronized void createAgents(Context context, int slotId) {
        HashMap<Integer, IAgent> agents = sAgentSlots.get(slotId);

        if (agents != null) {
            return;
        }

        agents = new HashMap<Integer, IAgent>(AGENT_MAX);

        agents.put(PHONE_STATE, new PhoneStateAgent(slotId));
        agents.put(TELEPHONY_STATE, new TelephonyStateAgent(slotId));
        agents.put(TELEPHONY_SUBSCRIBER, new TelephonySubscriberAgent(slotId));
        agents.put(PHONE_CALL_DB, new PhoneCallDBAgent(slotId));
        agents.put(SHARED_STATE, new SharedStateAgent(slotId));

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
        agentList.add(agents.get(PHONE_STATE));
        agentList.add(agents.get(TELEPHONY_STATE));
        agentList.add(agents.get(TELEPHONY_SUBSCRIBER));
        agentList.add(agents.get(PHONE_CALL_DB));
        agentList.add(agents.get(SHARED_STATE));

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
        sAgents.put(ALARM_TIMER, AlarmTimerAgent.getInstance());
        sAgents.put(BATTERY_STATE, BatteryStateAgent.getInstance());
        sAgents.put(PREFERENCE, PreferenceAgent.getInstance());
        sAgents.put(WAKE_LOCK, WakeLockAgent.getInstance());
        sAgents.put(WIFI_STATE, WifiStateAgent.getInstance());
        sAgents.put(DEVICE, DeviceAgent.getInstance());
    }

    public static void initAgentsForMIms(Context context, int slotId) {
        HashMap<Integer, IAgent> agents = sAgentSlots.get(slotId);

        if (agents == null) {
            return;
        }

        List<IAgent> agentList = new ArrayList<IAgent>();
        agentList.add(agents.get(PHONE_STATE));
        agentList.add(agents.get(TELEPHONY_STATE));
        agentList.add(agents.get(TELEPHONY_SUBSCRIBER));
        agentList.add(agents.get(PHONE_CALL_DB));
        agentList.add(agents.get(SHARED_STATE));

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
        AlarmTimerAgent.getInstance().init(context);
        BatteryStateAgent.getInstance().init(context);
        PreferenceAgent.getInstance().init(context);
        WakeLockAgent.getInstance().init(context);
        WifiStateAgent.getInstance().init(context);
        DeviceAgent.getInstance().init(context);
    }

    public static void setAgentForMIms(IAgent agent, int agentType, int slotId) {
        HashMap<Integer, IAgent> agents = sAgentSlots.get(slotId);

        if (agents != null) {
            agents.put(agentType, agent);
        }
    }

    public void createAgentsForSlot(int slotId) {
        mSystemCallAgents.put(slotId, new SystemCallAgent(slotId));

        ArrayMap<Class<?>, IAgent> agents = mAgentsForSlot.valueAt(slotId);

        if (agents != null) {
            synchronized(mLock) {
                agents.put(SimInterface.class, new SimAgent(slotId));
                agents.put(ConfigInterface.class, new ConfigAgent(slotId));
                agents.put(IpSecInterface.class, new IpSecAgent(slotId));
                agents.put(SubsInfoInterface.class, new SubsInfoAgent(slotId));
                agents.put(GbaInterface.class, new GbaAgent(slotId));
                agents.put(ImsRadioInterface.class, new ImsRadioAgent(slotId));
            }
        }
    }

    @VisibleForTesting
    public void setAgentsForSlot(IAgent agent, Class<?> agentName, int slotId) {
        ArrayMap<Class<?>, IAgent> agents = mAgentsForSlot.valueAt(slotId);

        if (agents != null) {
            agents.put(agentName, agent);
        }
    }

    public void destroyAgentsForSlot(int slotId) {
        ArrayMap<Class<?>, IAgent> agents = mAgentsForSlot.valueAt(slotId);

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

        ArrayMap<Class<?>, IAgent> agents = mAgentsForSlot.valueAt(slotId);

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
