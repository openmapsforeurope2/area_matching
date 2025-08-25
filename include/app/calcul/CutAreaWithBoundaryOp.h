#ifndef _APP_CALCUL_CUTAREAWITHBOUNDARYOP_H_
#define _APP_CALCUL_CUTAREAWITHBOUNDARYOP_H_

//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>


//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>
#include <epg/tools/MultiLineStringTool.h>


namespace app{
namespace calcul{

	/// @brief
	class CutAreaWithBoundaryOp {

	public:

	
		/// @brief Constructeur
		/// @param verbose Mode verbeux
		CutAreaWithBoundaryOp(
			std::string borderCode,
            bool verbose
        );

        /// @brief Destructeur
        ~CutAreaWithBoundaryOp();

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
		epg::tools::MultiLineStringTool*                         _boundaryTool;
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
    };
}
}

#endif
