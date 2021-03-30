/**
 * \file vhost/dispatcher.hh
 * \brief Dispatcher declaration.
 */

#pragma once

#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class Dispatcher
     * \brief Instance in charge of dispatching requests between vhosts.
     */
    class Dispatcher
    {
        using iterator = std::vector<shared_vhost>::iterator;
        using const_iterator = std::vector<shared_vhost>::const_iterator;

    public:
        Dispatcher() = default;
        Dispatcher(const Dispatcher &) = delete;
        Dispatcher &operator=(const Dispatcher &) = delete;
        Dispatcher(Dispatcher &&) = delete;
        Dispatcher &operator=(Dispatcher &&) = delete;

        void add_vhost(const shared_vhost &vhost);

        void dispatch(const shared_connection &connection,
                      std::shared_ptr<Request> &request);

        iterator begin()
        {
            return vhosts_.begin();
        }

        iterator end()
        {
            return vhosts_.end();
        }

        const_iterator begin() const
        {
            return vhosts_.cbegin();
        }

        const_iterator end() const
        {
            return vhosts_.cend();
        }

        void set_default_vhost(const shared_vhost &default_vhost);
        shared_vhost get_default_vhost();

    private:
        /* FIXME: Add members to store the information relative to the
        ** Dispatcher.
        */
        std::vector<shared_vhost> vhosts_;
        shared_vhost default_vhost_;
    };

    /**
     * \brief Service object.
     */
    extern Dispatcher dispatcher;
} // namespace http
