#ifndef _APP_PARAMS_THEMEPARAMETERS_H_
#define _APP_PARAMS_THEMEPARAMETERS_H_

//STL
#include <string>

//EPG
#include <epg/params/ParametersT.h>
#include <epg/SingletonT.h>



	enum TH_PARAMETERS{
		DB_CONF_FILE,
		WORKING_SCHEMA,
		AREA_TABLE_INIT,
		LANDMASK_TABLE,
		LAND_COVER_TYPE_NAME,
		TYPE_LAND_AREA,
		TYPE_INLAND_WATER,
		NATIONAL_IDENTIFIER_NAME,
		COUNTRY_CODE_W,
		W_TAG_NAME,

		MA_SMALL_AREA_THRESHOLD,
		MA_SLIM_AREA_THRESHOLD
	};


namespace app{
namespace params{

	class ThemeParameters : public epg::params::ParametersT< TH_PARAMETERS >
	{
		typedef  epg::params::ParametersT< TH_PARAMETERS > Base;

		public:

			/// \brief
			ThemeParameters();

			/// \brief
			~ThemeParameters();

			/// \brief
			virtual std::string getClassName()const;

	};

	typedef epg::Singleton< ThemeParameters >   ThemeParametersS;

}
}

#endif