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

import android.os.Parcel;
import android.testing.AndroidTestingRunner;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class IncomingCallInfoTest  {
    @Test
    public void testParcelReadWrite() {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcCall.SERVICETYPE_EMERGENCY);
        parcel.writeInt(IUMtcCall.CALLTYPE_RTT);
        parcel.writeInt(IncomingCallInfo.OIPTYPE_RESTRICTED);
        parcel.writeInt(IncomingCallInfo.OIPTYPE_RESTRICTED);
        parcel.writeString("1234");
        parcel.writeString("abcd");
        parcel.writeInt(1);
        parcel.setDataPosition(0);

        IncomingCallInfo incomingCallInfoParamParcel = new IncomingCallInfo(parcel);

        assertEquals(IUMtcCall.SERVICETYPE_EMERGENCY, incomingCallInfoParamParcel.mServiceType);
        assertEquals(IUMtcCall.CALLTYPE_RTT, incomingCallInfoParamParcel.mCallType);
        assertEquals(IncomingCallInfo.OIPTYPE_RESTRICTED, incomingCallInfoParamParcel.mOIR);
        assertEquals(IncomingCallInfo.OIPTYPE_RESTRICTED, incomingCallInfoParamParcel.mCNAP);
        assertEquals("1234", incomingCallInfoParamParcel.mOI);
        assertEquals("abcd", incomingCallInfoParamParcel.mCNA);

        incomingCallInfoParamParcel.mOI = "5678";
        incomingCallInfoParamParcel.mCNA = "EFGH";

        Parcel dest = Parcel.obtain();
        incomingCallInfoParamParcel.writeToParcel(dest, 0);
        dest.setDataPosition(0);

        IncomingCallInfo incomingCallInfoParamParcel2 = new IncomingCallInfo(dest);

        assertEquals(incomingCallInfoParamParcel.mServiceType,
                incomingCallInfoParamParcel2.mServiceType);
        assertEquals(incomingCallInfoParamParcel.mCallType, incomingCallInfoParamParcel2.mCallType);
        assertEquals(incomingCallInfoParamParcel.mOIR, incomingCallInfoParamParcel2.mOIR);
        assertEquals(incomingCallInfoParamParcel.mCNAP, incomingCallInfoParamParcel2.mCNAP);
        assertEquals(incomingCallInfoParamParcel.mOI, incomingCallInfoParamParcel2.mOI);
        assertEquals(incomingCallInfoParamParcel.mCNA, incomingCallInfoParamParcel2.mCNA);
    }
}
