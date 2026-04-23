// APP
#include <app/calcul/ClipAreaOutOfCountryOp.h>
#include <app/params/ThemeParameters.h>
#include <app/tools/geometry/GeometryCleaner.h>

// BOOST
#include <boost/progress.hpp>

// EPG
#include <epg/Context.h>
#include <epg/params/EpgParameters.h>
#include <ome2/feature/sql/NotDestroyedTools.h>
#include <epg/sql/DataBaseManager.h>
#include <epg/tools/StringTools.h>
#include <epg/tools/TimeTools.h>
#include <epg/tools/FilterTools.h>
#include <epg/tools/geometry/ToValidGeometry.h>

// SOCLE
#include <ign/geometry/algorithm/SnapOpGeos.h>

namespace app
{
    namespace calcul
    {
        ///
        ///
        ///
        ClipAreaOutOfCountryOp::ClipAreaOutOfCountryOp(
            std::string borderCode,
            bool verbose
        ) : 
            _borderCode(borderCode),
            _verbose(verbose)
        {
            _init();
        }

        ///
        ///
        ///
        ClipAreaOutOfCountryOp::~ClipAreaOutOfCountryOp()
        {
            // _shapeLogger->closeShape("cbl_working_zone");
        }

        ///
        ///
        ///
        void ClipAreaOutOfCountryOp::Compute(
            std::string borderCode,
			bool verbose
		) {
            ClipAreaOutOfCountryOp clipAreaOutOfCountryOp(borderCode, verbose);
            clipAreaOutOfCountryOp._compute();
        }

        ///
        ///
        ///
        void ClipAreaOutOfCountryOp::_init()
        {
            //--
            _logger = epg::log::EpgLoggerS::getInstance();
            _logger->log(epg::log::INFO, "[START] initialization: " + epg::tools::TimeTools::getTime());

            //--
            _shapeLogger = epg::log::ShapeLoggerS::getInstance();
            // _shapeLogger->addShape("cbl_working_zone", epg::log::ShapeLogger::POLYGON);

            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const boundaryTableName = epgParams.getValue(TARGET_BOUNDARY_TABLE).toString();
            std::string const areaTableName = epgParams.getValue(AREA_TABLE).toString();
            std::string const idName = epgParams.getValue(ID).toString();
            std::string const geomName = epgParams.getValue(GEOM).toString();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();

            // app parameters
            params::ThemeParameters *themeParameters = params::ThemeParametersS::getInstance();
            std::string const landmaskTableName = themeParameters->getValue(LANDMASK_TABLE).toString();
            std::string const landCoverTypeName = themeParameters->getValue(LAND_COVER_TYPE_NAME).toString();
            std::string const landAreaValue = themeParameters->getValue(TYPE_LAND_AREA).toString();
			std::string const inlandwaterValue = themeParameters->getValue(TYPE_INLAND_WATER).toString();

            // on recupere un buffer autour de la frontiere
            ign::geometry::GeometryPtr boundBuffPtr(new ign::geometry::Polygon());
            ign::feature::sql::FeatureStorePostgis* fsBoundary = context->getDataBaseManager().getFeatureStore(boundaryTableName, idName, geomName);
            ign::feature::FeatureFilter boundaryFilter(countryCodeName + "='" + _borderCode +"'");

            ign::feature::FeatureIteratorPtr itBoundary = ome2::feature::sql::NotDestroyedTools::GetFeatures(*fsBoundary,boundaryFilter);
            while (itBoundary->hasNext())
            {
                ign::feature::Feature fBoundary = itBoundary->next();
                ign::geometry::LineString const& ls = fBoundary.getGeometry().asLineString();

                ign::geometry::GeometryPtr tmpBuffPtr(ls.buffer(20000));

                boundBuffPtr.reset(boundBuffPtr->Union(*tmpBuffPtr));
            }

            //on recupere la geometry des pays
            std::vector<std::string> vCountry;
		    epg::tools::StringTools::Split(_borderCode, "#", vCountry);

            for (std::vector<std::string>::iterator vit = vCountry.begin() ; vit != vCountry.end() ; ++vit) {
                _mCountryGeomPtr.insert(std::make_pair(*vit, ign::geometry::GeometryPtr(new ign::geometry::Polygon()) ));

                ign::feature::sql::FeatureStorePostgis* fsLandmask = context->getDataBaseManager().getFeatureStore(landmaskTableName, idName, geomName);
                ign::feature::FeatureIteratorPtr itLandmask = ome2::feature::sql::NotDestroyedTools::GetFeatures(*fsLandmask,ign::feature::FeatureFilter("("+landCoverTypeName + " = '" + landAreaValue +"' OR "+ landCoverTypeName + " = '" + inlandwaterValue + "') AND " + countryCodeName + " = '" + *vit + "'"));
                while (itLandmask->hasNext())
                {
                    ign::feature::Feature fLandmask = itLandmask->next();
                    ign::geometry::MultiPolygon const& mp = fLandmask.getGeometry().asMultiPolygon();

                    //on calcul la geometry de travail
                    ign::geometry::GeometryPtr intersectionPtr(boundBuffPtr->Intersection(mp));
                    if( (intersectionPtr->isPolygon() || intersectionPtr->isMultiPolygon()) && !intersectionPtr->isNull() && !intersectionPtr->isEmpty())
                        _mCountryGeomPtr[*vit].reset(_mCountryGeomPtr[*vit]->Union(*intersectionPtr));
                }
            }

            //--
            _fsArea = context->getDataBaseManager().getFeatureStore(areaTableName, idName, geomName);
            
            //--
            _logger->log(epg::log::INFO, "[END] initialization: " + epg::tools::TimeTools::getTime());
        };


