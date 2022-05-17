package com.android.imsstack.core.agents;

import android.content.Context;
import android.util.ArrayMap;
import android.util.SparseArray;

import com.android.imsstack.core.agents.agentif.IAgent;
import com.android.imsstack.util.MSimUtils;

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
    public static final int TRM = 8;

    // agents with slot id
    public static final int ISIM = 9;
    public static final int SHARED_STATE = 10;
    public static final int SIM_STATE = 11;
    public static final int PHONE_STATE = 13;
    public static final int TELEPHONY_STATE = 14;
    public static final int TELEPHONY_SUBSCRIBER = 15;
    public static final int GBA = 16;
    public static final int CELL_INFO = 17;
    public static final int PHONE_CALL_DB = 18;
    public static final int IMS_PHONE = 19;
    public static final int CALL_SETTING = 20;
    public static final int VONR = 21;

    private static final int AGENT_END = (VONR + 1);
    private static final int AGENT_MAX = AGENT_END;

    private static Map<Integer, IAgent> sAgents
            = new HashMap<Integer, IAgent>(AGENT_MAX);

    private static Map<Integer, HashMap<Integer, IAgent>> sAgentSlots
            = new HashMap<Integer, HashMap<Integer, IAgent>>(MSimUtils.getMaxSimSlot());

    private final Object mLock = new Object();
    private final SparseArray<SystemCallAgent> mSystemCallAgents;
    private final SparseArray<ArrayMap<Class<?>, IAgent>> mAgentsForSlot;
    private static AgentFactory sInstance;

    private AgentFactory() {
        mSystemCallAgents = new SparseArray<>(MSimUtils.getMaxSimSlot());
        mAgentsForSlot = new SparseArray<>(MSimUtils.getMaxSimSlot());

        for (int i = 0; i < MSimUtils.getMaxSimSlot(); ++i) {
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

        agents.put(PHONE_STATE, new PhoneStateAgent(slotId));
        agents.put(TELEPHONY_STATE, new TelephonyStateAgent(slotId));
        agents.put(TELEPHONY_SUBSCRIBER, new TelephonySubscriberAgent(slotId));
        agents.put(ISIM, new ISIMAgent(slotId));
        agents.put(PHONE_CALL_DB, new PhoneCallDBAgent(slotId));
        agents.put(SHARED_STATE, new SharedStateAgent(slotId));
        agents.put(SIM_STATE, new SIMStateAgent(slotId));

        // below types should be initialized and cleaned up from VoLTE package
        agents.put(CELL_INFO, new CellInfoAgent(slotId));
        agents.put(GBA, new GBAAgent(slotId));
        agents.put(VONR, new VoNRAgent(slotId));

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
        agentList.add(agents.get(ISIM));
        agentList.add(agents.get(PHONE_CALL_DB));
        agentList.add(agents.get(SHARED_STATE));
        agentList.add(agents.get(SIM_STATE));
        agentList.add(agents.get(VONR));

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
        sAgents.put(TRM, TRMAgent.getInstance());
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
        agentList.add(agents.get(ISIM));
        agentList.add(agents.get(PHONE_CALL_DB));
        agentList.add(agents.get(SHARED_STATE));
        agentList.add(agents.get(SIM_STATE));
        agentList.add(agents.get(VONR));

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
        TRMAgent.getInstance().init(context);
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
                agents.put(ConfigInterface.class, new ConfigAgent(slotId));
                agents.put(IpSecInterface.class, new IpSecAgent(slotId));
                agents.put(SubsInfoInterface.class, new SubsInfoAgent(slotId));
            }
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
