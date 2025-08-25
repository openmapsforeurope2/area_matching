#ifndef _APP_STEP_CLIPAREAOUTOFCOUNTRY_H_
#define _APP_STEP_CLIPAREAOUTOFCOUNTRY_H_

#include <epg/step/StepBase.h>
#include <app/params/ThemeParameters.h>

namespace app{
namespace step{

	class ClipAreaOutOfCountry : public epg::step::StepBase< app::params::ThemeParametersS > {

	public:

		/// \brief
		int getCode() { return 420; };

		/// \brief
		std::string getName() { return "ClipAreaOutOfCountry"; };

		/// \brief
		void onCompute( bool );

		/// \brief
		void init();

	};

}
}

#endif