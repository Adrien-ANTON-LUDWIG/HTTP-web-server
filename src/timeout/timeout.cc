#include "timeout.hh"

#include "events/senders/sendresponse.hh"
namespace http
{
    Timeout::Timeout(EventWatcher *ew, unsigned timeout,
                     void (*callback)(struct ev_loop *, ev_timer *, int))
    {
        et_ = std::make_shared<ev_timer>();
        shared_ew_ = ew;
        ev_timer_init(et_.get(), callback, timeout, 0);
        et_->data = this;
        event_register.get_loop().register_timer_watcher(et_.get());
    }

    Timeout::Timeout(EventWatcher *ew, unsigned timeout,
                     unsigned int throughput_val,
                     void (*callback)(struct ev_loop *, ev_timer *, int))
    {
        et_ = std::make_shared<ev_timer>();
        throughput_val_ = throughput_val;
        shared_ew_ = ew;
        ev_timer_init(et_.get(), callback, timeout, timeout);
        et_->data = this;
        event_register.get_loop().register_timer_watcher(et_.get());
    }

    void Timeout::keep_alive_cb(struct ev_loop *, ev_timer *et, int)
    {
        auto timeout = static_cast<Timeout *>(et->data);
        auto response = Response(STATUS_CODE::REQUEST_TIMEOUT, "Keep-Alive");
        // std::cout << timeout->shared_ew_ << std::endl;
        // std::cout << timeout->shared_ew_->get_connection() << std::endl;
        // auto connection = timeout->shared_ew_->get_connection();
        // connection->keep_alive = false;
        // event_register.register_event<SendResponseEW>(connection, response);
        timeout->shared_ew_->get_connection()->keep_alive = false;
        event_register.register_event<SendResponseEW>(
            timeout->shared_ew_->get_connection(), response);
        event_register.unregister_ew(timeout->shared_ew_);
        event_register.get_loop().unregister_timer_watcher(et);
    }

    void Timeout::transaction_cb(struct ev_loop *, ev_timer *et, int)
    {
        auto timeout = static_cast<Timeout *>(et->data);
        auto response = Response(STATUS_CODE::REQUEST_TIMEOUT, "Transaction");

        timeout->shared_ew_->get_connection()->keep_alive = false;
        event_register.register_event<SendResponseEW>(
            timeout->shared_ew_->get_connection(), response);
        event_register.unregister_ew(timeout->shared_ew_);
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
            timeout->shared_ew_->get_connection()->keep_alive = false;
            event_register.register_event<SendResponseEW>(
                timeout->shared_ew_->get_connection(),
                Response(STATUS_CODE::REQUEST_TIMEOUT, "Throughput"));
            event_register.unregister_ew(timeout->shared_ew_);
            event_register.get_loop().unregister_timer_watcher(et);
        }
    }

    void Timeout::proxy_transaction_cb(struct ev_loop *, ev_timer *et, int)
    {
        auto timeout = static_cast<Timeout *>(et->data);

        timeout->shared_ew_->get_connection()->keep_alive = false;
        event_register.register_event<SendResponseEW>(
            timeout->shared_ew_->get_connection(),
            Response(STATUS_CODE::GATEWAY_TIMEOUT));
        event_register.unregister_ew(timeout->shared_ew_);
        event_register.get_loop().unregister_timer_watcher(et);
    }

    std::shared_ptr<ev_timer> Timeout::get_et()
    {
        return et_;
    }

    void Timeout::received_bytes(const unsigned int &bytes)
    {
        received_bytes_ += bytes;
    }
} // namespace http