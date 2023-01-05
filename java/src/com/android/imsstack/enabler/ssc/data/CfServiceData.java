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

public class CfServiceData extends SscServiceData {
    private int mNoReplyTimer = -1;
    private ArrayList<SscRuleData> mRuleSet = null;

    public CfServiceData(int slotId, ESsType sstype, int eventNum, int transactionId, int state,
            int condition, int timer, ArrayList<SscRuleData> ruleset) {
        super(slotId, sstype, eventNum, transactionId, state);
        mCondition = condition;
        mNoReplyTimer = timer;
        mRuleSet = ruleset;
    }

    public int getNoReplyTimer() {
        return mNoReplyTimer;
    }

    public ArrayList<SscRuleData> getRuleSet() {
        return mRuleSet;
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();
        if (mRuleSet != null) {
            for(SscRuleData ruleData : mRuleSet) {
                sb.append(ruleData.toString());
            }
        }

        return "\ncommunication-diversion : " + mServiceState + "\n" + "NoReplyTimer : "
                + mNoReplyTimer + "\n" + sb;
    }
}
