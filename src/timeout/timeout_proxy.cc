#include "timeout_proxy.hh"

#include "events/senders/sendresponse.hh"

namespace http
{
    TimeoutProxy::TimeoutProxy(EventWatcher *ew, unsigned timeout)
    {
        et_ = std::make_shared<ev_timer>();
        et_->data = &(ew->get_connection()->timeout_proxy);
        shared_ew_ = ew;
        ev_timer_init(et_.get(), callback, timeout, 0);
        event_register.get_loop().register_timer_watcher(et_.get());
    }

    void TimeoutProxy::callback(struct ev_loop *, ev_timer *et, int)
    {
        auto timeout = *static_cast<std::shared_ptr<TimeoutProxy> *>(et->data);

        timeout->shared_ew_->get_connection()->keep_alive = false;
        event_register.register_event<SendResponseEW>(
            timeout->shared_ew_->get_connection(),
            Response(STATUS_CODE::GATEWAY_TIMEOUT));
        event_register.unregister_ew(timeout->shared_ew_);
        event_register.get_loop().unregister_timer_watcher(et);
    }

    std::shared_ptr<ev_timer> TimeoutProxy::get_et()
    {
        return et_;
    }

    void TimeoutProxy::set_ew(EventWatcher *ew)
    {
        shared_ew_ = ew;
    }
} // namespace http
