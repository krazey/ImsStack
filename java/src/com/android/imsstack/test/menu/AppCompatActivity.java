/*
 * Copyright (C) 2024 The Android Open Source Project
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

import android.content.res.Configuration;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.view.MenuInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.LayoutRes;
import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.appcompat.widget.Toolbar;

@SuppressWarnings("deprecation")
public class AppCompatActivity extends PreferenceActivity {
    private AppCompatDelegate mCompatDelegate;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        mCompatDelegate = AppCompatDelegate.create(this, null);
        mCompatDelegate.installViewFactory();
        super.onCreate(savedInstanceState);
        mCompatDelegate.onCreate(savedInstanceState);
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
            actionBar.setDisplayShowTitleEnabled(true);
        }
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        mCompatDelegate.onPostCreate(savedInstanceState);
    }

    public void setSupportActionBar(Toolbar toolbar) {
        mCompatDelegate.setSupportActionBar(toolbar);
    }

    @NonNull
    @Override
    public MenuInflater getMenuInflater() {
        return mCompatDelegate.getMenuInflater();
    }

    @Override
    public void setContentView(@LayoutRes int layoutRes) {
        mCompatDelegate.setContentView(layoutRes);
    }

    @Override
    public void setContentView(View view) {
        mCompatDelegate.setContentView(view);
    }

    @Override
    public void setContentView(View view, ViewGroup.LayoutParams params) {
        mCompatDelegate.setContentView(view, params);
    }

    @Override
    public void addContentView(View view, ViewGroup.LayoutParams params) {
        mCompatDelegate.addContentView(view, params);
    }

    @Override
    protected void onPostResume() {
        super.onPostResume();
        mCompatDelegate.onPostResume();
    }

    @Override
    protected void onTitleChanged(CharSequence title, int color) {
        super.onTitleChanged(title, color);
        mCompatDelegate.setTitle(title);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mCompatDelegate.onConfigurationChanged(newConfig);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mCompatDelegate.onDestroy();
    }

    @Override
    public void invalidateOptionsMenu() {
        mCompatDelegate.invalidateOptionsMenu();
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return fragmentName != null;
    }

    protected ActionBar getSupportActionBar() {
        return mCompatDelegate.getSupportActionBar();
    }
}
