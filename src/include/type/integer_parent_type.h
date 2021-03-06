/**
 * @file integer_parent_type.h
 * @author sheep
 * @brief 
 * @version 0.1
 * @date 2022-05-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef INTEGER_PARENT_TYPE_H
#define INTEGER_PARENT_TYPE_H

#include "type/numeric_type.h"
#include "type/value.h"
#include "common/exception.h"
#include "common/logger.h"

#include <string>

namespace TinyDB {

// integer value of common sizes
class IntegerParentType: public NumericType {
public: 
    explicit IntegerParentType(TypeId type_id) : NumericType(type_id) {}
    ~IntegerParentType() override = default;

    // whether you declare it's virtual or not, it will be virtual
    // and = 0 indicate that this is pure method, which has no implementation
    // this will force the sub-class to implement those operations and not
    // fallback to base class to throw exception
    // i.e. use compiler to force the implementation, instead of runtime exception

    Value Add(const Value &lhs, const Value &rhs) const override = 0;
    Value Subtract(const Value &lhs, const Value &rhs) const override = 0;
    Value Multiply(const Value &lhs, const Value &rhs) const override = 0;
    Value Divide(const Value &lhs, const Value &rhs) const override = 0;
    Value Modulo(const Value &lhs, const Value &rhs) const override = 0;
    Value Sqrt(const Value &val) const override = 0;
    Value OperateNull(const Value &lhs, const Value &rhs) const override = 0;
    bool IsZero(const Value &val) const override = 0;

    CmpBool CompareEquals(const Value &lhs, const Value &rhs) const override = 0;
    CmpBool CompareNotEquals(const Value &lhs, const Value &rhs) const override = 0;
    CmpBool CompareLessThan(const Value &lhs, const Value &rhs) const override = 0;
    CmpBool CompareLessThanEquals(const Value &lhs, const Value &rhs) const override = 0;
    CmpBool CompareGreaterThan(const Value &lhs, const Value &rhs) const override = 0;
    CmpBool CompareGreaterThanEquals(const Value &lhs, const Value &rhs) const override = 0;

    // integer types are always inlined
    bool IsInlined(const Value &val) const override { return true; }

    // for debug purpose
    std::string ToString(const Value &val) const override = 0;

    void SerializeTo(const Value &val, char *storage) const override = 0;
    Value DeserializeFrom(const char *storage) const override = 0;

    Value Copy(const Value &val) const override = 0;

    Value CastAs(const Value &val, TypeId type_id) const override = 0;

protected:
    template <class T1, class T2>
    Value AddValue(const Value &lhs, const Value &rhs) const;

    template <class T1, class T2>
    Value SubtractValue(const Value &lhs, const Value &rhs) const;

    template <class T1, class T2>
    Value MultiplyValue(const Value &lhs, const Value &rhs) const;

    template <class T1, class T2>
    Value DivideValue(const Value &lhs, const Value &rhs) const;

    template <class T1, class T2>
    Value ModuloValue(const Value &lhs, const Value &rhs) const;
};

// template code should stay in header file
// otherwise you will fail to instantiate the functions

// TODO: check for null value. i.e. when result represent null value, throw an exception

// FIXME: is this too expensive?
template <class T1, class T2>
Value IntegerParentType::AddValue(const Value &lhs, const Value &rhs) const {
    auto x = lhs.GetAs<T1>();
    auto y = rhs.GetAs<T2>();
    auto sum1 = static_cast<T1> (x + y);
    auto sum2 = static_cast<T2> (x + y);

    if ((x + y) != sum1 && (x + y) != sum2) {
        THROW_OUT_OF_RANGE_EXCEPTION("Integer value out of range");
    }

    // overflow detection
    if (sizeof(x) >= sizeof(y)) {
        if ((x > 0 && y > 0 && sum1 < 0) || 
            (x < 0 && y < 0 && sum1 > 0)) {
            THROW_OUT_OF_RANGE_EXCEPTION("Integer value out of range");
        }
        return Value(lhs.GetTypeId(), sum1);
    }

    if ((x > 0 && y > 0 && sum2 < 0) || 
        (x < 0 && y < 0 && sum2 > 0)) {
        THROW_OUT_OF_RANGE_EXCEPTION("Integer value out of range");
    }
    return Value(rhs.GetTypeId(), sum2);
}

template <class T1, class T2>
Value IntegerParentType::SubtractValue(const Value &lhs, const Value &rhs) const {
    auto x = lhs.GetAs<T1>();
    auto y = rhs.GetAs<T2>();
    auto diff1 = static_cast<T1> (x - y);
    auto diff2 = static_cast<T2> (x - y);

    // i think this check can be avoided
    // by only do the up casting
    // i.e. convert x and y to the bigger type then do numeric operation
    if ((x - y) != diff1 && (x - y) != diff2) {
        THROW_OUT_OF_RANGE_EXCEPTION("Integer value out of range");
    }

    if (sizeof(x) >= sizeof(y)) {
        if ((x > 0 && y < 0 && diff1 < 0) ||
            (x < 0 && y > 0 && diff1 > 0)) {
            THROW_OUT_OF_RANGE_EXCEPTION("Integer value out of range");
        }
        return Value(lhs.GetTypeId(), diff1);
    }

    if ((x > 0 && y < 0 && diff2 < 0) ||
        (x < 0 && y > 0 && diff2 > 0)) {
        THROW_OUT_OF_RANGE_EXCEPTION("Integer value out of range");
    }
    return Value(rhs.GetTypeId(), diff2);
}

template <class T1, class T2>
Value IntegerParentType::MultiplyValue(const Value &lhs, const Value &rhs) const {
    auto x = lhs.GetAs<T1>();
    auto y = rhs.GetAs<T2>();
    auto prod1 = static_cast<T1> (x * y);
    auto prod2 = static_cast<T2> (x * y);
    if ((x * y) != prod1 && (x * y) != prod2) {
        // i'm thinking, if user intend to do this. should we return with the truncated value?
        THROW_OUT_OF_RANGE_EXCEPTION("Integer value out of range");
    }

    if (sizeof(x) >= sizeof(y)) {
        if ((x > 0 && y < 0 && prod1 < 0) ||
            (x < 0 && y > 0 && prod1 > 0)) {
            THROW_OUT_OF_RANGE_EXCEPTION("Integer value out of range");
        }
        return Value(lhs.GetTypeId(), prod1);
    }

    if ((x > 0 && y < 0 && prod2 < 0) ||
        (x < 0 && y > 0 && prod2 > 0)) {
        THROW_OUT_OF_RANGE_EXCEPTION("Integer value out of range");
    }
    return Value(rhs.GetTypeId(), prod2);
}

template <class T1, class T2>
Value IntegerParentType::DivideValue(const Value &lhs, const Value &rhs) const {
    auto x = lhs.GetAs<T1>();
    auto y = rhs.GetAs<T2>();
    
    if (y == 0) {
        THROW_DIVIDE_BY_ZERO_EXCEPTION("Division by zero");
    }

    auto quot1 = static_cast<T1> (x / y);
    auto quot2 = static_cast<T2> (x / y);

    if (sizeof(x) >= sizeof(y)) {
        return Value(lhs.GetTypeId(), quot1);
    }

    return Value(rhs.GetTypeId(), quot2);
}

template <class T1, class T2>
Value IntegerParentType::ModuloValue(const Value &lhs, const Value &rhs) const {
    auto x = lhs.GetAs<T1>();
    auto y = rhs.GetAs<T2>();
    
    if (y == 0) {
        THROW_DIVIDE_BY_ZERO_EXCEPTION("Division by zero");
    }

    auto rem1 = static_cast<T1> (x % y);
    auto rem2 = static_cast<T2> (x % y);

    if (sizeof(x) >= sizeof(y)) {
        return Value(lhs.GetTypeId(), rem1);
    }

    return Value(rhs.GetTypeId(), rem2);
}

}

#endif