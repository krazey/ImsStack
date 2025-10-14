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
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.Network;
import android.os.Message;
import android.telephony.PreciseDataConnectionState;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.EIpVersion;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.io.FileDescriptor;
import java.io.IOException;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.ConcurrentModificationException;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Set;

/**
 * This class provides followings.
 *  1. Factory of Apn objects which inherit IApn type.
 *  2. Apn object invoker according to the "apn type"
 *  3. Logical APN core logic of data connection package.
 */
public class DcApn implements IDcApn {

    // Variables--------------------------------------------------
    private final Object mLock = new Object();
    private final int mSlotId;
    private int mSubId = MSimUtils.INVALID_SUB_ID;

    @VisibleForTesting
    protected Context mContext;

    // APN controllers
    @VisibleForTesting
    protected LinkedHashMap<Integer, IApn> mMapApn = new LinkedHashMap<Integer, IApn>();

    // Local IP addresses
    @VisibleForTesting
    protected LinkedHashMap<Integer, String> mMapLocalIP = new LinkedHashMap<Integer, String>();

    @VisibleForTesting
    protected PreciseDcStateListener mPreciseDcStateListener = null;
    private Sim.Listener mSimListener;

    // Public methods --------------------------------------------
    public DcApn(int slotId) {
        ImsLog.d(slotId, "");
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        ImsLog.d(mSlotId, "");

        if (context == null) {
            return;
        }

        mContext = context;

        apnFactory();

        mSimListener = new Sim.Listener() {
                @Override
                public void onSimStateChanged() {
                    updateSubscription();
                }
            };
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (sim != null) {
            sim.addListener(mSimListener);
        }

        mPreciseDcStateListener = new PreciseDcStateListener();
        mSubId = MSimUtils.getSubId(mSlotId);
        if (mSubId != MSimUtils.INVALID_SUB_ID) {
            TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(mSubId);
            mPreciseDcStateListener.register(tmp);
        }
    }

    @Override
    public void cleanup() {
        ImsLog.d(mSlotId, "");

        if (mSimListener != null) {
            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
            if (sim != null) {
                sim.removeListener(mSimListener);
            }
            mSimListener = null;
        }

        if (mPreciseDcStateListener != null) {
            mPreciseDcStateListener.unregister();
            mPreciseDcStateListener = null;
        }

        cleanUpApns();
        mMapLocalIP.clear();
    }

    @Override
    public boolean connect(int apnType) {
        if (!isNativeServiceReady()) {
            ImsLog.w(mSlotId, "native is not ready, apnType = " + apnType);
            return false;
        }

        // Finally, we can get APN.
        IApn iApn = getApnControl(apnType);

        if (iApn == null) {
            ImsLog.w(mSlotId, "apn is null, apnType = " + apnType);
            return false;
        }

        if (apnType == EApnType.EMERGENCY.getType() && mSubId == MSimUtils.INVALID_SUB_ID) {
            TelephonyManagerProxy tmp = AppContext.getInstance()
                    .getSystemServiceProxy(TelephonyManagerProxy.class);
            mPreciseDcStateListener.register(tmp);
        }

        return iApn.connect();
    }

    @Override
    public boolean disconnect(int apnType) {
        // get proper apn controller
        IApn iApn = getApnControl(apnType);

        if (iApn == null) {
            ImsLog.w(mSlotId, "apn is null, apnType = " + apnType);
            return false;
        }

        return iApn.disconnect();
    }

    @Override
    public int getDataState(int apnType) {
        IApn apn = getApnControl(apnType);

        if (apn == null) {
            return EDataState.convertFromTMtoImsType(TelephonyManager.DATA_DISCONNECTED);
        }

        return EDataState.convertFromTMtoImsType(apn.getDataState());
    }

    @Override
    public boolean isConnected(int apnType) {
        IApn apn = getApnControl(apnType);

        if (apn == null) {
            return false;
        }

        return apn.isConnected();
    }

    @Override
    public String getApn(int apnType) {
        IApn apn = getApnControl(apnType);

        if (apn == null) {
            return null;
        }

        ImsLog.i(mSlotId, "apn :: apnType=" + apnType + ", name=" + apn.getApn());

        return apn.getApn();
    }

