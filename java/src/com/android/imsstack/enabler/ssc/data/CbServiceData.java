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

package com.android.imsstack.enabler.ssc.data;

import com.android.imsstack.enabler.ssc.ESsType;

import java.util.ArrayList;

public class CbServiceData extends SscServiceData {
    public ArrayList<SscRuleData> mRuleSet = null;

    public CbServiceData(int slotId, ESsType sstype, int eventNum, int transactionId, int state,
            int condition, ArrayList<SscRuleData> ruleSet) {
        super(slotId, sstype, eventNum, transactionId, state);
        mCondition = condition;
        mRuleSet = ruleSet;
    }

    public ArrayList<SscRuleData> getRuleSet() {
        return this.mRuleSet;
    }

    public String toString() {
        StringBuffer sb = new StringBuffer();
        if (mRuleSet != null) {
            for (SscRuleData ruleData : mRuleSet) {
                sb.append(ruleData.toString());
            }
        }
        return super.toString() + ", IsEnabled : " + mServiceState + ", mCondition : " + mCondition;
    }
}
