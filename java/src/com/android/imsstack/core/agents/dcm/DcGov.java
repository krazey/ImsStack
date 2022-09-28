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

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ICellInfo;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDc;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ISystemAPINetwork;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.io.FileDescriptor;

/**
 * This is main entry point of data connection governor.
 * Provide each core class data connection associated interface
 * Explicit/External contact point of Data Connection package(DC).
 */
public class DcGov implements IDc, ISystemAPINetwork {

    // Enumerations ----------------------------------------------
    // Internal classes-------------------------------------------
    // Constants--------------------------------------------------
    // Variables--------------------------------------------------
    private int mSlotId = 0;

    // Public methods --------------------------------------------
    public DcGov(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        ImsLog.d(mSlotId, "");

        ISystem system = SystemInterface.getInstance().getSystem(mSlotId);
        if (system != null) {
            system.setISystemAPINetwork(this);
        }
    }

    @Override
    public void cleanup() {
        ISystem system = SystemInterface.getInstance().getSystem(mSlotId);
        if (system != null) {
            system.setISystemAPINetwork(null);
        }
    }

    // Interface implementation methods --------------------------
    @Override
    public int activateDataConnection4Sys(int apnType) {
        ImsLog.i(mSlotId, "apnType = " + apnType);
        IDcApn dcapn = getDcApn();
        return ((dcapn != null) && dcapn.connect(apnType)) ? 1 : 0;
    }

    @Override
    public int deactivateDataConnection4Sys(int apnType) {
        ImsLog.i(mSlotId, "apnType = " + apnType);
        IDcApn dcapn = getDcApn();
        return ((dcapn != null) && dcapn.disconnect(apnType)) ? 1 : 0;
    }

    @Override
    public IDcUtils.AccessNetworkInfo getAccessNetworkInfo4Sys(int defaultNetworkType) {
        ImsLog.d(mSlotId, "");
        IDcUtils dcutil = getDcUtil();
        return (dcutil != null) ? dcutil.getAccessNetworkInfo(defaultNetworkType) : null;
    }

    @Override
    public String getApnName4Sys(int apnType) {
        ImsLog.d(mSlotId, "");
        IDcApn dcapn = getDcApn();
        return (dcapn != null) ? dcapn.getApn(apnType) : "";
    }

    @Override
    public int getDataConnectionState4Sys(int apnType) {
        ImsLog.d(mSlotId, "");
        IDcApn dcapn = getDcApn();
        //EDataState.DATA_STATE_DISCONNECTED        (0)
        return (dcapn != null) ? dcapn.getDataState(apnType) : 0;
    }

    @Override
    public String[] getHostByName4Sys(int apnType, int ipVersion, String host) {
        ImsLog.d(mSlotId, "");
        IDcApn dcapn = getDcApn();
        return (dcapn != null) ? dcapn.getHostByName(apnType, ipVersion, host) : null;
    }

    @Override
    public int getIfaceId4Sys(int apnType) {
        ImsLog.d(mSlotId, "");
        IDcApn dcapn = getDcApn();
        return (dcapn != null) ? dcapn.getIfaceId(apnType) : (-1);
    }

    @Override
    public String getIfaceName4Sys(int apnType) {
        ImsLog.d(mSlotId, "");
        IDcApn dcapn = getDcApn();
        return (dcapn != null) ? dcapn.getIfaceName(apnType) : "";
    }

    @Override
    public int getIpcanCategory4Sys(int apnType) {
        ImsLog.d(mSlotId, "");
        IDcApn dcapn = getDcApn();
        return (dcapn != null) ? dcapn.getIpcanCategory(apnType) : IApn.IPCAN_CATEGORY_MOBILE;
    }

    @Override
    public String[] getLastAccessNetworkInfo4Sys(int networkType) {
        ImsLog.d(mSlotId, "");

        ICellInfo ci = getCellInfo();

        if (ci == null) {
            return null;
        }

        if (networkType <= 0) {
            return ci.getAccessNetworkInfo();
        } else {
            return ci.getAccessNetworkInfo(networkType);
        }
    }

