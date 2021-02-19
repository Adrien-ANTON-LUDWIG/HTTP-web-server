#include "events/register.hh"

#include "events/event-loop.hh"

namespace http
{
    EventWatcherRegistry event_register;

    std::optional<std::shared_ptr<EventWatcher>>
    EventWatcherRegistry::at(EventWatcher *ew)
    {
        for (auto x : event_register.events_)
        {
            if (x.first == ew)
                return std::optional<std::shared_ptr<EventWatcher>>(x.second);
        }
        return std::nullopt;
    }

} // namespace http
