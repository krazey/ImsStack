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
package com.android.imsstack.core;

import android.content.Context;

import com.android.imsstack.core.agents.IVoLteAgent;
import com.android.imsstack.core.agents.LocationAgentManager;
import com.android.imsstack.core.agents.PhoneNumberAgent;
import com.android.imsstack.core.service.VoLteService;
import com.android.imsstack.core.service.serviceif.IVoLteService;
import com.android.imsstack.test.ImsTestHelper;
import com.android.imsstack.test.ImsTestMode;
import com.android.imsstack.util.ImsLog;

import java.util.concurrent.ConcurrentHashMap;

public class VoLteFactory {

    public static final int AGENT_DEFAULT = 0;
    public static final int AGENT_PHONENUMBER = 1;
    public static final int AGENT_LOCATION_AGENT_MANAGER = 2;
    public static final int AGENT_MAX = 3;

    private Context mContext = null;

    private static VoLteFactory sVoLteFactory = null;
    private static ConcurrentHashMap<Integer, IVoLteService> sVoLteServices
                                                = new ConcurrentHashMap<Integer, IVoLteService>();
    private static ConcurrentHashMap<Integer, IVoLteAgent> sAgents
                                                = new ConcurrentHashMap<Integer, IVoLteAgent>();


    /* ---------------------------------------------------------------------------------------------
        public methods
    --------------------------------------------------------------------------------------------- */
    public static VoLteFactory getInstance() {
        if (sVoLteFactory == null) {
            sVoLteFactory = new VoLteFactory();
        }

        return sVoLteFactory;
    }

    public IVoLteService getService(int slotID) {
        if (!sVoLteServices.containsKey(slotID)) {
            return null;
        }

        return sVoLteServices.get(slotID);
    }

    public IVoLteAgent getAgent(int agentType) {
        if (!sAgents.containsKey(agentType)) {
            return null;
        }

        return sAgents.get(agentType);
    }

    public void startService(Context context, int slotID) {
        ImsLog.d("size[" + sVoLteServices.size() + "] " + "slotID[" + slotID + "]");

        init(context);

        if (ImsTestMode.getInstance().getTestMode(slotID).isGenericTestMode()) {
            ImsTestHelper.getInstance();
        }

        if (!sVoLteServices.containsKey(slotID)) {
            IVoLteService voLteService = serviceFactory(slotID);
            sVoLteServices.put(slotID, voLteService);
            voLteService.start(slotID);
        } else {
            ImsLog.d("already started");
        }
    }

    public void stopService(int slotID) {
        if (sVoLteServices.containsKey(slotID)) {
            IVoLteService voLteService = sVoLteServices.get(slotID);
            voLteService.cleanup(mContext);
            voLteService = null;
            sVoLteServices.remove(slotID);
        } else {
            ImsLog.d("already stoped");
        }

        if (ImsTestMode.getInstance().getTestMode(slotID).isGenericTestMode()) {
            ImsTestHelper.getInstance().cleanup();
        }

        stopAgents(slotID);

        ImsLog.d("size[" + sVoLteServices.size() + "] " + "slotID[" + slotID + "]");
    }

    public void updateService(int slotID) {
        if (!sVoLteServices.containsKey(slotID)) {
            ImsLog.d("VoLteService is not started; slotId=" + slotID);
            return;
        }

        IVoLteService voLteService = sVoLteServices.get(slotID);

        if (voLteService != null) {
            voLteService.update(mContext);
        }
    }

    public Context getContext() {
        if (mContext == null) {
            ImsLog.e("VoLteService is not started");
        }

        return mContext;
    }

    /* ---------------------------------------------------------------------------------------------
        private methods
    --------------------------------------------------------------------------------------------- */
    private VoLteFactory() {
        ImsLog.i("");
    }

    private void init(Context context) {
        ImsLog.d("");

        if (!sVoLteServices.isEmpty()) {
            ImsLog.d("already started");
            return;
        }

        mContext = context;

        initAgents();
    }

    public void clear() {
        if (mContext == null) {
            return;
        }

        clearAgents();
    }

    private void initAgents() {
        ImsLog.d("");

        if (!sAgents.isEmpty()) {
            return;
        }

        sAgents.put(AGENT_PHONENUMBER, PhoneNumberAgent.getInstance());
        sAgents.put(AGENT_LOCATION_AGENT_MANAGER, new LocationAgentManager(mContext));
    }

    private void clearAgents() {
        for (int agentKey = AGENT_DEFAULT; agentKey < AGENT_MAX; agentKey++) {
            IVoLteAgent volteAgent = sAgents.get(agentKey);
            if (volteAgent != null) {
                volteAgent.cleanup();
            }
        }
        sAgents.clear();
    }

    private void startAgents(int slotID) {
        ImsLog.d("slotID[" + slotID + "]");

        for (int agentKey = AGENT_DEFAULT; agentKey < AGENT_MAX; agentKey++) {
            IVoLteAgent volteAgent = sAgents.get(agentKey);
            if (volteAgent != null) {
                volteAgent.start(slotID);
            }
        }
    }

    private void stopAgents(int slotID) {
        ImsLog.d("slotID[" + slotID + "]");

        for (int agentKey = AGENT_DEFAULT; agentKey < AGENT_MAX; agentKey++) {
            IVoLteAgent volteAgent = sAgents.get(agentKey);
            if (volteAgent != null) {
                volteAgent.stop(slotID);
            }
        }
    }

    /*
     * NOTICE
     *
     * If New BootupGov class for target operator is added in service package,
     * add branching statement to make target operator's service object in factory.
     */
    private IVoLteService serviceFactory(int slotID) {
        ImsLog.i("");

        return new VoLteService(mContext);
    }
}
