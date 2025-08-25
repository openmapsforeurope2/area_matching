// APP
#include <app/calcul/MergeAreasOp.h>
#include <app/params/ThemeParameters.h>

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
#include <epg/tools/geometry/getLength.h>
#include <ome2/geometry/tools/isSlimSurface.h>
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
        MergeAreasOp::MergeAreasOp(
            bool verbose
        ) : 
            _verbose(verbose)
        {
            _init();
        }

        ///
        ///
        ///
        MergeAreasOp::~MergeAreasOp()
        {
            _shapeLogger->closeShape("ma_merged_small_area");
            // _shapeLogger->closeShape("ma_merged_slim_area");
        }

        ///
        ///
        ///
        void MergeAreasOp::Compute(
			bool verbose
		) {
            MergeAreasOp MergeAreasOp(verbose);
            MergeAreasOp._compute();
        }

        ///
        ///
        ///
        void MergeAreasOp::_init()
        {
            //--
            _logger = epg::log::EpgLoggerS::getInstance();
            _logger->log(epg::log::INFO, "[START] initialization: " + epg::tools::TimeTools::getTime());

            //--
            _shapeLogger = epg::log::ShapeLoggerS::getInstance();
            _shapeLogger->addShape("ma_merged_small_area", epg::log::ShapeLogger::POLYGON);
            // _shapeLogger->addShape("ma_merged_slim_area", epg::log::ShapeLogger::POLYGON);

            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const areaTableName = epgParams.getValue(AREA_TABLE).toString();
            std::string const idName = epgParams.getValue(ID).toString();
            std::string const geomName = epgParams.getValue(GEOM).toString();

            //--
            _fsArea = context->getDataBaseManager().getFeatureStore(areaTableName, idName, geomName);
            
            //--
            _logger->log(epg::log::INFO, "[END] initialization: " + epg::tools::TimeTools::getTime());
        };

        ///
        ///
        ///
        void MergeAreasOp::_compute() const {
            _mergeByNatId();
            while(_mergeSmallAreas()){};
        }


        ///
        ///
        ///
        void MergeAreasOp::_mergeByNatId() const {
            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const geomName = epgParams.getValue(GEOM).toString();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();

            //--
			app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
			std::string const wTagName = themeParameters->getParameter(W_TAG_NAME).getValue().toString();
	        std::string const natIdIdName = themeParameters->getValue(NATIONAL_IDENTIFIER_NAME).toString();

            ign::feature::FeatureFilter filterArea( wTagName + " IS NOT NULL" );
            int numFeatures = ome2::feature::sql::NotDestroyedTools::NumFeatures(*_fsArea, filterArea);
            boost::progress_display display(numFeatures, std::cout, "[ merging by nat_id % complete ]\n");

            std::map<std::string, ign::geometry::GeometryPtr> mNatIdMergedGeom;
            std::map<std::string, std::set<std::string>> mNatIdIds;
            ign::feature::FeatureIteratorPtr itArea = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsArea, filterArea);
            while (itArea->hasNext())
            {
                ++display;
                
                ign::feature::Feature const& fArea = itArea->next();
                ign::geometry::MultiPolygon const& mp = fArea.getGeometry().asMultiPolygon();
                std::string idOrigin = fArea.getId();
                std::string natId = fArea.getAttribute(natIdIdName).toString();

                //DEBUG
                // if ( mp.intersects( ign::geometry::Point(4041531.3,2937780.9))) {
                //     bool test = true;
                // }
                // if ( mp.intersects( ign::geometry::Point(4041532.9,2937761.2))) {
                //     bool test = true;
                // }

                std::map<std::string, ign::geometry::GeometryPtr>::iterator mit = mNatIdMergedGeom.find(natId);
                if( mit == mNatIdMergedGeom.end() ) {
                    mNatIdMergedGeom.insert(std::make_pair(natId, ign::geometry::GeometryPtr(mp.clone())));
                    mNatIdIds.insert(std::make_pair(natId, std::set<std::string>()));
                } else {
                    ign::geometry::GeometryPtr snappedGeomPtr(ign::geometry::algorithm::SnapOpGeos::SnapTo( *mit->second, mp, 0.1 ));
                    ign::geometry::GeometryPtr snappedResultPtr(ign::geometry::algorithm::SnapOpGeos::SnapTo( *snappedGeomPtr, *mit->second, 0.1 ));
                    mit->second.reset(snappedResultPtr->Union(*snappedGeomPtr));
                }
                mNatIdIds[natId].insert(idOrigin);
            }

            for( std::map<std::string, std::set<std::string>>::const_iterator mit = mNatIdIds.begin() ; mit != mNatIdIds.end() ; ++mit ) {
                if( mit->second.size() < 2 )
                    continue;

                ign::geometry::GeometryPtr resultPtr(mNatIdMergedGeom[mit->first].release());

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

                ign::feature::Feature featRef;
                _fsArea->getFeatureById(*mit->second.begin(), featRef);
                for ( size_t i = 0 ; i < mpResult.numGeometries() ; ++i ) {
                    //DEBUG
                    // if ( mpResult.polygonN(i).intersects( ign::geometry::Point(4041531.3,2937780.9))) {
                    //     bool test = true;
                    // }
                    // if (!mpResult.polygonN(i).exteriorRing().isSimple()) {
                    //     epg::tools::geometry::ToValidGeometry::transform(mpResult.polygonN(i));
                    // }
                    featRef.setGeometry(mpResult.polygonN(i).toMulti());
                    _fsArea->createFeature(featRef);
                }

                for ( std::set<std::string>::const_iterator sit = mit->second.begin() ; sit != mit->second.end() ; ++sit ) {
                    _fsArea->deleteFeature(*sit);
                }
            }
        }

        ///
        ///
        ///
        bool MergeAreasOp::_mergeSmallAreas() const {
            //--
			app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
            double const areaThreshold = themeParameters->getValue(MA_SMALL_AREA_THRESHOLD).toDouble();
			std::string const wTagName = themeParameters->getParameter(W_TAG_NAME).getValue().toString();

            std::map<double, ign::feature::Feature> mSortedSmallAreas;

            ign::feature::FeatureFilter filterArea( wTagName + " IS NOT NULL" );
            ign::feature::FeatureIteratorPtr itArea = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsArea, filterArea);
            while (itArea->hasNext())
            {
                ign::feature::Feature fArea = itArea->next();
                ign::geometry::MultiPolygon const& mp = fArea.getGeometry().asMultiPolygon();
                std::string areaId = fArea.getId();
                
                double area = mp.area();

                //DEBUG
                // if ( mp.intersects( ign::geometry::Point(4089689.79,2543176.70))) {
                //     bool test = true;
                // }
                // if ( mp.intersects( ign::geometry::Point(4089689.79,2543176.70))) {
                //     bool test = true;
                // }
                
                //DEBUG
                // if ( _isSlimSurface(mp) ) {
                //     ign::feature::Feature feat;
                //     feat.setGeometry(mp);
                //     _shapeLogger->writeFeature("ma_merged_slim_area", feat);
                // }

                if( area > areaThreshold && !_isSlimSurface(mp) ) {
                    fArea.setAttribute(wTagName, ign::data::Null());
                    _fsArea->modifyFeature(fArea);
                    continue;
                };

                mSortedSmallAreas.insert(std::make_pair(area, fArea));
            }
            
            //--
            _resetWTag();

            //--
            std::set<std::string> sTreatedArea;

            boost::progress_display display(mSortedSmallAreas.size(), std::cout, "[ merging small areas % complete ]\n");
            for ( std::map<double, ign::feature::Feature>::iterator mit = mSortedSmallAreas.begin() ; mit != mSortedSmallAreas.end() ; ++mit, ++display ) {
                if( sTreatedArea.find(mit->second.getId()) != sTreatedArea.end() )
                    continue;

                //DEBUG
                // std::string test1 = mit->second.getId();
                // if ( mit->second.getGeometry().intersects( ign::geometry::Point(4089689.79,2543176.70))) {
                //     bool test = true;
                // }
                // if ( mit->second.getGeometry().intersects( ign::geometry::Point(4041532.9,2937761.2))) {
                //     bool test = true;
                // }

                std::pair<bool, ign::feature::Feature> foundBestNeighbour = _getBestNeighbour(mit->second);

                if ( !foundBestNeighbour.first ) 
                    continue;

                 //DEBUG
                // std::string test2 = foundBestNeighbour.second.getId();

                ign::geometry::GeometryPtr snappedGeomPtr(ign::geometry::algorithm::SnapOpGeos::SnapTo( foundBestNeighbour.second.getGeometry(), mit->second.getGeometry(), 0.1 ));
                ign::geometry::GeometryPtr snappedResultPtr(ign::geometry::algorithm::SnapOpGeos::SnapTo( *snappedGeomPtr, foundBestNeighbour.second.getGeometry(), 0.1 ));
                ign::geometry::GeometryPtr resultingGeomPtr(snappedResultPtr->Union(*snappedGeomPtr));

                //DEBUG
                // {
                //     ign::feature::Feature feat;
                //     feat.setGeometry(mit->second.getGeometry());
                //     _shapeLogger->writeFeature("ma_merged_small_area", feat);
                // }

                ign::geometry::Geometry::GeometryType geomType = resultingGeomPtr->getGeometryType();
                switch( geomType )
                {
                    case ign::geometry::Geometry::GeometryTypeMultiPolygon :
                        {
                            foundBestNeighbour.second.setGeometry(resultingGeomPtr->asMultiPolygon());
                            break;
                        }
                    case ign::geometry::Geometry::GeometryTypePolygon :
                        {
                            foundBestNeighbour.second.setGeometry(resultingGeomPtr->asPolygon().toMulti());
                            break;
                        }
                    default : 
                        {
                            _logger->log(epg::log::ERROR, "Unexpected merging result between features : "+mit->second.getId()+" / "+foundBestNeighbour.second.getId());
                            continue;
                        }
                }

                sTreatedArea.insert(foundBestNeighbour.second.getId());

                foundBestNeighbour.second.setAttribute(wTagName, ign::data::String("merged_area"));
                _fsArea->modifyFeature(foundBestNeighbour.second);

                _fsArea->deleteFeature(mit->second.getId());
            }

            return sTreatedArea.size() > 0;
        }


        bool MergeAreasOp::_isSlimSurface( ign::geometry::MultiPolygon const& mp ) const {
            //--
			app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
			double width = themeParameters->getParameter(MA_SLIM_AREA_THRESHOLD).getValue().toDouble();

            for( size_t i = 0 ; i < mp.numGeometries() ; ++i )
                if( !ome2::geometry::tools::isSlimSurface(mp.polygonN(i), width) )
                    return false;
            return true;
        }

        ///
        ///
        ///
        void MergeAreasOp::_resetWTag() const {
            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const areaTableName = epgParams.getValue(AREA_TABLE).toString();

            //--
			app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
			std::string const wTagName = themeParameters->getParameter(W_TAG_NAME).getValue().toString();

            std::ostringstream ss;
            ss << "UPDATE " << areaTableName 
                << " SET " << wTagName << " = NULL"
                << " WHERE " << wTagName << " IS NOT NULL";
            
            context->getDataBaseManager().getConnection()->update(ss.str());
        }

        ///
        ///
        ///
        std::pair<bool, ign::feature::Feature> MergeAreasOp::_getBestNeighbour(
            ign::feature::Feature const& fArea
        ) const {
            //--
            ign::geometry::MultiPolygon const& areaGeom = fArea.getGeometry().asMultiPolygon();
            ign::geometry::MultiLineString mlsExteriorRings;
            for (size_t i = 0 ; i < areaGeom.numGeometries() ; ++i)
                mlsExteriorRings.addGeometry(areaGeom.polygonN(i).exteriorRing());

            // epg parameters
            epg::params::EpgParameters const& epgParams = epg::ContextS::getInstance()->getEpgParameters();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();
            std::string const geomName = epgParams.getValue(GEOM).toString();

            ign::feature::FeatureFilter filterArea("ST_DISTANCE(" + geomName + ", ST_SetSRID(ST_GeomFromText('" + areaGeom.toString() + "'),3035)) < 0.1");
            ign::feature::FeatureIteratorPtr itArea = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsArea, filterArea);
            
            double maxLength = 0;
            ign::feature::Feature featMax;
            while (itArea->hasNext())
            {
                ign::feature::Feature const& fNeighbour = itArea->next();
                std::string idNeighbour = fNeighbour.getId();

                if(idNeighbour == fArea.getId()) continue;

                ign::geometry::MultiPolygon const& neighbourGeom = fNeighbour.getGeometry().asMultiPolygon();

                ign::geometry::GeometryPtr extendedGeom(neighbourGeom.buffer(0.1));
                ign::geometry::GeometryPtr intersectionGeom(extendedGeom->Intersection(mlsExteriorRings));

                double length = epg::tools::geometry::getLength(*intersectionGeom);

                if ( length > maxLength ) {
                    maxLength = length;
                    featMax = fNeighbour;
                }
            }

            return std::make_pair(maxLength == 0 ? false:true, featMax);
        }
    }
    
}
