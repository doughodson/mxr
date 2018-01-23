//------------------------------------------------------------------------------
// Classes:  Volume, CubicMeters, CubicFeet, CubicInches, Liters
//------------------------------------------------------------------------------

#ifndef __mixr_base_Volumes_H__
#define __mixr_base_Volumes_H__

#include "mixr/base/units/Unit.hpp"

#include "mixr/base/units/volume_utils.hpp"

namespace mixr {
namespace base {

//------------------------------------------------------------------------------
// Class: Volume
// Description:  Base class for volume.  Defined as a cubic meter which is
// equivalent to an instance of CubicMeters with its value equal to 1.0.
//
//
// Public methods (Defined in Volume, and inherited by all derived classes):
//
//     set(const double v)
//        Sets a Volume derived instance with an double.
//
//     set(const Volume& n)
//        Sets, and converts if necessary, a Volume derived
//        instance with another Volume derived instance.
//
//     double convert(const Volume& n)
//        Converts the value of a Volume derived instance into
//        the units of another Volume derived instance.
//
//   Output stream operator:<<
//        ostream& operator<<(ostream& sout, const Volume& n)
//        Sends "( <the Volume derived instance class name and value> )"
//        to the output stream.
//
//
// Public methods (For classes: CubicMeters, CubicFeet, CubicInches, Liters):
//    double convertStatic(const Volume &n)
//    Static function to convert the given Volume derived instance
//    into the units of a specific Volume derived class.
//
//------------------------------------------------------------------------------
class Volume : public Unit
{
    DECLARE_SUBCLASS(Volume, Unit)

public:
    Volume();
    Volume(const double);

    void set(const double v)  { val = v; }
    void set(const Volume& n) { val = fromVolume(n.toVolume()); }

    double convert(const Volume& n) const  { return fromVolume(n.toVolume()); }

    virtual double toVolume() const = 0;
    virtual double fromVolume(const double a) const = 0;
};

inline std::ostream& operator<<(std::ostream& sout, const Volume& n)
    { sout << "( " << n.getFactoryName() << " " << n.getReal() << " )"; return sout; }


//------------------------------------------------------------------------------
// Class: CubicMeters
// Description: An instance of CubicMeters with its value equal to 1.0 is one
// base unit for volume.
//------------------------------------------------------------------------------
class CubicMeters final: public Volume
{
    DECLARE_SUBCLASS(CubicMeters, Volume)

public:
    CubicMeters();
    CubicMeters(const double);
    CubicMeters(const Volume&);

    static double convertStatic(const Volume &n)      { return n.toVolume(); }

    double toVolume() const final                     { return val; }
    double fromVolume(const double a) const final     { return a; }
};

//------------------------------------------------------------------------------
// Class: CubicFeet
// Description: Cubic Meters * 35.31467
//------------------------------------------------------------------------------
class CubicFeet final: public Volume
{
    DECLARE_SUBCLASS(CubicFeet, Volume)

public:
    CubicFeet();
    CubicFeet(const double);
    CubicFeet(const Volume&);

    static double convertStatic(const Volume &n)     { return n.toVolume() * volume::CM2CFT; }

    double toVolume() const final                    { return (val * volume::CFT2CM); }
    double fromVolume(const double a) const final    { return a * volume::CM2CFT; }
};


//------------------------------------------------------------------------------
// Class: CubicInches
// Description: Cubic Meters * 61023.74
//------------------------------------------------------------------------------
class CubicInches final: public Volume
{
    DECLARE_SUBCLASS(CubicInches, Volume)

public:
    CubicInches();
    CubicInches(const double);
    CubicInches(const Volume&);

    static double convertStatic(const Volume &n)     { return n.toVolume() * volume::CM2CIN; }

    double toVolume() const final                    { return (val * volume::CIN2CM); }
    double fromVolume(const double a) const final    { return a * volume::CM2CIN; }
};


//------------------------------------------------------------------------------
// Class: Liters
// Description: Cubic Meters * 1000
//------------------------------------------------------------------------------
class Liters final: public Volume
{
    DECLARE_SUBCLASS(Liters, Volume)

public:
    Liters();
    Liters(const double);
    Liters(const Volume&);

    static double convertStatic(const Volume &n)     { return n.toVolume() * volume::CM2L; }

    double toVolume() const                          { return (val * volume::L2CM); }
    double fromVolume(const double a) const          { return a * volume::CM2L; }
};

}
}

#endif
