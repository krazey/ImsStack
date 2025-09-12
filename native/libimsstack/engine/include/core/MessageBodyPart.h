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
#ifndef MESSAGE_BODY_PART_H_
#define MESSAGE_BODY_PART_H_

#include "IMessageBodyPart.h"
#include "ISipMessageBodyPart.h"

class IMessage;

/**
 * @brief A MessageBodyPart can contain different kinds of content, for example text,
 *        an image or an audio clip.
 */
class MessageBodyPart : public IMessageBodyPart
{
public:
    MessageBodyPart(IN IMessage* piMessage, IN ISipMessageBodyPart* piBodyPart);
    ~MessageBodyPart() override = default;

    MessageBodyPart(IN const MessageBodyPart&) = delete;
    MessageBodyPart& operator=(IN const MessageBodyPart&) = delete;

public:
    inline ISipMessageBodyPart* GetBodyPart() const { return m_piBodyPart; }
    inline void SetBodyPart(IN ISipMessageBodyPart* piBodyPart) { m_piBodyPart = piBodyPart; }

private:
    // IMessageBodyPart interface
    inline const ByteArray& GetContent() const override { return m_piBodyPart->GetContent(); }
    AString GetHeader(IN const AString& strName) const override;
    IMS_RESULT SetContent(IN const ByteArray& objContent) override;
    IMS_RESULT SetHeader(IN const AString& strName, IN const AString& strValue) override;

    static IMS_SINT32 GetHeaderType(IN const AString& strName);

private:
    IMessage* m_piMessage;
    ISipMessageBodyPart* m_piBodyPart;
};

#endif
