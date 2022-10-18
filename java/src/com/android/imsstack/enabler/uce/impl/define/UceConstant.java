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

package com.android.imsstack.enabler.uce.impl.define;

public class UceConstant {

  public static final int RADIO_TECHNOLOGY_TYPE_UNKNOWN               = -1;
  public static final int RADIO_TECHNOLOGY_TYPE_GERAN                 = 0;
  public static final int RADIO_TECHNOLOGY_TYPE_HRPD                  = 1;
  public static final int RADIO_TECHNOLOGY_TYPE_UTRAN                 = 2;
  public static final int RADIO_TECHNOLOGY_TYPE_EHRPD                 = 3;
  public static final int RADIO_TECHNOLOGY_TYPE_LTE                   = 4;
  public static final int RADIO_TECHNOLOGY_TYPE_LTE_NO_VOPS           = 5;
  public static final int RADIO_TECHNOLOGY_TYPE_WIFI                  = 6;
  public static final int RADIO_TECHNOLOGY_TYPE_NR                    = 7;
  public static final int RADIO_TECHNOLOGY_TYPE_NR_NO_VOPS            = 8;

  // Bundle Get
  public static final String BUNDLE_GET_REQUEST_KEY                   = "requestKey";
  public static final String BUNDLE_GET_RESPONSE_CODE                 = "responseCode";
  public static final String BUNDLE_GET_REASON                        = "reason";
  public static final String BUNDLE_GET_REASON_HEADER_CAUSE           = "reasonHeaderCause";
  public static final String BUNDLE_GET_REASON_HEADER_TEXT            = "reasonHeaderText";
  public static final String BUNDLE_PIDF_XML                          = "pidfXml";

  // preference
  public static final String PREFERENCE_ETAG                          = "publish_etag";
}