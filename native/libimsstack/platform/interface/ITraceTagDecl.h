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
#ifndef INTERFACE_TRACE_TAG_DECL_H_
#define INTERFACE_TRACE_TAG_DECL_H_

////
// TRACE MODULE DEFINITIONS FOR FRAMEWORK & ENABLERS
////

//// IMS ("ims" package)
//// engine -> connection/service, etc...
#define __IMS_TRACE_TAG_IMS__          __IMS_TRACE_TAG__(IMS)

//// IMS ("ims" package)
//// engine -> registration
#define __IMS_TRACE_TAG_REG__          __IMS_TRACE_TAG__(REG)

//// CORE, CORE.MEDIA ("ims.core", "ims.core.media" package)
//// engine -> core, core/media
#define __IMS_TRACE_TAG_IMS_CORE__     __IMS_TRACE_TAG__(CORE)

//// PLATFORM
//// platform -> base
#define __IMS_TRACE_TAG_BASE__         __IMS_TRACE_TAG__(BASE)

//// ADAPT
//// platform -> os
#define __IMS_TRACE_TAG_ADAPT__        __IMS_TRACE_TAG__(ADAPT)

//// CONF
//// config
#define __IMS_TRACE_TAG_CONF__         __IMS_TRACE_TAG__(CONF)

//// SIP
//// engine -> sipcore, protocol/sip -> sip
#define __IMS_TRACE_TAG_SIP__          __IMS_TRACE_TAG__(SIP)

//// XML
//// protocol/xml -> xml
#define __IMS_TRACE_TAG_XML__          __IMS_TRACE_TAG__(XML)

//// SDP
//// protocol/sdp -> sdp
#define __IMS_TRACE_TAG_SDP__          __IMS_TRACE_TAG__(SDP)

//// AOS
//// enabler -> aos
#define __IMS_TRACE_TAG_AOS__          __IMS_TRACE_TAG__(AOS)

//// MTC
//// enabler -> mtc
#define __IMS_TRACE_TAG_COM_MTC__      __IMS_TRACE_TAG__(MTC)
#define __IMS_TRACE_TAG_MTC__          __IMS_TRACE_TAG__(MTC)

//// MTS
//// enabler -> mts
#define __IMS_TRACE_TAG_COM_MTS__      __IMS_TRACE_TAG__(MTS)
#define __IMS_TRACE_TAG_MTS__          __IMS_TRACE_TAG__(MTS)

//// UCE
//// enabler -> uce
#define __IMS_TRACE_TAG_UCE__          __IMS_TRACE_TAG__(UCE)

//// SIP_DELEGATE
//// enabler -> sipdelegate
#define __IMS_TRACE_TAG_SIP_DELEGATE__ __IMS_TRACE_TAG__(SIP_DELEGATE)

//// MEDIA
//// enabler -> media
#define __IMS_TRACE_TAG_MEDIA__        __IMS_TRACE_TAG__(MEDIA)

#endif
