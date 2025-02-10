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
#include "ImsUuid.h"
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "TextParser.h"

#include "Engine.h"
#include "IConfiguration.h"
#include "ISubscriberConfig.h"

#include "IXmlStreamWriter.h"
#include "XmlFactory.h"

#include "GeolocationPidfCreator.h"
#include "GeolocationPidfWriter.h"

#include <initializer_list>

__IMS_TRACE_TAG_BASE__;

LOCAL const AString DEFAULT_TUPLE_ID = "VoLte";
LOCAL const AString DEFAULT_DEVICE_NAME = "Phone";

using namespace enabler;

PUBLIC
GeolocationPidfCreator::GeolocationPidfCreator(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_nFeatures(0),
        m_strDeviceName(AString::ConstNull()),
        m_strDeviceId(AString::ConstNull()),
        m_strTupleId(AString::ConstNull())
{
}

PUBLIC VIRTUAL GeolocationPidfCreator::~GeolocationPidfCreator() {}

PUBLIC
IMS_BOOL GeolocationPidfCreator::CreateWithoutPosition(IN const AString& strEntityUri,
        IN IMS_BOOL bUnknownCountryAllowed, IN IMS_BOOL bIncludeState,
        OUT ByteArray& objContent) const
{
    ILocationProperties* piLocation =
            GetLocationProperties(bIncludeState ? ILocationInfo::LOCATION_ALL
                                                : ILocationInfo::LOCATION_POSITION_N_COUNTRY);
    if (piLocation == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const AString& strCountry = piLocation->GetCountry();
    if (!bUnknownCountryAllowed && (strCountry.GetLength() == 0 || strCountry.Equals("ZZ")))
    {
        IMS_TRACE_I("Country is empty or unknown", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strState = bIncludeState ? piLocation->GetState() : AString::ConstNull();

    // clang-format off
    objContent = PidfLoXml{
        new Presence{
            Presence::Namespace::COUNTRY | Presence::Namespace::GBP,
            strEntityUri.GetLength() > 0 ? strEntityUri : CreateEntityUri(), {
            CreateLocationElement(piLocation->GetCurrentTime(), {
                new Geopriv{
                    new LocationInfo{
                        new CivicAddress{
                            strCountry,
                            strState,
                        },
                    },
                    new UsageRules{
                        new RetransmissionAllowed{m_strRetransmissionAllowed},
                    },
                },
            }),
        }},
    }.Write();
    // clang-format on

    return objContent.GetLength() != 0;
}

PUBLIC
IMS_BOOL GeolocationPidfCreator::CreateWithPosition(IN const AString& strEntityUri,
        OUT ByteArray& objContent, IN IMS_SINT32 nConfidence /*= 0*/) const
{
    ILocationProperties* piLocation = GetLocationProperties();
    if (piLocation == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!IsPositionAvailable(piLocation))
    {
        return IMS_FALSE;
    }

    AString strCountry = piLocation->GetCountry();
    if (IsFeatureSet(FEATURE_NO_COUNTRY_IF_UNKNOWN) && strCountry.Equals("ZZ"))
    {
        strCountry = AString::ConstNull();
    }

    AString strConfidence;
    if (nConfidence > 0)
    {
        strConfidence.Sprintf("%d", nConfidence);
    }
    else
    {
        strConfidence = piLocation->GetConfidence();
    }

    AString strMethod =
            IsFeatureSet(FEATURE_NO_METHOD) ? AString::ConstNull() : piLocation->GetMethod();

    // clang-format off
    objContent = PidfLoXml{
        new Presence{Presence::Namespace::ALL, strEntityUri, {
            CreateLocationElement(piLocation->GetCurrentTime(), {
                new Geopriv{
                    new LocationInfo{
                        new CivicAddress{
                            strCountry,
                            piLocation->GetState(),
                            piLocation->GetCity(),
                            piLocation->GetPostal(),
                        },
                        CreateShapeElement(*piLocation),
                        new Confidence{strConfidence},
                    },
                    new UsageRules{
                        new RetransmissionAllowed{m_strRetransmissionAllowed},
                    },
                    new Method{strMethod},
                },
            }),
        }},
    }.Write();
    // clang-format on

    return objContent.GetLength() != 0;
}

PUBLIC
IMS_BOOL GeolocationPidfCreator::CreateWithPositionAndCountry(IN const AString& strEntityUri,
        OUT ByteArray& objContent, IN IMS_SINT32 nConfidence /*= 0*/) const
{
    ILocationProperties* piLocation =
            GetLocationProperties(ILocationInfo::LOCATION_POSITION_N_COUNTRY);
    if (piLocation == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!IsPositionAvailable(piLocation))
    {
        return IMS_FALSE;
    }

    AString strCountry = piLocation->GetCountry();
    if (IsFeatureSet(FEATURE_NO_COUNTRY_IF_UNKNOWN) && strCountry.Equals("ZZ"))
    {
        strCountry = AString::ConstNull();
    }

    AString strConfidence;
    if (nConfidence > 0)
    {
        strConfidence.Sprintf("%d", nConfidence);
    }
    else
    {
        strConfidence = piLocation->GetConfidence();
    }

    AString strMethod =
            IsFeatureSet(FEATURE_NO_METHOD) ? AString::ConstNull() : piLocation->GetMethod();

    // clang-format off
    objContent = PidfLoXml{
        new Presence{Presence::Namespace::ALL, strEntityUri, {
            CreateLocationElement(piLocation->GetCurrentTime(), {
                new Geopriv{
                    new LocationInfo{
                        new CivicAddress{strCountry},
                        CreateShapeElement(*piLocation),
                        new Confidence{strConfidence},
                    },
                    new UsageRules{
                        new RetransmissionAllowed{m_strRetransmissionAllowed},
                    },
                    new Method{strMethod},
                },
            }),
        }},
    }.Write();
    // clang-format on

    return objContent.GetLength() != 0;
}

PUBLIC
IMS_BOOL GeolocationPidfCreator::CreateWithoutCivic(IN const AString& strEntityUri,
        OUT ByteArray& objContent, IN IMS_SINT32 nConfidence /*= 0*/) const
{
    ILocationProperties* piLocation = GetLocationProperties(ILocationInfo::LOCATION_POSITION);
    if (piLocation == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!IsPositionAvailable(piLocation))
    {
        return IMS_FALSE;
    }

    AString strConfidence;
    if (nConfidence > 0)
    {
        strConfidence.Sprintf("%d", nConfidence);
    }
    else
    {
        strConfidence = piLocation->GetConfidence();
    }

    AString strMethod =
            IsFeatureSet(FEATURE_NO_METHOD) ? AString::ConstNull() : piLocation->GetMethod();

    // clang-format off
    objContent = PidfLoXml{
        new Presence{Presence::Namespace::ALL, strEntityUri, {
            CreateLocationElement(piLocation->GetCurrentTime(), {
                new Geopriv{
                    new LocationInfo{
                        CreateShapeElement(*piLocation),
                        new Confidence{strConfidence},
                    },
                    new UsageRules{
                        new RetransmissionAllowed{m_strRetransmissionAllowed},
                    },
                    new Method{strMethod},
                },
            }),
        }},
    }.Write();
    // clang-format on

    return objContent.GetLength() != 0;
}

PRIVATE
ILocationProperties* GeolocationPidfCreator::GetLocationProperties(IN IMS_SINT32 nType) const
{
    ILocationInfo* piLocationInfo =
            PhoneInfoService::GetPhoneInfoService()->GetLocationInfo(GetSlotId());
    ILocationProperties* piLocationProperties = piLocationInfo->GetLocationProperties(nType);

    if (piLocationInfo == IMS_NULL || piLocationProperties == IMS_NULL)
    {
        IMS_TRACE_E(0, "Location is not available", 0, 0, 0);
        return IMS_NULL;
    }

    if (piLocationProperties->GetCurrentTime().GetLength() <= 0)
    {
        IMS_TRACE_E(0, "Timestamp is not available", 0, 0, 0);
        return IMS_NULL;
    }

    return piLocationProperties;
}

PRIVATE
AString GeolocationPidfCreator::GetTupleId() const
{
    if (m_strTupleId.GetLength() > 0)
    {
        return m_strTupleId;
    }

    return DEFAULT_TUPLE_ID;
}

PRIVATE
AString GeolocationPidfCreator::GetDeviceId() const
{
    if (m_strDeviceId.GetLength() > 0)
    {
        return m_strDeviceId;
    }

    AString strDeviceId = "urn:uuid:";
    strDeviceId.Append(ImsUuid::GetUuid(ImsUuid::VERSION_1));
    return strDeviceId;
}

PRIVATE
AString GeolocationPidfCreator::GetDeviceName() const
{
    AString strDeviceName;
    PhoneInfoService::GetPhoneInfoService()->GetDeviceInfo()->GetDeviceName(strDeviceName);
    if (strDeviceName.GetLength() > 0)
    {
        return strDeviceName;
    }

    return DEFAULT_DEVICE_NAME;
}

PRIVATE
AString GeolocationPidfCreator::CreateEntityUri() const
{
    const ISubscriberConfig* piSubsConfig =
            Engine::GetConfiguration()->GetSubscriberConfig(GetSlotId());

    if (piSubsConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "ISubscriberConfig is null", 0, 0, 0);
        return AString::ConstNull();
    }

    AString strUri;
    strUri.Sprintf("pres:%s", piSubsConfig->GetPrivateUserId().GetStr());

    return strUri;
}

PRIVATE
Element* GeolocationPidfCreator::CreateLocationElement(
        IN const AString& strTimestamp, IN std::initializer_list<Element*> lstChildren) const
{
    Element* pElement;

    if (IsFeatureSet(FEATURE_FORMAT_TUPLE))
    {
        // clang-format off
        pElement = new Tuple{GetTupleId(), {
            new Status{
                lstChildren,
            },
        }};
        // clang-format on
        pElement->Append(new Timestamp(strTimestamp));
    }
    else
    {
        pElement = new Device{GetDeviceName(), lstChildren};
        pElement->Append(new DeviceId(GetDeviceId()));
        pElement->Append(new Timestamp(strTimestamp));
    }

    return pElement;
}

PRIVATE
Element* GeolocationPidfCreator::CreateShapeElement(IN const ILocationProperties& objLocation) const
{
    if (objLocation.GetShape().EqualsIgnoreCase("Circle"))
    {
        return new Circle(
                objLocation.GetLatitude(), objLocation.GetLongitude(), objLocation.GetRadius());
    }
    else if (objLocation.GetShape().EqualsIgnoreCase("Ellipsoid"))
    {
        return new Ellipsoid(objLocation.GetLatitude(), objLocation.GetLongitude(),
                objLocation.GetAltitude(), objLocation.GetRadius(),
                objLocation.GetVerticalAccuracy());
    }
    return Element::s_pEmptyElement;
}

PRIVATE
IMS_BOOL GeolocationPidfCreator::IsPositionAvailable(IN const ILocationProperties* piLocation) const
{
    const AString& strLatitude = piLocation->GetLatitude();
    const AString& strLongitude = piLocation->GetLongitude();

    if ((strLatitude.GetLength() == 0 || strLongitude.GetLength() == 0) ||
            (strLatitude.Equals("0.0") && strLongitude.Equals("0.0")))
    {
        IMS_TRACE_I("Position is not available", 0, 0, 0);
        return IMS_FALSE;
    }
    return IMS_TRUE;
}
