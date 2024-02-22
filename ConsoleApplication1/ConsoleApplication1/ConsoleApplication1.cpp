
// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
//  Project -> Properties -> General               PlatformToolset: v141         (ASUS)   v120         (THINK_PC)
//  Project -> Properties -> C/C++ -> General      BoostInclude:    boost_1_81_0 (ASUS)   boost_1_55_0 (THINK_PC)
//  project:         c:\users\dad\source\repos\ConsoleApplication1 (ASUS) c:\users\dad\documents\Visual Studio 2013\Projects\ConsoleApplication1 (THINK_PC)
//

#include "stdafx.h"
#include "Header.h"
using namespace std;



#ifndef _MSC_VER
// not VC++
#elif _MSC_VER < 1900
// MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
int _tmain(int argc, WCHAR* argv[])   // was _TCHAR but seems fine now with WCHAR
#else
// Future versions
int _tmain(int argc, WCHAR* argv[])
#endif

{
	size_t numChars;
	try{
		// initialise
		map<string, string> argWords;
		argWords["doFAR"]         = "";
		argWords["bumpOnlyALL"    ]         = "";
		argWords["slidingTheta"]            = "";
		argWords["doTesting"]               = "";
		argWords["doDeltas"       ]         = "";
		argWords["notIllustrative"]         = "";
		argWords["hasISIN"]                 = "";
		argWords["hasInventory"]            = "";
		argWords["notStale"]                = "";
		argWords["incomeProducts"]          = "";
		argWords["capitalProducts"]         = "";
		argWords["ignoreBenchmark"]         = "";
		argWords["debug"]                   = "number";
		argWords["gmmMinClusterFraction"]   = "0.0 < 0.5";
		argWords["ajaxCalling"]             = "";
		argWords["useMyEqEqCorr"]           = "0|1";
		argWords["useMyEqFxCorr"]           = "0|1";
		argWords["forwardValueCoupons"]     = "0|1";
		argWords["barrierBendAmort"]        = "endFraction:numDays";
		argWords["silent"]                  = "";
		argWords["verbose"]                 = "";
		argWords["updateProduct"]           = "";
		argWords["priips"]                  = "";
		argWords["doAnyIdTable"]            = "";
		argWords["getMarketData"]           = "";
		argWords["proto"]                   = "";
		argWords["bsPricer"]                = "";
		argWords["forceLocalVol"]           = "";
		argWords["stochasticDrift"]         = "";
		argWords["dbServer"]                = "spCloud|newSp|spIPRL";
		argWords["forceIterations"]         = "";
		argWords["useProductFundingFractionFactor"] = "";
		argWords["showMatured"]             = "";
		argWords["historyStep"]             = "nnn";
		argWords["corrUserId"]              = "nnn";
		argWords["startDate"]               = "YYYY-mm-dd";
		argWords["spotsDate"]               = "YYYY-mm-dd";
		argWords["endDate"]                 = "YYYY-mm-dd";
		argWords["fvEndDate"]               = "YYYY-mm-dd";
		argWords["arcVolDate"]              = "YYYY-mm-dd";
		argWords["arcCorDate"]              = "YYYY-mm-dd";
		argWords["arcDivDate"]              = "YYYY-mm-dd";
		argWords["arcOnCurveDate"]          = "YYYY-mm-dd";
		argWords["arcCurveDate"]            = "YYYY-mm-dd";
		argWords["arcCdsDate"]              = "YYYY-mm-dd";
		argWords["bumpUserId"]              = "nnn";
		argWords["minSecsTaken"]            = "nnn";
		argWords["maxSecsTaken"]            = "nnn";
		argWords["userParameters"]          = "userId BUT cannot also have getMarketData (which is for FV calcs); will use impvol(not localvol),impdivyield for this userId";
		argWords["only"]                    = "<comma-sep list of underlyings names>";
		argWords["notOnly"]                 = "<comma-sep list of underlyings names>";
		argWords["possibleIssuerIds"]       = "<comma-sep list of issuerIds>";
		argWords["UKSPA"]                   = "Bear|Neutral|Bull";
		argWords["Issuer"]                  = "partName";
		argWords["targetPremium"]           = "x.x (percent)";
		argWords["useThisVol"]              = "x.x (percent)";
		argWords["useThisVolShift"]         = "x.x (percent)";
		argWords["fundingFractionFactor"]   = "x.x";
		argWords["forceFundingFraction"]    = "x.x";
		argWords["rescale"]                 = "spots|tba:fraction  eg spots:0.8 for a 20% fall in underlyings";
		argWords["useThisPrice"]            = "x.x";
		argWords["useThisOIS"]              = "x.x";
		argWords["useThisBarrierBend"]      = "x.x (percent)";
		argWords["planSelect"]              = "only|none";		
		argWords["eqFx"]                    = "eqUid:fxId:x.x   eg 3:1:-0.5";
		argWords["eqEq"]                    = "eqUid:eqUid:x.x  eg 3:1:-0.5";
		argWords["ulLevel"]                 = "name:level       eg UK100:6500";
		argWords["solveFor"]                = "targetFairValue:whatToSolveFor[:commit]  eg 98.0:coupon|putBarrier|lastCallCap|digital|positiveParticipation|positivePutParticipation|shortPutStrike|autocallTrigger and add :commit to save solution";
		argWords["stickySmile"]             = "";
		argWords["bump"]                    = "bumpType:startBump:stepSize:numBumps eg delta|vega|rho|credit|corr~name~name:-0.05:0.05:3 >";
		argWords["bumpVolPoint"]            = "tenor:strike:bumpAmount(decimal) eg 1.0:0.6:0.01 ";
		argWords["forOptimisation"]         = "init|all  'init' truncates tables OTHERWISE results are ADDED; 'all' ALSO saves underlyings";
		argWords["duration"]                = "<number ~ number, or just number(min)>";
		argWords["volatility"]              = "<number ~ number, or just number(min) PERCENT>";
		argWords["arithReturn"]             = "<number ~ number, or just number(min) PERCENT>";
		argWords["CAGR"]                    = "<number ~ number, or just number(min) PERCENT>";
		argWords["CAGRsharpe"]              = "<number ~ number, or just number(min)>";
		argWords["CAGRtoCVAR95loss"]        = "<number ~ number, or just number(min)>";
		argWords["couponReturn"]            = "<number ~ number, or just number(min) PERCENT>";
		argWords["tailReturn"]              = "<number ~ number, or just number(min) PERCENT>";


		if (argc < 3){ 
			std::cout << "Usage: startId stopId (or a comma-separated list) numIterations <optionalArguments:>";
			for (std::map<string,string>::iterator iter = argWords.begin(); iter != argWords.end(); ++iter) {
				std::cout << iter->first.c_str();
				if (iter->second != ""){
					std::cout << ":" << iter->second.c_str();
				};
				std::cout << "\n";
			}
			std::cout << endl;  
			exit(100);
		}
		


		int              historyStep = 1, minSecsTaken=0, maxSecsTaken=0,corrUserId=0;
		int              commaSepList   = strstr(WcharToChar(argv[1], &numChars),",") ? 1:0;
		int              userParametersId(0),startProductId, stopProductId, fxCorrelationUid(0), fxCorrelationOtherId(0), eqCorrelationUid(0), eqCorrelationOtherId(0), optimiseNumDays(0);
		int              bumpUserId(3),requesterNumIterations = argc > 3 - commaSepList ? _ttoi(argv[3 - commaSepList]) : 100;
		int              debugLevel(0),corrUidx(0), corrOtherUidx(0), corrOtherIndex(0), doUseMyEqEqCorr(-1), doUseMyEqFxCorr(-1);
		bool             doForwardValueCoupons(true),doTesting(false),doFinalAssetReturn(false), requesterForceIterations(false), doDebug(false), getMarketData(false), notStale(false), hasISIN(false), hasInventory(false), notIllustrative(false), onlyTheseUls(false), forceEqFxCorr(false), forceEqEqCorr(false);
		bool             doUseTargetPremium(false),doUseThisVol(false), doUseThisVolShift(false), doUseThisBarrierBend(false), doUseThisOIS(false), doUseThisPrice(false), showMatured(false), doBumps(false), doDeltas(false), doPriips(false), ovveridePriipsStartDate(false), doUKSPA(false), doAnyIdTable(false);
		bool             doRescale(false), doRescaleSpots(false), doBarrierBendAmort(true) /* lets try it */, doStickySmile(false), useProductFundingFractionFactor(false), forOptimisation(false), saveOptimisationPaths(false), initOptimisation(false), silent(false), updateProduct(false),verbose(false), doIncomeProducts(false), doCapitalProducts(false), solveFor(false), solveForCommit(false);
		bool             bsPricer(false),forceLocalVol(false),localVol(true), stochasticDrift(false), ignoreBenchmark(false), done, forceFullPriceRecord(false), fullyProtected, firstTime, forceUlLevels(false),corrsAreEqEq(true);
		bool             bumpOnlyALL(false),doFvEndDate(false),updateCashflows(true),ajaxCalling(false),slidingTheta(false),cmdLineBarrierBend(false);
		
		bool             bumpEachUnderlying(false);
		char             lineBuffer[MAX_SP_BUF], charBuffer[10000];
		char             onlyTheseUlsBuffer[1000] = "";
		char             startDate[11]            = "";
		char             spotsDate[11]            = "";
		char             endDate[11]              = "";
		char             fvEndDate[11]            = "";
		char             arcVolDate[11]           = "";
		char             arcVolDateString[50]     = "";
		char             arcCorDate[11]           = "";
		char             arcCorDateString[50]     = "";
		char             arcDivDate[11]           = "";
		char             arcDivDateString[50]     = "";
		char             arcOnCurveDate[11]       = "";
		char             arcOnCurveDateString[50] = "";
		char             arcCurveDate[11]         = "";
		char             arcCurveDateString[50]   = "";
		char             arcCdsDate[11]           = "";
		char             arcCdsDateString[50]     = "";
		char             useProto[6]              = "";
		char             priipsStartDatePhrase[100];
		double           fundingFractionFactor    = MIN_FUNDING_FRACTION_FACTOR, forceEqFxCorrelation(0.0), forceEqEqCorrelation(0.0);
		double           targetPremium,useThisVol,gmmMinClusterFraction(0.001),useThisVolShift,rescaleFraction,useThisBarrierBend,useThisOIS,targetFairValue,useThisPrice,thisFairValue;
		double           deltaBumpAmount(0.05), deltaBumpStart(0.0), deltaBumpStep(0.0), vegaBumpStart(0.0), vegaBumpStep(0.0);
		int              thetaBumpStart(0), thetaBumpStep(0);
		double           rhoBumpStart(0.0), rhoBumpStep(0.0), creditBumpStart(0.0), creditBumpStep(0.0), corrBumpStart(0.0), corrBumpStep(0.0), barrierBendEndFraction(0.0), barrierBendDays(90.0);
		double           bumpPointTenor(0.0), bumpPointStrike(0.0), bumpPointAmount(0.0);
		int              optimiseNumUls(0), deltaBumps(1), vegaBumps(1), thetaBumps(1), rhoBumps(1), creditBumps(1), corrBumps(1), solveForThis(0);
		boost::gregorian::date lastDate;
		string           thisCommandLine,anyString, ukspaCase(""), rescaleType(""), issuerPartName(""), forceFundingFraction(""), planSelect(""), whatToSolveFor(""), lastOptimiseDate;
		map<char, int>   avgTenor; avgTenor['d'] = 1; avgTenor['w'] = 7; avgTenor['m'] = 30; avgTenor['q'] = 91; avgTenor['s'] = 182; avgTenor['y'] = 365;
		map<string, int> bumpIds; bumpIds["delta"] = 1; bumpIds["vega"] = 2; bumpIds["theta"] = 3; bumpIds["rho"] = 4; bumpIds["credit"] = 5; bumpIds["corr"] = 6;
		map<string, double> ulLevels;      // name:level
		map<string,string>  analysisTypes; // projectedReturn, name
		analysisTypes["0"] = "historical"; analysisTypes["8"] = "PRIIPs"; analysisTypes["10"] = "UKSPA_Bear"; analysisTypes["20"] = "UKSPA_Neutral"; analysisTypes["30"] = "UKSPA_Bull"; analysisTypes["40"] = "FV"; analysisTypes["50"] = "userDefined"; analysisTypes["100"] = "stress";
		char dbServer[100]; strcpy(dbServer, "newSp");  // on local PC: newSp for local, spIPRL for IXshared        on IXcloud: spCloud
		vector<string>   rangeFilterStrings,corrNames;
		vector<string>   rescaleTypes; rescaleTypes.push_back("spots");
		vector<int>      possibleIssuerIds;
		const int        maxUls(MAX_ULS);
		const int        bufSize(1000);

		RETCODE          retcode;
		SomeCurve        anyCurve;
		time_t           startTime = time(0);
		char             **szAllPrices = new char*[maxUls];
		vector<int>      corrIds,optimiseUids,allProductIds; allProductIds.reserve(1000);
		vector<int>::iterator intIterator, intIterator1;
		for (int i = 0; i < maxUls; i++){
			szAllPrices[i] = new char[bufSize];
		}
		srand((unsigned int)time(0)); // reseed rand

		// build thisCommaneLine
		for (int i=1; i < argc; i++){
			thisCommandLine  = thisCommandLine + WcharToChar(argv[i], &numChars) + ' ';
		}

		// open database
		done = false;
		for(int i=4 - commaSepList; i < argc && !done; i++){
			char *thisArg  = WcharToChar(argv[i], &numChars);
			if (sscanf(thisArg, "dbServer:%s", lineBuffer)){ 
				strcpy(dbServer, lineBuffer); 
				done = true;
			}
		}
		MyDB  mydb(thisCommandLine, (char **)szAllPrices, dbServer), mydb1(thisCommandLine,(char **)szAllPrices, dbServer);

		// some inits from db
		vector<string>   payoffType;  payoffType.push_back("");
		sprintf(lineBuffer, "%s", "select name from payofftype order by PayoffTypeId");
		mydb.prepare((SQLCHAR *)lineBuffer, 1);
		retcode = mydb.fetch(false, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; exit(1); }
		while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
			payoffType.push_back(szAllPrices[0]);
			retcode = mydb.fetch(false, "");
		}



		// build list of productIds
		if (!commaSepList == 1) {
			startProductId = argc > 1 ? _ttoi(argv[1]) : 363;
			stopProductId  = argc > 2 ? _ttoi(argv[2]) : 363;
		}
		// check for non-existent/mis-typed args 
		// ... DOME
				
		for (int i=4 - commaSepList; i < argc; i++){
			char *thisArg  = WcharToChar(argv[i], &numChars);
			strcpy(charBuffer,thisArg);
			char *token = std::strtok(charBuffer, ":");
			if (argWords.find(token) == argWords.end()){
				std::cerr << "IPRerror:Unknown argument: " << thisArg << endl;
				exit(101);
			}
		}
		
		// process optional argumants
		// parse range strings, of the form <name>:<number or number-number>
		// commandLineName: sql to select corresponding quantity from cashflows table
		map<string, string> rangeVerbs;  // key:sql
		rangeVerbs["duration"] = "duration";
		rangeVerbs["volatility"] = "100*EsVol*sqrt(duration)";
		rangeVerbs["arithReturn"] = "100*EarithReturn";
		rangeVerbs["CAGR"] = "100*ExpectedReturn";
		rangeVerbs["CAGRsharpe"] = "ExpectedReturn/(EsVol*sqrt(duration))";
		rangeVerbs["tailReturn"] = "eShortfall";
		// give FairValue precedence over BidAsk if we will run the IPR fairValue simulator
		sprintf(charBuffer, "%s%s", "ExpectedReturn/(1.0 - IssuePrice*EShortfallTest/100/",
			getMarketData ? "if(FairValueDate != LastDataDate,if((BidAskDate != LastDataDate) or StalePrice,IssuePrice,Ask),FairValue))"
			: "if((BidAskDate   != LastDataDate) or StalePrice,if(FairValueDate != LastDataDate,IssuePrice,FairValue),Ask))");
		rangeVerbs["CAGRtoCVAR95loss"] = charBuffer;
		rangeVerbs["couponReturn"] = "100*couponReturn";

		for (int i=4 - commaSepList; i<argc; i++){
			char *thisArg  = WcharToChar(argv[i], &numChars);
			if (strstr(thisArg, "forceIterations")){ requesterForceIterations = true; }
			else if (strstr(thisArg, "priips"            )){ doPriips           = true; }
			else if (strstr(thisArg, "useProductFundingFractionFactor")){  useProductFundingFractionFactor  = true; }
			else if (strstr(thisArg, "getMarketData"     )){ getMarketData      = true; }
			else if (strstr(thisArg, "bsPricer"          )){ bsPricer = true;  localVol           = false; }
			else if (strstr(thisArg, "forceLocalVol"     )){ forceLocalVol      = true; }
			// else if (strstr(thisArg, "proto"             )){ strcpy(useProto,"proto"); }
			else if (strstr(thisArg, "stochasticDrift"   )){ stochasticDrift    = true; }
			else if (strstr(thisArg, "doFAR"             )){ doFinalAssetReturn = true; }
			else if (strstr(thisArg, "bumpOnlyALL"       )){ bumpOnlyALL        = true; bumpEachUnderlying = false; }
			else if (strstr(thisArg, "slidingTheta"      )){ slidingTheta       = true; }
			else if (strstr(thisArg, "doTesting"         )){ doTesting          = true; }
			else if (strstr(thisArg, "doAnyIdTable"      )){ doAnyIdTable       = true; }
			else if (strstr(thisArg, "silent"            )){ silent             = true; }
			else if (strstr(thisArg, "verbose"           )){ verbose            = true; }
			else if (strstr(thisArg, "updateProduct"     )){ updateProduct      = true; }
			else if (strstr(thisArg, "notIllustrative"   )){ notIllustrative    = true; }
			else if (strstr(thisArg, "hasISIN"           )){ hasISIN            = true; }
			else if (strstr(thisArg, "hasInventory"      )){ hasInventory       = true; }
			else if (strstr(thisArg, "showMatured"       )){ showMatured        = true; }
			else if (strstr(thisArg, "notStale"          )){ notStale           = true; }
			else if (strstr(thisArg, "incomeProducts"    )){ doIncomeProducts   = true; }
			else if (strstr(thisArg, "capitalProducts"   )){ doCapitalProducts  = true; }
			else if (strstr(thisArg, "ignoreBenchmark"   )){ ignoreBenchmark    = true; }
			else if (strstr(thisArg, "stickySmile"       )){ doStickySmile      = true; }
			else if (strstr(thisArg, "forOptimisation"   )){ forOptimisation    = true; }
			else if (strstr(thisArg, "ajaxCalling"       )){ ajaxCalling        = true; }


			// parse range strings, of the form <name>:<number or number-number>
			// commandLineName: sql to select corresponding quantity from cashflows table
			strcpy(charBuffer, thisArg);
			char *thisVerb = std::strtok(charBuffer, ":");
			if (rangeVerbs.find(thisVerb) != rangeVerbs.end()){
				char *arg = std::strtok(NULL, ":");
				strcpy(lineBuffer, arg);
				// number-number, or just number(min)
				char *token = std::strtok(lineBuffer, "~");
				std::vector<std::string> tokens;
				while (token != NULL) { 
					tokens.push_back(token); token = std::strtok(NULL, "~"); 
				}
				int numTokens = (int)tokens.size();
				if (numTokens > 0){
					sprintf(lineBuffer, " and %s > %s", rangeVerbs[thisVerb].c_str(), tokens[0].c_str());
					if (numTokens > 1){
						sprintf(lineBuffer, "%s and %s < %s", lineBuffer, rangeVerbs[thisVerb].c_str(), tokens[1].c_str());
					}
					rangeFilterStrings.push_back(lineBuffer);
				}
			}
			if (strstr(thisArg, "doDeltas")){
				char *token = std::strtok(thisArg, ":");
				std::vector<std::string> tokens;
				while (token != NULL) { tokens.push_back(token); token = std::strtok(NULL, ":"); }
				if ((int)tokens.size() > 1){ 
					deltaBumpAmount = fabs(atof(tokens[1].c_str()));
				}
				getMarketData      = true;
				doDeltas           = true; 
				doBumps            = true;
				bumpEachUnderlying = !bumpOnlyALL;
				deltaBumpStart     = -deltaBumpAmount;
				deltaBumpStep      =  deltaBumpAmount;
				deltaBumps         = 3;
			}

			if (sscanf(thisArg, "planSelect:%s", lineBuffer)){
				if (strcmp(lineBuffer, "only") == 0 || strcmp(lineBuffer, "none") == 0){
					planSelect = lineBuffer;
			 	}
			}
			if (sscanf(thisArg, "eqFx:%s", lineBuffer)){
				forceEqFxCorr = true;
				char *token = std::strtok(lineBuffer, ":");
				std::vector<std::string> tokens;
				while (token != NULL) { tokens.push_back(token); token = std::strtok(NULL, ":"); }
				if ((int)tokens.size() != 3){ cerr << "eqFx: incorrect syntax" << endl; exit(102); }
				fxCorrelationUid        = atoi(tokens[0].c_str());
				fxCorrelationOtherId    = atoi(tokens[1].c_str());
				forceEqFxCorrelation    = atof(tokens[2].c_str());
			}
			if (sscanf(thisArg, "ulLevel:%s", lineBuffer)){
				forceUlLevels = true;
				char *token = std::strtok(lineBuffer, ":");
				std::vector<std::string> tokens;
				while (token != NULL) { tokens.push_back(token); token = std::strtok(NULL, ":"); }
				if ((int)tokens.size() != 2){ cerr << "ulLevel: incorrect syntax" << endl; exit(102); }
				ulLevels[tokens[0]] = atof(tokens[1].c_str());
			}
			if (sscanf(thisArg, "bumpVolPoint:%s", lineBuffer)){
				if (doDeltas){ cerr << "cannot do deltas and bumps together" << endl; exit(103); }
				doBumps            = true;
				getMarketData      = true;
				bumpEachUnderlying = !bumpOnlyALL;
				char *token   = std::strtok(lineBuffer, ":");
				std::vector<std::string> tokens;
				while (token != NULL) { tokens.push_back(token); token = std::strtok(NULL, ":"); }
				if ((int)tokens.size() != 3){ cerr << "bumpVolPoint: incorrect syntax" << endl; exit(104); }
				bumpPointTenor   = atof(tokens[0].c_str());
				bumpPointStrike  = atof(tokens[1].c_str());
				bumpPointAmount  = atof(tokens[2].c_str());
				if (bumpPointTenor  <= 0.0){ cerr << "bumpVolPoint: tenor  must be positive" << endl; exit(104); }
				if (bumpPointStrike <= 0.0){ cerr << "bumpVolPoint: strike must be positive" << endl; exit(104); }
			}
			if (sscanf(thisArg, "bump:%s", lineBuffer)){
				if (doDeltas){ cerr << "cannot do deltas and bumps together" << endl; exit(103); }
				doBumps       = true;
				getMarketData = true;
				char *token   = std::strtok(lineBuffer, ":");
				std::vector<std::string> tokens;
				while (token != NULL) { tokens.push_back(token); token = std::strtok(NULL, ":"); }
				if ((int)tokens.size() != 4){ cerr << "bump: incorrect syntax" << endl; exit(104); }
				double start, step;
				int     num;
				start = atof(tokens[1].c_str());
				step  = atof(tokens[2].c_str());
				num   = atoi(tokens[3].c_str());
				// check for correlation string
				size_t pos = 0;	
				const std::string corrKey("corr");
				if ((pos = tokens[0].find(corrKey)) != std::string::npos){
					std::string frag;
					tokens[0].erase(0, pos + corrKey.length() + 1);
					while ((pos = tokens[0].find("~")) != std::string::npos) {
						frag = tokens[0].substr(0, pos);
						corrNames.push_back(frag);
						tokens[0].erase(0, pos + 1);
					}
					corrNames.push_back(tokens[0]);
					if ((int)corrNames.size() != 2){ cerr << "bump: corr: incorrect syntax - need 2 names separated by '~' " << endl; exit(1041); }
					tokens[0] = corrKey;
					// find the correlation ids
					bool done=false;
					// are corrNames eq/eq
					sprintf(lineBuffer, "%s%s%s%s%s", "select c.UnderlyingId,c.OtherId from correlation c join underlying u1 using (UnderlyingId)  join underlying u2 on (c.OtherId=u2.UnderlyingId) where UserId=3 and u1.Name='", corrNames[0].c_str(), "' and u2.Name= '", corrNames[1].c_str(), "' and OtherIdIsCcy=0");
					if (mydb.prepare((SQLCHAR *)lineBuffer, 2)){ continue; }
					retcode = mydb.fetch(false, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; exit(1); }
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
						corrIds.push_back(atoi(szAllPrices[0]));
						corrIds.push_back(atoi(szAllPrices[1]));
						corrsAreEqEq = true;
						done         = true;
					}
					// are corrNames eq/fx
					else {
						sprintf(lineBuffer, "%s%s%s%s%s", "select c.UnderlyingId,c.OtherId from correlation c join underlying u1 using (UnderlyingId)  join currencies u2 on (c.OtherId=u2.CcyId) where UserId=3 and u1.Name='", corrNames[0].c_str(), "' and u2.Name= '", corrNames[1].c_str(), "' and OtherIdIsCcy=1");
						if (mydb.prepare((SQLCHAR *)lineBuffer, 2)){ continue; }
						retcode = mydb.fetch(false, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; exit(1); }
						if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
							corrIds.push_back(atoi(szAllPrices[0]));
							corrIds.push_back(atoi(szAllPrices[1]));
							corrsAreEqEq = false;
							done         = true;
						}
					}
					if (!done){ cerr << "bump: corr: cannot find eq/eq or fx/fx underlyings for " << corrNames[0].c_str() << " and " << corrNames[1].c_str() << endl; exit(1042); }
				}
				
				switch (bumpIds[tokens[0].c_str()]){
				case 1: // delta
					deltaBumpStart     = start;
					deltaBumpStep      = step;
					deltaBumps         = num;
					bumpEachUnderlying = !bumpOnlyALL;
					break;
				case 2: // vega
					vegaBumpStart      = start;
					vegaBumpStep       = step;
					vegaBumps          = num;
					bumpEachUnderlying = !bumpOnlyALL;
					break;
				case 3: // theta
					thetaBumpStart  = atoi(tokens[1].c_str());
					thetaBumpStep   = atoi(tokens[2].c_str());;
					thetaBumps      = num;
					break;
				case 4: // rho
					rhoBumpStart  = start;
					rhoBumpStep   = step;
					rhoBumps      = num;
					break;
				case 5: // credit
					creditBumpStart  = start;
					creditBumpStep   = step;
					creditBumps      = num;
					break;
				case 6: // correlation
					corrBumpStart  = start;
					corrBumpStep   = step;
					corrBumps      = num;
					break;
				}
			}
						
			if (sscanf(thisArg, "barrierBendAmort:%s", lineBuffer)){
				doBarrierBendAmort  = true;
				cmdLineBarrierBend  = true;
				getMarketData       = true;
				char *token         = std::strtok(lineBuffer, ":");
				std::vector<std::string> tokens;
				while (token != NULL) { tokens.push_back(token); token = std::strtok(NULL, ":"); }
				if ((int)tokens.size() != 2){ cerr << "barrierBendAmort: incorrect syntax" << endl; exit(104); }
				barrierBendEndFraction = atof(tokens[0].c_str());
				barrierBendDays        = atof(tokens[1].c_str());
				if (barrierBendEndFraction <0.0 || barrierBendEndFraction > 1.0){ cerr << "barrierBendAmort: first arg must be between 1.0 and 0.0" << endl; exit(104); }
				if (barrierBendDays <1.0){ cerr << "barrierBendAmort: second arg must be at least 1" << endl; exit(104); }
			}
			
			if (sscanf(thisArg, "rescale:%s", lineBuffer)){
				doRescale      = true;
				doRescaleSpots = true;
				char *token    = std::strtok(lineBuffer, ":");
				std::vector<std::string> tokens;
				while (token != NULL) { tokens.push_back(token); token = std::strtok(NULL, ":"); }
				if ((int)tokens.size() != 2){ cerr << "rescale: incorrect syntax" << endl; exit(104); }
				rescaleType       = tokens[0].c_str();
				rescaleFraction   = atof(tokens[1].c_str());
				std::vector<string>::iterator it = std::find(rescaleTypes.begin(), rescaleTypes.end(), rescaleType);
				if (it == rescaleTypes.end()){ cerr << "rescale: first arg not recognised"         << endl; exit(1041); }
				if (rescaleFraction <= 0.0)  { cerr << "rescale: second arg must be at least 0.0"  << endl; exit(1041); }
			}

			if (sscanf(thisArg, "solveFor:%s", lineBuffer)){
				solveFor      = true;
				getMarketData = true;
				char *token = std::strtok(lineBuffer, ":");
				std::vector<std::string> tokens;
				while (token != NULL) { tokens.push_back(token); token = std::strtok(NULL, ":"); }
				if (tokens.size() < 2){ cerr << "solveFor: incorrect syntax" << endl; exit(105); }
				targetFairValue   = atof(tokens[0].c_str());
				whatToSolveFor    = tokens[1];
				if (tokens.size() > 2 && tokens[2] == "commit"){ solveForCommit = true; }
				if (whatToSolveFor == "coupon"){
					solveForThis = solveForCoupon;
				}
				else if (whatToSolveFor == "autocallTrigger") {
					solveForThis = solveForAutocallTrigger;
				}
				else if (whatToSolveFor == "lastCallCap") {
					solveForThis = solveForLastCallCap;
				}
				else if (whatToSolveFor == "putBarrier"){
					solveForThis = solveForPutBarrier;
				}
				else if (whatToSolveFor == "digital") {
					solveForThis = solveForDigital;
				}
				else if (whatToSolveFor == "positiveParticipation") {
					solveForThis = solveForPositiveParticipation;
				}
				else if (whatToSolveFor == "positivePutParticipation") {
					solveForThis = solveForPositivePutParticipation;
				}
				else if (whatToSolveFor == "shortPutStrike") {
					solveForThis = solveForShortPutStrike;
				}
				else {
					cerr << "solveFor: incorrect solveFor" << endl; exit(105);
				}
			}
			if (sscanf(thisArg, "eqEq:%s", lineBuffer)){
				forceEqEqCorr = true;
				char *token = std::strtok(lineBuffer, ":");
				std::vector<std::string> tokens;
				while (token != NULL) { tokens.push_back(token); token = std::strtok(NULL, ":"); }
				if ((int)tokens.size() != 3){ cerr << "eqEq: incorrect syntax" << endl; exit(105); }
				eqCorrelationUid        = atoi(tokens[0].c_str());
				eqCorrelationOtherId    = atoi(tokens[1].c_str());
				forceEqEqCorrelation    = atof(tokens[2].c_str());
			}
			if (sscanf(thisArg, "only:%s", lineBuffer) || sscanf(thisArg, "notOnly:%s", lineBuffer)){
				bool notOnly = (bool)sscanf(thisArg, "notOnly:%s", lineBuffer);
				string  notOnlyStr = notOnly ? "" : "not";
				onlyTheseUls = true;
				char *token = std::strtok(lineBuffer, ",");
				std::vector<std::string> tokens;
				while (token != NULL) { tokens.push_back(token); token = std::strtok(NULL, ","); }
				strcpy(lineBuffer,"");
				for (int j=0; j < (int)tokens.size(); j++){
					sprintf(lineBuffer, "%s%s%s%s%s", lineBuffer, (j == 0 ? "" : ","), "'", tokens[j].c_str(), "'");
				}
				// to avoid large strings of productids, store them in anyid table
				/* better to create inline table 
				sprintf(charBuffer, "%s%s%s", 
					"CREATE TEMPORARY TABLE tempOnly ENGINE=MEMORY as (select productid from product where productid not in (select distinct pb.productid from productbarrier pb join barrierrelation br using (productbarrierid) join underlying u using (underlyingid) where u.name not in (",
					lineBuffer, ")))");
				mydb.prepare((SQLCHAR *)charBuffer, 1);
				*/
				
				// strcpy(onlyTheseUlsBuffer, " join tempOnly using (productid) ");
				sprintf(charBuffer, "%s%s%s%s%s",
					" join (select productid from product where productid not in (select distinct pb.productid from productbarrier pb join barrierrelation br using (productbarrierid) join underlying u using (underlyingid) where u.name ",
					notOnlyStr.c_str()," in (",
					lineBuffer, "))) z using (productid) ");
				sprintf(onlyTheseUlsBuffer, "%s%s", onlyTheseUlsBuffer, charBuffer);
			}

			if (sscanf(thisArg, "possibleIssuerIds:%s", lineBuffer)) {
				char *token = std::strtok(lineBuffer, ",");
				while (token != NULL) { possibleIssuerIds.push_back(atoi(token)); token = std::strtok(NULL, ","); }
			}
			if (sscanf(thisArg, "startDate:%s",  lineBuffer))  { strcpy(startDate, lineBuffer); }
			if (sscanf(thisArg, "debug:%s", lineBuffer))       { doDebug = true; debugLevel = atoi(lineBuffer); }
			if (sscanf(thisArg, "UKSPA:%s", lineBuffer))       {
				ukspaCase     = lineBuffer;
				doUKSPA       = ukspaCase != "";
				getMarketData = true;
			}
			if (sscanf(thisArg, "fvEndDate:%s", lineBuffer)) {
				doFvEndDate      = true;
				strcpy(endDate, lineBuffer);
			}
			if (sscanf(thisArg, "Issuer:%s", lineBuffer))                 { issuerPartName          = lineBuffer; }
			else if (sscanf(thisArg, "fundingFractionFactor:%s",   lineBuffer)){ fundingFractionFactor	= atof(lineBuffer);	}
			else if (sscanf(thisArg, "forceFundingFraction:%s",    lineBuffer)){ forceFundingFraction	= lineBuffer; }
			else if (sscanf(thisArg, "forOptimisation:%s", lineBuffer)) { forOptimisation    = true;
					if (strcmp("init",lineBuffer) == 0) { initOptimisation = true; }  
					if (strcmp("all", lineBuffer) == 0) { initOptimisation = true; saveOptimisationPaths = true; }
			}
			else if (sscanf(thisArg, "spotsDate:%s",             lineBuffer)){ strcpy(spotsDate, lineBuffer); }
			else if (sscanf(thisArg, "bumpUserId:%s",            lineBuffer)){ bumpUserId            = atoi(lineBuffer); }
			else if (sscanf(thisArg, "corrUserId:%s",            lineBuffer)){ corrUserId            = atoi(lineBuffer); }
			else if (sscanf(thisArg, "minSecsTaken:%s",          lineBuffer)){ minSecsTaken          = atoi(lineBuffer); }
			else if (sscanf(thisArg, "maxSecsTaken:%s",          lineBuffer)){ maxSecsTaken          = atoi(lineBuffer); }
			else if (sscanf(thisArg, "userParameters:%s",        lineBuffer)){ userParametersId      = atoi(lineBuffer); }
			else if (sscanf(thisArg, "historyStep:%s",           lineBuffer)){ historyStep           = atoi(lineBuffer); }
			else if (sscanf(thisArg, "useThisPrice:%s",          lineBuffer)){ useThisPrice          = atof(lineBuffer);         doUseThisPrice       = true; }
			else if (sscanf(thisArg, "forwardValueCoupons:%s",   lineBuffer)){ 
				doForwardValueCoupons = atoi(lineBuffer) == 1;}
			else if (sscanf(thisArg, "useThisOIS:%s",            lineBuffer)){ useThisOIS            = atof(lineBuffer);         doUseThisOIS         = true; }
			else if (sscanf(thisArg, "useThisBarrierBend:%s",    lineBuffer)){ useThisBarrierBend    = atof(lineBuffer);         doUseThisBarrierBend = true; }
			else if (sscanf(thisArg, "useThisVol:%s",            lineBuffer)){ useThisVol            = atof(lineBuffer) / 100.0; doUseThisVol         = true; }
			else if (sscanf(thisArg, "targetPremium:%s",         lineBuffer)){ targetPremium         = atof(lineBuffer) / 100.0; doUseTargetPremium   = true; }
			else if (sscanf(thisArg, "useThisVolShift:%s",       lineBuffer)){ useThisVolShift       = atof(lineBuffer) / 100.0; doUseThisVolShift    = true; }
			else if (sscanf(thisArg, "useMyEqEqCorr:%s",         lineBuffer)){ doUseMyEqEqCorr       = atoi(lineBuffer); }
			else if (sscanf(thisArg, "useMyEqFxCorr:%s",         lineBuffer)){ doUseMyEqFxCorr       = atoi(lineBuffer); }
			else if (sscanf(thisArg, "gmmMinClusterFraction:%s", lineBuffer)){ gmmMinClusterFraction = atof(lineBuffer); gmmMinClusterFraction = fMax(0.0, fMin(gmmMinClusterFraction, 0.5)); }

			if (doFvEndDate || sscanf(thisArg, "endDate:%s", endDate)) { strcpy(endDate, endDate); }
			if (doFvEndDate || sscanf(thisArg, "arcVolDate:%s", endDate)) { strcpy(arcVolDate, endDate); sprintf(arcVolDateString, "%s%s%s", " and LastDataDate='", arcVolDate, "' "); }
			if (doFvEndDate || sscanf(thisArg, "arcCorDate:%s", endDate)) { strcpy(arcCorDate, endDate); sprintf(arcCorDateString, "%s%s%s", " and LastDataDate='", arcCorDate, "' "); }
			if (doFvEndDate || sscanf(thisArg, "arcDivDate:%s", endDate)) { strcpy(arcDivDate, endDate); sprintf(arcDivDateString, "%s%s%s", " and LastDataDate='", arcDivDate, "' "); }
			if (doFvEndDate || sscanf(thisArg, "arcOnCurveDate:%s", endDate)) { strcpy(arcOnCurveDate, endDate); sprintf(arcOnCurveDateString, "%s%s%s", " and LastDataDate='", arcOnCurveDate, "' "); }
			if (doFvEndDate || sscanf(thisArg, "arcCurveDate:%s", endDate)) { strcpy(arcCurveDate, endDate); sprintf(arcCurveDateString, "%s%s%s", " and LastDataDate='", arcCurveDate, "' "); }
			if (doFvEndDate || sscanf(thisArg, "arcCdsDate:%s", endDate)) { strcpy(arcCdsDate, endDate); sprintf(arcCdsDateString, "%s%s%s", " and LastDataDate='", arcCdsDate, "' "); }
		}
		updateCashflows  = doFvEndDate && !userParametersId ? false : true;   // allow userParameters to update its own cashflows
		if (doPriips){
			if (strlen(startDate)){
				ovveridePriipsStartDate = true;
				cout << "Will ovveride PRIIPs start date as you have entered a startDate" << endl;
			}
		}
		if (doDebug){
			FILE * pFile;
			pFile = fopen("debug.txt", "w");
			fclose(pFile);
		}
		if (getMarketData && userParametersId>0){
			cout << "getMarketData analysis cannot be run with userParameters" << endl;
			exit(106);
		}


		// get list of productIds
		// ... but first deal with any optimisation demands
		//     ... first element of allProductIds is special ProductId=1 which has chosen underlyings to simulate: ONLY products with these underlyings can be optimised
		if (forOptimisation){
			if (!getMarketData && userParametersId <1){
				cout << "Optimiser needs scenario-generation from either getMarketData or userParameters" << endl;
				exit(106);
			}
			allProductIds.push_back(1);  // special product id=1 with UK100,SX5E,SPX etc chosen underlyings
			notIllustrative = !commaSepList;
		}

		if (commaSepList == 1) {
			sprintf(charBuffer, "%s%s%s", " where p.ProductId in (", WcharToChar(argv[1], &numChars),") ");
		}
		else if (doAnyIdTable){
			sprintf(charBuffer, "%s", " join anyid a on (p.ProductId=a.id) ");
		} else{
			sprintf(charBuffer, "%s%d%s%d%s", " where p.ProductId >= '", startProductId, "' and p.ProductId <= '", stopProductId, "'");
		}
		sprintf(lineBuffer, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s", "select distinct p.ProductId from ", useProto, "product p join ", useProto,
			"cashflows c using (ProductId) join wrappertype w using (wrappertypeid) join producttype pt using (ProductTypeId) join institution i on (p.counterpartyid=i.institutionid) ",
			(onlyTheseUls      ? onlyTheseUlsBuffer : ""),
			charBuffer,
			(notIllustrative   ? " and Illustrative=0 " : ""),
			showMatured        ? "" : " and Matured=0 ",
			(hasISIN           ? " and ISIN != '' " : ""),
			(hasInventory      ? " and p.Inventory > 0 " : ""),
			(notStale          ? " and StalePrice=0 " : ""),
			(doIncomeProducts  ? " and pt.name like '%income%' " : ""),
			(doCapitalProducts ? " and pt.name not like '%income%' " : "")
			);
		for (int i=0; i < (int)rangeFilterStrings.size(); i++){
			sprintf(lineBuffer, "%s%s", lineBuffer, rangeFilterStrings[i].c_str());
		}
		sprintf(lineBuffer, "%s%s%lf%s", lineBuffer, " and ProjectedReturn=", (int)rangeFilterStrings.size()>0 && userParametersId>0 ? 0.5 : 1.0," ");
		
		if (minSecsTaken){
			sprintf(lineBuffer, "%s%s%d",lineBuffer, " and SecsTaken>=", minSecsTaken);
		}
		if (planSelect != ""){
			sprintf(lineBuffer, "%s%s%s%s", lineBuffer, " and w.Name", planSelect == "only" ? "" : "!","='Plan' ");
		}
		if (maxSecsTaken){
			sprintf(lineBuffer, "%s%s%d%s", lineBuffer, " and (WhenEvaluated is null or SecsTaken<=", maxSecsTaken,")");
		}
		if (issuerPartName != ""){
			sprintf(lineBuffer, "%s%s%s%s", lineBuffer, " and i.name like '%", issuerPartName.c_str(), "%'");
		}
		if (forOptimisation){
			sprintf(lineBuffer, "%s%s", lineBuffer, " and i.name != 'Markets'");
		}

		sprintf(lineBuffer, "%s%s", lineBuffer, " order by productid");
		mydb.prepare((SQLCHAR *)lineBuffer, 1); 	retcode = mydb.fetch(true, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; exit(1); }
		while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			int x(atoi(szAllPrices[0])); allProductIds.push_back(x);
			retcode = mydb.fetch(false,"");
		}
		int  numProducts = (int)allProductIds.size();
		bool multiIssuer = (int)possibleIssuerIds.size() > 0;

		// cerr << "Doing:" << allProductIds.size() << " products " << lineBuffer << endl;

		/*
		* getBenchmarkPerf
		*/
		double bmSwapRate(0.0);
		sprintf(lineBuffer, "%s", "SELECT avg(Rate)/100.0 sixYSwaps from curve where ccy='GBP' and Tenor in (5,7)");
		mydb.prepare((SQLCHAR *)lineBuffer, 1); 	retcode = mydb.fetch(true, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; exit(1); }
		if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			bmSwapRate = atof(szAllPrices[0]);			
		}
		double projectedReturn = (requesterNumIterations <= 1 ? 0.0 : (doPriips ? 0.08 : 1.0));

		double bmEarithReturn(0.0), bmVol(0.18);
		sprintf(lineBuffer, "%s%.4lf%s","SELECT EarithReturn ArithmeticReturn_pa, esVol*sqrt(duration) Volatility from cashflows where ProductId=71 and ProjectedReturn='",projectedReturn,"'");
		mydb.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb.fetch(true, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; exit(1); }
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			bmEarithReturn = atof(szAllPrices[0]);
			bmVol          = atof(szAllPrices[1]);
		}
	
		

		// deal with any optimisation demands
		//    ProductId=1                     contains all the underlyings that will be simulated ... typically just those we can FV
		//    productreturns table            will be populated with each product's simulated returns, payoffs
		//    simulatedunderlyings table      will be populated with the simulated levels for each underlying
		if (forOptimisation){
			// identify underlyings in special ProductId=1
			sprintf(lineBuffer, "%s%d%s", "select UnderlyingId from (select distinct underlyingid from barrierrelation join productbarrier using (productbarrierid) where productid in (",
				allProductIds[0], "))x");			
			mydb.prepare((SQLCHAR *)lineBuffer, 1); 	
			retcode = mydb.fetch(true, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; exit(1); }
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				optimiseUids.push_back(atoi(szAllPrices[0]));
				retcode = mydb.fetch(false, "");
			}
			optimiseNumUls     = (int)optimiseUids.size();
			// set lastOptimiseDate to max event date for requested products (except ProdutId=1)
			strcpy(charBuffer, "");
			for (int i=1; i < numProducts; i++){
				sprintf(charBuffer, "%s%s%d", charBuffer, i > 1 ? "," : "", allProductIds[i]);
			}
			sprintf(lineBuffer, "%s%s%s", "select max(greatest(settlementDate,EndDate)) from barrierrelation join productbarrier using (productbarrierid) where productid in (",
				charBuffer, ") and SettlementDate < date_add(now(),INTERVAL 12 YEAR)");  // limit to 12y to avoid blowing memory, and some 'Markets' products are deliberately set to start way-in-the-future
			mydb.prepare((SQLCHAR *)lineBuffer, 1);
			retcode = mydb.fetch(true, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; exit(1); }
			lastOptimiseDate = szAllPrices[0]; 
			boost::gregorian::date bLastOptimiseDate(boost::gregorian::from_simple_string(lastOptimiseDate));

			// set thisLastDate to endDate, failing which find last data date for all underlyings in requested products
			sprintf(lineBuffer, "%s%s%s", "select group_concat(underlyingid) from (select distinct underlyingid from barrierrelation join productbarrier using (productbarrierid) where productid in (",
				charBuffer, "))x");
			mydb.prepare((SQLCHAR *)lineBuffer, 1);
			retcode = mydb.fetch(true, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; exit(1); }
			string concatUlIds = szAllPrices[0];
			string thisLastDate; 
			if (strlen(endDate)){
				thisLastDate = endDate;
			}
			else{
				sprintf(lineBuffer, "%s%s%s", "select max(Date) from prices where underlyingid in (", concatUlIds.c_str(), ")");
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				retcode = mydb.fetch(true, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; exit(1); }
				thisLastDate = szAllPrices[0];
			}
			
			
			// calc #days of underlyings simulated levels we will need to store
			boost::gregorian::date bLastDataDate(boost::gregorian::from_simple_string(thisLastDate));
			boost::gregorian::date_duration dateDiff(bLastOptimiseDate - bLastDataDate);
			optimiseNumDays = dateDiff.days();

			// force generation of daily paths
			forceFullPriceRecord = true;

			// on SpecialProduct (id=1) reset SettlementDate,StartDate,EndDate to lastOptimiseDate 
			sprintf(charBuffer, "%s", lastOptimiseDate.c_str());
			sprintf(lineBuffer, "%s%s%s%s%s%s%s%s%s%s%s%s%d", "update barrierrelation join productbarrier using (productbarrierid) join product using (productid) set ",
				"FinalValuationDate='", charBuffer,
				"',MaturityDate='", charBuffer,
				"',SettlementDate='", charBuffer,
				"',StartDate='", charBuffer,
				"',EndDate='", charBuffer,
				"' where productid=",
				allProductIds[0]);
			mydb.prepare((SQLCHAR *)lineBuffer, 1);

			// clear productreturns and simulatedunderlyings tables
			if (initOptimisation) {
				sprintf(lineBuffer, "%s", "delete from productreturns");
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				sprintf(lineBuffer, "%s", "delete from simulatedunderlyings");
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
			}
		}


		
		// loop through each product
		std::vector<int> optimiseUlIdNameMap(1000);  // underlyingId -> arrayIndex, so ulIdNameMap[uid] gives the index into ulPrices vector
		std::vector<std::vector<std::vector<double>>> optimiseMcLevels(optimiseNumUls, std::vector<std::vector<double>>(optimiseNumDays));
		if (numProducts>1){ doUseThisPrice = false; }
		for (int productIndx = 0; productIndx < numProducts; productIndx++) {
			int              thisNumIterations = requesterNumIterations, numBarriers = 0, thisIteration = 0, compoIntoCcyUid = 0;
			int              i, j, k, len, len1, anyInt, numUl, numMonPoints,totalNumDays, totalNumReturns, uid, daysToFirstCapitalBarrier = 0;
			int              productId, anyTypeId, thisPayoffId, productShapeId, protectionLevelId,barrierRelationId;
			double           anyDouble, cds5y, maxBarrierDays, barrier, uBarrier, payoff, strike, cap, participation, fixedCoupon, AMC, issuePrice, bidPrice, askPrice, midPrice;
			double           compoIntoCcyStrikePrice(0.0), baseCcyReturn, benchmarkStrike, thisBarrierBendDays, thisBarrierBendFraction;
			string           thisProjectedReturn,productShape, protectionLevel, couponFrequency, productStartDateString, productCcy, word, word1, thisPayoffType, startDateString, endDateString, nature, settlementDate,
				description, avgInAlgebra, productTimepoints, productPercentiles,fairValueDateString,bidAskDateString,lastDataDateString;
			bool             hasCompoIntoCcy(false),useUserParams(false), productNeedsFullPriceRecord(false), capitalOrIncome, above, at;
			vector<int>      barrierMonDateIndx,volsMonDateIndx,monDateIndx, reportableMonDateIndx, accrualMonDateIndx;
			vector<double>   barrierMonDateT, volsMonDateT, monDateT, accrualMonDateT;
			vector<UlTimeseries>  ulOriginalPrices(maxUls), ulPrices(maxUls); // underlying prices	

			// init
			maxBarrierDays = 0.0;
			startTime      = time(0);
			productId      = allProductIds.at(productIndx);
			cds5y          = 0.0; // used to calc product excessReturn

			// get general info:  productType, productShape, barrierType
			// ...productType
			map<int, string> productTypeMap;
			sprintf(lineBuffer, "%s", "select * from producttype"); 	mydb.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb.fetch(true,lineBuffer);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				productTypeMap[atoi(szAllPrices[0])] = szAllPrices[1];
				retcode = mydb.fetch(false,"");
			}
			// ...productShape
			map<int, string> productShapeMap;
			sprintf(lineBuffer, "%s", "select * from productshape"); 	mydb.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb.fetch(true,lineBuffer);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				productShapeMap[atoi(szAllPrices[0])] = szAllPrices[1];
				retcode = mydb.fetch(false,"");
			}
			// ...protectionLevel
			map<int, string> protectionLevelMap;
			sprintf(lineBuffer, "%s", "select * from protectionLevel"); 	mydb.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb.fetch(true, lineBuffer);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				protectionLevelMap[atoi(szAllPrices[0])] = szAllPrices[1];
				retcode = mydb.fetch(false, "");
			}


			// ...barrierType
			map<int, string> barrierTypeMap;
			sprintf(lineBuffer, "%s", "select * from barriertype"); 	mydb.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb.fetch(true,lineBuffer);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				barrierTypeMap[atoi(szAllPrices[0])] = szAllPrices[1];
				retcode = mydb.fetch(false,"");
			}

			// get product table data
			enum {
				colProductCounterpartyId = 2, colProtectionLevelId, colProductStrikeDate = 6, colProductCcy = 14, colProductUserId = 26, colProductFixedCoupon = 28, colProductFrequency, colProductBid, colProductAsk, colProductBidAskDate,
				colProductAMC = 43, colProductShapeId,colProductMaxIterations=55, colProductDepositGtee, colProductDealCheckerId, colProductAssetTypeId, colProductIssuePrice, 
				colProductCouponPaidOut, colProductCollateralised, colProductCurrencyStruck, colProductBenchmarkId, colProductHurdleReturn, colProductBenchmarkTER,
				colProductTimepoints, colProductPercentiles, colProductDoTimepoints, colProductDoPaths, colProductStalePrice, colProductFairValue, 
				colProductFairValueDate, colProductFundingFraction, colProductDefaultFundingFraction, colProductUseUserParams, colProductForceStartDate, colProductBaseCcy, 
				colProductFundingFractionFactor, colProductBenchmarkStrike, colProductBootstrapStride, colProductSettleDays, colProductBarrierBend, colProductCompoIntoCcy, colProductBarrierBendDays, colProductBarrierBendFraction, 
				colProductIssuerCallable, colProductVolShift, colProductUseMyEqEqCorr, colProductUseMyEqFxCorr, colProductLast
			};
			sprintf(lineBuffer, "%s%s%s%d%s", "select * from ", useProto, "product where ProductId='", productId, "'");
			mydb.prepare((SQLCHAR *)lineBuffer, colProductLast);
			retcode = mydb.fetch(true, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; continue; }
			int  productMaxIterations  = atoi(szAllPrices[colProductMaxIterations]);
			bool forceIterations = requesterForceIterations;
			if (productMaxIterations < thisNumIterations && !forceIterations){
				thisNumIterations = productMaxIterations; 
				forceIterations = true;
			}
			if (thisNumIterations<1) { thisNumIterations = 1; }
			int  counterpartyId     = atoi(szAllPrices[colProductCounterpartyId]);
			int  productUserId      = atoi(szAllPrices[colProductUserId]);
			int  userId             = getMarketData ? 3 : userParametersId > 0 ? userParametersId : atoi(szAllPrices[colProductUserId]);
			bool depositGteed       = atoi(szAllPrices[colProductDepositGtee]) == 1;
			bool couponPaidOut      = atoi(szAllPrices[colProductCouponPaidOut] ) == 1;
			bool collateralised     = atoi(szAllPrices[colProductCollateralised]) == 1;
			bool currencyStruck     = atoi(szAllPrices[colProductCurrencyStruck]) == 1;
			bool doTimepoints       = atoi(szAllPrices[colProductDoTimepoints]  ) == 1;
			bool doPaths            = atoi(szAllPrices[colProductDoPaths]       ) == 1;
			if ((doPaths || doTimepoints) && (numProducts > 1)){
				doTimepoints = false;
				doPaths      = false;
				cout << "We only doTimepoints/paths if #products is 1 ... if you need timepoints/paths please do each product singly as it can overburden the database..thanks" << endl;
			};
			bool stalePrice               = atoi(szAllPrices[colProductStalePrice]) == 1;
			double fairValuePrice         = atof(szAllPrices[colProductFairValue]);
			fairValueDateString           = szAllPrices[colProductFairValueDate];
			bidAskDateString              = szAllPrices[colProductBidAskDate];
			int  benchmarkId              = ignoreBenchmark ? 0 : atoi(szAllPrices[colProductBenchmarkId]);
			benchmarkStrike               = benchmarkId > 0 ? atof(szAllPrices[colProductBenchmarkStrike]) : 0.0;
			if (benchmarkId != 0 && (doPriips || (!doUKSPA && getMarketData))){ benchmarkId = 0; } // do not need (possibly-not-market-data-tracked) benchmark for a fairvalue calc
			double hurdleReturn           = atof(szAllPrices[colProductHurdleReturn])/100.0;
			double contBenchmarkTER       = -log(1.0 - atof(szAllPrices[colProductBenchmarkTER]) / 100.0);
			double fundingFraction        = atof(szAllPrices[colProductFundingFraction]);
			double defaultFundingFraction = atof(szAllPrices[colProductDefaultFundingFraction]);
			int    bootstrapStride        = atoi(szAllPrices[colProductBootstrapStride]);
			int    settleDays             = atoi(szAllPrices[colProductSettleDays]);
			double barrierBend            = atof(szAllPrices[colProductBarrierBend])  * (getMarketData && !doUKSPA /* && !doBumps && !doDeltas */ ? 1.0 : 0.0);
			if (doUseThisBarrierBend){ barrierBend = useThisBarrierBend / 100.0;  }
			string compoIntoCcy             = szAllPrices[colProductCompoIntoCcy];
			double forceBarrierBendDays     = atoi(szAllPrices[colProductBarrierBendDays]);
			double forceBarrierBendFraction = atof(szAllPrices[colProductBarrierBendFraction]);
			bool   issuerCallable           = atoi(szAllPrices[colProductIssuerCallable]) == 1;
			if (issuerCallable && getMarketData && thisNumIterations > 1 && thisNumIterations < 100000 && !doDebug) { thisNumIterations = 100000; } // callables need 20k burnin
			bool   useMyEqEqCorr            = ajaxCalling &&  (doUseMyEqEqCorr == 1 || (doUseMyEqEqCorr != 0 && atoi(szAllPrices[colProductUseMyEqEqCorr]) == 1)) ;
			bool   useMyEqFxCorr            = ajaxCalling &&  (doUseMyEqFxCorr == 1 || (doUseMyEqFxCorr != 0 && atoi(szAllPrices[colProductUseMyEqFxCorr]) == 1));
			double volShift                 = doUseThisVolShift ? useThisVolShift : atof(szAllPrices[colProductVolShift]);
			
			if (forceBarrierBendDays>0 && !cmdLineBarrierBend){  // does not overrise commandLine
				thisBarrierBendFraction    = forceBarrierBendFraction;
				thisBarrierBendDays        = forceBarrierBendDays;
			}
			else{
				thisBarrierBendFraction    = barrierBendEndFraction;
				thisBarrierBendDays        = barrierBendDays;
			}
			if (thisBarrierBendFraction <0.0 || thisBarrierBendFraction > 1.0){ cerr << "barrierBendAmort: first arg must be between 1.0 and 0.0" << endl; exit(104); }
			if (thisBarrierBendDays <1.0){ cerr << "barrierBendAmort: second arg must be at least 1" << endl; exit(104); }

			useUserParams                 = userParametersId > 0 ? true : atoi(szAllPrices[colProductUseUserParams]) == 1;
			string forceStartDate         = szAllPrices[colProductForceStartDate];
			if ( useProductFundingFractionFactor){
				fundingFraction = defaultFundingFraction*atof(szAllPrices[colProductFundingFractionFactor]);;
			}
			if (fundingFractionFactor > MIN_FUNDING_FRACTION_FACTOR){
				fundingFraction = defaultFundingFraction*fundingFractionFactor;
			}
			if (depositGteed) {
				fundingFraction = 0.0;
			}
			if (forceFundingFraction != ""){
				fundingFraction = atof(forceFundingFraction.c_str());
			}
			productStartDateString  = szAllPrices[colProductStrikeDate];
			productCcy              = szAllPrices[colProductCcy];
			std::transform(std::begin(productCcy), std::end(productCcy), std::begin(productCcy), ::toupper);
			fixedCoupon             = atof(szAllPrices[colProductFixedCoupon]);
			bidPrice                = atof(szAllPrices[colProductBid]);
			askPrice                = atof(szAllPrices[colProductAsk]);
			AMC                     = atof(szAllPrices[colProductAMC]);
			productShapeId          = atoi(szAllPrices[colProductShapeId]);
			productShape			= productShapeMap[productShapeId];
			protectionLevelId       = atoi(szAllPrices[colProtectionLevelId]);
			protectionLevel			= protectionLevelMap[protectionLevelId];
			fullyProtected          = protectionLevel.find("Full") != std::string::npos;
			// assemble timepoints request
			productTimepoints		= szAllPrices[colProductTimepoints];
			vector<string> timepoints;
			vector<int>    tempTimepointDays;
			splitCommaSepName(timepoints, productTimepoints);
			if ((int)timepoints.size() > 0) {
				for (i=0, len=(int)timepoints.size(); i<len; i++){
					len1               = (int)timepoints[i].size() - 1;
					char tenorChar     = tolower(timepoints[i].at(len1));
					string tenorLength = timepoints[i].substr(0, len1);
					anyInt = atoi(tenorLength.c_str()) * avgTenor[tenorChar];
					if (std::find(tempTimepointDays.begin(), tempTimepointDays.end(), anyInt) == tempTimepointDays.end()) { tempTimepointDays.push_back(anyInt); }
				}
			}

			// assemble percentile request
			productPercentiles		= szAllPrices[colProductPercentiles];
			vector<string> uPercentiles;
			vector<double> simPercentiles;
			splitCommaSepName(uPercentiles, productPercentiles);
			if ((int)uPercentiles.size() > 0) {
				for (i=0, len=(int)uPercentiles.size(); i<len; i++){
					anyDouble = atof(uPercentiles[i].c_str())/100.0;
					if (std::find(simPercentiles.begin(), simPercentiles.end(), anyDouble) == simPercentiles.end()) { simPercentiles.push_back(anyDouble); }
				}
			}
			
			if (doTimepoints && (int)simPercentiles.size() > 0 && (int)tempTimepointDays.size() > 0){
				if (std::find(simPercentiles.begin(), simPercentiles.end(), 0.0)   == simPercentiles.end())  simPercentiles.push_back(0.0);
				if (std::find(simPercentiles.begin(), simPercentiles.end(), 0.999) == simPercentiles.end())  simPercentiles.push_back(0.999);
				sort(simPercentiles.begin(), simPercentiles.end());
			}


			issuePrice              = atof(szAllPrices[colProductIssuePrice]);
			// clean bid,ask
			if (bidPrice <= 0.0 && askPrice > 0.0){ bidPrice=askPrice; }
			if (bidPrice > 0.0  && askPrice <= 0.0){ askPrice=bidPrice; }
			if (strlen(szAllPrices[colProductFrequency])){ 
				couponFrequency = szAllPrices[colProductFrequency]; 
				if (couponFrequency.length() == 1){
					couponFrequency = "1" + couponFrequency;
				}
			}
			boost::gregorian::date  bProductStartDate(boost::gregorian::from_simple_string(productStartDateString));

			// monitoring info
			//  ... thisProjectedReturn is NOT USED - just for info on which type of analysis is being done
			thisProjectedReturn = (thisNumIterations == 1 ? "0" : (doPriips ? "8" : "100"));
			if      (ukspaCase == "Bear")         { thisProjectedReturn = "10"; }
			else if (ukspaCase == "Neutral")      { thisProjectedReturn = "20"; }
			else if (ukspaCase == "Bull")         { thisProjectedReturn = "30"; }
			if (getMarketData && ukspaCase == "") { thisProjectedReturn = "40"; }
			if (useUserParams)                    { thisProjectedReturn = "50"; }			

			if (productStartDateString == ""){ cerr << productId << "ProductStartDateString is empty... skipping this product" << endl; continue; }
			cout << endl << productIndx << " of " << numProducts << "\nIterations:" << thisNumIterations << " ProductId:" << productId << " " << analysisTypes[thisProjectedReturn].c_str() << " " << (doBumps ? "bumps" : "") << endl;
			// cout << "Press a key to continue...";  getline(cin, word);  // KEEP in case you want to attach debugger

			// see if product has levelsCall payoffs ... in which case we do not do a lognormalShift of the underlyings
			bool doPriceShift(true);
			if (thisNumIterations <= 1){
				sprintf(lineBuffer, "%s%d%s", "select count(*) from productbarrier join payofftype pt using (payofftypeid) where productid='", productId, "' and pt.name like 'levels%'");
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				retcode = mydb.fetch(true, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; continue; }
				if (retcode == SQL_SUCCESS && atof(szAllPrices[0]) > 0){ doPriceShift = false; }
			}
			

			// get counterparty info
			const double                     recoveryRate(0.4);
			std::vector<std::vector<double>> cdsTenors;  
			std::vector<std::vector<double>> cdsSpreads; 
			// annual default probability curve
			std::vector<std::vector<double>> hazardCurves; 
			vector<int>  theseIssuerIds; theseIssuerIds.push_back(counterpartyId);
			for (int possibleIssuerIndx=0; possibleIssuerIndx < (int)possibleIssuerIds.size(); possibleIssuerIndx++) { theseIssuerIds.push_back(possibleIssuerIds[possibleIssuerIndx]); }			
			for (int possibleIssuerIndx=0; possibleIssuerIndx < (int)theseIssuerIds.size(); possibleIssuerIndx++) {
				int counterpartyId = theseIssuerIds[possibleIssuerIndx];
				cdsTenors.push_back(    vector<double>() );  // allocate 1
				cdsSpreads.push_back(   vector<double>() );  // allocate 1
				hazardCurves.push_back( vector<double>() );  // allocate 1
				
				// ...mult-issuer product's have comma-separated issuers...ASSUMED equal weight
				sprintf(lineBuffer, "%s%d%s", "select EntityName from institution where institutionid='", counterpartyId, "' ");
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				retcode = mydb.fetch(true, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR) { std::cerr << "IPRerror:" << lineBuffer << endl; continue; }
				string counterpartyName = szAllPrices[0];
				vector<string> counterpartyNames;
				splitCommaSepName(counterpartyNames, counterpartyName);
				sprintf(charBuffer, "%s%s%s", "'", counterpartyNames.at(0).c_str(), "'");
				for (i = 1; i < (int)counterpartyNames.size(); i++) {
					sprintf(charBuffer, "%s%s%s%s", charBuffer, ",'", counterpartyNames.at(i).c_str(), "'");
				}

				if (counterpartyNames.size() > 1) {
					sprintf(lineBuffer, "%s%s%s%s%s%s%s",
						"select Maturity,avg(Spread) Spread from cdsspread",
						strlen(arcCdsDate) ? "archive" : "",
						" join institution using (institutionid) where EntityName in (",
						charBuffer,
						") ",
						arcCdsDateString,
						" and spread is not null and ccy = '' group by Maturity order by Maturity");
				}
				// see if there are ccy-specific spreads
				else {
					sprintf(lineBuffer, "%s%s%s%d%s%s%s%s%s%s%s%d%s%s",
						"select * from (select maturity,spread from cdsspread",
						strlen(arcCdsDate) ? "archive" : "",
						" where institutionid=",
						counterpartyId,
						" and ccy='",
						productCcy.c_str(),
						"' ",
						arcCdsDateString,
						" and spread is not null union select maturity,spread from cdsspread",
						strlen(arcCdsDate) ? "archive" : "",
						" where institutionid=",
						counterpartyId,
						arcCdsDateString,
						"  and ccy=''  and spread is not null)x group by Maturity order by Maturity");
				}
				mydb.prepare((SQLCHAR *)lineBuffer, 2);
				retcode = mydb.fetch(false, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR) { std::cerr << "IPRerror:" << lineBuffer << endl; continue; }

				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					double thisSpread, thisTenor;
					thisTenor = atof(szAllPrices[0]);
					cdsTenors[possibleIssuerIndx].push_back(thisTenor);
					thisSpread = atof(szAllPrices[1]) / 10000.0;
					cdsSpreads[possibleIssuerIndx].push_back(thisSpread);
					if (fabs(thisTenor - 5.0) < 0.01) {
						cds5y = thisSpread;
					}
					retcode = mydb.fetch(false, "");
				}
			}
			
			// get baseCurve
			//  ... check, but THINK USD swaps are quoted semiannual compounding
			//  ... SQL to check:         select Tenor,(exp(2.0*log(1.0 + (Rate/100.0/2.0))) - 1.0) Spread from curve where ccy='USD'  order by Tenor;
			string rateString = productCcy == "USD" ? "(exp(2.0*log(1.0 + (Rate/100.0/2.0))) - 1.0) " : "Rate / 100 ";
			sprintf(lineBuffer, "%s%s%s%s%s%s%s%s%s", 
				"select Tenor,",
				rateString.c_str(),
				"Spread from curve",
				strlen(arcCurveDate) ? "archive" : "",
				" where ccy='", 
				productCcy.c_str(),
				"' ",
				arcCurveDateString,
				" order by Tenor");
			mydb.prepare((SQLCHAR *)lineBuffer, 2);
			retcode = mydb.fetch(false, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; continue; }
			// cerr << "baseCurve:" << lineBuffer << endl;
			vector<SomeCurve> baseCurve;
			const double targetReturnSpread(doUseTargetPremium ? targetPremium : 0.05);   // targetReturn will be the spread at/before targetReturnTenor +targetReturnSpread
			double targetReturn(0.0), targetReturnTenor(5.0);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				anyCurve.tenor  = atof(szAllPrices[0]);
				anyCurve.spread = atof(szAllPrices[1]);
				if (anyCurve.tenor <= targetReturnTenor) {
					targetReturn = anyCurve.spread + targetReturnSpread;
				}
				baseCurve.push_back(anyCurve);
				retcode = mydb.fetch(false,"");
			}


			// get any compoIntoCcy data ... NOTE THIS ONLY uses the FX return to FULL TERM
			if (compoIntoCcy != ""){
				std::transform(std::begin(compoIntoCcy), std::end(compoIntoCcy), std::begin(compoIntoCcy), ::toupper);
			}
			// get underlyingids for this product from DB
			// they can come in any order of UnderlyingId (this is deliberate to avoid the code becoming dependent on any ordering
			double maxTZhrs(0.0);
			vector<int> ulIds, ulPriceReturnUids;
			vector<string> ulCcys;
			vector<bool> ulFixedDivs;
			vector<string> ulNames;
			map<int, string> ccyToUidMap;
			vector<double> ulERPs, ulTZhrs;
			vector<int> ulIdNameMap(1000);  // underlyingId -> arrayIndex, so ulIdNameMap[uid] gives the index into ulPrices vector
			sprintf(lineBuffer, "%s%s%s%s%s%d%s", "select distinct u.UnderlyingId UnderlyingId,upper(u.ccy) ulCcy,ERP,u.name,PriceReturnUid,TZhrs,FixedDivs from ", useProto, "productbarrier join ", useProto, "barrierrelation using (ProductBarrierId) join underlying u using (underlyingid) where ProductId='",
				productId, "' ");
			if (benchmarkId && !getMarketData && !forOptimisation){
				sprintf(charBuffer, "%s%d%s%s%s%d%s", " union (select ", benchmarkId, ",upper(u.ccy) ulCcy,ERP,u.name,PriceReturnUid,TZhrs,FixedDivs from ", useProto, "product p join underlying u on (p.BenchmarkId=u.UnderlyingId) where ProductId='", productId, "') ");
				strcat(lineBuffer, charBuffer);
			}
			mydb.prepare((SQLCHAR *)lineBuffer, 7);
			retcode = mydb.fetch(true, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; continue; }
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
				string thisCcy            = szAllPrices[1];
				double thisERP            = atof(szAllPrices[2]);
				string thisName           = szAllPrices[3];
				int    thisPriceReturnUid = atoi(szAllPrices[4]);
				double thisTZhrs          = atof(szAllPrices[5]);
				bool   thisFixedDivs      = atoi(szAllPrices[6]) == 1;

				ulCcys.push_back(thisCcy);
				ulTZhrs.push_back(thisTZhrs);
				ulFixedDivs.push_back(thisFixedDivs);
				if (fabs(thisTZhrs) > maxTZhrs){ maxTZhrs = fabs(thisTZhrs); }
				uid         = atoi(szAllPrices[0]);
				if (forOptimisation && find(optimiseUids.begin(), optimiseUids.end(), uid) == optimiseUids.end()){
					cerr << "cannot optimise with this underlyingId:" << uid << endl;
					exit(107);
				}
				ccyToUidMap[uid] = thisCcy;
				if (find(ulIds.begin(), ulIds.end(), uid) == ulIds.end()) {      // build list of uids
					ulIds.push_back(uid);
					ulERPs.push_back(thisERP);
					ulNames.push_back(thisName);
					ulPriceReturnUids.push_back(thisPriceReturnUid);
				}
				ulIdNameMap.at(uid) = (int)ulIds.size() - 1;
				// next record
				retcode = mydb.fetch(false,"");
			}
			numUl = (int)ulIds.size();
			if (forOptimisation){
				optimiseUlIdNameMap = ulIdNameMap;
			}
			// force stride of 2days if product has not already set a stride
			if (maxTZhrs > 5.0 && bootstrapStride == 0){
				bootstrapStride = 2;	
			}

			//** currencyStruck deals will have nonZero values for $crossRateUids
			vector<int> crossRateUids ; for (i=0; i<numUl; i++) { crossRateUids.push_back(0); }
			//** quanto deals will have nonZero values for $quantoCrossRateUids
			vector<int>    quantoCrossRateUids; 
			vector<double> quantoCorrelations,quantoCrossRateVols;
			for (i=0; i < numUl; i++) { quantoCrossRateUids.push_back(0); quantoCorrelations.push_back(0.0); quantoCrossRateVols.push_back(0.0); }
			if (currencyStruck || doPriips){
				// get PRIIPs start date to use
				if (doPriips){
					if (strlen(endDate)){ strcpy(charBuffer, endDate); }
					else {
						sprintf(lineBuffer, "%s", "select max(date) from prices");
						mydb.prepare((SQLCHAR *)lineBuffer, 1);
						retcode = mydb.fetch(true, lineBuffer);
						strcpy(charBuffer, szAllPrices[0]);
					}
					sprintf(priipsStartDatePhrase, "%s%s%s", " and Date >= date_sub('", charBuffer, "', interval 5 year) ");
				}

				for (i=0; i < numUl; i++) {
					if (productCcy != ulCcys[i]){
						sprintf(lineBuffer, "%s%s%s%s%s", "select UnderlyingId from underlying where name=concat('", ulCcys[i].c_str(), "','", productCcy.c_str(), "')");
						mydb.prepare((SQLCHAR *)lineBuffer, 1);
						retcode = mydb.fetch(false, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; continue; }
						if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
							int thisId = atoi(szAllPrices[0]);
							if (currencyStruck             ){ crossRateUids[i]       = thisId; }
							if (doPriips){ 
								double previousPrice[2];
								quantoCrossRateUids[i] = thisId;
								// get prices for ul and ccy
								sprintf(lineBuffer, "%s%d%s%d%s", "select Date,p0.price ul,p1.price ccy from prices p0 join prices p1 using (Date) where p0.underlyingid=", ulIds[i], " and p1.underlyingid=", thisId, priipsStartDatePhrase);
								mydb.prepare((SQLCHAR *)lineBuffer, 3);
								retcode   = mydb.fetch(false, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; continue; }
								firstTime = true;
								vector<vector<double>> theseReturns(2);
								while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
									int    numDayDiff;
									boost::gregorian::date bDate(boost::gregorian::from_simple_string(szAllPrices[0]));
									if (!firstTime) {
										boost::gregorian::date_duration dateDiff(bDate - lastDate);
										numDayDiff = dateDiff.days();
									}
									for (j = 0; j < 2; j++) {
										double thisPrice;
										thisPrice = atof(szAllPrices[j + 1]);
										if (!firstTime) { theseReturns[j].push_back(thisPrice / previousPrice[j]); }
										previousPrice[j] = thisPrice;
									}
									// next row
									if (firstTime){ firstTime = false; }
									lastDate = bDate;
									retcode = mydb.fetch(false, "");
								}
								// compute correlation (currently daily returns, but might be better with stride=2 say, and dailyVol
								if ((int)theseReturns[0].size()>25){
									quantoCorrelations[i] = MyCorrelation(theseReturns[0], theseReturns[1],true);
									double mean, stdev, stdErr;
									MeanAndStdev(theseReturns[1], mean, stdev, stdErr);
									quantoCrossRateVols[i] = stdev;
								}
							}
						}
					}
				}
			}


			// calc baseCcyReturn: this caters for products like #1093 where GBP invests in USD and is quoted in GBP, so has to reflect the USDGBP return since StrikeDate
			baseCcyReturn = 1.0;
			if (compoIntoCcy != ""){
				anyString = productCcy + compoIntoCcy;
				// get uid
				sprintf(lineBuffer, "%s%s%s", "select UnderlyingId from underlying u where u.name='", anyString.c_str(), "'");
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				retcode = mydb.fetch(false, ""); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; continue; }
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
					compoIntoCcyUid   = atoi(szAllPrices[0]);
				}
				else{
					cerr << " no underlying found for compoIntoCcy:" << anyString.c_str() << endl;
					exit(1071);
				}
				hasCompoIntoCcy = true;
			}
			int  addCompoIntoCcy = hasCompoIntoCcy ? 1 : 0;


			// read underlying prices
			vector<double>   ulReturns[maxUls], originalUlReturns[maxUls];
			for (i = 0; i < numUl + addCompoIntoCcy; i++) {
				ulReturns[i].reserve(10000); 
				ulOriginalPrices[i].date.reserve(10000);
				ulOriginalPrices[i].price.reserve(10000);
				ulOriginalPrices[i].nonTradingDay.reserve(10000);
			}
			char ulSql[10000],holdUlSql[10000]; // enough for around 100 underlyings...
			char crossRateBuffer[100];
			

			// ...form sql joins
			sprintf(ulSql, "%s", "select p0.Date Date");
			for (i = 0; i<numUl; i++) { 
				if (crossRateUids[i]){ sprintf(crossRateBuffer, "%s%d%s", "*p", (numUl + numUl + i), ".price"); }
				sprintf(lineBuffer, "%s%d%s%s%s%d", ",p", i, ".price", (crossRateUids[i] ? crossRateBuffer : ""), " Price", i); strcat(ulSql, lineBuffer);
			}
			// perhaps add compoIntoCcy
			if (compoIntoCcyUid != 0){
				sprintf(lineBuffer, "%s%d%s%s%s%d", ",p", numUl, ".price","", " Price", numUl); strcat(ulSql, lineBuffer);
			}
			strcat(ulSql, " from prices p0 ");
			if (crossRateUids[0]){ sprintf(crossRateBuffer, "%s%d%s", " join prices p", numUl + numUl, " using (Date) "); strcat(ulSql, crossRateBuffer); }
			for (i = 1; i < numUl; i++) { 
				if (crossRateUids[i]){ sprintf(crossRateBuffer, "%s%d%s", " join prices p", (numUl + numUl + i), " using (Date) "); }
				sprintf(lineBuffer, "%s%d%s%s", " join prices p", i, " using (Date) ", (crossRateUids[i] ? crossRateBuffer : "")); strcat(ulSql, lineBuffer);
			}
			// perhaps add compoIntoCcy
			if (compoIntoCcyUid != 0){
				sprintf(lineBuffer, "%s%d%s%s", " join prices p", numUl, " using (Date) ", ""); strcat(ulSql, lineBuffer);
			}
			sprintf(lineBuffer, "%s%d%s", " where p0.underlyingId = '", ulIds.at(0), "'");
			strcat(ulSql, lineBuffer);
			if (crossRateUids[0]){ sprintf(crossRateBuffer, "%s%d%s%d%s", " and p", numUl + numUl, ".underlyingid='", crossRateUids[0], "'"); strcat(ulSql, crossRateBuffer); }
			for (i = 1; i < numUl; i++) {
				if (crossRateUids[i]){ sprintf(crossRateBuffer, "%s%d%s%d%s", " and p", (numUl + numUl + i), ".underlyingid='", crossRateUids[i], "'"); }
				sprintf(lineBuffer, "%s%d%s%d%s%s", " and p", i, ".underlyingId='", ulIds.at(i), "'", (crossRateUids[i] ? crossRateBuffer : "")); strcat(ulSql, lineBuffer);
			}
			// perhaps add compoIntoCcy
			if (compoIntoCcyUid != 0){
				sprintf(lineBuffer, "%s%d%s%d%s%s", " and p", numUl, ".underlyingId='", compoIntoCcyUid, "'", ""); strcat(ulSql, lineBuffer);
			}
			if (strlen(startDate)) { sprintf(ulSql, "%s%s%s%s", ulSql, " and Date >='", startDate, "'"); }
			else {
				if (doPriips){ sprintf(ulSql, "%s%s", ulSql, priipsStartDatePhrase); }
				else if (forceStartDate != "0000-00-00"){ sprintf(ulSql, "%s%s%s%s", ulSql, " and Date >='", forceStartDate.c_str(), "'"); }
				else if (thisNumIterations>1) { strcat(ulSql, " and Date >='1992-12-31' "); }
			}
			strcpy(holdUlSql, ulSql);   // copy for possible reuse
			if (strlen(endDate))   { sprintf(ulSql, "%s%s%s%s", ulSql, " and Date <='", endDate,   "'"); }
			strcat(ulSql, " order by Date");
			// cerr << ulSql << endl;
			// ...call DB
			mydb.prepare((SQLCHAR *)ulSql, numUl + 1 + addCompoIntoCcy);
			firstTime = true;
			vector<double> previousPrice(numUl + addCompoIntoCcy), lastRealPrice(numUl + addCompoIntoCcy);
			vector<double> minPrices, maxPrices, shiftPrices;
			vector<bool>   doShiftPrices;
			for (i = 0; i < numUl + addCompoIntoCcy; i++) {
				minPrices.push_back(DBL_MAX);
				maxPrices.push_back(-DBL_MAX);
				shiftPrices.push_back(0.0);
				doShiftPrices.push_back(false);
			}
			// .. parse each record <Date,price0,...,pricen>
			retcode = mydb.fetch(true, ulSql); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << ulSql << endl; continue; }
			int numGaps = 0;
			boost::gregorian::date bEndDate; if (strlen(endDate)) { bEndDate =  (boost::gregorian::from_simple_string(endDate)); }
			boost::gregorian::date_duration bOneDay(1);
			bool extendingPrices(false);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO || (strlen(endDate) && strcmp(szAllPrices[0], endDate) < 0))	{
				int    numDayDiff;
				// extend prices into the future if need be
				if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO){
					if (!extendingPrices){
						for (i = 0; i < numUl + addCompoIntoCcy; i++) {
							sprintf(szAllPrices[i + 1], "%lf", (doRescaleSpots ? rescaleFraction : 1.0) * previousPrice[i]);
						}						
						extendingPrices  = true;
					}					
					sprintf(szAllPrices[0], "%s", to_iso_extended_string(lastDate + bOneDay).c_str());
				}
				boost::gregorian::date bDate(boost::gregorian::from_simple_string(szAllPrices[0]));
				if (!firstTime) {
					boost::gregorian::date_duration dateDiff(bDate - lastDate);
					numDayDiff = dateDiff.days();
					if (numDayDiff > 10){
						numGaps += 1;
						if (numGaps>10){
							std::cerr << "gaps in underlying prices at " << bDate << " compared to lastDate " << lastDate << endl;
						}						
					}
				}
				for (i = 0; i < numUl + addCompoIntoCcy; i++) {
					double thisPrice;
					thisPrice = atof(szAllPrices[i + 1]);
					if (thisPrice < minPrices[i]){ minPrices[i] = thisPrice; }
					if (thisPrice > maxPrices[i]){ maxPrices[i] = thisPrice; }
					if (!firstTime) {
						double thisReturn = thisPrice / previousPrice[i];
						if (isinf(thisReturn)){
							int jj = 1;
						}
						ulReturns[i].push_back(thisReturn);
						for (j = 1; j < numDayDiff; j++){    // pad non-trading days
							ulOriginalPrices.at(i).date.push_back(szAllPrices[0]);
							ulOriginalPrices.at(i).price.push_back(thisPrice);
							ulOriginalPrices.at(i).nonTradingDay.push_back(true);
							ulReturns[i].push_back(1.0);
						}
					}
					previousPrice[i] = thisPrice;
					ulOriginalPrices.at(i).date.push_back(szAllPrices[0]);
					ulOriginalPrices.at(i).price.push_back(thisPrice);
					ulOriginalPrices.at(i).nonTradingDay.push_back(false);
				}
				// next row
				if (firstTime){ firstTime = false; }
				lastDate = bDate;
				retcode = mydb.fetch(false,"");
			}
			// do any price overrides ... overwtite the latest price
			if (ulLevels.size()>0){
				for (i = 0; i < numUl; i++) {
					if (ulLevels.find(ulNames[i]) != ulLevels.end()) {
						ulOriginalPrices.at(i).price[ulOriginalPrices.at(i).price.size()-1] = ulLevels[ulNames[i]];
					}
				}
			}
			
			// see if there is enough data
			if (!getMarketData && ulOriginalPrices.at(0).date[0] > productStartDateString){
				cerr << "Not enough data: prices start on:" << ulOriginalPrices.at(0).date[0] << " but product strike is:" << productStartDateString << endl;
				continue;
			}

			// shift prices if necessary - crude attempt to do shifted-lognormal analysis, where for example rates are negative
			if (doPriceShift){
				for (i=0; i<numUl; i++) {
					if (minPrices[i] <= 0.0){
						double thisShift = -minPrices[i] + 0.1*(maxPrices[i] - minPrices[i]);
						for (j=0; j<(int)ulOriginalPrices[0].price.size(); j++){
							ulOriginalPrices[i].price[j] += thisShift;
						}
						firstTime = true;

						for (k=j=0; j<(int)ulOriginalPrices[0].price.size(); j++){
							double previousPrice;
							if (true /* !ulOriginalPrices.at(i).nonTradingDay[j] */){
								if (firstTime){ firstTime = false; }
								else {
									double thisReturn = ulOriginalPrices[i].price[j] / previousPrice;									
									ulReturns[i][k++] = thisReturn;
								}
								previousPrice = ulOriginalPrices[i].price[j];
							}
						}
						shiftPrices[i]   = thisShift;
						doShiftPrices[i] = true;
					}
				}
			}
			
			totalNumDays         = (int)ulOriginalPrices.at(0).price.size();
			lastDataDateString   = ulOriginalPrices.at(0).date[totalNumDays - 1];
			totalNumReturns      = totalNumDays - 1;
			boost::gregorian::date  bLastDataDate(boost::gregorian::from_simple_string(lastDataDateString));
			int daysExtant = (bLastDataDate - bProductStartDate).days();
			// change to BID/ASK for post/pre-strike; to use MID do: (bidPrice + askPrice) / (2.0*issuePrice)
			bool ignoreBidAsk    = ((bidAskDateString != lastDataDateString) || stalePrice);
			bool validFairValue  = (fairValueDateString == lastDataDateString) && (daysExtant > 14) && fairValuePrice>0.0;
			bool isPostStrike    = productStartDateString < lastDataDateString;
			midPrice             = (isPostStrike && ignoreBidAsk && validFairValue ? fairValuePrice : (ignoreBidAsk ? (validFairValue && isPostStrike ? fairValuePrice : issuePrice) : (isPostStrike ? bidPrice:askPrice))) / issuePrice;
			if (midPrice <= 0.0){
				cerr << "Product:" << productId << "midPrice <= 0.0 ... reset to 0.01 " << endl;
				midPrice  =  0.01;
			}
			if (strlen(endDate)){
				// get ASK from productprices if exists
				sprintf(lineBuffer, "%s%d%s%s%s", "select Ask from productprices where ProductId=", productId," and date<='",endDate,"' order by Date desc limit 1");
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				retcode = mydb.fetch(false, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; continue; }
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					midPrice  = atof(szAllPrices[0]) / issuePrice;
					sprintf(lineBuffer, "%s%s%s%d", "update product set BidAskDate='",endDate,"' where ProductId=", productId);
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
				}
			}
			if (doUseThisPrice){ midPrice = useThisPrice / issuePrice; }
			// spotsDate ... change last price to those for some date
			if (strlen(spotsDate))   {
				int numPrices = (int)ulOriginalPrices[0].price.size();
				sprintf(lineBuffer, "%s%s%s%s", holdUlSql, " and Date ='", spotsDate, "'");
				mydb.prepare((SQLCHAR *)lineBuffer, numUl + 1 + addCompoIntoCcy);
				retcode = mydb.fetch(true, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; continue; }
				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
					for (i = 0; i < numUl + addCompoIntoCcy; i++) {
						double thisPrice;
						thisPrice = atof(szAllPrices[i + 1]);
						ulOriginalPrices[i].price[numPrices-1] = thisPrice;
					}
					retcode = mydb.fetch(false, "");
				}

			}
			ulPrices             = ulOriginalPrices; // copy constructor called
			cout << "NumPrices: " << totalNumDays << "  FirstDataDate: " << ulOriginalPrices.at(0).date[0] << " LastDataDate: " << lastDataDateString << "  MidPriceUsed: " << midPrice << endl;
			

			if (hasCompoIntoCcy){
				// get compoIntoCcy return-to-date
				if (daysExtant > 0){
					sprintf(lineBuffer, "%s%s%s%s%s%s%s", "select p0.Price,p1.Price/p0.Price from prices p0 join prices p1 using (underlyingid) join underlying u using (underlyingid) where u.name='",
						anyString.c_str(), "' and p0.date='", productStartDateString.c_str(), "' and p1.date='", lastDataDateString.c_str(), "'");
					mydb.prepare((SQLCHAR *)lineBuffer, 2);
					retcode = mydb.fetch(false, ""); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; continue; }
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
						compoIntoCcyStrikePrice = atof(szAllPrices[0]);
						baseCcyReturn           = atof(szAllPrices[1]);
					}
				}				
				else{
					compoIntoCcyStrikePrice = ulOriginalPrices.at(numUl).price[totalNumDays - 1];
				}
			}
			
			// save spots
			vector<double> spots,strikeDateLevels;
			for (i=0; i < numUl; i++){ 
				spots.push_back(ulPrices[i].price[totalNumDays-1]); 
				strikeDateLevels.push_back(ulPrices[i].price[totalNumDays - 1 - (daysExtant>0 ? daysExtant : 0)]);
			}

			double forwardStartT(0.0);
			if (daysExtant < 0){
				if (!doUKSPA && !doPriips){ forwardStartT = daysExtant / 365.25; }
				daysExtant = 0; 
			}


			// benchmark moneyness ... measure performance only from valuation date
			double benchmarkMoneyness = 1.0;
			/*
			if (benchmarkId != 0){
				// ...compute moneyness
				const UlTimeseries  &ulTimeseries =  ulOriginalPrices.at(ulIdNameMap[benchmarkId]);
				int lastIndx(ulTimeseries.price.size() - 1);  // range-checked now so can use vector[] to access elements
				benchmarkMoneyness  = ulTimeseries.price[lastIndx] / ulTimeseries.price[lastIndx - daysExtant];
			}
			*/
			

			// market data
			vector< vector<double> >          ulVolsTenor(numUl);
			vector< vector<double> >          ulFwdsAtVolTenor(numUl);
			vector< vector<vector<double>> >  ulVolsStrike(numUl);
			vector< vector<vector<double>> >  ulVolsImpVol(numUl);
			vector< vector<vector<double>> >  ulVolsBsImpVol(numUl);
			vector< vector<vector<double>> >  ulVolsBumpedLocalVol(numUl);
			vector< vector<vector<double>> >  ulVolsFwdVol(numUl);
			vector<vector<double>>            oisRatesTenor(numUl);
			vector<vector<double>>            oisRatesRate(numUl);
			vector<vector<double>>            divYieldsTenor(numUl);
			vector<vector<double>>            divYieldsRate(numUl);
			vector<vector<double>>            divYieldsStdErr(numUl);
			vector<vector<int>>               corrsOtherId(numUl);
			vector<vector<double>>            corrsCorrelation(numUl);
			vector<vector<int>>               fxcorrsOtherId(numUl);
			vector<vector<double>>            fxcorrsCorrelation(numUl);

			// get divs for a number of analysis cases
			if (doPriips || getMarketData || useUserParams){
				//  divYields
				sprintf(ulSql, "%s%s%s%s%s%s%s%d", "select d.underlyingid,",
					doUKSPA || doPriips ? "100 Tenor,(d.divyield+dd.divyield)/2.0" : "Tenor,impdivyield",
					" Rate,IsTotalReturn,d.StdErr from ",
					doUKSPA || doPriips ? "divyield dd join divyield " : "impdivyield",
					!doUKSPA && !doPriips && strlen(arcDivDate) ? "archive" : "",
					doUKSPA || doPriips ? "d using (underlyingid,userid)" : " d",
					" join underlying u using (underlyingid) where d.UnderlyingId in (", ulIds[0]);
				for (i = 1; i < numUl; i++) {
					sprintf(ulSql, "%s%s%d", ulSql, ",", ulIds[i]);
				}
				sprintf(ulSql, "%s%s%d%s%s%s", ulSql, ") and d.userid=", userId,
					doUKSPA || doPriips ? " and dd.tenor=1 and d.tenor=5 " : "",
					arcDivDateString,
					" order by UnderlyingId,Tenor ");
				// .. parse each record <Date,price0,...,pricen>
				mydb.prepare((SQLCHAR *)ulSql, 5);
				retcode = mydb.fetch(false, ulSql); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << ulSql << endl; continue; }
				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
					int    thisUidx      = ulIdNameMap.at(atoi(szAllPrices[0]));
					double thisTenor     = atof(szAllPrices[1]);
					double thisYield     = atof(szAllPrices[2]);
					bool   isTotalReturn = atoi(szAllPrices[3]) == 1;
					double thisRate      = thisYield;
					double thisStdErr    = atof(szAllPrices[4]);					

					if (ukspaCase != ""){
						thisRate       =  ulERPs[thisUidx] - thisYield;
						if (ukspaCase == "Bull")        { thisRate = -thisRate; }
						else if (ukspaCase == "Neutral"){ thisRate = 0.0; }
						if (isTotalReturn) {
							thisRate -= thisYield;
						}
					}
					else if (doPriips){
						thisRate = -thisRate;
					}
					divYieldsTenor[thisUidx].push_back(thisTenor);
					divYieldsRate[thisUidx].push_back(thisRate);
					divYieldsStdErr[thisUidx].push_back(thisStdErr);
					retcode = mydb.fetch(false, "");
				}
				// add dummy records for underlyings for which there are no divs (will include, for example total return indices)
				for (i = 0; i < numUl; i++) {
					if ((int)divYieldsTenor[i].size() == 0){
						double driftAdj = 0.0;
						if (ukspaCase != "" || doPriips){
							double thisERP = ulERPs[i];
							if (ulPriceReturnUids[i] || doPriips){  // see if there is a related underlying with a yield 
								sprintf(lineBuffer, "%s%d", "select divyield from divyield where UnderlyingId=",
									doPriips ? ulIds[i] : ulPriceReturnUids[i]);
								mydb.prepare((SQLCHAR *)lineBuffer, 1);
								retcode = mydb.fetch(false, ulSql);
								if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
									driftAdj -= atof(szAllPrices[0]);
								}
							}
							if (ukspaCase == "Bear")     { driftAdj  = thisERP + 2.0*driftAdj; }
							else if (ukspaCase == "Bull"){ driftAdj  = -thisERP; }
						}
						divYieldsTenor[i].push_back(10.0);
						divYieldsRate[i].push_back(driftAdj);
						divYieldsStdErr[i].push_back(0.0);
					}
				}
				// convert fixedDivs to money
				if (doUKSPA || doPriips){
					for (i = 0; i < numUl; i++) {
						if (ulFixedDivs[i]){
							for (j = 0; j < divYieldsRate[i].size(); j++) {
								divYieldsRate[i][j] *= spots[i];
							}
						}						
					}
				}
				
			}
			// get vols
			if (getMarketData || useUserParams){
				int    thisUidx;
				double thisFwdVol, thisTenor, previousVol, previousTenor;
				vector<double> someVols, someStrikes, someFwdVols;

				// UKSPA vols
				if (ukspaCase != "" && totalNumReturns > 2){
					// calc vols  - 5y window, daily returns
					int startPoint = totalNumDays <= 1825 ? 0 : totalNumDays - 1825;
					vector<double> tempPrices;
					vector<double> tempReturns, tempReturns1;
					for (thisUidx = 0; thisUidx < numUl; thisUidx++) {
						tempPrices.resize(0);
						tempReturns.resize(0);
						tempReturns1.resize(0);
						for (i=startPoint; i < totalNumDays; i++){
							if (!ulOriginalPrices[thisUidx].nonTradingDay[i]){
								tempPrices.push_back(ulOriginalPrices[thisUidx].price[i]);
							}
						}
						for (i=1; i<(int)tempPrices.size(); i++){
							double thisReturn = tempPrices[i - 1] > 0.0 ? tempPrices[i] / tempPrices[i - 1] : 1.0;
							tempReturns.push_back(thisReturn);
						}
						// calc 1y vol
						int numReturns = (int)tempReturns.size();
						for (j=0,i=numReturns-1; i >= 0 && j<253; j++,i--){
							tempReturns1.push_back(tempReturns[i]);
						}
						double thisMean, thisVol, thisVol1y, thisVol5y, thisStdErr;
						MeanAndStdev(tempReturns,  thisMean, thisVol5y, thisStdErr);
						MeanAndStdev(tempReturns1, thisMean, thisVol1y, thisStdErr);
						thisVol = (thisVol5y+thisVol1y)*8.0;  // average of 5y and 1y vols
						if (ukspaCase == "Bear"){
							thisVol *= 1.1;
						}
						else if (ukspaCase == "Bull"){
							thisVol *= 0.9;
						}
						someVols.push_back(thisVol);
						someFwdVols.push_back(thisVol);
						someStrikes.push_back(1.0);

						ulVolsTenor[thisUidx].push_back(10.0);
						ulVolsImpVol[thisUidx].push_back(someVols);
						ulVolsFwdVol[thisUidx].push_back(someFwdVols);
						ulVolsStrike[thisUidx].push_back(someStrikes);

						someVols.resize(0);
						someFwdVols.resize(0);
						someStrikes.resize(0);
					}

				}
				// getMarketData or useUserParams vols
				else {
					
					sprintf(ulSql, "%s%lf%s%s%s%s%s%d", "select UnderlyingId,Tenor,Strike,greatest(0.0,ImpVol+",volShift,") Impvol from ",
						localVol && !bsPricer && getMarketData && !useUserParams ? "local":"imp",
						"vol",
						strlen(arcVolDate) ? "archive" : "",
						" where underlyingid in (", ulIds[0]);
					for (i = 1; i < numUl; i++) {
						sprintf(ulSql, "%s%s%d", ulSql, ",", ulIds[i]);
					}
					sprintf(ulSql, "%s%s%d%s%s", ulSql, 
						") and userid=", userId, 
						arcVolDateString,
						" order by UnderlyingId, Tenor, Strike");
					// .. parse each record <UnderlyingId,Tenor,Strike,Impvol>
					thisUidx    = 0;          // underlying index
					thisTenor   = -1.0;
					mydb.prepare((SQLCHAR *)ulSql, 4);
					retcode = mydb.fetch(true, ulSql); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << ulSql << endl; continue; }

					while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
						int    nextUidx   = ulIdNameMap.at(atoi(szAllPrices[0]));
						double nextTenor  = atof(szAllPrices[1]);
						double thisStrike = atof(szAllPrices[2]);
						double thisVol    = atof(szAllPrices[3]);
						// data order: underlyingId, tenor, strike
						// ... if either underlyingId or tenor changes
						if (nextTenor != thisTenor || nextUidx != thisUidx){
							// save a row in volSurface for the current underlying 
							if ((int)someVols.size() > 0){
								ulVolsImpVol[thisUidx].push_back(someVols);
								someVols.resize(0);
								ulVolsFwdVol[thisUidx].push_back(someFwdVols);
								someFwdVols.resize(0);
								ulVolsStrike[thisUidx].push_back(someStrikes);
								someStrikes.resize(0);
							}
							if (nextUidx != thisUidx){
								thisUidx   = nextUidx;
							}
							thisTenor   = nextTenor;
							ulVolsTenor[thisUidx].push_back(thisTenor);
						}
						// accumulate this strike/vol/fwdVol
						someVols.push_back(thisVol);
						someStrikes.push_back(thisStrike);
						// push this forward vols
						int numTenors  = (int)ulVolsImpVol[thisUidx].size();
						int numStrikes = (int)someStrikes.size();
						if (numTenors > 0){ // calc forward vols
							previousVol   = ulVolsImpVol[thisUidx][numTenors - 1][numStrikes - 1];
							previousTenor = ulVolsTenor[thisUidx][numTenors - 1];
							double varianceDiff = (thisVol*thisVol*thisTenor - previousVol*previousVol*previousTenor);
							thisFwdVol    = varianceDiff<0.0 ? thisVol : pow(varianceDiff / (thisTenor - previousTenor), 0.5);
							if (thisFwdVol < 0.0){ thisFwdVol = 0.0; }
						}
						else {
							thisFwdVol  = thisVol;
						}
						someFwdVols.push_back(thisFwdVol);
						retcode = mydb.fetch(false, "");
					}
					// tail-end charlie
					if ((int)someVols.size()>0){
						ulVolsImpVol[thisUidx].push_back(someVols);
						ulVolsFwdVol[thisUidx].push_back(someFwdVols);
						ulVolsStrike[thisUidx].push_back(someStrikes);
					}
				} // END vols
				//  OIS rates
				//  ... 2022 saw a lot of ois tickers disappear, so we now use corresponding swap tickers ... hence need to convert to continuous compounding
				//  ... the impDivYield Excel calcs assume USD compounding is semi-annual
				sprintf(ulSql, "%s%s%s%s", "select ccy,Tenor,Rate from oncurve",
					strlen(arcOnCurveDate) ? "archive" : "",
					" v where ccy in ('", ulCcys[0].c_str());
				for (i = 1; i < numUl; i++) {
					sprintf(ulSql, "%s%s%s", ulSql, "','", ulCcys[i].c_str());
				}
				sprintf(ulSql, "%s%s%s%s", ulSql, "') ",
					arcOnCurveDateString,
					" order by ccy,Tenor");
				// .. parse each record <Date,price0,...,pricen>
				string thisCcy = "";
				mydb.prepare((SQLCHAR *)ulSql, 3);
				retcode = mydb.fetch(false, ulSql);
				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
					string thisCcy         = szAllPrices[0];
					double compoundingFreq = thisCcy == "USD" ? 2.0 : 1.0;
					double thisTenor       = atof(szAllPrices[1]);
					double thisRate        = atof(szAllPrices[2]) / 100.0;
					// DB now grabs SWAP rates from Bberg (previously ois tickers were available, but then became unavailable ... forget why
					// ... anyway we now must convert to continuous-compounding
					thisRate               = log(1.0 + (thisRate/compoundingFreq)) * compoundingFreq;
					if (doUseThisOIS) { thisRate = useThisOIS; }
					for (thisUidx = 0; thisUidx < numUl; thisUidx++) {
						if (ccyToUidMap[ulIds[thisUidx]] == thisCcy) {
							oisRatesRate[thisUidx].push_back(thisRate);
							oisRatesTenor[thisUidx].push_back(thisTenor);
						}
					}
					retcode = mydb.fetch(false, "");
				}
				// add dummy records for underlyings for which there are no rates
				for (i = 0; i < numUl; i++) {
					if ((int)oisRatesTenor[i].size() == 0){
						oisRatesTenor[i].push_back(10.0);
						oisRatesRate[i].push_back(0.0);
					}
				}

				//  eq-eq corr
				if (ukspaCase != "" && totalNumReturns > 2){
					// calc corrs - 1y window, 3day returns
					int periodicity = 3;
					int startPoint = totalNumDays <= 365 ? 0 : totalNumDays - 365;
					vector<double> tempPrices, tempReturns, tempPrices1, tempReturns1;
					for (thisUidx = 0; thisUidx < numUl; thisUidx++) {
						tempPrices.resize(0);
						tempReturns.resize(0);
						for (j=0, i=startPoint; i < totalNumDays; i++){
							if (!ulOriginalPrices[thisUidx].nonTradingDay[i]){
								j += 1;
								if (j % periodicity == 0){ tempPrices.push_back(ulOriginalPrices[thisUidx].price[i]); }
							}
						}
						for (i=1; i<(int)tempPrices.size(); i++){
							double thisReturn = tempPrices[i - 1] > 0.0 ? tempPrices[i] / tempPrices[i - 1] : 1.0;
							tempReturns.push_back(thisReturn);
						}
						for (int otherUidx=thisUidx + 1; otherUidx < numUl; otherUidx++) {
							tempPrices1.resize(0);
							tempReturns1.resize(0);
							for (j=0, i=startPoint; i < totalNumDays; i++){
								if (!ulOriginalPrices[otherUidx].nonTradingDay[i]){
									j += 1;
									if (j % periodicity == 0){ tempPrices1.push_back(ulOriginalPrices[otherUidx].price[i]); }
								}
							}
							for (i=1; i<(int)tempPrices1.size(); i++){
								double thisReturn = tempPrices1[i - 1] > 0.0 ? tempPrices1[i] / tempPrices1[i - 1] : 1.0;
								tempReturns1.push_back(thisReturn);
							}
							double thisCorr = MyCorrelation(tempReturns, tempReturns1,true);
							corrsOtherId[thisUidx].push_back(otherUidx);
							corrsCorrelation[thisUidx].push_back(thisCorr);
						}
					}
				}
				else {
					sprintf(ulSql, "%s%s%s%d", "select UnderlyingId,OtherId,Correlation from correlation",
						strlen(arcCorDate) ? "archive" : "",
						" c where OtherIdIsCcy=0 and UnderlyingId in (", ulIds[0]);
					for (i = 1; i < numUl; i++) {
						sprintf(ulSql, "%s%s%d", ulSql, ",", ulIds[i]);
					}
					sprintf(ulSql, "%s%s%d", ulSql, ") and OtherId in (", ulIds[0]);
					for (i = 1; i < numUl; i++) {
						sprintf(ulSql, "%s%s%d", ulSql, ",", ulIds[i]);
					}
					sprintf(ulSql, "%s%s%d%s%s", ulSql, ")   and UnderlyingId != OtherId and userid=", useMyEqEqCorr ? (corrUserId>0 ? corrUserId : productUserId) : userId,
						arcCorDateString,
						" order by UnderlyingId,OtherId ");
					// .. parse each record <Date,price0,...,pricen>
					mydb.prepare((SQLCHAR *)ulSql, 3);
					retcode   = mydb.fetch(false, ulSql);
					std::vector<std::tuple<int, int>> foundCorr;
					while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
						int    thisUid    = atoi(szAllPrices[0]);
						int    otherId    = atoi(szAllPrices[1]);
						int    thisUidx   = ulIdNameMap.at(thisUid);
						int    otherUidx  = ulIdNameMap.at(otherId);
						double thisCorr   = atof(szAllPrices[2]);
						if (forceEqEqCorr && ((eqCorrelationUid == thisUid && eqCorrelationOtherId == otherId) || (eqCorrelationUid == otherId && eqCorrelationOtherId == thisUid))){
							thisCorr = forceEqEqCorrelation;
						}
						// if we already have this correlation (correlation table may have also entered the correlation the other way round)
						bool done = false;
						for (int i=0; !done && i < foundCorr.size(); i++){
							if (std::get<0>(foundCorr[i]) == thisUid && std::get<1>(foundCorr[i]) == otherId){
								done = true;
							}
						}
						if (!done){
							corrsOtherId[thisUidx].push_back(otherUidx);
							corrsCorrelation[thisUidx].push_back(thisCorr);
							foundCorr.push_back(std::make_tuple(otherId, thisUid));
						}
						retcode = mydb.fetch(false, "");
					}
				}
				//  eq-fx corr
				sprintf(ulSql, "%s%d", "select UnderlyingId,OtherId,Correlation from correlation c join currencies y on (y.CcyId=c.OtherId) where OtherIdIsCcy=1 and UnderlyingId in (", ulIds[0]);
				for (i = 1; i < numUl; i++) {
					sprintf(ulSql, "%s%s%d", ulSql, ",", ulIds[i]);
				}
				sprintf(ulSql, "%s%s%s%s%d%s", ulSql, ") and y.Name='", productCcy.c_str(), "'  and userid=", useMyEqFxCorr ? (corrUserId>0 ? corrUserId : productUserId) : userId, " order by UnderlyingId,OtherId ");
				// .. parse each record <Date,price0,...,pricen>
				mydb.prepare((SQLCHAR *)ulSql, 3);
				retcode   = mydb.fetch(false, ulSql);
				std::vector<std::tuple<int, int>> foundFxCorr;
				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
					int    thisUid    = atoi(szAllPrices[0]);
					int    thisUidx   = ulIdNameMap.at(thisUid);
					int    otherId    = atoi(szAllPrices[1]);
					double thisCorr   = atof(szAllPrices[2]);
					// if we already have this correlation (correlation table may have also entered the correlation the other way round)
					bool done = false;
					for (int i=0; !done && i < foundFxCorr.size(); i++){
						if (std::get<0>(foundFxCorr[i]) == thisUid && std::get<1>(foundFxCorr[i]) == otherId){
							done = true;
						}
					}
					if (!done){
						fxcorrsOtherId[thisUidx].push_back(otherId);
						fxcorrsCorrelation[thisUidx].push_back(forceEqFxCorr &&	fxCorrelationUid == thisUid && fxCorrelationOtherId == otherId ? forceEqFxCorrelation : thisCorr);
						foundFxCorr.push_back(std::make_tuple(otherId, thisUid));
					}
					retcode = mydb.fetch(false, "");
				}
				// initialise any correlation bumps
				if (corrNames.size()>0){
					corrUidx       = ulIdNameMap.at(corrIds[0]);
					if (corrsAreEqEq){
						corrOtherIndex = getIndexInVector(corrsOtherId[corrUidx], ulIdNameMap.at(corrIds[1]));
					}
					else{
						corrOtherIndex = getIndexInVector(fxcorrsOtherId[corrUidx], corrIds[1]);
					}
					if (corrOtherIndex<0){
						cerr << " correlation  between " << corrNames[0].c_str() << " and " << corrNames[1].c_str() << " not involved in this product" << endl;
						continue;
					}
				}
				
				// check we have data for all underlyings
				for (i = 0; i < numUl; i++) {
					if ((int)ulVolsTenor[i].size() == 0){
						cerr << "No volatilities found for " << ulNames[i] << endl; 
						exit(107); 
					}
					if ((int)divYieldsTenor[i].size() == 0){
						cerr << "No dividends found for " << ulNames[i] << endl;
						exit(108);
					}
				}
			}
			/*
			*  collect all market data
			*/
			MarketData  thisMarketData(ulVolsTenor,
			ulVolsStrike,
			ulVolsImpVol,
			ulVolsFwdVol,
			oisRatesTenor,
			oisRatesRate,
			divYieldsTenor,
			divYieldsRate,
			divYieldsStdErr,
			corrsOtherId,
			corrsCorrelation,
			fxcorrsOtherId,
			fxcorrsCorrelation 
			);

			/*
			* doUseThisVol
			*/
			if (doUseThisVol) {
				for (i=0; i < numUl; i++) {
					for (j=0; j < (int)ulVolsTenor[i].size(); j++) {
						for (k=0; k < (int)ulVolsStrike[i][j].size(); k++) {
							ulVolsImpVol[i][j][k] = useThisVol;
						}
					}
				}
			}
			

			/*
			*  build underlyings' forwardPrices at vol-tenors ... in case we need to recalcLocalVol()
			*/
			for (i = 0; i < numUl; i++) {
				for (j=0; j < (int)ulVolsTenor[i].size(); j++){
					double thisTenor   = ulVolsTenor[i][j];
					double thisOisRate = interpVector(oisRatesRate[i], oisRatesTenor[i], thisTenor);
					double thisDivRate = interpVector(divYieldsRate[i], divYieldsTenor[i], thisTenor);
					ulFwdsAtVolTenor[i].push_back(exp((thisOisRate - thisDivRate)*thisTenor));
				}
			}
			if (forceLocalVol && bsPricer){
				localVol = true;
				// deep copy of impliedVols
				for (i=0; i < numUl; i++){
					for (j=0; j < (int)ulVolsTenor[i].size(); j++){
						vector<double>  someImpVol;
						for (k=0; k < (int)ulVolsStrike[i][j].size(); k++){
							someImpVol.push_back(ulVolsImpVol[i][j][k]);
						}
						ulVolsBsImpVol[i].push_back(someImpVol);
					}
				}
				recalcLocalVol(
					ulVolsTenor,
					ulVolsStrike,
					ulVolsImpVol,
					ulFwdsAtVolTenor,
					ulVolsBumpedLocalVol
					);
				thisMarketData.ulVolsImpVol = ulVolsBumpedLocalVol;
			}
			else {
				// deep copy of impliedVols
				for (i=0; i < numUl; i++){
					for (j=0; j < (int)ulVolsTenor[i].size(); j++){
						vector<double>  someImpVol;
						for (k=0; k < (int)ulVolsStrike[i][j].size(); k++){
							someImpVol.push_back(ulVolsImpVol[i][j][k]);
						}
						ulVolsBumpedLocalVol[i].push_back(someImpVol);
					}
				}
			}

			// enough data?
			if (totalNumDays - 1 < daysExtant){
				cerr << "Not enough data to determine strike for product#:" << productId << endl;
				if (doDebug){ exit(109); }
				continue;
			}

			// sundry init
			std::vector<double> &corrBumpVector(corrsAreEqEq ? thisMarketData.corrsCorrelation[corrUidx] : thisMarketData.fxcorrsCorrelation[corrUidx]);
			const double  holdCorr = corrNames.size()>0 ? corrBumpVector[corrOtherIndex] : 0.0;
			std::string  corrString(""); for (int i=0; i < corrNames.size(); i++){ corrString = corrString + corrNames[i] + " "; } 

			// create product
			if (AMC != 0.0){
				cerr << endl << "******NOTE******* product has an AMC:" << AMC << endl;
			}
			if (issuerCallable && getMarketData && thisNumIterations > 1 && !doDebug && (thisNumIterations < MIN_CALLABLE_ITERATIONS || thisNumIterations > MAX_CALLABLE_ITERATIONS)){
				cerr << endl << "******NOTE******* issuerCallable product needs iterations in the range" << MIN_CALLABLE_ITERATIONS << ":" << MAX_CALLABLE_ITERATIONS << endl;
				thisNumIterations = MAX_CALLABLE_ITERATIONS;
			}
			std::vector<double>  cdsVols;
			double annualFundingUnwindCost(0.0);  // not getting sensible results for IssuerCallables ... so hold off until we know more what issuers do ...
			SProduct spr(extendingPrices,thisCommandLine,mydb,&lineBuffer[0],bLastDataDate,productId, userId, productCcy, ulOriginalPrices.at(0), bProductStartDate, fixedCoupon, couponFrequency, couponPaidOut, AMC, showMatured,
				productShape, fullyProtected, benchmarkStrike,depositGteed, collateralised, daysExtant, midPrice, baseCurve, ulIds, forwardStartT, issuePrice, ukspaCase,
				doPriips,ulNames,(fairValueDateString == lastDataDateString),fairValuePrice / issuePrice, askPrice / issuePrice,baseCcyReturn,
				shiftPrices, doShiftPrices, forceIterations, optimiseMcLevels, optimiseUlIdNameMap,forOptimisation, saveOptimisationPaths, productIndx,
				bmSwapRate, bmEarithReturn, bmVol, cds5y, bootstrapStride, settleDays, silent, updateProduct, verbose, doBumps, stochasticDrift, localVol, ulFixedDivs, compoIntoCcyStrikePrice,
				hasCompoIntoCcy,issuerCallable,spots, strikeDateLevels, gmmMinClusterFraction, multiIssuer,cdsVols,volShift, targetReturn, doForwardValueCoupons || getMarketData);
			numBarriers = 0;

			// get barriers from DB
			enum {
				colProductBarrierId = 0, colProductId,
				colCapitalOrIncome, colNature, colPayoff, colTriggered, colSettlementDate, colDescription, colPayoffId, colParticipation,
				colStrike, colAvgTenor, colAvgFreq, colAvgType, colCap, colUnderlyingFunctionId, colParam1, colMemory, colIsAbsolute, colAvgInTenor, colAvgInFreq, colStrikeReset, colStopLoss, colAvgInAlgebra, colOriginalStrike, colForfeitCoupons, colCommands,colProductBarrierLast
			};
			sprintf(lineBuffer, "%s%s%s%d%s", "select * from ", useProto, "productbarrier where ProductId='", productId, "' order by SettlementDate,ProductBarrierId");
			mydb.prepare((SQLCHAR *)lineBuffer, colProductBarrierLast);
			retcode = mydb.fetch(true, lineBuffer); if (retcode == MY_SQL_GENERAL_ERROR){ std::cerr << "IPRerror:" << lineBuffer << endl; continue; }
			map<char, int>::iterator curr, end;
			// ...parse each productbarrier row
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
				int avgDays = 0, avgInDays = 0;
				int avgFreq = 0, avgInFreq = 0;
				bool isMemory      = atoi(szAllPrices[colMemory]     ) == 1;
				
				if (strlen(szAllPrices[colAvgTenor]) && strlen(szAllPrices[colAvgFreq])){
					productNeedsFullPriceRecord = true;
					buildAveragingInfo(szAllPrices[colAvgTenor], szAllPrices[colAvgFreq], avgDays, avgFreq);
				}
				if (strlen(szAllPrices[colAvgInTenor]) && strlen(szAllPrices[colAvgInFreq])){
					productNeedsFullPriceRecord = true;
					buildAveragingInfo(szAllPrices[colAvgInTenor], szAllPrices[colAvgInFreq], avgInDays, avgInFreq);
				}
				int barrierId           = atoi(szAllPrices[colProductBarrierId]);
				int avgType             = atoi(szAllPrices[colAvgType]);
				string barrierCommands  = szAllPrices[colCommands];
				bool isAbsolute         = atoi(szAllPrices[colIsAbsolute]     ) == 1;
				bool isStrikeReset      = atoi(szAllPrices[colStrikeReset]    ) == 1;
				bool isStopLoss         = atoi(szAllPrices[colStopLoss]       ) == 1;
				bool isForfeitCoupons   = atoi(szAllPrices[colForfeitCoupons] ) == 1;
				capitalOrIncome         = atoi(szAllPrices[colCapitalOrIncome]) == 1;
				nature                  = szAllPrices[colNature];
				payoff                  = atof(szAllPrices[colPayoff]) / 100.0;
				settlementDate          = szAllPrices[colSettlementDate];
				double thisCoupon       = capitalOrIncome ? max(0.0, payoff - 1.0) : payoff;
				double barrierBendAmort = (!(doBarrierBendAmort) || daysExtant <= 0) ? 1.0 : (daysExtant > thisBarrierBendDays ? thisBarrierBendFraction : 1.0 - (1.0 - thisBarrierBendFraction)*(double)daysExtant / thisBarrierBendDays);
				double thisBarrierBend  = (getMarketData && !doUKSPA) ?                        // only bend for FV
												(!doUseThisBarrierBend && thisCoupon > 0.0 ?   // does barrier have a "coupon" including capitalPayoff > 100%  (and not command-line-overridden)
													0.1*(thisCoupon>0.5 ? 0.5 : thisCoupon)    // ... if so:    10% of any coupon, but limit to 5%
													: barrierBend                              // ... if not:   product table field, possibly command-line-overriden by doUseThisBarrierBend
												) 
												: 0.0 ; 
				thisBarrierBend        *= barrierBendAmort;
				if (doUseThisBarrierBend){ thisBarrierBend = useThisBarrierBend / 100.0; }

				// PRIIPs Intermediate Scenario?
				description             = szAllPrices[colDescription];
				avgInAlgebra            = szAllPrices[colAvgInAlgebra];
				thisPayoffId            = atoi(szAllPrices[colPayoffId]);
				thisPayoffType          = payoffType[thisPayoffId];
				participation           = atof(szAllPrices[colParticipation]);
				// barrier bend
				string ucPayoffType(thisPayoffType);
				transform(ucPayoffType.begin(), ucPayoffType.end(), ucPayoffType.begin(), toupper);
				bool isCall  = std::regex_search(ucPayoffType, std::regex{ "CALL$" });
				bool isPut   = std::regex_search(ucPayoffType, std::regex{ "PUT$" });

				// OLD: double bendCallPut       = ucPayoffType.find("CALL") != std::string::npos ? -1.0 : ucPayoffType.find("PUT") != std::string::npos ?  1.0 : 0.0;
				double bendCallPut       = isCall ? -1.0 : (isPut ? 1.0 : 0.0);
				double bendParticipation = participation > 0.0                            ?  1.0 : participation < 0.0                           ? -1.0 : 0.0;
				double bendDirection     = bendCallPut * bendParticipation;
				if (ucPayoffType.find("FIXED") != std::string::npos) { bendDirection  = -1.0; }
				strike          = max(0.0,          atof(szAllPrices[colStrike]) + thisBarrierBend * bendDirection);
				// cap changed from barrierBend to thisBarrierBend ... can't think why we treated the cap differently to the strike
				// ... and changed again to reduce barrierBend by 50%, and 100% if cap < 0.1
				// ... actually lets reduce the cap barrierBend to zero ... there should be 1 amount of margin, shared by whichever strike is ausing the most gamma
				cap             = atof(szAllPrices[colCap]);
				cap             = max(-1.0,max(-1.0,cap - thisBarrierBend * bendDirection * 0.0));   
				int     underlyingFunctionId = atoi(szAllPrices[colUnderlyingFunctionId]);
				double  param1 = atof(szAllPrices[colParam1]);
				if (ucPayoffType.find("BASKET") != std::string::npos){
					param1 = max(0.0, param1 + thisBarrierBend*bendDirection);
				}
				
				/*
				* barrier creation
				*/
				spr.barrier.push_back(SpBarrier(numBarriers, barrierId, capitalOrIncome, nature, payoff, settlementDate, description,
					thisPayoffType, thisPayoffId, strike, cap, underlyingFunctionId, param1, participation, ulIdNameMap, avgDays, avgType,
					avgFreq, isMemory, isAbsolute, isStrikeReset, isStopLoss, isForfeitCoupons, barrierCommands, daysExtant, bProductStartDate, doFinalAssetReturn, midPrice,
					thisBarrierBend,bendDirection,spots,doDebug,debugLevel,annualFundingUnwindCost,productId,mydb,fixedCoupon,couponFrequency,
					couponPaidOut,spr.baseCurveTenor,spr.baseCurveSpread,productShape, doForwardValueCoupons || getMarketData));
				SpBarrier &thisBarrier(spr.barrier.at(numBarriers));
	
				// get barrier relations from DB
				enum {
					brcolBarrierRelationId = 0, brcolProductBarrierId,
					brcolUnderlyingId, brcolBarrier, brcolBarrierTypeId, brcolAbove, brcolAt, brcolStartDate, brcolEndDate,
					brcolTriggered, brcolIsAbsolute, brcolUpperBarrier, brcolWeight, brcolStrikeOverride, colBarrierRelationLast
				};
				// ** SQL fetch block
				sprintf(lineBuffer, "%s%s%s%d%s", "select * from ", useProto, "barrierrelation where ProductBarrierId='", barrierId, "' order by BarrierRelationId");  // WAS ordered by UnderlyingId ... for no reason I can think of
				mydb1.prepare((SQLCHAR *)lineBuffer, colBarrierRelationLast);
				retcode = mydb1.fetch(false,lineBuffer);
				// ...parse each barrierrelation row
				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
					double weight           = atof(szAllPrices[brcolWeight]);
					double strikeOverride   = atof(szAllPrices[brcolStrikeOverride]);
					bool   isAbsolute       = atoi(szAllPrices[brcolIsAbsolute]) == 1;
					barrierRelationId       = atoi(szAllPrices[brcolBarrierRelationId]);
					uid                     = atoi(szAllPrices[brcolUnderlyingId]);
					barrier                 = max(0.0,atof(szAllPrices[brcolBarrier]) + thisBarrierBend*bendDirection);
					uBarrier                = atof(szAllPrices[brcolUpperBarrier])   ;
					if (uBarrier > 999999 && uBarrier < 1000001.0) { uBarrier = NULL; } // using 1000000 as a quasiNULL, since C++ SQLFetch ignores NULL columns
					if (uBarrier != NULL){ uBarrier = max(0.0, uBarrier + barrierBend*bendDirection); }
					above                         = atoi(szAllPrices[brcolAbove]) == 1;
					at                            = atoi(szAllPrices[brcolAt]) == 1;
					startDateString               = szAllPrices[brcolStartDate];
					// PRIIPs Intermediate Scenario?
					endDateString                 = szAllPrices[brcolEndDate];
					// PRIIPs Intermediate Scenario?
					anyTypeId                     = atoi(szAllPrices[brcolBarrierTypeId]);
					bool   isContinuousALL        = _stricmp(barrierTypeMap[anyTypeId].c_str(), "continuousall") == 0;
					thisBarrier.isContinuousGroup = thisBarrier.isContinuousGroup || _stricmp(barrierTypeMap[anyTypeId].c_str(), "continuousgroup") == 0;
					thisBarrier.isContinuous      = thisBarrier.isContinuous || _stricmp(barrierTypeMap[anyTypeId].c_str(), "continuous") == 0;
					// express absolute levels as %ofSpot
					double thisStrikeDatePrice = ulPrices.at(ulIdNameMap[uid]).price[totalNumDays - 1 - daysExtant];
					// ...DOME only works with single underlying, for now...the issue is whether to add FixedStrike fields to each brel
					if (thisBarrier.isAbsolute)	{ 		// change fixed strike levels to percentages of spot
						thisBarrier.cap        /= thisStrikeDatePrice;
						thisBarrier.strike     /= thisStrikeDatePrice;
					}
					if (isAbsolute){
						barrier /= thisStrikeDatePrice;
						if (uBarrier != NULL) { uBarrier       /= thisStrikeDatePrice; }
						if (strikeOverride != 0.0)  { strikeOverride /= thisStrikeDatePrice; }
					}
					if (strikeOverride != 0.0)  { strikeOverride = max(0.0,strikeOverride + thisBarrierBend*bendDirection); }
				
					if (uid) {
						// create barrierRelation
						thisBarrier.brel.push_back(SpBarrierRelation(barrierRelationId,uid, barrier, uBarrier, isAbsolute, startDateString, endDateString,
							above, at, weight, daysExtant, strikeOverride != 0.0 ? strikeOverride : thisBarrier.strike, ulPrices.at(ulIdNameMap[uid]), 
							avgType, avgDays, avgFreq, avgInDays, avgInFreq, avgInAlgebra,productStartDateString,isContinuousALL,
							thisBarrier.isStrikeReset, thisBarrier.isStopLoss, thisBarrierBend, bendDirection));
					}
					// next barrierRelation record
					retcode = mydb1.fetch(false,"");
				}

				switch (thisBarrier.payoffTypeId) {
					// single-currency baskets
				case basketCallPayoff:	
				case basketPutPayoff:
					// quanto baskets	
				case basketCallQuantoPayoff:
				case basketPutQuantoPayoff:
					double basketFinal=0.0, basketRef=1.0;
					for (j=0; j<(int)thisBarrier.brel.size(); j++) {
						const SpBarrierRelation &thisBrel(thisBarrier.brel[j]);
						double w          = thisBrel.weight;
						int uid           = thisBrel.underlying;
						double finalPrice = ulPrices.at(ulIdNameMap[uid]).price[totalNumDays - 1];
						double refPrice   = ulPrices.at(ulIdNameMap[uid]).price[totalNumDays - 1 - daysExtant];
						basketFinal   += finalPrice / refPrice * w;
					}
					thisBarrier.strike  /= basketFinal / basketRef;
					break;
				}



				// detect extremumBarriers
				// DOME: for now ANY barrier with a brel which has different Start/End dates
				// NOTE: different brels can have different start dates
				// DOME:  need to check all brels have the same endDate
				bool isExtremumBarrier = false;
				for (i = 0; i < (int)thisBarrier.brel.size(); i++) {
					if (thisBarrier.brel[i].startDate != thisBarrier.brel[i].endDate 
						/* reinstated this next condition, otherwise strikeResets (which reset strike to spot on barrierStartDate) are set as continuousBarriers */ 
						&& !thisBarrier.isStrikeReset) {
						isExtremumBarrier = true;
						productNeedsFullPriceRecord = true;
					}
				}
				// ... force stopLoss barriers to be monitored daily, rather than be treated as an extremum, so stopLoss barriers do not need special processing in the simulator
				// ... we currently force american basket barriers to be evaluated on EACH date: otherwise need to recode much of the basket code
				thisBarrier.isExtremum = isExtremumBarrier && !thisBarrier.isStopLoss && (thisBarrier.payoffType.find("basket") == std::string::npos);

				// update vector of monitoring dates
				double thisEndDays = thisBarrier.getEndDays();
				if (thisEndDays <=0.0){
					if (find(accrualMonDateIndx.begin(), accrualMonDateIndx.end(), thisEndDays) == accrualMonDateIndx.end()) {
						accrualMonDateIndx.push_back((int)thisEndDays);
						accrualMonDateT.push_back(thisEndDays);
					}
				}
				else {
					// DOME: for now only use endDates, as all American barriers are detected below as extremum bariers
					if (thisBarrier.isExtremum || !thisBarrier.isContinuous || (thisBarrier.isStrikeReset && !thisBarrier.isStopLoss)){
						if (find(monDateIndx.begin(), monDateIndx.end(), thisEndDays) == monDateIndx.end()) {
							monDateIndx.push_back((int)thisEndDays);
							barrierMonDateIndx.push_back((int)thisEndDays);
							monDateT.push_back(thisEndDays);
							barrierMonDateT.push_back(thisEndDays);
						}
					}
					else {  // daily monitoring
						for (i = 0; i < (int)thisBarrier.brel.size(); i++) {
							const SpBarrierRelation &thisBrel(thisBarrier.brel[i]);
							if (thisBrel.startDate != thisBrel.endDate) {
								int startDays      = (thisBrel.bStartDate - bProductStartDate).days() - daysExtant;
								int endDays        = (thisBrel.bEndDate - bProductStartDate).days() - daysExtant;
								// don't want fair value simulations to redo history
								if (getMarketData && startDays < 0){ startDays = 0; }
								if (startDays < thisBarrier.startDays){
									thisBarrier.startDays = startDays;
								}
								for (j=startDays; j <= endDays; j++){
									if (find(monDateIndx.begin(), monDateIndx.end(), j) == monDateIndx.end()) {
										monDateIndx.push_back(j);
										barrierMonDateIndx.push_back(j);
										// don't think these should have divided by 365.25 ... so leaving monDateT as #days
										monDateT.push_back((double)j);
										barrierMonDateT.push_back((double)j);
										//  monDateT.push_back((double)j / 365.25);
										//  barrierMonDateT.push_back((double)j / 365.25);
									}
								}
							}
						}

					}
				}	

				

				// any other init
				int thisEndDaysInt = (int)thisEndDays;
				if (doTimepoints && std::find(tempTimepointDays.begin(), tempTimepointDays.end(), thisEndDaysInt) == tempTimepointDays.end()) { tempTimepointDays.push_back(thisEndDaysInt); }
				if (thisEndDays > maxBarrierDays){ maxBarrierDays = thisEndDays; }

				// next barrier record
				numBarriers += 1;
				retcode = mydb.fetch(false,"");
			}

			// calculate cds vols
			if (issuerCallable && thisNumIterations > 1) {
				for (int possibleIssuerIndx=0; possibleIssuerIndx < (int)theseIssuerIds.size(); possibleIssuerIndx++) {
					int counterpartyId = theseIssuerIds[possibleIssuerIndx];
					double cdsVol(0.01);    // default  1%pa vol
					sprintf(lineBuffer, "%s%d%s%lf%s%lf%s"
						, "select std(Spread)*sqrt(365.25/7)/10000 vol from cdsspreadarchive where InstitutionId="
						, counterpartyId
						, " and Maturity >= "
						, (maxBarrierDays - 365) / 365
						, " and Maturity <= least(7,"
						, (maxBarrierDays + 365) / 365
						, ") and LastDataDate != '0000-00-00' HAVING vol != NULL");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
					retcode = mydb.fetch(false, lineBuffer);
					if (retcode != MY_SQL_GENERAL_ERROR && retcode != SQL_NO_DATA_FOUND) {
						cdsVol = atof(szAllPrices[0]);
					}
					cdsVols.push_back(cdsVol);
				}
			}

			//	add vol tenors to MonDates
			spr.reportableMonDateIndx = monDateIndx;
			if ((getMarketData || useUserParams)){
				int  maxObsDays = monDateIndx.size() > 0 ? monDateIndx[monDateIndx.size() - 1] : 1000000;
				for (i = 0; i < ulVolsTenor[0].size(); i++) {
					double thisT  = 365.25*ulVolsTenor[0][i];
					int theseDays = (int)floor(thisT);
					if (maxObsDays >= theseDays && find(monDateIndx.begin(), monDateIndx.end(), theseDays) == monDateIndx.end()) {
						monDateIndx.push_back((int)theseDays);
						volsMonDateIndx.push_back((int)theseDays);
						monDateT.push_back(thisT);
						volsMonDateT.push_back(thisT);
					}
				}
				sort(monDateIndx.begin(), monDateIndx.end());
				sort(monDateT.begin(), monDateT.end());				
			}


			// possibly pad future ulPrices for resampling into if there is not enough history
			int daysPadding = (int)max(maxBarrierDays, maxBarrierDays + daysExtant - totalNumDays + 1);
			boost::gregorian::date  bTempDate = bLastDataDate;
			while (daysPadding>0){
				bTempDate += boost::gregorian::days(1);
				string tempString = boost::gregorian::to_iso_extended_string(bTempDate);
				sprintf(charBuffer, "%s", tempString.c_str());
				for (i = 0; i < numUl + addCompoIntoCcy; i++) {
					ulOriginalPrices.at(i).date.push_back(charBuffer);						ulPrices.at(i).date.push_back(charBuffer);
					ulOriginalPrices.at(i).price.push_back(0.0);							ulPrices.at(i).price.push_back(0.0);
					// DOME: crudely mimic-ing weekends
					int  dayOfWeek           = daysPadding % 7;
					bool mimicNonTradingDay  = (dayOfWeek == 5) || (dayOfWeek == 6);
					ulOriginalPrices.at(i).nonTradingDay.push_back(mimicNonTradingDay);		ulPrices.at(i).nonTradingDay.push_back(mimicNonTradingDay);
				}
				daysPadding -= 1;
			}


			// remove any timepointDays 
			vector<int> timepointDays;
			if (doTimepoints){
				for (i=0; i<(int)tempTimepointDays.size(); i++){
					if (tempTimepointDays[i] <= maxBarrierDays){
						timepointDays.push_back(tempTimepointDays[i]);
					}
				}
			}
			int numTimepoints  = (int)timepointDays.size();
			vector<string> timepointNames;
			if (doTimepoints){
				sort(timepointDays.begin(), timepointDays.end());
				// space for timepoint stats [timepointDays][uid][]
				for (i=0; i<numTimepoints; i++){
					sprintf(lineBuffer,"%d%s",timepointDays[i],"d");
					timepointNames.push_back(lineBuffer);
				}
				// put in any barrier names
				for (i=0; i < numBarriers; i++) {
					const SpBarrier&    b(spr.barrier.at(i));
					int thisEndDays = (int)b.endDays;
					bool done = false;
					for (i=0; !done && i < (int)timepointDays.size(); i++){
						if (thisEndDays == timepointDays[i]){
							done = true;
							sprintf(lineBuffer, "%s %s", timepointNames.at(i).c_str(), b.description.c_str());
							timepointNames[i] = lineBuffer;
						}
					}
				}
				// init database
				sprintf(lineBuffer, "%s%d%s%d", "delete from timepoints where userid=", userId, " and productid=", productId);
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				sprintf(lineBuffer, "%s%d%s%d", "delete from path where userid=", userId, " and productid=", productId);
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				/*
				retcode = mydb.fetch(true,lineBuffer);
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
					cerr << "Problem deleting: " << lineBuffer << endl;
				}
				*/
				
			}






			// further initialisation, given product info
						// calculate oncurve vol
			double oncurveVol(0.1); // default 10%pa vol
			if (issuerCallable && thisNumIterations > 1) {
				sprintf(lineBuffer, "%s%s%s%lf%s%lf%s"
					, "select std(Rate)*sqrt(365.25/7)/100 from oncurvearchive where ccy='"
					, productCcy.c_str()
					, "' and Tenor >= "
					, (maxBarrierDays - 366) / 365
					, " and Tenor <= least(7,"
					, (maxBarrierDays + 366) / 365
					, ") and LastDataDate != '0000-00-00' ");
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				retcode = mydb.fetch(false, lineBuffer);
				if (retcode != MY_SQL_GENERAL_ERROR) {
					oncurveVol = atof(szAllPrices[0]);
				}
			}
			spr.oncurveVol = oncurveVol;

			// ...check product not matured
			if ((int)monDateIndx.size() == 0 && (int)accrualMonDateIndx.size() == 0){ continue; }
			spr.maxProductDays = (int)maxBarrierDays + daysExtant;
			// enough data?
			if (totalNumDays<2 || (thisNumIterations<2 && totalNumDays < spr.maxProductDays)){
				cerr << "Not enough data for product#:" << productId << endl;
				if (doDebug){ exit(110); }
				continue;
			}
			double maxYears = 0; for (i = 0; i<numBarriers; i++) { double t = spr.barrier.at(i).yearsToBarrier;   if (t > maxYears){ maxYears = t; } }
			for (int possibleIssuerIndx=0; possibleIssuerIndx < (int)theseIssuerIds.size(); possibleIssuerIndx++) {
				buildHazardCurve(cdsSpreads[possibleIssuerIndx], cdsTenors[possibleIssuerIndx], maxYears, recoveryRate, hazardCurves[possibleIssuerIndx]);
			}
			// initialise product, now we have all the state
			spr.init(maxYears);
			productNeedsFullPriceRecord = forceFullPriceRecord || productNeedsFullPriceRecord;

			

			// possibly impose user-defined view of expectedReturn: only need to bump ulReturns


			// make convexity drift adjustment, from vol from daily continuous returns
			// ... this will mimic a lognormal price process for underlyings, which is the PRIIPs approach
			// ... on the other hand, an empirical distribution of returns simply says each return is equally likely on any given day
			// so omit for non-PRIIPs analysis
			vector<double> calendarDailyVariance;
			if (doPriips){
				// do convexity adjustment
				for (i = 0; i < numUl; i++) {
					originalUlReturns[i] = ulReturns[i];
					vector<double> thisSlice;
					// calc vol from daily continuous returns
					for (j = 0; j < (int)ulReturns[0].size() - 1; j++) {
						if (!ulOriginalPrices[i].nonTradingDay[j]){
							thisSlice.push_back(log(ulReturns[i][j]));
						}
					}
					double sliceMean, sliceStdev, sliceStderr;
					const double volScalingFactor(sqrt(253.0 / 365.25));
					MeanAndStdev(thisSlice, sliceMean, sliceStdev, sliceStderr);
					calendarDailyVariance.push_back(sliceStdev*sliceStdev * volScalingFactor);
					double thisDailyDriftCorrection = exp(-0.5*calendarDailyVariance[i]);
					double thisAnnualDriftCorrection = exp(-0.5*calendarDailyVariance[i] * 365.25);
					// change underlyings' drift rate
					for (j = 0; j < (int)ulReturns[i].size(); j++) {
						ulReturns[i][j] *= thisDailyDriftCorrection;
					}
				}				
			} // doPriips
			
			// issuerCallable ... turn off non-terminal Capital barriers
			if (issuerCallable){
				// find max(b.endDays)
				int maxEndDays(0);
				for (i=0; i < numBarriers; i++){
					const SpBarrier&    b(spr.barrier.at(i));
					if (b.capitalOrIncome){
						if (daysToFirstCapitalBarrier == 0) { daysToFirstCapitalBarrier  = b.endDays; }
						if (b.endDays > maxEndDays){ maxEndDays = b.endDays; }
					}
				}
				spr.maxEndDays = maxEndDays;
				// turn off barriers with (b.capitalOrIncome && b.endDays < maxEndDays)
				for (i=0; i < numBarriers; i++){
					SpBarrier&    b(spr.barrier.at(i));
					if ((b.capitalOrIncome && b.endDays < maxEndDays && b.endDays > 0) 
						// || (!b.capitalOrIncome && b.endDays == maxEndDays)
						){
						b.setIsNeverHit();
					}
				}
			}
			// get accrued coupons
			double accruedCoupon(0.0);
			bool   productHasMatured(false);
			EvalResult accrualEvalResult(0.0, 0.0, 0);
			
			
			// issuerCallable: since cannot control when Issuer might call, look for stale price 
			if (issuerCallable && stalePrice && daysToFirstCapitalBarrier <0){
				productHasMatured = true;
				// update DB with last MID
				sprintf(lineBuffer, "%s%lf%s%s%s%d%s%d", "update product set matured=1,MaturityPayoff=", (bidPrice + askPrice) / 2.0, ",DateMatured='", bidAskDateString.c_str(), "' where userid=", userId, " and productid=", productId);
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
			}
			else {
				accrualEvalResult = spr.evaluate(totalNumDays, totalNumDays - 1, totalNumDays, 1, historyStep, ulPrices, ulReturns,
					numBarriers, numUl, ulIdNameMap, accrualMonDateIndx, accrualMonDateT, recoveryRate, hazardCurves, mydb, accruedCoupon, true, false, doDebug, debugLevel, startTime, benchmarkId, benchmarkMoneyness,
					contBenchmarkTER, hurdleReturn, false, false, timepointDays, timepointNames, simPercentiles, false, useProto, false /* getMarketData */, useUserParams, thisMarketData,
					cdsTenors, cdsSpreads, fundingFraction, productNeedsFullPriceRecord, false, thisFairValue, false, false, productHasMatured, /* priipsUsingRNdrifts */ false,
					/* updateCashflows */false,/* issuerIndx */0);
			}
			// ...check product not matured
			numMonPoints = (int)monDateIndx.size();
			if (productHasMatured || !numMonPoints || (numMonPoints == 1 && monDateIndx[0] == 0)){ continue; }


			//
			//  impose any rescaling of market data
			//
			if (!extendingPrices && doRescaleSpots){
				for (i=0; i < numUl; i++){
					bumpSpots(spr, i, ulIds, spots, ulPrices, doStickySmile, thisMarketData, thisMarketData.ulVolsStrike, rescaleFraction - 1.0, totalNumDays, daysExtant, false);
					spots[i] = ulPrices[i].price[totalNumDays - 1];
				}
			}


			
			// finally evaluate the product...1000 iterations of a 60barrier product (eg monthly) = 60000
			// *** this call to evaluate() establishes baseCase "thisFairValue" for subsequent bumps
			spr.productDays    = *max_element(monDateIndx.begin(), monDateIndx.end());
			int thisStartPoint =  thisNumIterations == 1 ? daysExtant : totalNumDays - 1;
			int thisLastPoint  =  thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays; /*daysExtant + 1*/
			if (!doPriips && (thisNumIterations>1 || thisStartPoint<thisLastPoint)){
				
				EvalResult evalResult(0.0, 0.0, 0), evalResultBump(0.0, 0.0, 0);
				// multiIssuer issuerCallable products get .evaluated repeatedly cos product cashflows depend on each issuer's cds and cdsVol
				for (int thisIssuerIndx = 0; 
					thisIssuerIndx < (int)theseIssuerIds.size() && (thisIssuerIndx < 1 || (multiIssuer && issuerCallable && evalResult.errorCode == 0 ) ); 
					thisIssuerIndx++) {
					spr.ArtsRanInit();  // so that multiIssuer analysis always sees the same ran sequence
					// first-time we set conserveRande=doBumps and consumeRande=false
					srand((unsigned int)time(0)); // reseed rand
					evalResult = spr.evaluate(totalNumDays, thisStartPoint, thisLastPoint, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
						numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurves, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, debugLevel, startTime, benchmarkId, benchmarkMoneyness,
						contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
						useProto, getMarketData, useUserParams, thisMarketData, cdsTenors, cdsSpreads, fundingFraction, productNeedsFullPriceRecord,
						ovveridePriipsStartDate, thisFairValue, doBumps || solveFor /* conserveRands */, false /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,
						/* updateCashflows */!doBumps && !solveFor && !doRescale && !useMyEqEqCorr && !useMyEqFxCorr && updateCashflows && thisIssuerIndx == 0,/* issuerIndx */thisIssuerIndx);
				}
				if (evalResult.errorCode != 0) {
					continue;
				}

				/*
				*  Newton-Raphson root finding of some product parameter "solverParam" that gives FV = targetFairValue
				*/
				if (solveFor){
					// Newton-Raphson settings
					// ... f is (FV - targetFairValue) for some value "currentSolution" of x
					// ... which we want to be zero
					int maxit    = 100;
					double xacc  = 0.0001;      // 10bp accuracy
					double xInitialLo    =  0.5 ;       // lower bound guess
					double xInitialHi    =  2.0 ;       // upper bound guess
					double solverStep = 0.03;   // each iteration calculates FV multiplying solverParam by  
												//   a) currentSolution and 
											    //   b) (currentSolution PLUS solverStep)
												// ... so solverStep controls how local the slope is calculated ... don't want it too small
					double solverParam(0.0);
					int  j;
					double  dx,  // CURRENT NewtonRaphson step
						dxold,   // PREVIOUS dx  ... to be compared with dx to see if converging fast enough
						f,       // function value,(FV - targetFairValue), setting param  = solverParam*currentSolution
						f2,      // function value,(FV - targetFairValue), setting param  = solverParam*(currentSolution + solverStep)
						fSlope,  // change in f upon increasing x by solverStep
						fh,      // (FV - targetFairValue) at HIGH bracket
						fl,      // (FV - targetFairValue) at LOW  bracket
						temp, 
						xHi,      // HIGH bracket for solution 
						xLo,      // LOW  bracket for solution 
						currentSolution;  // root-temporary-solution
					EvalResult evalResult1(0.0, 0.0,0), evalResult2(0.0, 0.0,0);
					string adviceString = " - please choose a TargetValue closer to the current FairValue, or modify the product so as to have a FairValue closer to your TargetValue";
					// check product has some starting data
					bool couponFound(false), putFound(false), lastCapFound(false), digitalFound(false), positiveParticipationFound(false), positivePutParticipationFound(false), shortPutStrikeFound(false), autocallTriggerFound(false);
					int numIncomeBarriers(0);
					double coupon(0.0);
					double previousBarrierYears(0.0);

					// solve
					switch (solveForThis){
					case solveForCoupon:
						// first see if there are any income barriers
						for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
							if (!spr.barrier.at(thisBarrier).capitalOrIncome) { numIncomeBarriers  += 1; }
						}
						// now get an ANNUAL coupon
						for (j=0; !couponFound && j < numBarriers; j++){
							SpBarrier& b(spr.barrier.at(j));
							if (!b.capitalOrIncome || (numIncomeBarriers == 0 && b.payoffTypeId == fixedPayoff && (int)b.brel.size()>0)){
								couponFound = true;
								// capitalBarrier: coupon times CUMULATIVE years; incomeBarriers: coupon times INTERVAL years
								coupon      = (b.payoff - (b.capitalOrIncome ? 1.0 : 0.0)) / (b.capitalOrIncome ? b.totalBarrierYears : (b.totalBarrierYears - previousBarrierYears));								
								previousBarrierYears = b.totalBarrierYears;
							}
						}
						if (!couponFound){
							sprintf(lineBuffer, "%s%s%s", "solveFor:0:", whatToSolveFor.c_str(), ":no coupons found");
							std::cout << lineBuffer << std::endl;
							return(105);
						}
						if (coupon == 0.0){
							coupon = 0.1; // default 10%pa 
						}
						solverParam = coupon;
						xInitialLo    =  0.25;
						xInitialHi    =  4.0;
						break;
					case solveForPutBarrier:
						for (j=0; !putFound && j < numBarriers; j++){
							SpBarrier& b(spr.barrier.at(j));
							if (b.capitalOrIncome && (b.payoffTypeId == putPayoff || b.payoffTypeId == basketPutPayoff) && b.participation < 0.0 && (int)b.brel.size()>0){
								switch (b.payoffTypeId) {
									case putPayoff:
										putFound    = true;
										solverParam = b.brel[0].barrier;
										break;
									case basketPutPayoff:
										putFound    = true;
										solverParam = b.param1;
										break;
								}
							}
						}
						if (!putFound){
							sprintf(lineBuffer, "%s%s%s", "solveFor:0:", whatToSolveFor.c_str(), ":no put barrier found");
							std::cout << lineBuffer << std::endl;
							return(105);
						}
						xInitialLo    =  0.5;
						xInitialHi    =  1.5 / solverParam;  // max 1.5
						break;
					case solveForAutocallTrigger:
						// look for FIRST capitalOrIncome obs with payoffType == 'fixed' and hasBrels
						for (j=0; !autocallTriggerFound && j < numBarriers; j++) {
							SpBarrier& b(spr.barrier.at(j));
							if (b.capitalOrIncome && b.payoffTypeId == fixedPayoff && (int)b.brel.size() > 0) {
								autocallTriggerFound     = true;
								solverParam              = b.brel[0].barrier;
							}
						}
						if (!autocallTriggerFound) {
							sprintf(lineBuffer, "%s%s%s", "solveFor:0:", whatToSolveFor.c_str(), ":no autocall trigger found");
							std::cout << lineBuffer << std::endl;
							return(105);
						}
						xInitialLo    =  0.5;
						xInitialHi    =  2.0;
						break;
					case solveForLastCallCap:
						for (j=numBarriers-1; !lastCapFound && j >= 0; j--) {
							SpBarrier& b(spr.barrier.at(j));
							if ((b.payoffTypeId == callPayoff || b.payoffTypeId == basketCallPayoff) && b.participation > 0.0 && (int)b.brel.size()>0 && b.cap > 0.0 && b.strike > 0.0) {
								lastCapFound = true;
								solverParam  = b.cap;
							}
						}
						if (!lastCapFound) {
							sprintf(lineBuffer, "%s%s%s", "solveFor:0:", whatToSolveFor.c_str(), ":no lastCallCap found");
							std::cout << lineBuffer << std::endl;
							return(105);
						}
						xInitialLo    =  0.5;
						xInitialHi    =  2.0;
						break;
					case solveForDigital:
						// look for [productShape == digital &&] last payoffType == 'fixed' and hasBrels
						for (int j=numBarriers - 1; !digitalFound && j >= 0; j--) {
							SpBarrier& b(spr.barrier.at(j));
							if (/* productShape == "Digital" &&  */ b.payoffTypeId == fixedPayoff && (int)b.brel.size() > 0) {
								digitalFound = true;
								solverParam  = b.payoff;
							}
						}
						if (!digitalFound) {
							sprintf(lineBuffer, "%s%s%s", "solveFor:0:", whatToSolveFor.c_str(), ":no digital found");
							std::cout << lineBuffer << std::endl;
							return(105);
						}
						xInitialLo    =  0.5;
						xInitialHi    =  2.0;
						break;
					case solveForPositiveParticipation:
						// look for LAST participation > 0 && payoffType != 'fixed' and hasBrels
						for (int j=numBarriers - 1; !positiveParticipationFound && j >= 0; j--) {
							SpBarrier& b(spr.barrier.at(j));
							if (b.payoffTypeId != fixedPayoff && b.participation > 0.0 && (int)b.brel.size() > 0) {
								positiveParticipationFound = true;
								solverParam = b.participation;
							}
						}
						if (!positiveParticipationFound) {
							sprintf(lineBuffer, "%s%s%s", "solveFor:0:", whatToSolveFor.c_str(), ":no positiveParticipation found");
							std::cout << lineBuffer << std::endl;
							return(105);
						}
						xInitialLo    =  0.5;
						xInitialHi    =  2.0;
						break;
					case solveForPositivePutParticipation:
						// look for LAST participation > 0 && payoffType = 'put' and hasBrels
						for (int j=numBarriers - 1; !positivePutParticipationFound && j >= 0; j--) {
							SpBarrier& b(spr.barrier.at(j));
							if (b.payoffTypeId == putPayoff && b.participation > 0.0 && (int)b.brel.size() > 0) {
								positivePutParticipationFound = true;
								solverParam = b.participation;
							}
						}
						if (!positivePutParticipationFound) {
							sprintf(lineBuffer, "%s%s%s", "solveFor:0:", whatToSolveFor.c_str(), ":no positivePutParticipation found");
							std::cout << lineBuffer << std::endl;
							return(105);
						}
						xInitialLo    =  0.5;
						xInitialHi    =  2.0;
						break;
					case solveForShortPutStrike:
						// look for LAST participation < 0 && payoffType == 'put' and hasBrels
						for (int j=numBarriers - 1; !shortPutStrikeFound && j >= 0; j--) {
							SpBarrier& b(spr.barrier.at(j));
							if (b.payoffTypeId == putPayoff && b.participation < 0.0 && (int)b.brel.size() > 0) {
								shortPutStrikeFound = true;
								solverParam         = b.strike;
							}
						}
						if (!shortPutStrikeFound) {
							sprintf(lineBuffer, "%s%s%s", "solveFor:0:", whatToSolveFor.c_str(), ":no shortPutStrike found");
							std::cout << lineBuffer << std::endl;
							return(105);
						}
						xInitialLo    =  0.5;
						xInitialHi    =  1.5/solverParam; // max 1.5
						break;
					} // switch

					// possibly done already
					if (abs(evalResult.value - targetFairValue) < xacc){
						sprintf(lineBuffer, "%s%s%s%.4lf", "solveFor:1:", whatToSolveFor.c_str(), ":", solverParam);
						std::cout << lineBuffer << std::endl;
						return(0);
					}


					// initial values at upper/lower bound
					// try highBracket xInitialHi
					spr.solverSet(solveForThis, solverParam*xInitialHi);
					cerr << "    Initial HIGH bracket multiplier:" << xInitialHi << " giving paramValue:" << solverParam*xInitialHi << endl;
					evalResult2 = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
						numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurves, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, debugLevel, startTime, benchmarkId, benchmarkMoneyness,
						contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
						useProto, getMarketData, useUserParams, thisMarketData, cdsTenors, cdsSpreads, fundingFraction, productNeedsFullPriceRecord,
						ovveridePriipsStartDate, thisFairValue, doBumps /* conserveRands */, true /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,
						/* updateCashflows */false,/* issuerIndx */0);
					fh = evalResult2.value - targetFairValue;
					// are we done?
					if (abs(fh) < xacc) {
						if (solveForCommit) { spr.solverCommit(solveForThis, solverParam*xInitialHi); }
						sprintf(lineBuffer, "%s%s%s%.4lf", "solveFor:1:", whatToSolveFor.c_str(),":", solverParam*xInitialHi);
						std::cout << lineBuffer << std::endl;
						return(0);
					}
					// try lowBracket xInitialLo
					spr.solverSet(solveForThis, solverParam*xInitialLo);
					cerr << "    Initial LOW  bracket multiplier:" << xInitialLo << " giving paramValue:" << solverParam * xInitialLo << endl;
					evalResult1 = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
						numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurves, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, debugLevel, startTime, benchmarkId, benchmarkMoneyness,
						contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
						useProto, getMarketData, useUserParams, thisMarketData, cdsTenors, cdsSpreads, fundingFraction, productNeedsFullPriceRecord,
						ovveridePriipsStartDate, thisFairValue, doBumps /* conserveRands */, true /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,
						/* updateCashflows */false,/* issuerIndx */0);
					fl = evalResult1.value - targetFairValue;
					// are we done?
					if (abs(fl) < xacc) {
						if (solveForCommit) { spr.solverCommit(solveForThis, solverParam*xInitialLo); }
						sprintf(lineBuffer, "%s%s%s%.4lf", "solveFor:1:", whatToSolveFor.c_str(), ":", solverParam*xInitialLo);
						std::cout << lineBuffer << std::endl;
						return(0);
					}

					// handle when not bracketed
					if ((fl>0.0 && fh>0.0) || (fl<0.0 && fh<0.0)){
						sprintf(lineBuffer, "%s%s%s%s", "solveFor:0:", whatToSolveFor.c_str(), ":noSolution", adviceString.c_str());
						std::cout << lineBuffer << std::endl;
						return(0);
					}
					// orient the search so that f(xLo)<0
					//    fl was done using xInitialLo
					//    fh was done using xInitialHi
					if (fl<0.0){ xLo=xInitialLo; xHi=xInitialHi; }
					else       { xHi=xInitialLo; xLo=xInitialHi; }
					currentSolution   = 0.5*(xInitialLo + xInitialHi);      // initial guess for root
					dxold             = abs(xInitialHi - xInitialLo);       // stepsize before last
					dx                = dxold;              // last stepsize
					// initial f and fSlope
					spr.solverSet(solveForThis, solverParam*currentSolution);
					cerr << "    Initial MIDWAY multiplier:" << currentSolution << " giving paramValue:" << solverParam * currentSolution << endl;
					evalResult1 = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
						numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurves, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, debugLevel, startTime, benchmarkId, benchmarkMoneyness,
						contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
						useProto, getMarketData, useUserParams, thisMarketData, cdsTenors, cdsSpreads, fundingFraction, productNeedsFullPriceRecord,
						ovveridePriipsStartDate, thisFairValue, doBumps /* conserveRands */, true /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,
						/* updateCashflows */false,/* issuerIndx */0);
					f = evalResult1.value - targetFairValue;
					cerr << "    Initial MIDWAY multiplier + STEP of " << solverStep << " giving multiplier:" << (currentSolution + solverStep) << " giving paramValue:" << solverParam * (currentSolution + solverStep) << endl;
					spr.solverSet(solveForThis, solverParam*(currentSolution + solverStep));
					evalResult2 = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
						numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurves, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, debugLevel, startTime, benchmarkId, benchmarkMoneyness,
						contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
						useProto, getMarketData, useUserParams, thisMarketData, cdsTenors, cdsSpreads, fundingFraction, productNeedsFullPriceRecord,
						ovveridePriipsStartDate, thisFairValue, doBumps /* conserveRands */, true /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,
						/* updateCashflows */false,/* issuerIndx */0);
					f2 = evalResult2.value - targetFairValue;
					fSlope = (f2 - f) / solverStep; 
					cerr << "      INITIAL SLOPE:" << fSlope << " stepped-f:" << f2 << " f:" << f << endl;
					// NewtonRaphson iterate
					// if f <  0.0 we will move LOW  bracket UP     to currentSolution
					// if f >= 0.0 we will move HIGH bracket DOWN   to currentSolution
					bool newtonOutOfRange(false);
					bool notDecreasingFastEnough(false);
					for (j=0; j<maxit; j++){
						//
						// normal NR iterations will REDUCE currentSolution by dx = f/fSlope
						// ... making f zero if f is linear
						//

						// BuT 2 possible problems (solved by bisection):
						// 1. Newton out of range 
						//     ... recall that last iteration moved either xLo or xHi to currentSolution
						newtonOutOfRange  = (  (currentSolution - xHi)*fSlope - f  )      //   A = NEGATIVE if 
																						  //   either: f >= 0.0  ... cos we have just changed xHi to currentSolution: evaluates to -f
																						  //       or: if (currentSolution - xHi) < next dx (= f/fSlope) 
											       	        *                             // times
										    (  (currentSolution - xLo)*fSlope - f )       //   B = NEGATIVE if
																						  //   either: f <  0.0  ... cos we have just changed xLo to currentSolution: evaluates to -f
																						  //       or: if (currentSolution - xLo) < next dx (= f/fSlope) 
														> 0.0;                            // if A and B have same sign, next NR step-dx = f/fSlope will change currentSolution to a value
														                                  //  ... which is either below xLo or above xHi ie will not bracket the root
																				          //  ... so we reset currentSolution to halfway between xHi and xLo
						// or 2. f not decreasing fast enough: the previous dx would only move f by half
						notDecreasingFastEnough = abs(f*2.0) > abs(dxold*fSlope);
						if (newtonOutOfRange ||  notDecreasingFastEnough){
							// bisection works because:
							// ... Evaluate f at halfway between root-containing bounds xLo and xHi, and examine its sign
							// ... replace whichever limit xLo or xHi has the same sign
							// ... After each iteration the bounds containing the root decrease by a factor of two
							dxold              = dx; 
							dx                 = 0.5*(xHi - xLo); // NewtonRaphson step 
							currentSolution    = xLo + dx;        // move currentSolution to halfway between xHi and xLo
							if (xLo == currentSolution){
								cerr << "NR bisect has zero step:" << dx << endl;
								if (solveForCommit) { spr.solverCommit(solveForThis, solverParam*currentSolution); }
								sprintf(lineBuffer, "%s%s%s%.4lf", "solveFor:1:", whatToSolveFor.c_str(), ":", solverParam*currentSolution);
								std::cout << lineBuffer << std::endl;
								return(0);
							}                       // finish if change in root negligible
							cerr << "    NR bisecting because" << (newtonOutOfRange ? " newtonOutOfRange" : " notDecreasingFastEnough") << " x-h:" << xHi << " x-l:" << xLo <<" so bisected by:" << dx << " to multiplier:" << currentSolution << endl;
						}
						else { // Newton step acceptable - take it ie move ALL the way along slope
							dxold             = dx; 
							temp              = currentSolution;
							dx                = f / fSlope;        // REDUCING currentSolution by dx will result in a value where f would be zero (if f is linear)
							currentSolution   -= dx;
							// finish if change in root negligible
							if (temp == currentSolution){
								cerr << "NR has zero step:" << dx << endl;
								if (solveForCommit) { spr.solverCommit(solveForThis, solverParam*currentSolution); }
								sprintf(lineBuffer, "%s%s%s%.4lf", "solveFor:1:", whatToSolveFor.c_str(), ":", solverParam*currentSolution);
								std::cout << lineBuffer << std::endl;
								return(0);
							}
							// otherwise change currentSolution by dx
							cerr << "    NR IN-RANGE dx-step:" << -dx << " to multiplier:" << currentSolution << " giving paramValue:" << solverParam * currentSolution  << endl;
						}
						// convergence?
						if (abs(dx) < xacc){
							cerr << "    NR converged stepSize was:" << dx << " multiplier is:" << currentSolution << " giving paramValue:" << solverParam * currentSolution << endl;
							if (solveForCommit) { spr.solverCommit(solveForThis, solverParam*currentSolution); }
							sprintf(lineBuffer, "%s%s%s%.4lf", "solveFor:1:", whatToSolveFor.c_str(), ":", solverParam*currentSolution);
							std::cout << lineBuffer << std::endl;
							return(0);
						}
						// calc f and fSlope
						// ... calc f  at currentSolution
						spr.solverSet(solveForThis, solverParam*currentSolution);
						cerr << "    try param (no step):" << " multiplier is:" << currentSolution << " giving paramValue:" << solverParam * currentSolution << endl;
						evalResult1 = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
							numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurves, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, debugLevel, startTime, benchmarkId, benchmarkMoneyness,
							contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
							useProto, getMarketData, useUserParams, thisMarketData, cdsTenors, cdsSpreads, fundingFraction, productNeedsFullPriceRecord,
							ovveridePriipsStartDate, thisFairValue, doBumps /* conserveRands */, true /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,
							/* updateCashflows */false,/* issuerIndx */0);
						f = evalResult1.value - targetFairValue;						
						// ... calc f  at currentSolution PLUS solverStep
						cerr << "        for SLOPE try param (stepped) by:" << solverStep << " multiplier is:" << currentSolution + solverStep << " giving paramValue:" << solverParam * (currentSolution + solverStep) << endl;
						spr.solverSet(solveForThis, solverParam*(currentSolution + solverStep));
						evalResult2 = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
							numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurves, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, debugLevel, startTime, benchmarkId, benchmarkMoneyness,
							contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
							useProto, getMarketData, useUserParams, thisMarketData, cdsTenors, cdsSpreads, fundingFraction, productNeedsFullPriceRecord,
							ovveridePriipsStartDate, thisFairValue, doBumps /* conserveRands */, true /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,
							/* updateCashflows */false,/* issuerIndx */0);
						f2 = evalResult2.value - targetFairValue;
						// ... calculate slope fSlope
						fSlope = (f2 - f) / solverStep;    // slope
						cerr << "      SLOPE:" << fSlope << " stepped-f:" << f2 << " f:" << f << endl;

						// maintain the bracket on the root
						// ... either xLo or xHi will be moved to currentSolution
						if (f<0.0){ // FV below target
							xLo = currentSolution; // move LOW bracket UP     to currentSolution (where f is negative)
						}
						else {      // FV above target
							xHi = currentSolution; // move HIGH bracket DOWN  to currentSolution (where f is positive)
						} 
						cerr << "    After this iteration BRACKETS HIGH x-h:" << xHi << " LOW x-l:" << xLo << " multiplier:" << currentSolution << " paramValue:" << solverParam * currentSolution << endl;
					}  // for iterate

					{ //alert("IRR root-finding: iterations exHiausted"); 
						sprintf(lineBuffer, "%s%s%s%s", "solveFor:0:", whatToSolveFor.c_str(), ": iterations exhausted", adviceString.c_str());
						std::cout << lineBuffer << std::endl;
						return(0);
					}
				}  // END solveFor


				
				
				double deltaBumpAmount(0.0), vegaBumpAmount(0.0), rhoBumpAmount(0.0), creditBumpAmount(0.0), corrBumpAmount(0.0),bumpedFairValue(0.0);
				int    thetaBumpAmount(0);				
				if (doBumps && (deltaBumps || vegaBumps || thetaBumps || rhoBumps || creditBumps || bumpPointTenor > 0.0)  /* && daysExtant>0 */){
					vector< vector<vector<double>> >  holdUlFwdVol(thisMarketData.ulVolsFwdVol);
					vector< vector<vector<double>> >  holdUlImpVol(thisMarketData.ulVolsImpVol);
					vector<vector<vector<double>>>    holdUlVolsStrike(thisMarketData.ulVolsStrike);
					vector<double>                    holdCdsSpread(cdsSpreads[0]);
					vector<SomeCurve>                 holdBaseCurve(baseCurve);
					vector <int>                      holdMonDateIndx;
					vector <double>                   holdMonDateT;
					for (j=0; j < monDateIndx.size(); j++){
						holdMonDateIndx.push_back(monDateIndx[j]);
						holdMonDateT.push_back(monDateT[j]);
					}
					// bumpVolPoint
					if (bumpPointTenor > 0.0){
						bool done;
						for (i=0; i < numUl; i++){
							int tenorIndx  = 0;
							int strikeIndx = 0;
							// find tenor index
							done = false;
							for (j=0; !done && j < ulVolsTenor[i].size();j++){
								if (fabs(bumpPointTenor - ulVolsTenor[i][j]) < 0.03){
									done = true;
								}
							}
							tenorIndx = j-1;
							// find strike index
							done = false;
							for (j=0; !done && j < ulVolsStrike[i][tenorIndx].size(); j++){
								if (fabs(bumpPointStrike - ulVolsStrike[i][tenorIndx][j]) < 0.03){
									done = true;
								}
							}
							strikeIndx = j-1;
							vector<double> &thisVolSlice(bsPricer ? ulVolsBsImpVol[i][tenorIndx] : ulVolsBumpedLocalVol[i][tenorIndx]);
							if (thisVolSlice[strikeIndx] + bumpPointAmount > 0.0){
								thisVolSlice[strikeIndx]  += bumpPointAmount;
							}
							else {
								std::cerr << "bumpVolPoint for underlying indx:" << i << " tenor:" << bumpPointTenor << " strike:" << bumpPointTenor << " amount:" << bumpPointAmount << " would create non-positive vols" << std::endl;
							}
							
						}
						if (bsPricer){
							// recalc localVol
							recalcLocalVol(
								ulVolsTenor,
								ulVolsStrike,
								ulVolsBsImpVol,
								ulFwdsAtVolTenor,
								ulVolsBumpedLocalVol
								);
						}						
					}
					// delta - bump each underlying
					if (doDeltas){
						sprintf(lineBuffer, "%s%d", "delete from deltas where ProductId=", productId);
						mydb.prepare((SQLCHAR *)lineBuffer, 1);
					}
					else if (doBumps){
						sprintf(lineBuffer, "%s%d%s%d", "delete from bump where ProductId=", productId," and userId=",bumpUserId);
						mydb.prepare((SQLCHAR *)lineBuffer, 1);
					}
					// hold drift curves
					vector<vector<double>> holdOisRatesRate;
					for (i=0; i < numUl; i++){
						holdOisRatesRate.push_back(oisRatesRate[i]);
					}

					/*
					* theta loop
					*/
					for (int thetaBump=0; thetaBump < thetaBumps; thetaBump++){
						thetaBumpAmount = thetaBumpStart + thetaBumpStep*thetaBump;
						// re-initialise barriers
						for (j=0; j < numBarriers; j++){
							SpBarrier& b(spr.barrier.at(j));
							// clear hits
							if (b.startDays>0){ b.hit.clear(); }
							// bump barrier  
							b.bumpSomeDays(-thetaBumpAmount);
							// set/reset brel days
							int numBrel = (int)b.brel.size();
							for (k=0; k < numBrel; k++){
								SpBarrierRelation& thisBrel(b.brel.at(k));
								thisBrel.bumpSomeDays(-thetaBumpAmount);
							}
						}
						if (slidingTheta){  //theta Slides Along VolSurface
							// bump observation points
							for (j=0; j < monDateIndx.size(); j++){
								monDateIndx[j] -= thetaBumpAmount;
								monDateT[j] -= thetaBumpAmount;
							}
						}
						else{
							// bump barrier observation pointsm but keep vol tenors the same
							monDateIndx.resize(0);
							monDateT.resize(0);
							for (j=0; j < barrierMonDateIndx.size(); j++){
								monDateIndx.push_back(barrierMonDateIndx[j] - thetaBumpAmount);
								monDateT.push_back(barrierMonDateT[j] - thetaBumpAmount);
							}
							for (j=0; j < volsMonDateIndx.size(); j++){
								monDateIndx.push_back(volsMonDateIndx[j]);
								monDateT.push_back(volsMonDateT[j]);
							}
							sort(monDateIndx.begin(), monDateIndx.end());
							sort(monDateT.begin(), monDateT.end());
						}
						
						spr.productDays  -= thetaBumpAmount;

						/*
						* credit loop
						*/
						for (int creditBump=0; creditBump < creditBumps; creditBump++){
							creditBumpAmount = creditBumpStart + creditBumpStep*creditBump;
							// change credit curve
							for (i=0; i < (int)cdsSpreads[0].size(); i++){
								cdsSpreads[0][i] = holdCdsSpread[i] + creditBumpAmount;
							}
							buildHazardCurve(cdsSpreads[0], cdsTenors[0], maxYears, recoveryRate, hazardCurves[0]);

							/*
							* rho loop
							*/
							for (int rhoBump=0; rhoBump < rhoBumps; rhoBump++){
								rhoBumpAmount = rhoBumpStart + rhoBumpStep*rhoBump;
								// change rate curves
								for (i=0; i < (int)baseCurve.size(); i++){
									spr.baseCurveSpread[i] = holdBaseCurve[i].spread + rhoBumpAmount;
								}
								for (i=0; i < numUl; i++){
									for (j=0; j < (int)holdOisRatesRate[i].size(); j++){
										oisRatesRate[i][j] = holdOisRatesRate[i][j] + rhoBumpAmount;
									}
								}
								/*
								* vega loop
								*/
								for (int vegaBump=0; vegaBump < vegaBumps; vegaBump++){
									vegaBumpAmount = vegaBumpStart + vegaBumpStep*vegaBump;
									// recalculate forward vols
									vector< vector<vector<double>> >  theseUlFwdVol(numUl), theseUlImpVol(numUl);
									for (i=0; i < numUl; i++){
										double thisFwdVol;
										for (j=0; j < (int)ulVolsTenor[i].size(); j++){
											vector<double>  someImpVol, someFwdVol;
											for (k=0; k < (int)ulVolsStrike[i][j].size(); k++){
												someImpVol.push_back(ulVolsImpVol[i][j][k] + vegaBumpAmount);
												if (j == 0){
													someFwdVol.push_back(someImpVol[k]);
												}
												else {
													double thisVol       = someImpVol[k];
													double previousVol   = theseUlImpVol[i][j - 1][k];
													double thisTenor     = ulVolsTenor[i][j];
													double previousTenor = ulVolsTenor[i][j - 1];
													double varianceDiff = (thisVol*thisVol*thisTenor - previousVol*previousVol*previousTenor);
													thisFwdVol    = varianceDiff < 0.0 ? thisVol : pow(varianceDiff / (thisTenor - previousTenor), 0.5);
													if (thisFwdVol < 0.0){ thisFwdVol = 0.0; }
													someFwdVol.push_back(thisFwdVol);
												}
											}
											theseUlFwdVol[i].push_back(someFwdVol);
											theseUlImpVol[i].push_back(someImpVol);
										}
									} // END recalculate forward vols

									/*
									* correlation loop
									*/
									// maybe bump correlations
									for (int corrBump=0; corrBump < corrBumps; corrBump++){
										corrBumpAmount = corrBumpStart + corrBumpStep*corrBump;
										// change correlations
										if (corrBumpAmount != 0.0){
											corrBumpVector[corrOtherIndex] = max(-1.0,min(1.0,corrBumpVector[corrOtherIndex] + corrBumpAmount));
										}

										/*
										* delta loop
										*/
										for (int deltaBump=0; deltaBump < deltaBumps; deltaBump++){
											deltaBumpAmount = deltaBumpStart + deltaBumpStep*deltaBump;
											double bumpFactor = 1.0 / (1.0 + deltaBumpAmount);
											if (true || deltaBumpAmount != 0.0 || vegaBumpAmount != 0.0 || rhoBumpAmount != 0.0){
												// for each underlying
												if (bumpEachUnderlying){
													for (int thisUidx=0; thisUidx < numUl; thisUidx++){
														int ulId = ulIds[thisUidx];

														if (true){
															if (!doTesting){
																bumpSpots(spr, thisUidx, ulIds, spots, ulPrices, doStickySmile, thisMarketData, holdUlVolsStrike, deltaBumpAmount, totalNumDays, daysExtant, true);
															}
															else{
																// bump spot
																double newSpot      = spots[thisUidx] * (1.0 + (doStickySmile ? 0.0 : deltaBumpAmount));
																double newMoneyness = newSpot / ulPrices[thisUidx].price[totalNumDays - 1 - daysExtant];
																if (doStickySmile){
																	for (j=0; j < (int)thisMarketData.ulVolsTenor[thisUidx].size(); j++){
																		for (k=0; k < (int)thisMarketData.ulVolsStrike[thisUidx][j].size(); k++){
																			thisMarketData.ulVolsStrike[thisUidx][j][k] = holdUlVolsStrike[thisUidx][j][k] * bumpFactor;
																		}
																	}
																}
																ulPrices[thisUidx].price[totalNumDays - 1] = newSpot;
																// re-initialise barriers
																for (j=0; j < numBarriers; j++){
																	SpBarrier& b(spr.barrier.at(j));
																	// clear hits
																	if (b.startDays>0){ b.hit.clear(); }
																	// set/reset brel moneyness
																	int numBrel = (int)b.brel.size();
																	for (k=0; k < numBrel; k++){
																		SpBarrierRelation& thisBrel(b.brel.at(k));
																		if (ulId == thisBrel.underlying){
																			thisBrel.calcMoneyness(newMoneyness);
																		}
																		else {
																			thisBrel.calcMoneyness(thisBrel.originalMoneyness);
																		}
																	}
																}
															}


															// install any bumpVolPoint
															if (bumpPointTenor > 0.0){
																for (j=0; j < numUl; j++){
																	thisMarketData.ulVolsImpVol[j] = holdUlImpVol[j];
																}
																thisMarketData.ulVolsImpVol[thisUidx] = ulVolsBumpedLocalVol[thisUidx];
															}
															else {
																// install any bumped vols
																thisMarketData.ulVolsFwdVol[thisUidx] = theseUlFwdVol[thisUidx];
																thisMarketData.ulVolsImpVol[thisUidx] = theseUlImpVol[thisUidx];
															}

														}

														cerr << "BUMP: UnderlyingId:" << ulId << " theta:" << thetaBumpAmount << " credit:" << creditBumpAmount << " rho:" << rhoBumpAmount << " vega:" << vegaBumpAmount << " corr:" << corrBumpAmount << " delta:" << deltaBumpAmount << endl;

														// re-evaluate
														evalResultBump = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
															numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurves, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, debugLevel, startTime, benchmarkId, benchmarkMoneyness,
															contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false, useProto, getMarketData, useUserParams, thisMarketData,
															cdsTenors, cdsSpreads, fundingFraction, productNeedsFullPriceRecord, ovveridePriipsStartDate, bumpedFairValue, doBumps /* conserveRands */, true /* consumeRands */,
															productHasMatured,/* priipsUsingRNdrifts */ false,/* updateCashflows */false,/* issuerIndx */0);
														if (doDeltas){
															if (deltaBumpAmount != 0.0){
																//  Elasticity: double  delta = (bumpedFairValue / thisFairValue - 1.0) / deltaBumpAmount;
																double  delta = (bumpedFairValue - thisFairValue) / deltaBumpAmount / issuePrice;
																sprintf(lineBuffer, "%s", "insert into deltas (Delta,DeltaType,LastDataDate,UnderlyingId,ProductId) values (");
																sprintf(lineBuffer, "%s%.5lf%s%d%s%s%s%d%s%d%s", lineBuffer, delta, ",", deltaBump == 0 ? 0 : 1, ",'", lastDataDateString.c_str(), "',", ulIds[thisUidx], ",", productId, ")");
																mydb.prepare((SQLCHAR *)lineBuffer, 1);
															}
														}
														else {
															sprintf(lineBuffer, "%s", "insert into bump (ProductId,UserId,UnderlyingId,DeltaBumpAmount,VegaBumpAmount,RhoBumpAmount,CreditBumpAmount,ThetaBumpAmount,CorrBumpAmount,CorrNames,FairValue,BumpedFairValue,LastDataDate) values (");
															sprintf(lineBuffer, "%s%d%s%d%s%d%s%.5lf%s%.5lf%s%.5lf%s%.5lf%s%d%s%.5lf%s%s%s%.5lf%s%.5lf%s%s%s", lineBuffer, productId, ",", bumpUserId, ",", ulIds[thisUidx], ",", deltaBumpAmount, ",", vegaBumpAmount, ",", rhoBumpAmount, ",", creditBumpAmount, ",", thetaBumpAmount, ",", corrBumpAmount, ",'", corrString.c_str(), "',", thisFairValue, ",", bumpedFairValue, ",'", lastDataDateString.c_str(), "')");
															mydb.prepare((SQLCHAR *)lineBuffer, 1);
														}
														// cerr << lineBuffer << endl;
														// ... reinstate spots
														ulPrices[thisUidx].price[totalNumDays - 1] = spots[thisUidx];
														// ... reinstate vols
														thisMarketData.ulVolsFwdVol[thisUidx] = holdUlFwdVol[thisUidx];
														thisMarketData.ulVolsImpVol[thisUidx] = holdUlImpVol[thisUidx];
														thisMarketData.ulVolsStrike[thisUidx] = holdUlVolsStrike[thisUidx];

													} // for (thisUidx=0; thisUidx < numUl; thisUidx++){
												} // if (bumpEachUnderlying){
												// for ALL underlyings
												if (!doRescaleSpots || bumpedFairValue > 0.0){
													for (i=0; i < numUl; i++){
														int ulId = ulIds[i];
														bumpSpots(spr, i, ulIds, spots, ulPrices, doStickySmile, thisMarketData, holdUlVolsStrike, deltaBumpAmount, totalNumDays, daysExtant, false);
														if (bumpPointTenor > 0.0){
															thisMarketData.ulVolsImpVol[i] = ulVolsBumpedLocalVol[i];
														}
														else{
															// install any bumped vols
															thisMarketData.ulVolsFwdVol[i] = theseUlFwdVol[i];
															thisMarketData.ulVolsImpVol[i] = theseUlImpVol[i];
														}														
													}
												}
												// re-evaluate
												cerr << "BUMPALL: credit:" << creditBumpAmount << " rho:" << rhoBumpAmount << " vega:" << vegaBumpAmount << " corr:" << corrBumpAmount << " delta:" << deltaBumpAmount << endl;
												evalResultBump = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
													numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurves, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, debugLevel, startTime, benchmarkId, benchmarkMoneyness,
													contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false, useProto, getMarketData, useUserParams, thisMarketData,
													cdsTenors, cdsSpreads, fundingFraction, productNeedsFullPriceRecord, ovveridePriipsStartDate, bumpedFairValue, doBumps /* conserveRands */, true,
													productHasMatured,/* priipsUsingRNdrifts */ false,/* updateCashflows */false,/* issuerIndx */0);
												if (doDeltas) {
													if (deltaBumpAmount != 0.0){
														//  Elasticity: double  delta = (bumpedFairValue / thisFairValue - 1.0) / deltaBumpAmount;
														double  delta = (bumpedFairValue - thisFairValue) / deltaBumpAmount / issuePrice;
														sprintf(lineBuffer, "%s", "insert into deltas (Delta,DeltaType,LastDataDate,UnderlyingId,ProductId) values (");
														sprintf(lineBuffer, "%s%.5lf%s%d%s%s%s%d%s%d%s", lineBuffer, delta, ",", deltaBump == 0 ? 0 : 1, ",'", lastDataDateString.c_str(), "',", 0, ",", productId, ")");
														mydb.prepare((SQLCHAR *)lineBuffer, 1);
														if (bumpUserId == 3){
															sprintf(lineBuffer, "%s%s%s%.5lf%s%s%s%d%s", "update product set ", deltaBump == 0 ? "DeltaDown" : "DeltaUp", "=", delta, ",DeltaDate='", lastDataDateString.c_str(), "' where productid=", productId, "");
															mydb.prepare((SQLCHAR *)lineBuffer, 1);
														}
													}
												}
												else {
													sprintf(lineBuffer, "%s", "insert into bump (ProductId,UserId,UnderlyingId,DeltaBumpAmount,VegaBumpAmount,RhoBumpAmount,CreditBumpAmount,ThetaBumpAmount,CorrBumpAmount,CorrNames,FairValue,BumpedFairValue,LastDataDate) values (");
													sprintf(lineBuffer, "%s%d%s%d%s%d%s%.5lf%s%.5lf%s%.5lf%s%.5lf%s%d%s%.5lf%s%s%s%.5lf%s%.5lf%s%s%s", lineBuffer, productId, ",", bumpUserId, ",", 0, ",", deltaBumpAmount, ",", vegaBumpAmount, ",", rhoBumpAmount, ",", creditBumpAmount, ",", thetaBumpAmount, ",", corrBumpAmount, ",'", corrString.c_str(), "',", thisFairValue, ",", bumpedFairValue, ",'", lastDataDateString.c_str(), "')");
													mydb.prepare((SQLCHAR *)lineBuffer, 1);
													// save some greeks to product table
													if (bumpUserId == 3 && deltaBumpAmount == 0.0){
														// save vegas to product table
														if (vegaBumpAmount != 0.0 && thetaBumpAmount == 0.0 && creditBumpAmount == 0.0 && rhoBumpAmount == 0.0){
															double  vega = (bumpedFairValue - thisFairValue) / (100.0*vegaBumpAmount);
															sprintf(lineBuffer, "%s%s%s%.5lf%s%s%s%d%s", "update product set ", vegaBumpAmount < 0.0 ? "Vega" : "VegaUp", "=", vega, ",VegaDate='", lastDataDateString.c_str(), "' where productid=", productId, "");
															mydb.prepare((SQLCHAR *)lineBuffer, 1);
														}
														// save rho to product table
														if (vegaBumpAmount == 0.0 && thetaBumpAmount == 0.0 && creditBumpAmount == 0.0 && rhoBumpAmount != 0.0){
															double  rho = (bumpedFairValue - thisFairValue) / (100.0*rhoBumpAmount);
															sprintf(lineBuffer, "%s%s%s%.5lf%s%s%s%d%s", "update product set ", rhoBump == 0 ? "Rho" : "RhoUp", "=", rho, ",RhoDate='", lastDataDateString.c_str(), "' where productid=", productId, "");
															mydb.prepare((SQLCHAR *)lineBuffer, 1);
														}

													}
												}

												// ... reinstate spots
												for (i=0; i < numUl; i++){
													ulPrices[i].price[totalNumDays - 1] = spots[i];
												} // for (i=0; i < numUl; i++){	
											} // if (deltaBumpAmount != 0.0 || vegaBumpAmount != 0.0 || rhoBumpAmount != 0.0){
										} // for (int deltaBump=0; deltaBump < deltaBumps; deltaBump++){
										// reinstate correlations
										if (corrBumpAmount != 0.0){
											corrBumpVector[corrOtherIndex] = holdCorr;
										}
									}  // for (int corrBump=0; corrBump < corrBumps; corrBump++){
									// ... reinstate vols
									for (i=0; i < numUl; i++){
										thisMarketData.ulVolsFwdVol[i] = holdUlFwdVol[i];
										thisMarketData.ulVolsImpVol[i] = holdUlImpVol[i];
										thisMarketData.ulVolsStrike[i] = holdUlVolsStrike[i];
									}
								} // for (int vegaBump=0; vegaBump < vegaBumps; vegaBump++){
								// reinstate curves
								for (i=0; i < (int)baseCurve.size(); i++){
									spr.baseCurveSpread[i] = holdBaseCurve[i].spread;
								}
								for (i=0; i < numUl; i++){
									for (j=0; j < (int)holdOisRatesRate[i].size(); j++){
										oisRatesRate[i][j] = holdOisRatesRate[i][j];
									}
								}
							} // for (int rhoBump=0; rhoBump < rhoBumps; rhoBump++){
							// reinstate credit
							// change credit curve
							for (i=0; i < (int)cdsSpreads[0].size(); i++){
								cdsSpreads[0][i] = holdCdsSpread[i];
							}
							buildHazardCurve(cdsSpreads[0], cdsTenors[0], maxYears, recoveryRate, hazardCurves[0]);
						} // for (int creditBump=0; creditBump < creditBumps; creditBump++){
						// re-initialise barriers
						for (j=0; j < numBarriers; j++){
							SpBarrier& b(spr.barrier.at(j));
							// bump barrier
							b.bumpSomeDays(thetaBumpAmount);
							// set/reset brel days
							int numBrel = (int)b.brel.size();
							for (k=0; k < numBrel; k++){
								SpBarrierRelation& thisBrel(b.brel.at(k));
								thisBrel.bumpSomeDays(thetaBumpAmount);
							}
						}
						if (slidingTheta){
							// unbump ALL observation points
							for (j=0; j < numBarriers; j++){
								monDateIndx[j] += thetaBumpAmount;
								monDateT[j] += thetaBumpAmount;
							}
						}
						else{
							// reinstate observation points
							for (j=0; j < monDateIndx.size(); j++){
								monDateIndx[j] = holdMonDateIndx[j];
								monDateT[j] = holdMonDateT[j];
							}
						}
						spr.productDays  += thetaBumpAmount;
					} // for (int thetaBump=0; thetaBump < thetaBumps; thetaBump++){
										
				}
			}

			// PRIIPs
			if (doPriips){
				// real-world drifts
				EvalResult evalResultPriips(0.0, 0.0, 0);
				evalResultPriips = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
					numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurves, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, debugLevel, startTime, benchmarkId, benchmarkMoneyness,
					contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
					useProto, getMarketData, useUserParams, thisMarketData,cdsTenors, cdsSpreads, fundingFraction, productNeedsFullPriceRecord, 
					ovveridePriipsStartDate, thisFairValue, false, false, productHasMatured,/* priipsUsingRNdrifts */ false,/* updateCashflows */true,/* issuerIndx */0);

				// adjust driftrate to riskfree (? minus divs ?)
				// DOME: check 
				// ... at least 2y of daily data
				// ... monthly data is penalised by RiskScore +1
				for (i = 0; i < numUl; i++) {
					double thisVariance               = calendarDailyVariance[i];
					double thisDailyVol               = pow(thisVariance, .5);
					// calculate ACTUAL drift rates	
					// ... DO NOT subtract divYield: evident from CornishFisherVaR that the PRIIPSreturnsDistribution has mean=zero (minus the convexity term)
					//     ... so PRIIPs is only trying to measure dispersion and does not care about the PRIIPs' drift, so may as well force it to drift and discount at rfr
					//     ... so as to achieve a mean=zero distribution of pvs
					double thisDivYield               = 0.0; // interpVector(divYieldsRate[i], divYieldsTenor[i], maxYears); // NOTE PRIIPs divYieldsRate are NEGATIVE
					double dailyDriftContRate         = log(ulOriginalPrices.at(i).price.at(totalNumDays - 1) / ulOriginalPrices.at(i).price.at(0)) / (totalNumDays);
					double dailyQuantoAdj             = quantoCrossRateVols[i] * thisDailyVol * quantoCorrelations[i];
					double priipsDailyDriftCorrection = exp(log(1 + spr.priipsRfr + thisDivYield) / 365.0 - dailyDriftContRate - dailyQuantoAdj);
					double annualisedCorrection       = pow(priipsDailyDriftCorrection, 365.25);


					// change underlyings' drift rate
					for (j = 0; j < (int)ulReturns[i].size(); j++) {
						ulReturns[i][j] *= priipsDailyDriftCorrection;
					}
				}
				// re-evaluate, this time setting priipsUsingRNdrifts=true
				evalResultPriips = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
					numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurves, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, debugLevel, startTime, benchmarkId, benchmarkMoneyness,
					contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
					useProto, getMarketData, useUserParams, thisMarketData, cdsTenors, cdsSpreads, fundingFraction, productNeedsFullPriceRecord,
					ovveridePriipsStartDate, thisFairValue, false, false, productHasMatured, /* priipsUsingRNdrifts */ true,/* updateCashflows */true,/* issuerIndx */0);


				// PRIIPS stresstest
				// ... build rolling 21-day windows of historical log returns
				// ... priipsStressVol is the 90th percentile of this distribution
				const int rollingWindowSize(maxBarrierDays > 365 ? 63:21);
				const double roughVolAnnualiser(16.0);
				double sliceMean, sliceStdev, sliceStderr;
				for (i = 0; i < numUl; i++) {
					vector<double> stressVols;
					double thisReturn;
					const double thisNumReturns((double)originalUlReturns[0].size());
					vector<double>  bigSlice;
					deque<double> thisSlice;
					// calc vol from 21-day window of daily continuous returns
					if ((int)originalUlReturns[0].size() < rollingWindowSize){
						cerr << "Not enough data for PRIIPS stress test" << endl;
						exit(111);
					}
					// load the window
					for (j = 0; (int)thisSlice.size() < rollingWindowSize; j++) {
						if (!ulOriginalPrices[i].nonTradingDay[j]){
							thisReturn = log(originalUlReturns[i][j]);
							bigSlice.push_back(thisReturn);
							thisSlice.push_back(thisReturn);
						}
					}
					double highestVol(-1.0);
					int    obsAtHighestVol(0);
					// roll the window
					for (; j < thisNumReturns; j++) {
						if (!ulOriginalPrices[i].nonTradingDay[j]){
							thisReturn = log(originalUlReturns[i][j]);
							bigSlice.push_back(thisReturn);
							MeanAndStdev(thisSlice, sliceMean, sliceStdev, sliceStderr);
							if (sliceStdev > highestVol){
								highestVol = sliceStdev;
								obsAtHighestVol = j-1;
							}
							stressVols.push_back(sliceStdev*roughVolAnnualiser);
							thisSlice.pop_front();
							thisSlice.push_back(thisReturn);
						}
					}
					sort(stressVols.begin(), stressVols.end());
					double thisStressedVol     = stressVols[(unsigned int)floor((double)stressVols.size()*(maxBarrierDays > 365 ? 0.90 : 0.99))];
					MeanAndStdev(bigSlice, sliceMean, sliceStdev, sliceStderr);
					double originalVol         = sliceStdev * roughVolAnnualiser;
					double thisInflationFactor = thisStressedVol / originalVol;
					// easy to get a very high inflation factor with a timeseries like our GBPdeposit index which rarely changes by much
					if (thisInflationFactor > 10.0){ 
						thisInflationFactor = 10.0;
						// debug only
						thisSlice.clear();
						deque<string> highestVolDates;
						deque<double> highestVolLevels;
						for (j = obsAtHighestVol; (int)thisSlice.size() < rollingWindowSize; j--) {
							if (!ulOriginalPrices[i].nonTradingDay[j]){
								thisReturn = log(originalUlReturns[i][j]);
								thisSlice.push_front(thisReturn);
								highestVolDates.push_front(ulOriginalPrices[i].date[j]);
								highestVolLevels.push_front(ulOriginalPrices[i].price[j]);
							}
						}
						MeanAndStdev(thisSlice, sliceMean, sliceStdev, sliceStderr);
						// END debug
					}
					spr.priipsStressVols.push_back(thisInflationFactor);
					// inflate underlyings' returns
					for (j = 0; j < (int)ulReturns[i].size(); j++) {
						ulReturns[i][j] = exp(log(ulReturns[i][j])*thisInflationFactor);
					}
				}
				// reinitialise for the stresstest
				evalResultPriips = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
					numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurves, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, debugLevel, startTime, benchmarkId, benchmarkMoneyness,
					contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, true /* doPriipsStress */,
					useProto, getMarketData, useUserParams, thisMarketData, cdsTenors, cdsSpreads, fundingFraction, productNeedsFullPriceRecord,
					ovveridePriipsStartDate, thisFairValue, false, false, productHasMatured, /* priipsUsingRNdrifts */ true,/* updateCashflows */true,/* issuerIndx */0);

			}

		} // for each product
		std::cout << "timeTaken:" << difftime(time(0), startTime) << "secs" << endl;
		std::cout << "clytemnestra";
	} // try

	// tidy up
	catch (out_of_range oor){
		cerr << "Out of Range error \n" << oor.what();
	}
	catch (bad_alloc){
		cerr << "Out of Memory error \n";
	}
	catch (length_error){
		cerr << "Length error \n";
	}
	catch (...){
		cerr << "unknown error \n";
	}

    // any performance timing info?
	for (std::map<string, TimerData>::iterator iter = scopedTimers.begin(); iter != scopedTimers.end(); ++iter) {
		std::cout << "\nfunc: " << std::setfill(' ') << std::setw(25) << iter->first.c_str() << " :ms: " << iter->second.sumTime << " :#calls: " << iter->second.numCalls;
	}
	
	return 0;
}

