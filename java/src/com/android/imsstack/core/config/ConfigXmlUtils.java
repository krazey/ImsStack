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

package com.android.imsstack.core.config;

import android.os.PersistableBundle;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.Collection;

/** A class for providing the utility method of the configuration XML parser. */
public final class ConfigXmlUtils {
    private static final String TAG_PERSISTABLE_BUNDLE = "pbundle_as_map";

    /**
     * Parses the XML configuration and returns the structured format as {@link PersistableBundle}.
     *
     * @param parser A {@link XmlPullParser} containing XML configuration.
     * @return A {@link PersistableBundle} instance.
     * @throws XmlPullParserException If there was an XML parsing error.
     * @throws IOException If there was an I/O error.
     */
    public static PersistableBundle readConfig(XmlPullParser parser)
            throws XmlPullParserException, IOException {
        PersistableBundle config = new PersistableBundle();
        String tagName = parser.getName();
        int event;

        while ((event = parser.next()) != XmlPullParser.END_DOCUMENT) {
            if (event == XmlPullParser.START_TAG) {
                readConfig(parser, config);
            } else if (event == XmlPullParser.END_TAG) {
                if (parser.getName().equals(tagName)) {
                    return config;
                }
                throw new XmlPullParserException(
                        "Expected " + tagName + " end tag at: " + parser.getName());
            }
        }

        throw new XmlPullParserException(
                "Document ended before " + tagName + " end tag");
    }

    /**
     * Reads the configuration keys from the given XML configuration.
     *
     * @param parser A {@link XmlPullParser} containing XML configuration.
     * @param configKeys All the configuration keys of XML.
     * @throws XmlPullParserException If there was an XML parsing error.
     * @throws IOException If there was an I/O error.
     */
    public static void readConfigKeys(XmlPullParser parser, Collection<String> configKeys)
            throws XmlPullParserException, IOException {
        String rootName = parser.getName();
        int event;

        while ((event = parser.next()) != XmlPullParser.END_DOCUMENT) {
            if (event == XmlPullParser.START_TAG) {
                String tagName = parser.getName();

                configKeys.add(parser.getAttributeValue(null, "name"));

                int innerEvent;
                int sameTagCount = 1;
                while ((innerEvent = parser.next()) != XmlPullParser.END_DOCUMENT) {
                    if (innerEvent == XmlPullParser.START_TAG) {
                        if (parser.getName().equals(tagName)) {
                            sameTagCount++;
                        }
                    } else if (innerEvent == XmlPullParser.END_TAG) {
                        if (parser.getName().equals(tagName)) {
                            sameTagCount--;
                            if (sameTagCount == 0) {
                                break;
                            }
                        }
                    }
                }
            } else if (event == XmlPullParser.END_TAG) {
                if (parser.getName().equals(rootName)) {
                    return;
                }
                throw new XmlPullParserException(
                        "Expected " + rootName + " end tag at: " + parser.getName());
            }
        }

        throw new XmlPullParserException(
                "Document ended before " + rootName + " end tag");
    }

    private static void readConfig(XmlPullParser parser, PersistableBundle config)
            throws XmlPullParserException, IOException {
        final String key = parser.getAttributeValue(null, "name");
        final String tagName = parser.getName();

        if (tagName.equals("string")) {
            final StringBuilder value = new StringBuilder();
            int event;
            while ((event = parser.next()) != XmlPullParser.END_DOCUMENT) {
                if (event == XmlPullParser.END_TAG) {
                    if (parser.getName().equals("string")) {
                        config.putString(key, value.toString());
                        return;
                    }
                    throw new XmlPullParserException(
                            "Unexpected end tag in <string>: " + parser.getName());
                } else if (event == XmlPullParser.TEXT) {
                    value.append(parser.getText());
                } else if (event == XmlPullParser.START_TAG) {
                    throw new XmlPullParserException(
                            "Unexpected start tag in <string>: " + parser.getName());
                }
            }
            throw new XmlPullParserException(
                    "Unexpected end of document in <string>");
        } else if (tagName.equals("int")) {
            config.putInt(key, getValueInt(parser.getAttributeValue(null, "value")));
        } else if (tagName.equals("long")) {
            config.putLong(key, getValueLong(parser.getAttributeValue(null, "value")));
        } else if (tagName.equals("boolean")) {
            config.putBoolean(key, getValueBoolean(parser.getAttributeValue(null, "value")));
        } else if (tagName.equals("int-array")) {
            config.putIntArray(key, readIntArray(parser, "int-array"));
            return;
        } else if (tagName.equals("long-array")) {
            config.putLongArray(key, readLongArray(parser, "long-array"));
            return;
        } else if (tagName.equals("string-array")) {
            config.putStringArray(key, readStringArray(parser, "string-array"));
            return;
        } else if (tagName.equals(TAG_PERSISTABLE_BUNDLE)) {
            config.putPersistableBundle(key, readConfig(parser));
            return;
        } else {
            throw new XmlPullParserException("Unknown tag: " + tagName);
        }

        // Skip through to end tag.
        int event;
        while ((event = parser.next()) != XmlPullParser.END_DOCUMENT) {
            if (event == XmlPullParser.END_TAG) {
                if (parser.getName().equals(tagName)) {
                    return;
                }
                throw new XmlPullParserException(
                        "Unexpected end tag in <" + tagName + ">: " + parser.getName());
            } else if (event == XmlPullParser.TEXT) {
                throw new XmlPullParserException(
                        "Unexpected text in <" + tagName + ">: " + parser.getName());
            } else if (event == XmlPullParser.START_TAG) {
                throw new XmlPullParserException(
                        "Unexpected start tag in <" + tagName + ">: " + parser.getName());
            }
        }
        throw new XmlPullParserException(
                "Unexpected end of document in <" + tagName + ">");
    }

