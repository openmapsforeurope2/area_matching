#ifndef _APP_CALCUL_MERGEAREASOP_H_
#define _APP_CALCUL_MERGEAREASOP_H_

//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>


//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>
#include <epg/tools/MultiLineStringTool.h>


namespace app{
namespace calcul{

	/// @brief
	class MergeAreasOp {

	public:

	
		/// @brief Constructeur
		/// @param verbose Mode verbeux
		MergeAreasOp(
            bool verbose
        );

        /// @brief Destructeur
        ~MergeAreasOp();

		/// @brief 
		/// @param verbose Mode verbeux
		static void Compute(
			bool verbose
		);


	private:

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
		void _mergeByNatId() const;

		//--
		bool _mergeSmallAreas() const;

		//--
		void _resetWTag() const;

        //--
        std::pair<bool, ign::feature::Feature> _getBestNeighbour(
            ign::feature::Feature const& fArea
        ) const;

		//--
		bool _isSlimSurface( ign::geometry::MultiPolygon const& mp ) const;
    };
}
}

#endif
