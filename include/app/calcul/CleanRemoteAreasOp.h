#ifndef _APP_CALCUL_CLEANREMOTEAREASOP_H_
#define _APP_CALCUL_CLEANREMOTEAREASOP_H_

//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>


//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>
#include <epg/tools/MultiLineStringTool.h>


namespace app{
namespace calcul{

	/// @brief
	class CleanRemoteAreasOp {

		typedef std::multimap<std::string, std::string>::const_iterator  m_iterator;

	public:

		/// @brief Constructeur
		/// @param verbose Mode verbeux
		CleanRemoteAreasOp(
			std::string borderCode,
            bool verbose
        );

        /// @brief Destructeur
        ~CleanRemoteAreasOp();

		/// @brief 
		/// @param verbose Mode verbeux
		static void Compute(
			std::string borderCode,
			bool verbose
		);

	private:

		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsArea;
		//--
		std::map<std::string, ign::geometry::GeometryPtr>        _mCountryGeomPtr;
		//--
		epg::log::EpgLogger*                                     _logger;
		//--
		epg::log::ShapeLogger*                                   _shapeLogger;
		//--
		std::string                                              _borderCode;
		//--
		bool                                                     _verbose;


	private:

		//--
		void _init();

        //--
		void _compute() const;

		//--
		void _addAdjacentAreasConditions(
            ign::feature::FeatureFilter & filter,
            std::multimap<std::string, std::string> const& mAdjacency,
            std::string const& areaId
        ) const;
    };
}
}

#endif
