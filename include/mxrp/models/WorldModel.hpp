
#ifndef __mxrp_models_WorldModel_H__
#define __mxrp_models_WorldModel_H__

#include "mxrp/simulation/Simulation.hpp"

namespace mxrp {
namespace terrain { class Terrain; }
namespace models {
class AbstractAtmosphere;

//------------------------------------------------------------------------------
// Class: WorldModel
//
// Description: A model of the spatial world in terms of earth model, coordinates,
//              lat/long, etc.
//
//
// Factory name: WorldModel
//
// Slots --
//
//    latitude       <base::LatLon>           ! Reference (gaming area) latitude (default: 0.0)
//                   <base::Number>           ! Reference (gaming area) latitude (deg)
//
//    longitude      <base::LatLon>           ! reference (gaming area) longitude (default 0.0)
//                   <base::Number>           ! reference (gaming area) longitude (deg)
//
//    gamingAreaRange <base::Distance>        ! Max valid range of the simulation's gaming area or zero for unlimited
//                                            ! default: zero -- unlimited range
//
//    earthModel     <base::EarthModel>       ! Earth model for geodetic lat/lon (default is WGS-84)
//                   <base::Identifier>       ! Earth model by name (see EarthModel.hpp)
//
//    gamingAreaUseEarthModel <base::Boolean> ! If true, use the 'earthModel' or its WGS-84 default for flat
//                                            ! earth projections between geodetic lat/lon and the gaming
//                                            ! area's NED coordinates.  Otherwise, use a standard spherical
//                                            ! earth with a radius of nav::ERAD60. (default: false)
//
//                                            ! or zero to use current year (default: 0)
//
//
//    terrain        <terrain:Terrain>        ! Terrain elevation database (default: nullptr)
//    atmosphere     <Atmosphere>             ! Atmosphere
//

// Gaming area reference point:
//
//    The reference latitude and longitude is the center of the simulation's
//    gaming area.  Each player has a position, [ x y z ] NED in meters, from this
//    reference point that is computed using flat earth equations (see base::Nav)
//    and using the cosine of this reference latitude.  The NED coordinate system's
//    'down' is perpendicular to this tangent plane.
//
//       double getRefLatitude()
//       double getRefLongitude()
//          Return the reference point (degrees)
//
//       const base::Matrixd& getWorldMat()
//          Returns the world transformation matrix;
//          earth (NED) <== > Earth Centered, Earth Fixed (ECEF);
//          where the NED XY plane is tangent to and centered at
//          our reference point.
//
//                Matrix M = getWorldMat() = Rz[-lon] * Ry[90+lat]
//                Usage:
//                   Vned  = M * Vecef
//                   Vecef = Vned * M;
//
//
// Environments:
//
//    Current simulation environments include terrain elevation posts, getTerrain(),
//    and atmosphere model, getAtmosphere().
//
// Shutdown:
//
//    At shutdown, the parent object must send a SHUTDOWN_EVENT event to
//    this object, environments and other components.
//

//------------------------------------------------------------------------------
class WorldModel : public simulation::Simulation
{
    DECLARE_SUBCLASS(WorldModel, simulation::Simulation)

public:
    WorldModel();

    double getRefLatitude() const;                 // Returns the reference latitude (degs)
    double getRefLongitude() const;                // Returns the reference longitude (degs)
    double getSinRefLat() const;                   // Returns the sine of the reference latitude
    double getCosRefLat() const;                   // Returns the cosine of the reference latitude
    double getMaxRefRange() const;                 // Max valid range (meters) of the gaming area or zero if there's no limit.
    const base::Matrixd& getWorldMat() const;      // World transformation matrix
                                                   //    ECEF <==> NED
                                                   //       where the NED XY plane is tangent at our ref point
                                                   //    matrix = Rz[-lon] * Ry[90+lat]
                                                   //    Usage:
                                                   //       Vned  = M * Vecef
                                                   //       Vecef = Vned * M;

    const base::EarthModel* getEarthModel() const; // Returns a pointer to the EarthModel
                                                   // (default: if zero we're using base::EarthModel::wgs84)

    bool isGamingAreaUsingEarthModel() const;      // Gaming area using the earth model?



    // environmental interface
    const terrain::Terrain* getTerrain() const;            // returns the terrain elevation database
    AbstractAtmosphere* getAtmosphere();                   // returns the atmosphere model
    const AbstractAtmosphere* getAtmosphere() const;       // returns the atmosphere model (const version)

    virtual void reset() override;

protected:

    virtual bool setEarthModel(const base::EarthModel* const msg); // Sets our earth model
    virtual bool setGamingAreaUseEarthModel(const bool flg);

    virtual bool setRefLatitude(const double v);      // Sets Ref latitude
    virtual bool setRefLongitude(const double v);     // Sets Ref longitude
    virtual bool setMaxRefRange(const double v);      // Sets the max range (meters) of the gaming area or zero if there's no limit.

   // environmental interface
    terrain::Terrain* getTerrain();                        // returns the terrain elevation database
    virtual bool shutdownNotification() override;

private:
   void initData();

   bool setSlotRefLatitude(const base::LatLon* const msg);
   bool setSlotRefLatitude(const base::Number* const msg);
   bool setSlotRefLongitude(const base::LatLon* const msg);
   bool setSlotRefLongitude(const base::Number* const msg);

   bool setSlotGamingAreaRange(const base::Distance* const msg);
   bool setSlotEarthModel(const base::EarthModel* const msg);
   bool setSlotEarthModel(const base::String* const msg);
   bool setSlotGamingAreaEarthModel(const base::Number* const msg);

   // environmental interface
   bool setSlotTerrain(terrain::Terrain* const msg);
   bool setSlotAtmosphere(AbstractAtmosphere* const msg);

   // Our Earth Model, or default to using base::EarthModel::wgs84 if zero
   const base::EarthModel* em {};

   double refLat {};          // Reference (center of gaming area) latitude (deg)
   double refLon {};          // Reference (center of gaming area) longitude (deg)
   double sinRlat {};         // Sine of ref latitude
   double cosRlat {1.0};      // Cosine of ref latitude
   double maxRefRange {};     // Max valid range (meters) of the gaming area or zero if there's no limit.
   bool gaUseEmFlg {};        // Gaming area using earth model projections
   base::Matrixd wm;          // World transformation matrix:
                              //    Local tangent plane (NED) <==> Earth Centered, Earth Fixed (ECEF)
                              //    Usage:
                              //       ecef = wm; * earthNED
                              //       earthNED  = ecef * wm;

   AbstractAtmosphere* atmosphere {};
   terrain::Terrain* terrain {};

};

}
}

#endif