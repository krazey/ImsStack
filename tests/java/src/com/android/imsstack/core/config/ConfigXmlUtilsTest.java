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

package com.android.imsstack.core.config;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.fail;

import static java.nio.charset.StandardCharsets.UTF_8;

import androidx.test.filters.SmallTest;

import com.android.imsstack.util.ImsUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.ArrayList;

@RunWith(JUnit4.class)
public class ConfigXmlUtilsTest {
    private XmlPullParserFactory mFactory;
    private XmlPullParser mParser;
    private ByteArrayInputStream mInputStream;

    @Before
    public void setUp() throws Exception {
        if (mFactory == null) {
            mFactory = XmlPullParserFactory.newInstance();
        }
    }

    @After
    public void tearDown() throws Exception {
        ImsUtils.closeQuietly(mInputStream);
        mInputStream = null;
        mParser = null;
    }

    @Test
    @SmallTest
    public void testReadConfig() throws XmlPullParserException, IOException {
        String xmlConfig =
                "<config>\n"
                + "<boolean name=\"config_bool\" value=\"true\"/>\n"
                + "<int name=\"config_int\" value=\"1\"/>\n"
                + "<long name=\"config_long\" value=\"11\"/>\n"
                + "<string name=\"config_string\">text</string>\n"
                + "<int-array name=\"config_int_array\" num=\"2\">\n"
                + "    <item value=\"1\"/>\n"
                + "    <item value=\"2\"/>\n"
                + "</int-array>\n"
                + "<long-array name=\"config_long_array\" num=\"2\">\n"
                + "    <item value=\"11\"/>\n"
                + "    <item value=\"12\"/>\n"
                + "</long-array>\n"
                + "<string-array name=\"config_string_array\" num=\"2\">\n"
                + "    <item value=\"text1\"/>\n"
                + "    <item value=\"text2\"/>\n"
                + "</string-array>\n"
                + "<pbundle_as_map name=\"config_bundle\">\n"
                + "    <boolean name=\"config_bool\" value=\"false\"/>\n"
                + "    <int name=\"config_int\" value=\"111\"/>\n"
                + "</pbundle_as_map>\n"
                + "</config>";
        setUpXmlParser(xmlConfig);

        assertNotNull(ConfigXmlUtils.readConfig(mParser));
    }

