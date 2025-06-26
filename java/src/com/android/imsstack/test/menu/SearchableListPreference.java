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
package com.android.imsstack.test.menu;

import android.app.AlertDialog;
import android.content.Context;
import android.preference.ListPreference;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import androidx.appcompat.widget.SearchView;

import com.android.imsstack.R;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;

public class SearchableListPreference extends ListPreference {

    private ArrayAdapter<CharSequence> mAdapter;
    private CharSequence[] mOriginalEntries;
    private CharSequence[] mOriginalEntryValues;
    private ListView mListView;
    private SearchView mSearchView;

    public SearchableListPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public SearchableListPreference(Context context) {
        super(context);
    }

    @Override
    protected void onPrepareDialogBuilder(AlertDialog.Builder builder) {
        mOriginalEntries = getEntries();
        mOriginalEntryValues = getEntryValues();

        LayoutInflater inflater = LayoutInflater.from(getContext());
        View customView = inflater.inflate(R.layout.searchable_list_preference_dialog, null);

        mListView = customView.findViewById(android.R.id.list);
        mSearchView = customView.findViewById(R.id.search_view);

        mAdapter = new ArrayAdapter<>(getContext(), android.R.layout.simple_list_item_single_choice,
                new ArrayList<>(Arrays.asList(mOriginalEntries)));
        mListView.setAdapter(mAdapter);

        int selectedIndex = findIndexOfValue(getValue());
        if (selectedIndex != -1) {
            mListView.setItemChecked(selectedIndex, true);
            mListView.setSelection(selectedIndex);
        }

        mListView.setOnItemClickListener((parent, view, position, id) -> {
            CharSequence selectedEntry = mAdapter.getItem(position);
            for (int i = 0; i < mOriginalEntries.length; i++) {
                if (mOriginalEntries[i].toString().equals(selectedEntry.toString())) {
                    String selectedValue = mOriginalEntryValues[i].toString();
                    if (callChangeListener(selectedValue)) {
                        setValue(selectedValue);
                    }
                    break;
                }
            }

            getDialog().dismiss();
        });

        // Set up search view
        mSearchView.setOnQueryTextListener(new SearchView.OnQueryTextListener() {
            @Override
            public boolean onQueryTextSubmit(String query) {
                return false;
            }

            @Override
            public boolean onQueryTextChange(String newText) {
                filterList(newText);
                return true;
            }
        });

        builder.setView(customView);
        builder.setPositiveButton(null, null);
        builder.setNegativeButton(null, null);
        builder.setTitle(getDialogTitle());
    }

    private void filterList(String query) {
        mAdapter.clear();
        if (query == null || query.isEmpty()) {
            mAdapter.addAll(mOriginalEntries);
        } else {
            String lowerCaseQuery = query.toLowerCase(Locale.US);
            List<CharSequence> filteredEntries = new ArrayList<>();
            for (CharSequence entry : mOriginalEntries) {
                if (entry.toString().toLowerCase(Locale.US).contains(lowerCaseQuery)) {
                    filteredEntries.add(entry);
                }
            }
            mAdapter.addAll(filteredEntries);
        }
        mAdapter.notifyDataSetChanged();
    }
}
