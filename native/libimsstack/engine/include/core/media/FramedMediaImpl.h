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
#ifndef FRAMED_MEDIA_IMPL_H_
#define FRAMED_MEDIA_IMPL_H_

#include "media/FramedMedia.h"
#include "media/IOnMediaListener.h"
#include "media/MediaImpl.h"

class FramedMediaProposalImpl;

class FramedMediaImpl : public MediaImpl, public IMedia, public IOnMediaListener
{
public:
    explicit FramedMediaImpl(IN FramedMedia* pFramedMedia);
    ~FramedMediaImpl() override;

    FramedMediaImpl(IN const FramedMediaImpl&) = delete;
    FramedMediaImpl& operator=(IN const FramedMediaImpl&) = delete;

private:
    // MediaImpl class
    IMS_BOOL Equals(IN const IMedia* piMedia) const override;
    inline IMedia* GetInterface() override { return this; }
    inline Media* GetMedia() const override { return m_pFramedMedia; }

    // IMedia interface
    inline IMS_SINT32 GetDirection() const override { return m_pFramedMedia->GetDirection(); }
    ImsList<IMediaDescriptor*> GetMediaDescriptors() const override;
    IMedia* GetProposal() const override;
    inline IMS_SINT32 GetState() const override { return m_pFramedMedia->GetState(); }
    inline IMS_SINT32 GetUpdateState() const override { return m_pFramedMedia->GetUpdateState(); }
    inline IMS_RESULT SetDirection(IN IMS_SINT32 nDirection) override
    {
        return m_pFramedMedia->SetDirection(nDirection);
    }
    inline IMediaDescriptor* GetMediaDescriptor() const override
    {
        return m_pFramedMedia->GetMediaDescriptor();
    }
    inline IMS_SINT32 GetType() const override { return m_pFramedMedia->GetType(); }
    inline void RemoveMediaDescriptor(IN IMS_UINT32 nPosition) override
    {
        m_pFramedMedia->RemoveMediaDescriptor(nPosition);
    }

    void OnMedia_FictitiousMediaCreated(IN Media* pMedia) override;
    void OnMedia_FictitiousMediaDestroyed(IN Media* pMedia) override;

private:
    FramedMedia* m_pFramedMedia;
    FramedMediaProposalImpl* m_pFramedMediaProposal;
};

#endif
