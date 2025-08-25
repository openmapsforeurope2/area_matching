#ifndef _APP_CALCUL_CLIPAREAOUTOFCOUNTRYOP_H_
#define _APP_CALCUL_CLIPAREAOUTOFCOUNTRYOP_H_

//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>


//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>
#include <epg/tools/MultiLineStringTool.h>


namespace app{
namespace calcul{

	/// @brief
	class ClipAreaOutOfCountryOp {

	public:

	
		/// @brief Constructeur
		/// @param verbose Mode verbeux
		ClipAreaOutOfCountryOp(
			std::string borderCode,
            bool verbose
        );

        /// @brief Destructeur
        ~ClipAreaOutOfCountryOp();

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
		std::string _getOtherCountry(std::string const& country) const;
    };
}
}

#endif
