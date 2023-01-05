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

package com.android.imsstack.imsservice.mmtel;

import static org.mockito.Mockito.when;

import android.content.Context;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsStreamMediaProfile;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.ISubscription;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.UsatInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDc;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcServiceStateTracker;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.base.ImsApp;
import com.android.imsstack.imsservice.mmtel.internal.WfcSettingTracker;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.HashMap;
import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class ImsCallContextTest {
    /* Indicates that geolocation information is required to make a call */
    private static final int FLAG_LOCATION_REQUIRED = 0x00000001;
    private int mSlotId = 0;
    private static final int SUBSCRIPTION = 1;

    private ImsCallContext mImsCallContext;
    private MtcServiceStateTracker mStateTracker;
    private ImsApp mImsApp;

    //Mocked classes
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private SimInterface mMockSimInterface;
    @Mock private ISubscription mMockISubscription;
    @Mock private Context mContext;
    @Mock private IBaseContext mIBaseContext;
    @Mock private Executor mExecutor;
    @Mock private WfcSettingTracker mWfcsettingtracker;
    @Mock private MtcApp mMtcapp;
    @Mock private IDcNetWatcher mMockDcNetWatcher;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        AppContext.init(mContext);

        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, mSlotId);
        AgentFactory.getInstance().setAgent(SimInterface.class, mMockSimInterface, mSlotId);
        AgentFactory.getInstance().setDefaultAgent(SUBSCRIPTION, mMockISubscription);

        HashMap<Integer, IDc> dcs = new HashMap<Integer, IDc>(1);
        dcs.put(DcFactory.NETWORK_WATCHER, mMockDcNetWatcher);
        DcFactory.setObjects(mSlotId, dcs);

        mStateTracker = new MtcServiceStateTracker(mIBaseContext);
        mImsApp = new ImsApp(mSlotId) {
            public  void close() {};
            public  void onBinderDied() {}
        };
        mImsCallContext = new ImsCallContext(mContext, mExecutor, mImsApp,
                mWfcsettingtracker, mStateTracker, mMtcapp);

    }

    @After
    public void tearDown() throws Exception {
        AppContext.deinit();
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, mSlotId);
        AgentFactory.getInstance().setAgent(SimInterface.class, null, mSlotId);
        AgentFactory.getInstance().setDefaultAgent(SUBSCRIPTION, null);
        DcFactory.setObjects(mSlotId, null);
        mStateTracker = null;
        mImsCallContext = null;
    }

    @Test
    public void initTest() {
        mImsCallContext.init();
        Mockito.verify(mMtcapp).init();
        Mockito.verify(mMtcapp, Mockito.times(2)).setServiceStateListener(
                Mockito.any(MtcServiceStateTracker.class));
        Mockito.verify(mWfcsettingtracker).init();
    }

    @Test
    public void clearTest() {
        mImsCallContext.clear();
        Mockito.verify(mMtcapp).setServiceStateListener(null);
        Mockito.verify(mMtcapp).clear();
        Mockito.verify(mWfcsettingtracker).clear();
    }

    @Test
    public void disposeTest() {
        mImsCallContext.dispose();
        Mockito.verify(mMtcapp).setServiceStateListener(null);
        Mockito.verify(mWfcsettingtracker).dispose();
        Mockito.verify(mMtcapp).close();
    }

    @Test
    public void getContextTest() {
        Assert.assertNotNull(mImsCallContext.getContext());
    }

    @Test
    public void getExecutorTest() {
        Assert.assertNotNull(mImsCallContext.getExecutor());
    }

    @Test
    public void getPhoneIdTest() {
        Assert.assertEquals(mImsApp.getPhoneId(), mImsCallContext.getPhoneId());
    }

    @Test
    public void getSlotIdTest() {
        Assert.assertEquals(mImsCallContext.getPhoneId(), mImsCallContext.getSlotId());
    }

    @Test
    public void getSubIdTest() {
        Assert.assertEquals(0, mImsCallContext.getSubId());
        Mockito.verify(mMockISubscription).getSubId(mImsCallContext.getSlotId());
    }

    @Test
    public void getServiceStateTracker() {
        Assert.assertEquals(mStateTracker, mImsCallContext.getServiceStateTracker());
    }

    @Test
    public void getLocationAgentTest() {
        Assert.assertNull(mImsCallContext.getLocationAgent());
    }

    @Test
    public void isLocationRequiredForCallTest() {
        Assert.assertFalse(mImsCallContext.isLocationRequiredForCall());

        when(mMockCarrierConfig
            .getInt(CarrierConfig.Assets.KEY_GEOLOCATION_POLICY_FOR_LOCATION_BASED_CALL_INT))
                .thenReturn(FLAG_LOCATION_REQUIRED);
        Assert.assertTrue(mImsCallContext.isLocationRequiredForCall());
    }

    @Test
    public void hasAccessBearerCapabilitiesForHDCallTest() {
        Assert.assertFalse(mImsCallContext.hasAccessBearerCapabilitiesForHDCall());

        when(mWfcsettingtracker.isWfcEnabled()).thenReturn(true);
        when(mWfcsettingtracker.isWfcAvailable()).thenReturn(true);
        Assert.assertTrue(mImsCallContext.hasAccessBearerCapabilitiesForHDCall());

        when(mMockDcNetWatcher.is4G()).thenReturn(true);
        Assert.assertTrue(mImsCallContext.hasAccessBearerCapabilitiesForHDCall());
    }

    @Test
    public void getSrvccStateTrackerTest() {
        Assert.assertNull(mImsCallContext.getSrvccStateTracker());

        int[] intArray = {0, 1};
        when(mMockCarrierConfig.getIntArray(CarrierConfigManager.ImsVoice.KEY_SRVCC_TYPE_INT_ARRAY))
                .thenReturn(intArray);
        Assert.assertNotNull(mImsCallContext.getSrvccStateTracker());
    }

    @Test
    public void getUsatInterfaceTest() {
        Assert.assertNull(mImsCallContext.getUsatInterface());

        UsatInterface mUsatInterface = Mockito.mock(UsatInterface.class);
        when(mMockSimInterface.getUsatInterface()).thenReturn(mUsatInterface);
        Assert.assertNotNull(mImsCallContext.getUsatInterface());
    }

    @Test
    public void getCallLocationPolicyTest() {
        Assert.assertNull(mImsCallContext.getCallLocationPolicy());

        when(mMockCarrierConfig
            .getInt(CarrierConfig.Assets.KEY_GEOLOCATION_POLICY_FOR_LOCATION_BASED_CALL_INT))
                .thenReturn(FLAG_LOCATION_REQUIRED);
        Assert.assertNotNull(mImsCallContext.getCallLocationPolicy());
    }

    @Test
    public void getApiDefault() {
        //valid value of getTtyModeTracker required mocking of settingsutils which is final class
        Assert.assertNull(mImsCallContext.getTtyModeTracker());
        Assert.assertNotNull(mImsCallContext.getApp());
        Assert.assertNotNull(mImsCallContext.getWfcSettingTracker());
    }

    @Test
    public void getMediaCapabilitiesTest() {
        Assert.assertEquals(ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB,
                mImsCallContext.getMediaCapabilities(ImsCallProfile.CALL_TYPE_VOICE,
                    ICallContext.MEDIA_AUDIO));
        Assert.assertEquals(ImsStreamMediaProfile.VIDEO_QUALITY_QVGA_PORTRAIT,
                mImsCallContext.getMediaCapabilities(ImsCallProfile.CALL_TYPE_VOICE,
                    ICallContext.MEDIA_VIDEO));
        Assert.assertEquals(0, mImsCallContext.getMediaCapabilities(ImsCallProfile.CALL_TYPE_VOICE,
                0));

        int[] intArray = {0, 1};
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsVoice.KEY_EVS_PAYLOAD_TYPE_INT_ARRAY))
                .thenReturn(intArray);
        when(mMockDcNetWatcher.is4G()).thenReturn(true);

        Assert.assertEquals(ImsStreamMediaProfile.AUDIO_QUALITY_EVS_SWB,
                mImsCallContext.getMediaCapabilities(ImsCallProfile.CALL_TYPE_VOICE,
                    ICallContext.MEDIA_AUDIO));
        Assert.assertEquals(ImsStreamMediaProfile.AUDIO_QUALITY_EVS_SWB,
                mImsCallContext.getMediaCapabilities(ImsCallProfile.CALL_TYPE_VT,
                    ICallContext.MEDIA_AUDIO));
        Assert.assertEquals(ImsStreamMediaProfile.VIDEO_QUALITY_VGA_PORTRAIT,
                mImsCallContext.getMediaCapabilities(ImsCallProfile.CALL_TYPE_VOICE,
                    ICallContext.MEDIA_VIDEO));
    }
}
