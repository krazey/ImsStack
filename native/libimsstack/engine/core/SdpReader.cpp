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
#include "ServiceMemory.h"

#include "SdpParser.h"

#include "SdpReader.h"
#include "media/MediaDescriptionReader.h"

PUBLIC
SdpReader::SdpReader(IN const ByteArray& objSdp) :
        m_pSessionDescriptor(IMS_NULL)
{
    Init(objSdp);
}

PUBLIC VIRTUAL SdpReader::~SdpReader()
{
    if (m_pSessionDescriptor != IMS_NULL)
    {
        delete m_pSessionDescriptor;
    }

    for (IMS_UINT32 i = 0; i < m_objMediaDescriptors.GetSize(); ++i)
    {
        MediaDescriptionReader* pMdReader =
                static_cast<MediaDescriptionReader*>(m_objMediaDescriptors.GetAt(i));
        if (pMdReader != IMS_NULL)
        {
            delete pMdReader;
        }
    }

    m_objMediaDescriptors.Clear();
}

PRIVATE
void SdpReader::Init(IN const ByteArray& objSdp)
{
    SdpParser objParser;

    if (objParser.Decode(objSdp.ToString()))
    {
        m_pSessionDescriptor = new SessionDescriptionReader(objParser.GetSessionDescription());

        const ImsList<SdpMediaDescription>& objSmds = objParser.GetMediaDescriptions();

        for (IMS_UINT32 i = 0; i < objSmds.GetSize(); ++i)
        {
            m_objMediaDescriptors.Append(new MediaDescriptionReader(
                    objSmds.GetAt(i), m_pSessionDescriptor->GetRemoteAddressAsString()));
        }
    }
}
