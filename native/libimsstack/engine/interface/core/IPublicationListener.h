#ifndef _INTERFACE_PUBLICATION_LISTENER_H_
#define _INTERFACE_PUBLICATION_LISTENER_H_

class IPublication;

/**
 * @brief This class provides a listener interface to notify the application of
 *        the status of requested publications.
 *
 * @see IPublication
 */
class IPublicationListener
{
public:
    /**
     * @brief Notifies the application that the publication request was successfully delivered.
     *
     * @param piPublication Pointer to IPublication
     */
    virtual void PublicationDelivered(IN IPublication* piPublication) = 0;

    /**
     * @brief Notifies the application that the publication request was not successfully delivered.
     *
     * @param piPublication Pointer to IPublication
     */
    virtual void PublicationDeliveryFailed(IN IPublication* piPublication) = 0;

    /**
     * @brief Notifies the application that the publication was terminated.
     *
     * @param piPublication Pointer to IPublication
     */
    virtual void PublicationTerminated(IN IPublication* piPublication) = 0;

    //// IMS Extensions

    /**
     * @brief Notifies the application that the publication was refreshed.
     *
     * @param piPublication Pointer to IPublication
     */
    virtual void PublicationRefreshStarted(IN IPublication* piPublication) = 0;

    /**
     * @brief Notifies the application that the publication was refresh done.
     *
     * @param piPublication Pointer to IPublication
     */
    virtual void PublicationRefreshCompleted(IN IPublication* piPublication) = 0;
};

#endif  // _INTERFACE_PUBLICATION_LISTENER_H_
