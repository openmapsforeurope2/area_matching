#ifndef _APP_CALCUL_INTERSECTINGAREASMERGEROP_H_
#define _APP_CALCUL_INTERSECTINGAREASMERGEROP_H_

//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>
#include <ign/tools/stringtools.h>

//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>


namespace app{
namespace calcul{

	/// @brief 
	class IntersectingAreasMergerOp {

	public:

	
        /// @brief 
        /// @param borderCode 
        /// @param verbose 
        IntersectingAreasMergerOp(
			std::string const& borderCode,
            bool verbose
        );

        /// @brief 
        ~IntersectingAreasMergerOp();


		/// \brief
		static void Compute(
			std::string const& borderCode,
			bool verbose
		);


	private:
		//--
		std::string                                              _borderCode;
		//--
		std::vector<std::string>                                 _vCountry;
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsArea;
		//--
		epg::log::EpgLogger*                                     _logger;
		//--
		epg::log::ShapeLogger*                                   _shapeLogger;
		//--
		bool                                                     _verbose;


	private:

		//--
		void _init();

		//--
		void _compute() const;

		//--
        void _getIntersectingAreas(
            ign::geometry::MultiPolygon const& mp, 
            size_t country, 
            std::set<std::string> & sIntersectingArea
        ) const;

		//--
		std::string _toSqlList(std::set<std::string> const& s, std::string separator = ",") const;

    };

}
}

#endif