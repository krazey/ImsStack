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

package com.android.imsstack.enabler.ssc;

public enum ESsType {

    /**
     * NONE
     */
    NONE(null , null),
    /**
     * Originating Identification Presentation
     */
    OIP(SscXmlFormat.OIP, "oip_oir.xsd"),
    /**
     * Originating Identification Restriction
     */
    OIR(SscXmlFormat.OIR, "oip_oir.xsd"),
    /**
     * Terminating Identification Presentation
     */
    TIP(SscXmlFormat.TIP, "tip_tir.xsd"),
    /**
     * Termination Identification Restriction
     */
    TIR(SscXmlFormat.TIR, "tip_tir.xsd"),

    /**
     * Communication Diversion
     */
    CF(SscXmlFormat.CD, "cf.xsd"),
    /**
     * Communication Diversion - Service Capability
     */
    CFSC(SscXmlFormat.SC_CD, "cf_serv_cap.xsd"),

    /**
     * Communication Barring
     */
    OCB(SscXmlFormat.OCB, "cb.xsd"),
    ICB(SscXmlFormat.ICB, "cb.xsd"),
    /**
     * FIXME :: To Handle icb for KDDI
     * Rename : ICB --> ICBA
     */
    ICBA(SscXmlFormat.ICB, "cb.xsd"),
    /**
     * Communication Barring - Service Capability
     */
    CBSC(SscXmlFormat.SC_CB, "cb_serv_cap.xsd"),

    /**
     * Communication Waiting
     */
    CW(SscXmlFormat.CW, "cw.xsd"),

    /**
     * xcap error
     */
    XE(SscXmlFormat.XCAPERROR, "xe.xsd");


    private String mSSName = null;
    private String mSchemaFileName = null;

    private ESsType(String name, String xsd) {
        this.mSSName = name;
        this.mSchemaFileName = xsd;
    }

    public String getSsName() {
        return this.mSSName;
    }

    public String getSchema() {
        return this.mSchemaFileName;
    }

    public String toString() {
        return this.name();
    }
}
