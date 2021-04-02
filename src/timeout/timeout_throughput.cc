#include "timeout_throughput.hh"

#include "events/senders/sendresponse.hh"

namespace http
{
    TimeoutThroughput::TimeoutThroughput(EventWatcher *ew, float timeout,
                                         unsigned int throughput_val)
    {
        et_ = std::make_shared<ev_timer>();
        et_->data = &(ew->get_connection()->timeout_throughput);
        throughput_val_ = throughput_val;
        shared_ew_ = ew;
        ev_timer_init(et_.get(), callback, timeout, timeout);
        event_register.get_loop().register_timer_watcher(et_.get());
    }

    void TimeoutThroughput::callback(struct ev_loop *, ev_timer *et, int)
    {
        auto timeout =
            *static_cast<std::shared_ptr<TimeoutThroughput> *>(et->data);
        auto request = timeout->shared_ew_->get_request();

        if (timeout->received_bytes_ >= timeout->throughput_val_)
        {
            timeout->received_bytes_ = 0;
        }
        else
        {
            shared_connection c = timeout->shared_ew_->get_connection();
            c->keep_alive = false;
            event_register.register_event<SendResponseEW>(
                c, Response(STATUS_CODE::REQUEST_TIMEOUT, "Throughput"));
            if (c->timeout_transaction != nullptr)
                event_register.get_loop().unregister_timer_watcher(
                    c->timeout_transaction->get_et().get());
            event_register.unregister_ew(timeout->shared_ew_);
            event_register.get_loop().unregister_timer_watcher(et);
        }
    }

    std::shared_ptr<ev_timer> TimeoutThroughput::get_et()
    {
        return et_;
    }

    void TimeoutThroughput::set_ew(EventWatcher *ew)
    {
        shared_ew_ = ew;
    }

    void TimeoutThroughput::received_bytes(const unsigned int &bytes)
    {
        received_bytes_ += bytes;
    }
} // namespace http