    @Override
    public String[] getHostByName(int apnType, int ipVersion, String host) {
        if (mContext == null) {
            return null;
        }

        Network nw = getNetworkByCapability(apnType);

        if (nw == null) {
            ImsLog.w(mSlotId, "Network is null, apnType = " + apnType);
            return null;
        }

        ImsLog.i(mSlotId, "Network :: netId=" + nw.getNetId());

        InetAddress[] inetAddrs = null;

        try {
            inetAddrs = nw.getAllByName(host);
        } catch (UnknownHostException e) {
            ImsLog.e(mSlotId, "UnknownHostException :: " + e.toString());
        }

        if (inetAddrs == null) {
            ImsLog.w(mSlotId, "InetAddress[] is null, apnType = " + apnType);
            return null;
        }

        if (inetAddrs.length == 0) {
            ImsLog.w(mSlotId, "InetAddress[] is zero-length, apnType = " + apnType);
            return null;
        }

        ArrayList<String> ipAddrs = new ArrayList<String>();

        if (ipVersion == EIpVersion.IPV4.getInt()) {
            for (int i = 0; i < inetAddrs.length; ++i) {
                if (inetAddrs[i] instanceof Inet4Address) {
                    ipAddrs.add(inetAddrs[i].getHostAddress());
                }
            }
        } else {
            for (int i = 0; i < inetAddrs.length; ++i) {
                if (inetAddrs[i] instanceof Inet6Address) {
                    ipAddrs.add(inetAddrs[i].getHostAddress());
                }
            }
        }

        ImsLog.i(mSlotId, "apnType=" + apnType + ", ipVersion=" + ipVersion
                + ", size=" + ipAddrs.size());

        if (ipAddrs.isEmpty()) {
            return null;
        }

        return ipAddrs.toArray(new String[ipAddrs.size()]);
    }

    @Override
    public int getIfaceId(int apnType) {
        if (mContext == null) {
            return (-1);
        }

        Network nw = getNetworkByCapability(apnType);

        if (nw == null) {
            ImsLog.w(mSlotId, "Network is null, apnType = " + apnType);
            return (-1);
        }

        ImsLog.i(mSlotId, "Network :: apnType=" + apnType + ", netId=" + nw.getNetId());

        return nw.getNetId();
    }

    @Override
    public String getIfaceName(int apnType) {
        LinkProperties linkProperties = getLinkProperties(apnType);

        if (linkProperties == null) {
            ImsLog.w(mSlotId, "LinkProperties is null, apnType = " + apnType);
            return null;
        }

        String ifaceName = linkProperties.getInterfaceName();
        if (ifaceName == null) {
            ImsLog.w(mSlotId, "ifaceName is null, apnType = " + apnType);
            return null;
        }

        ImsLog.i(mSlotId, "NetworkInterface :: apnType=" + apnType + ", name=" + ifaceName);

        return ifaceName;
    }

    @Override
    public int getIpcanCategory(int apnType) {
        IApn apn = getApnControl(apnType);
        return (apn != null) ? apn.getIpcanCategory() : IApn.IPCAN_CATEGORY_MOBILE;
    }

    @Override
    public String getLocalAddress(int apnType, int ipVersion) {
        boolean ipCacheRequired = false;

        // ipVersion with -1 is for caching ip address
        // and selecting ip version based on configuration in java side
        if (ipVersion == -1) {
            ImsLog.w(mSlotId, "cache is required");
            ipCacheRequired = true;
            ipVersion = 0;
        }

        if (ipVersion == 0) {
            // Configuration-based ip selection
            ipVersion = getIpVersion(apnType);
        }

        LinkProperties linkProperties = getLinkProperties(apnType);
        if (linkProperties == null) {
            ImsLog.w(mSlotId, "LinkProperties is null, apnType = " + apnType);
            return null;
        }

        Collection<LinkAddress> linkAddresses = linkProperties.getLinkAddresses();
        if (linkAddresses.isEmpty()) {
            ImsLog.w(mSlotId, "LinkAddresses is empty, apnType = " + apnType);
            return null;
        }

        ImsLog.i(mSlotId, "LinkAddress :: apnType=" + apnType + ", size=" + linkAddresses.size());

        String ip = getIpAddress(linkAddresses, ipVersion);

        if (ipCacheRequired) {
            if (ip == null) {
                mMapLocalIP.remove(apnType);
            } else {
                mMapLocalIP.put(apnType, ip);
            }
        }

        return ip;
    }

