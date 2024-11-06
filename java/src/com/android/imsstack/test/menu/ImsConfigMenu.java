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
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.android.imsstack.R;
import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.carrier.CarrierInfo;
import com.android.imsstack.core.carrier.ImsCarrierResolver;
import com.android.imsstack.core.carrier.SimCarrierId;
import com.android.imsstack.util.ImsLog;

import java.util.ArrayList;

public class ImsConfigMenu extends AppCompatActivity {
    protected static final String CARRIER_CONFIG_MENU = "carrier_config_menu";
    protected static final String TEST_CONFIG_MENU = "test_config_menu";

    private int mSlotId;
    private ArrayList<String> mSimList;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        ImsLog.d("");
        super.onCreate(savedInstanceState);
        showSimList();
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
                    String[] tokens = new String[] { "", "", "SIM1", "SIM2", "SIM3" };
                    String selectedSim = mSimList.get(position);

                    // First item will be skipped.
                    for (int i = 2; i < tokens.length; ++i) {
                        if (selectedSim.contains(tokens[i])) {
                            setSlotId(i - 2);
                            break;
                        }
                    }

                    initConfigMenu();
                }
            };

    private void showSimList() {
        mSimList = new ArrayList<>();
        // Add empty lines (notification/title bar) for UI limitation.
        mSimList.add("");
        mSimList.add("");

        int activeSimCount = DeviceConfig.getActiveSimCount();

        for (int i = 0; i < activeSimCount; i++) {
            SimCarrierId cid = CarrierInfo.getInstance().getCarrierId(i);
            StringBuilder sb = new StringBuilder();

            sb.append("SIM").append(i + 1);

            if (cid != null) {
                ImsCarrierResolver.Carrier c = ImsCarrierResolver.getCarrierFromCarrierId(cid);
                sb.append(" - ");
                sb.append(c.getOperator());
                sb.append(" / ");
                sb.append(c.getCountry());
            }
            mSimList.add(sb.toString());
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
