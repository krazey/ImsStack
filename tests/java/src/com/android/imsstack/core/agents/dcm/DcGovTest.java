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

import static org.junit.Assert.assertEquals;
import static org.mockito.Matchers.anyInt;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.ICellInfo;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.EIpVersion;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.FileDescriptor;

@RunWith(AndroidTestingRunner.class)
public class DcGovTest {
    private static final int SLOT_ID = 0;
    private static final int APN_TYPE = 1;
    static ContextFixture sContext;
    DcGov mDcGov;

    @Mock private IDcApn mMockIDcApn;
    @Mock private IDcNetWatcher mMockIDcNetWatcher;
    @Mock private IDcUtils mMockIDcUtils;
    @Mock private ICellInfo mMockICellInfo;

    @BeforeClass
    public static void setUpOnce() {
        sContext = new ContextFixture();
        AppContext.init(sContext.getTestDouble());
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        // create the instance to test
        mDcGov = new FakeDcGov(SLOT_ID);
        mDcGov.init(AppContext.getInstance());
    }

    @After
    public void tearDown() throws Exception {
        if (mDcGov != null) {
            mDcGov.cleanup();
            mDcGov = null;
        }
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
        sContext = null;
    }

    @Test
    public void testActivateDataConnection4Sys() throws Exception {
        when(mMockIDcApn.connect(APN_TYPE)).thenReturn(true);

        assertEquals(1, mDcGov.activateDataConnection4Sys(APN_TYPE));
        verify(mMockIDcApn).connect(APN_TYPE);
    }

    @Test
    public void testDeactivateDataConnection4Sys() throws Exception {
        when(mMockIDcApn.disconnect(APN_TYPE)).thenReturn(true);

        assertEquals(1, mDcGov.deactivateDataConnection4Sys(APN_TYPE));
        verify(mMockIDcApn).disconnect(APN_TYPE);
    }

    @Test
    public void testGetAccessNetworkInfo4Sys() throws Exception {
        String[] info = {"mcc", "mnc", "cellid", "tac", "mode"};
        IDcUtils.AccessNetworkInfo ani =
                new IDcUtils.AccessNetworkInfo(TelephonyManager.NETWORK_TYPE_LTE, info);
        when(mMockIDcUtils.getAccessNetworkInfo(anyInt())).thenReturn(ani);

        assertEquals(ani, mDcGov.getAccessNetworkInfo4Sys(anyInt()));
        verify(mMockIDcUtils).getAccessNetworkInfo(anyInt());
    }

    @Test
    public void testGetApnName4Sys() throws Exception {
        String apn = "Apn String";
        when(mMockIDcApn.getApn(APN_TYPE)).thenReturn(apn);

        assertEquals(apn, mDcGov.getApnName4Sys(APN_TYPE));
        verify(mMockIDcApn).getApn(APN_TYPE);
    }

    @Test
    public void testGetDataConnectionState4Sys() throws Exception {
        int state = EDataState.DATA_STATE_CONNECTED.getState();
        when(mMockIDcApn.getDataState(APN_TYPE)).thenReturn(state);

        assertEquals(state, mDcGov.getDataConnectionState4Sys(APN_TYPE));
        verify(mMockIDcApn).getDataState(APN_TYPE);
    }

    @Test
    public void testGetHostByName4Sys() throws Exception {
        int ipVersion = EIpVersion.IPV4V6.getInt();
        String host = "TestHost";
        String[] addr = {"1.1.1.1"};
        when(mMockIDcApn.getHostByName(APN_TYPE, ipVersion, host)).thenReturn(addr);

        assertEquals(addr, mDcGov.getHostByName4Sys(APN_TYPE, ipVersion, host));
        verify(mMockIDcApn).getHostByName(APN_TYPE, ipVersion, host);
    }

    @Test
    public void testGetIfaceId4Sys() throws Exception {
        int netId = 100;
        when(mMockIDcApn.getIfaceId(APN_TYPE)).thenReturn(netId);

        assertEquals(netId, mDcGov.getIfaceId4Sys(APN_TYPE));
        verify(mMockIDcApn).getIfaceId(APN_TYPE);
    }

    @Test
    public void testGetIfaceName4Sys() throws Exception {
        String ifaceName = "TestIface";
        when(mMockIDcApn.getIfaceName(APN_TYPE)).thenReturn(ifaceName);

        assertEquals(ifaceName, mDcGov.getIfaceName4Sys(APN_TYPE));
        verify(mMockIDcApn).getIfaceName(APN_TYPE);
    }

    @Test
    public void testGetIpcanCategory4Sys() throws Exception {
        when(mMockIDcApn.getIpcanCategory(APN_TYPE)).thenReturn(IApn.IPCAN_CATEGORY_WLAN);

        assertEquals(IApn.IPCAN_CATEGORY_WLAN, mDcGov.getIpcanCategory4Sys(APN_TYPE));
        verify(mMockIDcApn).getIpcanCategory(APN_TYPE);
    }

    @Test
    public void testGetLastAccessNetworkInfo4Sys() throws Exception {
        int networkType = TelephonyManager.NETWORK_TYPE_LTE;
        String[] anInfo = {"NetworkType", "UtcTime", "CellInfoAge"};
        when(mMockICellInfo.getAccessNetworkInfo(networkType)).thenReturn(anInfo);

        assertEquals(anInfo, mDcGov.getLastAccessNetworkInfo4Sys(networkType));
        verify(mMockICellInfo).getAccessNetworkInfo(networkType);
    }

    @Test
    public void testGetLastAccessNetworkInfo4Sys_InvalidNetworkType() throws Exception {
        int networkType = -1;
        String[] anInfo = {"NetworkType", "UtcTime", "CellInfoAge"};
        when(mMockICellInfo.getAccessNetworkInfo()).thenReturn(anInfo);

        assertEquals(anInfo, mDcGov.getLastAccessNetworkInfo4Sys(networkType));
        verify(mMockICellInfo).getAccessNetworkInfo();
    }

