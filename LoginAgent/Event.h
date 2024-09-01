#pragma once

enum EVENT_ID
{
    EVENT_ID_LISTEN_SUCCESS,
    EVENT_ID_UPDATE_USERCOUNT,
};

class IEvent
{
public:
    virtual HRESULT NotifyEvent(EVENT_ID eid, WPARAM wParam, LPARAM lParam) = 0;
};