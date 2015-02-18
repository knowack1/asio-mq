/*
 * basic_mqueue_service.hpp
 *
 *  Created on: Apr 24, 2014
 *      Author: knowacki
 */

#ifndef BASIC_MQUEUE_SERVICE_HPP_
#define BASIC_MQUEUE_SERVICE_HPP_

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <future>

namespace asio
{

template <typename MqueueImplementation>
class basic_mqueue_service
    : public boost::asio::io_service::service
{
public:
    static boost::asio::io_service::id id;

    explicit basic_mqueue_service(boost::asio::io_service& io_service)
        : boost::asio::io_service::service(io_service),
          async_work_(new boost::asio::io_service::work(async_io_service_)),
          async_thread_(boost::bind(&boost::asio::io_service::run, &async_io_service_))
    {
    }

    ~basic_mqueue_service()
    {
        async_work_.reset();
        async_io_service_.stop();
//        async_thread_.join();
    }

    typedef boost::shared_ptr<MqueueImplementation> implementation_type;

    void construct(implementation_type& impl)
    {
        impl.reset(new MqueueImplementation());
    }

#if defined(BOOST_ASIO_HAS_MOVE)
    void move_construct(implementation_type& impl,
                        implementation_type& other_impl)
    {
        impl = std::move(other_impl);
    }
#endif // defined(BOOST_ASIO_HAS_MOVE)


    void destroy(implementation_type& impl)
    {
        if (impl)
        {
            impl->destroy();
        }
        impl.reset();
    }

    template <typename Handler>
    void async_send(implementation_type& impl, const void* buffer, int  buffer_size, unsigned int priority, Handler handler)
    {
        boost::weak_ptr<MqueueImplementation> weak_impl(impl);

        auto opaque = [ = ]
        {
            implementation_type impl = weak_impl.lock();
            if (impl)
            {
                boost::system::error_code ec;
                impl->send(buffer, buffer_size, priority, ec);
                handler(ec);
            }
            else
            {
                handler(boost::asio::error::operation_aborted);
            }
        };

        this->get_io_service().post(opaque);
    }

    template <typename Handler>
    void async_receive(implementation_type& impl, void* buffer, int  buffer_size, Handler handler)
    {
        boost::weak_ptr<MqueueImplementation> weak_impl(impl);
        boost::asio::io_service::work work(this->get_io_service());

        auto opaque = [ = ]() mutable
        {
            implementation_type impl = weak_impl.lock();
            if (impl)
            {
                boost::system::error_code ec;
                size_t bytes = impl->receive(buffer, buffer_size, ec);
                work.get_io_service().post(boost::asio::detail::bind_handler(handler, ec, bytes));
            }
            else
            {
                work.get_io_service().post(boost::asio::detail::bind_handler(handler, boost::asio::error::operation_aborted, -1));
            }
        };
        boost::thread thr(opaque);
        thr.detach();
//        this->async_io_service_.post(opaque);
    }

private:
    void shutdown_service()
    {
    }

    boost::asio::io_service async_io_service_;
    boost::scoped_ptr<boost::asio::io_service::work> async_work_;
    boost::thread async_thread_;
};

template <typename MqueueImplementation>
boost::asio::io_service::id basic_mqueue_service<MqueueImplementation>::id;

}

#endif /* BASIC_MQUEUE_SERVICE_HPP_ */
