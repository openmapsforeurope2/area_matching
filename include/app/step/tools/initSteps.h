#ifndef _APP_STEP_TOOLS_INITSTEPS_H_
#define _APP_STEP_TOOLS_INITSTEPS_H_

//EPG
#include <epg/step/StepSuite.h>
#include <epg/step/factoryNew.h>

//APP
#include <app/step/410_CutAreaWithBoundary.h>
#include <app/step/420_ClipAreaOutOfCountry.h>
#include <app/step/425_CleanRemoteAreas.h>
#include <app/step/430_MergeAreas.h>


namespace app{
namespace step{
namespace tools{

	template<  typename StepSuiteType >
	void initSteps( StepSuiteType& stepSuite )
	{
		stepSuite.addStep( epg::step::factoryNew< CutAreaWithBoundary >() );
		stepSuite.addStep( epg::step::factoryNew< ClipAreaOutOfCountry >() );
		stepSuite.addStep( epg::step::factoryNew< CleanRemoteAreas >() );
		stepSuite.addStep( epg::step::factoryNew< MergeAreas >() );
	}

}
}
}

#endif