#include "events/event-loop.hh"

namespace http
{
    EventLoop::EventLoop()
    {
        loop = EV_DEFAULT;
    }

    EventLoop::EventLoop(struct ev_loop *l)
    {
        loop = l;
    }

    EventLoop::~EventLoop()
    {
        /*FIXME*/
    }

    void EventLoop::register_watcher(EventWatcher *ew)
    {
        ev_io_start(loop, &ew->watcher_get());
    }

    void EventLoop::unregister_watcher(EventWatcher *ew)
    {
        ev_io_stop(loop, &ew->watcher_get());
    }

    void EventLoop::register_sigint_watcher(ev_signal *es) const
    {
        ev_signal_start(loop, es);
    }

    void EventLoop::register_timer_watcher(ev_timer *et) const
    {
        ev_timer_start(loop, et);
    }

    void EventLoop::unregister_timer_watcher(ev_timer *et) const
    {
        ev_timer_stop(loop, et);
    }

    void EventLoop::operator()() const
    {
        ev_run(loop);
    }
} // namespace http
