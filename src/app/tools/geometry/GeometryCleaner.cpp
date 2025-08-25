#include <app/tools/geometry/GeometryCleaner.h>

// EPG
#include <epg/tools/geometry/angle.h>

namespace app{
namespace tools{
namespace geometry{

	///
	///
	///
	GeometryCleaner::GeometryCleaner( double angleTreshold ):
		_angleTreshold( angleTreshold )
	{
	}

	///
	///
	///
	GeometryCleaner::~GeometryCleaner()
	{
	}

    ///
	///
	///
	void GeometryCleaner::Compute( ign::geometry::Geometry & geom, double angleTreshold ) {
        GeometryCleaner geometryCleaner(angleTreshold);
        geometryCleaner._compute(geom);
    }

	///
	///
	///
	void GeometryCleaner::_compute( ign::geometry::Geometry & geom ) const {
        switch( geom.getGeometryType() )
		{
			case ign::geometry::Geometry::GeometryTypeLineString :{
				_compute( geom.asLineString() );
				break;}
			case ign::geometry::Geometry::GeometryTypePolygon :{
				_compute( geom.asPolygon() );
				break;}
			case ign::geometry::Geometry::GeometryTypeMultiLineString :{
				_compute( geom.asMultiLineString() );
				break;}
			case ign::geometry::Geometry::GeometryTypeMultiPolygon :{
				_compute( geom.asMultiPolygon() );
				break;}
			default :{
				IGN_THROW_EXCEPTION( "app::tools::geometry::GeometryCleaner:Compute : geometry type "+geom.getGeometryTypeName()+" not allowed" );
				break;
			}
		}
    }

    ///
    ///
    ///
    void GeometryCleaner::_compute( ign::geometry::LineString & ls ) const {

        if( ls.numPoints() < 2 ) {
            ls = ign::geometry::LineString();
            return;
        }

        ign::math::Vec2d vecPrevious = ls.endPoint().toVec2d();
        for ( int i = ls.numPoints()-2 ; i > 0 ; ) {
            ign::math::Vec2d vecCurrent = ls.pointN(i).toVec2d();
            ign::math::Vec2d vecNext = ls.pointN(i-1).toVec2d();

            double angle = epg::tools::geometry::angle( vecPrevious-vecCurrent, vecNext-vecCurrent );
            if( angle < _angleTreshold ) {
                ls.removePointN(i);
                if ( i == ls.numPoints()-1 )
                    --i;
            } else {
                vecPrevious = vecCurrent;
                --i;
            }
        }

        if( ls.isClosed() ) {
            bool removed = false;
            do {
                size_t numPoints = ls.numPoints();
                if( ls.numPoints() < 4 ) {
                    ls = ign::geometry::LineString();
                    return;
                }
    
                ign::math::Vec2d vecPrevious = ls.pointN(ls.numPoints()-2).toVec2d();
                ign::math::Vec2d vecCurrent = ls.startPoint().toVec2d();
                ign::math::Vec2d vecNext = ls.pointN(1).toVec2d();
                double angle = epg::tools::geometry::angle( vecPrevious-vecCurrent, vecNext-vecCurrent );
                if( angle < _angleTreshold ) {
                    removed = true;
                    ls.startPoint() = ls.pointN(ls.numPoints()-2);
                    ls.removePointN(ls.numPoints()-1);
                } else {
                    removed = false;
                }
            } while ( removed );
        }
    }
    
    ///
    ///
    ///
    void GeometryCleaner::_compute( ign::geometry::Polygon & poly ) const {
        for ( int i = poly.numInteriorRing()-1 ; i >= 0 ; --i ) {
            _compute(poly.interiorRingN(i));
            if( poly.interiorRingN(i).isEmpty() )
                poly.removeInteriorRingN(i);
        }

        _compute(poly.exteriorRing());
        if( poly.exteriorRing().isEmpty() )
            poly = ign::geometry::Polygon();
    }
    
    ///
    ///
    ///
    void GeometryCleaner::_compute( ign::geometry::MultiLineString & mls ) const {
        for ( int i = mls.numGeometries()-1 ; i >= 0 ; --i ) {
            _compute(mls.lineStringN(i));
            if ( mls.lineStringN(i).isEmpty() ) {
                mls.removeGeometryN(i);
            }
        }
    }
    
    ///
    ///
    ///
    void GeometryCleaner::_compute( ign::geometry::MultiPolygon & mp ) const {
        for ( int i = mp.numGeometries()-1 ; i >= 0 ; --i ) {
            _compute(mp.polygonN(i));
            if ( mp.polygonN(i).isEmpty() ) {
                mp.removeGeometryN(i);
            }
        }
    }

}
}
}