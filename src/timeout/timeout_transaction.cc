#include "timeout_transaction.hh"

#include "events/senders/sendresponse.hh"

namespace http
{
    TimeoutTransaction::TimeoutTransaction(EventWatcher *ew, float timeout)
    {
        et_ = std::make_shared<ev_timer>();
        et_->data = &(ew->get_connection()->timeout_transaction);
        shared_ew_ = ew;
        ev_timer_init(et_.get(), callback, timeout, 0);
        event_register.get_loop().register_timer_watcher(et_.get());
    }

    void TimeoutTransaction::callback(struct ev_loop *, ev_timer *et, int)
    {
        auto timeout =
            *static_cast<std::shared_ptr<TimeoutTransaction> *>(et->data);
        auto response = Response(STATUS_CODE::REQUEST_TIMEOUT, "Transaction");

        shared_connection c = timeout->shared_ew_->get_connection();
        c->keep_alive = false;
        event_register.register_event<SendResponseEW>(c, response);
        if (c->timeout_throughput != nullptr)
            event_register.get_loop().unregister_timer_watcher(
                c->timeout_throughput->get_et().get());
        event_register.unregister_ew(timeout->shared_ew_);
        event_register.get_loop().unregister_timer_watcher(et);
    }

    void TimeoutTransaction::set_ew(EventWatcher *ew)
    {
        shared_ew_ = ew;
    }
    std::shared_ptr<ev_timer> TimeoutTransaction::get_et()
    {
        return et_;
    }
} // namespace http