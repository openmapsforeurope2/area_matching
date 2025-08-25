#ifndef _APP_STEP_CUTAREAWITHBOUNDARY_H_
#define _APP_STEP_CUTAREAWITHBOUNDARY_H_

#include <epg/step/StepBase.h>
#include <app/params/ThemeParameters.h>

namespace app{
namespace step{

	class CutAreaWithBoundary : public epg::step::StepBase< app::params::ThemeParametersS > {

	public:

		/// \brief
		int getCode() { return 410; };

		/// \brief
		std::string getName() { return "CutAreaWithBoundary"; };

		/// \brief
		void onCompute( bool );

		/// \brief
		void init();

	};

}
}

#endif