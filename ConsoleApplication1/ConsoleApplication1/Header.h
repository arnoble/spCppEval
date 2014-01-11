#include <boost/lambda/lambda.hpp>
#include <boost/regex.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <windows.h>
#include <sqlext.h>
#include <stdio.h>
#include <vector>
#include <regex>


// cds functions
double interpCurve(std::vector<double> curveTimes, std::vector<double> curveValues,double point){
	int len = curveTimes.size();
	if (!len) return 0.0;
	if (point > curveTimes[len - 1]) return curveValues[len - 1];
	if (point < curveTimes[0])       return curveValues[0];
	int i;
	for (i = 0; point > curveTimes[i] && i < len; i++) {}
	// linear interpolation for now
	double value;
	if (point == curveTimes[i]) value = curveValues[i];
	else {
		double fraction = (point - curveTimes[i - 1]) / (curveTimes[i] - curveTimes[i - 1]);
		value = curveValues[i - 1] + fraction*(curveValues[i] - curveValues[i - 1]);
	}
	return value;
}

double probDefault(std::vector<double> curveProbs, const double point){
	int len = curveProbs.size();
	if (!len) return 0.0;
	double cumProb(0.0);
	int i;
	for (i = 0; point > i + 1 && i<len; i++) { cumProb += curveProbs[i]; }
	// linear interpolation for now
	if (i == 0 || point == i + 1) cumProb += curveProbs[i];
	else {
		double fraction = (point - i) ;
		cumProb += curveProbs[i - 1] + fraction*(curveProbs[i] - curveProbs[i - 1]);
	}
	return cumProb;
}

void bootstrapCDS(const std::vector<double> r, std::vector<double> &dpCurve, const double recoveryRate){
	int len(r.size());
	if (!len) return;
	double thisProb, cumProbAlive(0.0), probAliveThisPeriod(1.0), cumProbDefault(0.0);
	for (int i = 0; i<len; i++) {
		thisProb = (r[i] * (cumProbAlive + probAliveThisPeriod) - (1 - recoveryRate)*cumProbDefault) / ((r[i] + 1 - recoveryRate)*probAliveThisPeriod);
		probAliveThisPeriod  *= 1 - thisProb;
		cumProbAlive         += probAliveThisPeriod;
		cumProbDefault       += thisProb;
		dpCurve.push_back(thisProb);
	}
}


// regex functions
void splitCounterpartyName( std::vector<std::string> &out,std::string s){

	std::regex word_regex("([^,]+)");
	auto words_begin = std::sregex_iterator(s.begin(), s.end(), word_regex);
	auto words_end   = std::sregex_iterator();

	int numWords = std::distance(words_begin, words_end);
	
	for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
		std::smatch match = *i;
		out.push_back(match.str());
	}	
}

// ExpectedShortfall for standard normal density
double Dnorm(double x) { return(exp(-0.5 * x*x) / (2.506628)); }  // standard normal density
double NormSInv(double p) {
	double a1 = -39.6968302866538, a2 = 220.946098424521, a3 = -275.928510446969;
	double a4 = 138.357751867269, a5 = -30.6647980661472, a6 = 2.50662827745924;
	double b1 = -54.4760987982241, b2 = 161.585836858041, b3 = -155.698979859887;
	double b4 = 66.8013118877197, b5 = -13.2806815528857, c1 = -7.78489400243029E-03;
	double c2 = -0.322396458041136, c3 = -2.40075827716184, c4 = -2.54973253934373;
	double c5 = 4.37466414146497, c6 = 2.93816398269878, d1 = 7.78469570904146E-03;
	double d2 = 0.32246712907004, d3 = 2.445134137143, d4 = 3.75440866190742;
	double p_low = 0.02425, p_high = 1 - p_low;
	double q, r;
	double retVal;

	if ((p < 0) || (p > 1) || (p == 0) || (p == 1))	{ retVal = 0; }
	else if (p < p_low)	{
		q = sqrt(-2 * log(p));
		retVal = (((((c1 * q + c2) * q + c3) * q + c4) * q + c5) * q + c6) / ((((d1 * q + d2) * q + d3) * q + d4) * q + 1);
	}
	else if (p <= p_high) {
		q = p - 0.5;
		r = q * q;
		retVal = (((((a1 * r + a2) * r + a3) * r + a4) * r + a5) * r + a6) * q / (((((b1 * r + b2) * r + b3) * r + b4) * r + b5) * r + 1);
	}
	else {
		q = sqrt(-2 * log(1 - p));
		retVal = -(((((c1 * q + c2) * q + c3) * q + c4) * q + c5) * q + c6) / ((((d1 * q + d2) * q + d3) * q + d4) * q + 1);
	}
	return retVal;
}
double ESnorm(double prob) { return Dnorm(NormSInv(prob)) / prob; }




enum { fixedPayoff = 1, callPayoff, putPayoff, twinWinPayoff, switchablePayoff, basketCallPayoff, lookbackCallPayoff };
enum { uFnLargest = 1, uFnLargestN };








// *************** CLASSES

class MapType { 
public:
	MapType(const int id, const std::string name) : id(id),name(name) {}
	const int          id; 
	const std::string  name;
};


