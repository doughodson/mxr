
#ifndef __mixr_simulation_AbstractIgHost_H__
#define __mixr_simulation_AbstractIgHost_H__

#include "mixr/base/Component.hpp"

namespace mixr {
namespace base { class PairStream; }
namespace simulation {
class AbstractPlayer;

//------------------------------------------------------------------------------
// Class: AbstractIgHost
// Description: Abstract image genertor visual system host interface
//
// Notes:
//    1) Derived versions of this class are used to interface with various
//       image generators.
//
//    2) One or more IgHost objects can be attached to and managed by a Station object.
//
// Factory name: AbstractIgHost
//------------------------------------------------------------------------------
class AbstractIgHost : public base::Component
{
   DECLARE_SUBCLASS(AbstractIgHost, base::Component)

public:
   AbstractIgHost();

   // Sets our ownship pointer; public version, which is usually called by the Station class.  Derived classes
   // can override this function and control the switching of the ownship using setOwnship0()
   virtual void setOwnship(AbstractPlayer* const) =0;
   virtual void setPlayerList(base::PairStream* const) =0; // Sets the player list that we're to use to generate player/models

   void updateTC(const double dt = 0.0) final {
      BaseClass::updateTC(dt);
      updateIg(dt);
   }

private:
   virtual void updateIg(const double dt = 0.0) =0;
};

}
}

#endif
