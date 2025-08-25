//APP
#include <app/tools/zTools.h>

//SOCLE
#include <ign/geometry/all.h>

namespace app{
namespace tools{

    ///
    ///
    ///
    void zFiller( ign::geometry::Geometry & geom, double value ) {
        ign::geometry::Geometry::GeometryType geomType = geom.getGeometryType();
        switch( geomType )
        {
            case ign::geometry::Geometry::GeometryTypePoint :
                {
                    ign::geometry::Point & pt = geom.asPoint();
                    if (ign::numeric::Numeric<double>::IsNaN(pt.z()))
                        pt.setZ(value);
                    break;
                }
            case ign::geometry::Geometry::GeometryTypeMultiPoint :
                {
                    ign::geometry::MultiPoint & mpt = geom.asMultiPoint();
                    for (size_t i = 0 ; i < mpt.numGeometries() ; ++i ) 
                        zFiller(mpt.pointN(i), value);
                    break;
                }
            case ign::geometry::Geometry::GeometryTypeLineString :
                {
                    ign::geometry::LineString & ls = geom.asLineString();
                    for (size_t i = 0 ; i < ls.numPoints() ; ++i)
                        zFiller(ls.pointN(i), value);
                    break;
                }
            case ign::geometry::Geometry::GeometryTypeMultiLineString :
                {
                    ign::geometry::MultiLineString & mls_ = geom.asMultiLineString();
                    for (size_t i = 0 ; i < mls_.numGeometries() ; ++i ) 
                        zFiller(mls_.lineStringN(i), value);
                    break;
                }
            case ign::geometry::Geometry::GeometryTypePolygon :
                {
                    ign::geometry::Polygon & p = geom.asPolygon();
                    for (size_t i = 0 ; i < p.numRings() ; ++i ) 
                        zFiller(p.ringN(i), value);
                    break;
                }
            case ign::geometry::Geometry::GeometryTypeMultiPolygon :
                {
                    ign::geometry::MultiPolygon & mp = geom.asMultiPolygon();
                    for( size_t i = 0 ; i < mp.numGeometries() ; ++i )
                        zFiller(mp.polygonN(i), value);
                    break;
                }
            case ign::geometry::Geometry::GeometryTypeGeometryCollection :
                {
                    ign::geometry::GeometryCollection & collection = geom.asGeometryCollection();
                    for( size_t i = 0 ; i < collection.numGeometries() ; ++i )
                        zFiller(collection.geometryN(i), value);
                    break;
                }
            default:
                break;
        }

    }

    ///
    ///
    ///
    void removePointWithZ( ign::geometry::Geometry & geom, double value ) {
        ign::geometry::Geometry::GeometryType geomType = geom.getGeometryType();
        switch( geomType )
        { 
            case ign::geometry::Geometry::GeometryTypeLineString :
                {
                    ign::geometry::LineString & ls = geom.asLineString();
                    if ( !ls.isClosed() ) break;

                    std::set<size_t> sPoint2Remove;
                    for (size_t i = 0  ; i < ls.numPoints() ; ++i) {
                        size_t nextVertex = i == ls.numPoints() - 1 ? 1 : i+1;
                        size_t previousVertex = i == 0 ? ls.numPoints() - 2 : i-1;
                        if (ls.pointN(i).z() == value && ls.pointN(previousVertex).z() != value && ls.pointN(nextVertex).z() != value )
                            sPoint2Remove.insert(i);
                    }

                    for (std::set<size_t>::const_reverse_iterator sit = sPoint2Remove.rbegin() ; sit != sPoint2Remove.rend() ; ++sit)
                        ls.removePointN(*sit);

                    if ( !ls.isEmpty() && ! (ls.startPoint() == ls.endPoint()) )
                        ls.addPoint(ls.startPoint());
                    break;
                }
            case ign::geometry::Geometry::GeometryTypeMultiLineString :
                {
                    ign::geometry::MultiLineString & mls_ = geom.asMultiLineString();
                    for (size_t i = 0 ; i < mls_.numGeometries() ; ++i ) 
                        removePointWithZ(mls_.lineStringN(i), value);
                    break;
                }
            case ign::geometry::Geometry::GeometryTypePolygon :
                {
                    ign::geometry::Polygon & p = geom.asPolygon();
                    for (size_t i = 0 ; i < p.numRings() ; ++i ) 
                        removePointWithZ(p.ringN(i), value);
                    break;
                }
            case ign::geometry::Geometry::GeometryTypeMultiPolygon :
                {
                    ign::geometry::MultiPolygon & mp = geom.asMultiPolygon();
                    for( size_t i = 0 ; i < mp.numGeometries() ; ++i )
                        removePointWithZ(mp.polygonN(i), value);
                    break;
                }
            case ign::geometry::Geometry::GeometryTypeGeometryCollection :
                {
                    ign::geometry::GeometryCollection & collection = geom.asGeometryCollection();
                    for( size_t i = 0 ; i < collection.numGeometries() ; ++i )
                        removePointWithZ(collection.geometryN(i), value);
                    break;
                }
            default:
                break;
        }

    }

}
}