#include "timeout.hh"

#include "events/senders/sendresponse.hh"
namespace http
{
    Timeout::Timeout(EventWatcher *ew, unsigned timeout,
                     void (*callback)(struct ev_loop *, ev_timer *, int))
    {
        shared_ew_ = std::shared_ptr<EventWatcher>(ew);
        ev_timer_init(&et_, callback, timeout, 0);
        et_.data = this;
        event_register.get_loop().register_timer_watcher(&et_);
    }

    Timeout::Timeout(EventWatcher *ew, unsigned timeout,
                     unsigned int throughput_val,
                     void (*callback)(struct ev_loop *, ev_timer *, int))
    {
        throughput_val_ = throughput_val;
        shared_ew_ = std::shared_ptr<EventWatcher>(ew);
        ev_timer_init(&et_, callback, timeout, timeout);
        et_.data = this;
        event_register.get_loop().register_timer_watcher(&et_);
    }

    void Timeout::keep_alive_cb(struct ev_loop *, ev_timer *et, int)
    {
        auto timeout = static_cast<Timeout *>(et->data);
        auto request = timeout->shared_ew_->get_request();

        request->headers["X-Timeout-Reason"] = "Keep-Alive";

        event_register.register_event<SendResponseEW>(
            timeout->shared_ew_->get_connection(),
            Response(*request, STATUS_CODE::REQUEST_TIMEOUT));
        event_register.unregister_ew(timeout->shared_ew_.get());
        event_register.get_loop().unregister_timer_watcher(et);
    }

    void Timeout::transaction_cb(struct ev_loop *, ev_timer *et, int)
    {
        auto timeout = static_cast<Timeout *>(et->data);
        auto request = timeout->shared_ew_->get_request();

        request->headers["X-Timeout-Reason"] = "Transaction";

        event_register.register_event<SendResponseEW>(
            timeout->shared_ew_->get_connection(),
            Response(*request, STATUS_CODE::REQUEST_TIMEOUT));
        event_register.unregister_ew(timeout->shared_ew_.get());
        event_register.get_loop().unregister_timer_watcher(et);
    }

    void Timeout::throughput_cb(struct ev_loop *, ev_timer *et, int)
    {
        auto timeout = static_cast<Timeout *>(et->data);
        auto request = timeout->shared_ew_->get_request();

        if (timeout->received_bytes_ >= timeout->throughput_val_)
        {
            timeout->received_bytes_ = 0;
        }
        else
        {
            request->headers["X-Timeout-Reason"] = "Throughput";

            event_register.register_event<SendResponseEW>(
                timeout->shared_ew_->get_connection(),
                Response(*request, STATUS_CODE::REQUEST_TIMEOUT));
            event_register.unregister_ew(timeout->shared_ew_.get());
            event_register.get_loop().unregister_timer_watcher(et);
        }
    }

    void Timeout::proxy_transaction_cb(struct ev_loop *, ev_timer *et, int)
    {
        auto timeout = static_cast<Timeout *>(et->data);
        auto request = timeout->shared_ew_->get_request();

        event_register.register_event<SendResponseEW>(
            timeout->shared_ew_->get_connection(),
            Response(*request, STATUS_CODE::GATEWAY_TIMEOUT));
        event_register.unregister_ew(timeout->shared_ew_.get());
        event_register.get_loop().unregister_timer_watcher(et);
    }

    void Timeout::received_bytes(const unsigned int &bytes)
    {
        received_bytes_ += bytes;
    }
} // namespace http