    @Override
    public String getCachedLocalAddress(int apnType) {
        return mMapLocalIP.get(apnType);
    }

    @Override
    public String[] getPcscfAddress(int apnType, int ipVersion) {
        LinkProperties linkProperties = getLinkProperties(apnType);
        if (linkProperties == null) {
            ImsLog.w(mSlotId, "LinkProperties is null, apnType = " + apnType);
            return null;
        }

        Collection<InetAddress> pcscfAddresses = linkProperties.getPcscfServers();
        if (pcscfAddresses == null || pcscfAddresses.isEmpty()) {
            ImsLog.d(mSlotId, "pcscfAddresses is empty");
            return null;
        }

        int preferredIpVersion = ipVersion;
        if (ipVersion == 0) {
            // Configuration-based ip selection
            ipVersion = getIpVersion(apnType);
            if (ipVersion == EIpVersion.IPV4V6.getInt() || ipVersion == EIpVersion.IPV4.getInt()) {
                preferredIpVersion = EIpVersion.IPV4.getInt();
            } else {
                preferredIpVersion = EIpVersion.IPV6.getInt();
            }
        }

        ImsLog.i(mSlotId, "P-CSCF address::apnType=" + EApnType.getApnSettingTypeFromType(apnType)
                + ", ipVersion=" + ipVersion + ", preferred ipVersion=" + preferredIpVersion);

        String[] validAddresses = getValidPcscf(pcscfAddresses, preferredIpVersion);
        Collection<LinkAddress> linkAddresses = linkProperties.getLinkAddresses();
        String ipAddress = getIpAddress(linkAddresses, preferredIpVersion);

        if (validAddresses == null || validAddresses.length == 0 || ipAddress == null) {
            if (ipVersion == EIpVersion.IPV4V6.getInt()) {
                String ipv6LinkAddress = getIpAddress(linkAddresses, EIpVersion.IPV6.getInt());
                if (ipv6LinkAddress != null) {
                    validAddresses = getValidPcscf(pcscfAddresses, EIpVersion.IPV6.getInt());
                }
            } else if (ipVersion == EIpVersion.IPV6V4.getInt()) {
                String ipv4LinkAddress = getIpAddress(linkAddresses, EIpVersion.IPV4.getInt());
                if (ipv4LinkAddress != null) {
                    validAddresses = getValidPcscf(pcscfAddresses, EIpVersion.IPV4.getInt());
                }
            }
        }

        return validAddresses;
    }


    @Override
    public int getMtu(int apnType) {
        LinkProperties linkProperties = getLinkProperties(apnType);

        if (linkProperties == null) {
            ImsLog.w(mSlotId, "LinkProperties is null, apnType = " + apnType);
            return 0;
        }

        ImsLog.i(mSlotId, "MTU = " + linkProperties.getMtu() + ", apnType = " + apnType);

        return linkProperties.getMtu();
    }

    @Override
    public boolean bindSocket(int apnType, FileDescriptor sockFd) {
        Network network = getNetworkByCapability(apnType);

        ImsLog.d(mSlotId, "bindSocket: network=" + network);

        if (network != null && sockFd != null) {
            try {
                network.bindSocket(sockFd);
                return true;
            } catch (IOException e) {
                ImsLog.e(mSlotId, "bindSocket: " + e.toString());
            }
        }

        return false;
    }

    @Override
    public void setApn(int apnType, IApn apn) {
        ImsLog.i(mSlotId, "apnType : " + apnType);

        if (apn == null) {
            ImsLog.w(mSlotId, "Apn is null");
            return;
        }

        mMapApn.put(apnType, apn);
    }

    @Override
    public IApn getApnControl(int apnType) {
        IApn apn = mMapApn.get(apnType);

        if (apn == null) {
            ImsLog.w(mSlotId, "IApn is null; type=" + apnType);
            return null;
        }
        return apn;
    }

    @Override
    public Network getNetworkByCapability(int apnType) {
        IApn apn = getApnControl(apnType);
        return (apn != null) ? apn.getCachedNetwork() : null;
    }

    @Override
    public boolean isIpv6Preferred(int apnType) {
        IApn apn = getApnControl(apnType);

        if (apn == null) {
            return false;
        }

        return apn.getIpVersion() == EIpVersion.IPV6V4.getInt();
    }

