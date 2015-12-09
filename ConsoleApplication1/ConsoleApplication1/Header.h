#include <boost/lambda/lambda.hpp>
//#include <boost/regex.hpp>
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

#define MAX_SP_BUF       500000

// convert date wchar to char
// ...consult this: http://msdn.microsoft.com/en-us/library/ms235631.aspx
char *WcharToChar(const WCHAR* orig, size_t* convertedChars) {
	size_t origsize      = wcslen(orig) + 1;
	const size_t newsize = origsize * 2;
	char *thisDate       = new char[newsize];
	*convertedChars      = 0;
	wcstombs_s(convertedChars, thisDate, newsize, orig, _TRUNCATE);
	return thisDate;
}

// convert char to wchar
static wchar_t* charToWChar(const char* text)
{
	size_t size = strlen(text) + 1;
	wchar_t* wa = new wchar_t[size];
	mbstowcs(wa, text, size);
	return wa;
}

// calculate averaging days and freq
void buildAveragingInfo(const char* avgTenorText, const char* avgFreqText, int &avgDays, int &avgFreq) {
	std::map<char, int> avgTenor; avgTenor['d'] = 1; avgTenor['w'] = 7; avgTenor['m'] = 30; avgTenor['q'] = 91; avgTenor['y'] = 365;
	std::map<char, int>::iterator curr, end;

	int tenorPeriodDays = 0;
	char avgChar1 = avgTenorText[0];
	int numTenor  = (avgChar1 - '0');
	char avgChar2 = tolower(avgTenorText[1]);
	/* one way to do it
	for (found = false, curr = avgTenor.begin(), end = avgTenor.end(); !found && curr != end; curr++) {
	if (curr->first == avgChar2){ found = true; tenorPeriodDays = curr->second; }
	}*/
	if (avgTenor.find(avgChar2) != avgTenor.end()){
		tenorPeriodDays = avgTenor[avgChar2];
	}
	else { throw std::out_of_range("map_at()"); }

	avgDays = numTenor * tenorPeriodDays;  // maybe add 1 since averaging invariably includes both end dates
	int avgFreqLen = strlen(avgFreqText);
	int avgFreqStride = 1;
	if (avgFreqLen > 1){ char buf[10];  strncpy(buf, avgFreqText, avgFreqLen-1); avgFreqStride = atoi(buf); }
	avgChar2 = tolower(avgFreqText[avgFreqLen-1]);
	if (avgTenor.find(avgChar2) != avgTenor.end()){
		avgFreq = avgTenor[avgChar2] * avgFreqStride;
	}
	else { throw std::out_of_range("map_at()"); }
}

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
void splitCommaSepName(std::vector<std::string> &out, std::string s){

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


// ************** sundry functions
double calcRiskCategory(const std::vector<double> &buckets,const double scaledVol,const double start){
	double riskCategory(start);  
	int i, len;
	for (i = 1, len = buckets.size(); i<len && scaledVol>buckets[i]; i++) { riskCategory += 1.0; }
	if (i != len) riskCategory += (scaledVol - buckets[i - 1]) / (buckets[i] - buckets[i - 1]);
	return(riskCategory);
}

enum { fixedPayoff = 1, callPayoff, putPayoff, twinWinPayoff, switchablePayoff, basketCallPayoff, lookbackCallPayoff, lookbackPutPayoff, basketPutPayoff, 
	basketCallQuantoPayoff, basketPutQuantoPayoff, cappuccinoPayoff
};
enum { uFnLargest = 1, uFnLargestN, uFnSmallest };




// *************** STRUCTS
typedef struct someCurve { double tenor, spread; } SomeCurve;


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
	std::string dataSource;

public:
	// makeDbConnection();
	bool makeDbConnection(){
		if (dataSource == "spCloud"){
			fsts = dbConn(hEnv, &hDBC, L"spCloud", L"anoble", L"Ragtin_Mor14_Lucian");
		}
		else if (dataSource == "newSp"){
			fsts =  dbConn(hEnv, &hDBC, L"newSp", L"root", L"ragtinmor");
		}
		else if (dataSource == "spIPRL"){
			fsts =  dbConn(hEnv, &hDBC, L"spIPRL", L"C85693_anoble", L"Ragtin_Mor14");
		}
		else {
			std::cerr << "Unknown database " << dataSource << "\n";
			exit(101);
		}
		// if (fsts != SQL_SUCCESS && fsts != SQL_SUCCESS_WITH_INFO) { exit(1); }
		return SQL_SUCCESS || SQL_SUCCESS_WITH_INFO;
	}
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

		do	{
			ret = SQLGetDiagRec(type, handle, ++i, &state[0], &native, &text[0], (SQLSMALLINT) sizeof(text), &len);
			if (SQL_SUCCEEDED(ret)) {
				text[len] = '\0';
				state[5] = '\0';
				printf("%c%c%c%c%c:%d:%s:%d\n", state[0], state[1], state[2], state[3], state[4], native, text, len);
			}
		} while (ret == SQL_SUCCESS);
	}
	// open connection to DataSource newSp
	/*
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
	
	*/
	// open connection to DataSource
	SQLRETURN dbConn(SQLHENV hEnv, SQLHDBC* hDBC, SQLWCHAR *szDSN, SQLWCHAR *szUID, SQLWCHAR *szPasswd) {
		SQLRETURN             fsts;

		fsts = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, hDBC);  // Allocate memory for the connection handle
		if (!SQL_SUCCEEDED(fsts))	{
			extract_error("SQLAllocHandle for dbc", hEnv, SQL_HANDLE_ENV);
			exit(1);
		}
		// Connect to Data Source  
		fsts = SQLConnect(*hDBC, szDSN, SQL_NTS, szUID, SQL_NTS, szPasswd, SQL_NTS); // use SQL_NTS for length...NullTerminatedString
		// std::cerr << "Connection params " << szDSN << szUID << szPasswd << "\n";
		if (!SQL_SUCCEEDED(fsts))	{
			char thisBuffer[200]; sprintf(thisBuffer,"SQLConnect for connect >>%ls<<", szDSN);
			extract_error(thisBuffer, hDBC, SQL_HANDLE_DBC);
			exit(1);
		}
		return fsts;
	}

	MyDB(char **bindBuffer, const std::string dataSource) : dataSource(dataSource), bindBuffer(bindBuffer){
		SQLAllocEnv(&hEnv);
		if (!makeDbConnection()) {
			std::cerr << "Failed to connect to " << dataSource << "\n";
			exit(102);
		};
	};
	~MyDB(){
		SQLDisconnect(hDBC);  // Disconnect from datasource
		SQLFreeConnect(hDBC); // Free the allocated connection handle
		SQLFreeEnv(hEnv);     // Free the allocated ODBC environment handle
	}
	void prepare(SQLCHAR* thisSQL,int numCols) {
		int numAttempts = 0;
		/* DEBUG ONLY
		if (strlen((char*)thisSQL)>MAX_SP_BUF){
			std::cerr << "String len:" << strlen((char*)thisSQL) << " will overflow\n";
			exit(102);
		}
		*/
		if (hStmt != NULL) {
			SQLFreeStmt(hStmt, SQL_DROP);
		}
		do {
			fsts  =  SQLAllocStmt(hDBC, &hStmt); 	                        // Allocate memory for statement handle
			fsts  =  SQLPrepareA(hStmt, thisSQL, SQL_NTS);                  // Prepare the SQL statement	
			fsts  =  SQLExecute(hStmt);                                     // Execute the SQL statement
			if (!SQL_SUCCEEDED(fsts))	{
				extract_error("prepare() failed to SQLExecute ... trying to re-connect", hStmt, SQL_HANDLE_STMT);
				// try a new connection...in case of MySQL server restart, or failed internet connection
				SQLDisconnect(hDBC);  // Disconnect from datasource
				SQLFreeConnect(hDBC); // Free the allocated connection handle
				if (!makeDbConnection()) {
					std::cerr << "prepare() failed to re-connect to " << dataSource << "\n";
					exit(103);
				};
				std::cerr << "prepare() re-connected OK to " << dataSource << " ...continuing\n";
			}
			numAttempts += 1;
		} while (!SQL_SUCCEEDED(fsts) && numAttempts<3);
		
		if (numAttempts >= 3) {
			std::cerr << "prepare() failed too many times with " << dataSource << " ...exiting\n";
			exit(104);
		};

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
		double              _barrier,
		double              _uBarrier,
		const bool          _isAbsolute,
		const std::string   startDate,
		const std::string   endDate,
		const bool          above,
		const bool          at,
		const double        weight,
		const int           daysExtant,
		const double        unadjStrike,
		const UlTimeseries  &ulTimeseries,
		const int           avgType, 
		const int           avgDays,
		const int           avgFreq,
		const int           avgInDays,
		const int           avgInFreq,
		const std::string   avgInAlgebra,
		const std::string   productStartDateString,
		const bool          isContinuousALL)
		: underlying(underlying), originalBarrier(_barrier), originalUbarrier(_uBarrier), isAbsolute(_isAbsolute),
		startDate(startDate), endDate(endDate), above(above), at(at), weight(weight), daysExtant(daysExtant),
		originalStrike(unadjStrike), avgType(avgType), avgDays(avgDays), avgFreq(avgFreq), avgInDays(avgInDays), avgInFreq(avgInFreq), avgInAlgebra(avgInAlgebra), isContinuousALL(isContinuousALL)
	{
		/*
		const boost::gregorian::date bStartDate(boost::gregorian::from_simple_string(startDate));
		const boost::gregorian::date bEndDate(boost::gregorian::from_simple_string(endDate));
		*/
		bStartDate = boost::gregorian::from_simple_string(startDate);
		bEndDate   = boost::gregorian::from_simple_string(endDate);
		const boost::gregorian::date bProductStartDate(boost::gregorian::from_simple_string(productStartDateString));
		avgWasHit.reserve(2000);
		runningAvgDays = 0;
		readyForAvgObs = false;
		uBarrierLevel  = NULL;
		startDays      = (bStartDate - bProductStartDate).days() - daysExtant;
		endDays        = (bEndDate   - bProductStartDate).days() - daysExtant;
		barrier        = originalBarrier;
		uBarrier       = originalUbarrier;
		strike         = originalStrike;
		int lastIndx(ulTimeseries.price.size() - 1);  // range-checked now so can use vector[] to access elements
		double lastPrice(ulTimeseries.price[lastIndx]);
		
		// post-strike initialisation
		if (daysExtant){				
			// averagingIn
			if (avgInDays>0) {
					double sumSoFar  = 0.0;
					int count        = 0;
					for (int k=lastIndx - daysExtant; count < avgInDays && k <= lastIndx; count++, k++) {
						if (count%avgInFreq == 0){
							std::string  thisDate = ulTimeseries.date[k]; // debug
							while (k <= lastIndx && ulTimeseries.nonTradingDay[k]){ count++; k++; thisDate = ulTimeseries.date[k]; }
							sumSoFar               += ulTimeseries.price[k];
							numAvgInSofar += 1;
							theseAvgPrices.push_back(ulTimeseries.price[k]/lastPrice); 
						}
					}
					avgInSofar      = sumSoFar / numAvgInSofar / lastPrice;
					countAvgInSofar = count;
			} // averagingIn


			// ...compute moneyness
			moneyness    = ulTimeseries.price[lastIndx] / ulTimeseries.price[lastIndx - daysExtant];
			strike      /= moneyness;
			// ...compute running averages
			// ... we are either inside an in-progress averaging (avgDays>endDays), or the entire averaging period is in the past (endDays<0)
			if (avgDays && avgDays > endDays){
				double refSpot(ulTimeseries.price[lastIndx]);
				setLevels(refSpot);
				for (int indx = lastIndx+endDays-avgDays,stopIndx=lastIndx+(endDays<0 ? endDays:0); indx <= stopIndx;indx++){
					if (runningAvgDays%avgFreq == 0){ readyForAvgObs = true; } // first obs on starting indx
					if (!ulTimeseries.nonTradingDay[indx]  && readyForAvgObs){
						readyForAvgObs = false;
						double p = ulTimeseries.price[indx];
						switch (avgType){
						case 0: // level
							runningAverage.push_back(p/refSpot);  // express fixings as %ofSpot; re-inflate later with prevailing Spot
							break;
						case 1: // proportional
							avgWasHit.push_back( above ? (p>barrierLevel && (uBarrierLevel == NULL || p<uBarrierLevel) ? true : false) : (p<barrierLevel && (uBarrierLevel == NULL || p>uBarrierLevel) ? true : false));
							break;
						}
					}
					runningAvgDays += 1;
				}
			}
		}
		else {
			moneyness      = 1.0;
		}
	};
	const bool        above, at, isAbsolute, isContinuousALL;
	const int         underlying, avgType, avgDays, avgFreq,daysExtant;
	const double      originalStrike,originalBarrier, originalUbarrier,weight;
	const std::string startDate, endDate, avgInAlgebra;
	int               count, j, k, startDays, endDays, runningAvgDays, avgInDays, avgInFreq, numAvgInSofar=0, countAvgInSofar=0, endDaysDiff=0;
	double            avgInSofar=0.0, refLevel, barrier, uBarrier, barrierLevel, uBarrierLevel, strike, moneyness;
	bool              readyForAvgObs;
	std::vector<double> runningAverage;
	std::vector<bool>   avgWasHit;
	boost::gregorian::date bStartDate,bEndDate;
	std::vector<double>  theseAvgPrices;


	// ** handle AvgInAlgebra
	// ... parse algebra and evaluate it on data
	// ... rerurns a number
	double evalAlgebra(const std::vector<double> data, std::string algebra){
		if (data.size() == 0 || algebra.length() == 0){ return 0.0; }

		// algebra, once tokenised, is evaluated in reverse polish
		int i,len,j;
		int numValues = data.size();
		char buffer[100]; sprintf(buffer,"%s", algebra.c_str());
		char *token = std::strtok(buffer, "_");
		std::vector<std::string> tokens;
		while (token != NULL) {
			tokens.push_back(token);
			token = std::strtok(NULL, "_");
		}

		int numTokens = tokens.size();
		double result = 0.0;
		std::vector<double> stack;    for (i=0; i<numValues; i++){ stack.push_back(data[i]); }
		double extremum;

		for (i=0; i < numTokens; i++){
			if (tokens[i] == "min"){
				extremum = +INFINITY;
				for (j=0,len=stack.size(); j < len; j++){
					if (stack[j] < extremum){ extremum = stack[j]; }
				}
				stack.clear();
				stack.push_back(extremum);
			}
			else if (tokens[i] == "max"){
				extremum = -INFINITY;
				for (j=0, len=stack.size(); j < len; j++){
					if (stack[j] > extremum){ extremum = stack[j]; }
				}
				stack.clear();
				stack.push_back(extremum);
			}
			else {
				stack.push_back(atof(tokens[i].c_str()));
			}
		}
		return stack[0];
	}




	// do any averagingIn
	void doAveragingIn(const double ulPrice,   // prevailing (possibly simulated) spot level
						const int thisPoint,   // current product starting point in global timeseries
						const int lastPoint,   // last point in global timeseries
						const UlTimeseries&  ulTimeseries
		){
		if (avgInDays > 0) {
			double sumAvg  = avgInSofar*numAvgInSofar*ulPrice;
			double numAvg  = numAvgInSofar;
			std::vector<double> theseAvgInObs(theseAvgPrices); 
			if (countAvgInSofar < avgInDays){
				for (count=countAvgInSofar, k=thisPoint; count <= avgInDays && k<lastPoint; count++, k++) {
					if (count%avgInFreq == 0){
						while (k<lastPoint && ulTimeseries.nonTradingDay[k]){ count++; k++; }
						double thisPrice = ulTimeseries.price[k];
						sumAvg    += thisPrice;
						numAvg    += 1;
						theseAvgInObs.push_back(thisPrice/ulPrice);
					}
				}
			}
			double avgInMoneyness = theseAvgInObs.size() && avgInAlgebra.length() ? evalAlgebra(theseAvgInObs, avgInAlgebra) : sumAvg / numAvg / ulPrice;
			moneyness = 1.0 / avgInMoneyness; // refLevel for average STRIKE options will need to MULTIPLY by moneyness (rather than the usual case of DIVIDE)
			strike = originalStrike * avgInMoneyness;
		}
	}

	// set levels which are linked to prevailing spot level 'ulPrice'
	void setLevels(const double ulPrice) {
		refLevel      = ulPrice / moneyness;
		barrierLevel  = barrier  * ulPrice / moneyness;
		if (uBarrier != NULL) {	uBarrierLevel = uBarrier * ulPrice / moneyness;	}
	}

};