class MyDB {
private:
	SQLHENV   hEnv;
	SQLHDBC   hDBC;
	SQLRETURN fsts;
	const int bufSize=256;
	SQLLEN    cbModel;		               // Model buffer bytes recieved
	HSTMT     hStmt;
	char     **bindBuffer;

public:
	// get info on SQL error
	void extract_error(
		char *fn,
		SQLHANDLE handle,
		SQLSMALLINT type)
	{
		SQLINTEGER  i = 0;
		SQLINTEGER  native;
		SQLWCHAR    state[7];
		SQLWCHAR    text[256];
		SQLSMALLINT len;
		SQLRETURN   ret;
		fprintf(stderr,
			"\n"
			"The driver reported the following diagnostics whilst running "
			"%s\n\n",
			fn);
		do
		{
			ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len);
			if (SQL_SUCCEEDED(ret))
				printf("%s:%ld:%ld:%s\n", state, i, native, text);
		} while (ret == SQL_SUCCESS);
	}
	// open connection to DataSource newSp
	SQLRETURN dbConn(SQLHENV hEnv, SQLHDBC* hDBC) {
		SQLWCHAR              szDSN[]    = L"newSp";       // Data Source Name buffer
		SQLWCHAR              szUID[]    = L"root";		   // User ID buffer
		SQLWCHAR              szPasswd[] = L"ragtinmor";   // Password buffer
		SQLRETURN             fsts;

		fsts = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, hDBC);  // Allocate memory for the connection handle
		if (!SQL_SUCCEEDED(fsts))	{
			extract_error("SQLAllocHandle for dbc", hEnv, SQL_HANDLE_ENV);
			exit(1);
		}
		// Connect to Data Source
		fsts = SQLConnect(*hDBC, szDSN, SQL_NTS, szUID, SQL_NTS, szPasswd, SQL_NTS); // use SQL_NTS for length...NullTerminatedString
		if (!SQL_SUCCEEDED(fsts))	{
			extract_error("SQLAllocHandle for connect", hDBC, SQL_HANDLE_DBC);
			exit(1);
		}
		return fsts;
	}

	MyDB(char **bindBuffer) : bindBuffer(bindBuffer){
		SQLAllocEnv(&hEnv);
		fsts = dbConn(hEnv, &hDBC);              // connect
		if (fsts != SQL_SUCCESS && fsts != SQL_SUCCESS_WITH_INFO) { exit(1); }
	};
	~MyDB(){
		SQLDisconnect(hDBC);           // Disconnect from datasource
		SQLFreeConnect(hDBC); // Free the allocated connection handle
		SQLFreeEnv(hEnv);    // Free the allocated ODBC environment handle
	}
	void prepare(SQLCHAR* thisSQL,int numCols) {
		if (hStmt != NULL) {
			SQLFreeStmt(hStmt, SQL_DROP);
		}
		fsts  =  SQLAllocStmt(hDBC, &hStmt); 	 // Allocate memory for statement handle
		fsts  =  SQLPrepareA(hStmt, thisSQL, SQL_NTS);                 // Prepare the SQL statement	
		fsts  =  SQLExecute(hStmt);                                     // Execute the SQL statement
		if (!SQL_SUCCEEDED(fsts))	{ extract_error("SQLExecute get basic info ", hStmt, SQL_HANDLE_STMT);	exit(1); }
		for (int i = 0; i < numCols;i++){
			SQLBindCol(hStmt, i+1, SQL_C_CHAR, bindBuffer[i], bufSize, &cbModel); // bind columns
		}
	}
	void bind(int col,char *buffer) {
		SQLBindCol(hStmt, col, SQL_C_CHAR, buffer, bufSize, &cbModel); // bind columns
	}
	SQLRETURN fetch(bool checkForErrors){
		fsts = SQLFetch(hStmt);
		if (checkForErrors){
			if (fsts != SQL_SUCCESS && fsts != SQL_SUCCESS_WITH_INFO)	{ extract_error("SQLFetch", hStmt, SQL_HANDLE_STMT);	exit(1); }
		}
		return fsts;
	}
	SQLRETURN execute(bool checkForErrors){
		fsts = SQLExecute(hStmt);
		if (checkForErrors){
			if (fsts != SQL_SUCCESS && fsts != SQL_SUCCESS_WITH_INFO)	{ extract_error("SQLExecute", hStmt, SQL_HANDLE_STMT);	exit(1); }
		}
		return fsts;
	}

};

class UlTimeseries {
public:
	UlTimeseries() {};
	std::vector <double>       price;
	std::vector <bool>         nonTradingDay;
	std::vector <std::string>  date;
};

class SpPayoff {

public:
	SpPayoff(std::string date, double amount) :
		// use this for debug only...in production it uses too much memory eg 1000 iterations of a 6000point timeseries with 72(monthly) barriers
		// date(date), 
		amount(amount){};
	// std::string date;
	double amount;
};


