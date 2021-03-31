#include "timeout_throughput.hh"

#include "events/senders/sendresponse.hh"

namespace http
{
    TimeoutThroughput::TimeoutThroughput(EventWatcher *ew, unsigned timeout,
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
            timeout->shared_ew_->get_connection()->keep_alive = false;
            event_register.register_event<SendResponseEW>(
                timeout->shared_ew_->get_connection(),
                Response(STATUS_CODE::REQUEST_TIMEOUT, "Throughput"));
            event_register.unregister_ew(timeout->shared_ew_);
            event_register.get_loop().unregister_timer_watcher(et);
        }
    }

    std::shared_ptr<ev_timer> TimeoutThroughput::get_et()
    {
        return et_;
    }

    void TimeoutThroughput::received_bytes(const unsigned int &bytes)
    {
        received_bytes_ += bytes;
    }
} // namespace http