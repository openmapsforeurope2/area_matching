
//BOOST
#include <boost/program_options.hpp>

//EPG
#include <epg/Context.h>
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>
#include <epg/tools/TimeTools.h>
#include <epg/params/tools/loadParameters.h>

//OME2
#include <ome2/utils/setTableName.h>

//APP
#include <app/params/ThemeParameters.h>
#include <app/step/tools/initSteps.h>


namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    epg::Context* context = epg::ContextS::getInstance();
	std::string     logDirectory = "";
	std::string     epgParametersFile = "";
	std::string     themeParametersFile = "";
	std::string     stepCode = "";
	std::string     countryCode = "";
	bool            verbose = true;

	epg::step::StepSuite< app::params::ThemeParametersS > stepSuite;
    app::step::tools::initSteps(stepSuite);

	std::ostringstream OperatorDetail;
	OperatorDetail << "set step :" << std::endl
		<< stepSuite.toString();

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("c" , po::value< std::string >(&epgParametersFile)     , "conf file" )
        ("cc" , po::value< std::string >(&countryCode)          , "country code" )
		("sp", po::value< std::string >(&stepCode), OperatorDetail.str().c_str())
    ;
    stepCode = stepSuite.getStepsRange();

    //main log
    std::string     logFileName = "log.txt";
    std::ofstream   logFile( logFileName.c_str() ) ;

    logFile << "[START] " << epg::tools::TimeTools::getTime() << std::endl;

    int returnValue = 0;
    try{

        po::variables_map vm;
        int style = po::command_line_style::default_style & ~po::command_line_style::allow_guessing;
        po::store( po::parse_command_line( argc, argv, desc, style ), vm );
        po::notify( vm );    

        if ( vm.count( "help" ) ) {
            std::cout << desc << std::endl;
            return 1;
        }

        //parametres EPG
		context->loadEpgParameters( epgParametersFile );

        //Initialisation du log de prod
        logDirectory = context->getConfigParameters().getValue( LOG_DIRECTORY ).toString();

        //test si le dossier de log existe sinon le creer
        boost::filesystem::path logDir(logDirectory);
        if (!boost::filesystem::is_directory(logDir))
        {
            if (!boost::filesystem::create_directory(logDir))
            {
                std::string mError = "le dossier " + logDirectory + " ne peut être cree";
                IGN_THROW_EXCEPTION(mError);
            }
        }

        //repertoire de travail
        context->setLogDirectory( logDirectory );

		//theme parameters
		themeParametersFile = context->getConfigParameters().getValue(THEME_PARAMETER_FILE).toString();
		app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
        epg::params::tools::loadParams(*themeParameters, themeParametersFile, countryCode);

        //info de connection db
        context->loadEpgParameters( themeParameters->getValue(DB_CONF_FILE).toString() );

		//definition AREA_TABLE_INIT_CLEANED
		std::string codeStepCleaned = "";
		std::string suiteStepStg = stepSuite.toString();
		std::vector<std::string> vStepSuiteNumNom;
		epg::tools::StringTools::Split(suiteStepStg, "\n", vStepSuiteNumNom);
		for (size_t i = 0; i < vStepSuiteNumNom.size(); ++i) {
			if (vStepSuiteNumNom[i] == "")
				continue;
			std::vector<std::string> vStepSuiteNumNomPair;
			epg::tools::StringTools::Split(vStepSuiteNumNom[i], " ", vStepSuiteNumNomPair);
			if (vStepSuiteNumNomPair[1] == "CleanByLandmask") {
				codeStepCleaned = vStepSuiteNumNomPair[0];
				epg::tools::StringTools::trim(codeStepCleaned, "[");
				epg::tools::StringTools::trim(codeStepCleaned, "]");
				break;
			}
		}
		std::string areaTableNameInitCleaned="";
		if (codeStepCleaned != "")
			areaTableNameInitCleaned += "_" + codeStepCleaned + "_";
		areaTableNameInitCleaned += themeParameters->getParameter(AREA_TABLE_INIT).getValue().toString();
		themeParameters->setParameter(AREA_TABLE_INIT_CLEANED, ign::data::String(areaTableNameInitCleaned));

        //epg logger
        epg::log::EpgLogger* logger = epg::log::EpgLoggerS::getInstance();
        // logger->setProdOfstream( logDirectory+"/au_merging.log" );
        logger->setDevOfstream( context->getLogDirectory()+"/area_matching.log" );

        //shape logger
        epg::log::ShapeLogger* shapeLogger = epg::log::ShapeLoggerS::getInstance();
	    shapeLogger->setDataDirectory( context->getLogDirectory()+"/shape" );

        //set BDD search path
        context->getDataBaseManager().setSearchPath(themeParameters->getValue(WORKING_SCHEMA).toString());
        ome2::utils::setTableName<app::params::ThemeParametersS>(LANDMASK_TABLE);
        ome2::utils::setTableName<epg::params::EpgParametersS>(TARGET_BOUNDARY_TABLE);

        
		logger->log(epg::log::INFO, "[START HY MATCHING PROCESS ] " + epg::tools::TimeTools::getTime());
        
        //lancement du traitement
		stepSuite.run(stepCode, verbose);

		logger->log(epg::log::INFO, "[END HY MATCHING PROCESS ] " + epg::tools::TimeTools::getTime());
	

    }
    catch( ign::Exception &e )
    {
        std::cerr<< e.diagnostic() << std::endl;
        epg::log::EpgLoggerS::getInstance()->log( epg::log::ERROR, std::string(e.diagnostic()));
        logFile << e.diagnostic() << std::endl;
        returnValue = 1;
    }
    catch( std::exception &e )
    {
        std::cerr << e.what() << std::endl;
        epg::log::EpgLoggerS::getInstance()->log( epg::log::ERROR, std::string(e.what()));
        logFile << e.what() << std::endl;
        returnValue = 1;
    }

    logFile << "[END] " << epg::tools::TimeTools::getTime() << std::endl;

    epg::ContextS::kill();
    epg::log::EpgLoggerS::kill();
    epg::log::ShapeLoggerS::kill();
    app::params::ThemeParametersS::kill();
    
    logFile.close();

    return returnValue ;
}