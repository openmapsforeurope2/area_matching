// APP
#include <app/calcul/CutAreaWithBoundaryOp.h>
#include <app/params/ThemeParameters.h>
#include <app/tools/geometry/PolygonSplitter.h>
#include <app/tools/zTools.h>

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


namespace app
{
    namespace calcul
    {
        ///
        ///
        ///
        CutAreaWithBoundaryOp::CutAreaWithBoundaryOp(
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
        CutAreaWithBoundaryOp::~CutAreaWithBoundaryOp()
        {
            // _shapeLogger->closeShape("cbl_working_zone");
        }

        ///
        ///
        ///
        void CutAreaWithBoundaryOp::Compute(
            std::string borderCode,
			bool verbose
		) {
            CutAreaWithBoundaryOp CutAreaWithBoundaryOp(borderCode, verbose);
            CutAreaWithBoundaryOp._compute();
        }

        ///
        ///
        ///
        void CutAreaWithBoundaryOp::_init()
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

            //--
            _fsArea = context->getDataBaseManager().getFeatureStore(areaTableName, idName, geomName);

            //--
            // on récupère l'extent des aires à traiter
            ign::geometry::Envelope areaExtent = _fsArea->getBounds();

            std::vector<std::string> vCountry;
		    epg::tools::StringTools::Split(_borderCode, "#", vCountry);

            ign::geometry::GeometryPtr boundBuffPtr(new ign::geometry::Polygon());
            ign::feature::sql::FeatureStorePostgis* fsBoundary = context->getDataBaseManager().getFeatureStore(boundaryTableName, idName, geomName);
            ign::feature::FeatureFilter boundaryFilter(countryCodeName + " LIKE '%" + vCountry.front() +"%' OR "+countryCodeName + " LIKE '%" + vCountry.back() +"%'");
            boundaryFilter.setExtent(areaExtent);

            //--
            _boundaryTool = new epg::tools::MultiLineStringTool(boundaryFilter, *fsBoundary);

            //--
            _logger->log(epg::log::INFO, "[END] initialization: " + epg::tools::TimeTools::getTime());
        };


        ///
        ///
        ///
        void CutAreaWithBoundaryOp::_compute() const {

            //--
			app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
			std::string const wTagName = themeParameters->getParameter(W_TAG_NAME).getValue().toString();

            ign::feature::FeatureFilter filterArea;
            int numFeatures = ome2::feature::sql::NotDestroyedTools::NumFeatures(*_fsArea, filterArea);
            boost::progress_display display(numFeatures, std::cout, "[ area clipping  % complete ]\n");

            ign::feature::FeatureIteratorPtr itArea = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsArea, filterArea);
            while (itArea->hasNext())
            {
                ++display;
                
                ign::feature::Feature fArea = itArea->next();
                ign::geometry::MultiPolygon const& mp = fArea.getGeometry().asMultiPolygon();
                std::string idOrigin = fArea.getId();

                //--
                std::vector<ign::geometry::Polygon> vPolygons;

                for( size_t i = 0 ; i < mp.numGeometries() ; ++i ) {
                    app::tools::geometry::PolygonSplitter polySplitter(mp.polygonN(i));

                    ign::geometry::MultiLineString mlsLocalBoundary;
                    _boundaryTool->getLocal(mp.polygonN(i).getEnvelope(), mlsLocalBoundary);

                    polySplitter.addCuttingGeometry(mlsLocalBoundary);
                    polySplitter.split( vPolygons );
                }

                if( vPolygons.size() < 2 ) 
                    continue;

                fArea.setAttribute(wTagName, ign::data::String("split_area"));
                for (size_t i = 0 ; i < vPolygons.size() ; ++i) {

                    tools::zFiller(vPolygons[i], -1000); //TODO a parametrer

                    fArea.setGeometry(vPolygons[i].toMulti());
                    _fsArea->createFeature(fArea);
                }

                _fsArea->deleteFeature(idOrigin);
            }
        }
    }
}