/*
 * Copyright (C) 2025 The Android Open Source Project
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

import static org.junit.Assert.assertEquals;

import android.testing.AndroidTestingRunner;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.ReasonCodeMap;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.ArrayList;
import java.util.List;

@RunWith(AndroidTestingRunner.class)
public class ImsRegistrationImplTest extends ImsStackTest {
    ImsRegistrationImpl mImsRegistrationImpl;

    @Before
    public void setup() throws Exception {
        mImsRegistrationImpl = new ImsRegistrationImpl();

    }

    @After
    public void tearDown() throws Exception {
        if (mImsRegistrationImpl != null) {
            mImsRegistrationImpl = null;
        }
        super.tearDown();
    }

    @Test
    public void getReasonInfo() {
        List<Integer> codes = new ArrayList<>();
        List<Integer> extraCodes = new ArrayList<>();
        List<Integer> expectedCodes = new ArrayList<>();
        List<Integer> expectedExtraCodes = new ArrayList<>();

        ReasonCodeMap.getReasonMap().forEach((key, pair) -> {
            codes.add(mImsRegistrationImpl.getReasonInfo(key, null).getCode());
            extraCodes.add(mImsRegistrationImpl.getReasonInfo(key, null).getExtraCode());

            expectedCodes.add(pair.first);
            expectedExtraCodes.add(pair.second);
        });
        assertEquals(codes, expectedCodes);
        assertEquals(extraCodes, expectedExtraCodes);
    }
}