    // Interface implementation methods --------------------------
    // Private/Protected methods ---------------------------------
    private void apnFactory() {
        if (mContext == null) {
            return;
        }

        mMapApn.put(EApnType.IMS.getType(), new ApnIms(mContext, mSlotId));
        mMapApn.put(EApnType.EMERGENCY.getType(), new ApnEmergency(mContext, mSlotId));
        mMapApn.put(EApnType.XCAP.getType(), new ApnXcap(mContext, mSlotId));
        mMapApn.put(EApnType.INTERNET.getType(), new ApnInternet(mContext, mSlotId));
    }

    private void cleanUpApns() {
        Set<Integer> apnList = mMapApn.keySet();
        Iterator<Integer> iterator = apnList.iterator();

        while (iterator.hasNext()) {
            Integer key;
            try {
                key = iterator.next();
            } catch (ConcurrentModificationException e) {
                mMapApn.clear();
                return;
            }

            IApn apn = mMapApn.get(key);
            if (apn != null) {
                apn.disconnect();
                apn.cleanup();
            }
        }

        mMapApn.clear();
    }

    private LinkProperties getLinkProperties(int apnType) {
        ConnectivityManagerProxy cmp =
                AppContext.getInstance().getSystemServiceProxy(ConnectivityManagerProxy.class);
        Network network = getNetworkByCapability(apnType);
        return network != null ? cmp.getLinkProperties(network) : null;
    }

    private String getIpAddress(Collection<LinkAddress> linkAddresses, int ipVersion) {
        if (linkAddresses == null) {
            return null;
        }

        Iterator<LinkAddress> iterator = linkAddresses.iterator();

        if (iterator == null) {
            return null;
        }

        String ipAddress = null;

        while (iterator.hasNext()) {
            LinkAddress linkAddress = iterator.next();

            if (linkAddress == null) {
                ImsLog.w(mSlotId, "linkAddress is null");
                continue;
            }

            InetAddress netAddress = linkAddress.getAddress();

            if (netAddress == null) {
                ImsLog.w(mSlotId, "Resolving InetAddress failed - " + linkAddress.toString());
                continue;
            }

            if (netAddress.isAnyLocalAddress()
                    || netAddress.isLinkLocalAddress()
                    || netAddress.isLoopbackAddress()) {
                ImsLog.w(mSlotId, "Invalid InetAddress - " + linkAddress.toString());
                continue;
            }

            if ((ipVersion == EIpVersion.IPV6.getInt())
                    || (ipVersion == EIpVersion.IPV6V4.getInt())) {
                if (netAddress instanceof Inet6Address) {
                    ipAddress = netAddress.getHostAddress();
                    return ipAddress;
                } else if (netAddress instanceof Inet4Address) {
                    if (ipVersion == EIpVersion.IPV6V4.getInt()) {
                        ipAddress = netAddress.getHostAddress();
                    }
                }
            } else if ((ipVersion == EIpVersion.IPV4.getInt())
                    || (ipVersion == EIpVersion.IPV4V6.getInt())) {
                if (netAddress instanceof Inet4Address) {
                    ipAddress = netAddress.getHostAddress();
                    return ipAddress;
                } else if (netAddress instanceof Inet6Address) {
                    if (ipVersion == EIpVersion.IPV4V6.getInt()) {
                        ipAddress = netAddress.getHostAddress();
                    }
                }
            }
        }

        return ipAddress;
    }

    private String[] getValidPcscf(Collection<InetAddress> inetAddresses, int ipVersion) {
        Iterator<InetAddress> iterator = inetAddresses.iterator();
        if (iterator == null) {
            return null;
        }

        String[] addr = new String[inetAddresses.size()];
        int i = 0;

        while (iterator.hasNext()) {
            InetAddress inetAddress = iterator.next();

            if (inetAddress == null) {
                continue;
            }

            if (inetAddress.isAnyLocalAddress()
                    || inetAddress.isLinkLocalAddress()
                    || inetAddress.isLoopbackAddress()) {
                ImsLog.w(mSlotId, "Invalid InetAddress - " + inetAddress.toString());
                continue;
            }

            if (inetAddress instanceof Inet6Address) {
                if (((Inet6Address) inetAddress).isIPv4CompatibleAddress()) {
                    continue;
                }
                if ((ipVersion == EIpVersion.IPV4.getInt())
                        || (ipVersion == EIpVersion.IPV4V6.getInt())) {
                    continue;
                }
            } else if (inetAddress instanceof Inet4Address) {
                if ((ipVersion == EIpVersion.IPV6.getInt())
                        || (ipVersion == EIpVersion.IPV6V4.getInt())) {
                    continue;
                }
            } else {
                continue;
            }

            String ipAddress = inetAddress.getHostAddress();
            boolean bSame = false;

            for (int j = 0; j < i; j++) {
                if (addr[j].equals(ipAddress)) {
                    bSame = true;
                }
            }

            if (!bSame) {
                addr[i] = ipAddress;
                i++;
            }
        }

        ImsLog.i(mSlotId, "Count of valid addresses :: " + i);

        if (inetAddresses.size() != i) {
            String[] validAddresses = new String[i];
            for (int k = 0; k < i; k++) {
                validAddresses[k] = addr[k];
            }

            return validAddresses;
        }

        return addr;
    }