    private static boolean getValueBoolean(String value) throws XmlPullParserException {
        if ("true".equalsIgnoreCase(value)) {
            return true;
        } else if ("false".equalsIgnoreCase(value)) {
            return false;
        } else {
            throw new XmlPullParserException("Invalid attribute(boolean=" + value + ")");
        }
    }

    private static int getValueInt(String value) throws XmlPullParserException {
        try {
            return Integer.parseInt(value);
        } catch (Exception e) {
            throw new XmlPullParserException("Invalid attribute(int=" + value + "): " + e);
        }
    }

    private static long getValueLong(String value) throws XmlPullParserException {
        try {
            return Long.parseLong(value);
        } catch (Exception e) {
            throw new XmlPullParserException("Invalid attribute(long=" + value + "): " + e);
        }
    }

    private static int[] readIntArray(XmlPullParser parser, String endTag)
            throws XmlPullParserException, IOException {
        int num = getValueInt(parser.getAttributeValue(null, "num"));
        int[] array = new int[num];
        int i = 0;

        int event;
        while ((event = parser.next()) != XmlPullParser.END_DOCUMENT) {
            if (event == XmlPullParser.START_TAG) {
                if (parser.getName().equals("item")) {
                    array[i] = getValueInt(parser.getAttributeValue(null, "value"));
                } else {
                    throw new XmlPullParserException(
                            "Expected item tag at: " + parser.getName());
                }
            } else if (event == XmlPullParser.END_TAG) {
                if (parser.getName().equals(endTag)) {
                    return array;
                } else if (parser.getName().equals("item")) {
                    i++;
                } else {
                    throw new XmlPullParserException(
                            "Expected " + endTag + " end tag at: " + parser.getName());
                }
            }
        }

        throw new XmlPullParserException(
                "Document ended before " + endTag + " end tag");
    }

    private static long[] readLongArray(XmlPullParser parser, String endTag)
            throws XmlPullParserException, IOException {
        int num = getValueInt(parser.getAttributeValue(null, "num"));
        long[] array = new long[num];
        int i = 0;

        int event;
        while ((event = parser.next()) != XmlPullParser.END_DOCUMENT) {
            if (event == XmlPullParser.START_TAG) {
                if (parser.getName().equals("item")) {
                    array[i] = getValueLong(parser.getAttributeValue(null, "value"));
                } else {
                    throw new XmlPullParserException(
                            "Expected item tag at: " + parser.getName());
                }
            } else if (event == XmlPullParser.END_TAG) {
                if (parser.getName().equals(endTag)) {
                    return array;
                } else if (parser.getName().equals("item")) {
                    i++;
                } else {
                    throw new XmlPullParserException(
                            "Expected " + endTag + " end tag at: " + parser.getName());
                }
            }
        }

        throw new XmlPullParserException(
                "Document ended before " + endTag + " end tag");
    }

    private static String[] readStringArray(XmlPullParser parser, String endTag)
            throws XmlPullParserException, IOException {
        int num = getValueInt(parser.getAttributeValue(null, "num"));
        String[] array = new String[num];
        int i = 0;

        int event;
        while ((event = parser.next()) != XmlPullParser.END_DOCUMENT) {
            if (event == XmlPullParser.START_TAG) {
                if (parser.getName().equals("item")) {
                    array[i] = parser.getAttributeValue(null, "value");
                } else {
                    throw new XmlPullParserException(
                            "Expected item tag at: " + parser.getName());
                }
            } else if (event == XmlPullParser.END_TAG) {
                if (parser.getName().equals(endTag)) {
                    return array;
                } else if (parser.getName().equals("item")) {
                    i++;
                } else {
                    throw new XmlPullParserException(
                            "Expected " + endTag + " end tag at: " + parser.getName());
                }
            }
        }

        throw new XmlPullParserException(
                "Document ended before " + endTag + " end tag");
    }

    private ConfigXmlUtils() {}
}
