#include "events/register.hh"

#include "events/event-loop.hh"

namespace http
{
    EventWatcherRegistry event_register;

    std::optional<std::shared_ptr<EventWatcher>>
    EventWatcherRegistry::at(EventWatcher *ew)
    {
        if (events_.find(ew) != events_.end())
            return std::optional<std::shared_ptr<EventWatcher>>(events_.at(ew));
        return std::nullopt;
    }

} // namespace http
