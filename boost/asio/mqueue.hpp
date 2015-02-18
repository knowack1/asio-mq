/*
 * mqueue.hpp
 *
 *  Created on: Apr 24, 2014
 *      Author: karol
 */

#ifndef MQUEUE_HPP_
#define MQUEUE_HPP_

#include "asio/basic_mqueue.hpp"
#include "asio/basic_mqueue_service.hpp"
#include "asio/posix_mqueue_impl.hpp"

namespace asio
{

typedef asio::basic_mqueue_service<asio::posix_mqueue_impl> mqueue_service;
typedef asio::basic_mqueue<mqueue_service> mqueue;

}

#endif /* MQUEUE_HPP_ */