    @Override
    public String getLocalAddress4Sys(int apnType, int ipVersion) {
        ImsLog.d(mSlotId, "");
        IDcApn dcapn = getDcApn();
        return (dcapn != null) ? dcapn.getLocalAddress(apnType, ipVersion) : "";
    }

    @Override
    public String[] getPcscfAddresses4Sys(int apnType, int ipVersion) {
        ImsLog.d(mSlotId, "");
        IDcApn dcapn = getDcApn();
        return (dcapn != null) ? dcapn.getPcscfAddress(apnType, ipVersion) : null;
    }

    @Override
    public int getRoamingState4Sys() {
        IDcNetWatcher dcnw = getDcNetWatcher();
        return ((dcnw != null) && dcnw.isRoaming()) ? 1 : 0;
    }

    @Override
    public int getVoiceRoamingType4Sys() {
        ImsLog.d(mSlotId, "");
        IDcNetWatcher dcnw = getDcNetWatcher();
        if (dcnw == null) {
            return 0;
        }

        return (dcnw.getVoiceRoamingType());
    }

    @Override
    public int getDataRoamingType4Sys() {
        ImsLog.d(mSlotId, "");
        IDcNetWatcher dcnw = getDcNetWatcher();
        if (dcnw == null) {
            return 0;
        }

        return (dcnw.getDataRoamingType());
    }

    @Override
    public int getServiceState4Sys() {
        IDcNetWatcher dcnw = getDcNetWatcher();
        //ServiceState.STATE_OUT_OF_SERVICE = 1
        return (dcnw != null) ? dcnw.getDataServiceState() : 1;
    }

    @Override
    public int getVoiceServiceState4Sys() {
        IDcNetWatcher dcnw = getDcNetWatcher();
        //ServiceState.STATE_OUT_OF_SERVICE = 1
        return (dcnw != null) ? dcnw.getVoiceServiceState() : 1;
    }

    @Override
    public int isLteEmergencyOnly4Sys() {
        IDcNetWatcher dcnw = getDcNetWatcher();
        return ((dcnw != null) && dcnw.isLteEmergencyOnly()) ? 1 : 0;
    }

    @Override
    public int isEmergencyAttachSupported4Sys() {
        IDcNetWatcher dcnw = getDcNetWatcher();
        return ((dcnw != null) && dcnw.isEmergencyServiceSupported()) ? 1 : 0;
    }

    @Override
    public int getMocnPlmnInfo4Sys() {
        IDcNetWatcher dcnw = getDcNetWatcher();
        return (dcnw != null) ? dcnw.getMocnPlmnInfo() : 0;
    }

    @Override
    public boolean isMobileDataEnabled() {
        IDcUtils dcutil = getDcUtil();
        return (dcutil != null) ? dcutil.isMobileDataEnabled() : false;
    }

    @Override
    public int getMtu4Sys(int apnType) {
        IDcApn dcapn = getDcApn();
        return (dcapn != null) ? dcapn.getMtu(apnType) : 0;
    }

    @Override
    public int bindSocket(int apnType, FileDescriptor sockFd) {
        IDcApn dcapn = getDcApn();
        return (dcapn != null) ? dcapn.bindSocket(apnType, sockFd) : 0;
    }

    @VisibleForTesting
    protected IDcApn getDcApn() {
        return (IDcApn) DcFactory.getDc(DcFactory.APN, mSlotId);
    }

    @VisibleForTesting
    protected IDcNetWatcher getDcNetWatcher() {
        return (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
    }

    @VisibleForTesting
    protected IDcUtils getDcUtil() {
        return (IDcUtils) DcFactory.getDc(DcFactory.UTIL, mSlotId);
    }

    @VisibleForTesting
    protected ICellInfo getCellInfo() {
        return (ICellInfo) AgentFactory.getAgent(AgentFactory.CELL_INFO, mSlotId);
    }
}
