/*
 * Copyright (c) 2018 Ambroz Bizjak
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef AIPSTACK_EVENT_LOOP_H
#define AIPSTACK_EVENT_LOOP_H

#include <cstdint>
#include <mutex>

#include <aipstack/misc/NonCopyable.h>
#include <aipstack/misc/OneOf.h>
#include <aipstack/misc/Use.h>
#include <aipstack/misc/Function.h>
#include <aipstack/structure/LinkModel.h>
#include <aipstack/structure/StructureRaiiWrapper.h>
#include <aipstack/structure/LinkedList.h>
#include <aipstack/structure/minimum/LinkedHeap.h>
#include <aipstack/event_loop/EventLoopCommon.h>

#if defined(__linux__)
#include <aipstack/event_loop/platform_specific/EventProviderLinux.h>
#else
#error "Unsupported OS"
#endif

namespace AIpStack {

#ifndef IN_DOXYGEN
class EventLoopTimer;
class EventLoopAsyncSignal;
#endif

class EventLoop :
    private NonCopyable<EventLoop>,
    private EventProvider
{
    friend class EventProviderBase;
    friend class EventLoopTimer;
    friend class EventLoopAsyncSignal;
    #if AIPSTACK_EVENT_LOOP_HAS_FD
    friend class EventProviderFdBase;
    #endif

    struct TimerHeapNodeAccessor;
    struct TimerCompare;

    using TimerLinkModel = PointerLinkModel<EventLoopTimer>;
    using TimerHeap = LinkedHeap<TimerHeapNodeAccessor, TimerCompare, TimerLinkModel>;
    using TimerHeapNode = LinkedHeapNode<TimerLinkModel>;

    static int const TimerStateOrderBits = 2;
    static std::uint8_t const TimerStateOrderMask = (1 << TimerStateOrderBits) - 1;

    enum class TimerState : std::uint8_t {
        Idle       = 0,
        Dispatch   = 1,
        TempUnset  = 2,
        TempSet    = 2 | (1 << TimerStateOrderBits),
        Pending    = 3
    };

    static constexpr auto OneOfHeapTimerStates ()
    {
        return OneOf(TimerState::Dispatch, TimerState::TempUnset,
                     TimerState::TempSet, TimerState::Pending);
    }

    struct AsyncSignalNode;
    struct AsyncSignalNodeAccessor;

    using AsyncSignalLinkModel = PointerLinkModel<AsyncSignalNode>;
    using AsyncSignalList = CircularLinkedList<
        AsyncSignalNodeAccessor, AsyncSignalLinkModel>;
    using AsyncSignalListNode = LinkedListNode<AsyncSignalLinkModel>;

    struct AsyncSignalNode {
        AsyncSignalListNode m_list_node;
    };

public:
    EventLoop ();

    ~EventLoop ();

    void stop ();

    void run ();

    inline static EventLoopTime getTime ()
    {
        return EventLoopClock::now();
    }

    inline EventLoopTime getEventTime () const
    {
        return m_event_time;
    }

private:
    void prepare_timers_for_dispatch (EventLoopTime now);

    bool dispatch_timers ();

    EventLoopWaitTimeoutInfo prepare_timers_for_wait ();

    bool dispatch_async_signals ();

private:
    StructureRaiiWrapper<TimerHeap> m_timer_heap;
    bool m_stop;
    EventLoopTime m_event_time;
    EventLoopTime m_last_wait_time;
    std::mutex m_async_signal_mutex;
    AsyncSignalNode m_pending_async_list;
    AsyncSignalNode m_dispatch_async_list;
};

class EventLoopTimer :
    private NonCopyable<EventLoopTimer>
{
    friend class EventLoop;

    AIPSTACK_USE_TYPES(EventLoop, (TimerHeapNode, TimerState))

public:
    EventLoopTimer (EventLoop &loop);

    ~EventLoopTimer ();

    inline bool isSet () const
    {
        return m_state != OneOf(TimerState::Idle, TimerState::TempUnset);
    }

    inline EventLoopTime getSetTime () const
    {
        return m_time;
    }

    void unset ();

    void setAt (EventLoopTime time);

    void setAfter (EventLoopDuration duration);

protected:
    virtual void handleTimerExpired () = 0;

private:
    TimerHeapNode m_heap_node;
    EventLoop &m_loop;
    EventLoopTime m_time;
    TimerState m_state;
};

class EventLoopAsyncSignal :
    private NonCopyable<EventLoopAsyncSignal>,
    private EventLoop::AsyncSignalNode
{
    friend class EventLoop;

    AIPSTACK_USE_TYPES(EventLoop, (AsyncSignalList))

public:
    using SignalEventHandler = Function<void()>;

    EventLoopAsyncSignal (EventLoop &loop, SignalEventHandler handler);

    ~EventLoopAsyncSignal ();

    void signal ();

    void reset ();

private:
    EventLoop &m_loop;
    SignalEventHandler m_handler;
};

#if AIPSTACK_EVENT_LOOP_HAS_FD || defined(IN_DOXYGEN)

class EventLoopFdWatcher :
    private NonCopyable<EventLoopFdWatcher>,
    private EventProviderFd
{
    friend class EventProviderFdBase;

public:
    using FdEventHandler = Function<void(EventLoopFdEvents events)>;

    EventLoopFdWatcher (EventLoop &loop, FdEventHandler handler);

    ~EventLoopFdWatcher ();

    inline bool hasFd () const
    {
        return m_watched_fd >= 0;
    }

    inline int getFd () const
    {
        return m_watched_fd;
    }

    inline EventLoopFdEvents getEvents () const
    {
        return m_events;
    }

    void initFd (int fd, EventLoopFdEvents events = {});

    void updateEvents (EventLoopFdEvents events);

    void reset ();

private:
    EventLoop &m_loop;
    FdEventHandler m_handler;
    int m_watched_fd;
    EventLoopFdEvents m_events;
};

#endif

}

#endif
