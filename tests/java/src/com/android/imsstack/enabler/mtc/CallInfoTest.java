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

package com.android.imsstack.enabler.mtc;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import android.os.Parcel;
import android.testing.AndroidTestingRunner;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class CallInfoTest  {
    @Test
    public void testDefaultConstructor() {
        CallInfo callInfo = new CallInfo();

        assertEquals(IUMtcCall.SERVICETYPE_NORMAL, callInfo.serviceType);
        assertEquals(IUMtcCall.CALLTYPE_VOIP, callInfo.callType);
        assertFalse(callInfo.emergency);
        assertFalse(callInfo.offline);
        assertFalse(callInfo.ussi);
        assertFalse(callInfo.isConf);
        assertFalse(callInfo.enabledConf);
        assertFalse(callInfo.confSub);
        assertFalse(callInfo.rttCapable);
        assertFalse(callInfo.videoCapable);
    }

    @Test
    public void testConstructorWithServiceTypeCallType() {
        CallInfo callInfo = new CallInfo(
                IUMtcCall.SERVICETYPE_EMERGENCY, IUMtcCall.CALLTYPE_VIDEO_RTT);

        assertEquals(IUMtcCall.SERVICETYPE_EMERGENCY, callInfo.serviceType);
        assertEquals(IUMtcCall.CALLTYPE_VIDEO_RTT, callInfo.callType);
        assertFalse(callInfo.emergency);
        assertFalse(callInfo.offline);
        assertFalse(callInfo.ussi);
        assertFalse(callInfo.isConf);
        assertFalse(callInfo.enabledConf);
        assertFalse(callInfo.confSub);
        assertFalse(callInfo.rttCapable);
        assertFalse(callInfo.videoCapable);
    }

    @Test
    public void testConstructorWithServiceTypeCallTypeIsConf() {
        CallInfo callInfo = new CallInfo(
                IUMtcCall.SERVICETYPE_NONE, IUMtcCall.CALLTYPE_VT, true);

        assertEquals(IUMtcCall.SERVICETYPE_NONE, callInfo.serviceType);
        assertEquals(IUMtcCall.CALLTYPE_VT, callInfo.callType);
        assertFalse(callInfo.emergency);
        assertFalse(callInfo.offline);
        assertFalse(callInfo.ussi);
        assertTrue(callInfo.isConf);
        assertFalse(callInfo.enabledConf);
        assertFalse(callInfo.confSub);
        assertFalse(callInfo.rttCapable);
        assertFalse(callInfo.videoCapable);
    }

    @Test
    public void testConstructorWithCallInfo() {
        CallInfo callInfo = new CallInfo();
        callInfo.serviceType = IUMtcCall.SERVICETYPE_EMERGENCY;
        callInfo.callType = IUMtcCall.CALLTYPE_VIDEO_RTT;
        callInfo.emergency = true;
        callInfo.offline = true;
        callInfo.ussi = true;
        callInfo.isConf = true;
        callInfo.enabledConf = true;
        callInfo.confSub = true;
        callInfo.rttCapable = true;
        callInfo.videoCapable = true;

        CallInfo callInfo2 = new CallInfo(callInfo);

        assertEquals(IUMtcCall.SERVICETYPE_EMERGENCY, callInfo2.serviceType);
        assertEquals(IUMtcCall.CALLTYPE_VIDEO_RTT, callInfo2.callType);
        assertTrue(callInfo2.emergency);
        assertTrue(callInfo2.offline);
        assertTrue(callInfo2.ussi);
        assertTrue(callInfo2.isConf);
        assertTrue(callInfo2.enabledConf);
        assertTrue(callInfo2.confSub);
        assertTrue(callInfo2.rttCapable);
        assertTrue(callInfo2.videoCapable);
    }

    @Test
    public void testUpdate() {
        CallInfo callInfo = new CallInfo();
        callInfo.serviceType = IUMtcCall.SERVICETYPE_EMERGENCY;
        callInfo.callType = IUMtcCall.CALLTYPE_VIDEO_RTT;
        callInfo.emergency = true;
        callInfo.offline = true;
        callInfo.ussi = true;
        callInfo.isConf = true;
        callInfo.enabledConf = true;
        callInfo.confSub = true;
        callInfo.rttCapable = true;
        callInfo.videoCapable = true;

        CallInfo callInfo2 = new CallInfo();
        callInfo2.update(callInfo);

        assertEquals(IUMtcCall.SERVICETYPE_EMERGENCY, callInfo2.serviceType);
        assertEquals(IUMtcCall.CALLTYPE_VIDEO_RTT, callInfo2.callType);
        assertTrue(callInfo2.emergency);
        assertTrue(callInfo2.offline);
        assertTrue(callInfo2.ussi);
        assertTrue(callInfo2.isConf);
        assertTrue(callInfo2.enabledConf);
        assertTrue(callInfo2.confSub);
        assertTrue(callInfo2.rttCapable);
        assertTrue(callInfo2.videoCapable);
    }

    @Test
    public void testParcelReadWrite() {
        CallInfo callInfo = new CallInfo(
                IUMtcCall.SERVICETYPE_NONE, IUMtcCall.CALLTYPE_VT, true);

        Parcel dest = Parcel.obtain();
        callInfo.writeToParcel(dest, 0);
        dest.setDataPosition(0);

        CallInfo callInfo2 = new CallInfo(dest);

        assertEquals(callInfo.serviceType, callInfo2.serviceType);
        assertEquals(callInfo.callType, callInfo2.callType);
        assertEquals(callInfo.emergency, callInfo2.emergency);
        assertEquals(callInfo.offline, callInfo2.offline);
        assertEquals(callInfo.ussi, callInfo2.ussi);
        assertEquals(callInfo.isConf, callInfo2.isConf);
        assertEquals(callInfo.enabledConf, callInfo2.enabledConf);
        assertEquals(callInfo.confSub, callInfo2.confSub);
        assertEquals(callInfo.rttCapable, callInfo2.rttCapable);
        assertEquals(callInfo.videoCapable, callInfo2.videoCapable);
    }
}
