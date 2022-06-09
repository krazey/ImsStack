#ifndef INTERFACE_MTC_EXTENSION_H_
#define INTERFACE_MTC_EXTENSION_H_

#include "IMSTypeDef.h"
#include "call/message/IMtcMessageHandler.h"

class AString;

/**
 * This class represents an extension for mmtel call. It stores whether the extension is available
 * in the call. And adds corresponding option tag to the message to be sent.
 */
class IMtcExtension : public IMtcMessageHandler
{
public:
    virtual ~IMtcExtension() {}

    /**
     * Make a clone of this instance.
     *
     * @return Copy of this instance.
     */
    virtual IMtcExtension* Clone() const;

    /**
     * Returns whether the extension is available on the remote.
     *
     * @return True if available on the remote.
     */
    virtual IMS_BOOL IsAvailableOnRemote() const;

    /**
     * Returns whether the extension is required on the remote.
     *
     * @return True if required on the remote.
     */
    virtual IMS_BOOL IsRequiredOnRemote() const;

    /**
     * Returns the extension's option tag
     *
     * @return Option tag string.
     */
    virtual const AString& GetOptionTag() const;
};

#endif