class SpBarrierRelation {

public:
	SpBarrierRelation(const int underlying,
		double              barrier,
		double              uBarrier,
		const bool          isAbsolute,
		const std::string   startDate,
		const std::string   endDate,
		const bool          above,
		const bool          at,
		const double        weight,
		const int           daysExtant,
		double              strike,
		const UlTimeseries  &ulTimeseries,
		const int           avgType, 
		const int           avgDays,
		const int           avgFreq,
		const std::string   productStartDateString)
		: underlying(underlying), barrier(barrier), uBarrier(uBarrier), isAbsolute(isAbsolute),
		startDate(startDate), endDate(endDate), above(above), at(at), weight(weight), daysExtant(daysExtant),
		strike(strike), avgType(avgType), avgDays(avgDays), avgFreq(avgFreq)
	{
		using namespace boost::gregorian;
		const date bStartDate(from_simple_string(startDate));
		const date bEndDate(from_simple_string(endDate));
		const date bProductStartDate(from_simple_string(productStartDateString));
		avgWasHit.reserve(2000);
		runningAverage = 0.0;
		runningAvgObs  = 0;
		runningAvgDays = 0;
		uBarrierLevel  = NULL;
		startDays      = (bStartDate - bProductStartDate).days() - daysExtant;
		endDays        = (bEndDate   - bProductStartDate).days() - daysExtant;
		// post-strike initialisation
		if (daysExtant){
			// ...compute moneyness
			int lastIndx(ulTimeseries.price.size() - 1);  // range-checked now so can use vector[] to access elements
			moneyness    = ulTimeseries.price[lastIndx] / ulTimeseries.price[lastIndx - daysExtant];
			strike      /= moneyness;
			// ...compute running averages
			if (avgDays && avgDays > endDays){
				setLevels(ulTimeseries.price[lastIndx]);
				for (int i = 0; endDays + i < avgDays;i++){
					if (!ulTimeseries.nonTradingDay[lastIndx - i]  && runningAvgObs%avgFreq == 0){
						double p = ulTimeseries.price[lastIndx - i];
						switch (avgType){
						case 0: // level
							runningAverage += p;
							break;
						case 1: // proportional
							avgWasHit.push_back( above ? (p>barrierLevel && (uBarrierLevel == NULL || p<uBarrierLevel) ? true : false) : (p<barrierLevel && (uBarrierLevel == NULL || p>uBarrierLevel) ? true : false));
							break;
						}
						runningAvgObs  += 1;
					}
					runningAvgDays += 1;
				}
				runningAverage /= ulTimeseries.price[lastIndx];   // express fixings as %ofSpot; re-inflate later with prevailing Spot
				if (runningAvgObs){ runningAverage /= runningAvgObs; }
			}
		}
		else {
			moneyness      = 1.0;
		}
	};
	const bool        above, at,isAbsolute;
	const int         underlying, avgType, avgDays, avgFreq,daysExtant;
	const double      barrier, uBarrier,weight;
	const std::string startDate, endDate;
	int               startDays, endDays, runningAvgObs,runningAvgDays;
	double            barrierLevel, uBarrierLevel,strike, moneyness;
	double            runningAverage;
	std::vector<bool> avgWasHit;
	void setLevels(const double ulPrice) {
		barrierLevel  = barrier  * ulPrice / moneyness;
		if (uBarrier != NULL) {	uBarrierLevel = uBarrier * ulPrice / moneyness;	}
	}

};

class SpBarrier {
private:
	

public:
	SpBarrier(const int         barrierId,
		const bool              capitalOrIncome,
		const std::string       nature,
		double                  payoff,
		const std::string       settlementDate,
		const std::string       description,
		const std::string       payoffType,
		const int               payoffTypeId,
		double                  strike,
		double                  cap,
		const int               underlyingFunctionId,
		const double            param1,
		const double            participation,
		const std::vector<int>  ulIdNameMap,
		const int               avgDays,
		const int               avgType,
		const int	            avgFreq,
		const bool              isMemory,
		const bool              isAbsolute,
		const int               daysExtant,
		const boost::gregorian::date bProductStartDate)
		: barrierId(barrierId), capitalOrIncome(capitalOrIncome), nature(nature), payoff(payoff),
		settlementDate(settlementDate), description(description), payoffType(payoffType),
		payoffTypeId(payoffTypeId), strike(strike), cap(cap), underlyingFunctionId(underlyingFunctionId),param1(param1),
		participation(participation), ulIdNameMap(ulIdNameMap),
		isAnd(nature == "and"), avgDays(avgDays), avgType(avgType),
		avgFreq(avgFreq), isMemory(isMemory), isAbsolute(isAbsolute),daysExtant(daysExtant)
	{
		using namespace boost::gregorian;
		date bEndDate(from_simple_string(settlementDate));
		endDays = (bEndDate - bProductStartDate).days() - daysExtant;
		yearsToBarrier         = endDays / 365.25;
		sumPayoffs             = 0.0;
		isExtremum             = false;
		isContinuous           = false;
		hasBeenHit             = false;
		proportionHits         = 1.0;
		sumProportion          = 0.0;
		proportionalAveraging  = avgDays > 0 && avgType == 1;
		brel.reserve(10);
		hit.reserve(100000);
	};
	// public members: DOME consider making private, in case we implement their content some other way
	const int                       barrierId, payoffTypeId, underlyingFunctionId, avgDays, avgType, avgFreq, daysExtant;
	const bool                      capitalOrIncome, isAnd, isMemory, isAbsolute;
	const double                    payoff, participation, param1;
	const std::string               nature, settlementDate, description, payoffType;
	const std::vector<int>          ulIdNameMap;
	bool                            hasBeenHit, isExtremum, isContinuous, proportionalAveraging;
	int                             endDays;
	double                          strike, cap, yearsToBarrier, sumPayoffs, proportionHits, sumProportion;
	std::vector <SpBarrierRelation> brel;
	std::vector <SpPayoff>          hit;

