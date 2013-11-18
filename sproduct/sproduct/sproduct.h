

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

enum { fixedPayoff = 1, callPayoff, putPayoff, twinWinPayoff, switchablePayoff, basketCallPayoff, lookbackCallPayoff };
enum { uFnLargest, uFnLargestN};


class UlTimeseries {
public:
	UlTimeseries() {};
	std::vector <double> price;
	std::vector <std::string> date;
};

class SpPayoff {

public:
	SpPayoff(std::string date, double amount) : date(date),amount(amount){};
	std::string date;
	double amount;
};


class SpBarrierRelation {

public:
	SpBarrierRelation(int underlying,
		double        barrier,
		double        uBarrier,
		std::string   startDate,	
		std::string   endDate,
		bool          above,
		bool          at,
		std::string   productStartDateString)
		: underlying(underlying), barrier(barrier), uBarrier(uBarrier), 
		startDate(startDate), endDate(endDate),above(above),at(at),
		strikeAdjForMoneyness(1.0), moneyness(1.0)
	{
		using namespace boost::gregorian;
		date bStartDate(from_simple_string(startDate));
		date bEndDate(from_simple_string(endDate));
		date bProductStartDate(from_simple_string(productStartDateString));
		
		startDays             = (bStartDate - bProductStartDate).days();
		endDays               = (bEndDate - bProductStartDate).days();
	};
	const bool        above, at; 
	const int         underlying;
	const double      barrier, uBarrier, strikeAdjForMoneyness,moneyness;
	const std::string startDate, endDate;		
	int               startDays, endDays;
	double            barrierLevel, uBarrierLevel;
	
	void setLevels(const double ulPrice) {
		barrierLevel  = barrier  * ulPrice;
		uBarrierLevel = uBarrier * ulPrice;
	}
	
};

class SpBarrier {
public:
	SpBarrier(bool       capitalOrIncome,
		std::string      nature, 
		double           payoff, 
		std::string      settlementDate, 
		std::string      description, 
		std::string      payoffType,
		int              payoffTypeId,
		double           strike, 
		double           cap,
		double           participation,
		std::vector<int> ulIdNameMap,
		boost::gregorian::date bProductStartDate)
		: capitalOrIncome(capitalOrIncome), nature(nature), payoff(payoff), 
		settlementDate(settlementDate), description(description), payoffType(payoffType), 
		payoffTypeId(payoffTypeId), strike(strike), cap(cap), participation(participation), ulIdNameMap(ulIdNameMap),
		underlyingFunctionId(0), isAnd(nature == "and")
		{
		using namespace boost::gregorian;
		date bEndDate(from_simple_string(settlementDate));
		endDays              = (bEndDate - bProductStartDate).days();
		sumPayoffs           = 0.0;
	};
	const int                       payoffTypeId, underlyingFunctionId;
	const bool                      capitalOrIncome,isAnd;
	const double                    payoff, strike, cap, participation;
	const std::string               nature, settlementDate, description,payoffType;
	const std::vector<int>          ulIdNameMap;
	int                             endDays;
	double                          sumPayoffs;
	std::vector <SpBarrierRelation> brel;
	std::vector <SpPayoff>          hit;

	int getEndDays(){ return endDays; };
	bool isHit(std::vector<double> &thesePrices) {  // test if barrier is hit
			int j;
			bool isHit        = isAnd; 
			int numBrel       = brel.size();  // could be zero eg simple maturity barrier (no attached barrier relations)
			if (numBrel == 0) return true;

			for (j = 0; j<numBrel; j++) {
				SpBarrierRelation &thisBrel(brel[j]);
				int  thisIndx      = ulIdNameMap[thisBrel.underlying];
				bool above         = thisBrel.above;
				double thisUlPrice = thesePrices[thisIndx], diff = thisUlPrice - thisBrel.barrierLevel;
				bool thisTest      = above ? diff>0 : diff < 0;
				
				if (thisBrel.uBarrier != 0.0){
					diff      = thisUlPrice - thisBrel.uBarrierLevel;
					thisTest &= above ? diff<0 : diff>0;
				}
				if (isAnd)  isHit &= thisTest;
				else        isHit |= thisTest;
			}
			return isHit;
		};
	double getPayoff(   std::vector<double> &startLevels, 
						std::vector<double> &lookbackLevel,
						std::vector<double> &thesePrices) {
		double         thisPayoff(payoff),optionPayoff(0.0), p, thisRefLevel, thisAssetReturn;
		std::vector<double> optionPayoffs;
		int            callOrPut = -1,j,len;     				// default option is a put

		switch (payoffTypeId) {
		case callPayoff:
		case lookbackCallPayoff:
		case twinWinPayoff:
			callOrPut = 1;
		case putPayoff:
			for (j = 0, len = brel.size(); j<len; j++) {
				SpBarrierRelation &thisBrel(brel[j]);
				int n         = ulIdNameMap[thisBrel.underlying];
				double strike = thisBrel.strikeAdjForMoneyness * startLevels[n];
				thisRefLevel  = startLevels[n] / thisBrel.moneyness;
				if (payoffTypeId == lookbackCallPayoff) {
					thisAssetReturn = lookbackLevel[n] / thisRefLevel;
				}
				else {
					thisAssetReturn = thesePrices[n] / thisRefLevel;
				}

				p = callOrPut*(thisAssetReturn - strike / thisRefLevel);
				if (payoffTypeId == twinWinPayoff) { p = fabs(p); }
				if (p > cap){ p = cap;}
				optionPayoffs.push_back(p);
			}
			switch (underlyingFunctionId) {
			case uFnLargest:
				for (optionPayoff = 0.0, j = 0, len = optionPayoffs.size(); j<len; j++) {
					if (optionPayoffs[j] > optionPayoff) { optionPayoff = optionPayoffs[j]; }
				}
				break;
			case uFnLargestN:
				/* DOME
				optionPayoffs.sort(function(a, b){ return b - a; }); // sort DESCENDING
				for (optionPayoff = 0, j = 0, len = thisBarrier.Param1; j<len; j++) { optionPayoff += optionPayoffs[j]; }
				optionPayoff = Math.max(0, optionPayoff / j);
				*/
				break;
			}
			thisPayoff += participation*optionPayoff;
			break;
		}
		return(thisPayoff);
	}
	void storePayoff(const std::string thisDateString, const double amount){
		sumPayoffs += amount;
		hit.push_back(SpPayoff(thisDateString, amount));
	}
};

// structured product
class SProduct {
	private:
		int id;

	public:
	SProduct() : id(0) {};
	int                      productDays;
	std::vector <SpBarrier>  barrier;
};
