#ifndef _APP_STEP_CLEANREMOTEAREAS_H_
#define _APP_STEP_CLEANREMOTEAREAS_H_

#include <epg/step/StepBase.h>
#include <app/params/ThemeParameters.h>

namespace app{
namespace step{

	class CleanRemoteAreas : public epg::step::StepBase< app::params::ThemeParametersS > {

	public:

		/// \brief
		int getCode() { return 425; };

		/// \brief
		std::string getName() { return "CleanRemoteAreas"; };

		/// \brief
		void onCompute( bool );

		/// \brief
		void init();

	};

}
}

#endif