        ///
        ///
        ///
        void ClipAreaOutOfCountryOp::_compute() const {
            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const geomName = epgParams.getValue(GEOM).toString();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();

            //--
			app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
			std::string const wTagName = themeParameters->getParameter(W_TAG_NAME).getValue().toString();
            double const cleaningAngle = themeParameters->getValue(GC_ANGLE_THRESHOLD).toDouble();

            ign::feature::FeatureFilter filterArea( wTagName + " IS NOT NULL" );
            int numFeatures = ome2::feature::sql::NotDestroyedTools::NumFeatures(*_fsArea, filterArea);
            boost::progress_display display(numFeatures, std::cout, "[ area clipping % complete ]\n");

            ign::feature::FeatureIteratorPtr itArea = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsArea, filterArea);
            while (itArea->hasNext())
            {
                ++display;
                
                ign::feature::Feature fArea = itArea->next();
                ign::geometry::MultiPolygon const& mp = fArea.getGeometry().asMultiPolygon();
                std::string idOrigin = fArea.getId();
                std::string country = fArea.getAttribute(countryCodeName).toString();

                //DEBUG
                _logger->log(epg::log::DEBUG, idOrigin);                

                std::map<std::string, ign::geometry::GeometryPtr>::const_iterator mit = _mCountryGeomPtr.find(country);
                if (mit == _mCountryGeomPtr.end()) {
                    _logger->log(epg::log::ERROR, "Unknown country [country code] " + country);
                    continue;
                }

                if (mit->second->intersects(mp.polygonN(0).pointOnSurface())) continue;

                //--
                std::string otherCountry = _getOtherCountry(country);
                ign::feature::FeatureFilter filterOtherArea( countryCodeName + " = '"+otherCountry+"' AND ST_INTERSECTS(" + geomName + ", ST_SetSRID(ST_GeomFromText('" + mp.toString() + "'),3035))");

                bool foundIntersection = false;
                ign::geometry::GeometryPtr allOthersPtr(new ign::geometry::Polygon());
                ign::feature::FeatureIteratorPtr itOtherArea = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsArea, filterOtherArea);
                while (itOtherArea->hasNext())
                {
                    foundIntersection = true;

                    ign::feature::Feature fOtherArea = itOtherArea->next();
                    ign::geometry::MultiPolygon const& OtherAreaGeom = fOtherArea.getGeometry().asMultiPolygon();

                    ign::geometry::GeometryPtr snappedGeomPtr(ign::geometry::algorithm::SnapOpGeos::SnapTo( *allOthersPtr, OtherAreaGeom, 0.1 ));
                    ign::geometry::GeometryPtr allOtherSnappedGeomPtr(ign::geometry::algorithm::SnapOpGeos::SnapTo( *snappedGeomPtr, *allOthersPtr, 0.1 ));
                    allOthersPtr.reset(allOtherSnappedGeomPtr->Union(*snappedGeomPtr));
                }

                if ( !foundIntersection )
                    continue;

                ign::geometry::GeometryPtr resultPtr(mp.Difference(*allOthersPtr));

                ign::geometry::MultiPolygon mpResult;
                ign::geometry::Geometry::GeometryType geomType = resultPtr->getGeometryType();
                switch( geomType )
                {
                    case ign::geometry::Geometry::GeometryTypePolygon :
                        {
                            ign::geometry::Polygon const& p = resultPtr->asPolygon();
                            if ( !p.isEmpty() ) mpResult.addGeometry(p);
                            break;
                        }
                    case ign::geometry::Geometry::GeometryTypeMultiPolygon :
                        {
                            ign::geometry::MultiPolygon const& mp = resultPtr->asMultiPolygon();
                            for( size_t i = 0 ; i < mp.numGeometries() ; ++i )
                                if ( !mp.polygonN(i).isEmpty() ) mpResult.addGeometry(mp.polygonN(i));
                            break;
                        }
                    case ign::geometry::Geometry::GeometryTypeGeometryCollection :
                        {
                            ign::geometry::GeometryCollection const& collection = resultPtr->asGeometryCollection();
                            for( size_t i = 0 ; i < collection.numGeometries() ; ++i ) {
                                if( collection.geometryN(i).isPolygon() ) {
                                    ign::geometry::Polygon const& p = collection.geometryN(i).asPolygon();
                                    if ( !p.isEmpty() ) mpResult.addGeometry(p);
                                }
                                if( collection.geometryN(i).isMultiPolygon() ) {
                                    ign::geometry::MultiPolygon const& mp = collection.geometryN(i).asMultiPolygon();
                                    for( size_t i = 0 ; i < mp.numGeometries() ; ++i )
                                        if ( !mp.polygonN(i).isEmpty() ) mpResult.addGeometry(mp.polygonN(i));
                                }
                            }
                            break;
                        }
                    default:
                        break;
                }

                fArea.setAttribute(wTagName, ign::data::String("split_area"));
                for ( size_t i = 0 ; i < mpResult.numGeometries() ; ++i ) {
                    //on supprime les artefacts
                    tools::geometry::GeometryCleaner::Compute(mpResult.polygonN(i), cleaningAngle);

                    if( mpResult.polygonN(i).isEmpty() )
                        continue;

                    fArea.setGeometry(mpResult.polygonN(i).toMulti());
                    _fsArea->createFeature(fArea);
                }

                _fsArea->deleteFeature(idOrigin);
            }
        }

        ///
        ///
        ///
        std::string ClipAreaOutOfCountryOp::_getOtherCountry(std::string const& country) const {
            std::vector<std::string> vCountry;
		    epg::tools::StringTools::Split(_borderCode, "#", vCountry);

            return vCountry.front() == country ? vCountry.back() : vCountry.front();
        }
    }
}