class SpBarrier {
private:
	const bool          doFinalAssetReturn;

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
		const bool              isStrikeReset,
		const bool              isStopLoss,
		const bool              isForfeitCoupons,
		const int               daysExtant,
		const boost::gregorian::date bProductStartDate,
		const bool              doFinalAssetReturn,
		const double            midPrice)
		: barrierId(barrierId), capitalOrIncome(capitalOrIncome), nature(nature), payoff(payoff),
		settlementDate(settlementDate), description(description), payoffType(payoffType),
		payoffTypeId(payoffTypeId), strike(strike),cap(cap), underlyingFunctionId(underlyingFunctionId),param1(param1),
		participation(participation), ulIdNameMap(ulIdNameMap),
		isAnd(nature == "and"), avgDays(avgDays), avgFreq(avgFreq), avgType(avgType),
		isMemory(isMemory), isAbsolute(isAbsolute), isStrikeReset(isStrikeReset), isStopLoss(isStopLoss), isForfeitCoupons(isForfeitCoupons), daysExtant(daysExtant), doFinalAssetReturn(doFinalAssetReturn), midPrice(midPrice)
	{
		using namespace boost::gregorian;
		date bEndDate(from_simple_string(settlementDate));
		endDays = (bEndDate - bProductStartDate).days() - daysExtant;
		startDays = endDays;
		yearsToBarrier         = endDays / 365.25;
		sumPayoffs             = 0.0;
		variableCoupon         = 0.0;
		isExtremum             = false;
		isContinuous           = false;
		isContinuousGroup      = false;
		hasBeenHit             = false;
		proportionHits         = 1.0;
		sumProportion          = 0.0;
		isLargestN             = underlyingFunctionId == uFnLargestN && payoffTypeId == fixedPayoff;
		if (isLargestN){
			isHit = &SpBarrier::isHitLargestN;
		}
		else if (payoffType.find("basket") != std::string::npos){
			isHit = &SpBarrier::isHitBasket;
		}
		else {
			isHit = &SpBarrier::isHitVanilla;
		}
		proportionalAveraging  = avgDays > 0 && avgType == 1;
		brel.reserve(10);
		if (doFinalAssetReturn){fars.reserve(100000); }
		hit.reserve(100000);
	};
	// public members: DOME consider making private, in case we implement their content some other way
	const int                       barrierId, payoffTypeId, underlyingFunctionId, avgDays, avgFreq, avgType, daysExtant;
	const bool                      capitalOrIncome, isAnd, isMemory, isAbsolute, isStrikeReset, isStopLoss, isForfeitCoupons;
	const double                    payoff,participation, param1,midPrice;
	const std::string               nature, settlementDate, description, payoffType;
	const std::vector<int>          ulIdNameMap;
	bool                            hasBeenHit, isExtremum, isContinuous, isContinuousGroup, proportionalAveraging, isLargestN;
	int                             startDays,endDays, numStrPosPayoffs=0, numPosPayoffs=0, numNegPayoffs=0;
	double                          variableCoupon, strike, cap, yearsToBarrier, sumPayoffs, sumStrPosPayoffs=0.0, sumPosPayoffs=0.0, sumNegPayoffs=0.0;
	double                          proportionHits, sumProportion, forwardRate;
	bool                            (SpBarrier::*isHit)(const int thisMonPoint, const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels);
	std::vector <double>            fars; // final asset returns
	std::vector <double>            bmrs; // benchmark returns
	std::vector <SpBarrierRelation> brel;
	std::vector <SpPayoff>          hit;

	// number of days until barrier end date
	int getEndDays() const { return endDays; }



	// VANILLA test if barrier is hit
	// ...set useUlMap to false if you have more thesePrices than numUnderlyings...for example product #217 has barriers with brels keyed to the same underlying
	//     so that a barrier can have multiple barrierRelations on the same underlying
	bool isHitVanilla(const int thisMonPoint,const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels) {
		int j, thisIndx;
		bool isHit  = isAnd;
		double thisRefLevel, nthLargestReturn;
		int numBrel = brel.size();  // could be zero eg simple maturity barrier (no attached barrier relations)
		if (numBrel == 0) return true;
		std::string word;

		for (j = 0; j<numBrel; j++) {
			const SpBarrierRelation &thisBrel(brel[j]);
			thisIndx    = useUlMap ? ulIdNameMap[thisBrel.underlying] : j;
			bool   above;          above       = thisBrel.above;
			double thisUlPrice;    thisUlPrice = thesePrices[thisIndx] ; 
			// if barrierRelation ends on a different date...
			if (thisBrel.endDaysDiff != 0){ thisUlPrice = ulPrices[thisIndx].price.at(thisMonPoint - thisBrel.endDaysDiff); }
			double diff;           diff        = thisUlPrice - thisBrel.barrierLevel;
			bool   thisTest;       thisTest    = above ? diff > 0 : diff < 0;
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


	// LargestN test if barrier is hit
	// ...set useUlMap to false if you have more thesePrices than numUnderlyings...for example product #217 has barriers with brels keyed to the same underlying
	//     so that a barrier can have multiple barrierRelations on the same underlying
	bool isHitLargestN(const int thisMonPoint, const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels) {
		int j, thisIndx;
		bool isHit  = isAnd;
		double thisRefLevel, nthLargestReturn;
		int numBrel = brel.size();  // could be zero eg simple maturity barrier (no attached barrier relations)
		if (numBrel == 0) return true;
		std::string word;


		/*
		* is this a LargestN barrier
		*/
		std::vector<bool>   activeBrels;
		std::vector<double> theseReturns;
		for (j=0; j < numBrel; j++){   // get returns
			const SpBarrierRelation &thisBrel(brel[j]);
			thisIndx     = useUlMap ? ulIdNameMap[thisBrel.underlying] : j;
			// thisRefLevel = startLevels[thisIndx] / thisBrel.moneyness;
			theseReturns.push_back(thesePrices[thisIndx] / thisBrel.refLevel);
		}
		sort(theseReturns.begin(), theseReturns.end(), std::greater<double>()); // sort DECENDING
		nthLargestReturn = theseReturns[param1];
		for (j=0; j<numBrel; j++){   // mark inactive barrierRelations
			const SpBarrierRelation &thisBrel(brel[j]);
			thisIndx         = useUlMap ? ulIdNameMap[thisBrel.underlying] : j;
			// thisRefLevel     = startLevels[thisIndx] / thisBrel.moneyness;
			activeBrels.push_back(thesePrices[thisIndx] / thisBrel.refLevel > nthLargestReturn);
		}
		/*
		* test active barrierRelations
		*/
		for (j = 0; j<numBrel; j++) {
			if (activeBrels[j]){
				const SpBarrierRelation &thisBrel(brel[j]);
				thisIndx    = useUlMap ? ulIdNameMap[thisBrel.underlying] : j;
				bool   above;          above       = thisBrel.above;
				double thisUlPrice;    thisUlPrice = thesePrices[thisIndx];
				double diff;           diff        = thisUlPrice - thisBrel.barrierLevel;
				bool   thisTest;       thisTest    = above ? diff > 0 : diff < 0;
				//std::cout << j << "Diff:" << diff << "Price:" << thisUlPrice << "Barrier:" << thisBrel.barrierLevel << std::endl;
				if (thisBrel.uBarrier != NULL){
					diff     = thisUlPrice - thisBrel.uBarrierLevel;
					thisTest &= above ? diff<0 : diff>0;
				}
				if (isAnd)  isHit &= thisTest;
				else        isHit |= thisTest;
			}
		}
		//std::cout << "isHit:" << isHit << "Press a key to continue..." << std::endl;  std::getline(std::cin, word);
		return isHit;
	};


	// basket test if barrier is hit
	bool isHitBasket(const int thisMonPoint, const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels) {
		int j, thisIndx;
		bool isHit  = isAnd;
		bool above;
		double w,thisRefLevel;
		int numBrel = brel.size();  // could be zero eg simple maturity barrier (no attached barrier relations)
		if (numBrel == 0) return true;
		/*
		* see if basket return breaches barrier level (in 'N' field)
		*/
		double basketReturn = 0.0;
		for (j = 0; j<numBrel; j++) {
			const SpBarrierRelation &thisBrel(brel[j]);
			thisIndx       = useUlMap ? ulIdNameMap[thisBrel.underlying] : j;
			above          = thisBrel.above;
			w              = thisBrel.weight;
			// thisRefLevel   = startLevels[thisIndx] / thisBrel.moneyness;
			basketReturn  += (thesePrices[thisIndx] / thisBrel.refLevel) * w;
		}
		double diff      = basketReturn - param1;
		bool   thisTest  = above ? diff > 0 : diff < 0;
		if (isAnd)  isHit &= thisTest;
		else        isHit |= thisTest;
		//std::cout << "isHit:" << isHit << "Press a key to continue..." << std::endl;  std::getline(std::cin, word);
		return isHit;
	};






	// VANILLA test if barrier is hit
	// ...set useUlMap to false if you have more thesePrices than numUnderlyings...for example product #217 has barriers with brels keyed to the same underlying
	//     so that a barrier can have multiple barrierRelations on the same underlying
	bool isHitOld(const int thisMonPoint, const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels) {
		int j, thisIndx;
		bool isHit  = isAnd;
		double thisRefLevel, nthLargestReturn;
		int numBrel = brel.size();  // could be zero eg simple maturity barrier (no attached barrier relations)
		if (numBrel == 0) return true;
		std::string word;


		/*
		* is this a LargestN barrier
		*
		* VERY UGLY ... DOME: tried virtual function but not as easy as I thought...do when more time
		*/
		std::vector<bool>   activeBrels;
		if (isLargestN) {
			std::vector<double> theseReturns;
			for (j=0; j < numBrel; j++){   // get returns
				const SpBarrierRelation &thisBrel(brel[j]);
				thisIndx     = useUlMap ? ulIdNameMap[thisBrel.underlying] : j;
				thisRefLevel = startLevels[thisIndx] / thisBrel.moneyness;
				theseReturns.push_back(thesePrices[thisIndx] / thisRefLevel);
			}
			sort(theseReturns.begin(), theseReturns.end(), std::greater<double>()); // sort DECENDING
			nthLargestReturn = theseReturns[param1];
			for (j=0; j<numBrel; j++){   // mark inactive barrierRelations
				const SpBarrierRelation &thisBrel(brel[j]);
				thisIndx         = useUlMap ? ulIdNameMap[thisBrel.underlying] : j;
				thisRefLevel     = startLevels[thisIndx] / thisBrel.moneyness;
				activeBrels.push_back(thesePrices[thisIndx] / thisRefLevel > nthLargestReturn);
			}
		}
		/*
		* test active barrierRelations
		*/
		for (j = 0; j<numBrel; j++) {
			if (!isLargestN || activeBrels[j]){
				const SpBarrierRelation &thisBrel(brel[j]);
				thisIndx    = useUlMap ? ulIdNameMap[thisBrel.underlying] : j;
				bool   above;          above       = thisBrel.above;
				double thisUlPrice;    thisUlPrice = thesePrices[thisIndx];
				double diff;           diff        = thisUlPrice - thisBrel.barrierLevel;
				bool   thisTest;       thisTest    = above ? diff > 0 : diff < 0;
				//std::cout << j << "Diff:" << diff << "Price:" << thisUlPrice << "Barrier:" << thisBrel.barrierLevel << std::endl;
				if (thisBrel.uBarrier != NULL){
					diff     = thisUlPrice - thisBrel.uBarrierLevel;
					thisTest &= above ? diff<0 : diff>0;
				}
				if (isAnd)  isHit &= thisTest;
				else        isHit |= thisTest;
			}
		}
		//std::cout << "isHit:" << isHit << "Press a key to continue..." << std::endl;  std::getline(std::cin, word);
		return isHit;
	};

	
	// get payoff
	double getPayoff(const std::vector<double> &startLevels,
		std::vector<double>                    &lookbackLevel,
		const std::vector<double>              &thesePrices,
		const double                            amc, 
		const std::string                       productShape,
		const bool                             doFinalAssetReturn,
		double                                 &finalAssetReturn,
		const std::vector<int>                 &ulIds,
		std::vector<bool>                      &useUl) {
		double              thisPayoff(payoff), optionPayoff(0.0), p, thisRefLevel, thisFinalLevel, thisAssetReturn, thisStrike;
		double              cumReturn,w, basketFinal, basketStart, basketRef;
		std::vector<double> basketPerfs,optionPayoffs; optionPayoffs.reserve(10);
		int                 callOrPut = -1, j, len,n;     				// default option is a put

		switch (payoffTypeId) {
		case callPayoff:
		case lookbackCallPayoff:
		case twinWinPayoff:
			callOrPut = 1;
		case lookbackPutPayoff:
		case putPayoff:
			for (j = 0, len = brel.size(); j<len; j++) {
				const SpBarrierRelation &thisBrel(brel[j]);
				n      = ulIdNameMap[thisBrel.underlying];
				// following line changed as we now correctly do SpBarrierRelation initialisation from strike(strike) to strike(unadjStrike)
				// ...previously strike /= moneyness was only affecting the parameter value! and not the member variable
				// thisStrike = thisBrel.strike * startLevels[n] / thisBrel.moneyness;
				thisStrike   = thisBrel.strike * startLevels[n] ;
				thisRefLevel = thisBrel.refLevel;  // startLevels[n] / thisBrel.moneyness;
				if (payoffTypeId == lookbackCallPayoff || payoffTypeId == lookbackPutPayoff) {
					thisAssetReturn = lookbackLevel[n] / thisRefLevel;
				}
				else {
					thisAssetReturn = thesePrices[n] / thisRefLevel;
				}
				double thisSpotReturn   = thesePrices[n] / startLevels[n];
				if (finalAssetReturn > thisSpotReturn)  { 
					finalAssetReturn = thisSpotReturn; 
				}

				// the typical optionPayoff = max(0,return) is done below in the 'for' loops initialised with 'optionPayoff=0'
				if (payoffTypeId == putPayoff && (productShape == "Autocall" || productShape == "Phoenix")){
					p = callOrPut*(thisAssetReturn*thisRefLevel / thisStrike - 1);
				}
				else {
					p = callOrPut*(thisAssetReturn - thisStrike / thisRefLevel);
				}
				
				if (payoffTypeId == twinWinPayoff) { p = fabs(p); }
				if (p > cap){ p = cap; }
				optionPayoffs.push_back(p);
			}
			switch (underlyingFunctionId) {
			case uFnSmallest:
			case uFnLargest :
				if (productShape == "Himalaya"){
					double bestUlReturn;
					int bestUlIndx=0;
					for (bestUlReturn=-INFINITY, optionPayoff=0.0, j=0, len=optionPayoffs.size(); j<len; j++) {
						if (useUl[ulIdNameMap[brel[j].underlying]]){
							if (optionPayoffs[j] > optionPayoff) { optionPayoff = optionPayoffs[j]; }
							if (optionPayoffs[j] > bestUlReturn) { bestUlIndx   = j; bestUlReturn = optionPayoffs[j]; }
						}
					}
					useUl[ulIdNameMap[brel[bestUlIndx].underlying]] = false;
				}
				else {
					if (underlyingFunctionId == uFnSmallest){
						for (optionPayoff = INFINITY, j = 0, len = optionPayoffs.size(); j<len; j++) {
							if (optionPayoffs[j] < optionPayoff) { optionPayoff = optionPayoffs[j]; }
						}
					}
					else {
						for (optionPayoff = 0.0, j = 0, len = optionPayoffs.size(); j<len; j++) {
							if (optionPayoffs[j] > optionPayoff) { optionPayoff = optionPayoffs[j]; }
						}
					}
				}
				break;
			case uFnLargestN: {
				double avgNpayoff(0.0);
				sort(optionPayoffs.begin(), optionPayoffs.end(), std::greater<double>()); // sort DECENDING
				for (optionPayoff=0, j=0, len=param1; j<len; j++){
					avgNpayoff += optionPayoffs[j] * (productShape == "Rainbow" ? param1*brel[j].weight : 1.0);
				}
				optionPayoff = avgNpayoff > 0.0 ? avgNpayoff / param1 : 0.0;
				break;
				}
			}
			thisPayoff += participation*optionPayoff;
			break;
		case basketCallPayoff:
		case basketCallQuantoPayoff:
			callOrPut = 1;
		case basketPutPayoff:
		case basketPutQuantoPayoff:
			basketFinal = 0.0;
			for (j = 0, len = brel.size(); j < len; j++)	{
				const SpBarrierRelation &thisBrel(brel[j]);
				n     = ulIdNameMap[thisBrel.underlying];
				thisRefLevel     = thisBrel.refLevel;  // startLevels[n] / thisBrel.moneyness;
				basketPerfs.push_back(thesePrices[n] / thisRefLevel);
			}
			if (productShape == "Rainbow"){
				sort(basketPerfs.begin(), basketPerfs.end(), std::greater<double>()); // sort DECENDING
			}
			for (j=0, len = brel.size(); j<len; j++) {
				const SpBarrierRelation &thisBrel(brel[j]);
				basketFinal   += basketPerfs[j] * thisBrel.weight;
			}
		   finalAssetReturn = basketFinal;
		   optionPayoff     = callOrPut *(basketFinal - strike);
		   if (optionPayoff > cap){ optionPayoff =cap; }
		   thisPayoff      += participation*(optionPayoff > 0.0 ? optionPayoff : 0.0);
		   break;
		case cappuccinoPayoff:
			callOrPut = 1;
			cumReturn = 0.0;
			w          = 1.0 / brel.size();
			basketFinal=0.0, basketStart=0.0, basketRef=0.0;
			for (j=0, len=len = brel.size(); j<len; j++) {
				const SpBarrierRelation &thisBrel(brel[j]);
				n              = ulIdNameMap[thisBrel.underlying];
				thisRefLevel   = startLevels[n] * thisBrel.strike;
				thisFinalLevel = thesePrices[n];
				basketFinal   += (thisFinalLevel / thisRefLevel) * w;
				basketStart   += (startLevels[n] / thisRefLevel) * w;
				cumReturn     += thisFinalLevel > thisRefLevel ? thisPayoff : thisFinalLevel / thisRefLevel - 1.0;
			}
			cumReturn        = cumReturn * w;
			finalAssetReturn = basketFinal / basketStart;
			optionPayoff     = callOrPut*cumReturn;
			if (optionPayoff > cap){ optionPayoff =cap; }
			thisPayoff       = participation*(optionPayoff > 0.0 ? optionPayoff : 0.0);
			proportionHits   = thisPayoff > 0.0 ? 1.0 : 0.0;
			break;			 
		case fixedPayoff:
			if (doFinalAssetReturn){
				// DOME: just record worstPerformer for now
				finalAssetReturn = 1.0e9;
				for (j = 0, len = ulIds.size(); j<len; j++)	{
					int    n     = ulIdNameMap[ulIds[j]];
					double perf  = thesePrices[n] / startLevels[n] ;
					if (perf < finalAssetReturn){ finalAssetReturn = perf; }
				}
			}
			break;
		}

		// if there is an AnnualManagementCharge
		if(amc > 0.0) {	thisPayoff *= pow(1.0 - amc, yearsToBarrier);}
		
		// finally, make sure payoffs cannot be negative...(a deal could be entered with a put struck at 500%
		// ...DOME: do dealCapture validation to prevent this sort of thing...will be complicated by absolute barriers
		// ALLOW negative payoffs, for example negative coupons    
		// if (thisPayoff<0.0){ thisPayoff = 0.0; }

		return(thisPayoff);
	}
	void storePayoff(const std::string thisDateString, const double amount, const double proportion, const double finalAssetReturn, const bool doFinalAssetReturn, const double benchmarkReturn, const bool storeBenchmarkReturn ){
		sumPayoffs     += amount;
		if (amount >  midPrice){ sumStrPosPayoffs += amount; numStrPosPayoffs++; }
		if (amount >= midPrice){ sumPosPayoffs    += amount; numPosPayoffs++; }
		else{                    sumNegPayoffs    += amount; numNegPayoffs++; }
		sumProportion  += proportion;
		hit.push_back(SpPayoff(thisDateString, amount));
		if (doFinalAssetReturn){ fars.push_back(finalAssetReturn); }
		if (storeBenchmarkReturn){ bmrs.push_back(benchmarkReturn); }
	}
	// do any averaging
	void doAveraging(const std::vector<double> &startLevels, std::vector<double> &thesePrices, std::vector<double> &lookbackLevel, const std::vector<UlTimeseries> &ulPrices,
		const int thisPoint, const int thisMonPoint, const int numUls) {
		int j,k,len,count;

		// averaging OUT
		if (avgDays && brel.size()) {
			switch (avgType) {
			case 0: //averageLevels
				for (j = 0, len = brel.size(); j < len; j++) {
					const SpBarrierRelation& thisBrel = brel[j];
					int n = ulIdNameMap[thisBrel.underlying];
					double thisStartLevel = startLevels[n];
					std::vector<double> avgObs;
					// create vector of observations
					for (k = 0; k < thisBrel.runningAverage.size(); k++) {
						avgObs.push_back(thisBrel.runningAverage[k] * thisStartLevel);
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
					else if (payoffType == "lookbackPut"){
						double anyValue(INFINITY);
						anyValue = *min_element(avgObs.begin(), avgObs.end());
						// DOME remove for loop if it gives the same value
						for (int k = 0, len1 = avgObs.size(); k<len1; k++) { double anyValue1 = avgObs[k]; if (anyValue1<anyValue){ anyValue = anyValue1; } }
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
						for (j = 0, len = brel.size(); j < len; j++) {
							wasHit = wasHit && brel[j].avgWasHit[i];
						}
						numHits         += wasHit ? 1 : 0;
						numPossibleHits += 1;
					}
				}
				for (k = 0; k < (avgDays - brel[0].runningAvgDays) && k < thisMonPoint; k++) {
					if (k%avgFreq == 0){
						while (k < thisMonPoint && (ulPrices.at(0).nonTradingDay.at(thisMonPoint - k))){ k++; };
						std::vector<double> testPrices(numUls);
						for (j = 0, len = brel.size(); j < len; j++) {
							SpBarrierRelation thisBrel = brel[j];
							int n = ulIdNameMap[thisBrel.underlying];
							testPrices.at(n) = ulPrices.at(n).price.at(thisMonPoint - k);
						}
						numHits +=  (this ->* (this->isHit))(thisMonPoint, ulPrices, testPrices, true, startLevels) ? 1 : 0;
						numPossibleHits += 1;
					}
				}
				proportionHits = numHits / numPossibleHits;
				break;
			}
		}
	}
};




// math functions
double PayoffMean(const std::vector<SpBarrier> &barrier){
	int                 numInstances(0);
	double              sumPayoffs(0.0);
	for (int thisBarrier = 0; thisBarrier < barrier.size(); thisBarrier++){
		const SpBarrier&    b(barrier.at(thisBarrier));
		if (b.capitalOrIncome) {
			numInstances    += b.hit.size();
			for (int i = 0; i < b.hit.size(); i++){
				sumPayoffs += b.hit[i].amount;
			}
		}
	}
	return (sumPayoffs / numInstances);
}

double PayoffStdev(const std::vector<SpBarrier> &barrier, const double mean){
	int                 numInstances(0);
	double              sumVariance(0.0);
	for (int thisBarrier = 0; thisBarrier < barrier.size(); thisBarrier++){
		const SpBarrier&    b(barrier.at(thisBarrier));
		if (b.capitalOrIncome) {
			numInstances    += b.hit.size();
			for (int i = 0; i < b.hit.size(); i++){
				double thisAmount = (b.hit[i].amount - mean);
				sumVariance += thisAmount*thisAmount;
			}
		}
	}
	return sqrt(sumVariance / numInstances);
}






// structured product
class SProduct {
private:
	int productId;
	const std::vector <bool>        &allNonTradingDays;
	const std::vector <int>         &ulIds;
	const std::vector <std::string> &allDates;
	const boost::gregorian::date    bProductStartDate;
	const int                       daysExtant;
	const double                    fixedCoupon, AMC, midPrice;
	const std::string               couponFrequency, productShape;
	const bool                      depositGteed, collateralised,couponPaidOut;
	const std::vector<SomeCurve>    baseCurve;

public:
	SProduct(const int                  productId,
		const UlTimeseries              &baseTimeseies,
		const boost::gregorian::date    bProductStartDate,
		const double                    fixedCoupon,
		const std::string               couponFrequency, 
		const bool                      couponPaidOut,
		const double                    AMC, 
		const std::string				productShape,
		const bool                      depositGteed, 
		const bool                      collateralised,
		const int                       daysExtant,
		const double                    midPrice,
		const std::vector<SomeCurve>    baseCurve,
		const std::vector<int>          &ulIds)
		: productId(productId), allDates(baseTimeseies.date), allNonTradingDays(baseTimeseies.nonTradingDay), bProductStartDate(bProductStartDate), fixedCoupon(fixedCoupon),
		couponFrequency(couponFrequency), 
		couponPaidOut(couponPaidOut), AMC(AMC), productShape(productShape),depositGteed(depositGteed), collateralised(collateralised), 
		daysExtant(daysExtant),	midPrice(midPrice),baseCurve(baseCurve),ulIds(ulIds) {
	};

	// public members: DOME consider making private
	int                             maxProductDays,productDays, numUls;
	std::vector <SpBarrier>         barrier;
	std::vector <bool>              useUl;
	std::vector <double>            baseCurveTenor, baseCurveSpread;
	std::vector<boost::gregorian::date> allBdates;

	// init
	void init(){
		// ...prebuild all dates outside loop
		int numAllDates = allDates.size();	allBdates.reserve(numAllDates);
		for (int thisPoint = 0; thisPoint < numAllDates; thisPoint += 1) { allBdates.push_back(boost::gregorian::from_simple_string(allDates.at(thisPoint))); }

		for (int i=0; i < baseCurve.size(); i++){ baseCurveTenor.push_back(baseCurve[i].tenor); baseCurveSpread.push_back(baseCurve[i].spread); }
		numUls = ulIds.size();
		for (int i=0; i < numUls; i++){ useUl.push_back(true); }
		barrier.reserve(100); // for more efficient push_back
	}


	// evaluate product at this point in time
	void evaluate(const int       totalNumDays, 
		const int                 startPoint, 
		const int                 lastPoint, 
		const int                 numMcIterations, 
		const int                 historyStep,
		std::vector<UlTimeseries> &ulPrices, 
		const std::vector<double> ulReturns[],
		const int                 numBarriers, 
		const int                 numUl, 
		const std::vector<int>    ulIdNameMap, 
		const std::vector<int>    monDateIndx,
		const double              recoveryRate, 
		const std::vector<double> hazardCurve,
		MyDB                      &mydb,
		double                    &accruedCoupon,
		const bool                doAccruals,
		const bool                doFinalAssetReturn,
		const bool                doDebug,
		const time_t              startTime,
		const int                 benchmarkId,
		const double              contBenchmarkTER,
		const double              hurdleReturn,
		const bool                doTimepoints, 
		const bool                doPaths,
		const std::vector<int>    timepointDays,
		const std::vector<std::string> timepointNames,
		const std::vector<double> simPercentiles,
		const bool doPriips){
		int                      totalNumReturns  = totalNumDays - 1;
		int                      numTimepoints    = timepointDays.size();
		char                     lineBuffer[MAX_SP_BUF], charBuffer[1000];
		int                      i, j, k, len, thisIteration,n;
		double                   couponValue, stdevRatio(1.0), stdevRatioPctChange(100.0);
		std::vector<double>      stdevRatioPctChanges;
		std::vector< std::vector<double> > someTimepoints[100];
		std::vector<double>           someLevels[100], somePaths[100]; 
		std::vector< std::vector<  std::vector<double> > > timepointLevels;
		std::vector< std::vector<double> >  timepointPaths;
		for (i=0; i < numTimepoints; i++) {
			timepointLevels.push_back(someTimepoints[i]);
			for (j=0; j < numUls; j++) {
				timepointLevels[i].push_back(someLevels[j]);
			}
		}
		for (i=0; i < numUls; i++) {
			timepointPaths.push_back(somePaths[i]);
		}
		int                      numIncomeBarriers(0);
		RETCODE                  retcode;

		// init
		if (!doAccruals){
			for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
				if (!barrier.at(thisBarrier).capitalOrIncome) { numIncomeBarriers  += 1; }
			}
		}
		std::vector<int>   numCouponHits(numIncomeBarriers+1);
		for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
			SpBarrier& b(barrier.at(thisBarrier));
			b.forwardRate            = 1.0 + interpCurve(baseCurveTenor, baseCurveSpread, b.yearsToBarrier); // DOME: very crude for now
		}

		// we do not need to pre-build the samples (uses too much memory, and we may not need all the samples anyway if there is convergence)
		// ... we imagine a virtual balancedResampling scheme
		// ... with N samples of length p we have the Np sequence 0,1,...,p,0,1,...p   
		// ... we have an integer "nPpos" indexing this Np vector and every time we want a random sample we choose index=rand()*(nPpos--) which is notionally in the index/p "block"
		// ... but since all the blocks are the same, we just use element index % p from the single block 0,1,...,p
		// ... ta-daa!!
		const unsigned int longNumOfSequences(1000);
		unsigned long int maxNpPos = longNumOfSequences*totalNumReturns;  // if maxNpPos is TOO large then sampling from its deceasing value is tantamount to no balancedResampling as the sample space essentially never shrinks materially
		unsigned long int npPos    = maxNpPos;
		// faster to put repeated indices in a vector, compared to modulo arithmetic, and we only need manageable arrays eg 100y of daily data is 36500 points, repeated 1000 - 36,500,000 which is well within the MAX_SIZE
		std::vector<unsigned int> returnsSeq; returnsSeq.reserve(maxNpPos); for (i=0; i < longNumOfSequences; i++){ for (j=0; j<totalNumReturns; j++) { returnsSeq.push_back(j); } }
		std::vector<std::vector <int>> resampledIndexs;
		if (0){  // the OLD memory-hog
			// prebuild all nonparametric bootstrap samples (each of sixe productDays) in resampledIndexes
			if (numMcIterations>1) {
				// balacedResampling (and antithetic): ensures each real observation is sampled roughly equally; more effective than antithetic
				// ... build concatenatedSample[] to contain a repeated sequence of integers, ranging from 0 to totalNumreturns-1, which will be used as indexes to select a random return 
				int concatenatedBlocks = (int)floor(numMcIterations / 2.0) + 1;
				std::vector<int> concatenatedSample; concatenatedSample.reserve((maxProductDays - 1)*concatenatedBlocks);
				std::vector <int>::size_type maxVec= concatenatedSample.max_size();
				// std::cout << "The maximum possible length of the vector is " << maxVec << "." << std::endl;
				for (i=0; i<(maxProductDays - 1)*concatenatedBlocks;) {
					for (j=0; j < totalNumReturns; j++) {
						concatenatedSample.push_back(j);
						i += 1;
					}
				}
				int concatenatedLength = concatenatedSample.size();
				// permutations will now give balanced sample
				for (int bootSample=0; bootSample<numMcIterations; bootSample++) {
					if (bootSample % 2 == 0) {
						std::vector<int> oneBootstrapSample; oneBootstrapSample.reserve(maxProductDays);
						for (j=0; j<maxProductDays; j++, concatenatedLength--) {
							int thisIndx; thisIndx = (int)floor(((double)rand() / (RAND_MAX))*(concatenatedLength - 1));
							oneBootstrapSample.push_back(concatenatedSample[thisIndx]);
						}
						resampledIndexs.push_back(oneBootstrapSample);
					}
				}
			}
		}
		
		

		// see if any brels have endDays shorter than barrier endDays
		for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
			SpBarrier& b(barrier.at(thisBarrier));
			int numBrel = b.brel.size();
			std::vector<double>	theseExtrema; theseExtrema.reserve(10);
			for (unsigned int uI = 0; uI < numBrel; uI++){
				SpBarrierRelation& thisBrel(b.brel.at(uI));
				thisBrel.endDaysDiff  = b.endDays - thisBrel.endDays;
			}
		}




		// main MC loop
		for (thisIteration = 0; thisIteration < numMcIterations && fabs(stdevRatioPctChange)>0.1; thisIteration++) {
			

			// create new random sample for next iteration
			if (numMcIterations > 1){
				bool useNewerMethod(true);
				bool useNewMethod(false);
				unsigned long int _notionalIx;
				int thisReturnIndex;
				if (useNewerMethod){
					// just uses balanced sampling - we can't do true antithetic sampling
					for (j = startPoint+1; j <= startPoint+productDays; j++){
						_notionalIx = (unsigned long int)floor(((double)rand() / (RAND_MAX))*(npPos - 1));
						// SLOW: thisReturnIndex = _notionalIx % totalNumReturns;
						thisReturnIndex = returnsSeq[_notionalIx];
						for (i = 0; i < numUl; i++) {
							double thisReturn; thisReturn = ulReturns[i][thisReturnIndex];
							ulPrices[i].price[j] = ulPrices[i].price[j - 1] * thisReturn;
						}
						// wind back one unit
						npPos = npPos>1 ? npPos - 1 : maxNpPos;
					}
				}
				else {
					if (useNewMethod){   // NEW adds representative to antithetic sampling
						int thisAntithetic = (int)floor(thisIteration / 2.0);
						std::vector<int> &thisTrace(resampledIndexs[thisAntithetic]);
						bool useAntithetic = thisIteration % 2 == 0;
						for (j = startPoint+1; j <= maxProductDays; j++){
							int thisIndx = useAntithetic ? totalNumReturns - thisTrace[j - 1] - 1 : thisTrace[j - 1];
							for (i = 0; i < numUl; i++) {
								double thisReturn; thisReturn = ulReturns[i][thisIndx];
								ulPrices[i].price[j] = ulPrices[i].price[j - 1] * thisReturn;
							}
						}
					}
					else{   // OLD method KEEP
						for (j = 1; j < totalNumReturns; j++){
							int thisIndx; thisIndx = (int)floor(((double)rand() / (RAND_MAX))*(totalNumReturns - 1));
							for (i = 0; i < numUl; i++) {
								double thisReturn; thisReturn = ulReturns[i][thisIndx];
								ulPrices[i].price[j] = ulPrices[i].price[j - 1] * thisReturn;
							}
						}
					}
				}
			}
			

			// wind 'thisPoint' forwards to next TRADING date, so as to start a new product
			for (int thisPoint = startPoint; thisPoint < lastPoint; thisPoint += historyStep) {
				if (numMcIterations == 1){  // only need to start on a trading day for HistoricBacktest
					while (allNonTradingDays.at(thisPoint) && thisPoint < lastPoint) {
						thisPoint += 1;
					}
					if (thisPoint >= lastPoint){ continue; }
				}
				
				// possibly track timepoints ulIds
				if (doTimepoints){
					for (i=0; i < numTimepoints; i++){
						int thisTpDays = timepointDays[i];
						for (j=0; j < numUls; j++){
							double thisLevel = ulPrices[j].price[thisTpDays + thisPoint] / ulPrices[j].price[thisPoint];
							timepointLevels[i][j].push_back(thisLevel);
						}
					}

					// save path
					if (doPaths){
						for (i=0; i < numTimepoints; i++){
							int thisTpDays = timepointDays[i];
							bool firstTime = true;
							std::string name = timepointNames[i];
							sprintf(lineBuffer, "%s", "insert into path (PathId,UserId,ProductId,UnderlyingId,TimePointDays,Name,SimValue) values ");
							for (j=0; j < numUls; j++){
								int ulId = ulIds[j];
								if (firstTime){ firstTime = false; }
								else { strcat(lineBuffer, ","); }
								double thisLevel = ulPrices[j].price[thisTpDays + thisPoint] / ulPrices[j].price[thisPoint];
								sprintf(lineBuffer, "%s%s%d%s%d%s%d%s%d%s%s%s%lf%s", lineBuffer, "(", thisPoint, ",3,", productId, ",", ulId, ",", thisTpDays, ",'", name.c_str(), "',", thisLevel, ")");
							}
							mydb.prepare((SQLCHAR *)lineBuffer, 1);
						}
					}
				}


				// initialise product eg record ulPrices at 'thisPoint'
				std::vector<bool>      barrierWasHit(numBarriers);
				std::string            startDateString = allDates.at(thisPoint);
				/*
				boost::gregorian::date bStartDate(boost::gregorian::from_simple_string(allDates.at(thisPoint)));
				if ((bStartDate - allBdates.at(thisPoint)).days() != 0.0){
					int jj=1;
				}
				*/
				boost::gregorian::date &bStartDate(allBdates.at(thisPoint));

				for (i=0; i < numUls; i++){ useUl[i] = true; }
				bool                   matured = false;
				couponValue                    = 0.0;
				double                 thisPayoff;
				double                 finalAssetReturn  = 1.0e9;
				double                 benchmarkReturn   = 1.0e9;
				std::vector<double>    thesePrices(numUl), startLevels(numUl), lookbackLevel(numUl), overrideThesePrices(numUl);

				/*
				* BEWARE ... startLevels[] has same underlyings-order as ulPrices[], so make sure any comparison with them is in the same order
				*        ... and watch out for useUlMap argument which tells callee function that array arguments are already in the correct synchronised order
				*/
				for (i = 0; i < numUl; i++) { startLevels.at(i) = ulPrices.at(i).price.at(thisPoint); }
				for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
					SpBarrier& b(barrier.at(thisBarrier));
					int numBrel = b.brel.size();
					std::vector<double>	theseExtrema; theseExtrema.reserve(10);
					for (unsigned int uI = 0; uI < numBrel; uI++){
						SpBarrierRelation& thisBrel(b.brel.at(uI));
						int thisName = ulIdNameMap.at(thisBrel.underlying);
						thisBrel.doAveragingIn(startLevels.at(thisName), thisPoint, lastPoint,ulPrices.at(thisName));
						if (b.isStrikeReset){
							int brelStartPoint = thisPoint + thisBrel.startDays; if (brelStartPoint < 0){ brelStartPoint  = 0; } if (brelStartPoint >= totalNumDays){ brelStartPoint  = totalNumDays-1; }
							thisBrel.setLevels(ulPrices.at(thisName).price.at(brelStartPoint));
						}
						else {
							thisBrel.setLevels(startLevels.at(thisName));
						}
						// cater for extremum barriers, where typically averaging does not apply to barrier hit test
						// ...so set barrierWasHit[thisBarrier] if the extremum condition is met
						// check to see if extremumBarriers hit
						if (b.isExtremum) {
							double thisExtremum;
							int firstPoint = thisPoint + thisBrel.startDays; if (firstPoint < 0           ){ firstPoint  = 0; }
							int lastPoint  = thisPoint + thisBrel.endDays;   // if (lastPoint  > totalNumDays-1){ lastPoint   = totalNumDays-1; }
							const std::vector<double>&  thisTimeseries = ulPrices.at(thisName).price;
							// 'Continuous'    means ONE-touch  so Above needs MAX  and !Above needs MIN
							// 'ContinuousALL' means  NO-touch  so Above needs MIN  and !Above needs MAX
							// ... so we need MAX in these cases
							if ((thisBrel.above && !thisBrel.isContinuousALL) || (!thisBrel.above && thisBrel.isContinuousALL)) {
								for (k = firstPoint, thisExtremum = -1.0e20; k <= lastPoint; k++) {
									if (thisTimeseries[k]>thisExtremum){ thisExtremum = thisTimeseries[k]; }
								}
							}
							else {
								for (k = firstPoint, thisExtremum = 1.0e20; k <= lastPoint; k++) {
									if (thisTimeseries[k] < thisExtremum){ thisExtremum = thisTimeseries[k]; }
								}
							}

							theseExtrema.push_back(thisExtremum);
						}
					}
					if (b.isExtremum && (!doAccruals || b.endDays<0)) {
						if (b.isContinuousGroup && numBrel>1 ) {  // eg product 536, all underlyings must be above their barriers on some common date
							int firstPoint = thisPoint + b.brel[0].startDays; if (firstPoint < 0){ firstPoint  = 0; }
							int lastPoint  = thisPoint + b.brel[0].endDays;   if (lastPoint  > totalNumDays - 1){ lastPoint   = totalNumDays - 1; }
							std::vector<int> ulNames;
							for (unsigned int uI = 0; uI < numBrel; uI++){
								ulNames.push_back(ulIdNameMap.at(b.brel.at(uI).underlying));
							}
							for (k=firstPoint; !barrierWasHit.at(thisBarrier) && k <= lastPoint; k++) {
								std::vector<double>    tempPrices;
								for (j=0; j<numBrel; j++) {
									tempPrices.push_back(ulPrices.at(ulNames[j]).price[k]);
								}
								barrierWasHit.at(thisBarrier) = b.hasBeenHit || (b .* (b.isHit))(k,ulPrices, tempPrices, false, startLevels);
								if (barrierWasHit.at(thisBarrier)){
									int jj=1;
								}
							}					
						}
						else {
							barrierWasHit.at(thisBarrier) = b.hasBeenHit || (b .* (b.isHit))(k,ulPrices, theseExtrema, false, startLevels);
						}
						if (doAccruals && b.yearsToBarrier<=0.0){     // for post-strike deals, record if barriers have already been hit
							b.hasBeenHit = barrierWasHit[thisBarrier]; 
						}
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
						if (b.endDays == thisMonDays || (b.isContinuous && thisMonDays <= b.endDays && thisMonDays >= b.startDays)) {
							// averaging/lookback - will replace thesePrices with their averages
							b.doAveraging(startLevels,thesePrices, lookbackLevel, ulPrices, thisPoint, thisMonPoint,numUls);
							// is barrier hit
							if (b.hasBeenHit || barrierWasHit[thisBarrier] || b.proportionalAveraging || (!b.isExtremum && (b .* (b.isHit))(thisMonPoint,ulPrices, thesePrices, true, startLevels))){
								barrierWasHit[thisBarrier] = true;
								if (doAccruals){         // for post-strike deals, record if barriers have already been hit
									b.hasBeenHit = true; 
								}  
								thisPayoff = b.getPayoff(startLevels, lookbackLevel, thesePrices, AMC, productShape, doFinalAssetReturn, finalAssetReturn,ulIds,useUl);
								// ***********
								// capitalBarrier hit ... so product terminates
								// ***********
								if (b.capitalOrIncome){
									if (thisMonDays > 0){
										// DOME: just because a KIP barrier is hit does not mean the put option is ITM
										// ...currently all payoffs for this barrier are measured...so we currently do not report when KIP is hit AND option is ITM
										// ...could just use this predicate around the next block: 
										// if(!(thisBarrier.payoffType === 'put' && thisBarrier.Participation<0 && optionPayoff === 0) ){
										// ** maybe record benchmark performance
										if (benchmarkId){
											n      = ulIdNameMap[benchmarkId];
											benchmarkReturn = thesePrices[n] / startLevels[n];
										}
										// END 
										matured = true;
										// add forwardValue of paidOutCoupons
										if (couponPaidOut) {
											for (int paidOutBarrier = 0; paidOutBarrier < thisBarrier; paidOutBarrier++){
												if (!barrier[paidOutBarrier].capitalOrIncome && barrierWasHit[paidOutBarrier] &&
													(barrier[paidOutBarrier].yearsToBarrier >= 0.0 || (barrier[paidOutBarrier].isMemory && !barrier[paidOutBarrier].hasBeenHit))){
													SpBarrier &ib(barrier[paidOutBarrier]);
													couponValue   += ((ib.payoffTypeId == fixedPayoff ? 1.0 : 0.0)*ib.proportionHits*ib.payoff + ib.variableCoupon)*pow(b.forwardRate, b.yearsToBarrier - ib.yearsToBarrier);
												}
											}
										}
										// add accumulated couponValue, unless b.forfeitCoupons is set
										if (!b.isForfeitCoupons){ thisPayoff += couponValue + accruedCoupon; }
										if (couponFrequency.size()) {  // add fixed coupon
											/*
											boost::gregorian::date bThisDate(boost::gregorian::from_simple_string(allDates.at(thisMonPoint)));
											if ((bThisDate - allBdates.at(thisMonPoint)).days() != 0.0){
											int jj=1;
											}
											*/
											boost::gregorian::date &bThisDate(allBdates.at(thisMonPoint));

											char   freqChar     = toupper(couponFrequency[1]);
											double couponEvery  = couponFrequency[0] - '0';
											double daysPerEvery = freqChar == 'D' ? 1 : freqChar == 'M' ? 30 : 365.25;
											double daysElapsed  = (bThisDate - bStartDate).days() + daysExtant;
											double couponPeriod = daysPerEvery*couponEvery;
											if (couponPaidOut){
												daysElapsed  -= floor(daysExtant / couponPeriod)*couponPeriod; // floor() so as to include accrued stub
											}
											double numFixedCoupons = /*floor*/(daysElapsed / couponPeriod); // allow fractional coupons
											double periodicRate    = exp(log(b.forwardRate) * (couponPeriod / 365.25));
											double effectiveNumCoupons = (pow(periodicRate, numFixedCoupons) - 1) / (periodicRate - 1);
											thisPayoff += fixedCoupon*(couponPaidOut ? effectiveNumCoupons : numFixedCoupons);
										}
									}
								}
								else {
									if (thisPayoff == 0.0){ // Rainbow option coupons for example, where a 'hit' is only known after the option payoff is calculated 
										barrierWasHit[thisBarrier] = false;
									}
									else {
										barrierWasHit[thisBarrier] = true;
									}
									
									if (!couponPaidOut || b.endDays >= 0) {
										if (!couponPaidOut){
											couponValue += b.proportionHits*thisPayoff;
										}
										else if (b.payoffTypeId != fixedPayoff){  // paid-out variable coupon
											b.variableCoupon = b.proportionHits*thisPayoff;
										}
										if (b.isMemory) {
											for (k = 0; k<thisBarrier; k++) {
												SpBarrier& bOther(barrier.at(k));
												if (!bOther.capitalOrIncome && !bOther.hasBeenHit  && !barrierWasHit[k]) {
													double payoffOther = bOther.payoff;
													barrierWasHit[k] = true;
													if (!couponPaidOut){
														couponValue += payoffOther;
													}
													// only store a hit if this barrier is in the future
													//if (thisMonDays>0){
														bOther.storePayoff(thisDateString, payoffOther, 1.0, finalAssetReturn,doFinalAssetReturn,0,false);
													//}
												}
											}
										}
									}
								}
								// only store a hit if this barrier is in the future
								//if (thisMonDays>0){
									b.storePayoff(thisDateString, b.proportionHits*thisPayoff, barrierWasHit[thisBarrier] ? b.proportionHits:0.0, 
										finalAssetReturn, doFinalAssetReturn, benchmarkReturn, benchmarkId>0 && matured);
									//cerr << thisDateString << "\t" << thisBarrier << endl; cout << "Press a key to continue...";  getline(cin, word);
								//}
							}
							else {
								// in case you want to see why not hit
								// b.isHit(thisMonPoint,ulPrices,thesePrices,true,startLevels);
							}
						}
					}
				}
				// collect statistics for this product episode
				if (!doAccruals){
					// count NUMBER of couponHits, and update running count for that NUMBER  
					int thisNumCouponHits=0;
					for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
						SpBarrier &b(barrier[thisBarrier]);
						if (!b.capitalOrIncome && (b.hasBeenHit || barrierWasHit[thisBarrier])){ thisNumCouponHits += 1; }
					}
					numCouponHits.at(thisNumCouponHits) += 1;
				}
			}

			if ((thisIteration + 1) % 1000 == 0){ std::cout << "."; }
			if (thisIteration>750 && (thisIteration + 1) % 10000 == 0){
				double thisMean       = PayoffMean(barrier);
				double thisStdevRatio = PayoffStdev(barrier, thisMean) / thisMean;
				double thisChange     = floor(10000.0*(thisStdevRatio - stdevRatio) / stdevRatio) / 100.0;
				const int numSigChanges(3);
				stdevRatioPctChanges.push_back(fabs(thisChange));
				int numStdevRatioPctChanges = stdevRatioPctChanges.size();
				if (numStdevRatioPctChanges > numSigChanges){
					double sumChanges(0.0);
					for (int j=numStdevRatioPctChanges - 1; j >= numStdevRatioPctChanges - numSigChanges; j--) {
						sumChanges += stdevRatioPctChanges[j];
					}
					stdevRatioPctChange = sumChanges / numSigChanges;
				} 
				std::cout << std::endl << " MeanPayoff:" << thisMean << " StdevRatio:" << thisStdevRatio << " StdevRatioChange:" << stdevRatioPctChange;
				stdevRatio = thisStdevRatio;
			}
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
			bool hasProportionalAvg(false);   // no couponHistogram, which only shows coupon-counts
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

			// doFinalAssetReturn
			char farBuffer[100000];
			int  farCounter(0),totalFarCounter(0);
			if (doFinalAssetReturn){
				strcpy(farBuffer, "insert into finalassetreturns values ");
				sprintf(lineBuffer, "%s%d%s", "delete from finalassetreturns where productid='", productId, "'");
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
			}
			

			// process resultsfor
			const double tol = 1.0e-6;
			const double unwindPayoff = 0.1;
			std::string      lastSettlementDate = barrier.at(numBarriers - 1).settlementDate;
			double   actualRecoveryRate = depositGteed ? 0.9 : (collateralised ? 0.9 : recoveryRate);
			double maxYears(0.0); for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){ double ytb=barrier.at(thisBarrier).yearsToBarrier; if (ytb>maxYears){ maxYears = ytb; } }
			for (int analyseCase = 0; analyseCase < (doPriips?1:2); analyseCase++) {
				if (doDebug){ std::cerr << "Starting analyseResults  for case \n" << analyseCase << std::endl; }
				bool     applyCredit = analyseCase == 1;
				double   projectedReturn = (numMcIterations == 1 ? (applyCredit ? 0.05 : 0.0) : (doPriips ? 0.08 : (applyCredit ? 0.02 : 1.0)));
				bool     foundEarliest = false;
				double   probEarly(0.0), probEarliest(0.0);
				std::vector<double> allPayoffs, allFVpayoffs,allAnnRets,bmAnnRets,bmRelLogRets;
				int    numPosPayoffs(0), numStrPosPayoffs(0), numNegPayoffs(0);
				double sumPosPayoffs(0), sumStrPosPayoffs(0), sumNegPayoffs(0);
				double sumPosDurations(0), sumStrPosDurations(0), sumNegDurations(0), sumYearsToBarrier(0);

				// ** process barrier results
				double eStrPosPayoff(0.0), ePosPayoff(0.0), eNegPayoff(0.0), sumPayoffs(0.0), sumAnnRets(0.0), sumParAnnRets(0.0), sumDuration(0.0), sumPossiblyCreditAdjPayoffs(0.0);
				int    numCapitalInstances(0), numStrPosInstances(0), numPosInstances(0), numNegInstances(0), numParInstances(0);
				for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
					if (doDebug){ std::cerr << "Starting analyseResults  for barrier \n" << thisBarrier << std::endl; }
					const SpBarrier&    b(barrier.at(thisBarrier));
					double              thisBarrierSumPayoffs(0.0), thisAmount;
					std::vector<double> thisBarrierPayoffs; thisBarrierPayoffs.reserve(100000);
					int                 numInstances    = b.hit.size();
					double              sumProportion   = b.sumProportion;
					double              thisYears       = b.yearsToBarrier;
					double              prob            = sumProportion / numAllEpisodes; // REMOVED: eg Memory coupons as in #586 (b.endDays < 0 ? 1 : numAllEpisodes); expired barriers have only 1 episode ... the doAccruals.evaluate()
					double              thisProbDefault = probDefault(hazardCurve, thisYears);
					for (i = 0; i < b.hit.size(); i++){
						thisAmount = b.hit[i].amount;
						// possibly apply credit adjustment
						if (applyCredit) { thisAmount *= ((double)rand() / (RAND_MAX)) < thisProbDefault ? actualRecoveryRate : 1; }
						thisBarrierPayoffs.push_back(thisAmount);
						thisBarrierSumPayoffs += thisAmount;
					}

					if (b.capitalOrIncome) {
						if (!foundEarliest)                        { foundEarliest = true; probEarliest = prob; }
						if (b.settlementDate < lastSettlementDate) {                       probEarly   += prob; }
						sumDuration                 += numInstances*thisYears;
						numCapitalInstances         += numInstances;
						sumPayoffs                  += b.sumPayoffs;
						sumPossiblyCreditAdjPayoffs += thisBarrierSumPayoffs;
						eStrPosPayoff    += b.sumStrPosPayoffs; numStrPosInstances += b.numStrPosPayoffs; 
						ePosPayoff       += b.sumPosPayoffs;    numPosInstances    += b.numPosPayoffs;
						eNegPayoff       += b.sumNegPayoffs;    numNegInstances    += b.numNegPayoffs;

						for (i = 0; i < b.hit.size(); i++){
							double thisAmount = thisBarrierPayoffs[i];
							double thisAnnRet = min(0.2,exp(log((thisAmount < unwindPayoff ? unwindPayoff : thisAmount) / midPrice) / thisYears) - 1.0); // assume once investor has lost 90% it is unwound...
							
							// maybe save finalAssetReturns
							if (doFinalAssetReturn && !applyCredit && totalFarCounter<400000){  // DOME: this is 100 iterations, with around 4000obs per iteration ... in many years time this limit needs to be increased!
								if (farCounter){ strcat(farBuffer, ","); }
								sprintf(farBuffer, "%s%s%d%s%.3lf%s%.3lf%s", farBuffer, "(", productId, ",", thisAmount, ",", b.fars[i], ")");
								farCounter += 1;
								if (farCounter == 100){
									totalFarCounter += farCounter;
									strcat(farBuffer, ";");
									mydb.prepare((SQLCHAR *)farBuffer, 1);
									strcpy(farBuffer, "insert into finalassetreturns values ");
									farCounter = 0;
								}
							}
							
							allPayoffs.push_back(thisAmount);
							allFVpayoffs.push_back(thisAmount*pow(b.forwardRate, maxYears - b.yearsToBarrier ));
							allAnnRets.push_back(thisAnnRet);
							double bmRet = benchmarkId >0 ? exp(log(b.bmrs[i]) / thisYears - contBenchmarkTER) - 1.0 : hurdleReturn;
							bmAnnRets.push_back(bmRet);
							sumYearsToBarrier += thisYears;
							bmRelLogRets.push_back(log((thisAmount < unwindPayoff ? unwindPayoff : thisAmount) / midPrice) - log(1 + bmRet)*thisYears);
							sumAnnRets += thisAnnRet;
							
							if (thisAnnRet > -tol &&  thisAnnRet < tol) { sumParAnnRets += thisAnnRet; numParInstances++; }
							if (thisAnnRet >  0.0 ) { sumStrPosPayoffs += thisAmount; numStrPosPayoffs++;    sumStrPosDurations += thisYears; }
							if (thisAnnRet > -tol ) { sumPosPayoffs    += thisAmount; numPosPayoffs++;       sumPosDurations    += thisYears; }
							else                    { sumNegPayoffs    += thisAmount; numNegPayoffs++;       sumNegDurations    += thisYears; }
						}
					}

					double mean      = numInstances ? thisBarrierSumPayoffs / numInstances : 0.0;
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

				if (numPosInstances    > 0)    { ePosPayoff    /= numPosInstances; }
				if (numStrPosInstances > 0)    { eStrPosPayoff /= numStrPosInstances; }
				if (numNegInstances    > 0)    { eNegPayoff    /= numNegInstances; }

				int numAnnRets(allAnnRets.size());
				double duration  = sumDuration / numAnnRets;

				// winlose ratios for different cutoff returns
				// ...two ways to do it
				// ...first recognises the fact that a 6y annuity is worth more than a 1y annuity
				// ...second assumes annualised returns have equal duration
				bool doWinLoseAnnualised = true; // as you want
				if (analyseCase == 0) {
					if (doDebug){ std::cerr << "Starting analyseResults WinLose for case \n" << analyseCase << std::endl; }
					sprintf(lineBuffer, "%s%d%s", "delete from winlose where productid='", productId, "';");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
					double winLoseMinRet       =  doWinLoseAnnualised ? -0.20 : 0.9;
					const double winLoseMaxRet =  doWinLoseAnnualised ?  0.21 : 2.0;
					const std::vector<double> &theseWinLoseMeasures = doWinLoseAnnualised ? allAnnRets : allFVpayoffs;
					const double thisWinLoseDivisor  = doWinLoseAnnualised ? 1.0 : midPrice;
					const double thisWinLoseClick    = doWinLoseAnnualised ? 0.01 : 0.05;
					const double thisWinLoseVolScale = doWinLoseAnnualised ? sqrt(duration) : 1;
					while (winLoseMinRet < winLoseMaxRet) {
						double sumWinLosePosPayoffs = 0.0;
						double sumWinLoseNegPayoffs = 0.0;
						int    numWinLosePosPayoffs = 0;
						int    numWinLoseNegPayoffs = 0;
						for (j = 0, len=allAnnRets.size(); j<len; j++) {
							double payoff   = theseWinLoseMeasures[j] /thisWinLoseDivisor - winLoseMinRet;
							if (payoff > 0){ sumWinLosePosPayoffs += payoff; numWinLosePosPayoffs += 1; }
							else           { sumWinLoseNegPayoffs -= payoff; numWinLoseNegPayoffs += 1; }
						}
						double winLose        = numWinLoseNegPayoffs ? (thisWinLoseVolScale*sumWinLosePosPayoffs / sumWinLoseNegPayoffs) : 1000.0;
						if (winLose > 1000.0){ winLose = 1000.0; }

						sprintf(lineBuffer, "%s%d%s%.4lf%s%.6lf%s",
							"insert into winlose values (", productId, ",", 100.0*winLoseMinRet, ",", winLose, ");");
						mydb.prepare((SQLCHAR *)lineBuffer, 1);
						winLoseMinRet += thisWinLoseClick;
					}

				}


				// possibly track timepoints
				if (doTimepoints && analyseCase == 0){
					for (i=0; i < numTimepoints; i++){
						int thisTpDays   = timepointDays[i];
						std::string name = timepointNames[i];
						for (j=0; j < numUls; j++){
							bool firstTime      = true;
							sprintf(lineBuffer, "%s", "insert into timepoints (UserId,ProductId,UnderlyingId,TimePointDays,Name,Percentile,SimValue) values ");
							int ulId = ulIds[j];
							sort(timepointLevels[i][j].begin(), timepointLevels[i][j].end());
							int numTpLevels = timepointLevels[i][j].size();
							// save simPercentiles
							for (k=0, len=simPercentiles.size(); k < len; k++){
								double pctile = simPercentiles[k];
								if (firstTime){ firstTime = false; }
								else { strcat(lineBuffer, ","); }
								int thisIndx = floor(numTpLevels*simPercentiles[k]);
								double value = timepointLevels[i][j][thisIndx];
								sprintf(lineBuffer, "%s%s%d%s%d%s%d%s%s%s%lf%s%lf%s", lineBuffer, "(3,", productId, ",", ulId, ",", thisTpDays, ",'", name.c_str(), "',", pctile, ",", value, ")");
							}
							mydb.prepare((SQLCHAR *)lineBuffer, 1);
						}
					}
				}

				// benchmark underperformance
				double benchmarkProbUnderperf, benchmarkCondUnderperf, benchmarkProbOutperf, benchmarkCondOutperf;
				double bmRelUnderperfPV, bmRelOutperfPV, bmRelCAGR, cumUnderperfPV, cumOutperfPV;
				double cumValue = 0.0, cumValue1 = 0.0;
				int    cumCount = 0, cumCount1 = 0;
				cumUnderperfPV = 0.0;
				cumOutperfPV   = 0.0;
				bmRelCAGR      = 0.0;
				for (i=0; i<numAnnRets; i++) {
					double anyDouble = allAnnRets[i] - bmAnnRets[i];
					if (anyDouble < 0.0){
						cumCount       += 1;
						cumValue       -= anyDouble;
						cumUnderperfPV += exp(bmRelLogRets[i]);
					}
					else {
						cumCount1    += 1;
						cumValue1    += anyDouble;
						cumOutperfPV += exp(bmRelLogRets[i]);
					}
					bmRelCAGR += bmRelLogRets[i];
				}
				if (cumCount) {
					benchmarkProbUnderperf = ((double)cumCount) / numAnnRets;
					benchmarkCondUnderperf = cumValue / cumCount;
					bmRelUnderperfPV       = cumUnderperfPV / cumCount;
				}
				if (cumCount1) {
					benchmarkProbOutperf = ((double)cumCount1) / numAnnRets;
					benchmarkCondOutperf = cumValue1 / cumCount1;
					bmRelOutperfPV       = cumOutperfPV / cumCount1;
				}
				bmRelCAGR = exp(bmRelCAGR / sumYearsToBarrier) - 1.0;

				

				// ** process overall product results
				const double confLevel(0.1), confLevelTest(0.05);  // confLevelTest is for what-if analysis, for different levels of conf
				sort(allPayoffs.begin(), allPayoffs.end());
				sort(allAnnRets.begin(), allAnnRets.end());
				double averageReturn = sumAnnRets / numAnnRets;
				double vaR975        = 100.0*allAnnRets[floor(numAnnRets*(0.025))];

				// SRRI vol
				struct srriParams { double conf, normStds, normES; };

				srriParams shortfallParams[] = { // in R,normalExpectedShorfall = vol*dnorm(qnorm(prob))/prob with prob=0.05 for 95%conf
					{ 0.975,  1.96,   2.3378 },
					{ 0.99,   2.326,  2.665 },
					{ 0.999,  3.09,   3.367 },
				};
				i=0;
				double srriConf, srriStds, srriConfRet, normES, cesrStrictVol;
				// some products have an averageReturn somewhat to the left of the 2.5% VaR, so we need to use a more extreme percentile
				do {
					srriConf      = shortfallParams[i].conf;
					srriStds      = shortfallParams[i].normStds;
					normES        = shortfallParams[i].normES;
					srriConfRet   = allAnnRets[floor(numAnnRets*(1 - srriConf))];
					if (i == 0){ cesrStrictVol = -(srriStds - sqrt(srriStds*srriStds + 2 * (log(1 + averageReturn) - log(1 + srriConfRet)))); }
					i += 1;
				} while (srriConfRet>averageReturn && i<3);


				//if(srriConfRet>historicalReturn) {srriConf = 1-1.0/numAnnRets;srriStds = -NormSInv(1.0/numAnnRets);  srriConfRet = annRetInstances[0];}
				// replaced the following line with the next one as 'averageReturn' rather than 'historicalReturn' is how we do ESvol
				// BUT DO NOT DELETE as 'historicalReturn' may be the better way: DOME
				//var srriVol        = -100*(srriStds - Math.sqrt(srriStds*srriStds+4*0.5*(Math.log(1+historicalReturn/100) - Math.log(1+srriConfRet/100))));
				double srriVol       = -(srriStds - sqrt(srriStds*srriStds + 2 * (log(1 + averageReturn) - log(1 + srriConfRet))));





				// pctiles and other calcs
				if (numMcIterations > 1 && analyseCase == 0) {
					if (doDebug){ std::cerr << "Starting analyseResults PcTile for case \n" << analyseCase << std::endl; }

					// pctiles
					double bucketSize = productShape == "Supertracker" ? 0.05 : 0.01;
					double minReturn = allAnnRets[0];
					std::vector<double>    returnBucket;
					std::vector<double>    bucketProb;
					for (i = j = 0; i < numAnnRets; i++) {
						double thisRet = allAnnRets[i];
						if (thisRet <= minReturn || thisRet > 1.0) { j += 1; }  // final bucket for any return above 100%pa
						else { returnBucket.push_back(minReturn); bucketProb.push_back(((double)j) / numAnnRets); j = 0; minReturn += bucketSize; }
					}
					returnBucket.push_back(minReturn); bucketProb.push_back(((double)j) / numAnnRets);
					sprintf(lineBuffer, "%s%d%s", "delete from pctiles where productid='", productId, "';");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
					
					sprintf(lineBuffer, "%s", "insert into pctiles values ");
					for (i=0; i < returnBucket.size(); i++){
						if (i != 0){ sprintf(lineBuffer, "%s%s", lineBuffer, ","); }
						sprintf(lineBuffer, "%s%s%d%s%.2lf%s%.4lf%s%d%s%d%s", lineBuffer, "(", productId, ",", 100.0*returnBucket[i], ",", bucketProb[i], ",", numMcIterations, ",", analyseCase == 0 ? 0 : 1, ")");
					}
					sprintf(lineBuffer, "%s%s", lineBuffer, ";");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);	
				}


				// eShortfall, esVol
				if (doDebug){ std::cerr << "Starting analyseResults SavingToDatabase for case \n" << analyseCase << std::endl; }

				const double depoRate = 0.01;  // in decimal...DOME: could maybe interpolate curve for each instance
				int numShortfall(    floor(confLevel    *numAnnRets));
				int numShortfallTest(floor(confLevelTest*numAnnRets));
				double eShortfall(0.0);	    for (i = 0; i < numShortfall;     i++){ eShortfall     += allAnnRets[i]; }	eShortfall     /= numShortfall;
				double eShortfallTest(0.0);	for (i = 0; i < numShortfallTest; i++){ eShortfallTest += allAnnRets[i]; }	eShortfallTest /= numShortfall;
				double esVol     = (log(1 + averageReturn) - log(1 + eShortfall))     / ESnorm(confLevel);
				double esVolTest = (log(1 + averageReturn) - log(1 + eShortfallTest)) / ESnorm(confLevelTest);
				double scaledVol = esVol * sqrt(duration);
				double geomReturn(0.0);	
				for (i = 0; i < numAnnRets; i++){ 
					double thisPayoff = allPayoffs[i]; 
					geomReturn += log((thisPayoff<unwindPayoff ? unwindPayoff : thisPayoff) / midPrice);
				}
				geomReturn = exp(geomReturn / sumDuration) - 1;
				double sharpeRatio = scaledVol > 0.0 ? (geomReturn / scaledVol>1000.0 ? 1000.0 : geomReturn / scaledVol) : 1000.0;
				std::vector<double> cesrBuckets = { 0.0, 0.005, .02, .05, .1, .15, .25, .4 };
				std::vector<double> cubeBuckets ={ 0.0, 0.026, 0.052, 0.078, 0.104, 0.130, 0.156, 0.182, 0.208, 0.234, 0.260, 0.40 };
				double riskCategory   = calcRiskCategory(cesrBuckets,scaledVol,1.0);  
				double riskScore1to10 = calcRiskCategory(cubeBuckets, scaledVol, 0.0);

				// WinLose
				double sumNegRet(0.0), sumPosRet(0.0),sumBelowDepo(0.0);
				int    numNegRet(0), numPosRet(0), numBelowDepo(0);
				for (j = 0; j<numAnnRets; j++) {
					double ret = allAnnRets[j];
					if (ret<0){           sumNegRet    += ret;  numNegRet++;    }
					else {                sumPosRet    += ret;  numPosRet++;    }
					if (ret < depoRate) { sumBelowDepo += ret;  numBelowDepo++; }
				}
				double probBelowDepo  = (double)numBelowDepo / (double)numAnnRets;
				double eShortfallDepo = numBelowDepo ? sumBelowDepo / (double)numBelowDepo : 0.0;
				double esVolBelowDepo = (log(1 + averageReturn) - log(1 + eShortfallDepo)) / ESnorm(probBelowDepo);
				double eNegRet        = numNegRet ? sumNegRet / (double)numNegRet : 0.0;
				double probNegRet     = (double)numNegRet / (double)numAnnRets;
				double esVolNegRet    = (log(1 + averageReturn) - log(1 + eNegRet)) / ESnorm(probNegRet);
				double strPosDuration(sumStrPosDurations / numStrPosPayoffs), posDuration(sumPosDurations / numPosPayoffs), negDuration(sumNegDurations / numNegPayoffs);
				double ecGain         = 100.0*(numPosPayoffs ? exp(log(sumPosPayoffs / midPrice / numPosPayoffs) / posDuration) - 1.0 : 0.0);
				
				double ecStrictGain   = 100.0*(numStrPosPayoffs ? exp(log(sumStrPosPayoffs / midPrice / numStrPosPayoffs) / strPosDuration) - 1.0 : 0.0);
				double ecLoss         = -100.0*(numNegPayoffs ? exp(log(sumNegPayoffs / midPrice / numNegPayoffs) / negDuration) - 1.0 : 0.0);
				double probGain       = numPosRet ? ((double)numPosRet) / numAnnRets : 0;
				double probStrictGain = numStrPosPayoffs ? ((double)numStrPosPayoffs) / numCapitalInstances : 0;
				double probLoss       = 1 - probGain;
				double eGainRet       = ecGain * probGain;
				double eLossRet       = ecLoss * probLoss;
				// on balance, prefer to use annualised returns, rather than payoffs
				double winLose;
				if (doWinLoseAnnualised){ winLose = sumNegRet ? (sumPosRet / -sumNegRet)*sqrt(duration) : 1000.0; }
				else { winLose        = numNegPayoffs ? -(sumPosPayoffs / midPrice - numPosPayoffs*1.0) / (sumNegPayoffs / midPrice - numNegPayoffs*1.0) : 1000.0; }
				if (winLose > 1000.0){ winLose = 1000.0; }
				double expectedPayoff = sumPayoffs / numAnnRets;
				int    secsTaken = difftime(time(0), startTime);

				sprintf(lineBuffer, "%s%s%d", lineBuffer, "',SecsTaken='",                   secsTaken);
				sprintf(lineBuffer, "%s%.5lf", "update cashflows set ExpectedPayoff='",      expectedPayoff);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ExpectedGainPayoff='",       ePosPayoff);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ExpectedStrictGainPayoff='", eStrPosPayoff);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ExpectedLossPayoff='",       eNegPayoff);
				sprintf(lineBuffer, "%s%s%s", lineBuffer, "',FirstDataDate='",               allDates[0].c_str());
				sprintf(lineBuffer, "%s%s%s", lineBuffer, "',LastDataDate='",                allDates[totalNumDays - 1].c_str());
				sprintf(lineBuffer, "%s%s%d", lineBuffer, "',NumResamples='",       thisIteration);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',Duration='",        duration);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VolStds='",         srriStds);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VolConf='",         srriConf);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',AverageAnnRet='",   averageReturn);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',CESRvol='",         srriVol);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',CESRstrictVol='",   cesrStrictVol);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ESvol='",           esVol);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ESvolTest='",      esVolTest);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ESvolBelowDepo='", esVolBelowDepo);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ESvolNegRet='",    esVolNegRet);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ExpectedReturn='", geomReturn);
				// sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',EArithReturn='",   averageReturn);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',EArithReturn='", pow(sumPossiblyCreditAdjPayoffs / midPrice / numAnnRets, 1.0 / duration) - 1.0);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',SharpeRatio='",    sharpeRatio);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',RiskCategory='",   riskCategory);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',RiskScore1to10='", riskScore1to10);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',WinLose='",        winLose);
				std::time_t rawtime;	struct std::tm * timeinfo;  time(&rawtime);	timeinfo = localtime(&rawtime);
				strftime(charBuffer, 100L, "%Y-%m-%d %H:%M:%S", timeinfo);
				sprintf(lineBuffer, "%s%s%s",    lineBuffer, "',WhenEvaluated='", charBuffer);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ProbEarliest='",  probEarliest);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ProbEarly='",     probEarly);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VaR='",           vaR975);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecGain='",        ecGain);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecStrictGain='",  ecStrictGain);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecLoss='",        ecLoss);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probGain='",      probGain);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probStrictGain='",probStrictGain);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probLoss='",      probLoss);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecPar='",         numParInstances ? sumParAnnRets / (double)numParInstances : 0.0);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probPar='",       (double)numParInstances / (double)numAnnRets);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',eShortfall='",    eShortfall*100.0);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',eShortfallDepo='",eShortfallDepo*100.0);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ProbBelowDepo='", probBelowDepo);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BenchmarkProbShortfall='", benchmarkProbUnderperf);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BenchmarkCondShortfall='", benchmarkCondUnderperf*100.0);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BenchmarkProbOutperf='", benchmarkProbOutperf);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BenchmarkCondOutperf='", benchmarkCondOutperf*100.0);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BmRelCAGR='", bmRelCAGR);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BmRelOutperfPV='", bmRelOutperfPV);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BmRelUnderperfPV='", bmRelUnderperfPV);
				sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BmRelAverage='", bmRelUnderperfPV*benchmarkProbUnderperf + bmRelOutperfPV*benchmarkProbOutperf);

				sprintf(lineBuffer, "%s%s%d", lineBuffer, "',NumEpisodes='", numAllEpisodes);

				sprintf(lineBuffer, "%s%s%d%s%.2lf%s", lineBuffer, "' where ProductId='", productId, "' and ProjectedReturn='", projectedReturn, "'");
				std::cout << lineBuffer <<  std::endl;

				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				//retcode = mydb.execute(true);
			}
		}
	}
};