// Copyright 2024 Google LLC

#include "ServiceTrace.h"

#include "SdpNegotiator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC SdpNegotiator::SdpNegotiator(IN const MEDIA_CONTENT_TYPE eType)
{
    IMS_TRACE_I("+SdpNegotiator() media type[%d]", eType, 0, 0);
    m_eType = eType;
}

PUBLIC VIRTUAL SdpNegotiator::~SdpNegotiator()
{
    IMS_TRACE_I("~SdpNegotiator()", 0, 0, 0);
}