	// number of days until barrier end date
	int getEndDays() const { return endDays; }
	// test if barrier is hit
	bool isHit (const std::vector<double> &thesePrices) const {  
		int j;
		bool isHit  = isAnd;
		int numBrel = brel.size();  // could be zero eg simple maturity barrier (no attached barrier relations)
		if (numBrel == 0) return true;
		std::string word;

		for (j = 0; j<numBrel; j++) {
			const SpBarrierRelation &thisBrel(brel[j]);
			int    thisIndx;       thisIndx    = ulIdNameMap[thisBrel.underlying];
			bool   above;          above       = thisBrel.above;
			double thisUlPrice;    thisUlPrice = thesePrices[thisIndx];
			double diff;           diff        = thisUlPrice - thisBrel.barrierLevel;
			bool   thisTest;       thisTest    = above ? diff>0 : diff < 0;
			//std::cout << j << "Diff:" << diff << "Price:" << thisUlPrice << "Barrier:" << thisBrel.barrierLevel << std::endl;
			if (thisBrel.uBarrier != NULL){
				diff     = thisUlPrice - thisBrel.uBarrierLevel;
				thisTest &= above ? diff<0 : diff>0;
			}
			if (isAnd)  isHit &= thisTest;
			else        isHit |= thisTest;
		}
		//std::cout << "isHit:" << isHit << "Press a key to continue..." << std::endl;  std::getline(std::cin, word);
		return isHit;
	};
	// get payoff
	double getPayoff(const std::vector<double> &startLevels,
		std::vector<double> &lookbackLevel,
		const std::vector<double> &thesePrices,
		const double amc) {
		double              thisPayoff(payoff), optionPayoff(0.0), p, thisRefLevel, thisAssetReturn,thisStrike;
		std::vector<double> optionPayoffs; optionPayoffs.reserve(10);
		int                 callOrPut = -1, j, len,n;     				// default option is a put

		switch (payoffTypeId) {
		case callPayoff:
		case lookbackCallPayoff:
		case twinWinPayoff:
			callOrPut = 1;
		case putPayoff:
			for (j = 0, len = brel.size(); j<len; j++) {
				const SpBarrierRelation &thisBrel(brel[j]);
				n      = ulIdNameMap[thisBrel.underlying];
				thisStrike = thisBrel.strike * startLevels[n] / thisBrel.moneyness;
				thisRefLevel = startLevels[n] / thisBrel.moneyness;
				if (payoffTypeId == lookbackCallPayoff) {
					thisAssetReturn = lookbackLevel[n] / thisRefLevel;
				}
				else {
					thisAssetReturn = thesePrices[n] / thisRefLevel;
				}

				p = callOrPut*(thisAssetReturn - thisStrike / thisRefLevel);
				if (payoffTypeId == twinWinPayoff) { p = fabs(p); }
				if (p > cap){ p = cap; }
				optionPayoffs.push_back(p);
			}
			switch (underlyingFunctionId) {
			case uFnLargest:
				for (optionPayoff = 0.0, j = 0, len = optionPayoffs.size(); j<len; j++) {
					if (optionPayoffs[j] > optionPayoff) { optionPayoff = optionPayoffs[j]; }
				}
				break;
			case uFnLargestN:
				double avgNpayoff(0.0);
				sort(optionPayoffs.begin(), optionPayoffs.end()); // sort ASCENDING
				for (len=optionPayoffs.size(),j = len-param1; j<len; j++) { avgNpayoff += optionPayoffs[j]; }
				optionPayoff = avgNpayoff > 0.0 ? avgNpayoff / param1 : 0.0;		
				break;
			}
			thisPayoff += participation*optionPayoff;
			break;
		case basketCallPayoff:
			double basketFinal = 0.0, basketStart = 0.0, basketRef = 0.0;
			for (j = 0, len = brel.size(); j<len; j++)
			{
				const SpBarrierRelation &thisBrel(brel[j]);
				int    n     = ulIdNameMap[thisBrel.underlying];
				double w     = thisBrel.weight;
				thisRefLevel = startLevels[n] / thisBrel.moneyness;
				basketFinal += thesePrices[n] * w;
				basketStart += startLevels[n] * w;
				basketRef   += thisRefLevel   * w;
			}
			double finalAssetReturn = basketFinal / basketStart;
			optionPayoff = basketFinal / basketRef - (strike*basketStart / basketRef);
			thisPayoff  += participation*(optionPayoff > 0.0 ? optionPayoff: 0.0);
			break;
		}

		// if there is an AnnualManagementCharge
		if(amc > 0.0) {	thisPayoff *= pow(1.0 - amc, yearsToBarrier);}

		return(thisPayoff);
	}
	void storePayoff(const std::string thisDateString, const double amount,const double proportion){
		sumPayoffs     += amount;
		sumProportion += proportion;
		hit.push_back(SpPayoff(thisDateString, amount));
	}
	// do any averaging
	void doAveraging(const std::vector<double> &startLevels, std::vector<double> &thesePrices, std::vector<double> &lookbackLevel, const std::vector<UlTimeseries> &ulPrices,
		const int thisPoint, const int thisMonPoint) {
		int k;
		if (avgDays && brel.size()) {
			switch (avgType) {
			case 0: //averageLevels
				for (int j = 0, len = brel.size(); j < len; j++) {
					const SpBarrierRelation& thisBrel = brel[j];
					int n = ulIdNameMap[thisBrel.underlying];
					double runningAverageLevel = thisBrel.runningAverage * startLevels.at(n);
					std::vector<double> avgObs;
					// create vector of observations
					for (k = 0; k < thisBrel.runningAvgObs; k++) {
						avgObs.push_back(runningAverageLevel);
					}
					for (k = 0; k < (avgDays - thisBrel.runningAvgDays) && k < thisMonPoint; k++) {
						if (k%avgFreq == 0){
							while (k < thisMonPoint && (ulPrices.at(n).nonTradingDay.at(thisMonPoint - k))){ k++; };
							avgObs.push_back(ulPrices.at(n).price.at(thisMonPoint - k));
						}
					}
					// calculate some value for these observations
					if (payoffType == "lookbackCall"){
						double anyValue(0.0);
						anyValue = *max_element(avgObs.begin(), avgObs.end());
						// DOME remove for loop if it gives the same value
						for (int k = 0, len1 = avgObs.size(); k<len1; k++) { double anyValue1 = avgObs[k]; if (anyValue1>anyValue){ anyValue = anyValue1; } }
						lookbackLevel[n] = anyValue;
					}
					else {
						double anyValue(0.0);
						for (int k = 0, len1 = avgObs.size(); k < len1; k++) {	anyValue += avgObs[k];	}
						thesePrices[n] = anyValue/ avgObs.size();
					}
				}
				break;
			case 1: // proportion
				double numHits(0.0), numPossibleHits(0.0);
				if (daysExtant){
					for (int i = 0, numObs = brel[0].avgWasHit.size(); i < numObs; i++) {
						bool wasHit = true;
						for (int j = 0, len = brel.size(); j < len; j++) {
							wasHit = wasHit && brel[j].avgWasHit[i];
						}
						numHits         += wasHit ? 1 : 0;
						numPossibleHits += 1;
					}
				}
				for (k = 0; k < (avgDays - brel[0].runningAvgDays) && k < thisMonPoint; k++) {
					if (k%avgFreq == 0){
						while (k < thisMonPoint && (ulPrices.at(0).nonTradingDay.at(thisMonPoint - k))){ k++; };
						std::vector<double> testPrices;
						for (int j = 0, len = brel.size(); j < len; j++) {
							SpBarrierRelation thisBrel = brel[j];
							int n = ulIdNameMap[thisBrel.underlying];
							testPrices.push_back(ulPrices.at(n).price.at(thisMonPoint - k));
						}
						numHits += isHit(testPrices) ? 1 : 0;
						numPossibleHits += 1;
					}
				}
				proportionHits = numHits / numPossibleHits;
				break;
			}
		}
	}
};

