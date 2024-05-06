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
package com.android.imsstack.test;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.android.imsstack.R;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.carrier.CarrierInfo;
import com.android.imsstack.core.carrier.ImsCarrierResolver;
import com.android.imsstack.core.carrier.SimCarrierId;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosDebug;
import com.android.imsstack.test.menu.ImsConfigMenu;
import com.android.imsstack.util.ImsUtils;

import java.util.Timer;
import java.util.TimerTask;

public class DebugScreen extends Activity {

    private int mSlotId = MSimUtils.INVALID_SLOT_ID;
    private Handler mHandler;
    private IAosDebug mAosDebug;
    private Timer mTimer;

    private Button mButton;
    private Switch mSwitch;
    private TextView mTextView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        handleCreate();
    }

    @Override
    public void onResume() {
        super.onResume();
        handleResume();
    }

    @Override
    public void onPause() {
        super.onPause();
        handlePause();
    }

    @Override
    public void onRequestPermissionsResult(
            int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        handlePermissionsResult(requestCode, permissions, grantResults);
    }

    private void handleCreate() {
        // Check slotId and build type.
        mSlotId = getIntent().getIntExtra(MSimUtils.EXTRA_KEY_SLOT_ID, MSimUtils.INVALID_SLOT_ID);
        if (mSlotId < 0 || ImsUtils.IS_USER) {
            showToast("Not support Debug Screen!");
            finish();
            return;
        }

        setContentView(R.xml.debug_screen);

        // Initialize views.
        mButton = findViewById(R.id.button);
        mSwitch = findViewById(R.id.enableSwitch);
        mTextView = findViewById(R.id.textView);

        // initialize AosDebug and update Text.
        boolean isEnabled = isEnabledDebugScreen();
        mSwitch.setChecked(isEnabled);
        init(isEnabled);
        updateText(isEnabled);

        // Button click listener.
        mButton.setOnClickListener(v -> startImsConfigMenuActivity());

        // Switch change listener.
        mSwitch.setOnCheckedChangeListener(this::restartOnSwitchChanged);
    }

    private void handleResume() {
        updateText(isEnabledDebugScreen());
    }

    private void handlePause() {
        clearTimer();
    }

    private void handlePermissionsResult(
            int requestCode, String[] permissions, int[] grantResults) {
        mAosDebug.notifyPermissionsResult(requestCode, permissions, grantResults, this);
    }

    private void startImsConfigMenuActivity() {
        Intent intent = new Intent(DebugScreen.this, ImsConfigMenu.class);
        startActivity(intent);
    }

    private void restartOnSwitchChanged(CompoundButton buttonView, boolean isChecked) {
        ImsPrivateProperties.Persistent.setBoolean(
                ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_SCREEN_ENABLED,
                isChecked, mSlotId);
        showOrDismissNotification();
        showToast("ImsStack restarted...");

        // Kill the current process to apply changes immediately.
        android.os.Process.killProcess(android.os.Process.myPid());
    }

    private void updateText(boolean isEnabled) {
        clearTimer();
        String carrierInfo = getCarrierInfo();
        String message;

        if (!isEnabled) {
            message = carrierInfo
                    + " Debug screen is disabled.\nTo use, please turn on the switch.";
        } else if (mAosDebug == null) {
            message = carrierInfo + " Debug screen is not available.";
        } else {
            mTimer = new Timer();
            mTimer.schedule(new TimerTask() {
                @Override
                public void run() {
                    String debugMessage = mAosDebug.getDebugMessage();
                    mHandler.post(() -> updateTextView(debugMessage));
                }
            }, 0, 1000);
            return;
        }

        mTextView.setText(message);
    }

    private void updateTextView(String debugMessage) {
        if (mTextView != null) {
            mTextView.setText(debugMessage);
        }
    }

    private boolean isEnabledDebugScreen() {
        return ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_SCREEN_ENABLED,
                false, mSlotId);
    }
    private void init(boolean isChecked) {
        if (isChecked) {
            mHandler = new Handler(Looper.getMainLooper());
            mAosDebug = AosFactory.getInstance().getAosDebug(mSlotId);
        }
    }

    private void clearTimer() {
        if (mTimer != null) {
            mTimer.cancel();
            mTimer = null;
        }
    }

    private void showOrDismissNotification() {
        if (mAosDebug != null) {
            mAosDebug.showOrDismissNotification(this);
        }
    }

    private String getCarrierInfo() {
        SimCarrierId cid = CarrierInfo.getInstance().getCarrierId(mSlotId);
        if (cid != null) {
            ImsCarrierResolver.Carrier c = ImsCarrierResolver.getCarrierFromCarrierId(cid);
            return "[" + c.getOperator() + "/" + c.getCountry() + "]";
        }
        return IAosDebug.DebugData.STR_EMPTY;
    }

    private void showToast(String text) {
        Toast.makeText(DebugScreen.this, text, Toast.LENGTH_SHORT).show();
    }
}
