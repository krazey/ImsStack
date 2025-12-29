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

#ifndef MTC_EXTENSION_SET_H_
#define MTC_EXTENSION_SET_H_

#include "AString.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "call/message/IMtcMessageHandler.h"

class IMtcCallContext;
class IMtcExtension;

/*
 * Holds a set of extensions supported by the local.
 * It provides the methods to inspect if they are available in the call.
 */
class MtcExtensionSet final : public IMtcMessageFormatter, public IMtcMessageHandler
{
public:
    static const AString OPTION_TAG_EARLY_DIALOG_TERMINATED;
    static const AString OPTION_TAG_FROM_CHANGE;
    static const AString OPTION_TAG_HISTORY_INFO;
    static const AString OPTION_TAG_PRECONDITION;
    static const AString OPTION_TAG_REPLACES;
    static const AString OPTION_TAG_RPR;
    static const AString OPTION_TAG_SESSION_TIMER;
    static const AString OPTION_TAG_TARGET_DIALOG;

    explicit MtcExtensionSet(
            IN IMtcCallContext& objContext, IN const ImsList<IMtcExtension*>& lstExtensions);
    MtcExtensionSet(IN const MtcExtensionSet& objRhs);
    virtual ~MtcExtensionSet() override;
    MtcExtensionSet& operator=(IN const MtcExtensionSet& objRhs);

    /**
     * Checks if the given extension is available on both local and remote.
     *
     * @param strOptionTag Option tag of the extension to inspect the availability.
     * @return True if the given extension is available.
     */
    IMS_BOOL IsAvailableOnBoth(IN const AString& strOptionTag) const;

    /**
     * Checks if the given extension is available on the local.
     *
     * @param strOptionTag Option tag of the extension to inspect the availability.
     * @return True if the given extension is available.
     */
    IMS_BOOL IsAvailableOnLocal(IN const AString& strOptionTag) const;

    /**
     * Checks if the given extension is required on the remote.
     *
     * @param strOptionTag Option tag of the extension to inspect the availability.
     * @return True if the given extension is required.
     */
    IMS_BOOL IsRequiredOnRemote(IN const AString& strOptionTag) const;

    /**
     * Checks if the all required extensions in the message are available on the local.
     *
     * @param objMessage Message containing required extensions.
     * @param strNotSupportedExtension Option tag of the extension that's not supported.
     * @return True if all extensions are available.
     */
    IMS_BOOL IsSupportRequiredExtensions(
            IN const IMessage& objMessage, OUT AString& strNotSupportedExtension) const;

    void FormatRequest(IN RequestType eType, IN_OUT IMessage& objRequest) override;
    void FormatResponse(IN ResponseType eType, IN_OUT IMessage& objResponse) override;
    void HandleRequest(IN RequestType eType, IN const IMessage& objRequest) override;
    void HandleResponse(IN ResponseType eType, IN const IMessage& objResponse) override;

private:
    class CaseInsensitiveExtensionMap
    {
    public:
        inline void Add(const AString& strKey, IMtcExtension* pExtension)
        {
            m_objMap.Add(strKey.MakeLower(), pExtension);
        }

        inline const AString& GetKeyAt(IMS_UINT32 nIndex) const
        {
            return m_objMap.GetKeyAt(nIndex);
        }

        inline IMS_SLONG GetIndexOfKey(const AString& strKey) const
        {
            return m_objMap.GetIndexOfKey(strKey.MakeLower());
        }

        inline IMtcExtension* GetValueAt(IMS_UINT32 nIndex) const
        {
            return m_objMap.GetValueAt(nIndex);
        }

        inline IMS_UINT32 GetSize() const { return m_objMap.GetSize(); }

        inline void Clear() { m_objMap.Clear(); }

    private:
        ImsMap<AString, IMtcExtension*> m_objMap;
    };

    void CopyFrom(IN const MtcExtensionSet& objRhs);
    void Clear();

    IMtcCallContext& m_objContext;
    CaseInsensitiveExtensionMap m_objExtensions;
};

#endif
