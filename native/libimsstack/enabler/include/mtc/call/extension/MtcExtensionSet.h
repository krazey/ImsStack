#ifndef MTC_EXTENSION_SET_H_
#define MTC_EXTENSION_SET_H_

#include "AString.h"
#include "IMSMap.h"
#include "IMSTypeDef.h"
#include "call/message/IMtcMessageHandler.h"

class IMtcExtension;

/*
 * Holds a set of extensions supported by the local.
 * It provides the methods to inspect if they are available in the call.
 */
class MtcExtensionSet final : public IMtcMessageHandler
{
public:
    static const AString OPTION_TAG_EARLY_DIALOG_TERMINATED;
    static const AString OPTION_TAG_FROM_CHANGE;
    static const AString OPTION_TAG_HISTORY_INFO;
    static const AString OPTION_TAG_PRECONDITION;
    static const AString OPTION_TAG_REPLACES;
    static const AString OPTION_TAG_RPR;
    static const AString OPTION_TAG_TARGET_DIALOG;
    static const AString OPTION_TAG_TIMER;

    explicit MtcExtensionSet(IN const IMSList<AString>& lstOptionTags);
    explicit MtcExtensionSet(IN const MtcExtensionSet& objRhs);
    virtual ~MtcExtensionSet();
    MtcExtensionSet& operator=(IN const MtcExtensionSet& objRhs);

    /**
     * Checks if the given extension is available on both local and remote.
     *
     * @param eExtensionTag Option tag of the extension to inspect the availability.
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
     * Checks if the all required extensions in the message are available on the local.
     *
     * @param pMessage Message containing required extensions.
     * @return True if all extensions are available.
     */
    IMS_BOOL IsSupportRequiredExtensions(IN const IMessage& objMessage) const;

    void FormatRequest(IN IMS_UINT32 nMethod, IN_OUT IMessage& objRequest) override;
    void FormatResponse(IN IMS_UINT32 nMethod, IN_OUT IMessage& objResponse) override;
    void HandleRequest(IN IMS_UINT32 nMethod, IN const IMessage& objRequest) override;
    void HandleResponse(IN IMS_UINT32 nMethod, IN const IMessage& objResponse) override;

private:
    void CopyFrom(IN const MtcExtensionSet& objRhs);
    void Clear();
    IMtcExtension* CreateExtension(IN const AString& strOptionTag) const;

    IMSMap<AString, IMtcExtension*> m_objExtensions;
};

#endif