    private int getIpVersion(int apnType) {
        IApn apn = getApnControl(apnType);

        if (apn == null) {
            ImsLog.w(mSlotId, "Apn is null; apnType=" + apnType);
            return 0;
        }

        return apn.getIpVersion();
    }

    private boolean isNativeServiceReady() {
        NativeStateInterface nsi =
                AgentFactory.getInstance().getAgent(NativeStateInterface.class, mSlotId);
        return nsi != null && nsi.isServiceReady();
    }

    private void sendDataConnectionState(int apnType,
            PreciseDataConnectionState dataConnectionState) {
        IApn apn = getApnControl(apnType);
        if (apn != null) {
            Message msg = Message.obtain();
            msg.what = Apn.EVENT_PRECISE_DATA_CONNECTION_STATE_CHANGED;
            msg.obj = dataConnectionState;

            apn.sendMessage(msg);
        }
    }

    private void updateSubscription() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);

        if (sim == null || !sim.isSimLoadCompleted()) {
            return;
        }

        synchronized (mLock) {
            int subId = sim.getSubId();

            if (mSubId == subId || subId == MSimUtils.INVALID_SUB_ID) {
                return;
            }

            mSubId = subId;
            ImsLog.i(mSlotId, "updateSubscription :: subId=" + subId);

            if (mPreciseDcStateListener != null) {
                TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(mSubId);
                mPreciseDcStateListener.unregister();
                mPreciseDcStateListener.register(tmp);
            }
        }
    }

    @VisibleForTesting
    protected class PreciseDcStateListener extends TelephonyCallback
            implements TelephonyCallback.PreciseDataConnectionStateListener {
        private TelephonyManagerProxy mTelephonyManagerProxy = null;

        PreciseDcStateListener() {
        }

        public void register(TelephonyManagerProxy tmp) {
            mTelephonyManagerProxy = tmp;

            if (mTelephonyManagerProxy != null) {
                mTelephonyManagerProxy.registerTelephonyCallback(
                        AppContext.getInstance().getMainExecutor(), this);
            }
        }

        public void unregister() {
            if (mTelephonyManagerProxy != null) {
                mTelephonyManagerProxy.unregisterTelephonyCallback(this);
                mTelephonyManagerProxy = null;
            }
        }

        @Override
        public void onPreciseDataConnectionStateChanged(
                PreciseDataConnectionState dataConnectionState) {
            ImsLog.d(mSlotId, "onPreciseDataConnectionStateChanged :: " + dataConnectionState);

            if (dataConnectionState.getState() == TelephonyManager.DATA_SUSPENDED) {
                return;
            }

            ApnSetting apnSetting = dataConnectionState.getApnSetting();
            if (apnSetting == null) {
                ImsLog.i(mSlotId, "Invalid apnSetting");
                return;
            }

            int apnTypes = apnSetting.getApnTypeBitmask();
            if ((apnTypes & ApnSetting.TYPE_IMS) != 0) {
                sendDataConnectionState(EApnType.IMS.getType(), dataConnectionState);
            }
            if ((apnTypes & ApnSetting.TYPE_EMERGENCY) != 0) {
                sendDataConnectionState(EApnType.EMERGENCY.getType(), dataConnectionState);
            }
            if ((apnTypes & ApnSetting.TYPE_XCAP) != 0) {
                sendDataConnectionState(EApnType.XCAP.getType(), dataConnectionState);
            }
        }
    }

    @VisibleForTesting
    protected int getSlotId(int subId) {
        return MSimUtils.getSlotId(subId);
    }
}