// structured product
class SProduct {
private:
	int productId;
	const std::vector <bool>        &allNonTradingDays;
	const std::vector <std::string> &allDates;
	const boost::gregorian::date    bProductStartDate;
	const int                       daysExtant;
	const double                    fixedCoupon, AMC, midPrice;
	const std::string               couponFrequency;
	const bool                      depositGteed, collateralised,couponPaidOut;
	

public:
	SProduct(const int                  productId,
		const UlTimeseries              &baseTimeseies,
		const boost::gregorian::date    bProductStartDate,
		const double                    fixedCoupon,
		const std::string               couponFrequency, 
		const bool                      couponPaidOut,
		const double                    AMC, 
		const bool                      depositGteed, 
		const bool                      collateralised,
		const int                       daysExtant,
		const double                    midPrice)
		: productId(productId), allDates(baseTimeseies.date), allNonTradingDays(baseTimeseies.nonTradingDay), bProductStartDate(bProductStartDate), fixedCoupon(fixedCoupon),
		couponFrequency(couponFrequency), 
		couponPaidOut(couponPaidOut), AMC(AMC), depositGteed(depositGteed), collateralised(collateralised), daysExtant(daysExtant), midPrice(midPrice) {
		
		barrier.reserve(100); // for more efficient push_back
	};

	// public members: DOME consider making private
	int                             productDays;
	std::vector <SpBarrier>         barrier;

	// evaluate product at this point in time
	void evaluate(const int totalNumDays, const int startPoint, const int lastPoint, const int numMcIterations, const int historyStep,
		std::vector<UlTimeseries>   &ulPrices, const std::vector<double> ulReturns[],
		const int numBarriers, const int numUl, const std::vector<int> ulIdNameMap, const std::vector<int> monDateIndx,
		const double recoveryRate, const std::vector<double> hazardCurve,MyDB &mydb,double &accruedCoupon,const bool doAccruals){
		int              totalNumReturns  = totalNumDays - 1;
		char             lineBuffer[50000], charBuffer[1000];
		int              i, j, k, len;
		double           couponValue;
		int              numIncomeBarriers(0);
		RETCODE          retcode;

		// init
		if (!doAccruals){
			for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
				if (!barrier[thisBarrier].capitalOrIncome) { numIncomeBarriers  += 1; }
			}
		}
		std::vector<int>   numCouponHits(numIncomeBarriers+1);

