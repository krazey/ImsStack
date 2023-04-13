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
package com.android.imsstack.core.service;

import android.content.Context;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.VoLteFactory;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ILocationAgent;
import com.android.imsstack.core.agents.ILocationAgentManager;
import com.android.imsstack.core.agents.LocationPolicy;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.service.serviceif.IService;
import com.android.imsstack.core.service.serviceif.IVoLteService;
import com.android.imsstack.util.ImsLog;

import java.util.concurrent.ConcurrentHashMap;

public class VoLteService implements IVoLteService {
    protected Context mContext;
    protected int mSlotID = -1;
    private ConcurrentHashMap<Integer, IService> mServics =
            new ConcurrentHashMap<Integer, IService>();

    /**
     * Temporary information
     */
    private String mOperator = "";
    private String mCountry = "";

    public VoLteService(Context context) {
        ImsLog.i("");

        mContext = context;
    }

    /* ---------------------------------------------------------------------------------------------
        implements IVoLteService
    --------------------------------------------------------------------------------------------- */
    @Override
    public void start(int slotID) {
        ImsLog.i("[" + slotID + "]");
        mSlotID = slotID;

        mOperator = ImsGlobal.getOperator(mSlotID);
        mCountry = ImsGlobal.getCountry(mSlotID);

        startServices();
        setOperatorSpecificLogic();
    }

    /* ---------------------------------------------------------------------------------------------
        implements IService
    --------------------------------------------------------------------------------------------- */
    @Override
    public boolean start(IVoLteService voLteService) {
        return true;
    }

    @Override
    public void cleanup(Context context) {
        ImsLog.i("");
        cleanServices();
    }

    @Override
    public void update(Context context) {
        mContext = context;
        updateServices(context);
    }

    /* ---------------------------------------------------------------------------------------------
        implements IVoLteService
    --------------------------------------------------------------------------------------------- */
    @Override
    public Context getContext() {
        return mContext;
    }

    @Override
    public int getSlotID() {
        return mSlotID;
    }

    @Override
    public IService getService(int type) {
        if(!mServics.containsKey(type)) {
            return null;
        }

        return mServics.get(type);
    }

    /* ---------------------------------------------------------------------------------------------
        protected methods
    --------------------------------------------------------------------------------------------- */
    protected void startServices() {
        ImsLog.i("");

        for (int type = TYPE_DEFAULT; type < TYPE_MAX; type++) {
            IService service = mServics.get(type);
            if (service == null) {
                continue;
            }
            if (!service.start(this)) {
                service.cleanup(mContext);
                service = null;
                mServics.remove(type);
            }
        }
    }

    protected void cleanServices() {
        ImsLog.i("");

        for (int type = TYPE_DEFAULT; type < TYPE_MAX; type++) {
            IService service = mServics.get(type);
            if (service == null) {
                continue;
            }
            service.cleanup(mContext);
            service = null;
        }
        mServics.clear();
    }

    protected void updateServices(Context context) {
        ImsLog.i("");

        for (int type = TYPE_DEFAULT; type < TYPE_MAX; type++) {
            IService service = mServics.get(type);
            if (service == null) {
                continue;
            }
            service.update(context);
        }
    }