    @Test
    public void testGetLocalAddress4Sys() throws Exception {
        int ipVersion = EIpVersion.IPV4V6.getInt();
        String addr = "1.1.1.1";
        when(mMockIDcApn.getLocalAddress(APN_TYPE, ipVersion)).thenReturn(addr);

        assertEquals(addr, mDcGov.getLocalAddress4Sys(APN_TYPE, ipVersion));
        verify(mMockIDcApn).getLocalAddress(APN_TYPE, ipVersion);
    }

    @Test
    public void testGetPcscfAddresses4Sys() throws Exception {
        int ipVersion = EIpVersion.IPV4V6.getInt();
        String[] addr = {"1.1.1.1"};
        when(mMockIDcApn.getPcscfAddress(APN_TYPE, ipVersion)).thenReturn(addr);

        assertEquals(addr, mDcGov.getPcscfAddresses4Sys(APN_TYPE, ipVersion));
        verify(mMockIDcApn).getPcscfAddress(APN_TYPE, ipVersion);
    }

    @Test
    public void testGetRoamingState4Sys() throws Exception {
        when(mMockIDcNetWatcher.isRoaming()).thenReturn(true);

        assertEquals(1, mDcGov.getRoamingState4Sys());
        verify(mMockIDcNetWatcher).isRoaming();
    }

    @Test
    public void testGetVoiceRoamingType4Sys() throws Exception {
        int roamingType = ServiceState.ROAMING_TYPE_NOT_ROAMING;
        when(mMockIDcNetWatcher.getVoiceRoamingType()).thenReturn(roamingType);

        assertEquals(roamingType, mDcGov.getVoiceRoamingType4Sys());
        verify(mMockIDcNetWatcher).getVoiceRoamingType();
    }

    @Test
    public void testGetDataRoamingType4Sys() throws Exception {
        int roamingType = ServiceState.ROAMING_TYPE_NOT_ROAMING;
        when(mMockIDcNetWatcher.getDataRoamingType()).thenReturn(roamingType);

        assertEquals(roamingType, mDcGov.getDataRoamingType4Sys());
        verify(mMockIDcNetWatcher).getDataRoamingType();
    }

    @Test
    public void testGetServiceState4Sys() throws Exception {
        int serviceState = ServiceState.STATE_IN_SERVICE;
        when(mMockIDcNetWatcher.getDataServiceState()).thenReturn(serviceState);

        assertEquals(serviceState, mDcGov.getServiceState4Sys());
        verify(mMockIDcNetWatcher).getDataServiceState();
    }

    @Test
    public void testGetVoiceServiceState4Sys() throws Exception {
        int serviceState = ServiceState.STATE_IN_SERVICE;
        when(mMockIDcNetWatcher.getVoiceServiceState()).thenReturn(serviceState);

        assertEquals(serviceState, mDcGov.getVoiceServiceState4Sys());
        verify(mMockIDcNetWatcher).getVoiceServiceState();
    }

    @Test
    public void testIsLteEmergencyOnly4Sys() throws Exception {
        when(mMockIDcNetWatcher.isLteEmergencyOnly()).thenReturn(true);

        assertEquals(1, mDcGov.isLteEmergencyOnly4Sys());
        verify(mMockIDcNetWatcher).isLteEmergencyOnly();
    }

    @Test
    public void testIsEmergencyAttachSupported4Sys() throws Exception {
        when(mMockIDcNetWatcher.isEmergencyServiceSupported()).thenReturn(true);

        assertEquals(1, mDcGov.isEmergencyAttachSupported4Sys());
        verify(mMockIDcNetWatcher).isEmergencyServiceSupported();
    }

    @Test
    public void testGetMocnPlmnInfo4Sys() throws Exception {
        when(mMockIDcNetWatcher.getMocnPlmnInfo()).thenReturn(0);

        assertEquals(0, mDcGov.getMocnPlmnInfo4Sys());
        verify(mMockIDcNetWatcher).getMocnPlmnInfo();
    }

    @Test
    public void testIsMobileDataEnabled() throws Exception {
        when(mMockIDcUtils.isMobileDataEnabled()).thenReturn(true);

        assertEquals(true, mDcGov.isMobileDataEnabled());
        verify(mMockIDcUtils).isMobileDataEnabled();
    }

    @Test
    public void testGetMtu4Sys() throws Exception {
        int mtu = 1500;
        when(mMockIDcApn.getMtu(APN_TYPE)).thenReturn(mtu);

        assertEquals(mtu, mDcGov.getMtu4Sys(APN_TYPE));
        verify(mMockIDcApn).getMtu(APN_TYPE);
    }

    @Test
    public void testBindSocket() throws Exception {
        FileDescriptor fd = new FileDescriptor();
        when(mMockIDcApn.bindSocket(APN_TYPE, fd)).thenReturn(1);

        assertEquals(1, mDcGov.bindSocket(APN_TYPE, fd));
        verify(mMockIDcApn).bindSocket(APN_TYPE, fd);
    }

    private class FakeDcGov extends DcGov {
        FakeDcGov(int slotId) {
            super(slotId);
        }

        @Override
        protected IDcApn getDcApn() {
            return mMockIDcApn;
        }

        @Override
        protected IDcNetWatcher getDcNetWatcher() {
            return mMockIDcNetWatcher;
        }

        @Override
        protected IDcUtils getDcUtil() {
            return mMockIDcUtils;
        }

        @Override
        protected ICellInfo getCellInfo() {
            return mMockICellInfo;
        }
    }
}
