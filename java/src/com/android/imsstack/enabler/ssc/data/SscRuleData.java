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

import com.android.imsstack.enabler.ssc.SscConstant;
import com.android.imsstack.util.ImsLog;

import java.util.ArrayList;

public class SscRuleData {
    private String mRuleId = null;
    private int mSsCondition = -1;
    private int mState = SscConstant.STATUS_DISABLE;
    private int mServiceClass = 0;

    protected ArrayList<SscRuleElement> mRuleConditionList;
    protected ArrayList<SscRuleElement> mRuleActionList;

    public SscRuleData() {
    }

    public String getRuleId() {
        return mRuleId;
    }

    public void setRuleId(String id) {
        mRuleId = id;
    }

    public int getSsCondition() {
        return mSsCondition;
    }

    public void setSsCondition(int condition) {
        mSsCondition = condition;
    }

    public int getState() {
        return mState;
    }

    public void setState(int state) {
        mState = state;
    }

    public int getServiceClass() {
        return mServiceClass;
    }

    public void setServiceClass(int serviceClass) {
        mServiceClass = serviceClass;
    }

    public void addServiceClass(int serviceClass) {
        mServiceClass |= serviceClass;
    }

    public ArrayList<SscRuleElement> getConditionList() {
        return mRuleConditionList;
    }

    public void setConditionList(ArrayList<SscRuleElement> list) {
        mRuleConditionList = list;
    }

    public ArrayList<SscRuleElement> getActionList() {
        return mRuleActionList;
    }

    public void setActionList(ArrayList<SscRuleElement> list) {
        mRuleActionList = list;
    }

    public String toString() {
        StringBuffer sb = new StringBuffer();

        sb.append("==== Rule ID : " + mRuleId + " ====\n");
        sb.append("==== Rule Data Start ====\n");
        sb.append("<< Condition >>\n");

        if (mRuleConditionList != null) {
            for (SscRuleElement rule : mRuleConditionList) {
                sb.append(rule.toString());
            }
        }

        sb.append("<<  Action  >>\n");
        if (mRuleActionList != null) {
            for (SscRuleElement rule : mRuleActionList) {
                sb.append(rule.toString());
            }
        }
        sb.append("======ServiceClass :" + mServiceClass + "=====\n");
        sb.append("====  Rule Data End  ====\n");

        return sb.toString();
    }
}
