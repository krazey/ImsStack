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

// Tag Definitions
//    Enumeration Type: "IMS_TRACE_TAG_" prefix will be used
//    Tag Name
//    Tag Module: "IMS_TRACE_MODULE_" prefix will be used

// COMMON -- starts
__IMS_TRACE_TAG_DEF__(IMS, "IMS", IMS)
__IMS_TRACE_TAG_DEF__(REG, "REG", REG)
__IMS_TRACE_TAG_DEF__(CORE, "CORE", CORE)
__IMS_TRACE_TAG_DEF__(BASE, "BASE", BASE)
__IMS_TRACE_TAG_DEF__(ADAPT, "ADAPT", BASE)
__IMS_TRACE_TAG_DEF__(CONF, "CONF", BASE)
__IMS_TRACE_TAG_DEF__(SIP, "SIP", SIP)
__IMS_TRACE_TAG_DEF__(XML, "XML", XML)
__IMS_TRACE_TAG_DEF__(SDP, "SDP", SDP)
__IMS_TRACE_TAG_DEF__(AOS, "AOS", AOS)
// COMMON -- ends

// ENABLER -- starts
__IMS_TRACE_TAG_DEF__(MTC, "MTC", ENABLER_MTC)
__IMS_TRACE_TAG_DEF__(MTS, "MTS", ENABLER_MTS)
__IMS_TRACE_TAG_DEF__(UCE, "UCE", ENABLER_UCE)
__IMS_TRACE_TAG_DEF__(SIP_DELEGATE, "SipDelegate", ENABLER_SIP_DELEGATE)
__IMS_TRACE_TAG_DEF__(MEDIA, "MEDIA", ENABLER_MEDIA)
// ENABLER -- ends
