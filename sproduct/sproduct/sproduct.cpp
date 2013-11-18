// sproduct.cpp : Defines the entry point for the console application.
//
// http://www.boost.org/doc/libs/1_34_0/doc/html/date_time/examples.html
//
#include "stdafx.h"
#include "sproduct.h"
using namespace std;

/*
#import "C:\Program Files\Common Files\System\ado\MSADO15.dll" rename("EOF", "EOFile")
struct StartOLEProcess{
StartOLEProcess() {
::CoInitialize(NULL);
}
~StartOLEProcess() {
::CoUninitialize();
}
} _start_StartOLEProcess;


ADODB::_ConnectionPtr  con = NULL;
ADODB::_RecordsetPtr   rec = NULL;
bstr_t                 sConString;
bstr_t                 sSQLString;
HRESULT                hr = S_OK;
// long variable needed for Execute method of Connection object
VARIANT                *vRecordsAffected = NULL;
hr = con.CreateInstance(_uuidof(ADODB::Connection));
sConString = L"Driver = { MySQL ODBC 5.2a Driver }; Server = localhost;Database=sp;User=root;Password=ragtinmor;Option=4;";
con->Open(sConString, L"", L"", -1);


*/
int _tmain(int argc, char* argv[])
{
	// variables
	int numMcIterations = argc>1 ? atoi(argv[1]) : 100;
	int productId          = 363;
	string productStartDateString("2014-01-03");
	cout << "Iterations:" << numMcIterations << " ProductId:" << productId << endl;
	
		
	boost::gregorian::date  bProductStartDate(boost::gregorian::from_simple_string(productStartDateString));
	unsigned	uI; 
	int oldProductBarrierId = 0, productBarrierId = 0;
	int historyStep = 1;
	int numBarriers = 0, thisIteration = 0;
	int anyInt, i, j, k, len, callOrPut, thisPoint, thisBarrier, thisMonIndx, thisMonPoint, numUl, numMonPoints, lastPoint, productDays, totalNumDays, totalNumReturns,uid;
	int thisPayoffId,thisMonDays;
	double anyDouble, barrier, uBarrier, payoff, strike,cap,participation;
	string word, word1, thisPayoffType,startDateString, endDateString, nature, settlementDate, description;
	bool capitalOrIncome,above,at;
	vector<double> ulReturns;
	vector<int>    monDateIndx;
	vector<string> payoffType = { "", "fixed", "call", "put", "twinWin", "switchable", "basketCall", "lookbackCall" };
	vector<int>::iterator intIterator;

	// get underlying prices
	vector<UlTimeseries> ulOriginalPrices(1),ulPrices(1);
	char lineBuffer[1000], charBuffer[1000];

	ifstream ulFile;
	ulFile.open("c:/sites/sppdf/flatfiles/ulprices/1.txt");
	ulFile.getline(lineBuffer,1000); // headings row
	bool firstTime(true);
	double previousPrice;
	boost::gregorian::date lastDate;
	while (!ulFile.eof()) {
		int numDayDiff;
		double thisPrice;
		ulFile.getline(lineBuffer, 1000);
		istringstream iss(lineBuffer, istringstream::in);
		iss >> word >> word1;
		thisPrice = atof(word1.c_str());
		// pad non-trading days
		{
			using namespace boost::gregorian;
			date bDate(from_simple_string(word));
			if (!firstTime) {
				double thisReturn = thisPrice / previousPrice;
				ulReturns.push_back(thisReturn);
				date_duration dateDiff(bDate - lastDate);
				numDayDiff = dateDiff.days();
				while (numDayDiff>1){
					ulOriginalPrices.at(0).date.push_back(word);
					ulOriginalPrices.at(0).price.push_back(thisPrice);
					ulReturns.push_back(1.0);
					numDayDiff -= 1;
				}
			}
			else{ firstTime = false; }
			lastDate      = bDate;
			previousPrice = thisPrice;
		}
		ulOriginalPrices.at(0).date.push_back(word);
		ulOriginalPrices.at(0).price.push_back(thisPrice);
		//sscanf(lineBuffer, "%s%lf", charBuffer, &anyDouble);
	}
	ulFile.close();
	totalNumDays    = ulOriginalPrices.at(0).price.size();
	totalNumReturns = totalNumDays - 1;
	numUl           = ulOriginalPrices.size();
	ulPrices        = ulOriginalPrices; // copy onstructor called
	vector<double> thesePrices(numUl), startLevels(numUl);
	vector<int> ulIdNameMap = { -1, 0 };


	// get product
	SProduct spr;
	numBarriers = 0;
	ifstream pbFile;
	pbFile.open("c:/sites/sppdf/flatfiles/363.txt");
	while (!pbFile.eof()) {
		pbFile.getline(lineBuffer, 1000);
		productBarrierId = atoi(lineBuffer);
		if (productBarrierId != 0) {
			if (productBarrierId != oldProductBarrierId) {
				oldProductBarrierId = productBarrierId;
				pbFile.getline(lineBuffer, 1000);  // productId
				pbFile.getline(lineBuffer, 1000);  capitalOrIncome = atoi(lineBuffer) == 1; 
				pbFile.getline(lineBuffer, 1000);  nature          = lineBuffer;
				pbFile.getline(lineBuffer, 1000);  payoff          = atof(lineBuffer);
				pbFile.getline(lineBuffer, 1000);  // Triggered
				pbFile.getline(lineBuffer, 1000);  settlementDate  = lineBuffer;  
				pbFile.getline(lineBuffer, 1000);  description     = lineBuffer;  
				pbFile.getline(lineBuffer, 1000);  thisPayoffId    = atoi(lineBuffer); thisPayoffType = payoffType[thisPayoffId];// PayoffTypeId
				pbFile.getline(lineBuffer, 1000);  participation   = atof(lineBuffer); // Participation
				pbFile.getline(lineBuffer, 1000);  strike          = atof(lineBuffer);  // PayoffStrike
				pbFile.getline(lineBuffer, 1000);  // AvgTenor
				pbFile.getline(lineBuffer, 1000);  // AvgFreq
				pbFile.getline(lineBuffer, 1000);  // AvgType
				pbFile.getline(lineBuffer, 1000);  cap = atof(lineBuffer);  // Cap
				pbFile.getline(lineBuffer, 1000);  // UnderlyingFunctionId
				pbFile.getline(lineBuffer, 1000);  // Param1
				pbFile.getline(lineBuffer, 1000);  // Memory
				pbFile.getline(lineBuffer, 1000);  // IsAbsolute
				spr.barrier.push_back(SpBarrier(capitalOrIncome, nature, payoff, settlementDate, description, 
					thisPayoffType, thisPayoffId, strike, cap, participation, ulIdNameMap,bProductStartDate));
				anyInt = spr.barrier.at(numBarriers).getEndDays();
				if (find(monDateIndx.begin(), monDateIndx.end(), anyInt) == monDateIndx.end()) {
					monDateIndx.push_back(anyInt);
				}
				numBarriers += 1;
			}
			// barrier relations
			pbFile.getline(lineBuffer, 1000);  // BarrierRelationId
			pbFile.getline(lineBuffer, 1000);  uid = atoi(lineBuffer);     // UnderlyingId
			pbFile.getline(lineBuffer, 1000);  barrier = atof(lineBuffer); // Barrier
			pbFile.getline(lineBuffer, 1000);  // BarrierTypeId
			pbFile.getline(lineBuffer, 1000);  above            = atoi(lineBuffer) == 1; // Above
			pbFile.getline(lineBuffer, 1000);  at               = atoi(lineBuffer) == 1; // At
			pbFile.getline(lineBuffer, 1000);  startDateString  = lineBuffer;   // StartDate
			pbFile.getline(lineBuffer, 1000);  endDateString    = lineBuffer;   // EndDate
			pbFile.getline(lineBuffer, 1000);  // Triggered.1
			pbFile.getline(lineBuffer, 1000);  // IsAbsolute.1
			pbFile.getline(lineBuffer, 1000);  uBarrier = atof(lineBuffer); // UpperBarrier
			pbFile.getline(lineBuffer, 1000);  // Weight
			if (uid) {
				spr.barrier.at(numBarriers - 1).brel.push_back(SpBarrierRelation(uid, barrier, uBarrier, startDateString, endDateString, above,at,productStartDateString));
			}
		}
	}
	pbFile.close();	
	spr.productDays = *max_element(monDateIndx.begin(), monDateIndx.end());
	vector<int> numBarrierHits(numBarriers,0);
	numMonPoints = monDateIndx.size();


	// evaluate product
	productDays = spr.productDays;
	lastPoint = totalNumDays - productDays;
	// main MC loop
	for (thisIteration = 0; thisIteration < numMcIterations; thisIteration++) {
		// start a product on each date
		for (thisPoint = 0; thisPoint < lastPoint; thisPoint += historyStep) {
			// initialise product
			bool   matured(false);
			vector<double> lookbackLevel;
			// DOME
			startLevels[0] = ulPrices.at(0).price.at(thisPoint);
			for (thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
				SpBarrier &b(spr.barrier.at(thisBarrier));
				for (uI = 0; uI < b.brel.size(); uI++){
					SpBarrierRelation &thisBrel(b.brel.at(uI));
					thisBrel.setLevels(startLevels[0]);
				}
			}

			// go through each monitoring date
			for (thisMonIndx = 0; !matured && thisMonIndx < numMonPoints; thisMonIndx++){
				thisMonDays    = monDateIndx.at(thisMonIndx);
				thisMonPoint   = thisPoint + thisMonDays;
				const string   thisDateString(ulPrices.at(0).date.at(thisMonPoint));
				thesePrices[0] = ulPrices.at(0).price.at(thisMonPoint);
				// test each barrier
				for (thisBarrier = 0; !matured && thisBarrier<numBarriers; thisBarrier++){
					SpBarrier &b(spr.barrier.at(thisBarrier));
					// is barrier alive
					if (b.endDays == thisMonDays) {
						// is barrier hit
						if (b.isHit(thesePrices)){
							matured = b.capitalOrIncome;
							b.storePayoff(thisDateString, b.getPayoff(startLevels, lookbackLevel, thesePrices));
						}
					}
				}
			}
		}

		// create new random sample for next iteration
		for (i = 1; i < totalNumReturns; i++){
			int thisIndx = (int)floor(((double)rand() / (RAND_MAX))*(totalNumReturns-1));
			double thisReturn = ulReturns[thisIndx];
			ulPrices.at(0).price[i] = ulPrices.at(0).price[i-1] * thisReturn;
		}
		cout << ".";
	}

	int numAllIterations(0);
	for (thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
		numAllIterations += spr.barrier.at(thisBarrier).hit.size();
	}
	cout << endl;
	for (thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
		SpBarrier &b(spr.barrier.at(thisBarrier));
		double mean = b.sumPayoffs / b.hit.size();
		double prob = (100.0*b.hit.size()) / numAllIterations;
		/*
		printf("%20s\t%s\t%lf\t%s\t%lf\n", b.description,"Prob:", prob,"ExpectedPayoff",mean);
		*/
		cout << b.description << " Prob:" << prob << " ExpectedPayoff:" << mean << endl;
	}
	
	/**************************************************

	std::string line;
	boost::regex pat("^Subject: (Re: |Aw: )*(.*)");
	while (std::cin)
	{
		std::getline(std::cin, line);
		boost::smatch matches;
		if (boost::regex_match(line, matches, pat))
			std::cout << matches[2] << std::endl;
	}


	using namespace boost::lambda;
	typedef std::istream_iterator<int> in;

	std::for_each(
		in(std::cin), in(), std::cout << (_1 * 3) << " ");
	**************************************************/

	
	return 0;
}