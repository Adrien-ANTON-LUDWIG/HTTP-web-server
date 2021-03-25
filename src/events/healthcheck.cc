#include "healthcheck.hh"

namespace http
{
    HealthCheckEW::HealthCheckEW(const Backend &backend)
        : backend_(backend)
    {
        ev_timer_init(&et_, timeout_cb, 12., 0.);
        event_register.get_loop().register_timer_watcher(&et_);
    }

    HealthCheckEW::~HealthCheckEW()
    {
        return;
    }

    // static void callback(EV_P_ ev_timer *w, int revents)
    void HealthCheckEW::check(struct ev_loop *, ev_timer *, int revents)
    {}
} // namespace http