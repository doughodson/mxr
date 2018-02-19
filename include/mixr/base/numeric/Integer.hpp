
#ifndef __mixr_base_numeric_Integer_H__
#define __mixr_base_numeric_Integer_H__

#include "mixr/base/numeric/Number.hpp"

namespace mixr {
namespace base {

//------------------------------------------------------------------------------
// Class: Integer
// Description: Class that stores an EDL defined (regex) integer value as a C++ int
//------------------------------------------------------------------------------
// EDL Interface:
//
// Factory name: Integer
// Slots:
//     value  <Integer>  ! Sets the value of this number (default: 0)
//------------------------------------------------------------------------------
class Integer final: public Number
{
    DECLARE_SUBCLASS(Integer, Number)

public:
    explicit Integer()                         { STANDARD_CONSTRUCTOR() }
    explicit Integer(const int x) : val(x)     { STANDARD_CONSTRUCTOR() }

    void setValue(const int x)                 { val = x; }
    int asInt() const                          { return val; }

private:
   double getValue() const final               { return static_cast<int>(val); }
   int val{};

private:
   // slot table helper methods
   virtual bool setSlotValue(const Integer* const);
};

}
}

#endif
