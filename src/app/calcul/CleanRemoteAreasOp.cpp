// APP
#include <app/calcul/CleanRemoteAreasOp.h>
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

// SOCLE
#include <ign/graph/Graph.h>
#include <ign/graph/algorithm/ConnectedComponents.h>


namespace app
{
    namespace calcul
    {
        ///
        ///
        ///
        CleanRemoteAreasOp::CleanRemoteAreasOp(
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
        CleanRemoteAreasOp::~CleanRemoteAreasOp()
        {
            // _shapeLogger->closeShape("cbl_working_zone");
        }

        ///
        ///
        ///
        void CleanRemoteAreasOp::Compute(
            std::string borderCode,
			bool verbose
		) {
            CleanRemoteAreasOp CleanRemoteAreasOp(borderCode, verbose);
            CleanRemoteAreasOp._compute();
        }

        ///
        ///
        ///
        void CleanRemoteAreasOp::_init()
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

            ign::feature::FeatureIteratorPtr itBoundary = ome2::feature::sql::NotDestroyedTools::GetFeatures(*fsBoundary, boundaryFilter);
            while (itBoundary->hasNext())
            {
                ign::feature::Feature const& fBoundary = itBoundary->next();
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
                    ign::feature::Feature const& fLandmask = itLandmask->next();
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
        void CleanRemoteAreasOp::_compute() const {
            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const idName = epgParams.getValue(ID).toString();
            std::string const geomName = epgParams.getValue(GEOM).toString();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();

            //--
			app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
			std::string const wTagName = themeParameters->getParameter(W_TAG_NAME).getValue().toString();

            //--
            typedef ign::graph::NoPropertyGraph                      Graph;
            typedef ign::graph::NoPropertyGraph::vertex_descriptor   vertex_descriptor;
            Graph graph;

            //--
            ign::feature::FeatureFilter filterArea( wTagName + " IS NOT NULL" );
            int numFeatures = ome2::feature::sql::NotDestroyedTools::NumFeatures(*_fsArea, filterArea);

            //--
            ign::feature::FeatureIteratorPtr itArea = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsArea, filterArea);
            std::map<std::string, vertex_descriptor> mAreaVertex;
            while (itArea->hasNext())
            {
                ign::feature::Feature const& fArea = itArea->next();
                std::string areaId = fArea.getId();

                mAreaVertex.insert(std::make_pair(areaId, graph.addVertex()));
            }

            //--
            boost::progress_display display(numFeatures, std::cout, "[ cleaning remote areas (1/2) % complete ]\n");

            std::multimap<std::string, std::string> mAdjacency;

            itArea = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsArea, filterArea);
            while (itArea->hasNext())
            {
                ++display;

                ign::feature::Feature const& fArea = itArea->next();
                ign::geometry::MultiPolygon const& areaGeom = fArea.getGeometry().asMultiPolygon();
                std::string areaId = fArea.getId();
                std::string const& areaCountry = fArea.getAttribute(countryCodeName).toString();

                ign::feature::FeatureFilter filterArea( wTagName + " IS NOT NULL" );
                _addAdjacentAreasConditions(filterArea, mAdjacency, areaId);
                epg::tools::FilterTools::addAndConditions(filterArea, "ST_DISTANCE(" + geomName + ", ST_SetSRID(ST_GeomFromText('" + areaGeom.toString() + "'),3035)) < 0.1" );
                epg::tools::FilterTools::addAndConditions(filterArea, countryCodeName + " = '" + areaCountry +"'" );
                ign::feature::FeatureIteratorPtr itArea2 = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsArea, filterArea);
                while (itArea2->hasNext())
                {
                    ign::feature::Feature const& fNeighbour = itArea2->next();
                    std::string idNeighbour = fNeighbour.getId();
                    graph.addEdge( mAreaVertex[areaId], mAreaVertex[idNeighbour]);

                    mAdjacency.insert(std::make_pair(idNeighbour, areaId));
                }
            }

            //--
            std::map< vertex_descriptor, size_t > mConn;
            ign::graph::algorithm::ConnectedComponents< Graph >::Search( mConn , graph );

            //--
            boost::progress_display display2(numFeatures, std::cout, "[ cleaning remote areas (2/2) % complete ]\n");

            std::set<size_t> sNotRemoteConn;
            itArea = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsArea, filterArea);
            while (itArea->hasNext())
            {
                ++display2;

                ign::feature::Feature const& fArea = itArea->next();
                ign::geometry::MultiPolygon const& areaGeom = fArea.getGeometry().asMultiPolygon();
                std::string areaId = fArea.getId();
                std::string const& areaCountry = fArea.getAttribute(countryCodeName).toString();

                if( sNotRemoteConn.find(mConn[mAreaVertex[areaId]]) != sNotRemoteConn.end() )
                    continue;

                std::map<std::string, ign::geometry::GeometryPtr>::const_iterator mit = _mCountryGeomPtr.find(areaCountry);
                if (mit == _mCountryGeomPtr.end()) {
                    _logger->log(epg::log::ERROR, "Unknown country [country code] " + areaCountry);
                    continue;
                }

                if (!mit->second->intersects(areaGeom))
                    continue;
                
                sNotRemoteConn.insert(mConn[mAreaVertex[areaId]]);
            }

            //--
            for ( std::map<std::string, vertex_descriptor>::const_iterator mit = mAreaVertex.begin() ; mit != mAreaVertex.end() ; ++mit ) {
                if( sNotRemoteConn.find(mConn[mit->second]) != sNotRemoteConn.end() )
                    continue;
                _fsArea->deleteFeature(mit->first);
            }
        }

        ///
        ///
        ///
        void CleanRemoteAreasOp::_addAdjacentAreasConditions(
            ign::feature::FeatureFilter & filter,
            std::multimap<std::string, std::string> const& mAdjacency,
            std::string const& areaId
        ) const {
            //--
            std::string const idName = epg::ContextS::getInstance()->getEpgParameters().getValue(ID).toString();

            std::pair<m_iterator, m_iterator> bounds = mAdjacency.equal_range( areaId );
            std::string list = "";
            for ( m_iterator it = bounds.first ; it != bounds.second ; ++it )
            {
                list += (it != bounds.first ? "','":"") + it->second;
            }
            if ( list.empty() )
                return;

            epg::tools::FilterTools::addAndConditions(filter, idName + " NOT IN ('" + list + "')");
        }
    }
}