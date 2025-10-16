/*
 * Copyright (C) 2023 The Android Open Source Project
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

import static android.telephony.TelephonyManager.CALL_STATE_IDLE;
import static android.telephony.TelephonyManager.NETWORK_TYPE_UNKNOWN;

import static com.android.imsstack.base.TestAppContext.SLOT0;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.telephony.ServiceState;
import android.telephony.TelephonyManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.EIpVersion;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemCallInterface;
import com.android.imsstack.system.SystemInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.FileDescriptor;
import java.util.Collections;
import java.util.List;

@RunWith(JUnit4.class)
public class SystemCallAgentTest {
    @Mock private ISystem mSystem;
    @Mock private SystemInterface mSystemInterface;
    @Mock private ConfigInterface mConfigInterface;
    @Mock private LocationInterface mLocationInterface;
    @Mock private IpSecInterface mIpSecInterface;
    @Mock private TelephonyInterface mTelephonyInterface;
    @Mock private CellInfoInterface mCellInfoInterface;
    @Mock private ImsRadioInterface mRadioInterface;
    @Mock private IDcApn mDcApn;
    @Mock private IDcNetWatcher mDcNetWatcher;
    @Mock private IDcUtils mDcUtils;
    @Mock private SimAgent mSimAgent;
    @Mock private NativeStateAgent mNativeStateAgent;

    private SystemCallAgent mSystemCallAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        SystemInterface.setSystemInterface(mSystemInterface);
        when(mSystemInterface.getSystem(eq(SLOT0))).thenReturn(mSystem);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mConfigInterface, SLOT0);
        AgentFactory.getInstance().setAgent(LocationInterface.class, mLocationInterface, SLOT0);
        AgentFactory.getInstance().setAgent(IpSecInterface.class, mIpSecInterface, SLOT0);
        AgentFactory.getInstance().setAgent(TelephonyInterface.class, mTelephonyInterface, SLOT0);
        AgentFactory.getInstance().setAgent(CellInfoInterface.class, mCellInfoInterface, SLOT0);
        AgentFactory.getInstance().setAgent(ImsRadioInterface.class, mRadioInterface, SLOT0);
        AgentFactory.getInstance().setAgent(SimInterface.class, mSimAgent, SLOT0);
        AgentFactory.getInstance().setAgent(NativeStateInterface.class, mNativeStateAgent, SLOT0);
        replaceDcApn(mDcApn);
        replaceDcNetWatcher(mDcNetWatcher);
        replaceDcUtils(mDcUtils);

        mSystemCallAgent = new SystemCallAgent(SLOT0);
    }

    @After
    public void tearDown() throws Exception {
        if (mSystemCallAgent != null) {
            mSystemCallAgent.destroy();
            mSystemCallAgent = null;
        }

        replaceDcApn(null);
        replaceDcNetWatcher(null);
        replaceDcUtils(null);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(LocationInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(IpSecInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(CellInfoInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(ImsRadioInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(NativeStateInterface.class, null, SLOT0);
        SystemInterface.setSystemInterface(null);
        mNativeStateAgent = null;
        mSimAgent = null;
        mDcUtils = null;
        mDcNetWatcher = null;
        mDcApn = null;
        mRadioInterface = null;
        mCellInfoInterface = null;
        mTelephonyInterface = null;
        mIpSecInterface = null;
        mLocationInterface = null;
        mConfigInterface = null;
        mSystem = null;
        mSystemInterface = null;
    }

    @Test
    @SmallTest
    public void testDestroy() {
        verify(mSystem).setSystemCallInterface(any(SystemCallInterface.class));

        mSystemCallAgent.destroy();
        mSystemCallAgent = null;

        verify(mSystem).setSystemCallInterface(eq(null));
    }

    @Test
    @SmallTest
    public void testGetCarrierConfig() {
        mSystemCallAgent.getCarrierConfig();

        verify(mConfigInterface).getCarrierConfig();

        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);

        assertNull(mSystemCallAgent.getCarrierConfig());
        verifyNoMoreInteractions(mConfigInterface);
    }

    @Test
    @SmallTest
    public void testAddIpSecSaParameter() {
        mSystemCallAgent.addIpSecSaParameter(null);

        verify(mIpSecInterface).addIpSecSaParameter(eq(null));

        AgentFactory.getInstance().setAgent(IpSecInterface.class, null, SLOT0);
        int result = mSystemCallAgent.addIpSecSaParameter(null);

        assertEquals(SystemCallInterface.RESULT_ERROR, result);
        verifyNoMoreInteractions(mIpSecInterface);
    }

    @Test
    @SmallTest
    public void testRemoveIpSecSaParameter() {
        mSystemCallAgent.removeIpSecSaParameter(1);

        verify(mIpSecInterface).removeIpSecSaParameter(eq(1));

        AgentFactory.getInstance().setAgent(IpSecInterface.class, null, SLOT0);

        verifyNoMoreInteractions(mIpSecInterface);
    }

    @Test
    @SmallTest
    public void testApplyIpSecSa() {
        mSystemCallAgent.applyIpSecSa(1, 2, 3, null);

        verify(mIpSecInterface).applyIpSecSa(eq(1), eq(2), eq(3), eq(null));

        AgentFactory.getInstance().setAgent(IpSecInterface.class, null, SLOT0);
        int result = mSystemCallAgent.applyIpSecSa(1, 2, 3, null);

        assertEquals(SystemCallInterface.RESULT_ERROR, result);
        verifyNoMoreInteractions(mIpSecInterface);
    }

    @Test
    @SmallTest
    public void testRemoveIpSecSa() {
        mSystemCallAgent.removeIpSecSa(1, 2, 3, null);

        verify(mIpSecInterface).removeIpSecSa(eq(1), eq(2), eq(3), eq(null));

        AgentFactory.getInstance().setAgent(IpSecInterface.class, null, SLOT0);

        verifyNoMoreInteractions(mIpSecInterface);
    }

    @Test
    @SmallTest
    public void testGetIsimState() {
        mSystemCallAgent.getIsimState();

        verify(mSimAgent).getIsimStateString();

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);

        assertEquals("UNKNOWN", mSystemCallAgent.getIsimState());
        verifyNoMoreInteractions(mSimAgent);
    }

    @Test
    @SmallTest
    public void testGetIsimRecord() {
        when(mSimAgent.getIsimImpi()).thenReturn("impi");
        List<String> record = mSystemCallAgent.getIsimRecord(Sim.ISIM_FILE_ID_IMPI);

        assertEquals(1, record.size());
        assertEquals("impi", record.get(0));

        when(mSimAgent.getIsimDomain()).thenReturn("domain");
        record = mSystemCallAgent.getIsimRecord(Sim.ISIM_FILE_ID_DOMAIN);

        assertEquals(1, record.size());
        assertEquals("domain", record.get(0));

        when(mSimAgent.getIsimImpu()).thenReturn(List.of("impu"));
        record = mSystemCallAgent.getIsimRecord(Sim.ISIM_FILE_ID_IMPU);

        assertEquals(1, record.size());
        assertEquals("impu", record.get(0));

        when(mSimAgent.getIsimPcscf()).thenReturn(List.of("pcscfAddress"));
        record = mSystemCallAgent.getIsimRecord(Sim.ISIM_FILE_ID_PCSCF);

        assertEquals(1, record.size());
        assertEquals("pcscfAddress", record.get(0));
    }

    @Test
    @SmallTest
    public void testGetIsimRecordWhenContentNullOrEmpty() {
        when(mSimAgent.getIsimImpi()).thenReturn(null);
        List<String> record = mSystemCallAgent.getIsimRecord(Sim.ISIM_FILE_ID_IMPI);

        assertTrue(record.isEmpty());

        when(mSimAgent.getIsimDomain()).thenReturn(null);
        record = mSystemCallAgent.getIsimRecord(Sim.ISIM_FILE_ID_DOMAIN);

        assertTrue(record.isEmpty());

        when(mSimAgent.getIsimImpu()).thenReturn(Collections.emptyList());
        record = mSystemCallAgent.getIsimRecord(Sim.ISIM_FILE_ID_IMPU);

        assertTrue(record.isEmpty());

        int unknownFileId = 0x6FFF;
        record = mSystemCallAgent.getIsimRecord(unknownFileId);

        assertTrue(record.isEmpty());

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        record = mSystemCallAgent.getIsimRecord(Sim.ISIM_FILE_ID_IMPI);

        assertTrue(record.isEmpty());
    }

    @Test
    @SmallTest
    public void testRequestIsimAuthentication() {
        String nonce = "EMQHeGstjjt2pkTck3aM95AQtArxaBmBAADaolOr+yoMYw==";
        long owner = 1L;
        int result = mSystemCallAgent.requestIsimAuthentication(nonce, owner);

        assertEquals(SystemCallInterface.RESULT_OK, result);
        verify(mSimAgent).requestSimAuthentication(eq(Sim.APP_TYPE_ISIM), eq(nonce), eq(owner));

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        result = mSystemCallAgent.requestIsimAuthentication(nonce, owner);

        assertEquals(SystemCallInterface.RESULT_FAIL, result);
        verifyNoMoreInteractions(mSimAgent);
    }

    @Test
    @SmallTest
    public void testRequestUsimAuthentication() {
        String nonce = "EMQHeGstjjt2pkTck3aM95AQtArxaBmBAADaolOr+yoMYw==";
        long owner = 1L;
        int result = mSystemCallAgent.requestUsimAuthentication(nonce, owner);

        assertEquals(SystemCallInterface.RESULT_OK, result);
        verify(mSimAgent).requestSimAuthentication(eq(Sim.APP_TYPE_USIM), eq(nonce), eq(owner));

        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        result = mSystemCallAgent.requestUsimAuthentication(nonce, owner);

        assertEquals(SystemCallInterface.RESULT_FAIL, result);
        verifyNoMoreInteractions(mSimAgent);
    }

    @Test
    @SmallTest
    public void testGetCsCallState() {
        mSystemCallAgent.getCsCallState();

        verify(mTelephonyInterface).getCsCallState();

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        int callState = mSystemCallAgent.getCsCallState();

        assertEquals(CALL_STATE_IDLE, callState);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testGetCsCallStateInOtherSlot() {
        mSystemCallAgent.getCsCallStateInOtherSlot();

        verify(mTelephonyInterface).getCsCallStateInOtherSlot();

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        int callState = mSystemCallAgent.getCsCallStateInOtherSlot();

        assertEquals(CALL_STATE_IDLE, callState);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testGetNetworkType() {
        mSystemCallAgent.getNetworkType();

        verify(mTelephonyInterface).getNetworkType();

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        int networkType = mSystemCallAgent.getNetworkType();

        assertEquals(NETWORK_TYPE_UNKNOWN, networkType);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testGetVoiceNetworkType() {
        mSystemCallAgent.getVoiceNetworkType();

        verify(mTelephonyInterface).getVoiceNetworkType();

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        int networkType = mSystemCallAgent.getVoiceNetworkType();

        assertEquals(NETWORK_TYPE_UNKNOWN, networkType);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testGetImei() {
        mSystemCallAgent.getImei();

        verify(mTelephonyInterface).getImei();

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        String imei = mSystemCallAgent.getImei();

        assertNull(imei);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testGetDeviceSoftwareVersion() {
        mSystemCallAgent.getDeviceSoftwareVersion();

        verify(mTelephonyInterface).getDeviceSoftwareVersion();

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        String svn = mSystemCallAgent.getDeviceSoftwareVersion();

        assertNull(svn);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testGetPhoneNumber() {
        mSystemCallAgent.getPhoneNumber();

        verify(mTelephonyInterface).getPhoneNumber();

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        String phoneNumber = mSystemCallAgent.getPhoneNumber();

        assertEquals("", phoneNumber);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testGetSubscriberId() {
        mSystemCallAgent.getSubscriberId();

        verify(mTelephonyInterface).getSubscriberId();

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        String imsi = mSystemCallAgent.getSubscriberId();

        assertNull(imsi);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testGetSimMcc() {
        mSystemCallAgent.getSimMcc();

        verify(mTelephonyInterface).getSimMcc();

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        String mcc = mSystemCallAgent.getSimMcc();

        assertNull(mcc);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testGetSimMnc() {
        mSystemCallAgent.getSimMnc();

        verify(mTelephonyInterface).getSimMnc();

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        String mnc = mSystemCallAgent.getSimMnc();

        assertNull(mnc);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testGetSimCountryIso() {
        mSystemCallAgent.getSimCountryIso();

        verify(mTelephonyInterface).getSimCountryIso();

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        String countryIso = mSystemCallAgent.getSimCountryIso();

        assertEquals("", countryIso);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testGetNetworkCountryIso() {
        mSystemCallAgent.getNetworkCountryIso();

        verify(mTelephonyInterface).getNetworkCountryIso();

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        String countryIso = mSystemCallAgent.getNetworkCountryIso();

        assertEquals("", countryIso);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testGetNetworkOperator() {
        mSystemCallAgent.getNetworkOperator();

        verify(mTelephonyInterface).getNetworkOperator();

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        String operator = mSystemCallAgent.getNetworkOperator();

        assertEquals("", operator);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testIsEmergencyNumber() {
        String number = "911";
        mSystemCallAgent.isEmergencyNumber(number);

        verify(mTelephonyInterface).isEmergencyNumber(eq(number));

        AgentFactory.getInstance().setAgent(TelephonyInterface.class, null, SLOT0);
        boolean result = mSystemCallAgent.isEmergencyNumber(number);

        assertFalse(result);
        verifyNoMoreInteractions(mTelephonyInterface);
    }

    @Test
    @SmallTest
    public void testRequestNetwork() {
        mSystemCallAgent.requestNetwork(EApnType.IMS.getType());

        verify(mDcApn).connect(eq(EApnType.IMS.getType()));

        replaceDcApn(null);
        boolean result = mSystemCallAgent.requestNetwork(EApnType.IMS.getType());

        assertFalse(result);
        verifyNoMoreInteractions(mDcApn);
    }

    @Test
    @SmallTest
    public void testReleaseNetwork() {
        mSystemCallAgent.releaseNetwork(EApnType.IMS.getType());

        verify(mDcApn).disconnect(eq(EApnType.IMS.getType()));

        replaceDcApn(null);
        boolean result = mSystemCallAgent.releaseNetwork(EApnType.IMS.getType());

        assertFalse(result);
        verifyNoMoreInteractions(mDcApn);
    }

    @Test
    @SmallTest
    public void testGetApnName() {
        mSystemCallAgent.getApnName(EApnType.IMS.getType());

        verify(mDcApn).getApn(eq(EApnType.IMS.getType()));

        replaceDcApn(null);
        String result = mSystemCallAgent.getApnName(EApnType.IMS.getType());

        assertEquals("", result);
        verifyNoMoreInteractions(mDcApn);
    }

    @Test
    @SmallTest
    public void testGetDataConnectionState() {
        mSystemCallAgent.getDataConnectionState(EApnType.IMS.getType());

        verify(mDcApn).getDataState(eq(EApnType.IMS.getType()));

        replaceDcApn(null);
        int result = mSystemCallAgent.getDataConnectionState(EApnType.IMS.getType());

        assertEquals(EDataState.DATA_STATE_DISCONNECTED.getState(), result);
        verifyNoMoreInteractions(mDcApn);
    }

    @Test
    @SmallTest
    public void testGetIfaceId() {
        mSystemCallAgent.getIfaceId(EApnType.IMS.getType());

        verify(mDcApn).getIfaceId(eq(EApnType.IMS.getType()));

        replaceDcApn(null);
        int result = mSystemCallAgent.getIfaceId(EApnType.IMS.getType());

        assertEquals(SystemCallInterface.RESULT_ERROR, result);
        verifyNoMoreInteractions(mDcApn);
    }

    @Test
    @SmallTest
    public void testGetIfaceName() {
        mSystemCallAgent.getIfaceName(EApnType.IMS.getType());

        verify(mDcApn).getIfaceName(eq(EApnType.IMS.getType()));

        replaceDcApn(null);
        String result = mSystemCallAgent.getIfaceName(EApnType.IMS.getType());

        assertEquals("", result);
        verifyNoMoreInteractions(mDcApn);
    }

    @Test
    @SmallTest
    public void testGetMtu() {
        mSystemCallAgent.getMtu(EApnType.IMS.getType());

        verify(mDcApn).getMtu(eq(EApnType.IMS.getType()));

        replaceDcApn(null);
        int result = mSystemCallAgent.getMtu(EApnType.IMS.getType());

        assertEquals(0, result);
        verifyNoMoreInteractions(mDcApn);
    }

    @Test
    @SmallTest
    public void testGetIpcanCategory() {
        mSystemCallAgent.getIpcanCategory(EApnType.IMS.getType());

        verify(mDcApn).getIpcanCategory(eq(EApnType.IMS.getType()));

        replaceDcApn(null);
        int result = mSystemCallAgent.getIpcanCategory(EApnType.IMS.getType());

        assertEquals(IApn.IPCAN_CATEGORY_MOBILE, result);
        verifyNoMoreInteractions(mDcApn);
    }

    @Test
    @SmallTest
    public void testGetLocalAddress() {
        mSystemCallAgent.getLocalAddress(EApnType.IMS.getType(), EIpVersion.IPV6V4.getInt());

        verify(mDcApn).getLocalAddress(eq(EApnType.IMS.getType()), eq(EIpVersion.IPV6V4.getInt()));

        replaceDcApn(null);
        String result = mSystemCallAgent.getLocalAddress(
                EApnType.IMS.getType(), EIpVersion.IPV6V4.getInt());

        assertEquals("", result);
        verifyNoMoreInteractions(mDcApn);
    }

    @Test
    @SmallTest
    public void testGetPcscfAddresses() {
        mSystemCallAgent.getPcscfAddresses(EApnType.IMS.getType(), EIpVersion.IPV6V4.getInt());

        verify(mDcApn).getPcscfAddress(
                eq(EApnType.IMS.getType()), eq(EIpVersion.IPV6V4.getInt()));

        replaceDcApn(null);
        String[] result = mSystemCallAgent.getPcscfAddresses(
                EApnType.IMS.getType(), EIpVersion.IPV6V4.getInt());

        assertNotNull(result);
        assertEquals(0, result.length);
        verifyNoMoreInteractions(mDcApn);
    }

    @Test
    @SmallTest
    public void testIsIpv6Preferred() {
        mSystemCallAgent.isIpv6Preferred(EApnType.IMS.getType());

        verify(mDcApn).isIpv6Preferred(eq(EApnType.IMS.getType()));

        replaceDcApn(null);
        boolean result = mSystemCallAgent.isIpv6Preferred(EApnType.IMS.getType());

        assertFalse(result);
        verifyNoMoreInteractions(mDcApn);
    }

    @Test
    @SmallTest
    public void testGetNetworkRegistrationRejectCause() {
        mSystemCallAgent.getNetworkRegistrationRejectCause();

        verify(mDcNetWatcher).getNetworkRegistrationRejectCause();

        replaceDcNetWatcher(null);
        int result = mSystemCallAgent.getNetworkRegistrationRejectCause();

        assertEquals(IDcNetWatcher.REGISTRATION_REJECT_CAUSE_NONE, result);
        verifyNoMoreInteractions(mDcNetWatcher);
    }

    @Test
    @SmallTest
    public void testGetAccessNetworkPlmn() {
        mSystemCallAgent.getAccessNetworkPlmn();

        verify(mDcUtils).getAccessNetworkPlmn();

        replaceDcUtils(null);
        String result = mSystemCallAgent.getAccessNetworkPlmn();

        assertNotNull(result);
        assertEquals("", result);
        verifyNoMoreInteractions(mDcUtils);
    }

    @Test
    @SmallTest
    public void testGetHostByName() {
        String host = "test.ims.com";
        mSystemCallAgent.getHostByName(EApnType.IMS.getType(), EIpVersion.IPV6V4.getInt(), host);

        verify(mDcApn).getHostByName(
                eq(EApnType.IMS.getType()), eq(EIpVersion.IPV6V4.getInt()), eq(host));

        replaceDcApn(null);
        String[] result = mSystemCallAgent.getHostByName(
                EApnType.IMS.getType(), EIpVersion.IPV6V4.getInt(), host);

        assertNull(result);
        verifyNoMoreInteractions(mDcApn);
    }

    @Test
    @SmallTest
    public void testBindSocket() {
        FileDescriptor sockFd = new FileDescriptor();
        mSystemCallAgent.bindSocket(EApnType.IMS.getType(), sockFd);

        verify(mDcApn).bindSocket(eq(EApnType.IMS.getType()), eq(sockFd));

        replaceDcApn(null);
        boolean result = mSystemCallAgent.bindSocket(EApnType.IMS.getType(), sockFd);

        assertFalse(result);
        verifyNoMoreInteractions(mDcApn);
    }

    @Test
    @SmallTest
    public void testGetVoiceServiceState() {
        mSystemCallAgent.getVoiceServiceState();

        verify(mDcNetWatcher).getVoiceServiceState();

        replaceDcNetWatcher(null);
        int result = mSystemCallAgent.getVoiceServiceState();

        assertEquals(ServiceState.STATE_OUT_OF_SERVICE, result);
        verifyNoMoreInteractions(mDcNetWatcher);
    }

    @Test
    @SmallTest
    public void testGetVoiceRoamingType() {
        mSystemCallAgent.getVoiceRoamingType();

        verify(mDcNetWatcher).getVoiceRoamingType();

        replaceDcNetWatcher(null);
        int result = mSystemCallAgent.getVoiceRoamingType();

        assertEquals(ServiceState.ROAMING_TYPE_NOT_ROAMING, result);
        verifyNoMoreInteractions(mDcNetWatcher);
    }

    @Test
    @SmallTest
    public void testGetDataServiceState() {
        mSystemCallAgent.getDataServiceState();

        verify(mDcNetWatcher).getDataServiceState();

        replaceDcNetWatcher(null);
        int result = mSystemCallAgent.getDataServiceState();

        assertEquals(ServiceState.STATE_OUT_OF_SERVICE, result);
        verifyNoMoreInteractions(mDcNetWatcher);
    }

    @Test
    @SmallTest
    public void testGetCellularDataServiceState() {
        mSystemCallAgent.getCellularDataServiceState();

        verify(mDcNetWatcher).getCellularDataServiceState();

        replaceDcNetWatcher(null);
        int result = mSystemCallAgent.getCellularDataServiceState();

        assertEquals(ServiceState.STATE_OUT_OF_SERVICE, result);
        verifyNoMoreInteractions(mDcNetWatcher);
    }

    @Test
    @SmallTest
    public void testGetDataRoamingType() {
        mSystemCallAgent.getDataRoamingType();

        verify(mDcNetWatcher).getDataRoamingType();

        replaceDcNetWatcher(null);
        int result = mSystemCallAgent.getDataRoamingType();

        assertEquals(ServiceState.ROAMING_TYPE_NOT_ROAMING, result);
        verifyNoMoreInteractions(mDcNetWatcher);
    }

    @Test
    @SmallTest
    public void testGetMocnPlmnInfo() {
        mSystemCallAgent.getMocnPlmnInfo();

        verify(mDcNetWatcher).getMocnPlmnInfo();

        replaceDcNetWatcher(null);
        int result = mSystemCallAgent.getMocnPlmnInfo();

        assertEquals(0, result);
        verifyNoMoreInteractions(mDcNetWatcher);
    }

    @Test
    @SmallTest
    public void testIsNetworkRoaming() {
        mSystemCallAgent.isNetworkRoaming();

        verify(mDcNetWatcher).isRoaming();

        replaceDcNetWatcher(null);
        boolean result = mSystemCallAgent.isNetworkRoaming();

        assertFalse(result);
        verifyNoMoreInteractions(mDcNetWatcher);
    }

    @Test
    @SmallTest
    public void testIsEmergencyOnly() {
        mSystemCallAgent.isEmergencyOnly();

        verify(mDcNetWatcher).isEmergencyOnly();

        replaceDcNetWatcher(null);
        boolean result = mSystemCallAgent.isEmergencyOnly();

        assertFalse(result);
        verifyNoMoreInteractions(mDcNetWatcher);
    }

    @Test
    @SmallTest
    public void testIsEmergencyAttachSupported() {
        mSystemCallAgent.isEmergencyAttachSupported();

        verify(mDcNetWatcher).isEmergencyServiceSupported();

        replaceDcNetWatcher(null);
        boolean result = mSystemCallAgent.isEmergencyAttachSupported();

        assertFalse(result);
        verifyNoMoreInteractions(mDcNetWatcher);
    }

    @Test
    @SmallTest
    public void testIsMobileDataEnabled() {
        mSystemCallAgent.isMobileDataEnabled();

        verify(mDcUtils).isMobileDataEnabled();

        replaceDcUtils(null);
        boolean result = mSystemCallAgent.isMobileDataEnabled();

        assertFalse(result);
        verifyNoMoreInteractions(mDcUtils);
    }

    @Test
    @SmallTest
    public void testGetAccessNetworkInfo() {
        mSystemCallAgent.getAccessNetworkInfo(TelephonyManager.NETWORK_TYPE_LTE);

        verify(mDcUtils).getAccessNetworkInfo(eq(TelephonyManager.NETWORK_TYPE_LTE));

        replaceDcUtils(null);
        IDcUtils.AccessNetworkInfo result =
                mSystemCallAgent.getAccessNetworkInfo(TelephonyManager.NETWORK_TYPE_LTE);

        assertNotNull(result);
        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, result.mNetworkType);
        verifyNoMoreInteractions(mDcUtils);
    }

    @Test
    @SmallTest
    public void testGetLastAccessNetworkInfo() {
        mSystemCallAgent.getLastAccessNetworkInfo(TelephonyManager.NETWORK_TYPE_UNKNOWN);
        mSystemCallAgent.getLastAccessNetworkInfo(TelephonyManager.NETWORK_TYPE_LTE);

        verify(mCellInfoInterface).getAccessNetworkInfo();
        verify(mCellInfoInterface).getAccessNetworkInfo(eq(TelephonyManager.NETWORK_TYPE_LTE));

        AgentFactory.getInstance().setAgent(CellInfoInterface.class, null, SLOT0);
        String[] result =
                mSystemCallAgent.getLastAccessNetworkInfo(TelephonyManager.NETWORK_TYPE_LTE);

        assertNotNull(result);
        assertEquals(0, result.length);
        verifyNoMoreInteractions(mCellInfoInterface);
    }

    @Test
    @SmallTest
    public void testStartImsTraffic() {
        int id = 1;
        int trafficType = ImsRadioInterface.TRAFFIC_TYPE_VOICE;
        int accessNetworkType = ImsRadioInterface.ACCESS_NETWORK_TYPE_EUTRAN;
        int direction = ImsRadioInterface.DIRECTION_MO;
        mSystemCallAgent.startImsTraffic(id, trafficType, accessNetworkType, direction);

        verify(mRadioInterface).startImsTraffic(eq(id),
                eq(trafficType), eq(accessNetworkType), eq(direction));

        AgentFactory.getInstance().setAgent(ImsRadioInterface.class, null, SLOT0);
        mSystemCallAgent.startImsTraffic(id, trafficType, accessNetworkType, direction);

        verifyNoMoreInteractions(mRadioInterface);
    }

    @Test
    @SmallTest
    public void testStopImsTraffic() {
        int id = 1;
        mSystemCallAgent.stopImsTraffic(id);

        verify(mRadioInterface).stopImsTraffic(eq(id));

        AgentFactory.getInstance().setAgent(ImsRadioInterface.class, null, SLOT0);
        mSystemCallAgent.stopImsTraffic(id);

        verifyNoMoreInteractions(mRadioInterface);
    }

    @Test
    @SmallTest
    public void testTriggerEpsFallback() {
        int reason = 1;
        mSystemCallAgent.triggerEpsFallback(reason);

        verify(mRadioInterface).triggerEpsFallback(eq(reason));

        AgentFactory.getInstance().setAgent(ImsRadioInterface.class, null, SLOT0);
        boolean result = mSystemCallAgent.triggerEpsFallback(reason);

        assertFalse(result);
        verifyNoMoreInteractions(mRadioInterface);
    }

    @Test
    @SmallTest
    public void testIsImsVoiceCallSupported() {
        mSystemCallAgent.isImsVoiceCallSupported();

        verify(mDcNetWatcher).isVopsSupported();

        replaceDcNetWatcher(null);
        boolean result = mSystemCallAgent.isImsVoiceCallSupported();

        assertFalse(result);
        verifyNoMoreInteractions(mDcNetWatcher);
    }

    @Test
    @SmallTest
    public void testUpdateNativeServiceReady() {
        int result = mSystemCallAgent.updateNativeServiceReady(true);

        assertEquals(SystemCallInterface.RESULT_OK, result);
        verify(mNativeStateAgent).updateServiceReady(eq(true));

        AgentFactory.getInstance().setAgent(NativeStateInterface.class, null, SLOT0);
        result = mSystemCallAgent.updateNativeServiceReady(true);

        assertEquals(SystemCallInterface.RESULT_FAIL, result);
        verifyNoMoreInteractions(mNativeStateAgent);
    }

    @Test
    @SmallTest
    public void testGetLastKnownLocation() {
        mSystemCallAgent.getLastKnownLocation(LocationInterface.LOCATION_CATEGORY_ALL);

        verify(mLocationInterface).getLastKnownLocation(
                eq(LocationInterface.LOCATION_CATEGORY_ALL));

        AgentFactory.getInstance().setAgent(LocationInterface.class, null, SLOT0);
        String[] result = mSystemCallAgent.getLastKnownLocation(
                LocationInterface.LOCATION_CATEGORY_ALL);

        assertNotNull(result);
        assertEquals(0, result.length);
        verifyNoMoreInteractions(mLocationInterface);
    }

    @Test
    @SmallTest
    public void testStartListeningForLocation() {
        mSystemCallAgent.startListeningForLocation(10);

        verify(mLocationInterface).startListeningForLocation(eq(10));

        AgentFactory.getInstance().setAgent(LocationInterface.class, null, SLOT0);
        mSystemCallAgent.startListeningForLocation(10);

        verifyNoMoreInteractions(mLocationInterface);
    }

    @Test
    @SmallTest
    public void testStopListeningForLocation() {
        mSystemCallAgent.stopListeningForLocation();

        verify(mLocationInterface).stopListeningForLocation();

        AgentFactory.getInstance().setAgent(LocationInterface.class, null, SLOT0);
        mSystemCallAgent.stopListeningForLocation();

        verifyNoMoreInteractions(mLocationInterface);
    }

    @Test
    @SmallTest
    public void testRequestLocationUpdate() {
        int waitTimeMs = 2000; // 2s
        mSystemCallAgent.requestLocationUpdate(waitTimeMs);

        verify(mLocationInterface).requestLocationUpdate(eq(waitTimeMs));

        AgentFactory.getInstance().setAgent(LocationInterface.class, null, SLOT0);
        mSystemCallAgent.requestLocationUpdate(waitTimeMs);

        verifyNoMoreInteractions(mLocationInterface);
    }

    @Test
    @SmallTest
    public void testCancelLocationUpdate() {
        int requestId = 1;
        mSystemCallAgent.cancelLocationUpdate(requestId);

        verify(mLocationInterface).cancelLocationUpdate(eq(requestId));

        AgentFactory.getInstance().setAgent(LocationInterface.class, null, SLOT0);
        mSystemCallAgent.cancelLocationUpdate(requestId);

        verifyNoMoreInteractions(mLocationInterface);
    }

    private void replaceDcApn(IDcApn apn) {
        DcFactory.setDcAgent(IDcApn.class, apn, SLOT0);
    }

    private void replaceDcNetWatcher(IDcNetWatcher netWatcher) {
        DcFactory.setDcAgent(IDcNetWatcher.class, netWatcher, SLOT0);
    }

    private void replaceDcUtils(IDcUtils utils) {
        DcFactory.setDcAgent(IDcUtils.class, utils, SLOT0);
    }
}