    @Test
    @SmallTest
    public void testReadConfigWhenRootTagMismatched() throws XmlPullParserException {
        String xmlConfigWithoutRootTag =
                "<config>\n"
                + "<boolean name=\"config_bool\" value=\"true\"/>\n"
                + "<int name=\"config_int\" value=\"1\"/>";
        setUpXmlParser(xmlConfigWithoutRootTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithMismatchedRootTag =
                "<config>\n"
                + "<boolean name=\"config_bool\" value=\"true\"/>\n"
                + "<int name=\"config_int\" value=\"1\"/>\n"
                + "</wrong-end>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithMismatchedRootTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });
    }

    @Test
    @SmallTest
    public void testReadConfigWhenBooleanTagMismatched() throws XmlPullParserException {
        String xmlConfigWithoutEndTag =
                "<config>\n"
                + "<boolean name=\"config_bool\" value=\"true\">";
        setUpXmlParser(xmlConfigWithoutEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithMismatchedEndTag =
                "<config>\n"
                + "<boolean name=\"config_bool\" value=\"true\"></bool>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithMismatchedEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithStartTagBeforeEndTag =
                "<config>\n"
                + "<boolean name=\"config_bool\" value=\"true\">\n"
                + "<int name=\"config_int\" value=\"1\"/>\n"
                + "</boolean>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithStartTagBeforeEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithTextTagBeforeEndTag =
                "<config>\n"
                + "<boolean name=\"config_bool\" value=\"true\">true</boolean>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithTextTagBeforeEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });
    }

    @Test
    @SmallTest
    public void testReadConfigWhenStringTagMismatched() throws XmlPullParserException {
        String xmlConfigWithoutEndTag =
                "<config>\n"
                + "<string name=\"config_string\">text";
        setUpXmlParser(xmlConfigWithoutEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithMismatchedEndTag =
                "<config>\n"
                + "<string name=\"config_string\">text</string-end>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithMismatchedEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithStartTagBeforeEndTag =
                "<config>\n"
                + "<string name=\"config_string\">\n"
                + "<int name=\"config_int\" value=\"1\"/>\n"
                + "</string>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithStartTagBeforeEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });
    }

    @Test
    @SmallTest
    public void testReadConfigWhenIntArrayTagMismatched() throws XmlPullParserException {
        String xmlConfigWithoutEndTag =
                "<config>\n"
                + "<int-array name=\"config_int_array\" num=\"2\">\n"
                + "    <item value=\"1\"/>\n"
                + "    <item value=\"2\"/>";
        setUpXmlParser(xmlConfigWithoutEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithMismatchedEndTag =
                "<config>\n"
                + "<int-array name=\"config_int_array\" num=\"2\">\n"
                + "    <item value=\"1\"/>\n"
                + "    <item value=\"2\"/>\n"
                + "</int-array-end>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithMismatchedEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithUnknownTag =
                "<config>\n"
                + "<int-array name=\"config_int_array\" num=\"2\">\n"
                + "    <item value=\"1\"/>\n"
                + "    <unknown value=\"2\"/>\n"
                + "</int-array>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithUnknownTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });
    }

    @Test
    @SmallTest
    public void testReadConfigWhenLongArrayTagMismatched() throws XmlPullParserException {
        String xmlConfigWithoutEndTag =
                "<config>\n"
                + "<long-array name=\"config_long_array\" num=\"2\">\n"
                + "    <item value=\"11\"/>\n"
                + "    <item value=\"22\"/>";
        setUpXmlParser(xmlConfigWithoutEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithMismatchedEndTag =
                "<config>\n"
                + "<long-array name=\"config_long_array\" num=\"2\">\n"
                + "    <item value=\"11\"/>\n"
                + "    <item value=\"22\"/>\n"
                + "</long-array-end>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithMismatchedEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithUnknownTag =
                "<config>\n"
                + "<long-array name=\"config_long_array\" num=\"2\">\n"
                + "    <item value=\"1\"/>\n"
                + "    <unknown value=\"2\"/>\n"
                + "</long-array>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithUnknownTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });
    }

    @Test
    @SmallTest
    public void testReadConfigWhenStringArrayTagMismatched() throws XmlPullParserException {
        String xmlConfigWithoutEndTag =
                "<config>\n"
                + "<string-array name=\"config_string_array\" num=\"2\">\n"
                + "    <item value=\"text1\"/>\n"
                + "    <item value=\"text2\"/>";
        setUpXmlParser(xmlConfigWithoutEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithMismatchedEndTag =
                "<config>\n"
                + "<string-array name=\"config_string_array\" num=\"2\">\n"
                + "    <item value=\"text1\"/>\n"
                + "    <item value=\"text2\"/>\n"
                + "</string-array-end>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithMismatchedEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithUnknownTag =
                "<config>\n"
                + "<string-array name=\"config_string_array\" num=\"2\">\n"
                + "    <item value=\"text1\"/>\n"
                + "    <unknown value=\"text2\"/>\n"
                + "</string-array>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithUnknownTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });
    }

    @Test
    @SmallTest
    public void testReadConfigWhenUnknownTagOrValueInvalid() throws XmlPullParserException {
        String xmlConfigWithUnknownTag =
                "<config>\n"
                + "<bool name=\"config_bool\" value=\"true\"/>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithUnknownTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithInvalidBooleanValue =
                "<config>\n"
                + "<boolean name=\"config_bool\" value=\"abc\"/>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithInvalidBooleanValue);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithInvalidIntValue =
                "<config>\n"
                + "<int name=\"config_int\" value=\"abc\"/>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithInvalidIntValue);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });

        String xmlConfigWithInvalidLongValue =
                "<config>\n"
                + "<long name=\"config_long\" value=\"abc\"/>\n"
                + "</config>";
        setUpXmlParser(xmlConfigWithInvalidLongValue);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfig(mParser);
        });
    }

    @Test
    @SmallTest
    public void testReadConfigKeys() throws XmlPullParserException, IOException {
        ArrayList<String> configKeys = new ArrayList<>();
        String xmlConfig =
                "<config>\n"
                + "<boolean name=\"config_bool\" value=\"true\"/>\n"
                + "<int name=\"config_int\" value=\"1\"/>\n"
                + "<long name=\"config_long\" value=\"11\"/>\n"
                + "<string name=\"config_string\">text</string>\n"
                + "<int-array name=\"config_int_array\" num=\"2\">\n"
                + "    <item value=\"1\"/>\n"
                + "    <item value=\"2\"/>\n"
                + "</int-array>\n"
                + "<long-array name=\"config_long_array\" num=\"2\">\n"
                + "    <item value=\"11\"/>\n"
                + "    <item value=\"12\"/>\n"
                + "</long-array>\n"
                + "<string-array name=\"config_string_array\" num=\"2\">\n"
                + "    <item value=\"text1\"/>\n"
                + "    <item value=\"text2\"/>\n"
                + "</string-array>\n"
                + "<pbundle_as_map name=\"config_bundle\">\n"
                + "    <pbundle_as_map name=\"inner_config_bundle\">\n"
                + "        <boolean name=\"config_bool\" value=\"true\"/>\n"
                + "        <int name=\"config_int\" value=\"111\"/>\n"
                + "    </pbundle_as_map>\n"
                + "</pbundle_as_map>\n"
                + "</config>";
        setUpXmlParser(xmlConfig);
        ConfigXmlUtils.readConfigKeys(mParser, configKeys);

        assertEquals(8, configKeys.size());

        configKeys.clear();
        String xmlConfigWithoutEndTag =
                "<config>\n"
                + "<boolean name=\"config_bool\" value=\"true\"/>\n"
                + "<int name=\"config_int\" value=\"1\"/>\n"
                + "<long name=\"config_long\" value=\"11\"/>\n"
                + "<string name=\"config_string\">text</string>";
        setUpXmlParser(xmlConfigWithoutEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfigKeys(mParser, configKeys);
        });

        configKeys.clear();
        String xmlConfigWithMismatchedEndTag =
                "<config>\n"
                + "<boolean name=\"config_bool\" value=\"true\"/>\n"
                + "<int name=\"config_int\" value=\"1\"/>\n"
                + "<long name=\"config_long\" value=\"11\"/>\n"
                + "<string name=\"config_string\">text</string>\n"
                + "</config-end>";
        setUpXmlParser(xmlConfigWithMismatchedEndTag);

        assertThrows(XmlPullParserException.class, () -> {
            ConfigXmlUtils.readConfigKeys(mParser, configKeys);
        });
    }

    private void setUpXmlParser(String xml) throws XmlPullParserException {
        if (mInputStream != null) {
            ImsUtils.closeQuietly(mInputStream);
            mInputStream = null;
        }
        try {
            mInputStream = new ByteArrayInputStream(xml.getBytes(UTF_8.name()));
            mParser = mFactory.newPullParser();
            mParser.setInput(mInputStream, UTF_8.name());
            // Consume the document element.
            mParser.next();
        } catch (IOException e) {
            fail("setUpXmlParser fails: " + e.toString());
        }
    }
}
