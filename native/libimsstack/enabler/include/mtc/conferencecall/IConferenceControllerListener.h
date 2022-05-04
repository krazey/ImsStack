#ifndef INTERFACE_CONFERENCE_CONTROLLER_LISTENER_H_
#define INTERFACE_CONFERENCE_CONTROLLER_LISTENER_H_

class ConferenceController;

class IConferenceControllerListener
{
public:
    virtual void OnClosed(IN ConferenceController* pController) = 0;
};

#endif
