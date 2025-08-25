#include <app/step/425_CleanRemoteAreas.h>

//EPG
#include <ome2/utils/CopyTableUtils.h>

//APP
#include <app/calcul/CleanRemoteAreasOp.h>

namespace app {
	namespace step {

		///
		///
		///
		void CleanRemoteAreas::init()
		{
			addWorkingEntity(AREA_TABLE_INIT);
		}

		///
		///
		///
		void CleanRemoteAreas::onCompute(bool verbose = false)
		{
			// copie 
			_epgParams.setParameter(AREA_TABLE, ign::data::String(getCurrentWorkingTableName(AREA_TABLE_INIT)));
			ome2::utils::CopyTableUtils::copyAreaTable(getLastWorkingTableName(AREA_TABLE_INIT), "", false, true, true);

			// traitement
			app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
			std::string countryCodeW = themeParameters->getParameter(COUNTRY_CODE_W).getValue().toString();

			app::calcul::CleanRemoteAreasOp::Compute(countryCodeW, verbose);

		}

	}
}