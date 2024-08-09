// Copyright 2024 Google LLC

#include "ServiceTrace.h"

#include "SdpGenerator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC SdpGenerator::SdpGenerator(IN const MEDIA_CONTENT_TYPE eType)
{
    IMS_TRACE_I("+SdpGenerator() media type[%d]", eType, 0, 0);
    m_eType = eType;
}

PUBLIC VIRTUAL SdpGenerator::~SdpGenerator()
{
    IMS_TRACE_I("~SdpGenerator()", 0, 0, 0);
}
