#pragma once

enum EVENT_ID
{
    EVENT_ID_LISTEN_SUCCESS,
    EVENT_ID_USER_CONNECTED,
    EVENT_ID_USER_DISCONNECTED,
};

class IEvent
{
public:
    virtual HRESULT NotifyEvent(EVENT_ID eid, WPARAM wParam, LPARAM lParam) = 0;
};