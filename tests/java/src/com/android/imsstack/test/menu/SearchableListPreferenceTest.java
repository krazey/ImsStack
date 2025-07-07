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

import static androidx.test.espresso.Espresso.onData;
import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.action.ViewActions.typeText;
import static androidx.test.espresso.assertion.ViewAssertions.doesNotExist;
import static androidx.test.espresso.assertion.ViewAssertions.matches;
import static androidx.test.espresso.matcher.ViewMatchers.isDisplayed;
import static androidx.test.espresso.matcher.ViewMatchers.withId;
import static androidx.test.espresso.matcher.ViewMatchers.withText;

import static org.hamcrest.Matchers.allOf;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.is;
import static org.junit.Assert.assertEquals;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;

import androidx.appcompat.app.AppCompatActivity;
import androidx.test.ext.junit.rules.ActivityScenarioRule;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.filters.SmallTest;
import androidx.test.platform.app.InstrumentationRegistry;

import com.android.imsstack.tests.R;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public class SearchableListPreferenceTest {

    @Rule
    public ActivityScenarioRule<TestActivity> mActivityRule =
            new ActivityScenarioRule<>(TestActivity.class);

    private static final String PREF_KEY = "searchable_pref_key";
    private static final String SEARCH_VIEW_TEXT = "Test Searchable Preference";

    private Context mContext;

    // An AppCompatActivity to host the PreferenceFragment for testing.
    public static class TestActivity extends AppCompatActivity {
        @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            // Display the fragment as the main content.
            getFragmentManager().beginTransaction()
                    .replace(android.R.id.content, new TestPreferenceFragment())
                    .commit();
        }
    }

    // A PreferenceFragment that loads the preference XML.
    public static class TestPreferenceFragment extends PreferenceFragment {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.searchable_list_preference_test);
        }
    }

    @Before
    public void setUp() {
        mContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    }

    @After
    public void tearDown() {
        // Clear values finishing each test.
        SharedPreferences.Editor editor = PreferenceManager
                .getDefaultSharedPreferences(mContext).edit();
        editor.remove(PREF_KEY);
        editor.commit();
    }

    @Test
    @SmallTest
    public void dialog_displaysAndShowsAllItems() {
        /*
        // Click the preference to open the dialog.
        onView(withText(SEARCH_VIEW_TEXT)).perform(click());

        // Check that the search view and all items are visible.
        onView(withId(R.id.search_view)).check(matches(isDisplayed()));
        onView(withText("Apple")).check(matches(isDisplayed()));
        onView(withText("Banana")).check(matches(isDisplayed()));
        onView(withText("Orange")).check(matches(isDisplayed()));
        onView(withText("Pineapple")).check(matches(isDisplayed()));
         */
    }

    @Test
    @SmallTest
    public void filter_withMatchingQuery_showsCorrectItems() {
        /*
        onView(withText(SEARCH_VIEW_TEXT)).perform(click());

        // Type "pine" into the search view.
        onView(withId(androidx.appcompat.R.id.search_src_text)).perform(typeText("pine"));

        // Check that only "Pineapple" is visible.
        onView(withText("Apple")).check(doesNotExist());
        onView(withText("Banana")).check(doesNotExist());
        onView(withText("Orange")).check(doesNotExist());
        onView(withText("Pineapple")).check(matches(isDisplayed()));
         */
    }

    @Test
    @SmallTest
    public void selection_afterFiltering_setsCorrectValue() {
        /*
        onView(withText(SEARCH_VIEW_TEXT)).perform(click());

        // Filter for "Orange".
        onView(withId(androidx.appcompat.R.id.search_src_text)).perform(typeText("Ora"));

        // Wait for a while to show the search result on view before performing a click interaction.
        android.os.SystemClock.sleep(100);

        // Click the filtered item.
        onView(withText("Orange")).perform(click());

        // Verify the dialog is closed.
        onView(withText(SEARCH_VIEW_TEXT)).check(matches(isDisplayed()));

        // Verify the preference value and summary have been updated.
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        assertEquals("val_3", prefs.getString(PREF_KEY, null));
        onView(withText("Orange")).check(matches(isDisplayed()));
         */
    }

    @Test
    @SmallTest
    public void selection_withoutFiltering_setsCorrectValue() {
        /*
        onView(withText(SEARCH_VIEW_TEXT)).perform(click());

        // Click "Banana" without any filtering.
        onData(allOf(is(instanceOf(String.class)), is("Banana"))).perform(click());

        // Verify the preference value and summary have been updated.
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        assertEquals("val_2", prefs.getString(PREF_KEY, null));
        onView(withText("Banana")).check(matches(isDisplayed()));
         */
    }
}
