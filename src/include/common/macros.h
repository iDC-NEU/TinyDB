/**
 * @file macros.h
 * @author sheep
 * @brief macros from bustub
 * @version 0.1
 * @date 2022-04-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef MACROS_H
#define MACROS_H

#include <cassert>
#include <stdexcept>

namespace TinyDB {

#define TINYDB_ASSERT(expr, message) assert((expr) && (message))

#define UNREACHABLE(message) throw std::logic_error(message)

// disable copying and moving

#define DISALLOW_COPY(cname)        \
    cname(const cname &) = delete;  \
    cname &operator=(const cname &) = delete;

#define DISALLOW_MOVE(cname)        \
    cname(cname &&) = delete;       \
    cname &operator=(cname &&) = delete;

#define DISALLOW_COPY_AND_MOVE(cname)   \
    DISALLOW_COPY(cname);               \
    DISALLOW_MOVE(cname);

}

#endif