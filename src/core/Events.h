#pragma once

struct FrameBeginEventListener;
HAPI void FrameBeginListener_add(FrameBeginEventListener* in);
HAPI void FrameBeginListener_remove(FrameBeginEventListener* in);

struct HAPI FrameBeginEventListener
{
    FrameBeginEventListener() { FrameBeginListener_add(this); }
    FrameBeginEventListener(const FrameBeginEventListener&) { FrameBeginListener_add(this); }
    virtual ~FrameBeginEventListener() { FrameBeginListener_remove(this); }
    FrameBeginEventListener& operator=(const FrameBeginEventListener&) = default;

    virtual void on_event() = 0;
};
