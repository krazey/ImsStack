/*
 * Copyright (C) 2026 The Android Open Source Project
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
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "SipMessageFraming.h"

namespace android
{
namespace
{

const std::string kAck =
        "ACK sip:user@192.0.2.1 SIP/2.0\r\n"
        "Via: SIP/2.0/TCP 192.0.2.2;branch=z9hG4bK-ack\r\n"
        "Max-Forwards: 70\r\n"
        "From: <sip:caller@example.org>;tag=remote\r\n"
        "To: <sip:user@example.org>;tag=local\r\n"
        "Call-ID: framing-test@example.org\r\n"
        "CSeq: 1 ACK\r\n"
        "Content-Length: 0\r\n\r\n";

bool Append(SipMessageFraming& framing, const std::string& bytes)
{
    return framing.AppendPacket(reinterpret_cast<const IMS_BYTE*>(bytes.data()),
            static_cast<IMS_SINT32>(bytes.size()));
}

std::string ReadMessage(const SipMessageFraming& framing)
{
    ByteArray message;
    if (!framing.GetCompleteMessage(message))
    {
        return {};
    }
    return std::string(reinterpret_cast<const char*>(message.GetData()), message.GetLength());
}

bool Receive(SipMessageFraming& framing, const std::string& bytes,
        std::vector<std::string>& messages)
{
    if (!Append(framing, bytes))
    {
        return false;
    }

    // Mirror SipStreamSocket: IgnoreCrlf is called once per socket read, not
    // repeatedly until it returns false. Then drain all complete messages.
    framing.IgnoreCrlf();
    while (framing.CheckCompleteMessage())
    {
        std::string message = ReadMessage(framing);
        if (message.empty())
        {
            return false;
        }
        messages.push_back(message);
        framing.UpdateState();
    }
    return true;
}

}  // namespace

TEST(SipMessageFramingTest, EmptyBufferHasNoCrlf)
{
    SipMessageFraming framing;
    EXPECT_FALSE(framing.IgnoreCrlf());
    EXPECT_FALSE(framing.CheckCompleteMessage());
}

TEST(SipMessageFramingTest, ConsumesAllCompleteLeadingPairs)
{
    SipMessageFraming framing;
    ASSERT_TRUE(Append(framing, "\r\n\r\n\r\n"));
    EXPECT_TRUE(framing.IgnoreCrlf());
    EXPECT_TRUE(framing.IsEmpty());
    EXPECT_FALSE(framing.IgnoreCrlf());
    EXPECT_FALSE(framing.CheckCompleteMessage());
}

TEST(SipMessageFramingTest, KeepsIncompleteLeadingPair)
{
    SipMessageFraming framing;
    ASSERT_TRUE(Append(framing, "\r\n\r"));
    EXPECT_TRUE(framing.IgnoreCrlf());
    EXPECT_FALSE(framing.IsEmpty());
    EXPECT_FALSE(framing.CheckCompleteMessage());
    ASSERT_TRUE(Append(framing, "\n"));
    EXPECT_TRUE(framing.IgnoreCrlf());
    EXPECT_TRUE(framing.IsEmpty());
}

TEST(SipMessageFramingTest, AckAfterLeadingPairsIsByteExact)
{
    SipMessageFraming framing;
    std::vector<std::string> messages;
    ASSERT_TRUE(Receive(framing, "\r\n\r\n" + kAck, messages));
    ASSERT_EQ(1u, messages.size());
    EXPECT_EQ(kAck, messages[0]);
    EXPECT_TRUE(framing.IsEmpty());
}

TEST(SipMessageFramingTest, LeadingPairsDoNotStrandFragmentedAck)
{
    SipMessageFraming framing;
    std::vector<std::string> messages;
    const size_t split = kAck.find("Content-Length") + 7;

    ASSERT_TRUE(Receive(framing, "\r\n\r\n" + kAck.substr(0, split), messages));
    EXPECT_TRUE(messages.empty());
    ASSERT_TRUE(Receive(framing, kAck.substr(split), messages));
    ASSERT_EQ(1u, messages.size());
    EXPECT_EQ(kAck, messages[0]);
    EXPECT_TRUE(framing.IsEmpty());
}

TEST(SipMessageFramingTest, AckAtEveryTwoReadBoundary)
{
    std::string compactAck = kAck;
    compactAck.replace(compactAck.find("Content-Length"), 14, "l");
    for (const std::string& ack : {kAck, compactAck})
    {
        for (const std::string& prefix : {std::string(), std::string("\r\n"),
                     std::string("\r\n\r\n"), std::string("\r\n\r\n\r\n")})
        {
            const std::string wire = prefix + ack;
            for (size_t split = 1; split < wire.size(); ++split)
            {
                SCOPED_TRACE("prefix=" + std::to_string(prefix.size()) +
                        " split=" + std::to_string(split));
                SipMessageFraming framing;
                std::vector<std::string> messages;
                ASSERT_TRUE(Receive(framing, wire.substr(0, split), messages));
                EXPECT_TRUE(messages.empty());
                ASSERT_TRUE(Receive(framing, wire.substr(split), messages));
                ASSERT_EQ(1u, messages.size());
                EXPECT_EQ(ack, messages[0]);
                EXPECT_TRUE(framing.IsEmpty());
            }
        }
    }
}

TEST(SipMessageFramingTest, AckOneBytePerRead)
{
    SipMessageFraming framing;
    std::vector<std::string> messages;
    const std::string wire = "\r\n\r\n" + kAck;
    for (size_t i = 0; i < wire.size(); ++i)
    {
        ASSERT_TRUE(Receive(framing, wire.substr(i, 1), messages));
        if (i + 1 < wire.size())
        {
            EXPECT_TRUE(messages.empty());
        }
    }
    ASSERT_EQ(1u, messages.size());
    EXPECT_EQ(kAck, messages[0]);
    EXPECT_TRUE(framing.IsEmpty());
}

TEST(SipMessageFramingTest, CoalescedMessagesAtEveryTwoReadBoundary)
{
    const std::string wire = "\r\n\r\n" + kAck + "\r\n\r\n" + kAck;
    for (size_t split = 1; split < wire.size(); ++split)
    {
        SCOPED_TRACE("split=" + std::to_string(split));
        SipMessageFraming framing;
        std::vector<std::string> messages;
        ASSERT_TRUE(Receive(framing, wire.substr(0, split), messages));
        ASSERT_TRUE(Receive(framing, wire.substr(split), messages));
        ASSERT_EQ(2u, messages.size());
        EXPECT_EQ(kAck, messages[0]);
        EXPECT_EQ(kAck, messages[1]);
        EXPECT_TRUE(framing.IsEmpty());
    }
}

TEST(SipMessageFramingTest, BodyBytesAreNotKeepalive)
{
    // A framing fixture: body bytes deliberately contain CRLF and an embedded
    // NUL. The framer must preserve Content-Length bytes without interpreting them.
    const std::string body("\r\n\0\r\nabc", 8);
    std::string message = kAck;
    message.replace(message.find("Content-Length: 0"), 17, "Content-Length: 8");
    message += body;
    const std::string wire = "\r\n\r\n" + message;
    for (size_t split = 1; split < wire.size(); ++split)
    {
        SCOPED_TRACE("split=" + std::to_string(split));
        SipMessageFraming framing;
        std::vector<std::string> messages;
        ASSERT_TRUE(Receive(framing, wire.substr(0, split), messages));
        EXPECT_TRUE(messages.empty());
        ASSERT_TRUE(Receive(framing, wire.substr(split), messages));
        ASSERT_EQ(1u, messages.size());
        EXPECT_EQ(message, messages[0]);
        EXPECT_TRUE(framing.IsEmpty());
    }
}

TEST(SipMessageFramingTest, DoesNotShiftAlreadyParsedHeader)
{
    SipMessageFraming framing;
    const std::string wire = "\r\n\r\n" + kAck;
    const size_t split = wire.find("Content-Length") + 7;

    // Exercise the defensive guard: a caller has already started parsing before
    // asking to discard leading CRLF. The saved offset must stay valid.
    ASSERT_TRUE(Append(framing, wire.substr(0, split)));
    EXPECT_FALSE(framing.CheckCompleteMessage());
    EXPECT_FALSE(framing.IgnoreCrlf());
    ASSERT_TRUE(Append(framing, wire.substr(split)));
    ASSERT_TRUE(framing.CheckCompleteMessage());
    EXPECT_EQ(wire, ReadMessage(framing));
    framing.UpdateState();
    EXPECT_TRUE(framing.IsEmpty());
}

}  // namespace android
