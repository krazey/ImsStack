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
package com.android.imsstack.test.menu;

import android.content.Intent;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.android.imsstack.R;
import com.android.imsstack.core.carrier.CarrierInfo;
import com.android.imsstack.core.carrier.ImsCarrierResolver;
import com.android.imsstack.core.carrier.SimCarrierId;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

import java.util.ArrayList;

@SuppressWarnings("deprecation")
public class ImsConfigMenu extends PreferenceActivity {
    private int mSlotId = -1;
    private ArrayList<String> mSimList;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        ImsLog.d("");

        super.onCreate(savedInstanceState);

        if (mSlotId < 0) {
            showSimList();
        } else {
            initConfigMenu();
        }
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return fragmentName != null;
    }

    private void initConfigMenu() {
        ImsLog.i(mSlotId, "initConfigMenu");

        addPreferencesFromResource(R.xml.ims_config_menu);

        // CarrierConfig {
        Preference carrierConfigMenu = findPreference("carrier_config_menu");

        if (carrierConfigMenu != null) {
            Intent intent = carrierConfigMenu.getIntent();

            if (intent != null) {
                intent.putExtra(MSimUtils.EXTRA_KEY_SLOT_ID, mSlotId);
            }
        }
        // }

        // TestConfig {
        Preference testConfigMenu = findPreference("test_config_menu");

        if (testConfigMenu != null) {
            Intent intent = testConfigMenu.getIntent();

            if (intent != null) {
                intent.putExtra(MSimUtils.EXTRA_KEY_SLOT_ID, mSlotId);
            }
        }
        // }
    }

    private void setSlotId(int slotId) {
        mSlotId = slotId;
    }

    private AdapterView.OnItemClickListener mSimSelectionListener =
            new AdapterView.OnItemClickListener() {
                @Override
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                    String selectedSim = mSimList.get(position);

                    if (selectedSim.contains("SIM1")) {
                        setSlotId(0);
                    } else if (selectedSim.contains("SIM2")) {
                        setSlotId(1);
                    } else if (selectedSim.contains("SIM3")) {
                        setSlotId(2);
                    } else {
                        setSlotId(0);
                    }

                    initConfigMenu();
                }
            };

    private void showSimList() {
        mSimList = new ArrayList<>();

        int activeSimCount = MSimUtils.getActiveSimCount();

        for (int i = 0; i < activeSimCount; i++) {
            SimCarrierId cid = CarrierInfo.getInstance().getCarrierId(i);

            if (cid != null) {
                ImsCarrierResolver.Carrier c = ImsCarrierResolver.getCarrierFromCarrierId(cid);

                mSimList.add("SIM" + (i + 1) + " - " + c.getOperator() + " / " + c.getCountry());
            } else {
                mSimList.add("SIM" + (i + 1));
            }
        }

        ImsLog.d("showSimList: sims=" + activeSimCount);

        ArrayAdapter<String> simListAdaptor =
                new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, mSimList);

        ListView listView = getListView();

        if (listView != null) {
            listView.setAdapter(simListAdaptor);
            listView.setOnItemClickListener(mSimSelectionListener);
        }
    }
}
