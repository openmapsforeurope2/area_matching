
//APP
#include <app/params/ThemeParameters.h>

//SOCLE
#include <ign/Exception.h>


namespace app{
namespace params{


	///
	///
	///
	ThemeParameters::ThemeParameters()
	{
		_initParameter( DB_CONF_FILE, "DB_CONF_FILE");
		_initParameter( WORKING_SCHEMA, "WORKING_SCHEMA");
		_initParameter( AREA_TABLE_INIT, "AREA_TABLE_INIT");
		_initParameter( AREA_TABLE_INIT_BASE, "AREA_TABLE_INIT_BASE");
		_initParameter( LANDMASK_TABLE, "LANDMASK_TABLE");
		_initParameter( LANDMASK_TABLE, "LANDMASK_TABLE" );
		_initParameter( LAND_COVER_TYPE_NAME, "LAND_COVER_TYPE_NAME" );
		_initParameter( TYPE_LAND_AREA, "TYPE_LAND_AREA");
		_initParameter( TYPE_INLAND_WATER, "TYPE_INLAND_WATER" );
		_initParameter( NATIONAL_IDENTIFIER_NAME, "NATIONAL_IDENTIFIER_NAME");
		_initParameter( COUNTRY_CODE_W, "COUNTRY_CODE_W");
		_initParameter( W_TAG_NAME, "W_TAG_NAME");

		_initParameter( GC_ANGLE_THRESHOLD, "GC_ANGLE_THRESHOLD");

		_initParameter( MA_SMALL_AREA_THRESHOLD, "MA_SMALL_AREA_THRESHOLD");
		_initParameter( MA_SLIM_AREA_THRESHOLD, "MA_SLIM_AREA_THRESHOLD");
	}

	///
	///
	///
	ThemeParameters::~ThemeParameters()
	{
	}

	///
	///
	///
	std::string ThemeParameters::getClassName()const
	{
		return "app::params::ThemeParameters";
	}


}
}