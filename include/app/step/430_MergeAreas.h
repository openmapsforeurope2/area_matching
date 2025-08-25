#ifndef _APP_STEP_MERGEAREAS_H_
#define _APP_STEP_MERGEAREAS_H_

#include <epg/step/StepBase.h>
#include <app/params/ThemeParameters.h>

namespace app{
namespace step{

	class MergeAreas : public epg::step::StepBase< app::params::ThemeParametersS > {

	public:

		/// \brief
		int getCode() { return 430; };

		/// \brief
		std::string getName() { return "MergeAreas"; };

		/// \brief
		void onCompute( bool );

		/// \brief
		void init();

	};

}
}

#endif