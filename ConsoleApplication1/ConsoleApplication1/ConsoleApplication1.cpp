
// ConsoleApplication1.cpp : Defines the entry point for the console application.
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
		argWords["doFAR"          ]         = "";      
		argWords["doDeltas"       ]         = "";
		argWords["notIllustrative"]         = "";
		argWords["hasISIN"]                 = "";
		argWords["hasInventory"]            = "";
		argWords["notStale"]                = "";
		argWords["incomeProducts"]          = "";
		argWords["capitalProducts"]         = "";
		argWords["ignoreBenchmark"]         = "";
		argWords["debug"]                   = "";
		argWords["silent"]                  = "";
		argWords["priips"]                  = "";
		argWords["doAnyIdTable"]            = "";
		argWords["getMarketData"]           = "";
		argWords["proto"]                   = "";
		argWords["bsPricer"]                = "";
		argWords["stochasticDrift"]         = "";
		argWords["dbServer"]                = "spCloud|newSp|spIPRL";
		argWords["forceIterations"]         = "";
		argWords["useProductFundingFractionFactor"] = "";
		argWords["showMatured"]             = "";
		argWords["historyStep"]             = "nnn";
		argWords["startDate"]               = "YYYY-mm-dd";
		argWords["endDate"]                 = "YYYY-mm-dd";
		argWords["bumpUserId"]              = "nnn";
		argWords["minSecsTaken"]            = "nnn";
		argWords["maxSecsTaken"]            = "nnn";
		argWords["userParameters"]          = "userId";
		argWords["only"]                    = "<comma-sep list of underlyings names>";
		argWords["notOnly"]                 = "<comma-sep list of underlyings names>";
		argWords["UKSPA"]                   = "Bear|Neutral|Bull";
		argWords["Issuer"]                  = "partName";
		argWords["fundingFractionFactor"]   = "x.x";
		argWords["forceFundingFraction"]    = "x.x";
		argWords["useThisPrice"]            = "x.x";
		argWords["useThisOIS"]              = "x.x";
		argWords["planSelect"]              = "only|none";		
		argWords["eqFx"]                    = "eqUid:fxId:x.x   eg 3:1:-0.5";
		argWords["eqEq"]                    = "eqUid:eqUid:x.x  eg 3:1:-0.5";
		argWords["ulLevel"]                 = "name:level       eg UK100:6500";
		argWords["solveFor"]                = "targetFairValue:whatToSolveFor[:commit]  eg 98.0:coupon|putBarrier and add :commit to save solution";
		argWords["stickySmile"]             = "";
		argWords["bump"]                    = "bumpType:startBump:stepSize:numBumps eg delta:-0.05:0.05:3 >";
		argWords["forOptimisation"]         = "";
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
		
		
		int              historyStep = 1, minSecsTaken=0, maxSecsTaken=0;
		int              commaSepList   = strstr(WcharToChar(argv[1], &numChars),",") ? 1:0;
		int              userParametersId(0),startProductId, stopProductId, fxCorrelationUid(0), fxCorrelationOtherId(0), eqCorrelationUid(0), eqCorrelationOtherId(0), optimiseNumDays(0);
		int              bumpUserId(3),requesterNumIterations = argc > 3 - commaSepList ? _ttoi(argv[3 - commaSepList]) : 100;
		bool             doFinalAssetReturn(false), requesterForceIterations(false), doDebug(false), getMarketData(false), notStale(false), hasISIN(false), hasInventory(false), notIllustrative(false), onlyTheseUls(false), forceEqFxCorr(false), forceEqEqCorr(false);
		bool             doUseThisOIS(false),doUseThisPrice(false),showMatured(false), doBumps(false), doDeltas(false), doPriips(false), ovveridePriipsStartDate(false), doUKSPA(false), doAnyIdTable(false);
		bool             doStickySmile(false), useProductFundingFractionFactor(false), forOptimisation(false), silent(false), doIncomeProducts(false), doCapitalProducts(false), solveFor(false), solveForCommit(false);
		bool             localVol(true), stochasticDrift(false), ignoreBenchmark(false), done, forceFullPriceRecord(false), fullyProtected, firstTime, forceUlLevels(false);
		char             lineBuffer[MAX_SP_BUF], charBuffer[10000];
		char             onlyTheseUlsBuffer[1000] = "";
		char             startDate[11]            = "";
		char             endDate[11]              = "";
		char             useProto[6]              = "";
		char             priipsStartDatePhrase[100];
		double           fundingFractionFactor    = MIN_FUNDING_FRACTION_FACTOR, forceEqFxCorrelation(0.0), forceEqEqCorrelation(0.0);
		double           useThisOIS,targetFairValue,useThisPrice,thisFairValue, bumpedFairValue;
		double           deltaBumpAmount(0.05),deltaBumpStart(0.0), deltaBumpStep(0.0), vegaBumpStart(0.0), vegaBumpStep(0.0), thetaBumpStart(0.0), thetaBumpStep(0.0);
		double           rhoBumpStart(0.0), rhoBumpStep(0.0), creditBumpStart(0.0), creditBumpStep(0.0);
		int              optimiseNumUls(0), deltaBumps(1), vegaBumps(1), thetaBumps(1), rhoBumps(1), creditBumps(1),solveForThis(0);
		boost::gregorian::date lastDate;
		string           anyString, ukspaCase(""), issuerPartName(""), forceFundingFraction(""), planSelect(""),whatToSolveFor(""),lastOptimiseDate;
		map<char, int>   avgTenor; avgTenor['d'] = 1; avgTenor['w'] = 7; avgTenor['m'] = 30; avgTenor['q'] = 91; avgTenor['s'] = 182; avgTenor['y'] = 365;
		map<string, int> bumpIds; bumpIds["delta"] = 1; bumpIds["vega"] = 2; bumpIds["theta"] = 3; bumpIds["rho"] = 4; bumpIds["credit"] = 5;
		map<string, double> ulLevels;  // name:level
		char dbServer[100]; strcpy(dbServer, "newSp");  // on local PC: newSp for local, spIPRL for IXshared        on IXcloud: spCloud
		vector<string>   rangeFilterStrings;
		const int        maxUls(100);
		const int        bufSize(1000);

		RETCODE          retcode;
		SomeCurve        anyCurve;
		time_t           startTime = time(0);
		char             **szAllPrices = new char*[maxUls];
		vector<int>      optimiseProductIds,allProductIds; allProductIds.reserve(1000);
		vector<int>::iterator intIterator, intIterator1;
		for (int i = 0; i < maxUls; i++){
			szAllPrices[i] = new char[bufSize];
		}
		srand((unsigned int)time(0)); // reseed rand


		// open database
		done = false;
		for(int i=4 - commaSepList; i < argc && !done; i++){
			char *thisArg  = WcharToChar(argv[i], &numChars);
			if (sscanf(thisArg, "dbServer:%s", lineBuffer)){ 
				strcpy(dbServer, lineBuffer); 
				done = true;
			}
		}
		MyDB  mydb((char **)szAllPrices, dbServer), mydb1((char **)szAllPrices, dbServer);

		// some inits from db
		vector<string>   payoffType;  payoffType.push_back("");
		sprintf(lineBuffer, "%s", "select name from payofftype order by PayoffTypeId");
		mydb.prepare((SQLCHAR *)lineBuffer, 1);
		retcode = mydb.fetch(false, lineBuffer);
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
				cerr << "Unknown argument: " << thisArg << endl;
				exit(101);
			}
		}
		

		// process optional argumants
		for (int i=4 - commaSepList; i<argc; i++){
			char *thisArg  = WcharToChar(argv[i], &numChars);
			if (strstr(thisArg, "forceIterations")){ requesterForceIterations = true; }
			if (strstr(thisArg, "priips"            )){ doPriips           = true; }
			if (strstr(thisArg, "useProductFundingFractionFactor")){  useProductFundingFractionFactor  = true; }
			if (strstr(thisArg, "getMarketData"     )){ getMarketData      = true; }
			if (strstr(thisArg, "bsPricer"          )){ localVol           = false; }
			// if (strstr(thisArg, "proto"             )){ strcpy(useProto,"proto"); }
			if (strstr(thisArg, "stochasticDrift"   )){ stochasticDrift    = true; }				
			if (strstr(thisArg, "doFAR"             )){ doFinalAssetReturn = true; }
			if (strstr(thisArg, "doAnyIdTable"      )){ doAnyIdTable       = true; }
			if (strstr(thisArg, "debug"             )){ doDebug            = true; }
			if (strstr(thisArg, "silent"            )){ silent             = true; }
			if (strstr(thisArg, "notIllustrative"   )){ notIllustrative    = true; }
			if (strstr(thisArg, "hasISIN"           )){ hasISIN            = true; }
			if (strstr(thisArg, "hasInventory"      )){ hasInventory       = true; }
			if (strstr(thisArg, "showMatured"       )){ showMatured        = true; }			
			if (strstr(thisArg, "notStale"          )){ notStale           = true; }
			if (strstr(thisArg, "incomeProducts"    )){ doIncomeProducts   = true; }			
			if (strstr(thisArg, "capitalProducts"   )){ doCapitalProducts  = true; }
			if (strstr(thisArg, "ignoreBenchmark"   )){ ignoreBenchmark    = true; }
			if (strstr(thisArg, "stickySmile"       )){ doStickySmile      = true; }
			if (strstr(thisArg, "forOptimisation"   )){ forOptimisation    = true; }


			// parse range strings, of the form <name>:<number or number-number>
			// commandLineName: sql to select corresponding quantity from cashflows table
			map<string, string> rangeVerbs;  // key:sql
			rangeVerbs["duration"   ] = "duration";
			rangeVerbs["volatility" ] = "100*EsVol*sqrt(duration)";
			rangeVerbs["arithReturn"] = "100*EarithReturn";
			rangeVerbs["CAGR"       ] = "100*ExpectedReturn";
			rangeVerbs["CAGRsharpe" ] = "ExpectedReturn/(EsVol*sqrt(duration))";
			rangeVerbs["tailReturn" ] = "eShortfall";
			// give FairValue precedence over BidAsk if we will run the IPR fairValue simulator
			sprintf(charBuffer, "%s%s", "ExpectedReturn/(1.0 - IssuePrice*EShortfallTest/100/", getMarketData ? "if(FairValueDate != LastDataDate,if((BidAskDate != LastDataDate) or StalePrice,IssuePrice,Ask),FairValue))" : "if((BidAskDate != LastDataDate) or StalePrice,if(FairValueDate != LastDataDate,IssuePrice,FairValue),Ask))");
			rangeVerbs["CAGRtoCVAR95loss"] = charBuffer;
			rangeVerbs["couponReturn"] = "100*couponReturn";

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
				getMarketData   = true;
				doDeltas        = true; 
				doBumps         = true;
				deltaBumpStart  = -deltaBumpAmount;
				deltaBumpStep   =  deltaBumpAmount;
				deltaBumps      = 3;
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
				switch (bumpIds[tokens[0].c_str()]){
					case 1: // delta
						deltaBumpStart  = start;
						deltaBumpStep   = step;
						deltaBumps      = num;
						break;
					case 2: // vega
						vegaBumpStart  = start;
						vegaBumpStep   = step;
						vegaBumps      = num;
						break;
					case 3: // theta
						thetaBumpStart  = start;
						thetaBumpStep   = step;
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
				}
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
				else if (whatToSolveFor == "putBarrier"){
					solveForThis = solveForPutBarrier;
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
				bool notOnly = sscanf(thisArg, "notOnly:%s", lineBuffer);
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
			if (sscanf(thisArg, "startDate:%s",  lineBuffer))             { strcpy(startDate, lineBuffer); }
			if (sscanf(thisArg, "UKSPA:%s", lineBuffer))                  {
				ukspaCase     = lineBuffer;
				doUKSPA       = ukspaCase != "";
				getMarketData = true;
			}
			if (sscanf(thisArg, "Issuer:%s", lineBuffer))                 { issuerPartName          = lineBuffer; }
			if (sscanf(thisArg, "fundingFractionFactor:%s", lineBuffer))  { fundingFractionFactor	= atof(lineBuffer);	}
			if (sscanf(thisArg, "forceFundingFraction:%s", lineBuffer))   { forceFundingFraction	= lineBuffer; }
		
			else if (sscanf(thisArg, "endDate:%s",        lineBuffer)){ strcpy(endDate, lineBuffer); }
			else if (sscanf(thisArg, "bumpUserId:%s",     lineBuffer)){ bumpUserId       = atoi(lineBuffer); }
			else if (sscanf(thisArg, "minnSecsTaken:%s",  lineBuffer)){ minSecsTaken     = atoi(lineBuffer); }
			else if (sscanf(thisArg, "maxSecsTaken:%s",   lineBuffer)){ maxSecsTaken     = atoi(lineBuffer); }
			else if (sscanf(thisArg, "userParameters:%s", lineBuffer)){ userParametersId = atoi(lineBuffer); }
			else if (sscanf(thisArg, "historyStep:%s",    lineBuffer)){ historyStep      = atoi(lineBuffer); }
			else if (sscanf(thisArg, "useThisPrice:%s",   lineBuffer)){ useThisPrice     = atof(lineBuffer); doUseThisPrice = true; }
			else if (sscanf(thisArg, "useThisOIS:%s",     lineBuffer)){ useThisOIS       = atof(lineBuffer); doUseThisOIS   = true; }
		}
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
		if (forOptimisation){
			if (!getMarketData && userParametersId <1){
				cout << "Optimiser needs scenario-generation from either getMarketData or userParameters" << endl;
				exit(106);
			}
			allProductIds.push_back(1);  // special product id=1 with UK100,SX5E,SPX			
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
			showMatured        ? "" : " and Matured=0 ",
			(notIllustrative   ? " and Illustrative=0 " : ""),
			(hasISIN           ? " and ISIN != '' " : ""),
			(hasInventory      ? " and p.Inventory > 0 " : ""),
			(notStale          ? " and StalePrice=0 " : ""),
			(doIncomeProducts  ? " and pt.name like '%income%' " : ""),
			(doCapitalProducts ? " and pt.name not like '%income%' " : "")
			);
		for (int i=0; i < (int)rangeFilterStrings.size(); i++){
			sprintf(lineBuffer, "%s%s", lineBuffer, rangeFilterStrings[i].c_str());
		}
		sprintf(lineBuffer, "%s%s", lineBuffer, " and ProjectedReturn=1 ");
		
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
		mydb.prepare((SQLCHAR *)lineBuffer, 1); 	retcode = mydb.fetch(true,lineBuffer);
		while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			int x(atoi(szAllPrices[0])); allProductIds.push_back(x);
			retcode = mydb.fetch(false,"");
		}
		int numProducts = (int)allProductIds.size();
		// cerr << "Doing:" << allProductIds.size() << " products " << lineBuffer << endl;

		/*
		* getBenchmarkPerf
		*/
		double bmSwapRate(0.0);
		sprintf(lineBuffer, "%s", "SELECT avg(Rate)/100.0 sixYSwaps from curve where ccy='GBP' and Tenor in (5,7)");
		mydb.prepare((SQLCHAR *)lineBuffer, 1); 	retcode = mydb.fetch(true, lineBuffer);
		if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			bmSwapRate = atof(szAllPrices[0]);			
		}
		double projectedReturn = (requesterNumIterations <= 1 ? 0.0 : (doPriips ? 0.08 : 1.0));

		double bmEarithReturn(0.0), bmVol(0.18);
		sprintf(lineBuffer, "%s%.4lf%s","SELECT EarithReturn ArithmeticReturn_pa, esVol*sqrt(duration) Volatility from cashflows where ProductId=71 and ProjectedReturn='",projectedReturn,"'");
		mydb.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb.fetch(true, lineBuffer);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			bmEarithReturn = atof(szAllPrices[0]);
			bmVol          = atof(szAllPrices[1]);
		}
	
		

		// deal with any optimisation demands
		if (forOptimisation){
			// find #underlyings
			sprintf(lineBuffer, "%s%d%s", "select UnderlyingId from (select distinct underlyingid from barrierrelation join productbarrier using (productbarrierid) where productid in (",
				allProductIds[0], "))x");			
			mydb.prepare((SQLCHAR *)lineBuffer, 1); 	
			retcode = mydb.fetch(true, lineBuffer);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				optimiseProductIds.push_back(atoi(szAllPrices[0]));
				retcode = mydb.fetch(false, "");
			}
			optimiseNumUls     = (int)optimiseProductIds.size();
			// find max date for these products
			strcpy(charBuffer, "");
			for (int i=1; i < numProducts; i++){
				sprintf(charBuffer, "%s%s%d", charBuffer, i > 1 ? "," : "", allProductIds[i]);
			}
			sprintf(lineBuffer, "%s%s%s", "select max(greatest(settlementDate,EndDate)) from barrierrelation join productbarrier using (productbarrierid) where productid in (",
				charBuffer, ") and SettlementDate < date_add(now(),INTERVAL 12 YEAR)");  // limit to 12y to avoid blowing memory, and some 'Markets' products are deliberately set to start way-in-the-future
			mydb.prepare((SQLCHAR *)lineBuffer, 1);
			retcode = mydb.fetch(true, lineBuffer);
			lastOptimiseDate = szAllPrices[0]; 
			boost::gregorian::date bLastOptimiseDate(boost::gregorian::from_simple_string(lastOptimiseDate));

			// find last data date
			sprintf(lineBuffer, "%s%s%s", "select group_concat(underlyingid) from (select distinct underlyingid from barrierrelation join productbarrier using (productbarrierid) where productid in (",
				charBuffer, "))x");
			mydb.prepare((SQLCHAR *)lineBuffer, 1);
			retcode = mydb.fetch(true, lineBuffer);
			string concatUlIds = szAllPrices[0];
			string thisLastDate; 
			if (strlen(endDate)){
				thisLastDate = endDate;
			}
			else{
				sprintf(lineBuffer, "%s%s%s", "select max(Date) from prices where underlyingid in (", concatUlIds.c_str(), ")");
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				retcode = mydb.fetch(true, lineBuffer);
				thisLastDate = szAllPrices[0];
			}
			
			
			// calc #days of underlyings simulated levels we will need to store
			boost::gregorian::date bLastDataDate(boost::gregorian::from_simple_string(thisLastDate));
			boost::gregorian::date_duration dateDiff(bLastOptimiseDate - bLastDataDate);
			optimiseNumDays = dateDiff.days();

			// force generation of daily paths
			forceFullPriceRecord = true;

			// reset SettlementDate,StartDate,EndDate on SpecialProduct (id=1)
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
			sprintf(lineBuffer, "%s", "delete from productreturns");
			mydb.prepare((SQLCHAR *)lineBuffer, 1);
			sprintf(lineBuffer, "%s", "delete from simulatedunderlyings");
			mydb.prepare((SQLCHAR *)lineBuffer, 1);
		}


		// loop through each product
		std::vector<int> optimiseUlIdNameMap(1000);  // underlyingId -> arrayIndex, so ulIdNameMap[uid] gives the index into ulPrices vector
		std::vector<std::vector<std::vector<double>>> optimiseMcLevels(optimiseNumUls, std::vector<std::vector<double>>(optimiseNumDays));
		if (numProducts>1){ doUseThisPrice = false; }
		for (int productIndx = 0; productIndx < numProducts; productIndx++) {
			int              thisNumIterations = requesterNumIterations, numBarriers = 0, thisIteration = 0;
			int              i, j, k, len, len1, anyInt, numUl, numMonPoints,totalNumDays, totalNumReturns, uid;
			int              productId, anyTypeId, thisPayoffId, productShapeId, protectionLevelId,barrierRelationId;
			double           anyDouble, cds5y, maxBarrierDays, barrier, uBarrier, payoff, strike, cap, participation, fixedCoupon, AMC, issuePrice, bidPrice, askPrice, midPrice, baseCcyReturn, benchmarkStrike;
			string           productShape, protectionLevel, couponFrequency, productStartDateString, productCcy, productBaseCcy, word, word1, thisPayoffType, startDateString, endDateString, nature, settlementDate,
				description, avgInAlgebra, productTimepoints, productPercentiles,fairValueDateString,bidAskDateString,lastDataDateString;
			bool             useUserParams(false), productNeedsFullPriceRecord(false), capitalOrIncome, above, at;
			vector<int>      monDateIndx, reportableMonDateIndx, accrualMonDateIndx;
			vector<double>   monDateT, accrualMonDateT;
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
				colProductFundingFractionFactor, colProductBenchmarkStrike, colProductBootstrapStride, colProductSettleDays, colProductBarrierBend, colProductLast
			};
			sprintf(lineBuffer, "%s%s%s%d%s", "select * from ", useProto, "product where ProductId='", productId, "'");
			mydb.prepare((SQLCHAR *)lineBuffer, colProductLast);
			retcode = mydb.fetch(true,lineBuffer);
			int  productMaxIterations  = atoi(szAllPrices[colProductMaxIterations]);
			bool forceIterations = requesterForceIterations;
			if (productMaxIterations < thisNumIterations){ 
				thisNumIterations = productMaxIterations; 
				forceIterations = true;
			}
			if (thisNumIterations<1) { thisNumIterations = 1; }
			int  counterpartyId     = atoi(szAllPrices[colProductCounterpartyId]);
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
			

			useUserParams                 = userParametersId > 0 ? true : atoi(szAllPrices[colProductUseUserParams]) == 1;
			string forceStartDate         = szAllPrices[colProductForceStartDate];
			if ( useProductFundingFractionFactor){
				fundingFraction = defaultFundingFraction*atof(szAllPrices[colProductFundingFractionFactor]);;
			}
			if (fundingFractionFactor > MIN_FUNDING_FRACTION_FACTOR){
				fundingFraction = defaultFundingFraction*fundingFractionFactor;
				}
			if (forceFundingFraction != ""){
				fundingFraction = atof(forceFundingFraction.c_str());
			}
			productStartDateString  = szAllPrices[colProductStrikeDate];
			productCcy              = szAllPrices[colProductCcy];
			std::transform(std::begin(productCcy), std::end(productCcy), std::begin(productCcy), ::toupper);
			productBaseCcy          = szAllPrices[colProductBaseCcy];
			std::transform(std::begin(productBaseCcy), std::end(productBaseCcy), std::begin(productBaseCcy), ::toupper);
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
			if (productStartDateString == ""){ cerr << productId << "ProductStartDateString is empty..." << endl; continue; }
			cout << endl << endl << productIndx << " of " << numProducts << "\nIterations:" << thisNumIterations << " ProductId:" << productId << endl << endl;
			// cout << "Press a key to continue...";  getline(cin, word);  // KEEP in case you want to attach debugger

			// see if product has levelsCall payoffs ... in which case we do not do a lognormalShift of the underlyings
			bool doPriceShift(true);
			if (thisNumIterations <= 1){
				sprintf(lineBuffer, "%s%d%s", "select count(*) from productbarrier join payofftype pt using (payofftypeid) where productid='", productId, "' and pt.name like 'levels%'");
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				retcode = mydb.fetch(true, lineBuffer);
				if (retcode == SQL_SUCCESS && atof(szAllPrices[0]) > 0){ doPriceShift = false; }
			}
			
			// get counterparty info
			// ...mult-issuer product's have comma-separated issuers...ASSUMED equal weight
			sprintf(lineBuffer, "%s%d%s", "select EntityName from institution where institutionid='", counterpartyId, "' ");
			mydb.prepare((SQLCHAR *)lineBuffer, 1);
			retcode = mydb.fetch(true,lineBuffer);
			string counterpartyName = szAllPrices[0];
			vector<string> counterpartyNames;
			splitCommaSepName(counterpartyNames, counterpartyName);
			sprintf(charBuffer, "%s%s%s", "'", counterpartyNames.at(0).c_str(), "'");
			for (i = 1; i < (int)counterpartyNames.size(); i++){
				sprintf(charBuffer, "%s%s%s%s", charBuffer, ",'", counterpartyNames.at(i).c_str(), "'");
			}

			if (counterpartyNames.size()>1){
				sprintf(lineBuffer, "%s%s%s", "select Maturity,avg(Spread) Spread from cdsspread join institution using (institutionid) where EntityName in (", charBuffer,
					") and spread is not null and ccy = '' group by Maturity order by Maturity");
			}
			// see if there are ccy-specific spreads
			else {
				sprintf(lineBuffer, "%s%d%s%s%s%d%s","select * from (select maturity,spread from cdsspread where institutionid=", counterpartyId,
					" and ccy='", productCcy.c_str(), "'  and spread is not null union select maturity,spread from cdsspread where institutionid=",counterpartyId,"  and ccy=''  and spread is not null)x group by Maturity order by Maturity");
			}
			mydb.prepare((SQLCHAR *)lineBuffer, 2);
			retcode = mydb.fetch(false,lineBuffer);
			vector<double> cdsTenor, cdsSpread;
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				double thisSpread,thisTenor;
				thisTenor = atof(szAllPrices[0]);
				cdsTenor.push_back(thisTenor);
				thisSpread = atof(szAllPrices[1]) / 10000.0;
				cdsSpread.push_back(thisSpread);
				if (fabs(thisTenor - 5.0) < 0.01){
					cds5y = thisSpread;
				}
				retcode = mydb.fetch(false,"");
			}

			// get baseCurve
			sprintf(lineBuffer, "%s%s%s", "select Tenor,Rate/100 Spread from curve where ccy='", productCcy.c_str(),
				"' order by Tenor");
			mydb.prepare((SQLCHAR *)lineBuffer, 2);
			retcode = mydb.fetch(false,lineBuffer);
			vector<SomeCurve> baseCurve;
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				anyCurve.tenor  = atof(szAllPrices[0]);
				anyCurve.spread = atof(szAllPrices[1]);
				baseCurve.push_back(anyCurve);
				retcode = mydb.fetch(false,"");
			}


			// get underlyingids for this product from DB
			// they can come in any order of UnderlyingId (this is deliberate to avoid the code becoming dependent on any ordering
			double maxTZhrs(0.0);
			vector<int> ulIds, ulPriceReturnUids;
			vector<string> ulCcys;
			vector<bool> ulFixedDivs;
			vector<string> ulNames;
			map<string, int> ccyToUidMap;
			vector<double> ulERPs, ulTZhrs;
			vector<int> ulIdNameMap(1000);  // underlyingId -> arrayIndex, so ulIdNameMap[uid] gives the index into ulPrices vector
			sprintf(lineBuffer, "%s%s%s%s%s%d%s", "select distinct u.UnderlyingId UnderlyingId,upper(u.ccy) ulCcy,ERP,u.name,PriceReturnUid,TZhrs,FixedDivs from ", useProto, "productbarrier join ", useProto, "barrierrelation using (ProductBarrierId) join underlying u using (underlyingid) where ProductId='",
				productId, "' ");
			if (benchmarkId && !getMarketData && !forOptimisation){
				sprintf(charBuffer, "%s%d%s%s%s%d%s", " union (select ", benchmarkId, ",upper(u.ccy) ulCcy,ERP,u.name,PriceReturnUid,TZhrs,FixedDivs from ", useProto, "product p join underlying u on (p.BenchmarkId=u.UnderlyingId) where ProductId='", productId, "') ");
				strcat(lineBuffer, charBuffer);
			}
			mydb.prepare((SQLCHAR *)lineBuffer, 7);
			retcode = mydb.fetch(true,lineBuffer);
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
				if (forOptimisation && find(optimiseProductIds.begin(), optimiseProductIds.end(), uid) == optimiseProductIds.end()){
					cerr << "cannot optimise with this underlyingId:" << uid << endl;
					exit(107);
				}
				ccyToUidMap[thisCcy] = uid;
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
						retcode = mydb.fetch(false,lineBuffer);
						if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
							int thisId = atoi(szAllPrices[0]);
							if (currencyStruck             ){ crossRateUids[i]       = thisId; }
							if (doPriips){ 
								double previousPrice[2];
								quantoCrossRateUids[i] = thisId;
								// get prices for ul and ccy
								sprintf(lineBuffer, "%s%d%s%d%s", "select Date,p0.price ul,p1.price ccy from prices p0 join prices p1 using (Date) where p0.underlyingid=", ulIds[i], " and p1.underlyingid=", thisId, priipsStartDatePhrase);
								mydb.prepare((SQLCHAR *)lineBuffer, 3);
								retcode   = mydb.fetch(false, lineBuffer);
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
									quantoCorrelations[i] = MyCorrelation(theseReturns[0], theseReturns[1]);
									double mean, stdev, stdErr;
									MeanAndStdev(theseReturns[1], mean, stdev, stdErr);
									quantoCrossRateVols[i] = stdev;
								}
							}
						}
					}
				}
			}



			// read underlying prices
			vector<double>   ulReturns[maxUls], originalUlReturns[maxUls];
			for (i = 0; i < numUl; i++) {
				ulReturns[i].reserve(10000); 
				ulOriginalPrices[i].date.reserve(10000);
				ulOriginalPrices[i].price.reserve(10000);
				ulOriginalPrices[i].nonTradingDay.reserve(10000);
			}
			char ulSql[10000]; // enough for around 100 underlyings...
			char crossRateBuffer[100];
			

			// ...form sql joins
			sprintf(ulSql, "%s", "select p0.Date Date");
			for (i = 0; i<numUl; i++) { 
				if (crossRateUids[i]){ sprintf(crossRateBuffer, "%s%d%s", "*p",(numUl + i),".price");}
				sprintf(lineBuffer, "%s%d%s%s%s%d", ",p", i, ".price", (crossRateUids[i] ? crossRateBuffer : ""), " Price", i); strcat(ulSql, lineBuffer);
			}
			strcat(ulSql, " from prices p0 ");
			if (crossRateUids[0]){ sprintf(crossRateBuffer, "%s%d%s", " join prices p", numUl, " using (Date) "); strcat(ulSql, crossRateBuffer); }
			for (i = 1; i < numUl; i++) { 
				if (crossRateUids[i]){ sprintf(crossRateBuffer, "%s%d%s", " join prices p",(numUl + i)," using (Date) "); }
				sprintf(lineBuffer, "%s%d%s%s", " join prices p", i, " using (Date) ", (crossRateUids[i] ? crossRateBuffer : "")); strcat(ulSql, lineBuffer);
			}
			sprintf(lineBuffer, "%s%d%s", " where p0.underlyingId = '", ulIds.at(0), "'");
			strcat(ulSql, lineBuffer);
			if (crossRateUids[0]){ sprintf(crossRateBuffer, "%s%d%s%d%s", " and p",numUl, ".underlyingid='", crossRateUids[0], "'"); strcat(ulSql, crossRateBuffer); }
			for (i = 1; i < numUl; i++) {
				if (crossRateUids[i]){ sprintf(crossRateBuffer, "%s%d%s%d%s", " and p",(numUl + i),".underlyingid='",crossRateUids[i],"'" ); }
				sprintf(lineBuffer, "%s%d%s%d%s%s", " and p", i, ".underlyingId='", ulIds.at(i), "'", (crossRateUids[i] ? crossRateBuffer : "")); strcat(ulSql, lineBuffer);
			}
			if (strlen(startDate)) { sprintf(ulSql, "%s%s%s%s", ulSql, " and Date >='", startDate, "'"); }
			else {
				if (doPriips){ sprintf(ulSql, "%s%s", ulSql, priipsStartDatePhrase); }
				else if (forceStartDate != "0000-00-00"){ sprintf(ulSql, "%s%s%s%s", ulSql, " and Date >='", forceStartDate.c_str(), "'"); }
				else if (thisNumIterations>1) { strcat(ulSql, " and Date >='1992-12-31' "); }
			}
			if (strlen(endDate))   { sprintf(ulSql, "%s%s%s%s", ulSql, " and Date <='", endDate,   "'"); }
			strcat(ulSql, " order by Date");
			// cerr << ulSql << endl;
			// ...call DB
			mydb.prepare((SQLCHAR *)ulSql, numUl + 1);
			firstTime = true;
			vector<double> previousPrice(numUl);
			vector<double> minPrices, maxPrices, shiftPrices;
			vector<bool>   doShiftPrices;
			for (i = 0; i < numUl; i++) {
				minPrices.push_back(DBL_MAX);
				maxPrices.push_back(-DBL_MAX);
				shiftPrices.push_back(0.0);
				doShiftPrices.push_back(false);
			}
			// .. parse each record <Date,price0,...,pricen>
			retcode = mydb.fetch(true,ulSql);
			int numGaps = 0;
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
				int    numDayDiff;
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
				for (i = 0; i < numUl; i++) {
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
			if (ulOriginalPrices.at(0).date[0] > productStartDateString){
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
			// change to ASK; to use MID do: (bidPrice + askPrice) / (2.0*issuePrice)
			bool ignoreBidAsk    = ((bidAskDateString < lastDataDateString) || stalePrice);
			bool validFairValue  = (fairValueDateString == lastDataDateString) && (daysExtant > 14) && fairValuePrice>0.0;
			bool isPostStrike    = productStartDateString < lastDataDateString;
			midPrice             = (isPostStrike && ignoreBidAsk && validFairValue ? fairValuePrice : (ignoreBidAsk ? (validFairValue && isPostStrike ? fairValuePrice : issuePrice) : askPrice)) / issuePrice;
			if (strlen(endDate)){
				// get ASK from productprices if exists
				sprintf(lineBuffer, "%s%d%s%s%s", "select Ask from productprices where ProductId=", productId," and date<='",endDate,"' order by Date desc limit 1");
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				retcode = mydb.fetch(false, lineBuffer);
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					midPrice  = atof(szAllPrices[0]) / issuePrice;
					sprintf(lineBuffer, "%s%s%s%d", "update product set BidAskDate='",endDate,"' where ProductId=", productId);
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
				}
			}
			if (doUseThisPrice){ midPrice = useThisPrice / issuePrice; }
			ulPrices             = ulOriginalPrices; // copy constructor called
			cout << "NumPrices: " << totalNumDays << "  FirstDataDate: " << ulOriginalPrices.at(0).date[0] << " LastDataDate: " << lastDataDateString << "  MidPriceUsed: " << midPrice << endl;
			// save spots
			vector<double> spots;
			for (i=0; i < numUl; i++){ spots.push_back(ulPrices[i].price[totalNumDays-1]); }

			double forwardStartT(0.0);
			if (daysExtant < 0){
				if (!doUKSPA && !doPriips){ forwardStartT = daysExtant / 365.25; }
				daysExtant = 0; 
			}
			int tradingDaysExtant(0);
			for (i = 0; ulOriginalPrices.at(0).date[totalNumDays - 1 - i] > productStartDateString; i++){
				tradingDaysExtant += 1;
			}


			// calc baseCcyReturn: this caters for products like #1093 where GBP invests in USD and is quoted in GBP, so has to reflect the USDGBP return since StrikeDate
			baseCcyReturn = 1.0;
			if (productBaseCcy != ""){
				anyString = productCcy + productBaseCcy;
				sprintf(lineBuffer, "%s%s%s%s%s%s%s", "select p1.Price/p0.Price from prices p0 join prices p1 using (underlyingid) join underlying u using (underlyingid) where u.name='", 
					anyString.c_str(), "' and p0.date='", productStartDateString.c_str(), "' and p1.date='", lastDataDateString.c_str(),"'");
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				retcode = mydb.fetch(false, "");
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
					baseCcyReturn = atof(szAllPrices[0]);
				}
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
			vector< vector<vector<double>> >  ulVolsStrike(numUl);
			vector< vector<vector<double>> >  ulVolsImpVol(numUl);
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
				sprintf(ulSql, "%s%s%s%s%s%d", "select d.underlyingid,",
					doUKSPA || doPriips ? "100 Tenor,(d.divyield+dd.divyield)/2.0" : "Tenor,impdivyield",
					" Rate,IsTotalReturn,d.StdErr from ",
					doUKSPA || doPriips ? "divyield dd join divyield d using (underlyingid,userid)" : "impdivyield d",
					" join underlying u using (underlyingid) where d.UnderlyingId in (", ulIds[0]);
				for (i = 1; i < numUl; i++) {
					sprintf(ulSql, "%s%s%d", ulSql, ",", ulIds[i]);
				}
				sprintf(ulSql, "%s%s%d%s%s", ulSql, ") and d.userid=", userId,
					doUKSPA || doPriips ? " and dd.tenor=1 and d.tenor=5 " : "",
					" order by UnderlyingId,Tenor ");
				// .. parse each record <Date,price0,...,pricen>
				mydb.prepare((SQLCHAR *)ulSql, 5);
				retcode = mydb.fetch(false, ulSql);
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
			if (getMarketData || useUserParams){
				int    thisUidx;
				double thisFwdVol, thisTenor, previousVol, previousTenor;
				vector<double> someVols, someStrikes, someFwdVols;

				// vols
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
				else {
					sprintf(ulSql, "%s%s%s%d", "select UnderlyingId,Tenor,Strike,ImpVol from ",localVol && getMarketData && !useUserParams ? "local":"imp","vol where underlyingid in (", ulIds[0]);
					for (i = 1; i < numUl; i++) {
						sprintf(ulSql, "%s%s%d", ulSql, ",", ulIds[i]);
					}
					sprintf(ulSql, "%s%s%d%s", ulSql, ") and userid=", userId, " order by UnderlyingId, Tenor, Strike");
					// .. parse each record <UnderlyingId,Tenor,Strike,Impvol>
					thisUidx    = 0;          // underlying index
					thisTenor   = -1.0;
					mydb.prepare((SQLCHAR *)ulSql, 4);
					retcode = mydb.fetch(true, ulSql);

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
				sprintf(ulSql, "%s%s", "select ccy,Tenor,Rate from oncurve v where ccy in ('", ulCcys[0].c_str());
				for (i = 1; i < numUl; i++) {
					sprintf(ulSql, "%s%s%s", ulSql, "','", ulCcys[i].c_str());
				}
				sprintf(ulSql, "%s%s", ulSql, "') order by ccy,Tenor");
				// .. parse each record <Date,price0,...,pricen>
				string thisCcy = "";
				mydb.prepare((SQLCHAR *)ulSql, 3);
				retcode = mydb.fetch(false, ulSql);
				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
					string thisCcy   = szAllPrices[0];
					double thisTenor = atof(szAllPrices[1]);
					double thisRate  = atof(szAllPrices[2]);
					int    thisUidx  = ulIdNameMap[ccyToUidMap[thisCcy]];
					if (doUseThisOIS){ thisRate = useThisOIS; }
					oisRatesRate[thisUidx].push_back(thisRate / 100.0);
					oisRatesTenor[thisUidx].push_back(thisTenor);
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
							double thisCorr = MyCorrelation(tempReturns, tempReturns1);
							corrsOtherId[thisUidx].push_back(otherUidx);
							corrsCorrelation[thisUidx].push_back(thisCorr);
						}
					}
				}
				else {
					sprintf(ulSql, "%s%d", "select UnderlyingId,OtherId,Correlation from correlation c where OtherIdIsCcy=0 and UnderlyingId in (", ulIds[0]);
					for (i = 1; i < numUl; i++) {
						sprintf(ulSql, "%s%s%d", ulSql, ",", ulIds[i]);
					}
					sprintf(ulSql, "%s%s%d", ulSql, ") and OtherId in (", ulIds[0]);
					for (i = 1; i < numUl; i++) {
						sprintf(ulSql, "%s%s%d", ulSql, ",", ulIds[i]);
					}
					sprintf(ulSql, "%s%s%d%s", ulSql, ")  and userid=", userId, " order by UnderlyingId,OtherId ");
					// .. parse each record <Date,price0,...,pricen>
					mydb.prepare((SQLCHAR *)ulSql, 3);
					retcode   = mydb.fetch(false, ulSql);
					while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
						int    thisUid    = atoi(szAllPrices[0]);
						int    otherId    = atoi(szAllPrices[1]);
						int    thisUidx   = ulIdNameMap.at(thisUid);
						int    otherUidx  = ulIdNameMap.at(otherId);
						double thisCorr   = atof(szAllPrices[2]);
						corrsOtherId[thisUidx].push_back(otherUidx);
						if (forceEqEqCorr && ((eqCorrelationUid == thisUid && eqCorrelationOtherId == otherId) || (eqCorrelationUid == otherId && eqCorrelationOtherId == thisUid))){
							thisCorr = forceEqEqCorrelation;
						}
						corrsCorrelation[thisUidx].push_back(thisCorr);
						retcode = mydb.fetch(false, "");
					}
				}
				//  eq-fx corr
				sprintf(ulSql, "%s%d", "select UnderlyingId,OtherId,Correlation from correlation c join currencies y on (y.CcyId=c.OtherId) where OtherIdIsCcy=1 and UnderlyingId in (", ulIds[0]);
				for (i = 1; i < numUl; i++) {
					sprintf(ulSql, "%s%s%d", ulSql, ",", ulIds[i]);
				}
				sprintf(ulSql, "%s%s%s%s%d%s", ulSql, ") and y.Name='", productCcy.c_str(), "'  and userid=", userId, " order by UnderlyingId,OtherId ");
				// .. parse each record <Date,price0,...,pricen>
				mydb.prepare((SQLCHAR *)ulSql, 3);
				retcode   = mydb.fetch(false, ulSql);
				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
					int    thisUid    = atoi(szAllPrices[0]);
					int    thisUidx   = ulIdNameMap.at(thisUid);
					int    otherId    = atoi(szAllPrices[1]);
					int    otherUidx  = ulIdNameMap.at(otherId);
					double thisCorr   = atof(szAllPrices[2]);
					fxcorrsOtherId[thisUidx].push_back(otherUidx);
					fxcorrsCorrelation[thisUidx].push_back(forceEqFxCorr &&	fxCorrelationUid == thisUid && fxCorrelationOtherId == otherId ? forceEqFxCorrelation : thisCorr);
					retcode = mydb.fetch(false, "");
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

			// enough data?
			if (totalNumDays - 1 < daysExtant){
				cerr << "Not enough data to determine strike for product#:" << productId << endl;
				if (doDebug){ exit(109); }
				continue;
			}


			// create product
			if (AMC != 0.0){
				cerr << endl << "******NOTE******* product has an AMC:" << AMC << endl;
			}
			SProduct spr(mydb,&lineBuffer[0],bLastDataDate,productId, userId, productCcy, ulOriginalPrices.at(0), bProductStartDate, fixedCoupon, couponFrequency, couponPaidOut, AMC, showMatured,
				productShape, fullyProtected, benchmarkStrike,depositGteed, collateralised, daysExtant, midPrice, baseCurve, ulIds, forwardStartT, issuePrice, ukspaCase,
				doPriips,ulNames,(fairValueDateString == lastDataDateString),fairValuePrice / issuePrice, askPrice / issuePrice,baseCcyReturn,
				shiftPrices, doShiftPrices, forceIterations, optimiseMcLevels, optimiseUlIdNameMap,forOptimisation, productIndx,
				bmSwapRate, bmEarithReturn, bmVol, cds5y, bootstrapStride, settleDays, silent, doBumps, stochasticDrift, localVol, ulFixedDivs);
			numBarriers = 0;

			// get barriers from DB
			enum {
				colProductBarrierId = 0, colProductId,
				colCapitalOrIncome, colNature, colPayoff, colTriggered, colSettlementDate, colDescription, colPayoffId, colParticipation,
				colStrike, colAvgTenor, colAvgFreq, colAvgType, colCap, colUnderlyingFunctionId, colParam1, colMemory, colIsAbsolute, colAvgInTenor, colAvgInFreq, colStrikeReset, colStopLoss, colAvgInAlgebra, colOriginalStrike, colForfeitCoupons, colCommands,colProductBarrierLast
			};
			sprintf(lineBuffer, "%s%s%s%d%s", "select * from ", useProto, "productbarrier where ProductId='", productId, "' order by SettlementDate,ProductBarrierId");
			mydb.prepare((SQLCHAR *)lineBuffer, colProductBarrierLast);
			retcode = mydb.fetch(true,lineBuffer);
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
				double thisBarrierBend  = getMarketData && !doUKSPA ? (thisCoupon > 0.0 ? 0.1*(thisCoupon>0.5 ? 0.5 : thisCoupon) : barrierBend) : 0.0;  // 10% of any coupon, but limit to 5%
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
				strike          = max(0.0,atof(szAllPrices[colStrike]) + thisBarrierBend*bendDirection);
				cap             = max(0.0,atof(szAllPrices[colCap])    - barrierBend*bendDirection);
				int     underlyingFunctionId = atoi(szAllPrices[colUnderlyingFunctionId]);
				double  param1 = atof(szAllPrices[colParam1]);
				if (ucPayoffType.find("BASKET") != std::string::npos){
					param1 = max(0.0, param1 + thisBarrierBend*bendDirection);
				}
				
				/*
				* barrier creation
				*/
				spr.barrier.push_back(SpBarrier(barrierId, capitalOrIncome, nature, payoff, settlementDate, description,
					thisPayoffType, thisPayoffId, strike, cap, underlyingFunctionId, param1, participation, ulIdNameMap, avgDays, avgType,
					avgFreq, isMemory, isAbsolute, isStrikeReset, isStopLoss, isForfeitCoupons, barrierCommands, daysExtant, bProductStartDate, doFinalAssetReturn, midPrice,
					thisBarrierBend,bendDirection));
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
							monDateT.push_back(thisEndDays);
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
										monDateT.push_back((double)j / 365.25);
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

			//	add vol tenors to MonDates
			spr.reportableMonDateIndx = monDateIndx;
			if (getMarketData || useUserParams){
				int  maxObsDays = monDateIndx.size() > 0 ? monDateIndx[monDateIndx.size() - 1] : 1000000;
				for (i = 0; i < ulVolsTenor[0].size(); i++) {
					double thisT  = 365.25*ulVolsTenor[0][i];
					int theseDays = (int)floor(thisT);
					if (maxObsDays >= theseDays && find(monDateIndx.begin(), monDateIndx.end(), theseDays) == monDateIndx.end()) {
						monDateIndx.push_back((int)theseDays);
						monDateT.push_back(thisT);
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
				for (i = 0; i < numUl; i++) {
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
			vector<double> hazardCurve;  // annual default probability curve
			const double recoveryRate(0.4); 
			buildHazardCurve(cdsSpread, cdsTenor, maxYears, recoveryRate, hazardCurve);

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
			

			// get accrued coupons
			double accruedCoupon(0.0);
			bool   productHasMatured(false);
			spr.evaluate(totalNumDays, totalNumDays - 1, totalNumDays, 1, historyStep, ulPrices, ulReturns,
				numBarriers, numUl, ulIdNameMap, accrualMonDateIndx, accrualMonDateT, recoveryRate, hazardCurve, mydb, accruedCoupon, true, false, doDebug, startTime, benchmarkId, benchmarkMoneyness,
				contBenchmarkTER, hurdleReturn, false, false, timepointDays, timepointNames, simPercentiles, false, useProto, false /* getMarketData */,useUserParams,thisMarketData,
				cdsTenor, cdsSpread, fundingFraction, productNeedsFullPriceRecord, false, thisFairValue, false, false, productHasMatured, /* priipsUsingRNdrifts */ false,/* updateCashflows */false);

			// ...check product not matured
			numMonPoints = (int)monDateIndx.size();
			if (productHasMatured || !numMonPoints || (numMonPoints == 1 && monDateIndx[0] == 0)){ continue; }

			// finally evaluate the product...1000 iterations of a 60barrier product (eg monthly) = 60000
			spr.productDays    = *max_element(monDateIndx.begin(), monDateIndx.end());
			int thisStartPoint =  thisNumIterations == 1 ? daysExtant : totalNumDays - 1;
			int thisLastPoint  =  thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays; /*daysExtant + 1*/
			if (!doPriips && (thisNumIterations>1 || thisStartPoint<thisLastPoint)){
				EvalResult evalResult(0.0,0.0);
				// first-time we set conserveRande=true and consumeRande=false
				evalResult = spr.evaluate(totalNumDays,thisStartPoint, thisLastPoint, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
					numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurve, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, startTime, benchmarkId, benchmarkMoneyness,
					contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles,false /* doPriipsStress */, 
					useProto, getMarketData, useUserParams, thisMarketData,cdsTenor, cdsSpread, fundingFraction, productNeedsFullPriceRecord, 
					ovveridePriipsStartDate, thisFairValue, doBumps /* conserveRands */, false /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,/* updateCashflows */!doBumps && !solveFor);
				if (solveFor){
					// Newton-Raphson		
					int maxit    = 100;
					double xacc  = 0.0001;      // 10bp accuracy
					double x1    =  0.5;       // lower bound guess
					double x2    =  2.0;       // upper bound guess
					double solverStep = 0.03;
					double solverParam(0.0);
					int  j;
					double  dx, dxold, f,f2, df, fh, fl, temp, xh, xl, rts;
					EvalResult evalResult1(0.0, 0.0), evalResult2(0.0, 0.0);
					string adviceString = " - please choose a TargetValue closer to the current FairValue, or modify the product so as to have a FairValue closer to your TargetValue";
					// check product has some starting data
					bool couponFound(false);
					int numIncomeBarriers(0);
					double coupon(0.0);
					bool putFound(false);
					double solveBarrier;
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
						break;
					case solveForPutBarrier:
						for (j=0; !putFound && j < numBarriers; j++){
							SpBarrier& b(spr.barrier.at(j));
							if (b.capitalOrIncome && b.participation < 0.0 && (int)b.brel.size()>0){
								putFound = true;
								solveBarrier = b.brel[0].barrier;
							}
						}
						if (!putFound){
							sprintf(lineBuffer, "%s%s%s", "solveFor:0:", whatToSolveFor.c_str(), ":no put barrier found");
							std::cout << lineBuffer << std::endl;
							return(105);
						}
						solverParam = solveBarrier;
						break;
					} // switch

					// possibly done already
					if (abs(evalResult.value - targetFairValue) < xacc){
						sprintf(lineBuffer, "%s%s%s%.4lf", "solveFor:1:", whatToSolveFor.c_str(), ":", solverParam);
						std::cout << lineBuffer << std::endl;
						return(0);
					}


					// initial values at upper/lower bound
					spr.solverSet(solveForThis,solverParam*x1);
					cerr << "try:" << solverParam*x1 << endl;
					evalResult1 = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
						numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurve, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, startTime, benchmarkId, benchmarkMoneyness,
						contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
						useProto, getMarketData, useUserParams, thisMarketData, cdsTenor, cdsSpread, fundingFraction, productNeedsFullPriceRecord,
						ovveridePriipsStartDate, thisFairValue, doBumps /* conserveRands */, false /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,/* updateCashflows */false);
					fl = evalResult1.value - targetFairValue;
					if (fl == 0.0) {
						if (solveForCommit) { spr.solverCommit(solveForThis, solverParam*x1); }
						sprintf(lineBuffer, "%s%s%s%.4lf", "solveFor:1:", whatToSolveFor.c_str(), ":", solverParam*x1);
						std::cout << lineBuffer << std::endl;
						return(0);
					}
					spr.solverSet(solveForThis, solverParam*x2);
					cerr << "try:" << solverParam*x2 << endl;
					evalResult2 = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
						numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurve, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, startTime, benchmarkId, benchmarkMoneyness,
						contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
						useProto, getMarketData, useUserParams, thisMarketData, cdsTenor, cdsSpread, fundingFraction, productNeedsFullPriceRecord,
						ovveridePriipsStartDate, thisFairValue, doBumps /* conserveRands */, false /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,/* updateCashflows */false);
					fh = evalResult2.value - targetFairValue;
					if (fh == 0.0) {
						if (solveForCommit) { spr.solverCommit(solveForThis, solverParam*x2); }
						sprintf(lineBuffer, "%s%s%s%.4lf", "solveFor:1:", whatToSolveFor.c_str(),":", solverParam*x2);
						std::cout << lineBuffer << std::endl;
						return(0);
					}
					
					// handle when not bracketed
					if ((fl>0.0 && fh>0.0) || (fl<0.0 && fh<0.0)){
						sprintf(lineBuffer, "%s%s%s%s", "solveFor:0:", whatToSolveFor.c_str(), ":noSolution", adviceString.c_str());
						std::cout << lineBuffer << std::endl;
						return(0);
					}
					// orient the search so that f(x1)<0
					if (fl<0.0){ xl=x1; xh=x2; }
					else      { xh=x1; xl=x2; }
					rts   = 0.5*(x1 + x2);      // initial guess for root
					dxold = abs(x2 - x1);       // stepsize before last
					dx    = dxold;              // last stepsize
					// initial f and df
					spr.solverSet(solveForThis, solverParam*rts);
					cerr << "try:" << solverParam*rts << endl;
					evalResult1 = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
						numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurve, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, startTime, benchmarkId, benchmarkMoneyness,
						contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
						useProto, getMarketData, useUserParams, thisMarketData, cdsTenor, cdsSpread, fundingFraction, productNeedsFullPriceRecord,
						ovveridePriipsStartDate, thisFairValue, doBumps /* conserveRands */, false /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,/* updateCashflows */false);
					f = evalResult1.value - targetFairValue;
					spr.solverSet(solveForThis, solverParam*(rts + solverStep));
					evalResult2 = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
						numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurve, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, startTime, benchmarkId, benchmarkMoneyness,
						contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
						useProto, getMarketData, useUserParams, thisMarketData, cdsTenor, cdsSpread, fundingFraction, productNeedsFullPriceRecord,
						ovveridePriipsStartDate, thisFairValue, doBumps /* conserveRands */, false /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,/* updateCashflows */false);
					f2 = evalResult2.value - targetFairValue;
					df = (f - f2) / solverStep;
					// iterate
					for (j=0; j<maxit; j++){
						if ((((rts - xh)*df - f)*((rts - xl)*df - f) > 0.0) ||  	// bisect if Newton out of range 
							(abs(f*2.0) > abs(dxold*df))){           // or not decreasing fast enough
							dxold = dx; dx = 0.5*(xh - xl); rts = xl + dx;
							if (xl == rts){
								if (solveForCommit) { spr.solverCommit(solveForThis, solverParam*rts); }
								sprintf(lineBuffer, "%s%s%s%.4lf", "solveFor:1:", whatToSolveFor.c_str(), ":", solverParam*rts);
								std::cout << lineBuffer << std::endl;
								return(0);
							}                       // finish if change in root negligible
						}
						else {                      // Newton step acceptable. Take it
							dxold=dx; dx=f / df; temp=rts; rts -= dx;
							if (temp == rts){
								if (solveForCommit) { spr.solverCommit(solveForThis, solverParam*rts); }
								sprintf(lineBuffer, "%s%s%s%.4lf", "solveFor:1:", whatToSolveFor.c_str(), ":", solverParam*rts);
								std::cout << lineBuffer << std::endl;
								return(0);
							}                     // finish if change in root negligible
						}
						if (abs(dx) < xacc){
							if (solveForCommit) { spr.solverCommit(solveForThis, solverParam*rts); }
							sprintf(lineBuffer, "%s%s%s%.4lf", "solveFor:1:", whatToSolveFor.c_str(), ":", solverParam*rts);
							std::cout << lineBuffer << std::endl;
							return(0);
						}                 // convergence criterion
						// initial f and df
						spr.solverSet(solveForThis, solverParam*rts);
						cerr << "try:" << solverParam*rts << endl;
						evalResult1 = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
							numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurve, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, startTime, benchmarkId, benchmarkMoneyness,
							contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
							useProto, getMarketData, useUserParams, thisMarketData, cdsTenor, cdsSpread, fundingFraction, productNeedsFullPriceRecord,
							ovveridePriipsStartDate, thisFairValue, doBumps /* conserveRands */, false /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,/* updateCashflows */false);
						f = evalResult1.value - targetFairValue;
						
						spr.solverSet(solveForThis, solverParam*(rts + solverStep));
						evalResult2 = spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
							numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurve, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, startTime, benchmarkId, benchmarkMoneyness,
							contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
							useProto, getMarketData, useUserParams, thisMarketData, cdsTenor, cdsSpread, fundingFraction, productNeedsFullPriceRecord,
							ovveridePriipsStartDate, thisFairValue, doBumps /* conserveRands */, false /* consumeRands */, productHasMatured,/* priipsUsingRNdrifts */ false,/* updateCashflows */false);
						f2 = evalResult2.value - targetFairValue;
						df = (f - f2) / solverStep;
						if (f<0.0){ xl=rts; }
						else { xh=rts; } // maintain the bracket on the root
					}
					{ //alert("IRR root-finding: iterations exhausted"); 
						sprintf(lineBuffer, "%s%s%s%s", "solveFor:0:", whatToSolveFor.c_str(), ": iterations exhausted", adviceString.c_str());
						std::cout << lineBuffer << std::endl;
						return(0);
					}
				}  // END solveFor


				
				
				double deltaBumpAmount(0.0), vegaBumpAmount(0.0), thetaBumpAmount(0.0), rhoBumpAmount(0.0), creditBumpAmount(0.0);
				if (doBumps && (deltaBumps || vegaBumps || thetaBumps || rhoBumps || creditBumps)  /* && daysExtant>0 */){
					vector< vector<vector<double>> >  holdUlFwdVol(thisMarketData.ulVolsFwdVol);
					vector< vector<vector<double>> >  holdUlImpVol(thisMarketData.ulVolsImpVol);
					vector<vector<vector<double>>>    holdUlVolsStrike(thisMarketData.ulVolsStrike);
					vector<double>                    holdCdsSpread(cdsSpread);
					vector<SomeCurve>                 holdBaseCurve(baseCurve);
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
					* credit loop
					*/
					for (int creditBump=0; creditBump < creditBumps; creditBump++){
						creditBumpAmount = creditBumpStart + creditBumpStep*creditBump;
						// change credit curve
						for (i=0; i < (int)cdsSpread.size(); i++){
							cdsSpread[i] = holdCdsSpread[i] + creditBumpAmount;
						}
						buildHazardCurve(cdsSpread, cdsTenor, maxYears, recoveryRate, hazardCurve);

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
								* delta loop
								*/
								for (int deltaBump=0; deltaBump < deltaBumps; deltaBump++){
									deltaBumpAmount = deltaBumpStart + deltaBumpStep*deltaBump;
									double bumpFactor = 1.0 / (1.0 + deltaBumpAmount);
									if (true || deltaBumpAmount != 0.0 || vegaBumpAmount != 0.0 || rhoBumpAmount != 0.0){
										// for each underlying
										for (i=0; i < numUl; i++){
											int ulId = ulIds[i];
											// bump spot
											double newSpot      = spots[i] * (1.0 + (doStickySmile ? 0.0 : deltaBumpAmount));
											double newMoneyness = newSpot / ulPrices[i].price[totalNumDays - 1 - daysExtant];
											if (doStickySmile){
												for (j=0; j < (int)thisMarketData.ulVolsTenor[i].size(); j++){
													for (k=0; k < (int)thisMarketData.ulVolsStrike[i][j].size(); k++){
														thisMarketData.ulVolsStrike[i][j][k] = holdUlVolsStrike[i][j][k] * bumpFactor;
													}
												}
											}
											ulPrices[i].price[totalNumDays - 1] = newSpot;
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
											// bump vols
											thisMarketData.ulVolsFwdVol[i] = theseUlFwdVol[i];
											thisMarketData.ulVolsImpVol[i] = theseUlImpVol[i];

											cerr << "BUMP:" << ulId << " credit:" << creditBumpAmount << " rho:" << rhoBumpAmount << " vega:" << vegaBumpAmount << " delta:" << deltaBumpAmount << endl;

											// re-evaluate
											spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
												numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurve, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, startTime, benchmarkId, benchmarkMoneyness,
												contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false, useProto, getMarketData, useUserParams, thisMarketData,
												cdsTenor, cdsSpread, fundingFraction, productNeedsFullPriceRecord, ovveridePriipsStartDate, bumpedFairValue, doBumps /* conserveRands */, true /* consumeRands */,
												productHasMatured,/* priipsUsingRNdrifts */ false,/* updateCashflows */false);
											if (doDeltas){
												if (deltaBumpAmount != 0.0){
													//  Elasticity: double  delta = (bumpedFairValue / thisFairValue - 1.0) / deltaBumpAmount;
													double  delta = (bumpedFairValue - thisFairValue) / deltaBumpAmount/issuePrice;
													sprintf(lineBuffer, "%s", "insert into deltas (Delta,DeltaType,LastDataDate,UnderlyingId,ProductId) values (");
													sprintf(lineBuffer, "%s%.5lf%s%d%s%s%s%d%s%d%s", lineBuffer, delta, ",", deltaBump == 0 ? 0 : 1, ",'", lastDataDateString.c_str(), "',", ulIds[i], ",", productId, ")");
													mydb.prepare((SQLCHAR *)lineBuffer, 1);
												}
											}
											else {
												sprintf(lineBuffer, "%s", "insert into bump (ProductId,UserId,UnderlyingId,DeltaBumpAmount,VegaBumpAmount,RhoBumpAmount,CreditBumpAmount,FairValue,BumpedFairValue,LastDataDate) values (");
												sprintf(lineBuffer, "%s%d%s%d%s%d%s%.5lf%s%.5lf%s%.5lf%s%.5lf%s%.5lf%s%.5lf%s%s%s", lineBuffer, productId, ",", bumpUserId, ",", ulIds[i], ",", deltaBumpAmount, ",", vegaBumpAmount, ",", rhoBumpAmount, ",", creditBumpAmount, ",", thisFairValue, ",", bumpedFairValue, ",'", lastDataDateString.c_str(), "')");
												mydb.prepare((SQLCHAR *)lineBuffer, 1);
											}
											// cerr << lineBuffer << endl;
											// ... reinstate spots
											ulPrices[i].price[totalNumDays - 1] = spots[i];
											// ... reinstate vols
											thisMarketData.ulVolsFwdVol[i] = holdUlFwdVol[i];
											thisMarketData.ulVolsImpVol[i] = holdUlImpVol[i];
											thisMarketData.ulVolsStrike[i] = holdUlVolsStrike[i];
										} // for (i=0; i < numUl; i++){
										// for ALL underlyings
										for (i=0; i < numUl; i++){
											int ulId = ulIds[i];
											// bump spot
											double newSpot      = spots[i] * (1.0 + (doStickySmile ? 0.0 : deltaBumpAmount));
											double newMoneyness = newSpot / ulPrices[i].price[totalNumDays - 1 - daysExtant];
											ulPrices[i].price[totalNumDays - 1] = newSpot;
											if (doStickySmile){
												for (j=0; j < (int)thisMarketData.ulVolsTenor.size(); j++){
													for (k=0; k < (int)thisMarketData.ulVolsStrike[i][j].size(); k++){
														thisMarketData.ulVolsStrike[i][j][k] = holdUlVolsStrike[i][j][k] * bumpFactor;
													}
												}
											}
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
												}
											}
											// bump vols
											thisMarketData.ulVolsFwdVol[i] = theseUlFwdVol[i];
											thisMarketData.ulVolsImpVol[i] = theseUlImpVol[i];
										}
										// re-evaluate
										cerr << "BUMPALL: credit:" << creditBumpAmount << " rho:" << rhoBumpAmount << " vega:" << vegaBumpAmount << " delta:" << deltaBumpAmount << endl;
										spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
											numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurve, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, startTime, benchmarkId, benchmarkMoneyness,
											contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false, useProto, getMarketData, useUserParams, thisMarketData,
											cdsTenor, cdsSpread, fundingFraction, productNeedsFullPriceRecord, ovveridePriipsStartDate, bumpedFairValue, doBumps /* conserveRands */, true,
											productHasMatured,/* priipsUsingRNdrifts */ false,/* updateCashflows */false);
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
											sprintf(lineBuffer, "%s", "insert into bump (ProductId,UserId,UnderlyingId,DeltaBumpAmount,VegaBumpAmount,RhoBumpAmount,CreditBumpAmount,FairValue,BumpedFairValue,LastDataDate) values (");
											sprintf(lineBuffer, "%s%d%s%d%s%d%s%.5lf%s%.5lf%s%.5lf%s%.5lf%s%.5lf%s%.5lf%s%s%s", lineBuffer, productId, ",", bumpUserId, ",", 0, ",", deltaBumpAmount, ",", vegaBumpAmount, ",", rhoBumpAmount, ",", creditBumpAmount, ",", thisFairValue, ",", bumpedFairValue, ",'", lastDataDateString.c_str(), "')");
											mydb.prepare((SQLCHAR *)lineBuffer, 1);
											// save some greeks to product table
											if (bumpUserId == 3 && deltaBumpAmount == 0.0){
												// save vegas to product table
												if (vegaBumpAmount != 0.0 && thetaBumpAmount == 0.0 && creditBumpAmount == 0.0 && rhoBumpAmount == 0.0){
													double  vega = (bumpedFairValue - thisFairValue) / (100.0*vegaBumpAmount);
													sprintf(lineBuffer, "%s%s%s%.5lf%s%s%s%d%s", "update product set ", vegaBump == 0 ? "Vega" : "VegaUp", "=", vega, ",VegaDate='", lastDataDateString.c_str(), "' where productid=", productId, "");
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
						for (i=0; i < (int)cdsSpread.size(); i++){
							cdsSpread[i] = holdCdsSpread[i];
						}
						buildHazardCurve(cdsSpread, cdsTenor, maxYears, recoveryRate, hazardCurve);
					} // for (int creditBump=0; creditBump < creditBumps; creditBump++){
				}
			}

			// PRIIPs
			if (doPriips){
				// real-world drifts
				spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
					numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurve, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, startTime, benchmarkId, benchmarkMoneyness,
					contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
					useProto, getMarketData, useUserParams, thisMarketData,cdsTenor, cdsSpread, fundingFraction, productNeedsFullPriceRecord, 
					ovveridePriipsStartDate, thisFairValue, false, false, productHasMatured,/* priipsUsingRNdrifts */ false,/* updateCashflows */true);

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
				spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
					numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurve, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, startTime, benchmarkId, benchmarkMoneyness,
					contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, false /* doPriipsStress */,
					useProto, getMarketData, useUserParams, thisMarketData, cdsTenor, cdsSpread, fundingFraction, productNeedsFullPriceRecord,
					ovveridePriipsStartDate, thisFairValue, false, false, productHasMatured, /* priipsUsingRNdrifts */ true,/* updateCashflows */true);


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
				spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
					numBarriers, numUl, ulIdNameMap, monDateIndx, monDateT, recoveryRate, hazardCurve, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, startTime, benchmarkId, benchmarkMoneyness,
					contBenchmarkTER, hurdleReturn, doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, true /* doPriipsStress */,
					useProto, getMarketData, useUserParams, thisMarketData, cdsTenor, cdsSpread, fundingFraction, productNeedsFullPriceRecord,
					ovveridePriipsStartDate, thisFairValue, false, false, productHasMatured, /* priipsUsingRNdrifts */ true,/* updateCashflows */true);

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

	return 0;
}