		// main MC loop
		for (int thisIteration = 0; thisIteration < numMcIterations; thisIteration++) {
			// start a product on each TRADING date
			for (int thisPoint = startPoint; thisPoint < lastPoint; thisPoint += historyStep) {
				// wind forwards to next trading date
				while (allNonTradingDays.at(thisPoint) && thisPoint < lastPoint) {
					thisPoint += 1;
				}
				if (thisPoint >= lastPoint){ continue; }

				// initialise product
				std::vector<bool>      barrierWasHit(numBarriers);
				std::string            startDateString = allDates.at(thisPoint);
				boost::gregorian::date bStartDate(boost::gregorian::from_simple_string(allDates.at(thisPoint)));
				bool                   matured = false;
				couponValue    = 0.0;
				double                 thisPayoff;
				std::vector<double>    thesePrices(numUl), startLevels(numUl), lookbackLevel(numUl);

				for (i = 0; i < numUl; i++) { startLevels[i] = ulPrices.at(i).price.at(thisPoint); }
				for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
					SpBarrier& b(barrier[thisBarrier]);
					std::vector<double>	theseExtrema; theseExtrema.reserve(10);
					for (unsigned int uI = 0; uI < b.brel.size(); uI++){
						SpBarrierRelation& thisBrel(b.brel[uI]);
						thisBrel.setLevels(startLevels[uI]);
						// cater for extremum barriers, where typically averaging does not apply to barrier hit test
						// ...so set barrierWasHit[thisBarrier] if the extremum condition is met
						int thisName = ulIdNameMap[thisBrel.underlying];
						// check to see if extremumBarriers hit
						if (b.isExtremum) {
							double thisExtremum;
							int firstPoint = thisPoint + thisBrel.startDays; if (firstPoint < 0           ){ firstPoint  = 0; }
							int lastPoint  = thisPoint + thisBrel.endDays;   if (lastPoint  > totalNumDays){ lastPoint   = totalNumDays; }
							const std::vector<double>&  thisTimeseries = ulPrices.at(thisName).price;
							if (thisBrel.above) {
								for (k = firstPoint, thisExtremum = -1.0e20; k<lastPoint; k++) {
									if (thisTimeseries[k]>thisExtremum){ thisExtremum = thisTimeseries[k]; }
								}
							}
							else {
								for (k = firstPoint, thisExtremum = 1.0e20; k < lastPoint; k++) {
									if (thisTimeseries[k] < thisExtremum){ thisExtremum = thisTimeseries[k]; }
								}
							}

							theseExtrema.push_back(thisExtremum);
						}
					}
					if (b.isExtremum) {
						barrierWasHit[thisBarrier] = b.hasBeenHit || b.isHit(theseExtrema);
						if (doAccruals){ b.hasBeenHit = barrierWasHit[thisBarrier]; }  // for post-strike deals, record if barriers have already been hit
					}
				}

				// go through each monitoring date
				for (int thisMonIndx = 0; !matured && thisMonIndx < monDateIndx.size(); thisMonIndx++){
					int thisMonDays  = monDateIndx[thisMonIndx];
					int thisMonPoint = thisPoint + thisMonDays;
					const std::string   thisDateString(allDates.at(thisMonPoint));
					for (i = 0; i < numUl; i++) {
						thesePrices[i] = ulPrices[i].price.at(thisMonPoint);
					}

					// test each barrier
					for (int thisBarrier = 0; !matured && thisBarrier<numBarriers; thisBarrier++){
						SpBarrier &b(barrier[thisBarrier]);
						// is barrier alive
						if (b.endDays == thisMonDays) {
							// averaging/lookback - will replace thesePrices with their averages
							b.doAveraging(startLevels,thesePrices, lookbackLevel, ulPrices, thisPoint, thisMonPoint);
							// is barrier hit
							if (b.hasBeenHit || barrierWasHit[thisBarrier] || b.proportionalAveraging || b.isHit(thesePrices)){
								barrierWasHit[thisBarrier] = true;
								thisPayoff = b.getPayoff(startLevels, lookbackLevel, thesePrices, AMC);
								if (b.capitalOrIncome){
									if (thisMonDays>0){
										// DOME: just because a KIP barrier is hit does not mean the put option is ITM
										// ...currently all payoffs for this barrier are measured...so we currently do not report when KIP is hit AND option is ITM
										// ...could just use this predicate around the next block: 
										// if(!(thisBarrier.payoffType === 'put' && thisBarrier.Participation<0 && optionPayoff === 0) ){
										matured = true;
										thisPayoff += couponValue + accruedCoupon;
										if (couponFrequency.size()) {  // add fixed coupon
											boost::gregorian::date bThisDate(boost::gregorian::from_simple_string(allDates.at(thisMonPoint)));
											double daysElapsed = (bThisDate - bStartDate).days() + daysExtant;
											char   freqChar = toupper(couponFrequency[1]);
											double couponEvery = couponFrequency[0] - '0';
											double daysPerEvery = freqChar == 'D' ? 1 : freqChar == 'M' ? 30 : 360;
											thisPayoff += fixedCoupon*floor(daysElapsed / daysPerEvery / couponEvery);
										}
									}
								}
								else {
									barrierWasHit[thisBarrier] = true;
									if (!couponPaidOut || b.endDays >= 0) {
										couponValue += b.proportionHits*thisPayoff;
										if (b.isMemory) {
											for (k = 0; k<thisBarrier; k++) {
												SpBarrier& bOther(barrier.at(k));
												if (!bOther.capitalOrIncome && !barrierWasHit[k]) {
													double payoffOther = bOther.payoff;
													barrierWasHit[k] = true;
													couponValue += payoffOther;
													// only store a hit if this barrier is in the future
													if (thisMonDays>0){
														bOther.storePayoff(thisDateString, payoffOther, 1.0);
													}
												}
											}
										}
									}
								}
								// only store a hit if this barrier is in the future
								if (thisMonDays>0){
									b.storePayoff(thisDateString, b.proportionHits*thisPayoff, b.proportionHits);
									//cerr << thisDateString << "\t" << thisBarrier << endl; cout << "Press a key to continue...";  getline(cin, word);
								}
							}
							else {
								// in case you want to see why not hit
								// b.isHit(thesePrices);
							}
						}
					}
				}
				// collect statistics for this product episode
				int thisNumCouponHits=0;
				for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
					if (!barrier[thisBarrier].capitalOrIncome && barrierWasHit[thisBarrier]){ thisNumCouponHits += 1;}
				}
				numCouponHits.at(thisNumCouponHits) += 1;
			}

			// create new random sample for next iteration
			if (numMcIterations>1){
				for (j = 1; j < totalNumReturns; j++){
					int thisIndx; thisIndx = (int)floor(((double)rand() / (RAND_MAX))*(totalNumReturns - 1));
					for (i = 0; i < numUl; i++) {
						double thisReturn; thisReturn = ulReturns[i][thisIndx];
						ulPrices[i].price[j] = ulPrices[i].price[j - 1] * thisReturn;
					}
				}
			}
			std::cout << ".";
		}
		std::cout << std::endl;



		// *****************
		// ** handle results
		// *****************
		if (doAccruals){                       // store accrued coupon
			accruedCoupon = couponValue;
		}
		else {
			int numAllEpisodes(0);
			bool hasProportionalAvg(false);   // no couponHistogram
			for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
				if (barrier.at(thisBarrier).capitalOrIncome) {
					numAllEpisodes += barrier.at(thisBarrier).hit.size();
				}
				hasProportionalAvg = hasProportionalAvg || barrier.at(thisBarrier).proportionalAveraging;
			}

			// couponHistogram
			if (!hasProportionalAvg && numIncomeBarriers){
				// ** delete old
				sprintf(lineBuffer, "%s%d%s%d%s",
					"delete from couponhistogram where ProductId='", productId, "' and IsBootstrapped='", numMcIterations == 1 ? 0:1, "'");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
				// ** insert new
				for (int thisNumHits=0; thisNumHits < numCouponHits.size(); thisNumHits++){
					sprintf(lineBuffer, "%s%d%s%d%s%.5lf%s%d%s",
					"insert into couponhistogram (ProductId,NumCoupons,Prob,IsBootstrapped) values (",productId,",",
					thisNumHits, ",", ((double)numCouponHits[thisNumHits]) / numAllEpisodes, ",", numMcIterations == 1 ? 0 : 1, ")");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
				}
			}


			std::string      lastSettlementDate = barrier.at(numBarriers - 1).settlementDate;
			double   actualRecoveryRate = depositGteed ? 0.9 : (collateralised ? 0.9 : recoveryRate);
			for (int analyseCase = 0; analyseCase < 2; analyseCase++) {
				bool     applyCredit = analyseCase == 1;
				double   projectedReturn = (numMcIterations == 1 ? (applyCredit ? 0.05 : 0.0) : (applyCredit ? 0.02 : 1.0));
				bool     foundEarliest = false;
				double   probEarly(0.0), probEarliest(0.0);
				std::vector<double> allPayoffs, allAnnRets;
				int    numPosPayoffs(0), numStrPosPayoffs(0), numNegPayoffs(0);
				double sumPosPayoffs(0), sumStrPosPayoffs(0), sumNegPayoffs(0);
				double sumPosDurations(0), sumStrPosDurations(0), sumNegDurations(0);

				// ** process barrier results
				double sumPayoffs(0.0), sumAnnRets(0.0), sumDuration(0.0);
				int    numCapitalInstances(0);
				for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
					const SpBarrier&    b(barrier.at(thisBarrier));
					double              thisBarrierSumPayoffs(0.0);
					std::vector<double> thisBarrierPayoffs; thisBarrierPayoffs.reserve(100000);
					int                 numInstances    = b.hit.size();
					double              sumProportion   = b.sumProportion;
					double              thisYears       = b.yearsToBarrier;
					double              prob            = sumProportion / numAllEpisodes;
					double              thisProbDefault = probDefault(hazardCurve, thisYears);
					for (i = 0; i < b.hit.size(); i++){
						double thisAmount = b.hit[i].amount;
						// possibly apply credit adjustment
						if (applyCredit) { thisAmount *= ((double)rand() / (RAND_MAX)) < thisProbDefault ? actualRecoveryRate : 1; }
						thisBarrierPayoffs.push_back(thisAmount);
						thisBarrierSumPayoffs += thisAmount;
					}

					if (b.capitalOrIncome) {
						if (!foundEarliest)                        { foundEarliest = true; probEarliest = prob; }
						if (b.settlementDate < lastSettlementDate) {                       probEarly   += prob; }
						sumDuration         += numInstances*thisYears;
						numCapitalInstances += numInstances;
						sumPayoffs          += b.sumPayoffs;
						for (i = 0; i < b.hit.size(); i++){
							double thisAmount = thisBarrierPayoffs[i];
							double thisAnnRet = exp(log(thisAmount / midPrice) / thisYears) - 1.0;
							allPayoffs.push_back(thisAmount);
							allAnnRets.push_back(thisAnnRet);
							sumAnnRets += thisAnnRet;
							if (thisAmount >  1.0) { sumStrPosPayoffs += thisAmount; numStrPosPayoffs++;    sumStrPosDurations += thisYears; }
							if (thisAmount >= 1.0) { sumPosPayoffs    += thisAmount; numPosPayoffs++;       sumPosDurations    += thisYears; }
							else                   { sumNegPayoffs    += thisAmount; numNegPayoffs++;       sumNegDurations    += thisYears; }
						}
					}
					double mean      = thisBarrierSumPayoffs / numInstances;
					double annReturn = numInstances ? (exp(log(((b.capitalOrIncome ? 0.0 : 1.0) + mean) / midPrice) / b.yearsToBarrier) - 1.0) : 0.0;
					std::cout << b.description << " Prob:" << prob << " ExpectedPayoff:" << mean << std::endl;
					// ** SQL barrierProb
					sprintf(lineBuffer, "%s%.5lf%s%.5lf%s%.5lf%s%d%s%d%s%.2lf%s", "update barrierprob set Prob='", prob,
						"',AnnReturn='", annReturn,
						"',CondPayoff='", mean,
						"',NumInstances='", numInstances,
						"' where ProductBarrierId='", barrier.at(thisBarrier).barrierId, "' and ProjectedReturn='", projectedReturn, "'");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
					//retcode = mydb.execute(true);
				}

				// ** process overall product results
				int numAnnRets(allAnnRets.size());
				const double confLevel(0.1);
				sort(allPayoffs.begin(), allPayoffs.end());
				sort(allAnnRets.begin(), allAnnRets.end());
				double averageReturn = sumAnnRets / numAnnRets;
				double vaR95         = 100.0*allPayoffs[floor(numAnnRets*0.05)];

				// pctiles
				if (numMcIterations > 1 && analyseCase == 0) {
					double minReturn = allAnnRets[0];
					std::vector<double>    returnBucket;
					std::vector<double>    bucketProb;
					for (i = j = 0; i < numAnnRets; i++) {
						if (allAnnRets[i] <= minReturn) { j += 1; }
						else { returnBucket.push_back(minReturn); bucketProb.push_back(((double)j) / numAnnRets); j = 0; minReturn += 0.01; }
					}
					returnBucket.push_back(minReturn); bucketProb.push_back(((double)j) / numAnnRets);
					sprintf(lineBuffer, "%s%d%s", "delete from pctiles where productid='", productId, "';");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
					//retcode = mydb.execute(true);
					sprintf(lineBuffer, "%s", "insert into pctiles values ");
					for (i=0; i < returnBucket.size(); i++){
						if (i != 0){ sprintf(lineBuffer, "%s%s", lineBuffer, ","); }
						sprintf(lineBuffer, "%s%s%d%s%.4lf%s%.6lf%s%d%s%d%s", lineBuffer, "(", productId, ",", 100.0*returnBucket[i], ",", bucketProb[i], ",", numMcIterations, ",", analyseCase == 0 ? 0 : 1, ")");
					}
					sprintf(lineBuffer, "%s%s", lineBuffer, ";");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
					//retcode = mydb.execute(true);
				}


				// eShortfall, esVol
				int numShortfall(floor(confLevel*allAnnRets.size()));
				double eShortfall(0.0);	for (i = 0; i < numShortfall; i++){ eShortfall += allAnnRets[i]; }	eShortfall /= numShortfall;
				double duration  = sumDuration / numAnnRets;
				double esVol     = (log(1 + averageReturn) - log(1 + eShortfall)) / ESnorm(.1);
				double scaledVol = esVol * sqrt(duration);
				double geomReturn(0.0);	for (i = 0; i < numAnnRets; i++){ geomReturn += log(allPayoffs[i] / midPrice); }
				geomReturn = exp(geomReturn / sumDuration) - 1;
				double sharpeRatio = scaledVol > 0.0 ? (geomReturn / scaledVol>1000.0 ? 1000.0 : geomReturn / scaledVol) : 1000.0;
				std::vector<double> cesrBuckets = { 0.0, 0.005, .02, .05, .1, .15, .25, .4 };
				double riskCategory(1.0);  for (i = 1, len = cesrBuckets.size(); i<len && scaledVol>cesrBuckets[i]; i++) { riskCategory += 1.0; }
				if (i != len) riskCategory += (scaledVol - cesrBuckets[i - 1]) / (cesrBuckets[i] - cesrBuckets[i - 1]);
				// WinLose
				double sumNegRet(0.0), sumPosRet(0.0), sumStrPosRet(0.0);
				int    numNegRet(0), numPosRet(0), numStrPosRet(0);
				for (j = 0; j<numAnnRets; j++) {
					double ret = allAnnRets[j];
					if (ret>0){ sumStrPosRet += ret; numStrPosRet++; }
					if (ret<0){ sumNegRet    += ret; numNegRet++; }
					else { sumPosRet += ret; numPosRet++; }
				}
				double strPosDuration(sumStrPosDurations / numStrPosPayoffs), posDuration(sumPosDurations / numPosPayoffs), negDuration(sumNegDurations / numNegPayoffs);
				double ecGain         = 100.0*(numPosPayoffs ? exp(log(sumPosPayoffs / midPrice / numPosPayoffs) / posDuration) - 1.0 : 0.0);
				double ecStrictGain   = 100.0*(numStrPosPayoffs ? exp(log(sumStrPosPayoffs / midPrice / numStrPosPayoffs) / strPosDuration) - 1.0 : 0.0);
				double ecLoss         = -100.0*(numNegPayoffs ? exp(log(sumNegPayoffs / midPrice / numNegPayoffs) / negDuration) - 1.0 : 0.0);
				double probGain       = numPosRet ? ((double)numPosRet) / numAnnRets : 0;
				double probStrictGain = numStrPosRet ? ((double)numStrPosRet) / numAnnRets : 0;
				double probLoss       = 1 - probGain;
				double eGainRet       = ecGain * probGain;
				double eLossRet       = ecLoss * probLoss;
				double winLose        = sumNegRet ? (eGainRet / eLossRet>1000.0 ? 1000.0 : eGainRet / eLossRet) : 1000.0;

				sprintf(lineBuffer, "%s%.5lf", "update cashflows set ExpectedPayoff='", sumPayoffs / numAnnRets);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ExpectedReturn='", geomReturn);
				sprintf(lineBuffer, "%s%s%s",    lineBuffer, "',FirstDataDate='",  allDates[0].c_str());
				sprintf(lineBuffer, "%s%s%s",    lineBuffer, "',LastDataDate='",   allDates[totalNumDays - 1].c_str());
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',SharpeRatio='",    sharpeRatio);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',RiskCategory='",   riskCategory);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',WinLose='", winLose);
				std::time_t rawtime;	struct std::tm * timeinfo;  time(&rawtime);	timeinfo = localtime(&rawtime);
				strftime(charBuffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
				sprintf(lineBuffer, "%s%s%s",    lineBuffer, "',WhenEvaluated='", charBuffer);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ProbEarliest='",  probEarliest);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ProbEarly='",     probEarly);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VaR='",           vaR95);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ESvol='",         esVol);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',Duration='",      duration);
				sprintf(lineBuffer, "%s%s%d",    lineBuffer, "',NumResamples='",  numMcIterations);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecGain='",        ecGain);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecStrictGain='",  ecStrictGain);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecLoss='",        ecLoss);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probGain='",      probGain);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probStrictGain='",probStrictGain);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probLoss='",      probLoss);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',eShortfall='",    eShortfall*100.0);
				sprintf(lineBuffer, "%s%s%d",    lineBuffer, "',NumEpisodes='",   numAllEpisodes);

				sprintf(lineBuffer, "%s%s%d%s%.2lf%s", lineBuffer, "' where ProductId='", productId, "' and ProjectedReturn='", projectedReturn, "'");
				std::cout << lineBuffer <<  std::endl;

				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				//retcode = mydb.execute(true);
			}
		}
	}
};