    protected void setOperatorSpecificLogic() {
        ImsLog.i("");

        ILocationAgentManager lam = (ILocationAgentManager)VoLteFactory.getInstance().getAgent(
                VoLteFactory.AGENT_LOCATION_AGENT_MANAGER);
        ILocationAgent locAgent = (lam != null) ? lam.getAgent(getSlotID()) : null;

        if (locAgent != null) {
            LocationPolicy lp = null;
            int policy = LocationPolicy.POLICY_ENABLE_CACHED_LOCATION
                    | LocationPolicy.POLICY_USE_CACHED_LOCATION;
            int addressResolutionTimeMillis = -1;
            long validityPeriod = -1L;

            if (ImsGlobal.equalsOperatorCountry(mOperator, mCountry, "TMO", "US")) {
                lp = locAgent.getLocationPolicy();

                policy |= LocationPolicy.POLICY_USE_CACHED_ADDRESS;
                policy |= LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_DISTANCE;
                policy |= LocationPolicy.POLICY_LOCATION_UPDATE_USING_SMD;
                policy |= LocationPolicy.POLICY_USE_FLP;
                policy |= LocationPolicy.POLICY_NOTIFY_COUNTRY_CHANGED_EVENT;

                addressResolutionTimeMillis = 1000;
                validityPeriod = LocationPolicy.LOCATION_VALIDITY_PERIOD;

                lp.setAddressValidityPeriod(LocationPolicy.LOCATION_VALIDITY_PERIOD);
                lp.setAddressTolerableDistance(150);
                lp.setSearchDurationForGps(20);
                lp.setShape(LocationPolicy.SHAPE_ELLIPSOID);
            } else if (ImsGlobal.equalsOperatorCountry(mOperator, mCountry, "ATT", "US")) {
                lp = locAgent.getLocationPolicy();

                policy |= LocationPolicy.POLICY_NOTIFY_LOCATION_FIXED_FOR_INSTANT_REQUEST;
                policy |= LocationPolicy.POLICY_NOTIFY_COUNTRY_CHANGED_EVENT;
                policy |= LocationPolicy.POLICY_LOCATION_UPDATE_USING_SMD;
                policy |= LocationPolicy.POLICY_USE_CACHED_ADDRESS;
                policy |= LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_DISTANCE;
                policy |= LocationPolicy.POLICY_UPDATE_COUNTRY_VIA_OTHER_SCHEME;

                SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                        SubsInfoInterface.class, getSlotID());
                if ((subsInfo != null) && subsInfo.isTestModeEnabled()) {
                    // For WFC E911 test which should be tested in Puerto Rico area,
                    // allow mock location
                    policy |= LocationPolicy.POLICY_ALLOW_MOCK_LOCATION_UPDATE;
                }

                addressResolutionTimeMillis = LocationPolicy.ADDRESS_RESOLUTION_RTD_TIME;
                validityPeriod = LocationPolicy.LOCATION_VALIDITY_PERIOD;

                lp.setAddressTolerableDistance(150);
            } else if (ImsGlobal.equalsOperatorCountry(mOperator, mCountry, "VZW", "US")
                    && ImsGlobal.isWfcEnabled(mContext, getSlotID())) {
                lp = locAgent.getLocationPolicy();

                policy |= LocationPolicy.POLICY_LOCATION_NOT_ALLOWED_PERIODIC_POLLING;
                policy |= LocationPolicy.POLICY_INIT_REQUIRED_ON_GETTING_LAST_LOCATION;

                addressResolutionTimeMillis = LocationPolicy.ADDRESS_RESOLUTION_MAX_TIME;
                validityPeriod = LocationPolicy.LOCATION_VALIDITY_PERIOD_SHORT;
            } else if (ImsGlobal.equalsCountry(mCountry, "CA")) {
                lp = locAgent.getLocationPolicy();

                policy |= LocationPolicy.POLICY_INIT_REQUIRED_ON_GETTING_LAST_LOCATION;
                policy |= LocationPolicy.POLICY_LOCATION_UPDATE_USING_SMD;
                policy |= LocationPolicy.POLICY_UPDATE_COUNTRY_VIA_OTHER_SCHEME;
                policy |= LocationPolicy.POLICY_USE_CACHED_ADDRESS;
                policy |= LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_DISTANCE;

                lp.setAddressTolerableDistance(3000);
                lp.setDefaultUpdateInterval(3600);
            } else if (ImsGlobal.isWfcEnabled(mContext, mSlotID)) {
                lp = locAgent.getLocationPolicy();

                policy = LocationPolicy.POLICY_INIT_REQUIRED_ON_GETTING_LAST_LOCATION;
                policy |= LocationPolicy.POLICY_LOCATION_UPDATE_USING_SMD;

                if (isForcedSwitchOnGpsPolicyRequired()) {
                    policy |= LocationPolicy.POLICY_UPDATE_COUNTRY_VIA_OTHER_SCHEME;
                }

                if (ImsGlobal.isOperatorCountry(mSlotID, "VDA" , "AU")) {
                    policy |= LocationPolicy.POLICY_UPDATE_COUNTRY_FROM_USIM;
                }

                if (isCachedLocationRequired()) {
                    policy |= LocationPolicy.POLICY_ENABLE_CACHED_LOCATION;
                    policy |= LocationPolicy.POLICY_USE_CACHED_LOCATION;
                }

                policy |= LocationPolicy.POLICY_USE_CACHED_ADDRESS;
                policy |= LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_DISTANCE;
                policy |= LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_TIME;

                // 200 ms
                lp.setAddressTolerableDistance(200);
                // 10 Min: 10 * 60 * 1000 * 1000000L
                lp.setAddressValidityPeriod(10 * 60 * 1000 * 1000000L);
            }

            if (lp != null) {
                lp.setPolicy(policy);

                if (addressResolutionTimeMillis > 0) {
                    lp.setDefaultAddressResolutionTime(addressResolutionTimeMillis);
                }

                if (validityPeriod > 0) {
                    lp.setValidityPeriod(validityPeriod);
                }

                ImsLog.d(mSlotID, lp.toString());

                locAgent.setLocationPolicy(lp);
            }
        }
    }

    /**
     * Add here if it requires to turn on forcibly the location setting
     */
    private boolean isForcedSwitchOnGpsPolicyRequired() {
        if (ImsGlobal.isOperatorCountry(mSlotID, "O2", "GB")
                || ImsGlobal.isOperatorCountry(mSlotID, "SUN", "SW")
                || ImsGlobal.isOperatorCountry(mSlotID, "VDA" , "AU")
                || ImsGlobal.isOperatorCountry(mSlotID, "OPT" , "AU")
                || ImsGlobal.isCountry(mSlotID, "HK")) {
            return true;
        }

        return false;
    }

    /**
     * Add here if it requires to use chached location for location information
     */
    private boolean isCachedLocationRequired() {
        if (ImsGlobal.isOperatorCountry(mSlotID, "ORG", "FR")
                || ImsGlobal.isOperatorCountry(mSlotID, "EEO", "GB")
                || ImsGlobal.isOperatorCountry(mSlotID, "BT", "GB")) {
            return true;
        }

        return false;
    }
}
