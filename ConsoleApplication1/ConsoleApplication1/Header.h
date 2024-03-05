#include <boost/lambda/lambda.hpp>
//#include <boost/regex.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
// standard includes: C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\um
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <windows.h>
#include <sqlext.h>             // includes seem to be in C:\Program Files (x86)\Windows Kits\8.1\Include\um
#include <stdio.h>
#include <vector>
#include <regex>
#include <iomanip>
#include <chrono>
#include <iomanip>

#define DAYS_PER_YEAR                      365.25
#define MAX_ULS                            100
#define MAX_ISSUERS                        10
#define MAX_SP_BUF                         500000
#define MIN_CALLABLE_ITERATIONS            100
#define MAX_CALLABLE_ITERATIONS          200000
#define CALLABLE_REGRESSION_FRACTION       0.2
#define MIN_FUNDING_FRACTION_FACTOR       -10.0
#define YEARS_TO_INT_MULTIPLIER           1000000.0
#define ARTS_MAX_RAND                     4294967296.0   // 2^32
#define LARGE_RETURN                      10.0
#define CI_CLOSEDOUBLE					  1.0e-1	    // for tests of equality between 2 doubles
#define EQ                                ==
#define NEQ                               !=
#define USE_GMM_CLUSTERS                  1             // Gaussian Mixture Model for issuerCallable
#define MAX_GMM_CLUSTERS                  7
#define MIN_GMM_CLUSTERS                  3
#define GMM_INIT_MIX_RANDOMLY             0
#define	INVERT_USING_LU_DECOMPOSITION     1
#define MY_SQL_GENERAL_ERROR             -1            // all the SQL codes seem to be non-negative, so this is a way of signalling something general went wrong
#define PI                              3.141592653589793238

// Numerical Recipes types
typedef double DP;


// NR NRVec type
template <class T>
class NRVec {
private:
	int nn;	// size of array. upper index is nn-1
	T *v;
public:
	NRVec();
	explicit NRVec(int n);		// Zero-based array
	NRVec(const T &a, int n);	//initialize to constant value
	NRVec(const T *a, int n);	// Initialize to array
	NRVec(const NRVec &rhs);	// Copy constructor
	NRVec & operator=(const NRVec &rhs);	//assignment
	NRVec & operator=(const T &a);	//assign a to every element
	inline T & operator[](const int i);	//i'th element
	inline const T & operator[](const int i) const;
	inline int size() const;
	~NRVec();
};

template <class T>
NRVec<T>::NRVec() : nn(0), v(0) {}

template <class T>
NRVec<T>::NRVec(int n) : nn(n), v(new T[n]) {}

template <class T>
NRVec<T>::NRVec(const T& a, int n) : nn(n), v(new T[n])
{
	for (int i=0; i<n; i++)
		v[i] = a;
}

template <class T>
NRVec<T>::NRVec(const T *a, int n) : nn(n), v(new T[n])
{
	for (int i=0; i<n; i++)
		v[i] = *a++;
}

template <class T>
NRVec<T>::NRVec(const NRVec<T> &rhs) : nn(rhs.nn), v(new T[nn])
{
	for (int i=0; i<nn; i++)
		v[i] = rhs[i];
}

template <class T>
NRVec<T> & NRVec<T>::operator=(const NRVec<T> &rhs)
// postcondition: normal assignment via copying has been performed;
//		if vector and rhs were different sizes, vector
//		has been resized to match the size of rhs
{
	if (this != &rhs)
	{
		if (nn != rhs.nn) {
			if (v != 0) delete[](v);
			nn=rhs.nn;
			v= new T[nn];
		}
		for (int i=0; i<nn; i++)
			v[i]=rhs[i];
	}
	return *this;
}

template <class T>
NRVec<T> & NRVec<T>::operator=(const T &a)	//assign a to every element
{
	for (int i=0; i<nn; i++)
		v[i]=a;
	return *this;
}

template <class T>
inline T & NRVec<T>::operator[](const int i)	//subscripting
{
#if DO_BOUNDS_CHECKING  > 0
	if (i>nn) CiError("Vector out of bounds");
#endif
	return v[i];
}

template <class T>
inline const T & NRVec<T>::operator[](const int i) const	//subscripting
{
#if DO_BOUNDS_CHECKING  > 0
	if (i>nn) CiError("Vector out of bounds");
#endif
	return v[i];
}

template <class T>
inline int NRVec<T>::size() const
{
	return nn;
}

template <class T>
NRVec<T>::~NRVec()
{
	if (v != 0)
		delete[](v);
}

typedef const NRVec<DP> Vec_I_DP;
typedef NRVec<DP> Vec_DP, Vec_O_DP, Vec_IO_DP;
typedef const NRVec<int> Vec_I_INT;
typedef NRVec<int> Vec_INT, Vec_O_INT, Vec_IO_INT;



// NR NRMat type
template <class T>
class NRMat {
private:
	int nn;
	int mm;
	T **v;
public:
	NRMat();
	NRMat(int n, int m);			        // Zero-based array
	NRMat(const T &a, int n, int m);	    // Initialize to constant
	NRMat(const T *a, int n, int m);	    // Initialize to array
	NRMat(const NRMat &rhs);		        // Copy constructor
	NRMat & operator=(const NRMat &rhs);	// assignment
	NRMat & operator=(const T &a);		    // assign a to every element
	inline T* operator[](const int i);	    // subscripting: pointer to row i
	inline const T* operator[](const int i) const;
	inline int nrows() const;
	inline int ncols() const;
	~NRMat();
};

template <class T>
NRMat<T>::NRMat() : nn(0), mm(0), v(0) {}

template <class T>
NRMat<T>::NRMat(int n, int m) : nn(n), mm(m), v(new T*[n])
{
	v[0] = new T[m*n];
	for (int i=1; i< n; i++)
		v[i] = v[i - 1] + m;
}

template <class T>
NRMat<T>::NRMat(const T &a, int n, int m) : nn(n), mm(m), v(new T*[n])
{
	int i, j;
	v[0] = new T[m*n];
	for (i=1; i< n; i++)
		v[i] = v[i - 1] + m;
	for (i=0; i< n; i++)
	for (j=0; j<m; j++)
		v[i][j] = a;
}

template <class T>
NRMat<T>::NRMat(const T *a, int n, int m) : nn(n), mm(m), v(new T*[n])
{
	int i, j;
	v[0] = new T[m*n];
	for (i=1; i< n; i++)
		v[i] = v[i - 1] + m;
	for (i=0; i< n; i++)
	for (j=0; j<m; j++)
		v[i][j] = *a++;
}

template <class T>
NRMat<T>::NRMat(const NRMat &rhs) : nn(rhs.nn), mm(rhs.mm), v(new T*[nn])
{
	int i, j;
	v[0] = new T[mm*nn];
	for (i=1; i< nn; i++)
		v[i] = v[i - 1] + mm;
	for (i=0; i< nn; i++)
	for (j=0; j<mm; j++)
		v[i][j] = rhs[i][j];
}

template <class T>
NRMat<T> & NRMat<T>::operator=(const NRMat<T> &rhs)
// postcondition: normal assignment via copying has been performed;
//		if matrix and rhs were different sizes, matrix
//		has been resized to match the size of rhs
{
	if (this != &rhs) {
		int i, j;
		if (nn != rhs.nn || mm != rhs.mm) {
			if (v != 0) {
				delete[](v[0]);
				delete[](v);
			}
			nn=rhs.nn;
			mm=rhs.mm;
			v = new T*[nn];
			v[0] = new T[mm*nn];
		}
		for (i=1; i< nn; i++)
			v[i] = v[i - 1] + mm;
		for (i=0; i< nn; i++)
		for (j=0; j<mm; j++)
			v[i][j] = rhs[i][j];
	}
	return *this;
}

template <class T>
NRMat<T> & NRMat<T>::operator=(const T &a)	//assign a to every element
{
	for (int i=0; i< nn; i++)
	for (int j=0; j<mm; j++)
		v[i][j] = a;
	return *this;
}

template <class T>
inline T* NRMat<T>::operator[](const int i)	//subscripting: pointer to row i
{
#if DO_BOUNDS_CHECKING  > 0
	if (i>nn) CiError("Vector out of bounds");
#endif
	return v[i];
}

template <class T>
inline const T* NRMat<T>::operator[](const int i) const
{
#if DO_BOUNDS_CHECKING  > 0
	if (i>nn) CiError("Vector out of bounds");
#endif
	return v[i];
}

template <class T>
inline int NRMat<T>::nrows() const
{
	return nn;
}

template <class T>
inline int NRMat<T>::ncols() const
{
	return mm;
}

template <class T>
NRMat<T>::~NRMat()
{
	if (v != 0) {
		delete[](v[0]);
		delete[](v);
	}
}
typedef const NRMat<DP> Mat_I_DP;
typedef NRMat<DP> Mat_DP, Mat_O_DP, Mat_IO_DP;



// structs
union ArtsRandomNumber {
	long unsigned int bucket;
	unsigned int      bits[2];
};

struct EvalResult	{
	double value, stdErr;
	int    errorCode;
	EvalResult(double value, double stdErr, int errorCode) : value(value), stdErr(stdErr), errorCode(errorCode) {}
};


struct PriipsStruct	{
	double pvReturn, yearsToPayoff;

	PriipsStruct(double pvReturn, double yearsToPayoff) : pvReturn(pvReturn), yearsToPayoff(yearsToPayoff) {}

	bool operator < (const PriipsStruct& other) const	{
		return (pvReturn < other.pvReturn);
	}
};

struct AnnRet	{
	double annRet, yearsToPayoff;

	AnnRet(double annRet, double yearsToPayoff) : annRet(annRet), yearsToPayoff(yearsToPayoff) {}

	bool operator < (const AnnRet& other) const	{
		return (annRet < other.annRet);
	}
};

struct finalAssetInfo	{
	double ret;
	int    assetIndx, barrierIndx;

	finalAssetInfo(double ret, int assetIndx, int barrierIndx) : ret(ret), assetIndx(assetIndx), barrierIndx(barrierIndx) {}
};

struct postStrikeState	{
	double initialBudget,budgetUsed,lockedIn;

	void init(){
		initialBudget = 0.0;
		budgetUsed    = 0.0;
		lockedIn      = 0.0;
	}

	postStrikeState() {
		init();
	}
};

struct fAndDf	{
	double f,df;
	fAndDf(double f, double df) : f(f), df(df) {}
};


/*
* functions
*/
// little error handler
void CiError(const std::string msg) {
	std::cerr << msg;
	//DebugActiveProcess(?how get this processID?);
	//DebugBreak();
	exit(1);
}
// two matrices equal?
void MEqual(const Mat_I_DP &one, const Mat_I_DP &two){
	char m_sBuf[1000];

	if (one.nrows() NEQ two.nrows() || one.ncols() NEQ two.ncols()) CiError("MEqual: different sizes");
	int i, j;

	for (i=0; i<one.nrows(); i++)
	for (j=0; j<one.ncols(); j++)
	if (fabs(one[i][j] - two[i][j]) > CI_CLOSEDOUBLE)	{
		sprintf(m_sBuf,"%s %d %d %lf %lf", "MEqual:", i, j, one[i][j], two[i][j]);
		CiError(m_sBuf);
	}
}
// tests and returns true/false rather than screen pop-ups
bool MEqualTest(const Mat_I_DP &one, const Mat_I_DP &two){
	if (one.nrows() NEQ two.nrows() || one.ncols() NEQ two.ncols()) CiError("MEqual: different sizes");
	int i, j;

	for (i=0; i<one.nrows(); i++)
	for (j=0; j<one.ncols(); j++)
	if (fabs(one[i][j] - two[i][j]) > CI_CLOSEDOUBLE)
		return false;

	return true;
}




int    iMax(const    int a, const    int b){ return a > b ? a : b; }
int    iMin(const    int a, const    int b){ return a < b ? a : b; }
double fMax(const double a, const double b){ return a > b ? a : b; }
double fMin(const double a, const double b){ return a < b ? a : b; }

/*
* recalcLocalVol() from impvol
*   - follows Gatheral
*/
void recalcLocalVol(
	const std::vector<std::vector<double>>                    &ulVolsTenor,
	const std::vector<std::vector<std::vector<double>> >      &ulVolsStrike,
	const std::vector<std::vector<std::vector<double>> >      &ulVolsImpvol,
	const std::vector<std::vector<double>>                    &ulFwdsAtVolTenor,
	std::vector<std::vector<std::vector<double>> >            &ulVolsBumpedLocalVol
	){
	int thisUidx, thisTenorIdx, thisStrikeIdx, iUp, iDown, jUp, jDown, numStrikes;
	double dI,dJ,thisT,thisStrike,thisVariance,thisVol,thisFwd;
	std::vector<double> someVect;
	std::vector<int> numTenors;
	int numUl = (int)ulVolsTenor.size();
	for (thisUidx=0; thisUidx < numUl; thisUidx++){
		numTenors.push_back((int)ulVolsTenor[thisUidx].size());
	}
	/*
	* build w=totalVarianceMatrix 
	*/
	std::vector<std::vector<std::vector<double>> >      totalVariance(numUl);
	for(thisUidx=0; thisUidx < numUl; thisUidx++){		
		for (thisTenorIdx=0; thisTenorIdx < numTenors[thisUidx]; thisTenorIdx++){
			thisT = ulVolsTenor[thisUidx][thisTenorIdx];
			numStrikes = (int)ulVolsStrike[thisUidx][thisTenorIdx].size();
			someVect.resize(0); 
			for (thisStrikeIdx=0; thisStrikeIdx < numStrikes; thisStrikeIdx++){
				thisStrike   = ulVolsStrike[thisUidx][thisTenorIdx][thisStrikeIdx];
				thisVol      = ulVolsImpvol[thisUidx][thisTenorIdx][thisStrikeIdx];
				thisVariance = thisT * thisVol * thisVol;
				someVect.push_back(thisVariance);
			}
			totalVariance[thisUidx].push_back(someVect);
		}
	}
	/*
	* build dWbyDt
	*/
	std::vector<std::vector<std::vector<double>> >      dWbyDt(numUl);
	for (thisUidx=0; thisUidx < numUl; thisUidx++){
		for (thisTenorIdx=0; thisTenorIdx < numTenors[thisUidx]; thisTenorIdx++){
			numStrikes = (int)ulVolsStrike[thisUidx][thisTenorIdx].size();
			someVect.resize(0); 
			for (thisStrikeIdx=0; thisStrikeIdx < numStrikes; thisStrikeIdx++){
				iDown = iMax(thisTenorIdx - 1, 0);
				iUp   = iMin(thisTenorIdx + 1, numTenors[thisUidx] - 1);
				jDown = iDown;
				jUp   = iUp;
				dI    = ulVolsTenor  [thisUidx]               [iUp] - ulVolsTenor  [thisUidx]                [iDown];
				dJ    = totalVariance[thisUidx][jUp][thisStrikeIdx] - totalVariance[thisUidx][jDown][thisStrikeIdx];
				someVect.push_back(dJ / dI);
			}
			dWbyDt[thisUidx].push_back(someVect);
		}
	}
	/*
	* build y=logStrikeByFwd
	*/
	std::vector<std::vector<std::vector<double>> >      logStrikeByFwd(numUl);
	for (thisUidx=0; thisUidx < numUl; thisUidx++){
		for (thisTenorIdx=0; thisTenorIdx < numTenors[thisUidx]; thisTenorIdx++){
			thisFwd    = ulFwdsAtVolTenor[thisUidx][thisTenorIdx];
			numStrikes = (int)ulVolsStrike[thisUidx][thisTenorIdx].size();
			someVect.resize(0);
			for (thisStrikeIdx=0; thisStrikeIdx < numStrikes; thisStrikeIdx++){
				thisStrike   = ulVolsStrike[thisUidx][thisTenorIdx][thisStrikeIdx];
				someVect.push_back(log(thisStrike/thisFwd));
			}
			logStrikeByFwd[thisUidx].push_back(someVect);
		}
	}
	/*
	* build dWbyDy
	*/
	std::vector<std::vector<std::vector<double>> >      dWbyDy(numUl);
	for (thisUidx=0; thisUidx < numUl; thisUidx++){
		for (thisTenorIdx=0; thisTenorIdx < numTenors[thisUidx]; thisTenorIdx++){
			numStrikes = (int)ulVolsStrike[thisUidx][thisTenorIdx].size();
			someVect.resize(0);
			for (thisStrikeIdx=0; thisStrikeIdx < numStrikes; thisStrikeIdx++){
				jDown = iMax(thisStrikeIdx - 1, 0);
				jUp   = iMin(thisStrikeIdx + 1, numStrikes - 1);
				dI    = logStrikeByFwd[thisUidx][thisTenorIdx][jUp] - logStrikeByFwd[thisUidx][thisTenorIdx][jDown];
				dJ    = totalVariance [thisUidx][thisTenorIdx][jUp] - totalVariance [thisUidx][thisTenorIdx][jDown];
				someVect.push_back(dJ / dI);
			}
			dWbyDy[thisUidx].push_back(someVect);
		}
	}
	/*
	* build d2WbyDy2
	*/
	std::vector<std::vector<std::vector<double>> >      d2WbyDy2(numUl);
	for (thisUidx=0; thisUidx < numUl; thisUidx++){
		for (thisTenorIdx=0; thisTenorIdx < numTenors[thisUidx]; thisTenorIdx++){
			numStrikes = (int)ulVolsStrike[thisUidx][thisTenorIdx].size();
			someVect.resize(0);
			for (thisStrikeIdx=0; thisStrikeIdx < numStrikes; thisStrikeIdx++){
				if (thisStrikeIdx == 0 || thisStrikeIdx == (numStrikes-1)){
					jDown = iMax(thisStrikeIdx - 1, 0);
					jUp   = iMin(thisStrikeIdx + 1, numStrikes - 1);
					dI    = logStrikeByFwd[thisUidx][thisTenorIdx][jUp] - logStrikeByFwd[thisUidx][thisTenorIdx][jDown];
					dJ    = dWbyDy[thisUidx][thisTenorIdx][jUp] - dWbyDy[thisUidx][thisTenorIdx][jDown];
					someVect.push_back(dJ / dI);
				}
				else{   // more exact second derivative of w
					double base1 = logStrikeByFwd[thisUidx][thisTenorIdx][thisStrikeIdx - 1];
					double base2 = logStrikeByFwd[thisUidx][thisTenorIdx][thisStrikeIdx    ];
					double base3 = logStrikeByFwd[thisUidx][thisTenorIdx][thisStrikeIdx + 1];
					double part1 = totalVariance[thisUidx][thisTenorIdx][thisStrikeIdx - 1] / ((base2 - base1)*(base3 - base1));
					double part2 = totalVariance[thisUidx][thisTenorIdx][thisStrikeIdx    ] / ((base3 - base2)*(base2 - base1));
					double part3 = totalVariance[thisUidx][thisTenorIdx][thisStrikeIdx + 1] / ((base3 - base2)*(base3 - base1));
					someVect.push_back(2.0 *(part1 - part2 + part3));
				}
			}
			d2WbyDy2[thisUidx].push_back(someVect);
		}
	}
	/*
	* build denominator
	*/
	std::vector<std::vector<std::vector<double>> >      denom(numUl);
	for (thisUidx=0; thisUidx < numUl; thisUidx++){
		for (thisTenorIdx=0; thisTenorIdx < numTenors[thisUidx]; thisTenorIdx++){
			numStrikes = (int)ulVolsStrike[thisUidx][thisTenorIdx].size();
			someVect.resize(0);
			for (thisStrikeIdx=0; thisStrikeIdx < numStrikes; thisStrikeIdx++){
				double thisDwByDy    = dWbyDy[thisUidx][thisTenorIdx][thisStrikeIdx];
				double thisLogStrike = logStrikeByFwd[thisUidx][thisTenorIdx][thisStrikeIdx];
				double thisVariance  = totalVariance[thisUidx][thisTenorIdx][thisStrikeIdx];
				double thisD2eByDy2  = d2WbyDy2[thisUidx][thisTenorIdx][thisStrikeIdx];
				someVect.push_back(1.0 - thisDwByDy*thisLogStrike / thisVariance + 0.25*(-0.25 - 1 / thisVariance + thisLogStrike*thisLogStrike / thisVariance / thisVariance)*(thisDwByDy*thisDwByDy) + 0.5*thisD2eByDy2);
			}
			denom[thisUidx].push_back(someVect);
		}
	}
	/*
	* build localVol
	*/
	double maxVariance     = 0.7*0.7;
	for (thisUidx=0; thisUidx < numUl; thisUidx++){
		double defaultVariance = 0.2*0.2;
		ulVolsBumpedLocalVol[thisUidx].resize(0);
		for (thisTenorIdx=0; thisTenorIdx < numTenors[thisUidx]; thisTenorIdx++){
			thisT      = ulVolsTenor[thisUidx][thisTenorIdx];
			numStrikes = (int)ulVolsStrike[thisUidx][thisTenorIdx].size();
			someVect.resize(0);
			for (thisStrikeIdx=0; thisStrikeIdx < numStrikes; thisStrikeIdx++){
				thisStrike       = ulVolsStrike[thisUidx][thisTenorIdx][thisStrikeIdx];
				double thisValue = dWbyDt[thisUidx][thisTenorIdx][thisStrikeIdx] / denom[thisUidx][thisTenorIdx][thisStrikeIdx];
				if (thisValue <= 0.0){ 
					std::cerr << "recalcLocalVol set to default: uidx:" << thisUidx << " tenor:" << thisT << " strike:" << thisStrike << std::endl;
					thisValue = defaultVariance;
				}  // default 20% vol if something goes wrong
				else if (thisValue > maxVariance){
					thisValue = maxVariance;
				}
				someVect.push_back(sqrt(thisValue));
				defaultVariance = thisValue;
			}
			ulVolsBumpedLocalVol[thisUidx].push_back(someVect);
		}
	}
	return;
}

// LU decomposition
void ludcmp(Mat_IO_DP &a, Vec_O_INT &indx, DP &d) {
	const DP TINY=1.0e-20;
	int i, imax, j, k;
	DP big, dum, sum, temp;

	int n=a.nrows();
	Vec_DP vv(n);
	d=1.0;
	for (i=0; i<n; i++) {
		big=0.0;
		for (j=0; j<n; j++)
		if ((temp=fabs(a[i][j])) > big) big=temp;
		if (big == 0.0) CiError("Singular matrix in routine ludcmp");
		vv[i]=1.0 / big;
	}
	for (j=0; j<n; j++) {
		for (i=0; i<j; i++) {
			sum=a[i][j];
			for (k=0; k<i; k++) sum -= a[i][k] * a[k][j];
			a[i][j]=sum;
		}
		big=0.0;
		for (i=j; i<n; i++) {
			sum=a[i][j];
			for (k=0; k<j; k++) sum -= a[i][k] * a[k][j];
			a[i][j]=sum;
			if ((dum=vv[i] * fabs(sum)) >= big) {
				big=dum;
				imax=i;
			}
		}
		if (j != imax) {
			for (k=0; k<n; k++) {
				dum=a[imax][k];
				a[imax][k]=a[j][k];
				a[j][k]=dum;
			}
			d = -d;
			vv[imax]=vv[j];
		}
		indx[j]=imax;
		if (a[j][j] == 0.0) a[j][j]=TINY;
		if (j != n - 1) {
			dum=1.0 / (a[j][j]);
			for (i=j + 1; i<n; i++) a[i][j] *= dum;
		}
	}
}

void lubksb(Mat_I_DP &a, Vec_I_INT &indx, Vec_IO_DP &b){
	int i, ii=0, ip, j;
	DP sum;

	int n=a.nrows();
	for (i=0; i<n; i++) {
		ip=indx[i];
		sum=b[ip];
		b[ip]=b[i];
		if (ii != 0)
		for (j=ii - 1; j<i; j++) sum -= a[i][j] * b[j];
		else if (sum != 0.0)
			ii=i + 1;
		b[i]=sum;
	}
	for (i=n - 1; i >= 0; i--) {
		sum=b[i];
		for (j=i + 1; j<n; j++) sum -= a[i][j] * b[j];
		b[i]=sum / a[i][i];
	}
}

// just print out matrix
void PrintMatrix(const Mat_I_DP &in, std::string name) {
	const int C_ROWS(in.nrows());
	const int C_COLS(in.ncols());

	std::cerr << name << "-->" << std::endl;
	for (int i=0; i < C_ROWS; i++){
		for (int j=0; j < C_COLS; j++){
			std::cerr << in[i][j] << "\t";
		}
		std::cerr << std::endl;
	}
	std::cerr << "<--" << name << std::endl;
}

// element by element copy
void MatCopy(const Mat_I_DP &in, Mat_O_DP &out) {
	const int C_ROWS(in.nrows());
	const int C_COLS(in.ncols());
	if (out.nrows() NEQ C_ROWS || out.ncols() NEQ C_COLS)	CiError("MatCopy: wrong sizes");
	int i, j;

	for (i=0; i<C_ROWS; i++)
	for (j=0; j<C_COLS; j++)
		out[i][j] = in[i][j];
}
// check matrix is diagonal
// - output sum of diagonal elements
// - if allEqual true they must all equal toThis
bool Diagonal(const	Mat_I_DP	&mat, double *sum, const bool allEqual, const double toThis) {
	const int C_ROWS(mat.nrows());
	const int C_COLS(mat.ncols());
	if (C_ROWS NEQ C_COLS) CiError("Diagonal: not square");
	int i, j;
	double thisSum, thisValue;

	thisSum = 0.0;
	for (i=0; i<C_ROWS; i++)
	for (j=0; j<C_COLS; j++) {
		thisValue = mat[i][j];
		if (i EQ j)	{
			if (allEqual EQ true && fabs(thisValue - toThis) > CI_CLOSEDOUBLE) {/*CCointegDoc::CiError("Diagonal: not all equal");*/ return false; }
			thisSum += thisValue;
		}
		else
		if (fabs(thisValue - 0.0) > CI_CLOSEDOUBLE)	{/*CCointegDoc::CiError("Diagonal: not zero");*/return false; }
	}
	*sum = thisSum;

	return true;
}
#define BIG_VECTOR   10000
#define HUGE_VECTOR  1000000
// matrix-matrix multiply
void MMult(const Mat_I_DP	&one,
	const Mat_I_DP	&two,
	Mat_O_DP		&out,
	const bool		oneTranspose,
	const bool		twoTranspose)
{
	const int		oneRows=one.nrows(), oneCols=one.ncols();
	const int		twoRows=two.nrows(), twoCols=two.ncols();
	const int		outRows=out.nrows(), outCols=out.ncols();
	int		indx, i, j, k, l, m;
	double	sum, *oneVals;
	double *twoVals = new double[twoRows*twoCols];

	//clock_t  startTicks,stopTicks;
	//if(m_clockStarted EQ true) startTicks=clock();

	if (oneTranspose EQ true)	{
		oneVals = new double[oneRows];
		if (oneCols NEQ outRows) CiError("MMult: one&out wrong rows");
		if (twoTranspose EQ true) {
			if (twoRows NEQ outCols) CiError("MMult: two&out wrong cols");
			if (oneRows NEQ twoCols) CiError("MMult: one&two incompatible");
			// NEW CODE - extra accuracy by mean-correcting each vector, and at the end adding back n*mean1*mean2 (McKinnon p29)
			Vec_DP twoMeans(0.0, twoRows), oneMeans(0.0, oneCols);
			// stack two's rows
			for (i=0; i<twoRows; i++) {
				for (j=0; j<twoCols; j++) twoMeans[i] += two[i][j];
				twoMeans[i] /= twoCols;
			}
			for (indx=i=0; i<twoRows; i++) for (j=0; j<twoCols; j++) twoVals[indx++]= two[i][j] - twoMeans[i];
			for (k=0; k<outRows; k++){
				for (m=0; m<oneRows; m++) oneMeans[k] += one[m][k];
				oneMeans[k] /= oneRows;
				for (m=0; m<oneRows; m++) oneVals[m]=one[m][k] - oneMeans[k];
				for (indx=l=0; l<outCols; l++){
					sum=0.0;
					for (m=0; m<oneRows; m++) sum += oneVals[m] * twoVals[indx++];
					out[k][l] = sum + oneRows * oneMeans[k] * twoMeans[l];
				}
			}
		}
		else {
			if (twoCols NEQ outCols) CiError("MMult: two&out wrong cols");
			if (oneRows NEQ twoRows) CiError("MMult: one&two incompatible");
			// NEW CODE - extra accuracy by mean-correcting each vector, and at the end adding back n*mean1*mean2 (McKinnon p29)
			Vec_DP twoMeans(0.0, twoCols), oneMeans(0.0, oneCols);
			// stack two's columns
			for (j=0; j<twoCols; j++){
				for (i=0; i<twoRows; i++) twoMeans[j] += two[i][j];
				twoMeans[j] /= twoRows;
			}
			for (indx=j=0; j<twoCols; j++) for (i=0; i<twoRows; i++) twoVals[indx++]= two[i][j] - twoMeans[j];
			for (k=0; k<outRows; k++){
				for (m=0; m<oneRows; m++) oneMeans[k] += one[m][k];
				oneMeans[k] /= oneRows;
				for (m=0; m<oneRows; m++) oneVals[m]=one[m][k] - oneMeans[k];
				for (indx=l=0; l<outCols; l++){
					sum=0.0;
					for (m=0; m<oneRows; m++) sum += oneVals[m] * twoVals[indx++];
					out[k][l] = sum + oneRows * oneMeans[k] * twoMeans[l];
				}
			}
		}
	}
	else
	{
		oneVals = new double[oneCols];
		if (oneRows NEQ outRows) CiError("MMult: one&out wrong rows");
		if (twoTranspose EQ true) {
			if (twoRows NEQ outCols) CiError("MMult: two&out wrong cols");
			if (oneCols NEQ twoCols) CiError("MMult: one&two incompatible");

			// NEW CODE
			Vec_DP twoMeans(0.0, twoRows), oneMeans(0.0, oneRows);
			// stack two's rows
			for (i=0; i<twoRows; i++){
				for (j=0; j<twoCols; j++) twoMeans[i] += two[i][j];
				twoMeans[i] /= twoCols;
			}
			for (indx=i=0; i<twoRows; i++) for (j=0; j<twoCols; j++) twoVals[indx++]= two[i][j] - twoMeans[i];
			for (k=0; k<outRows; k++){
				for (m=0; m<oneCols; m++) oneMeans[k] += one[k][m];
				oneMeans[k] /= oneCols;
				for (m=0; m<oneCols; m++) oneVals[m]=one[k][m] - oneMeans[k];
				for (indx=l=0; l<outCols; l++){
					sum=0.0;
					for (m=0; m<oneCols; m++) sum += oneVals[m] * twoVals[indx++];
					out[k][l] = sum + oneCols * oneMeans[k] * twoMeans[l];
				}
			}
			//end NEW CODE
		}
		else {
			if (twoCols NEQ outCols) CiError("MMult: two&out wrong cols");
			if (oneCols NEQ twoRows) CiError("MMult: one&two incompatible");
			// NEW CODE - extra accuracy by mean-correcting each vector, and at the end adding back n*mean1*mean2 (McKinnon p29)
			Vec_DP twoMeans(0.0, twoCols), oneMeans(0.0, oneRows);
			// stack two's columns
			for (j=0; j<twoCols; j++){
				for (i=0; i<twoRows; i++) twoMeans[j] += two[i][j];
				twoMeans[j] /= twoRows;
			}
			for (indx=j=0; j<twoCols; j++) for (i=0; i<twoRows; i++) twoVals[indx++]= two[i][j] - twoMeans[j];
			for (k=0; k<outRows; k++) {

				for (m=0; m<oneCols; m++) oneMeans[k] += one[k][m];
				oneMeans[k] /= oneCols;
				for (m=0; m<oneCols; m++) oneVals[m]=one[k][m] - oneMeans[k];
				for (indx=l=0; l<outCols; l++){
					sum=0.0;
					for (m=0; m<oneCols; m++) sum += oneVals[m] * twoVals[indx++];
					out[k][l] = sum + oneCols * oneMeans[k] * twoMeans[l];
				}
			}
			// END NEW CODE
		}
	}
	delete[] oneVals; delete[] twoVals;
}


// compute matrix inverse, using svd
// PROBLEMS: -if the matrix is near singular, chances are the model you've chosen isn't a good one
// see commments on SolveEigensystem
bool MatInverse(const Mat_I_DP	&mat, Mat_O_DP &inv){
	const int	C_MSIZE(mat.nrows());
	if (C_MSIZE NEQ mat.ncols()
		|| C_MSIZE NEQ inv.nrows()
		|| C_MSIZE NEQ inv.ncols()) CiError("MatInverse: wrong size");
	int			k, l;
	double		anyDouble, sum;
	double		&dRef = anyDouble;
	Mat_DP		augMat(0.0, C_MSIZE, C_MSIZE + C_MSIZE), test(0.0, C_MSIZE, C_MSIZE), test2(C_MSIZE, C_MSIZE), u(0.0, C_MSIZE, C_MSIZE), v(C_MSIZE, C_MSIZE);
	Vec_INT		indx(C_MSIZE);
	Vec_DP		w(0.0, C_MSIZE);

	
	//DumpMatrix(mat,"MatrixToInvert.dmp",(ofstream *)0,20);
	MatCopy(mat, u);   // copy original matrix into u, which NR routines destroy
#if INVERT_USING_LU_DECOMPOSITION > 0
	ludcmp(u, indx, dRef);
	for (int j=0; j<C_MSIZE; j++){
		// form the unit vector
		for (int i=0; i<C_MSIZE; i++) w[i]=0.0;
		w[j]=1.0;
		lubksb(u, indx, w);
		for (int i=0; i < C_MSIZE; i++){
			test[i][j] = w[i];
		}
	}
#endif
#if INVERT_LONGHAND > 0
	// augment matrix with Identity matrix
	for (i=0; i<C_MSIZE; i++)
	for (j=0; j<C_MSIZE; j++)
	{
		augMat[i][j] = mat[i][j]; if (i EQ j) augMat[i][C_MSIZE + j] = 1.0;
	}

	// just pivot one row/col at a time
	for (i=0; i<C_MSIZE; i++)
	{
		pivot=augMat[i][i];
		if (pivot EQ 0.0) CiError("InvertMatrix: zero pivot...");
		//divide this row by pivot
		for (k=0; k<C_MSIZE + C_MSIZE; k++) augMat[i][k] /= pivot;
		// now subtract this row from the other rows
		for (j=0; j<C_MSIZE; j++)
		{
			if (j NEQ i)
			{
				pivot = augMat[j][i];
				for (k=0; k<C_MSIZE + C_MSIZE; k++) augMat[j][k] -= pivot * augMat[i][k];
			}
		}
		// DumpMatrix(augMat,"AugMat.dmp",(ofstream *)0,8);
	}
	for (i=0; i<C_MSIZE; i++)
	for (j=0; j<C_MSIZE; j++)
		test[i][j] = augMat[i][C_MSIZE + j];
#endif
	//check it is inverse
	//DumpMatrix(test,"InverseMatrix.dmp",(ofstream *)0,20);
	MMult(test, mat, test2, false, false);
	if (Diagonal(test2, &sum, true, 1.0) EQ false)
	{
		//we could...
		return false;
		// 
		//...but just warn user for now: lets see what happens if we try and get close...
		//
		CiError("MatInverse:ill-conditioned matrix may give poor inversion, TREAT SOLUTIONS WITH CARE....");
	}
	// return the inverse
	for (k=0; k<C_MSIZE; k++)
	{
		for (l=0; l<C_MSIZE; l++)
			inv[k][l] = test[k][l];
	}
	return true;
}

/*
* my2dDet   - determinant of a 2d matrix  a  b
*                                         c  d     =  a.d - b.c
*/
double my2dDet(const double a, const double b, const double c, const double d) { return(a*d - b*c); }

/*
*  mvpdf - compute z probabilities for x,y points under multiple normal densities with parameters (mu, covariance=covX,covXY,covY) 
*/
EvalResult mvpdf(Mat_IO_DP     &z,
	const std::vector<double>  &x, 
	const std::vector<double>  &y, 
	const Mat_IO_DP            &mu,
	const Vec_IO_DP            &covX, 
	const Vec_IO_DP            &covY,
	const Vec_IO_DP            &covXY,
	const int                  thisNumClusters            // only use this number of elements/columns in above matrices/vectors
	) {

	EvalResult  evalResult(0.0, 0.0, 0);
	const int   numClusters(thisNumClusters);

	for (int i=0; i < numClusters;i++) {
		//
		// inverse of cov[i]   a  b            =  1/(a.d - b.c)   *   d   -b
		//                     c  d                                  -c    a
		//
		double det  = my2dDet(           covX[i],           covXY[i],            covXY[i],            covY[i]);
		double oneOver2piRootDet = 1.0 / (pow(det * 2.0 * PI, 0.5));
		double recipDet, inverseA, inverseB, inverseD;
		recipDet = 1.0 / det;
		inverseA =  recipDet * covY [i];
		inverseB = -recipDet * covXY[i];
		inverseD =  recipDet * covX [i];
		if (det == 0) {
			std::cerr << "IPRerror: Determinant is equal to 0 for cluster: " << i << "\n";
			evalResult.errorCode = 11111;
			return(evalResult);
		}
		// for each x,y point compute normal probability under this normal distribution
		for (int j=0; j < x.size(); j++) {
			double thisX = x[j] - mu[i][0];
			double thisY = y[j] - mu[i][1];
			// (x,y) * [inverseA  inverseB] * [x]
			//         [inverseB  inverseD]   [y]
			double thisProb =  exp(-0.5 * (thisX*(thisX*inverseA + thisY * inverseB) + thisY * (thisX*inverseB + thisY * inverseD))) * oneOver2piRootDet;
			z[j][i] = thisProb;
		}
	}
	return(evalResult);
}


/*
* evalAlgebra
*/
// ** handle AvgInAlgebra
// ... parse algebra and evaluate it on data   eg    min_-1.0_add_abs   would calculate min(data), subtract 1.0 and take its abs()
// ... rerurns a number
double evalAlgebra(const std::vector<double> data, std::string algebra){
	if (data.size() == 0 || algebra.length() == 0){ return 0.0; }

	// algebra, once tokenised, is evaluated in reverse polish
	int i, len, pos, j;
	int numValues = (int) data.size();
	char buffer[100]; sprintf(buffer, "%s", algebra.c_str());
	char *token = std::strtok(buffer, "_");
	std::vector<std::string> tokens;
	while (token != NULL) {
		tokens.push_back(token);
		token = std::strtok(NULL, "_");
	}

	int numTokens =(int) tokens.size();
	double result = 0.0;
	std::vector<double> stack;    for (i=0; i<numValues; i++){ stack.push_back(data[i]); }
	double extremum;

	for (i=0; i < numTokens; i++){
		pos = (int) stack.size() - 1;
		if (tokens[i] == "min"){
			extremum = DBL_MAX;
			for (j=0, len=(int)stack.size(); j < len; j++){
				if (stack[j] < extremum){ extremum = stack[j]; }
			}
			stack.clear();
			stack.push_back(extremum);
		}
		else if (tokens[i] == "max"){
			extremum = -DBL_MAX;
			for (j=0, len=(int)stack.size(); j < len; j++){
				if (stack[j] > extremum){ extremum = stack[j]; }
			}
			stack.clear();
			stack.push_back(extremum);
		}
		else if (tokens[i] == "add"){
			stack[pos-1] = stack[pos] + stack[pos-1];
			stack.pop_back();
		}
		else if (tokens[i] == "multiply"){
			stack[pos - 1] =  stack[pos - 1] * stack[pos];
			stack.pop_back();
		}
		else if (tokens[i] == "divide"){
			stack[pos - 1] =  stack[pos - 1] / stack[pos];
			stack.pop_back();
		}
		else if (tokens[i] == "abs"){
			stack[pos] = abs(stack[pos]);
		}
		else if (tokens[i] == "floor"){
			stack[pos] = floor(stack[pos]);
		}
		else if (tokens[i] == "latest"){
			stack.push_back(data[numValues-1]);
		}
		else {
			stack.push_back(atof(tokens[i].c_str()));
		}
	}
	return stack[0];
}



/*
* irr
*/

// calculate fn(x) and fn'(x)
// (SpBarrier::*isHit)(const double) 
fAndDf functionEval(double(*fn)(const double, const std::vector<double> &c, const std::vector<double> &t), 
	const double x, 
	const std::vector<double> &c, 
	const std::vector<double> &t){
	double f  = fn(x,c,t);
	double f1 = fn(x + 0.0001,c,t);
	double f2 = fn(x - 0.0001,c,t);
	double df = (f1 - f2) / 0.0002;
	return(fAndDf(f, df));
}
// calculate PV of a bunch of cashflows c occurring at time t, at a continuous rate r
double pv(const double r, const std::vector<double> &c, const std::vector<double> &t){
	double sumPv = 0.0;
	for (int i=0; i<(int)c.size(); i++){
		sumPv += c[i] * exp(-r*t[i]);
	}
	return(sumPv);
}

// ** Newton-Raphson root finding for IRR of cashflows c occurring at time t  - Press pp370
double irr(const std::vector<double> &c, const std::vector<double> &t) {
	int maxit = 100;
	double xacc  = 0.0001;     // 1bp accuracy
	double x1    = -0.9;       // lower bound guess
	double x2    =  0.9;       // upper bound guess ... some products nearing KO may have funny midPrices
	int  i, j;
	double  dx, dxold, f, df, fh, fl, temp, xh, xl, rts;
	fAndDf funcResults(0,0) ;


	// initial values at upper/lower bound
	funcResults = functionEval(pv, x1,c,t); fl = funcResults.f; df = funcResults.df;
	funcResults = functionEval(pv, x2,c,t); fh = funcResults.f; df = funcResults.df;
	// handle when not bracketed
	if ((fl>0.0 && fh>0.0) || (fl<0.0 && fh<0.0)){
		double sumCashflows(0.0);
		for (i=0; i < (int)c.size(); i++){ sumCashflows += c[i]; }
		if (sumCashflows <= 0.0){   // products about to mature for x may have a slightly high midPrice making sumCashflows negative
			return(x1);
		}
		else {                   // products about to mature for x may have a slightly low  midPrice requiring a very high IRR to zero them
			return(x2);
		}
	}
	if (fl == 0.0) { return(x1); }
	if (fh == 0.0) { return(x2); }
	// orient the search so that f(x1)<0
	if (fl<0.0){ xl=x1; xh=x2; }
	else      { xh=x1; xl=x2; }
	rts   = 0.5*(x1 + x2);      // initial guess for root
	dxold = abs(x2 - x1);  // stepsize before last
	dx    = dxold;            // last stepsize
	funcResults = functionEval(pv, rts,c,t); f = funcResults.f; df = funcResults.df;
	// iterate
	for (j=0; j<maxit; j++){
		if ((((rts - xh)*df - f)*((rts - xl)*df - f) > 0.0) ||  	// bisect if Newton out of range 
			(abs(f*2.0) > abs(dxold*df))){           // or not decreasing fast enough
			dxold = dx; dx = 0.5*(xh - xl); rts = xl + dx;
			if (xl == rts){
				return(rts);
			}                       // finish if change in root negligible
		}
		else {                                                // Newton step acceptable. Take it
			dxold=dx; dx=f / df; temp=rts; rts -= dx;
			if (temp == rts){
				return(rts);
			}                     // finish if change in root negligible
		}
		if (abs(dx) < xacc){
			return(rts);
		}                 // convergence criterion
		funcResults = functionEval(pv, rts,c,t); f = funcResults.f; df = funcResults.df;
		if (f<0.0){ xl=rts; }
		else { xh=rts; } // maintain the bracket on the root
	}
	{ //alert("IRR root-finding: iterations exhausted"); 
		return(0.0); 
	}
}







// correlation
double MyCorrelation(std::vector<double> aValues, std::vector<double> bValues, const bool corrOrCov) {
	int N = (int)aValues.size();
	if (N == 0) { return(0.0); }
	double fMean  = std::accumulate(aValues.begin(), aValues.end(), 0.0) / N;
	double fMean1 = std::accumulate(bValues.begin(), bValues.end(), 0.0) / N;
	double aVariance = 0.0, bVariance = 0.0, coVariance = 0.0;
	for (int i=0; i < N; i++) {
		double diff       = aValues[i] - fMean;
		double diff1      = bValues[i] - fMean1;
		aVariance  += diff*diff;
		bVariance  += diff1*diff1;
		coVariance += diff*diff1;
	}
	if (corrOrCov) {
		coVariance /= (sqrt(aVariance)*sqrt(bVariance));
	}
	return(coVariance/(N-1));
}


// mean and stdev
template<typename T>
void MeanAndStdev(T&v, double &mean, double &stdev, double &stdErr){
	int N = (int)v.size();
	// mean
	double sum = std::accumulate(v.begin(), v.end(), 0.0);
	mean = sum / N;
	// stdev
	std::vector<double> diff(N);
	std::transform(v.begin(), v.end(), diff.begin(), std::bind2nd(std::minus<double>(), mean));
	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	stdev  = std::sqrt(sq_sum / N);
	stdErr = std::sqrt(sq_sum) / N;
}
template<typename T>
double Mean(T&v){
	int N = (int)v.size();
	double sum = std::accumulate(v.begin(), v.end(), 0.0);
	return(sum / N);
}

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
	std::map<char, int> avgTenor; avgTenor['d'] = 1; avgTenor['w'] = 7; avgTenor['m'] = 30; avgTenor['q'] = 91; avgTenor['s'] = 182; avgTenor['y'] = 365;
	std::map<char, int>::iterator curr, end;
	char buf[10];
	int tenorPeriodDays = 0;
	int tenorLen  = (int)strlen(avgTenorText);
	char avgChar2 = tolower(avgTenorText[tenorLen-1]);
	/* one way to do it
	for (found = false, curr = avgTenor.begin(), end = avgTenor.end(); !found && curr != end; curr++) {
	if (curr->first == avgChar2){ found = true; tenorPeriodDays = curr->second; }
	}*/
	if (avgTenor.find(avgChar2) != avgTenor.end()){
		tenorPeriodDays = avgTenor[avgChar2];
	}
	else { throw std::out_of_range("map_at()"); }
	strncpy(buf, avgTenorText, tenorLen - 1);
	buf[tenorLen - 1] = '\0';
	int numTenor = atoi(buf);
	avgDays  = numTenor * tenorPeriodDays;  // maybe add 1 since averaging invariably includes both end dates

	int avgFreqLen = (int)strlen(avgFreqText);
	int avgFreqStride = 1;
	if (avgFreqLen > 1){ 
		strncpy(buf, avgFreqText, avgFreqLen-1); 
		buf[avgFreqLen - 1] = '\0';
		avgFreqStride = atoi(buf);
	}
	avgChar2 = tolower(avgFreqText[avgFreqLen-1]);
	if (avgTenor.find(avgChar2) != avgTenor.end()){
		avgFreq = avgTenor[avgChar2] * avgFreqStride;
	}
	else { throw std::out_of_range("map_at()"); }
}

// cds functions

//  interpCurve: interpolate the 'curveValue' for a 'point' in time
double interpCurve(std::vector<double> curveTimes, std::vector<double> curveValues,double point){
	int len = (int)curveTimes.size();
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

// probDefault: cumulatveProbDefault by time 'point'
double probDefault(std::vector<double> curveProbs, const double point){
	int len = (int)curveProbs.size();
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

// bootstrapCDS:  calculate probOfDefault for each time bucket of a given CDS curve
void bootstrapCDS(const std::vector<double> r,                     // cdsSpreads
	              std::vector<double>       &dpCurve,              // probOfDefault vector to build and return
	              const double              recoveryRate){         // constant % of debt recovery
	int len((int)r.size());
	if (!len) return;
	double thisProb, cumProbAlive(0.0), probAliveThisPeriod(1.0), cumProbDefault(0.0);
	for (int i = 0; i<len; i++) {
		// probability of default during this time slot
		// ... the CDS spread is the required compensation, conditional on still being alive at the start of the period, assuming a given recoveryRate
		// ... see my "DefaultProbabilities from CDS.docx" for the derivation of this calc  
		thisProb = (r[i] * (cumProbAlive + probAliveThisPeriod) - (1 - recoveryRate)*cumProbDefault) / ((r[i] + 1 - recoveryRate)*probAliveThisPeriod);
		probAliveThisPeriod  *= 1 - thisProb;
		cumProbAlive         += probAliveThisPeriod;
		cumProbDefault       += thisProb;
		dpCurve.push_back(thisProb);
	}
}

// buld a hazardCurve = probOfdefault in each time bucket
void buildHazardCurve(const std::vector<double> cdsSpread, const std::vector<double> cdsTenor,const double maxYears, const double recoveryRate,std::vector<double> &hazardCurve){
	std::vector<double> dpCurve, fullCurve;             
	// first populate a full annual CDS curve of cdsSpreads
	for (int j = 0; j<maxYears + 1; j++) {
		fullCurve.push_back(interpCurve(cdsTenor, cdsSpread, j + 1));
	}
	// now compute a bootstrapped 'defaultProbability' curve dpCurve
	// ... and copy it into hazardCurve
	bootstrapCDS(fullCurve, dpCurve, recoveryRate);
	hazardCurve.empty();
	for (int j = 0, len = (int)fullCurve.size(); j<len; j++) {
		hazardCurve.push_back(dpCurve[j]);
	}
}


// regex functions
void splitCommaSepName(std::vector<std::string> &out, std::string s){

	std::regex word_regex("([^,]+)");
	auto words_begin = std::sregex_iterator(s.begin(), s.end(), word_regex);
	auto words_end   = std::sregex_iterator();

	int numWords = (int)std::distance(words_begin, words_end);
	
	for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
		std::smatch match = *i;
		out.push_back(match.str());
	}	
}
// split barrierCommand
void splitBarrierCommand(std::vector<std::string> &out, std::string s){

	std::regex word_regex("([^()]+)");
	auto words_begin = std::sregex_iterator(s.begin(), s.end(), word_regex);
	auto words_end   = std::sregex_iterator();

	int numWords = (int)std::distance(words_begin, words_end);

	for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
		std::smatch match = *i;
		out.push_back(match.str());
	}
}
// avoids regex - splits delimited string into stringVector
std::vector<std::string> split(std::string str, char delimiter) {
	std::vector<std::string> internal;
	std::stringstream ss(str); // Turn the string into a stream.
	std::string tok;
	while (getline(ss, tok, delimiter)) {
		internal.push_back(tok);
	}
	return internal;
}
// minAbs - return min of a and b, based on sign of b
double MyMinAbs(double a, double b){
	double absA  = fabs(a);
	double absB  = fabs(b);
	double sign  = b < 0.0 ? -1.0 : 1.0;
	return absA < absB ? sign*a : b;
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

// ********************
// ********************* functions for riskNeutral simulations
// ********************

// Cholesky decomposition of (correlation) matrix
EvalResult CHOL(const std::vector<std::vector<double>>  &matrix, std::vector<std::vector<double>> &outputMatrix) {
	int         i, j, k, N;
	double      element;
	EvalResult  evalResult(0.0, 0.0, 0);
	// init
	N = (int)matrix.size();
	std::vector<std::vector<double>>  a(N, std::vector<double>(N));             // the original matrix
	std::vector<std::vector<double>>  L_Lower(N, std::vector<double>(N));       // the new matrix
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			a[i][j] = matrix[i][j];   L_Lower[i][j] = 0.0;
		}
	}
	// decompose
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			element = a[i][j];
			for (k=0; k<i; k++) {
				element = element - L_Lower[i][k] * L_Lower[j][k];
			}
			if (i == j){ 
				if (element < 0.0){ 
					std::cerr << "IPRerror: Correlation matrix infeasible " << i << "\n"; 
					evalResult.errorCode = 10221;
					return(evalResult);
				}
				L_Lower[i][i] = sqrt(element);
			}
			else if (i < j) { 
				if (L_Lower[i][i] == 0.0){ 
					std::cerr << "IPRerror: Correlation matrix infeasible " << i << "\n"; 
					evalResult.errorCode = 10222;
					return(evalResult);
				}
				L_Lower[j][i] = element / L_Lower[i][i]; 
			}
		}
	}

	// finally transpose it
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			outputMatrix[i][j] = L_Lower[j][i];
		}
	}
	return(evalResult);
}


// arrayPosition - get first value in a 1-d array that is equal-to-or-larger than 'theValue'
int ArrayPosition(const std::vector<double> &theArray, 
		const double theValue, 
		const int comparison) {
	int i, found, len;
	found = -1;
	len   = (int)theArray.size();
	switch (comparison) {
	case -1: // first number greater than or equal to, or maximum
		for (i=0; i<len && found == -1; i++){
			if (theArray[i] >= theValue){ found = i; }
		}
		if (found == -1) { found = len - 1; }
		break;
	case 0: // exact match
		for (i=0; i<len && found == -1; i++){
			if (theArray[i] == theValue){ found = i; }
		}
		break;
	case 1: // first number less than or equal to, or minimum
		for (i=len - 1; i >= 0 && found == -1; i--){
			if (theArray[i] <= theValue){ found = i; }
		}
		if (found == -1) { found = 0; }
		break;
	}
	return found;
}

// interpolate matrix, based on how rowValue and colValue index into rowAxis and colAxis
double InterpolateMatrix(const std::vector<std::vector<double>> &matrix, 
	const std::vector<double> &rowAxis, 
	const std::vector<double> &colAxis, 
	const double rowValue, 
	const double colValue) {
	int loColIndx, loRowIndx, hiColIndx, hiRowIndx;
	double thisValue,colRange, rowRange, colFraction, rowFraction, interpolatedRowLoCol, interpolatedRowHiCol;
	// find column interpolation (indx,fraction)
	hiRowIndx   = ArrayPosition(rowAxis, rowValue, -1);
	hiColIndx   = ArrayPosition(colAxis, colValue, -1);
	loRowIndx   = ArrayPosition(rowAxis, rowValue, 1);
	loColIndx   = ArrayPosition(colAxis, colValue, 1);
	rowRange    = rowAxis[hiRowIndx] - rowAxis[loRowIndx];
	colRange    = colAxis[hiColIndx] - colAxis[loColIndx];
	colFraction = colRange == 0.0 ? 0.0 : (colValue - colAxis[loColIndx]) / colRange;
	rowFraction = rowRange == 0.0 ? 0.0 : (rowValue - rowAxis[loRowIndx]) / rowRange;


	// imagine a new row
	interpolatedRowLoCol = matrix[loRowIndx][loColIndx] + rowFraction * (matrix[hiRowIndx][loColIndx] - matrix[loRowIndx][loColIndx]);
	interpolatedRowHiCol = matrix[loRowIndx][hiColIndx] + rowFraction * (matrix[hiRowIndx][hiColIndx] - matrix[loRowIndx][hiColIndx]);

	// can check for NAN
	// According to the IEEE standard, NaN values have the odd property that comparisons involving them are always false. 
	// That is, for a float f, f != f will be true only if f is NaN
	// so you can test thisValue != thisValue
	thisValue = interpolatedRowLoCol + colFraction * (interpolatedRowHiCol - interpolatedRowLoCol);	
	return thisValue;
}



// interpolate a vector based on a value 'point' indexing an axis
double interpVector(const std::vector<double> &vector, 
	const std::vector<double> &axis, 
	const double point){
	int i;
	int len = (int)vector.size();
	if (len != (int)axis.size()) return DBL_MAX;
	if (point > axis[len - 1]) return vector[len - 1];  // flat extrapolation beyond longest  axis
	if (point < axis[0]) return vector[0];  // flat extrapolation before shortest axis
	for (i=0; point > axis[i] && i<len; i++) {}  // empty block...just getting "i" to the axis beyond point
	// linear interpolation for now
	double value;
	if (point == axis[i]) value = vector[i];
	else {
		double previousAxis   = axis[i - 1];
		double previousVector = vector[i - 1];
		double fraction       = (point - previousAxis) / (axis[i] - previousAxis);
		value                 = previousVector + fraction*(vector[i] - previousVector);
	}
	return value;
}




// ************** sundry functions
double calcRiskCategory(const std::vector<double> &buckets,const double scaledVol,const double start){
	double riskCategory(start);  
	int i, len;
	for (i = 1, len = (int)buckets.size(); i<len && scaledVol>buckets[i]; i++) { riskCategory += 1.0; }
	if (i != len) riskCategory += (scaledVol - buckets[i - 1]) / (buckets[i] - buckets[i - 1]);
	return(riskCategory);
}

enum { fixedPayoff = 1, callPayoff, putPayoff, twinWinPayoff, switchablePayoff, basketCallPayoff, lookbackCallPayoff, lookbackPutPayoff, basketPutPayoff, 
	basketCallQuantoPayoff, basketPutQuantoPayoff, cappuccinoPayoff, levelsCallPayoff, outperformanceCallPayoff, outperformancePutPayoff, varianceSwapPayoff, 
	autocallPutPayoff, autocallCallPayoff, lockinCallPayoff
};
enum { uFnLargest = 1, uFnLargestN, uFnSmallest };
enum { solveForCoupon, solveForPutBarrier,solveForLastCallCap, solveForDigital, solveForPositiveParticipation, solveForPositivePutParticipation, solveForShortPutStrike, solveForAutocallTrigger};





// *************** STRUCTS
typedef struct someCurve { double tenor, spread; } SomeCurve;


// *************** CLASSES

// performance instrumentation ... object in/out scope triggers timers
struct TimerData { long sumTime; long numCalls; };
std::map<std::string, TimerData> scopedTimers;
class ScopedTimer {
public:
	using ClockType = std::chrono::steady_clock;
	ScopedTimer(const std::string func)
		: function_name_( func ), start_( ClockType::now() ) {
		// int jj = 1;
	}
	ScopedTimer(const ScopedTimer&) = delete;
	ScopedTimer(ScopedTimer&&) = delete;
	auto operator=(const ScopedTimer&)->ScopedTimer& = delete;
	auto operator=(ScopedTimer&&)->ScopedTimer& = delete;
	~ScopedTimer() {
		using namespace std::chrono;
		auto stop = ClockType::now();
		auto duration = (stop - start_);
		auto ms = duration_cast<milliseconds>(duration).count();
		scopedTimers[function_name_].sumTime  += (long)ms;
		scopedTimers[function_name_].numCalls += 1;
		// std::cout << ms << " ms " << function_name_ << '\n';
	}

private:
	const std::string function_name_;
	const ClockType::time_point start_;
};


class MapType { 
public:
	MapType(const int id, const std::string name) : id(id),name(name) {}
	const int          id; 
	const std::string  name;
};

class MarketData {
public:
	MarketData(
		std::vector<std::vector<double>>                    &ulVolsTenor,
		std::vector< std::vector<std::vector<double>> >     &ulVolsStrike,
		std::vector< std::vector<std::vector<double>> >     &ulVolsImpVol,
		std::vector< std::vector<std::vector<double>> >     &ulVolsFwdVol,
		std::vector<std::vector<double>>                    &oisRatesTenor,
		std::vector<std::vector<double>>                    &oisRatesRate,
		std::vector<std::vector<double>>                    &divYieldsTenor,
		std::vector<std::vector<double>>                    &divYieldsRate,
		std::vector<std::vector<double>>                    &divYieldsStdErr,
		std::vector<std::vector<int>>                       &corrsOtherId,
		std::vector<std::vector<double>>                    &corrsCorrelation,
		std::vector<std::vector<int>>                       &fxcorrsOtherId,
		std::vector<std::vector<double>>                    &fxcorrsCorrelation
		) :ulVolsTenor(ulVolsTenor), ulVolsStrike(ulVolsStrike), ulVolsImpVol(ulVolsImpVol), ulVolsFwdVol(ulVolsFwdVol), oisRatesTenor(oisRatesTenor), oisRatesRate(oisRatesRate),
		divYieldsTenor(divYieldsTenor), divYieldsRate(divYieldsRate), divYieldsStdErr(divYieldsStdErr),corrsOtherId(corrsOtherId), corrsCorrelation(corrsCorrelation), fxcorrsOtherId(fxcorrsOtherId), fxcorrsCorrelation(fxcorrsCorrelation)
		{
		}
	std::vector< std::vector<double> >                  &ulVolsTenor;
	std::vector< std::vector<std::vector<double>> >     &ulVolsStrike;
	std::vector< std::vector<std::vector<double>> >     &ulVolsImpVol; 
	std::vector< std::vector<std::vector<double>> >     &ulVolsFwdVol;
	std::vector<std::vector<double>>                    &oisRatesTenor;
	std::vector<std::vector<double>>                    &oisRatesRate;
	std::vector<std::vector<double>>                    &divYieldsTenor;
	std::vector<std::vector<double>>                    &divYieldsRate;
	std::vector<std::vector<double>>                    &divYieldsStdErr;
	std::vector<std::vector<int>>                       &corrsOtherId;
	std::vector<std::vector<double>>                    &corrsCorrelation;
	std::vector<std::vector<int>>                       &fxcorrsOtherId;
	std::vector<std::vector<double>>                    &fxcorrsCorrelation;
	
};


class MyDB {
private:
	SQLHENV   hEnv;
	SQLHDBC   hDBC;
	SQLRETURN fsts;
	const int bufSize=256;
	SQLLEN    cbModel;		               // Model buffer bytes recieved
	HSTMT     hStmt;
	std::string dataSource, anyString;
	const std::string thisCommandLine;
	
public:
	char     **bindBuffer;
	// log an error
	int  logAnError(std::string errorString){
		char       lineBuffer[1000];
		std::replace(errorString.begin(), errorString.end(), '#', 'H');
		sprintf(lineBuffer, "%s%s%s%s%s", "insert into errors(Date,CommandString,ErrorString) values (now(),'", thisCommandLine.c_str(), "','", errorString.c_str(), "')");
		return(prepare((SQLCHAR *)lineBuffer, 1));
	}

	// makeDbConnection();
	bool makeDbConnection(){
		if (dataSource == "spCloud"){
			fsts = dbConn(hEnv, &hDBC, L"spCloud", L"anoble", L"Ragtin_Mor14_Lucian");
		}
		else if (dataSource == "newSp"){
			fsts =  dbConn(hEnv, &hDBC, L"newSp", L"root", L"ragtinmor");
		}
		else if (dataSource == "spArrow"){
			fsts =  dbConn(hEnv, &hDBC, L"spArrow", L"anoble", L"Ragtin_Mor14_Lucian");
		}
		else if (dataSource == "Arrow_AWS_Prod") {
			fsts =  dbConn(hEnv, &hDBC, L"Arrow_AWS_Prod", L"risk_engine_user", L"R1zk3gineUz3r");
		}
		else if (dataSource == "spLevendi") {
			fsts =  dbConn(hEnv, &hDBC, L"spLevendi", L"anoble", L"Ragtin_Mor14_Lucian");
		}
		else if (dataSource == "Arrow_AWS") {
			fsts =  dbConn(hEnv, &hDBC, L"Arrow_AWS", L"risk_engine_user", L"R!skEC1ie7");
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
		std::string msg,
		SQLHANDLE handle,
		SQLSMALLINT type)
	{
		SQLSMALLINT i = 0;
		SQLINTEGER  native;
		SQLWCHAR    state[7];
		SQLWCHAR    text[512];
		SQLSMALLINT len;
		SQLRETURN   ret;
		size_t      numChars;
		char        *cptr;
		fprintf(stderr,	"\n%s%s%s%s\n",	"IPRerror: Database problem running ",fn," ",msg.c_str());

		do	{
			ret = SQLGetDiagRec(type, handle, ++i, &state[0], &native, &text[0], (SQLSMALLINT) sizeof(text)/2, &len);
			if (SQL_SUCCEEDED(ret)) {
				cptr = WcharToChar(text, &numChars);
				// text[len] = '\0';
				// state[5] = '\0';
				// https://docs.microsoft.com/en-us/sql/odbc/reference/develop-app/sqlstates
				// https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/appendix-a-odbc-error-codes
				// eg 08S01	Communication link failure
				printf("%s%c%c%s%c%c%c:%d:%s\n", "SQLSTATE Class:", state[0], state[1], "SQLSTATE SUBClass:", state[2], state[3], state[4], native, cptr);
			}
			else{
				fprintf(stderr, "\nSQLGetDiagRec call failed\n");
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
		// ScopedTimer timer{ "dbConn" };

		SQLRETURN  fsts;
		int        numAttempts   = 0;

		fsts = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, hDBC);  // Allocate memory for the connection handle
		if (!SQL_SUCCEEDED(fsts))	{
			extract_error("SQLAllocHandle for dbc", "",hEnv, SQL_HANDLE_ENV);
			exit(1);
		}
		// Connect to Data Source  
		do {
			fsts         = SQLConnect(*hDBC, szDSN, SQL_NTS, szUID, SQL_NTS, szPasswd, SQL_NTS); // use SQL_NTS for length...NullTerminatedString			
			numAttempts += 1;
		} while (!SQL_SUCCEEDED(fsts) && numAttempts<10);
		if (!SQL_SUCCEEDED(fsts))	{
			char thisBuffer[200]; sprintf(thisBuffer, "SQLConnect for connect >>%ls<< UID >>%ls<< PWD >>%ls<<", szDSN, szUID, szPasswd);
			extract_error(thisBuffer, "", hDBC, SQL_HANDLE_DBC);
			// std::cerr << "Connection params " << szDSN << szUID << szPasswd << "\n";
			exit(104);
		}
		return fsts;
	}

	MyDB(const std::string thisCommandLine, char **bindBuffer, const std::string dataSource) : dataSource(dataSource), bindBuffer(bindBuffer), thisCommandLine(thisCommandLine) {
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
	int prepare(SQLCHAR* thisSQL,const int numCols) {
		// ScopedTimer timer{ "db prepare" };

		int numAttempts = 0;
		// DEBUG ONLY
		if ((int)strlen((char*)thisSQL)>MAX_SP_BUF){
			std::cerr << "String len:" << strlen((char*)thisSQL) << " will overflow\n";
			exit(102);
		}
		if (strstr((char*)thisSQL, "#")){
			anyString = (char *)thisSQL ;
			std::cerr << "SQL must not contain hash symbol: " << thisSQL << std::endl;
			logAnError(anyString);
			return(1);
		}

		if (hStmt != NULL) {
			SQLFreeStmt(hStmt, SQL_DROP);
		}
		do {
			fsts  =  SQLAllocStmt(hDBC, &hStmt); 	                        // Allocate memory for statement handle
			fsts  =  SQLPrepareA(hStmt, thisSQL, SQL_NTS);                  // Prepare the SQL statement	
			fsts  =  SQLExecute(hStmt);                                     // Execute the SQL statement
			if (!SQL_SUCCEEDED(fsts))	{
				extract_error("prepare() failed to SQLExecute ... trying to re-connect", (char*)thisSQL, hStmt, SQL_HANDLE_STMT);
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
		} while (!SQL_SUCCEEDED(fsts) && numAttempts<10);
		
		if (numAttempts >= 10) {
			std::cerr << "prepare() failed too many times with " << dataSource << " ...exiting\n";
			exit(104);
		};

		for (int i = 0; i < numCols;i++){
			SQLBindCol(hStmt, i+1, SQL_C_CHAR, bindBuffer[i], bufSize, &cbModel); // bind columns
		}
		return(0);
	}
	void bind(int col,char *buffer) {
		SQLBindCol(hStmt, col, SQL_C_CHAR, buffer, bufSize, &cbModel); // bind columns
	}
	SQLRETURN fetch(const bool checkForErrors,const std::string msg){
		// ScopedTimer timer{ "db fetch" };
		fsts = SQLFetch(hStmt);
		if (checkForErrors){
			if (fsts != SQL_SUCCESS && fsts != SQL_SUCCESS_WITH_INFO)	{ extract_error("SQLFetch", msg, hStmt, SQL_HANDLE_STMT);	return(MY_SQL_GENERAL_ERROR); }
		}
		return fsts;
	}
	SQLRETURN execute(bool checkForErrors, const std::string msg){
		// ScopedTimer timer{ "db execute" };
		fsts = SQLExecute(hStmt);
		if (checkForErrors){
			if (fsts != SQL_SUCCESS && fsts != SQL_SUCCESS_WITH_INFO)	{ extract_error("SQLExecute", msg, hStmt, SQL_HANDLE_STMT);	return(MY_SQL_GENERAL_ERROR); }
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
		amount(amount){
		};
	// std::string date;
	double amount;
};

class SpPayoffAndDate {
public:
	SpPayoffAndDate(std::string date, double amount) :
		date(date), 
		amount(amount){
	};
	std::string date;
	double amount;
};



class SpBarrierRelation {
private:
	const double thisBarrierBend;
	const double bendDirection;
	const bool   getMarketData, doForwardValueCoupons;
public:
	SpBarrierRelation(const int barrierRelationId, 
		const int underlying,
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
		const bool          isContinuousALL,
		const bool          isStrikeReset,
		const bool          isStopLoss,
		const double        thisBarrierBend,
		const double        bendDirection,
		const bool          getMarketData, 
		const bool          doForwardValueCoupons,
		const double        unBentStrike)
		: barrierRelationId(barrierRelationId), underlying(underlying), originalBarrier(_barrier), originalUbarrier(_uBarrier), isAbsolute(_isAbsolute),
		startDate(startDate), endDate(endDate), above(above), at(at), weight(weight), daysExtant(daysExtant),
		originalStrike(unadjStrike), avgType(avgType), avgDays(avgDays), avgFreq(avgFreq), avgInDays(avgInDays), avgInFreq(avgInFreq),
		avgInAlgebra(avgInAlgebra), isContinuousALL(isContinuousALL), isStrikeReset(isStrikeReset), isStopLoss(isStopLoss), thisBarrierBend(thisBarrierBend), bendDirection(bendDirection),
		getMarketData(getMarketData),doForwardValueCoupons(doForwardValueCoupons), unBentStrike(unBentStrike)
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
		if (!getMarketData && !doForwardValueCoupons && daysExtant <= 0) {
			// Rounding to the nearest multiple of 7, so that pre-strike dates are rounded to the nearest week
			endDays = (int)round(endDays / 7) * 7;
		}

		barrier        = originalBarrier;
		uBarrier       = originalUbarrier;
		strike         = originalStrike;
		int lastIndx((int)ulTimeseries.price.size() - 1);  // range-checked now so can use vector[] to access elements
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
			calcMoneyness(ulTimeseries.price[lastIndx] / ulTimeseries.price[lastIndx - (isStrikeReset && startDays<0 ? -startDays : daysExtant)]);
			// bumpSpots() can change the spot levels, so as to change barrier 'moneyness'
			// ... so we use originalMoneyness to remember this initial moneyness
			originalMoneyness = moneyness;

			// ...compute running averages
			// ... we are either inside an in-progress averaging (avgDays>endDays), or the entire averaging period is in the past (endDays<0)
			if (avgDays && avgDays > endDays){
				double refSpot(ulTimeseries.price[lastIndx]);
				setLevels(refSpot, false);
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
						case 2: // count
							avgWasHit.push_back(above ? (p>barrierLevel && (uBarrierLevel == NULL || p<uBarrierLevel) ? true : false) : (p<barrierLevel && (uBarrierLevel == NULL || p>uBarrierLevel) ? true : false));
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
	// SpBarrierRelation constructor

	// public members
	const bool             above, at, isAbsolute, isContinuousALL, isStrikeReset, isStopLoss;
	const int              barrierRelationId, underlying, avgType, avgDays, avgFreq, daysExtant;
	const double           unBentStrike,originalStrike,originalBarrier, originalUbarrier,weight;
	const std::string      startDate, endDate, avgInAlgebra;
	int                    count, j, k, startDays, endDays, runningAvgDays, avgInDays, avgInFreq, numAvgInSofar=0, countAvgInSofar=0, endDaysDiff=0;
	double                 avgInSofar=0.0, refLevel, barrier, uBarrier, barrierLevel, uBarrierLevel, strike, moneyness, originalMoneyness=1.0;
	bool                   readyForAvgObs;
	std::vector<double>    runningAverage;
	std::vector<bool>      avgWasHit;
	boost::gregorian::date bStartDate,bEndDate;
	std::vector<double>     theseAvgPrices;

	// calculate moneyness
	void calcMoneyness(const double thisMoneyness){
		// moneyness = 1.0 for barriers that are:
		// ... strikeReset in the future, or
		// ... a hack to accomodate #2774 where investor is short a strip of daily-reset KIPs struck at YESTERDAY's close
		moneyness    = isStrikeReset && (startDays>0 || isStopLoss) ? 1.0 : thisMoneyness ;
		strike       = originalStrike/moneyness;
	}

	// bump this brel by someDays
	void bumpSomeDays(const int someDays){
		endDays          +=  someDays;
		startDays        +=  someDays;
	}


	// do any averagingIn
	void doAveragingIn(const double ulPrice,   // prevailing (possibly simulated) spot level
						const int thisPoint,   // current product starting point in global timeseries
						const int lastPoint,   // last point in global timeseries
						const UlTimeseries&  ulTimeseries,
						std::vector<bool> &useUl
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
	void setLevels(const double ulPrice,const bool reverseBarrierBend) {
		refLevel  = ulPrice / moneyness;
		// NOTE: strikeReset barriers will also reset the barrierLevel, as follows
		//     ... if we need barriers to reference StrikeDate levels, will need to pass in 2nd arg    startLevels.at(thisName)    and use those
		double thisBarrier = reverseBarrierBend ? max(0.0, barrier - thisBarrierBend*bendDirection) : barrier;  // only bend barriers when doing fairValue
		barrierLevel       = thisBarrier * refLevel;
		if (uBarrier != NULL) { 
				uBarrierLevel     = uBarrier * refLevel; 
		}
	}
	// straightenBends
	void straightenBends() {
		strike = unBentStrike/originalMoneyness;
	}

};
// END class SpBarrierRelation



class SpBarrier {
private:
	const bool                  getMarketData,couponPaidOut,doFinalAssetReturn, doForwardValueCoupons;
	const bool                  doDebug;
	const int                   productId, debugLevel;
	const int                   barrierNum;
	const double                daysPerYear,fixedCoupon;
	const double                bendDirection;
	const std::vector <double>  &baseCurveTenor, &baseCurveSpread;
	const std::string           couponFrequency,productShape;
	const std::vector <double> &spots;
	MyDB                       &mydb;

public:
	SpBarrier(const int             barrierNum, 
		const int                   barrierId,
		const bool                  capitalOrIncome,
		const std::string           nature,
		double                      payoff,
		const std::string           settlementDate,
		const std::string           description,
		const std::string           payoffType,
		const int                   payoffTypeId,
		double                      strike,
		double                      cap,
		const int                   underlyingFunctionId,
		const double                param1,
		const double                participation,
		const std::vector<int>      ulIdNameMap,
		const int                   avgDays,
		const int                   avgType,
		const int	                avgFreq,
		const bool                  isMemory,
		const bool                  isAbsolute,
		const bool                  isStrikeReset,
		const bool                  isStopLoss,
		const bool                  isForfeitCoupons,
		const std::string            barrierCommands,
		const int                    daysExtant,
		const boost::gregorian::date bProductStartDate,
		const bool                   doFinalAssetReturn,
		const double                 midPrice,
		const double                 thisBarrierBend, 
		const double                 bendDirection,
		const std::vector<double>   &spots,
		const bool                  doDebug,
		const int                   debugLevel,
		const double                annualFundingUnwindCost,
		const int                   productId,
		MyDB                        &mydb,
		const double                fixedCoupon,
		const std::string           couponFrequency,
		const bool                  couponPaidOut,
		const std::vector <double>  baseCurveTenor,
		const std::vector <double>  baseCurveSpread,
		const std::string           productShape,
		const bool                  doForwardValueCoupons,
		const double                daysPerYear,
		const bool                  getMarketData,
		const double                unBentStrike, 
		const double                unBentCap, 
		const double                unBentParam1
		)
		: barrierNum(barrierNum), barrierId(barrierId), capitalOrIncome(capitalOrIncome), nature(nature), payoff(payoff),
		settlementDate(settlementDate), description(description), payoffType(payoffType),
		payoffTypeId(payoffTypeId), strike(strike),cap(cap), underlyingFunctionId(underlyingFunctionId),param1(param1),
		participation(participation), ulIdNameMap(ulIdNameMap),
		isAnd(nature == "and"), avgDays(avgDays), avgFreq(avgFreq), avgType(avgType),
		isMemory(isMemory), isAbsolute(isAbsolute), isStrikeReset(isStrikeReset), isStopLoss(isStopLoss), isForfeitCoupons(isForfeitCoupons), 
		barrierCommands(barrierCommands), daysExtant(daysExtant), bProductStartDate(bProductStartDate), doFinalAssetReturn(doFinalAssetReturn), 
		midPrice(midPrice), thisBarrierBend(thisBarrierBend), bendDirection(bendDirection), isCountAvg(avgType == 2 && avgDays), spots(spots), doDebug(doDebug),
		debugLevel(debugLevel), annualFundingUnwindCost(annualFundingUnwindCost),productId(productId),mydb(mydb),fixedCoupon(fixedCoupon),
		couponFrequency(couponFrequency),couponPaidOut(couponPaidOut), baseCurveTenor(baseCurveTenor), baseCurveSpread(baseCurveSpread),
		productShape(productShape), doForwardValueCoupons(doForwardValueCoupons), daysPerYear(daysPerYear),getMarketData(getMarketData),
		unBentStrike(unBentStrike), unBentCap(unBentCap), unBentParam1(unBentParam1)
	{
		init();
	};

	//
	// ******* SpBarrier.gmmConditionalExpectation()
	//
	double gmmConditionalExpectation(const double x) {
		// priors - the relative probs of obtaining this x under each cluster marginal
		Vec_IO_DP wts(gmmNumClusters);
		double    sumWts(0.0);
		for (int i=0; i < gmmNumClusters; i++) {
			double thisX = x - muX[i];
			double thisProb;
			thisProb =  a[i] * exp(-0.5 * (thisX*thisX)/covX[i]) / pow(2.0*PI*covX[i], 0.5);
			wts[i]   = thisProb;
			sumWts  += thisProb;
		}
		// conditional is weighted expectation of each cluster's expected y|x
		double thisExpectation(0.0);
		// inefficient, but useful in debugging to see the wts
		for (int i=0; i < gmmNumClusters; i++) {
			wts[i]  = wts[i] / sumWts;
		}
		for (int i=0; i < gmmNumClusters; i++) {
			// VERY unlikely for worstOf > 1.0 and long PUT position; more likely an unfortunate cluster, which will give too low a condExp
			double thisCovXY = x > 1.0 && covXY[i] < 0.0 ? 0.0 : covXY[i];
			double thisSlope = thisCovXY / covX[i];
			thisExpectation += wts[i] * ( muY[i]  +  thisSlope*(x - muX[i]) );
		}
		return(thisExpectation);
	}
	
	//
	// ******* SpBarrier.bumpSomeDays() ... bump this barrier by someDays
	//
	void bumpSomeDays(const int someDays){
		endDays          +=  someDays;
		startDays        +=  someDays;
		yearsToBarrier    = endDays / daysPerYear;
		totalBarrierYears = (endDays + daysExtant) / daysPerYear;
	}
	//
	// ******* SpBarrier.init()
	//
	void init(){
		using namespace boost::gregorian;
		date bEndDate(from_simple_string(settlementDate));
		endDays                  = (bEndDate - bProductStartDate).days() - daysExtant;
		if (!getMarketData && !doForwardValueCoupons && daysExtant <= 0) {
			// Rounding to the nearest multiple of 7, so that pre-strike dates are rounded to the nearest week
			endDays = (int)round(endDays / 7) * 7;
		}
		startDays                = endDays;
		yearsToBarrier           = endDays / daysPerYear;
		totalBarrierYears        = (endDays + daysExtant)/ daysPerYear;
		sumPayoffs               = 0.0;
		variableCoupon           = 0.0;
		isExtremum               = false;
		isContinuous             = false;
		isContinuousGroup        = false;
		hasBeenHit               = false;   // SProduct.evaluate(...,doAccruals=true,...) will check if this barrier 'hasBeenHit' post-strike
		proportionHits           = 1.0;
		sumProportion            = 0.0;
		fixedCouponValue         = 0.0;
		forwardRate              = 1.0 + interpCurve(baseCurveTenor, baseCurveSpread, yearsToBarrier); // DOME: very crude for now
		if (couponFrequency.size()) {  // add fixed coupon
			int    freqLen      = (int)couponFrequency.length();
			char   freqChar     = toupper(couponFrequency[freqLen - 1]);
			std::string freqNumber = couponFrequency.substr(0, freqLen - 1);
			sprintf(charBuffer, "%s", freqNumber.c_str());
			double couponEvery  = atof(charBuffer);
			double daysPerEvery = freqChar == 'D' ? 1 : freqChar == 'M' ? 30 : daysPerYear;
			double daysElapsed  = (bEndDate - bProductStartDate).days();
			double couponPeriod = daysPerEvery * couponEvery;
			if (couponPaidOut) {
				daysElapsed  -= floor(daysExtant / couponPeriod)*couponPeriod; // floor() so as to include accrued stub
			}
			double numFixedCoupons = max(0.0, /*floor*/(daysElapsed / couponPeriod)); // allow fractional coupons
			double periodicRate    = exp(log(forwardRate) * (couponPeriod / daysPerYear));
			double effectiveNumCoupons = (pow(periodicRate, numFixedCoupons) - 1) / (periodicRate - 1);
			fixedCouponValue = fixedCoupon * (couponPaidOut && doForwardValueCoupons ? effectiveNumCoupons : numFixedCoupons);
		}

		isLargestN = underlyingFunctionId == uFnLargestN && payoffTypeId == fixedPayoff;
		// FIXED PAYOFFs ONLY: will be hit if the N bestPerforming underlyings are above their respective barriers
		if (isLargestN){			
			isHit = &SpBarrier::isHitLargestN;
		}
		else if (payoffType.find("basket") != std::string::npos){
			isHit = &SpBarrier::isHitBasket;
		}
		else if (payoffType.find("outperf") != std::string::npos){
			isHit = &SpBarrier::isHitOutperf;
		}
		else if (payoffType.find("fixed") != std::string::npos && productShape == "AutocallHimalaya") {
			isHit = &SpBarrier::isHitAutocallHimalaya;
		}
		else {
			isHit = &SpBarrier::isHitVanilla;
		}
		proportionalAveraging  = avgDays > 0 && avgType == 1;
		countAveraging         = avgDays > 0 && avgType == 2;
		brel.reserve(10);
		if (doFinalAssetReturn){ fars.reserve(100000); }
		hit.reserve(100000);
	};
	//
	// ******* END: SpBarrier.init()
	//

	//
	// SpBerrier.straightenBends()
	// ... only getMarketData/userParams bends barriers
	// ... so if we want to switch to non-FV evaluation of barriers, we need to unbend things
	void straightenBends() {
		strike = unBentStrike;
		cap    = unBentCap;
		param1 = unBentParam1;
	}


	// public members: DOME consider making private, in case we implement their content some other way
	std::vector<double>             a;        // GMM  mix    for each cluster
	std::vector<double>             muX;      // GMM  muX    for each cluster
	std::vector<double>             muY;      // GMM  muY    for each cluster
	std::vector<double>             covXY;    //f GMM  covXY  for each cluster
	std::vector<double>             covX;     // GMM  covX   for each cluster
	int                             gmmNumClusters;
	char                            charBuffer[1000];
	std::vector<double>             worstUlRegressionPrices;     // worst underlying prices if issuerCallable	
	std::vector<double>             nextWorstUlRegressionPrices; // next worst underlying prices if issuerCallable	
	const int                       barrierId, payoffTypeId, underlyingFunctionId, avgDays, avgFreq, avgType, daysExtant;
	const bool                      capitalOrIncome, isAnd, isMemory, isAbsolute, isStrikeReset, isStopLoss, isForfeitCoupons, isCountAvg;
	double                          unBentCap, unBentParam1, unBentStrike;
	const double                    thisBarrierBend,midPrice;
	const std::string               nature, settlementDate, description, payoffType, barrierCommands;
	const std::vector<int>          ulIdNameMap;
	const boost::gregorian::date    bProductStartDate;
	bool                            hasBeenHit, isExtremum, isContinuous, isContinuousGroup, proportionalAveraging, countAveraging, isLargestN;
	int                             numHitsAtCheckpoint,maxEndDays,startDays,endDays, numStrPosPayoffs=0, numPosPayoffs=0, numNegPayoffs=0;
	double                          param1, thisCouponValue, fixedCouponValue,maxYears,annualFundingUnwindCost, payoff, variableCoupon, strike, participation,cap, totalBarrierYears,yearsToBarrier, sumPayoffs, sumStrPosPayoffs=0.0, sumPosPayoffs=0.0, sumNegPayoffs=0.0;
	double                          lsConstant, lsWorstB, lsWorstSquaredB, lsNextWorstB, lsNextWorstSquaredB, proportionHits, totalNumPossibleHits=0.0, sumProportion, forwardRate, discountFactor;
	bool                            (SpBarrier::*isHit)(const int thisMonPoint, const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels, std::vector<bool> &useUl);
	std::vector <finalAssetInfo>    fars; // final asset returns
	std::vector <double>            bmrs; // benchmark returns
	std::vector <SpBarrierRelation> brel;
	std::vector <SpPayoff>          hit;
	std::vector <SpPayoffAndDate>   hitWithDate;
	std::vector <double>            couponValues;    // storePayoff() does .pushBack()
	std::vector <bool>              payoffContainsVariableCoupons;
	const int numLRMrhs             = (int)spots.size() > 1 ? 5 : 3;
	const bool aimForHeadlineAnnRet = !getMarketData && !doForwardValueCoupons;
	// number of days until barrier end date
	int getEndDays() const { return endDays; }

	

	//
	// ******* SpBarrier.isNeverHit()  
	//       ... issuerCallable LEARNING_PHASE needs to turn off non-terminal Capital barriers
	//       ... so, .isHit() initially set to isNeverHit for the burnInIterations
	//       ... after burInIterations we set .isHit() to isCallableHit() which does the call-or-continue logic
	// 
	bool isNeverHit(const int thisMonPoint, const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels, std::vector<bool> &useUl
	) {
		int j;
		int numBrel = (int)brel.size();
		// ... some products have zero brels eg simple maturity barrier (no attached barrier relations)
		if (numBrel == 0) return false;
		// ... otherwise, find the worst_ and nextWorst_ brel returns
		std::vector<double> tempPrices; 
		for (j = 0; j < numBrel; j++) {
			const SpBarrierRelation &thisBrel(brel[j]);
			tempPrices.push_back(thesePrices[j] / thisBrel.refLevel);    // normalize prices to avoid overflow on matrix inversion
		}
		if (numBrel > 1){			
			sort(tempPrices.begin(), tempPrices.end());
			worstUlRegressionPrices.push_back(tempPrices[0]);
			nextWorstUlRegressionPrices.push_back(tempPrices[1]);
		}
		else{
			worstUlRegressionPrices.push_back(tempPrices[0]);
		}
		return(false);
	}
	
	//
	// ******* SpBarrier.isCallableHit()
	//       ... does the call-or-continue logic
	//       ... after burinInIterations we set .isHit() to isCallableHit()
	// 
	bool isCallableHit(const int thisMonPoint, const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels, std::vector<bool> &useUl
	) {
		double conditionalExpectation(0.0);
		double thisWorst, nextWorst;
		const int numPrices = (int)thesePrices.size();
		const int numBrel   = (int)brel.size();
		// some products have zero brels eg simple maturity barrier (no attached barrier relations)
		if (numBrel == 0) return true;
		std::vector<double> tempPrices;

		// form dependent variables x
		for (int j = 0; j < numBrel; j++) {
			const SpBarrierRelation &thisBrel(brel[j]);
			tempPrices.push_back(thesePrices[j] / thisBrel.refLevel);    // normalize prices to avoid overflow on matrix inversion
		}
		if (numPrices > 1) {
			sort(tempPrices.begin(), tempPrices.end());
			thisWorst  = tempPrices[0];
			nextWorst  = tempPrices[1];
		}
		else {
			thisWorst = tempPrices[0];
		}

		// calculate expected continuation value
		if (USE_GMM_CLUSTERS) {
			// Gaussian Mixture Model ... the conditionalExpectation of y for this x
			conditionalExpectation  = gmmConditionalExpectation(thisWorst);
		}
		else {
			// linear y = b.x + c.x^2   + CONSTANT
			conditionalExpectation = (lsConstant + thisWorst * lsWorstB + thisWorst * thisWorst*lsWorstSquaredB);
			if (numPrices > 1) {
				conditionalExpectation  += nextWorst * lsNextWorstB + nextWorst * nextWorst*lsNextWorstSquaredB;
			}
		}
		// check for early exercise
		double thisPayoff = payoff + fixedCouponValue + thisCouponValue;
		bool   isCalled(false);
		double thisFundingUnwindCost = annualFundingUnwindCost * (maxYears - (daysExtant + endDays) / daysPerYear);
		// Issuer should rationally either:
		//   a) pay thisPayoff NOW if cheaper than letting it continue at a value of conditionalExpectation 
		//   b) continue, since conditionalExpectation expected to e cheaper than paying thisPayoff NOW
		if (conditionalExpectation > (thisPayoff + thisFundingUnwindCost)){
			isCalled = true;
		}
		if (doDebug  && debugLevel == 4 ) {
			sprintf(charBuffer, "%s%d%s%d%s%lf%s%lf%s%lf%s%d%s",
				"insert into conditionalexpectation (ProductId,BarrierId,X1,Y,Payoff,Called) values (",
				productId, ",",
				barrierNum, ",",
				thisWorst, ",",
				conditionalExpectation, ",",
				thisPayoff, ",",
				isCalled ? 1:0, ");");
			mydb.prepare((SQLCHAR *)charBuffer, 1);
		}
		return (isCalled);
	}
	
	//
	// ******* SpBarrier.setIsNeverHit()
	//        ... IssuerCallable barriers initialise .isHit() to .isNeverHit()
	// 
	void setIsNeverHit(){
		if ((capitalOrIncome && endDays < maxEndDays && endDays > 0)
			// || (!b.capitalOrIncome && b.endDays == maxEndDays)
			) {
			worstUlRegressionPrices.clear();
			nextWorstUlRegressionPrices.clear();
			worstUlRegressionPrices.reserve((int)(MAX_CALLABLE_ITERATIONS*CALLABLE_REGRESSION_FRACTION));
			nextWorstUlRegressionPrices.reserve((int)(MAX_CALLABLE_ITERATIONS*CALLABLE_REGRESSION_FRACTION));
			isHit = &SpBarrier::isNeverHit;
		}
	}
	//
	// ******* SpBarrier.setIsCallableHit()
	//        ... IssuerCallable barriers, after burnInIterations, set .isHit() to .isCallableHit()
	// 
	void setIsCallableHit(){
		isHit = &SpBarrier::isCallableHit;
	}

	//
	// ******* SpBarrier.isHitVanilla()
	// 
	// VANILLA test if barrier is hit
	// ...set useUlMap to false if you have more thesePrices than numUnderlyings...for example product #217 has barriers with brels keyed to the same underlying
	//     so that a barrier can have multiple barrierRelations on the same underlying
	bool isHitVanilla(const int thisMonPoint, const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels, std::vector<bool> &useUl
	) {
		int j, thisIndx;
		bool isHit  = isAnd;
		int numBrel = (int)brel.size();  // could be zero eg simple maturity barrier (no attached barrier relations)
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


	//
	// ******* SpBarrier.isHitAutocallHimalaya()
	// 
	// AutocallHimalaya test if barrier is hit
	// ... underlyings that hit barrier are Memoried
	bool isHitAutocallHimalaya(const int thisMonPoint, const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels, std::vector<bool> &useUl) {
		int j, thisIndx;
		bool isHit  = isAnd;
		int numBrel = (int)brel.size();  // could be zero eg simple maturity barrier (no attached barrier relations)
		if (numBrel == 0) return true;
		std::string word;

		for (j = 0; j < numBrel; j++) {
			const SpBarrierRelation &thisBrel(brel[j]);
			thisIndx    = useUlMap ? ulIdNameMap[thisBrel.underlying] : j;
			bool   above;          above       = thisBrel.above;
			double thisUlPrice;    thisUlPrice = thesePrices[thisIndx];
			// if barrierRelation ends on a different date...
			if (thisBrel.endDaysDiff != 0) { thisUlPrice = ulPrices[thisIndx].price.at(thisMonPoint - thisBrel.endDaysDiff); }
			double diff;           diff        = thisUlPrice - thisBrel.barrierLevel;
			bool   thisTest;       thisTest    = above ? diff > 0 : diff < 0;
			//std::cout << j << "Diff:" << diff << "Price:" << thisUlPrice << "Barrier:" << thisBrel.barrierLevel << std::endl;
			if (thisBrel.uBarrier != NULL) {
				diff     = thisUlPrice - thisBrel.uBarrierLevel;
				thisTest &= above ? diff < 0 : diff>0;
			}
			if (isAnd) {
				// memory of thisTest
				if (thisTest) {
					useUl[thisIndx] = false;
				}
				if (!useUl[thisIndx]) {
					thisTest = true;
				}
				isHit &= thisTest;
			}
			else        isHit |= thisTest;
		}
		//std::cout << "isHit:" << isHit << "Press a key to continue..." << std::endl;  std::getline(std::cin, word);
		return isHit;
	};


	//
	// ******* SpBarrier.isHitLargestN()
	// 
	// LargestN test if barrier is hit
	//  e.g barrier is hit if the best 3 underlyings are above their respective barrierLevels
	//  ... for which param1 should be set to 4
	//  ... so that, once theseReturns are sorted in DESCENDING order, we set hurdle return nthLargestReturn = theseReturns[3]
	//  ... and only use (marking them with activeBrels(bool)) those brels whose returns are > nthLargestReturn
	//
	// ...set useUlMap to false if you have more thesePrices than numUnderlyings...for example product #217 has barriers with brels keyed to the same underlying
	//     so that a barrier can have multiple barrierRelations on the same underlying
	bool isHitLargestN(const int thisMonPoint, const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels, std::vector<bool> &useUl) {
		int j, thisIndx;
		bool isHit  = isAnd;
		double nthLargestReturn;
		int numBrel = (int)brel.size();  // could be zero eg simple maturity barrier (no attached barrier relations)
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
		nthLargestReturn = theseReturns[param1 <= 0 ? 0 : (unsigned int)param1 - 1];
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


	//
	// ******* SpBarrier.isHitBasket()
	// 
	// basket test if barrier is hit
	// ... to performance-weight basket, use underlyingFunctionId == uFnLargestN
	bool isHitBasket(const int thisMonPoint, const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels, std::vector<bool> &useUl) {
		int numBrel = (int)brel.size();  // could be zero eg simple maturity barrier (no attached barrier relations)
		if (numBrel == 0) return true;
		int j, thisIndx;
		bool isHit  = isAnd;
		bool above  = brel[0].above;
		double w;

		// ** if basket performance-weighted, set performances() and performanceBasedWeights()
		std::vector<double> performanceBasedWeights;
		std::vector<double> performances;
		if (underlyingFunctionId == uFnLargestN){
			for (j=0; j<numBrel; j++) {
				const SpBarrierRelation &thisBrel(brel[j]);
				thisIndx       = useUlMap ? ulIdNameMap[thisBrel.underlying] : j;
				performanceBasedWeights.push_back(thisBrel.weight);
				performances.push_back(thesePrices[thisIndx] / thisBrel.refLevel);
			}
			sort(performances.begin(), performances.end(), std::greater<double>()); // sort DECENDING
			sort(performanceBasedWeights.begin(), performanceBasedWeights.end(), std::greater<double>()); // sort DECENDING
		}

		/*
		* see if basket return breaches barrier level (in 'N' field)
		*/
		double basketReturn = 0.0;
		for (j = 0; j<numBrel; j++) {
			// performance-weighted?
			if ((int)performances.size()>0){
				basketReturn  += performances[j] * performanceBasedWeights[j];
			}
			// static-weighted
			else{
				const SpBarrierRelation &thisBrel(brel[j]);
				thisIndx       = useUlMap ? ulIdNameMap[thisBrel.underlying] : j;
				above          = thisBrel.above;
				w              = thisBrel.weight;
				// thisRefLevel   = startLevels[thisIndx] / thisBrel.moneyness;
				basketReturn  += (thesePrices[thisIndx] / thisBrel.refLevel) * w;
			}
		}
		// test weighted return vs param1
		double diff      = basketReturn - param1;
		bool   thisTest  = above ? diff > 0 : diff < 0;
		if (isAnd)  isHit &= thisTest;
		else        isHit |= thisTest;
		//std::cout << "isHit:" << isHit << "Press a key to continue..." << std::endl;  std::getline(std::cin, word);
		return isHit;
	};


	//
	// ******* SpBarrier.isHitOutperf()
	// 
	// outperf test if barrier is hit
	bool isHitOutperf(const int thisMonPoint, const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels, std::vector<bool> &useUl
	) {
		int numBrel = (int)brel.size();  // could be zero eg simple maturity barrier (no attached barrier relations)
		if (numBrel < 2) return true;
		int j, thisIndx;
		bool isHit  = isAnd;
		bool above  = brel[0].above;

		/*
		* see if return difference breaches barrier level (in 'N' field)
		*/
		double returnDiff = 0.0;
		for (j = 0; j<2; j++) {
			const SpBarrierRelation &thisBrel(brel[j]);
			thisIndx       = useUlMap ? ulIdNameMap[thisBrel.underlying] : j;
			above          = thisBrel.above;
			// thisRefLevel   = startLevels[thisIndx] / thisBrel.moneyness;
			returnDiff  += (thesePrices[thisIndx] / thisBrel.refLevel) * (j == 0 ? 1.0 : -1.0);
		}
		double diff      = 1.0 + returnDiff - param1;
		bool   thisTest  = above ? diff > 0 : diff < 0;
		if (isAnd)  isHit &= thisTest;
		else        isHit |= thisTest;
		//std::cout << "isHit:" << isHit << "Press a key to continue..." << std::endl;  std::getline(std::cin, word);
		return isHit;
	};





	// VANILLA test if barrier is hit
	// ...set useUlMap to false if you have more thesePrices than numUnderlyings...for example product #217 has barriers with brels keyed to the same underlying
	//     so that a barrier can have multiple barrierRelations on the same underlying
	bool isHitOld(const int thisMonPoint, const std::vector<UlTimeseries> &ulPrices, const std::vector<double> &thesePrices, const bool useUlMap, const std::vector<double> &startLevels, std::vector<bool> &useUl
	) {
		int j, thisIndx;
		bool isHit  = isAnd;
		double thisRefLevel, nthLargestReturn;
		int numBrel = (int)brel.size();  // could be zero eg simple maturity barrier (no attached barrier relations)
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
			nthLargestReturn = theseReturns[param1 <= 0.0 ? 0 : (unsigned int)param1 - 1];
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

	
	//
	// ******* SpBarrier.getPayoff()
	// 
	// get payoff
	double getPayoff(const std::vector<double> &startLevels,
		std::vector<double>                    &lookbackLevel,
		const std::vector<double>              &thesePrices,
		const double                            amc, 
		const std::string                       productShape,
		const bool                             doFinalAssetReturn,
		double                                 &finalAssetReturn,
		double                                 &optionPayoff,
		int                                    &finalAssetIndx,
		const std::vector<int>                 &ulIds,
		std::vector<bool>                      &useUl,
		const int                               thisPoint,
		std::vector<UlTimeseries>              &ulPrices,
		const double                            lockedIn) {
		double              thisPayoff(payoff), p, thisRefLevel, thisFinalLevel, thisAssetReturn, thisStrike;
		double              cumReturn,w, basketFinal, basketStart, basketRef;
		std::vector<double> basketPerfs,basketWeights,uPerfs,optionPayoffs; optionPayoffs.reserve(10);
		int                 callOrPut = -1, j, k, len,n;     				// default option is a put

		// init
		optionPayoff = 0.0;

		switch (payoffTypeId) {
		case callPayoff:
		case levelsCallPayoff:
		case lookbackCallPayoff:
		case twinWinPayoff:
		case autocallCallPayoff:
		case lockinCallPayoff:
			callOrPut = 1;
		case lookbackPutPayoff:
		case putPayoff:
		case autocallPutPayoff:
			for (j = 0, len = (int)brel.size(); j<len; j++) {
				const SpBarrierRelation &thisBrel(brel[j]);
				n      = ulIdNameMap[thisBrel.underlying];
				// following line changed as we now correctly do SpBarrierRelation initialisation from strike(strike) to strike(unadjStrike)
				// ...previously strike /= moneyness was only affecting the parameter value! and not the member variable
				// thisStrike = thisBrel.strike * startLevels[n] / thisBrel.moneyness;
				if (payoffTypeId == levelsCallPayoff){
					thisRefLevel = 1.0;
					thisStrike   = thisBrel.originalStrike;
				}
				else {
					thisRefLevel = thisBrel.refLevel;  // startLevels[n] / thisBrel.moneyness;
					thisStrike   = thisBrel.strike * (isStrikeReset && (thisBrel.startDays>0 || isStopLoss) ? thisRefLevel : startLevels[n]);
				}
				if (payoffTypeId == lookbackCallPayoff || payoffTypeId == lookbackPutPayoff) {
					thisAssetReturn = lookbackLevel[n] / thisRefLevel;
				}
				else {
					thisAssetReturn = thesePrices[n] / thisRefLevel;
				}
				double thisSpotReturn   = thesePrices[n] / startLevels[n];
				if (finalAssetReturn > thisSpotReturn)  { 
					finalAssetReturn = thisSpotReturn; 
					finalAssetIndx   = thisBrel.underlying;
				}

				// the typical optionPayoff = max(0,return) is done below in the 'for' loops initialised with 'optionPayoff=0'
				// if (payoffTypeId == putPayoff && (productShape == "Autocall" || productShape == "Phoenix")){
				if (payoffTypeId == autocallPutPayoff || payoffTypeId == autocallCallPayoff){
					p = callOrPut*(thisAssetReturn*thisRefLevel / thisStrike - 1);
				}
				else {
					p = callOrPut*(thisAssetReturn - thisStrike / thisRefLevel);
				}
				if (payoffTypeId == lockinCallPayoff && p < lockedIn){
					p = lockedIn;
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
					for (bestUlReturn=(double)-DBL_MAX, optionPayoff=0.0, j=0, len=(int)optionPayoffs.size(); j<len; j++) {
						if (useUl[ulIdNameMap[brel[j].underlying]]){
							if (optionPayoffs[j] > optionPayoff) { optionPayoff = optionPayoffs[j]; }
							if (optionPayoffs[j] > bestUlReturn) { bestUlIndx   = j; bestUlReturn = optionPayoffs[j]; }
						}
					}
					useUl[ulIdNameMap[brel[bestUlIndx].underlying]] = false;
				}
				else {
					if (underlyingFunctionId == uFnSmallest){
						for (optionPayoff = DBL_MAX, j = 0, len = (int)optionPayoffs.size(); j<len; j++) {
							if (optionPayoffs[j] < optionPayoff) { optionPayoff = optionPayoffs[j]; }
						}
						if (optionPayoff < 0.0){
							optionPayoff = 0.0;
						} 
					}
					else {
						for (optionPayoff = 0.0, j = 0, len = (int)optionPayoffs.size(); j<len; j++) {
							if (optionPayoffs[j] > optionPayoff) { optionPayoff = optionPayoffs[j]; }
						}
					}
				}
				break;
			case uFnLargestN: {
				// weighted average of best N=param1 optionPayoffs
				double avgNpayoff(0.0);
				sort(optionPayoffs.begin(), optionPayoffs.end(), std::greater<double>()); // sort DECENDING
				for (optionPayoff=0, j=0, len=(int)param1; j<len; j++){
					double brelWeight = brel[j].weight;
					avgNpayoff += optionPayoffs[j] * (productShape == "Rainbow" || brelWeight != 0.0 ? param1*brelWeight : 1.0);
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
			// collect each underlying returns
			basketFinal = 0.0;
			for (j = 0, len = (int)brel.size(); j < len; j++)	{
				const SpBarrierRelation &thisBrel(brel[j]);
				n     = ulIdNameMap[thisBrel.underlying];
				thisRefLevel     = thisBrel.refLevel;  // startLevels[n] / thisBrel.moneyness;
				basketPerfs.push_back(thesePrices[n] / thisRefLevel);
				basketWeights.push_back(thisBrel.weight);
			}
			//  maybe sort DECREASING
			if (productShape == "Rainbow" || underlyingFunctionId == uFnLargestN){
				sort(basketPerfs.begin(),   basketPerfs.end(),   std::greater<double>()); // sort DECENDING
				sort(basketWeights.begin(), basketWeights.end(), std::greater<double>()); // sort DECENDING
			}
			// compute basket weighted return
			for (j=0, len = (int)brel.size(); j<len; j++) {
				const SpBarrierRelation &thisBrel(brel[j]);
				basketFinal   += basketPerfs[j] * basketWeights[j];
			}
			// compute payoff vs strike
		   finalAssetReturn = basketFinal;
		   optionPayoff     = callOrPut *(basketFinal - strike);
		   if (optionPayoff > cap){ optionPayoff = cap; }
		   thisPayoff      += participation*(optionPayoff > 0.0 ? optionPayoff : 0.0);
		   break;
		case cappuccinoPayoff:
			callOrPut = 1;
			cumReturn = 0.0;
			w          = 1.0 / (double)brel.size();
			basketFinal=0.0, basketStart=0.0, basketRef=0.0;
			for (j=0, len = (int)brel.size(); j<len; j++) {
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
		case outperformanceCallPayoff:
			callOrPut = 1;
		case outperformancePutPayoff:
			// first MINUS second
			if (brel.size() == 2){
				for (j=0; j<2; j++) {
					const SpBarrierRelation &thisBrel(brel[j]);
					n              = ulIdNameMap[thisBrel.underlying];
					thisRefLevel   = startLevels[n];
					thisFinalLevel = thesePrices[n];
					uPerfs.push_back(thisFinalLevel / thisRefLevel);
				}
				optionPayoff     = callOrPut *(uPerfs[0] - uPerfs[1]) - strike;
				if (optionPayoff > cap){ optionPayoff = cap; }
				thisPayoff      += participation*(optionPayoff > 0.0 ? optionPayoff : 0.0);
			}			
			break;
		case fixedPayoff:
			if (doFinalAssetReturn){
				// DOME: just record worstPerformer for now
				finalAssetReturn = 1.0e9;
				for (j = 0, len = (int)ulIds.size(); j<len; j++)	{
					int    n     = ulIdNameMap[ulIds[j]];
					double perf  = thesePrices[n] / startLevels[n] ;
					if (perf < finalAssetReturn){ finalAssetReturn = perf; }
				}
			}
			break;
		case varianceSwapPayoff:
			const SpBarrierRelation &thisBrel(brel[0]);
			callOrPut        = 1;
			n                = ulIdNameMap[thisBrel.underlying];
			std::vector<double> thisPriceSlice, thisPriceLevel;
			for (k=thisPoint + thisBrel.startDays; k <= thisPoint + thisBrel.endDays; k++){
				if (!ulPrices[n].nonTradingDay[k]){
					double thisPrice = ulPrices[n].price[k];
					thisPriceSlice.push_back(log(thisPrice));
					thisPriceLevel.push_back(thisPrice);
				}
			}
			double thisVariance;
			bool   param1IsZero  = param1 == 0.0;
			double lowerBound    = thisBrel.refLevel * param1;
			for (thisVariance=0.0, k=1; k < (int)thisPriceSlice.size(); k++){
				if (param1IsZero || thisPriceLevel[k] > lowerBound){
					double anyDouble = thisPriceSlice[k] - thisPriceSlice[k - 1];
					thisVariance += anyDouble*anyDouble;
				}
			}
			thisVariance      = 252.0 * thisVariance / ((double)thisPriceSlice.size() - 1);
			double swapPayoff = (thisVariance - strike*strike) / (2.0*strike);
			if (fabs(swapPayoff) > cap){ swapPayoff = cap * (swapPayoff < 0.0 ? -1.0 : 1.0); }
			thisPayoff      += participation*swapPayoff;

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

	//
	// ******* SpBarrier.storePayoff()
	// 
	void storePayoff(const std::string thisDateString, const double amount, const double couponValue, const double proportion, 
		const double finalAssetReturn, const int finalAssetIndx, const int barrierIndx, const bool doFinalAssetReturn, const double benchmarkReturn, 
		const bool storeBenchmarkReturn, const bool doAccruals, const bool containsVariableCoupons, const int thisIteration){
		
		sumPayoffs     += amount;
		if (amount >  midPrice){ sumStrPosPayoffs += amount; numStrPosPayoffs++; }
		if (amount >= midPrice){ sumPosPayoffs    += amount; numPosPayoffs++; }
		else{                    sumNegPayoffs    += amount; numNegPayoffs++; }
		sumProportion  += proportion;
		if (doAccruals) { 
			hitWithDate.push_back(SpPayoffAndDate(thisDateString, amount)); 
		}
		else {            
			hit.push_back( SpPayoff(thisDateString, amount) );
			if (aimForHeadlineAnnRet) {
				payoffContainsVariableCoupons.push_back(containsVariableCoupons);
			}
		}
		couponValues.push_back(couponValue);
		if (doFinalAssetReturn){ fars.push_back(finalAssetInfo(finalAssetReturn,finalAssetIndx,barrierIndx)); }
		if (storeBenchmarkReturn){ 
			if (benchmarkReturn<0.0){
				int jj = 1;
			}
			bmrs.push_back(benchmarkReturn);
		}
	}


	//
	// ******* SpBarrier.doAveraging()
	// 
	// do any averaging
	void doAveraging(const std::vector<double> &startLevels, std::vector<double> &thesePrices, std::vector<double> &lookbackLevel, const std::vector<UlTimeseries> &ulPrices,
		const int thisPoint, const int thisMonPoint, const int numUls, std::vector<bool> &useUl
	) {
		int j,k,len;

		// averaging OUT
		if (avgDays && brel.size()) {
			switch (avgType) {
			case 0: //averageLevels
				for (j = 0, len = (int)brel.size(); j < len; j++) {
					const SpBarrierRelation& thisBrel = brel[j];
					int n = ulIdNameMap[thisBrel.underlying];
					double thisStartLevel = startLevels[n];
					std::vector<double> avgObs;
					// create DECREASING DATE ORDER (most recent is at index 0) avgObs vector of observations
					for (k = 0; k < (int)thisBrel.runningAverage.size(); k++) {
						avgObs.push_back(thisBrel.runningAverage[k] * thisStartLevel);
					}
					for (k = 0; k <= (avgDays - thisBrel.runningAvgDays) && k < thisMonPoint; k++) {
						if (k%avgFreq == 0){
							while (k < thisMonPoint && (ulPrices.at(n).nonTradingDay.at(thisMonPoint - k))){ k++; };
							avgObs.push_back(ulPrices.at(n).price.at(thisMonPoint - k));
						}
					}

					// calculate some value for these observations
					// hacky: if there is no averagingIn use any algebra here
					if (thisBrel.avgInDays == 0 && avgObs.size() && thisBrel.avgInAlgebra.length()) {
						// convert to returns
						//  NOTE avgObs is in DECREASING DATE ORDER
						std::vector<double> thesePerfs;
						for (int k = (int)avgObs.size() - 1; k>=0; k--) { thesePerfs.push_back(avgObs[k] / thisBrel.refLevel); }
						double thisAlgebraLevel =	evalAlgebra(thesePerfs, thisBrel.avgInAlgebra);
						lookbackLevel[n] = thisAlgebraLevel * thisBrel.refLevel;
					}
					else if (payoffType == "lookbackCall"){
						double anyValue(0.0);
						anyValue = *max_element(avgObs.begin(), avgObs.end());
						// DOME remove for loop if it gives the same value
						for (int k = 0, len1 = (int)avgObs.size(); k<len1; k++) { double anyValue1 = avgObs[k]; if (anyValue1>anyValue){ anyValue = anyValue1; } }
						lookbackLevel[n] = anyValue;
					}
					else if (payoffType == "lookbackPut"){
						double anyValue(DBL_MAX);
						anyValue = *min_element(avgObs.begin(), avgObs.end());
						// DOME remove for loop if it gives the same value
						for (int k = 0, len1 = (int)avgObs.size(); k<len1; k++) { double anyValue1 = avgObs[k]; if (anyValue1<anyValue){ anyValue = anyValue1; } }
						lookbackLevel[n] = anyValue;
					}
					else {
						double anyValue(0.0);
						for (int k = 0, len1 = (int)avgObs.size(); k < len1; k++) { anyValue += avgObs[k]; }
						double thisAvg = anyValue / (double)avgObs.size();
						double deleteAnytime = startLevels[0];
						thesePrices[n] = thisAvg;
					}
				}
				break;
			case 1: // proportion
			case 2: // count
				double numHits(0.0), numPossibleHits(0.0);
				if (daysExtant){
					for (int i = 0, numObs = (int)brel[0].avgWasHit.size(); i < numObs; i++) {
						bool wasHit = true;
						for (j = 0, len = (int)brel.size(); j < len; j++) {
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
						for (j = 0, len = (int)brel.size(); j < len; j++) {
							SpBarrierRelation thisBrel = brel[j];
							int n = ulIdNameMap[thisBrel.underlying];
							testPrices.at(n) = ulPrices.at(n).price.at(thisMonPoint - k);
						}
						numHits +=  (this ->* (this->isHit))(thisMonPoint, ulPrices, testPrices, true, startLevels,useUl) ? 1 : 0;
						numPossibleHits += 1;
					}
				}
				proportionHits        = numHits;
				totalNumPossibleHits += numPossibleHits;
				if (avgType == 1) { proportionHits /= numPossibleHits; }
				break;
			}
		}
	}
};
// END class SpBarrier




// 
// public: PayoffMean - calculate average Capital payoff, across all barriers
//
double PayoffMean(const std::vector<SpBarrier> &barrier){
	int                 numInstances(0);
	double              sumPayoffs(0.0);
	for (int thisBarrier = 0; thisBarrier < (int)barrier.size(); thisBarrier++){
		const SpBarrier&    b(barrier.at(thisBarrier));
		if (b.capitalOrIncome) {
			numInstances    += (int)b.hit.size();
			for (int i = 0; i < (int)b.hit.size(); i++){
				sumPayoffs += b.hit[i].amount;
			}
		}
	}
	return (sumPayoffs / numInstances);
}

// 
// public: PayoffStdev - calculate standard deviation of all Capital payoffs, across all barriers
//
double PayoffStdev(const std::vector<SpBarrier> &barrier, const double mean){
	int                 numInstances(0);
	double              sumVariance(0.0);
	for (int thisBarrier = 0; thisBarrier < (int)barrier.size(); thisBarrier++){
		const SpBarrier&    b(barrier.at(thisBarrier));
		if (b.capitalOrIncome) {
			numInstances    += (int)b.hit.size();
			for (int i = 0; i < (int)b.hit.size(); i++){
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
	const int                       productId;
	const int                       userId;
	const std::string               productCcy,thisCommandLine;
	const std::vector <bool>        &allNonTradingDays;
	const std::vector <bool>        &ulFixedDivs;
	const std::vector <double>      &spots;
	const std::vector <double>      &strikeDateLevels;
	const std::vector <int>         &ulIds;
	const std::vector<std::string>  &ulNames;
	const std::vector <std::string> &allDates;
	const boost::gregorian::date    bProductStartDate,&bLastDataDate;
	const int                       bootstrapStride, daysExtant, productIndx;
	const double                    daysPerYear,volShift,gmmMinClusterFraction,compoIntoCcyStrikePrice, benchmarkStrike, fixedCoupon, AMC, midPrice, askPrice, fairValue, baseCcyReturn;
	const std::string               productShape;
	const bool                      doTurkey,hasCompoIntoCcy, localVol, doBumps, silent, updateProduct, verbose, doBootstrapStride, forOptimisation, saveOptimisationPaths, fullyProtected, validFairValue, depositGteed, collateralised, couponPaidOut, showMatured, forceIterations;
	const std::vector<SomeCurve>    baseCurve;
	postStrikeState                 thisPostStrikeState;
	const bool                      extendingPrices;
	const bool                      issuerCallable;
	const bool                      multiIssuer;
	const double                    targetReturn;
	const bool                      doForwardValueCoupons;
public:
	SProduct(
		const bool                      extendingPrices,
		const std::string               thisCommandLine,
		MyDB                           &mydb,
		char                           *lineBuffer,
		const boost::gregorian::date    &bLastDataDate,
		const int                       productId,
		const int                       userId,
		const std::string               productCcy,
		const UlTimeseries              &baseTimeseies,
		const boost::gregorian::date    bProductStartDate,
		const double                    fixedCoupon,
		const std::string               couponFrequency, 
		const bool                      couponPaidOut,
		const double                    AMC, 
		const bool                      showMatured,
		const std::string				productShape,
		const bool                      fullyProtected,
		const double                    benchmarkStrike,
		const bool                      depositGteed, 
		const bool                      collateralised,
		const int                       daysExtant,
		const double                    midPrice,
		const std::vector<SomeCurve>    baseCurve,
		const std::vector<int>          &ulIds,
		const double                    forwardStartT,
		const double                    issuePrice,
		const std::string               ukspaCase,
		const bool                      doPriips,
		const std::vector<std::string>  &ulNames,
		const bool                      validFairValue, 
		const double                    fairValue, 
		const double                    askPrice,
		const double                    baseCcyReturn,
		const std::vector<double>       &shiftPrices,
		const std::vector<bool>         &doShiftPrices,
		const bool                      forceIterations,
		std::vector<std::vector<std::vector<double>>> &optimiseMcLevels,
		std::vector<int>                &optimiseUlIdNameMap,
		const bool                      forOptimisation, 
		const bool                      saveOptimisationPaths,
		const int                       productIndx,
		const double                    bmSwapRate, 
		const double                    bmEarithReturn,
		const double                    bmVol,
		const double                    cds5y,
		const int                       bootstrapStride,
		const int                       settleDays,
		const bool                      silent,
		const bool                      updateProduct,
		const bool                      verbose,
		const bool                      doBumps,
		const bool                      stochasticDrift,
		const bool                      localVol,
		const std::vector<bool>         &ulFixedDivs,
		const double                    compoIntoCcyStrikePrice, 
		const bool                      hasCompoIntoCcy,
		const bool                      issuerCallable,
		const std::vector<double>       &spots,
		const std::vector<double>       &strikeDateLevels,
		const double                    gmmMinClusterFraction,
		const bool                      multiIssuer,
		std::vector<double>             &cdsVols,
		const double                    volShift,
		const double                    targetReturn,
		const bool                      doForwardValueCoupons,
		const double                    daysPerYear,
		const bool                      doTurkey
		)
		: extendingPrices(extendingPrices), thisCommandLine(thisCommandLine), mydb(mydb), lineBuffer(lineBuffer), bLastDataDate(bLastDataDate), productId(productId), userId(userId), productCcy(productCcy), allDates(baseTimeseies.date),
		allNonTradingDays(baseTimeseies.nonTradingDay), bProductStartDate(bProductStartDate), fixedCoupon(fixedCoupon),	couponFrequency(couponFrequency), 
		couponPaidOut(couponPaidOut), AMC(AMC), showMatured(showMatured), productShape(productShape), fullyProtected(fullyProtected), 
		benchmarkStrike(benchmarkStrike), depositGteed(depositGteed), collateralised(collateralised),
		daysExtant(daysExtant), midPrice(midPrice), baseCurve(baseCurve), ulIds(ulIds), forwardStartT(forwardStartT), issuePrice(issuePrice), 
		ukspaCase(ukspaCase), doPriips(doPriips), ulNames(ulNames), validFairValue(validFairValue), fairValue(fairValue), askPrice(askPrice), baseCcyReturn(baseCcyReturn),
		shiftPrices(shiftPrices), doShiftPrices(doShiftPrices), forceIterations(forceIterations), optimiseMcLevels(optimiseMcLevels),
		optimiseUlIdNameMap(optimiseUlIdNameMap), forOptimisation(forOptimisation), saveOptimisationPaths(saveOptimisationPaths), productIndx(productIndx), bmSwapRate(bmSwapRate),
		bmEarithReturn(bmEarithReturn), bmVol(bmVol), cds5y(cds5y), bootstrapStride(bootstrapStride),
		settleDays(settleDays), doBootstrapStride(bootstrapStride != 0), silent(silent), updateProduct(updateProduct), verbose(verbose), doBumps(doBumps), stochasticDrift(stochasticDrift),
		localVol(localVol), ulFixedDivs(ulFixedDivs), compoIntoCcyStrikePrice(compoIntoCcyStrikePrice), hasCompoIntoCcy(hasCompoIntoCcy), issuerCallable(issuerCallable), 
		spots(spots), strikeDateLevels(strikeDateLevels), gmmMinClusterFraction(gmmMinClusterFraction), multiIssuer(multiIssuer),cdsVols(cdsVols),volShift(volShift), 
		targetReturn(targetReturn), doForwardValueCoupons(doForwardValueCoupons), daysPerYear(daysPerYear), doTurkey(doTurkey){
	
		for (int i=0; i < (int)baseCurve.size(); i++) { baseCurveTenor.push_back(baseCurve[i].tenor); baseCurveSpread.push_back(baseCurve[i].spread); }
	};

	// public members: DOME consider making private
	MyDB                           &mydb;
	char                           *lineBuffer;
	const unsigned int              longNumOfSequences=1000;
	bool                            doPriips, notUKSPA, stochasticDrift;
	int                             addCompoIntoCcy, numIncomeBarriers, settleDays, maxProductDays, productDays, numUls, maxEndDays;
	double                          oncurveVol,cds5y,bmSwapRate, bmEarithReturn, bmVol, forwardStartT, issuePrice, priipsRfr;
	std::string                     couponFrequency,ukspaCase;
	std::vector <SpBarrier>         barrier;
	std::vector <bool>              useUl,doShiftPrices;
	std::vector <double>            &cdsVols,fixedDiv,priipsStressVols,baseCurveTenor, baseCurveSpread,randnosStore,shiftPrices;
	std::vector<boost::gregorian::date> allBdates;
	std::vector<SpPayoffAndDate>        storeFixedCoupons;
	std::vector<std::vector<std::vector<double>>> &optimiseMcLevels;
	std::vector<int>                    &optimiseUlIdNameMap;
	std::vector<int>                reportableMonDateIndx;
	
	//
	// ******* SProduct.GenerateCorrelatedNormal()
	//
	void GenerateCorrelatedNormal(const int numUnderlyings,
		std::vector<double>                    &correlatedRandom,
		const std::vector<std::vector<double>> &cholMatrix,
		std::vector<double>                    &normalRandom,
		const bool                             useAntithetic,
		const int                              antitheticRow,
		std::vector<std::vector<double>>       &antitheticRandom,
		const bool                             conserveRands,
		const bool                             consumeRands,
		int                                    &randnoIndx,
		std::vector<double>                    &randnosStore) {
		int    j, k;
		double anyDouble;
		// get correlated standard Normal shocks
		for (j = 0; j<numUnderlyings; j++){
			if (useAntithetic) {
				normalRandom[j] = -antitheticRandom[antitheticRow][j];
			}
			else {
				double thisRandno;
				if (consumeRands){
					thisRandno = randnosStore[randnoIndx++];
				}
				else{
					thisRandno = /*(double)rand() / RAND_MAX*/ ArtsRan();
					if (conserveRands){
						randnosStore.push_back(thisRandno);
					}
				}
				normalRandom[j]   = NormSInv(thisRandno);
				antitheticRandom[antitheticRow][j] = normalRandom[j];
			}
		}
		for (j = 0; j<numUnderlyings; j++){
			anyDouble = 0.0;
			for (k = 0; k<numUnderlyings; k++){
				anyDouble = anyDouble + normalRandom[k] * cholMatrix[k][j];
			}
			correlatedRandom[j] = anyDouble;
		}
	}
	//
	// ******* END: SProduct.GenerateCorrelatedNormal()
	//



	// ************
	// ******* randomNumber stuff
	// ************
	// ... for occasional use, seeing what random numbers are actually being generated
	std::vector<unsigned int> randomNumbers;
	unsigned int reportSomeRandomNumbersCounter = 0;
	bool         reportSomeRandomNumbersDone    = false;
	void ReportSomeRandomNumbers(const unsigned int thisRandomNumber){		
		if (!reportSomeRandomNumbersDone){
			if (reportSomeRandomNumbersCounter++ > 5){
				for (int i=0; i < randomNumbers.size(); i++){
					std::cerr << "randNo:" << randomNumbers[i] << std::endl;
				}
				reportSomeRandomNumbersDone = true;
			}
			else {
				randomNumbers.push_back(thisRandomNumber);
			}
		}
	}
	// 
	// ******* SProduct.ArtsRan()
	//
	ArtsRandomNumber artsRandomNumber;
	double ArtsRan(){
		static long unsigned int numTimesCalled;
		// numTimesCalled += 1;
		static const double normalizer(1.0 / ARTS_MAX_RAND);
		artsRandomNumber.bucket = artsRandomNumber.bucket * 1664525 + 1013904223;
		// ReportSomeRandomNumbers(artsRandomNumber.bits[0]);
		return(artsRandomNumber.bits[0] * normalizer);
	}
	void ArtsRanInit() { artsRandomNumber.bucket = 0; }






	// 
	// ******* SProduct.init()
	//
	void init(const double maxYears){
		// ...prebuild all dates outside loop
		int numAllDates = (int)allDates.size();	allBdates.reserve(numAllDates);
		for (int thisPoint = 0; thisPoint < numAllDates; thisPoint += 1) { allBdates.push_back(boost::gregorian::from_simple_string(allDates.at(thisPoint))); }

		numUls = (int)ulIds.size();
		for (int i=0; i < numUls; i++){ useUl.push_back(true); fixedDiv.push_back(0.0);  }
		barrier.reserve(100); // for more efficient push_back

		// PRIIPs init
		if (doPriips){
			// drifts and discounting all at the rfr for the RecommendedHoldingPeriod, assumed to be max term
			priipsRfr  = interpCurve(baseCurveTenor, baseCurveSpread, maxYears);
		}

		// UKSPA init
		notUKSPA = ukspaCase == "";

		// sundry
		addCompoIntoCcy  =  hasCompoIntoCcy ? 1 : 0;
		int numBarriers = (int)barrier.size();
		for (int j=0; j < numBarriers; j++) {
			SpBarrier& b(barrier.at(j));
			b.maxYears = maxYears;
		}
	}

	// 
	// ******* SProduct.resetBarriers()  ... re-initialise barriers
	// 
	void resetBarriers(){
		numIncomeBarriers = 0;
		int numBarriers = (int)barrier.size();
		for (int j=0; j < numBarriers; j++){
			SpBarrier& b(barrier.at(j));

			if (!b.capitalOrIncome){ numIncomeBarriers += 1; }
			// clear un-accrued hits ... where we call evaluate() several times eg PRIIPs and PRIIPsStresstest, or doing bumps
			if (!b.hasBeenHit){
				//
				// all these member variables get set anytime storePayoff() is called
				//
				b.sumProportion     = 0.0;
				b.sumPayoffs        = 0.0;
				b.sumStrPosPayoffs  = 0.0;
				b.numStrPosPayoffs  = 0;
				b.sumPosPayoffs     = 0.0;
				b.numPosPayoffs     = 0;
				b.sumNegPayoffs     = 0.0;
				b.numNegPayoffs     = 0;
				b.hitWithDate.clear();
				b.hit.clear();
				b.payoffContainsVariableCoupons.clear();
				b.couponValues.clear();
				b.fars.clear();
				b.bmrs.clear();
			}
		}
	}

	// 
	// ******* SProduct.solverSet()  ... set some product param
	// 
	void solverSet(const int solveForThis,const double paramValue){
		int numBarriers = (int)barrier.size();
		bool putBarrierFound(false),lastCapFound(false), digitalFound(false), positiveParticipationFound(false), positivePutParticipationFound(false), shortPutStrikeFound(false);
		double previousBarrierYears(0.0);
		switch (solveForThis){
		case solveForCoupon:
			// set each coupon to an annualised rate			
			for (int j=0; j < numBarriers; j++){
				SpBarrier& b(barrier.at(j));
				if (!b.capitalOrIncome || (numIncomeBarriers == 0 && b.payoffTypeId == fixedPayoff && b.brel.size()>0)){
					// capitalBarrier: coupon times CUMULATIVE years; incomeBarriers: coupon times INTERVAL years
					b.payoff  =  (b.capitalOrIncome ? 1.0 : 0.0) + paramValue * (b.capitalOrIncome ? b.totalBarrierYears : (b.totalBarrierYears - previousBarrierYears));
					previousBarrierYears = b.totalBarrierYears;
				}
			}
			break;
		case solveForAutocallTrigger:
			// look for ALL capitalOrIncome obs with payoffType == 'fixed' and hasBrels
			for (int j=0; j < numBarriers; j++) {
				SpBarrier& b(barrier.at(j));
				int numBrels = (int)b.brel.size();
				if (b.capitalOrIncome && b.payoffTypeId == fixedPayoff && numBrels > 0) {
					for (int k=0; k < numBrels; k++) {
						b.brel[k].barrier = paramValue;
					}
				}
			}
			break;
		case solveForLastCallCap:
			// set lastCap
			for (int j=numBarriers - 1; !lastCapFound && j >= 0; j--) {
				SpBarrier& b(barrier.at(j));
				if ((b.payoffTypeId == callPayoff || b.payoffTypeId == basketCallPayoff) && b.participation > 0.0 && (int)b.brel.size() > 0 && b.cap > 0.0 && b.strike > 0.0) {
					lastCapFound = true;
					b.cap        = paramValue;
				}
			}
			break;
		case solveForPutBarrier:
			// set put barrier
			for (int j=0; !putBarrierFound && j < numBarriers; j++) {
				SpBarrier& b(barrier.at(j));
				int numBrels = (int)b.brel.size();
				if (b.capitalOrIncome && (b.payoffTypeId == putPayoff || b.payoffTypeId == basketPutPayoff) && b.participation < 0.0 && (int)b.brel.size()>0) {
					switch (b.payoffTypeId) {
					case putPayoff:
						putBarrierFound    = true;
						for (int k=0; k < numBrels; k++) {
							b.brel[k].barrier = paramValue;
						}
						break;
					case basketPutPayoff:
						putBarrierFound    = true;
						b.param1           = paramValue;;
						break;
					}
				}
			}
			break;
		case solveForDigital:
			// look for productShape == digital && last payoffType == 'fixed' and hasBrels
			for (int j=numBarriers - 1; !digitalFound && j >= 0; j--) {
				SpBarrier& b(barrier.at(j));
				if (productShape == "Digital" &&  b.payoffTypeId == fixedPayoff && (int)b.brel.size() > 0) {
					digitalFound = true;
					b.payoff     = paramValue;
				}
			}
			break;
		case solveForPositiveParticipation:
			// look for LAST participation > 0 && payoffType != 'fixed' and hasBrels
			for (int j=numBarriers - 1; !positiveParticipationFound && j >= 0; j--) {
				SpBarrier& b(barrier.at(j));
				if (b.payoffTypeId != fixedPayoff && b.participation > 0.0 && (int)b.brel.size() > 0) {
					positiveParticipationFound = true;
					b.participation = paramValue;
				}
			}
			break;
		case solveForPositivePutParticipation:
			// look for LAST participation > 0 && payoffType == 'put' and hasBrels
			for (int j=numBarriers - 1; !positivePutParticipationFound && j >= 0; j--) {
				SpBarrier& b(barrier.at(j));
				if (b.payoffTypeId == putPayoff && b.participation > 0.0 && (int)b.brel.size() > 0) {
					positivePutParticipationFound = true;
					b.participation = paramValue;
				}
			}
			break;
		case solveForShortPutStrike:
			// look for LAST participation < 0 && payoffType == 'put' and hasBrels
			// IMPORTANT:  MUST also set brel.strikes
			for (int j=numBarriers - 1; !shortPutStrikeFound && j >= 0; j--) {
				SpBarrier& b(barrier.at(j));
				int numBrels = (int)b.brel.size();
				if (b.payoffTypeId == putPayoff && b.participation < 0.0 && numBrels > 0) {
					shortPutStrikeFound = true;
					b.strike            = paramValue;
					b.cap               = paramValue;
					for (int k=0; k < numBrels; k++) {
						b.brel[k].strike  = paramValue;
						b.brel[k].barrier = paramValue;
					}
				}
			}
			break;
		} // switch
	}

	// 
	// ******* SProduct.solverCommit()  ... commit solver solution to DB
	// 
	void solverCommit(const int solveForThis, const double paramValue){
		solverSet(solveForThis, paramValue);
		int numBarriers = (int)barrier.size();
		bool putBarrierFound(false),lastCapFound(false), digitalFound(false), positiveParticipationFound(false), positivePutParticipationFound(false), shortPutStrikeFound(false);
		switch (solveForThis){
		case solveForCoupon:
			// set each coupon to an annualised rate
			for (int j=0; j < numBarriers; j++){
				SpBarrier& b(barrier.at(j));
				if (!b.capitalOrIncome || (numIncomeBarriers == 0 && b.payoffTypeId == fixedPayoff && b.brel.size()>0)){
					sprintf(lineBuffer, "%s%.5lf%s", "update productbarrier set Payoff='", 100.0*b.payoff,"%'");
					sprintf(lineBuffer, "%s%s%d%s", lineBuffer, " where ProductBarrierId='", b.barrierId, "'");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
				}
			}
			break;
		case solveForAutocallTrigger:
			// look for ALL capitalOrIncome obs with payoffType == 'fixed' and hasBrels
			for ( int j=0; j < numBarriers; j++) {
				SpBarrier& b(barrier.at(j));
				int numBrels = (int)b.brel.size();
				if (b.capitalOrIncome && b.payoffTypeId == fixedPayoff && numBrels > 0) {
					for (int k=0; k < numBrels; k++) {
						SpBarrierRelation& br(b.brel[k]);
						sprintf(lineBuffer, "%s%.5lf%s", "update barrierrelation set Barrier='", br.barrier, "'");
						sprintf(lineBuffer, "%s%s%d%s", lineBuffer, " where BarrierRelationId='", br.barrierRelationId, "'");
						mydb.prepare((SQLCHAR *)lineBuffer, 1);
					}
				}
			}
			break;
		case solveForLastCallCap:
			// set lastCap
			for (int j=numBarriers - 1; !lastCapFound && j >= 0; j--) {
				SpBarrier& b(barrier.at(j));
				if ((b.payoffTypeId == callPayoff || b.payoffTypeId == basketCallPayoff) && b.participation > 0.0 && (int)b.brel.size() > 0 && b.cap > 0.0 && b.strike > 0.0) {
					lastCapFound = true;
					sprintf(lineBuffer, "%s%.5lf%s", "update productbarrier set Cap='", b.cap, "'");
					sprintf(lineBuffer, "%s%s%d%s", lineBuffer, " where ProductBarrierId='", b.barrierId, "'");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
				}
			}
			break;
		case solveForPutBarrier:
			// set put barrier
			for (int j=0; !putBarrierFound && j < numBarriers; j++){
				SpBarrier& b(barrier.at(j));
				int numBrels = (int)b.brel.size();
				if (b.capitalOrIncome && (b.payoffTypeId == putPayoff || b.payoffTypeId == basketPutPayoff) && b.participation < 0.0 && (int)b.brel.size()>0) {
					switch (b.payoffTypeId) {
					case putPayoff:
						putBarrierFound    = true;
						for (int k=0; k < numBrels; k++) {
							SpBarrierRelation& br(b.brel[k]);
							sprintf(lineBuffer, "%s%.5lf%s", "update barrierrelation set Barrier='", br.barrier, "'");
							sprintf(lineBuffer, "%s%s%d%s", lineBuffer, " where BarrierRelationId='", br.barrierRelationId, "'");
							mydb.prepare((SQLCHAR *)lineBuffer, 1);
						}
						break;
					case basketPutPayoff:
						putBarrierFound    = true;
						sprintf(lineBuffer, "%s%.5lf%s", "update productbarrier set Param1='", b.param1, "%'");
						sprintf(lineBuffer, "%s%s%d%s", lineBuffer, " where ProductBarrierId='", b.barrierId, "'");
						mydb.prepare((SQLCHAR *)lineBuffer, 1);
						break;
					}
				}
			}
			break;
		case solveForDigital:
			// look for productShape == digital && last payoffType == 'fixed' and hasBrels
			for (int j=numBarriers - 1; !digitalFound && j >= 0; j--) {
				SpBarrier& b(barrier.at(j));
				if (productShape == "Digital" &&  b.payoffTypeId == fixedPayoff && (int)b.brel.size() > 0) {
					digitalFound = true;
					sprintf(lineBuffer, "%s%.5lf%s", "update productbarrier set Payoff='", 100.0*b.payoff, "%'");
					sprintf(lineBuffer, "%s%s%d%s", lineBuffer, " where ProductBarrierId='", b.barrierId, "'");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
				}
			}
			break;
		case solveForPositiveParticipation:
			// look for LAST participation > 0 && payoffType != 'fixed' and hasBrels
			for (int j=numBarriers - 1; !positiveParticipationFound && j >= 0; j--) {
				SpBarrier& b(barrier.at(j));
				if (b.payoffTypeId != fixedPayoff && b.participation > 0.0 && (int)b.brel.size() > 0) {
					positiveParticipationFound = true;
					sprintf(lineBuffer, "%s%.5lf%s", "update productbarrier set Participation='", b.participation, "'");
					sprintf(lineBuffer, "%s%s%d%s", lineBuffer, " where ProductBarrierId='", b.barrierId, "'");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
				}
			}
			break;
		case solveForPositivePutParticipation:
			// look for LAST participation > 0 && payoffType == 'put' and hasBrels
			for (int j=numBarriers - 1; !positivePutParticipationFound && j >= 0; j--) {
				SpBarrier& b(barrier.at(j));
				if (b.payoffTypeId == putPayoff && b.participation > 0.0 && (int)b.brel.size() > 0) {
					positivePutParticipationFound = true;
					sprintf(lineBuffer, "%s%.5lf%s", "update productbarrier set Participation='", b.participation, "'");
					sprintf(lineBuffer, "%s%s%d%s", lineBuffer, " where ProductBarrierId='", b.barrierId, "'");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
				}
			}
			break;
		case solveForShortPutStrike:
			// look for LAST participation < 0 && payoffType == 'put' and hasBrels
			for (int j=numBarriers - 1; !shortPutStrikeFound && j >= 0; j--) {
				SpBarrier& b(barrier.at(j));
				int numBrels = (int)b.brel.size();
				if (b.payoffTypeId == putPayoff && b.participation < 0.0 && numBrels > 0) {
					shortPutStrikeFound = true;
					sprintf(lineBuffer, "%s%.5lf%s%.5lf%s", "update productbarrier set Strike='", b.strike, "',Cap='",b.cap,"'");
					sprintf(lineBuffer, "%s%s%d%s", lineBuffer, " where ProductBarrierId='", b.barrierId, "'");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
					for (int k=0; k < numBrels; k++) {
						SpBarrierRelation& br(b.brel[k]);
						sprintf(lineBuffer, "%s%.5lf%s", "update barrierrelation set Barrier='", br.barrier, "'");
						sprintf(lineBuffer, "%s%s%d%s", lineBuffer, " where BarrierRelationId='", br.barrierRelationId, "'");
						mydb.prepare((SQLCHAR *)lineBuffer, 1);
					}
				}
			}
			break;
		} // switch
	}

	// 
	// ******* SProduct.logAnError()  ... log an error
	// 
	int  logAnError(const std::string errorString){
		sprintf(lineBuffer, "%s%s%s%s%s", "insert into errors(Date,CommandString,ErrorString) values (now(),'", thisCommandLine.c_str(),"','", errorString.c_str(), "')");
		mydb.prepare((SQLCHAR *)lineBuffer, 1);
		return(0);
	}







	// ***********************
	// evaluate product at this point in time
	// ***********************
	EvalResult evaluate(const int totalNumDays,
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
		const std::vector<double> monDateT, 
		const double              recoveryRate,
		const std::vector<std::vector<double>> hazardCurves,
		MyDB                      &mydb,
		double                    &accruedCoupon,
		const bool                doAccruals,
		const bool                doFinalAssetReturn,
		const bool                doDebug,
		const int                 debugLevel,
		const time_t              startTime,
		const int                 benchmarkId,
		const double              benchmarkMoneyness,
		const double              contBenchmarkTER,
		const double              hurdleReturn,
		const bool                doTimepoints, 
		const bool                doPaths,
		const std::vector<int>    timepointDays,
		const std::vector<std::string> timepointNames,
		const std::vector<double> simPercentiles,
		const bool                doPriipsStress,
		const char                *useProto,
		bool                      getMarketData, 
		const bool                useUserParams,
		const MarketData          &md,
		const std::vector<std::vector<double>> &cdsTenors,
		const std::vector<std::vector<double>> &cdsSpreads,
		const double              fundingFraction,
		const bool                productNeedsFullPriceRecord,
		const bool                ovveridePriipsStartDate,
		double                    &thisFairValue,
		const bool                conserveRands,
		const bool                consumeRands,
		bool                      &productHasMatured,
		const bool                priipsUsingRNdrifts,
		bool                      updateCashflows,
		const int                 issuerIndx
		){
		// ScopedTimer timer{ "evaluate " + std::to_string(numMcIterations) };
		char                     charBuffer[1000];
		EvalResult               evalResult(0.0, 0.0, 0);
		std::vector<bool>		 barrierDisabled;
		const int                optMaxNumToSend = 1000;
		const double             unwindPayoff    = 0.000000001; // avoid zero as is forces CAGR to -1.0 which is probably unreasonable, except for a naked option strategy
		const int                numBurnInIterations = (int)(issuerCallable ? numMcIterations * CALLABLE_REGRESSION_FRACTION * (doTurkey ? 0.5 : 1.0): 0);
		int                      startBurnInIteration(0);
		int                      stopBurnInIteration(numBurnInIterations);
		const int                numLRMrhs = numUls > 1 ? 5 : 3;
		const int                halfNumMcIterations(doAccruals ? 1 : numMcIterations / 2);  // C++ discards any remaider
		bool                     optFirstTime;
		bool	                 optOptimiseAnnualisedReturn(!getMarketData); 
		bool                     matured(false);
		bool                     usingProto(strcmp(useProto,"proto") == 0);
		int                      totalNumReturns  = totalNumDays - 1;
		int                      numTimepoints    = (int)timepointDays.size();
		int                      randnoIndx       =  0;
		int                      optCount         = 0;
		int                      i, j, k, m, n, len, thisIteration, maturityBarrier;
		double                   simulatedFairValue,thisAmount,couponValue(0.0), stdevRatio(1.0), stdevRatioPctChange(100.0);
		std::string              anyString;
		boost::gregorian::date   bFixedCouponsDate(bLastDataDate);
		std::vector< std::vector<double> > simulatedLogReturnsToMaxYears(numUl);
		std::vector< std::vector<double> > simulatedReturnsToMaxYears(numUl);
		std::vector< std::vector<double> > simulatedLevelsToMaxYears(numUl);
		std::vector<double>      callableCashflows; if (issuerCallable) { callableCashflows.reserve(numBurnInIterations); }  // .push_back() in burnIn iterations
		std::vector<double>      stdevRatioPctChanges;
		std::vector<double>      optimiseProductResult;   
		std::vector<double>      optimiseProductPayoff;   
		std::vector<double>      optimiseProductDuration; 
		std::vector<int>         optimiseDayMatured;
		if (forOptimisation) {
			optimiseProductResult.reserve(numMcIterations);
			optimiseProductPayoff.reserve(numMcIterations);
			optimiseProductDuration.reserve(numMcIterations);
			optimiseDayMatured.reserve(numMcIterations);
		}
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
		

		// ***************
		// ******* init
		// ***************
		// form totalVariance surfaces, for use in thisSig InterpolateMatrix ... give a more correct time-interpolation of volSurfaces
		std::vector< std::vector<  std::vector<double>> >  totalVariance;
		if ((getMarketData || useUserParams) && (numMcIterations > 1)) {
			for (i=0; i < numUl; i++) {
				int numStrikes = (int)md.ulVolsStrike[i][0].size();
				int numTenors  = (int)md.ulVolsTenor[i].size();
				std::vector<std::vector<double>>  someVolSurface(numTenors, std::vector<double>(numStrikes));
				for (j=0; j < numTenors; j++) {
					double thisT = md.ulVolsTenor[i][j];
					std::vector<double>  someImpVol;
					for (k=0; k < numStrikes; k++) {
						double thisVol         = md.ulVolsImpVol[i][j][k];
						someVolSurface[j][k]   = thisVol * thisVol * thisT;
					}
				}
				totalVariance.push_back(someVolSurface);
			}
		}

		if (!doAccruals){
			resetBarriers();
			for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
				SpBarrier& b(barrier.at(thisBarrier));
				// MUST recalc discountRate as b.yearsToBarrier may have changed when bumpTheta  b.bumpSomeDays()
				double thisDiscountRate   = b.forwardRate + fundingFraction*interpCurve(cdsTenors[issuerIndx], cdsSpreads[issuerIndx], b.yearsToBarrier);
				b.discountFactor = pow(thisDiscountRate, -(b.yearsToBarrier - forwardStartT));

				if (!barrier.at(thisBarrier).capitalOrIncome) { numIncomeBarriers  += 1; }
			}
		}
		
		std::vector<int>   numCouponHits(numIncomeBarriers+1);
		for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
			barrierDisabled.push_back(false);
		}


		// we do not need to pre-build the samples (uses too much memory, and we may not need all the samples anyway if there is convergence)
		// ... we imagine a virtual balancedResampling scheme
		// ... with N samples of length p we have the Np sequence 0,1,...,p,0,1,...p   
		// ... we have an integer "nPpos" indexing this Np vector and every time we want a random sample we choose index=rand()*(nPpos--) which is notionally in the index/p "block"
		// ... but since all the blocks are the same, we just use element index % p from the single block 0,1,...,p
		// ... ta-daa!!
		const unsigned int firstObs = 0;
		unsigned long int maxNpPos = longNumOfSequences*(totalNumReturns - firstObs);  // if maxNpPos is TOO large then sampling from its deceasing value is tantamount to no balancedResampling as the sample space essentially never shrinks materially
		unsigned long int npPos    = maxNpPos;
		// faster to put repeated indices in a vector, compared to modulo arithmetic, and we only need manageable arrays eg 100y of daily data is 36500 points, repeated 1000 - 36,500,000 which is well within the MAX_SIZE
		std::vector<unsigned int> returnsSeq; returnsSeq.reserve(maxNpPos); 
		for (i=0; i < (int)longNumOfSequences; i++){
			for (j=firstObs; j<totalNumReturns; j++) { 
				returnsSeq.push_back(j); 
			} 
		}
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
				int concatenatedLength = (int)concatenatedSample.size();
				// permutations will now give balanced sample
				for (int bootSample=0; bootSample<numMcIterations; bootSample++) {
					if (bootSample % 2 == 0) {
						std::vector<int> oneBootstrapSample; oneBootstrapSample.reserve(maxProductDays);
						for (j=0; j<maxProductDays; j++, concatenatedLength--) {
							int thisIndx; thisIndx = (int)floor( /*((double)rand() / RAND_MAX ) */ ArtsRan()*(concatenatedLength - 1));
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
			unsigned int numBrel = (unsigned int)b.brel.size();
			std::vector<double>	theseExtrema; theseExtrema.reserve(10);
			for (unsigned int uI = 0; uI < numBrel; uI++){
				SpBarrierRelation& thisBrel(b.brel.at(uI));
				thisBrel.endDaysDiff  = b.endDays - thisBrel.endDays;
			}
		}
		// ***************
		// ******* END: init
		// ***************







		// *****************
		// main MC loop
		// *****************
		int numMonDates = (int)monDateIndx.size();
		double accuracyTol(0.1);
		bool   doQuantoDriftAdj  = notUKSPA;
		bool   useAntithetic     = true;
		double thisT,thatT,thisPrice;
		std::vector<std::vector<double> > simulatedShocks(numUl);
		std::vector<std::vector<double> > cholMatrix(numUl, std::vector<double>(numUl)), rnCorr(numUl, std::vector<double>(numUl)),
			antitheticRandom(productDays+1, std::vector<double>(numUl));
		std::vector<std::vector<std::vector<double>>> mcForwards(numUl, std::vector<std::vector<double>>(numMonDates));
		std::vector<double> thisEqFxCorr(numUl),thisDivYieldRate(numUl), thisDriftRate(numUl), spotLevels(numUl), currentLevels(numUl), currentQuantoLevels(numUl), correlatedRandom(numUl), normalRandom(numUl);
		// set up forwardVols for the period to each obsDate
		std::vector< std::vector< std::vector<double>> >  ObsDateVols(numUl); // numUl x numObsDates x strike
		std::vector<double>                               ObsDatesT(numMonDates);
		
		// 
		// ******** init GBM correlation matrix, forward vols
		//
		if (getMarketData || useUserParams){
			// debug only: init antitheticRandom and force its use below using 'true' for 'useAntithetic' in the call to GenerateCorrelatedNormal()			
			for (i=0; i < (int)antitheticRandom.size(); i++){
				for (j=0; j < numUl;j++){
					antitheticRandom[i][j] = -1.0;
				}
			}
			//	
			// ******* init correlation matrix rnCorr
			//
			// ... initialise to unit diagonal
			for (i=0; i<numUl; i++) {        	    
				for (j=0; j<numUl; j++) {
					rnCorr[i][j] = 0.0;
				}
				rnCorr[i][i] = 1.0;
			}
			// ... now populate with whatever correlations we are given
			for (i=0; i < (int)md.corrsCorrelation.size(); i++) {
				for (j=0; j<(int)md.corrsCorrelation[i].size(); j++) {
					int otherId = md.corrsOtherId[i][j];
					double thisCorr = md.corrsCorrelation[i][j];
					if (i == otherId) {
						int jj = 1;
					}
					rnCorr[i][otherId]  = thisCorr;
					rnCorr[otherId][i]  = thisCorr;
				}
			}
			// ... finally Cholesky-decompose this rnCorr correlation matrix into cholMatrix
			evalResult = CHOL(rnCorr, cholMatrix);
			if (evalResult.errorCode != 0){
				return(evalResult);
			}

			//
			// ******* set up forwardVols for the period to each obsDate
			//
			double oneDay  = 1.0 / daysPerYear;
			for (i=0; i<numMonDates; i++) {
				ObsDatesT[i] = monDateT[i] * oneDay;
			}
			for (i=0; i<numUl; i++) {
				std::vector< std::vector<double>> &thisFwdVol = md.ulVolsFwdVol[i];
				std::vector<double>               &thisTenor  = md.ulVolsTenor[i];
				int numStrikes = (int)thisFwdVol[0].size();
				int numTenors  = (int)thisFwdVol.size();
				std::vector<std::vector<double>>  someVolSurface(numMonDates,std::vector<double>(numStrikes));
				double latestObsVol = -1.0;
				for (j=0; j<numStrikes; j++) {
					int thisDateIndx           = 0;
					int k                      = 0;
					double cumulativeVariance  = 0.0;
					double cumulativeT         = 0.0;
					thatT                      = 0.0;
					while (thisDateIndx < numMonDates && k < numTenors) {
						//
						// accumulate vol to the next ObsDate
						// ... imagine we have volTenors {1,2,3} but Obsdates {2,3}
						//
						double thisT   = thisTenor[k];
						double thisVol = thisFwdVol[k][j];
						double thisDt;
						if (ObsDatesT[thisDateIndx] > thisT) {
							// obsDate is beyond this strikeTenor, so just accumulate the variance and move to the next strikeTenor
							thisDt             = (thisT - thatT);
							cumulativeT        = cumulativeT + thisDt;
							cumulativeVariance = cumulativeVariance + thisVol * thisVol * thisDt;
							thatT = thisT;
							k = k + 1;
						}
						else {
							// obsDate is less than this strikeTenor, so accumulate part of this variance and move to the next obsDate
							thisDt             = ObsDatesT[thisDateIndx] - thatT;
							if (thisDt == 0.0) { thisDt = oneDay; }  // if evaluationDate is ON an obsDate then Dt can be zero, which would be pathological
							cumulativeT        = cumulativeT + thisDt;
							cumulativeVariance = cumulativeVariance + thisVol * thisVol * thisDt;
							latestObsVol       = sqrt(cumulativeVariance / cumulativeT);
							someVolSurface[thisDateIndx][j] = latestObsVol;
							// re-initiaise
							cumulativeVariance  = 0.0;
							cumulativeT         = 0.0;
							thatT               = ObsDatesT[thisDateIndx];
							thisDateIndx        = thisDateIndx + 1;
						}
						// if there is no more vol surface then just repeat the last vol
						if (k == numTenors) {
							// if obsDate is beyond the last tenor, just use the cumulative variance to the last tenor
							latestObsVol = sqrt(cumulativeVariance / cumulativeT);
							for (m = thisDateIndx; m<numMonDates; m++) {
								someVolSurface[m][j] = latestObsVol;
							}
						}
					}
				}
				ObsDateVols[i] = someVolSurface;
			}
			//
			// ******* END: set up forwardVols for the period to each obsDate
			//
		}
		// 
		// ******** END: init GBM correlation matrix, forward vols
		//

		//  stopping-rule accuracies
		if (     !getMarketData && numMcIterations <= 25000){ accuracyTol = 2.0; }
		else if (!getMarketData && numMcIterations <= 50000){ accuracyTol = 1.0; }
		else if ( getMarketData || numMcIterations > 200000){ accuracyTol = 0.01; }

		/*
		* debug
		*/
		if (doDebug && debugLevel>=3) {
			// marketData
			for (int i=0; i < spots.size();i++) {
				std::cerr << "SPOT" << i << ":" << spots[i] << std::endl;
			}
		}







		// ***********************
		// ******* START LOOP McIterations
		// ***********************
		std::vector<double> debugCorrelatedRandNos;
		int numDisables(0);
		int randnosStoreSize = (int)randnosStore.size();
		double lognormalAdj  = useUserParams ? 0.0 : 0.5;
		ArtsRanInit();            // so that doTurkey sees the same ran sequence
		for (thisIteration = 0; thisIteration < numMcIterations &&
			(!consumeRands || randnoIndx<=randnosStoreSize)     && 
			(forceIterations || fabs(stdevRatioPctChange)>accuracyTol); thisIteration++) {

			// if it looks like a turkey it IS a turkey
			if (doTurkey && thisIteration == halfNumMcIterations) {
				getMarketData = false;
				ArtsRanInit();            // so that doTurkey sees the same ran sequence
				int totalNumHits(0);      // debugOnly: # CAPITAL hits
				//
				// unBend things - they are only bent for getMarketData purposes
				//
				for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++) {
					SpBarrier& b(barrier.at(thisBarrier));
					// ... keep track of #hits so far
					int numHits = (int)b.hit.size();
					b.numHitsAtCheckpoint = numHits;
					if (b.capitalOrIncome) {
						totalNumHits  += numHits;
					}
					
					// ... straighten bends
					unsigned int numBrel = (unsigned int)b.brel.size();
					b.straightenBends();
					for (unsigned int uI = 0; uI < numBrel; uI++) {
						SpBarrierRelation& thisBrel(b.brel.at(uI));
						thisBrel.straightenBends();
					}
					// ... issuerCallable
					if (issuerCallable) {
						startBurnInIteration = thisIteration;
						stopBurnInIteration  = startBurnInIteration + numBurnInIterations;
						b.setIsNeverHit();
						b.a.clear();
						b.muX.clear();
						b.muY.clear();
						b.covX.clear();
						b.covXY.clear();
					}
				}
				callableCashflows.clear();
			}
			// create new random sample for next iteration
			if (numMcIterations > 1){

				// **************
				// ******* generate price paths - Geometric Brownian Motion - change/populate future ulPrices[someUlIndx].price[someFuturePoint]
				// **************
				if (getMarketData || useUserParams){
					// init
					useAntithetic = !useAntithetic;
					for (i = 0; i < numUl; i++) {
						thisPrice = ulPrices[i].price[startPoint];
						spotLevels[i] = currentLevels[i] = currentQuantoLevels[i] = thisPrice;
					}
					// ************
					// riskNeutral simulation of underlyings
					// ...get market data as at each monDay, since marketData is likely less frequent, so makes no sense to sample more often
					// ************
					thisT           = 0.0;
					int thisNumDays = 1;
					for (int thisMonIndx = 0; thisMonIndx < numMonDates; thisMonIndx++){
						int thatNumDays     = monDateIndx[thisMonIndx];
						int thisMonPoint    = startPoint + thatNumDays;
						const std::string   thatDateString(allDates.at(thisMonPoint));
						double thisReturn, thisSig, thisValue, thatValue;
						

						// what is the time now
						thatT   = monDateT[thisMonIndx]/ daysPerYear;
						/*
						* get market data for each underlying, on this date
						*/
						for (i = 0; i < numUl; i++) {
							// get forward (OIS) drift rate = r2.t2 = r1.t1 + dr.dt
							std::vector<double>  theseTenors  = md.oisRatesTenor[i];
							std::vector<double>  theseRates   = md.oisRatesRate[i];
							thisValue           = interpVector(theseRates, theseTenors, thisT);
							thatValue           = interpVector(theseRates, theseTenors, thatT);
							thisDriftRate[i]    = thatT == thisT ? thatValue : (thatValue * thatT - thisValue * thisT) / (thatT - thisT);
							// get forward div rate = r2.t2 = r1.t1 + dr.d
							theseTenors = md.divYieldsTenor[i];
							theseRates  = md.divYieldsRate[i];
							thisValue           = interpVector(theseRates, theseTenors, thisT);
							thatValue           = interpVector(theseRates, theseTenors, thatT);
							thisDivYieldRate[i] = thatT == thisT ? thatValue : (thatValue * thatT - thisValue * thisT) / (thatT - thisT);
							if (stochasticDrift){
								theseRates          = md.divYieldsStdErr[i];
								thisValue           = interpVector(theseRates, theseTenors, thisT);
								thatValue           = interpVector(theseRates, theseTenors, thatT);
								double thisAdj      = thatT == thisT ? thatValue : (thatValue * thatT - thisValue * thisT) / (thatT - thisT);
								thisDivYieldRate[i] += thisAdj*NormSInv(ArtsRan());
							}
							if (ulFixedDivs[i]){
								fixedDiv[i]         = thisDivYieldRate[i];
								thisDivYieldRate[i] = 0.0;
							}
							//... any Quanto drift: LEAVE HERE IN CASE correlations become time-dependent
							// DOME: this assumes all payoffs are quanto
							// DOME: assumes all fx vols are 10% ...
							thisEqFxCorr[i]     = (int)md.fxcorrsCorrelation[i].size() == 0 ? 0.0 : md.fxcorrsCorrelation[i][0];
						}
						/*
						*  now generate underlying shocks until this obsDate
						*/
						double dt      = productNeedsFullPriceRecord ? (1.0 / daysPerYear) : (thatT - thisT);
						double rootDt  = sqrt(dt);
						for (int thisDay = productNeedsFullPriceRecord ? thisNumDays : thatNumDays; thisDay <= thatNumDays; thisDay++){
							if (thisDay > 0){  // some barriers will end on exactly the as-at date
								/*
								* calculate new prices for thisDt
								*/
								thisT          += dt;
								int thatPricePoint  = startPoint + thisDay;

								/*
								* EITHER: simulate new correlated shocks and calculate levels from local vols,drifts
								*/
								if (forOptimisation && productIndx != 0){
									// if forOptimisation we already saved (in optimiseMcLevels) simulated levels for underlyings in ProductId=1 
									// ... for use by other productIds so that we are using the same simulated levels for all such products
									for (i = 0; i < numUl; i++) {
										int id = ulIds[i];
										int ix = optimiseUlIdNameMap[id];
										//
										// create path point
										//
										ulPrices[i].price[thatPricePoint] = optimiseMcLevels[ix][thisDay-1][thisIteration];
									}
								}
								/*
								* OR: reuse saved levels
								*/
								// ... simulate a set of standardNormal shocks
								GenerateCorrelatedNormal(numUl, correlatedRandom, cholMatrix, normalRandom,
									useAntithetic,     // if you want to check things using fixed shocks, just set this to 'true' and set the shocks in 'antitheticRandom'
									thisDay,
									antitheticRandom,
									conserveRands,
									consumeRands,
									randnoIndx,
									randnosStore);
								if (doDebug && debugLevel > 4) {
									for (i = 0; i < numUl; i++) {
										simulatedShocks[i].push_back(correlatedRandom[i]);
									}
								}
								for (i = 0; i < numUl; i++) {
									double thisFixedDiv = fixedDiv[i] * dt;
									double varianceBasedSig;

									// assume for now that all strikeVectors are the same ... so we just use the first with md.ulVolsStrike[i][0]
									// thisSig          = InterpolateMatrix(localVol ? md.ulVolsImpVol[i] : ObsDateVols[i], localVol ? md.ulVolsTenor[i] : ObsDatesT, md.ulVolsStrike[i][0], thisT, currentLevels[i] / spotLevels[i]);
									varianceBasedSig = InterpolateMatrix(localVol ? totalVariance[i] : ObsDateVols[i], localVol ? md.ulVolsTenor[i] : ObsDatesT, md.ulVolsStrike[i][0], thisT, currentLevels[i] / spotLevels[i]);
									varianceBasedSig = pow(varianceBasedSig / thisT, 0.5);
									thisSig          = varianceBasedSig;
									//... calculate return for thisDt  for this underlying
									
									thisReturn             = exp((thisDriftRate[i] - thisDivYieldRate[i] - lognormalAdj*thisSig * thisSig)* dt + thisSig * correlatedRandom[i] * rootDt);
									currentLevels[i]       = currentLevels[i] * thisReturn - thisFixedDiv;
									if (currentLevels[i] < 0.0){ currentLevels[i]  = 0.1; }  // some fixed divs could do this eg UKXFD, tiny positive avoids varianceCalcs blowing up
									currentQuantoLevels[i] = currentQuantoLevels[i] * thisReturn *  (doQuantoDriftAdj ? exp(-thisSig * thisEqFxCorr[i] * 0.15 * dt) : 1.0) - thisFixedDiv;
									if (currentQuantoLevels[i] < 0.0){ currentQuantoLevels[i]  = 0.0; }  // some fixed divs could do this eg UKXFD
									//
									// create path point
									//
									ulPrices[i].price[thatPricePoint] = currentQuantoLevels[i];

									// debugCorrelatedRandNos.push_back(currentQuantoLevels[i]/spotLevels[i]);
									// if forOptimisation we save simulated levels for underlyings in ProductId=1 
									// ... for use by other productIds so that we are using the same simulated levels for all such products
									if (forOptimisation && productIndx == 0){
										optimiseMcLevels[i][thisDay-1].push_back(currentQuantoLevels[i]);
									}
								}
							}
						}
						/*
						* move to next ObsDate
						*/
						thisT       = thatT;
						thisNumDays = thatNumDays + 1;
						// accumulate simulated prices - to be able to check forwards
						for (i = 0; i < numUl; i++) {
							mcForwards[i][thisMonIndx].push_back(currentLevels[i]);
						}
					}
				}
				// **************
				// ******* generate price paths - bootstrap resampling - change/populate future ulPrices[someUlIndx].price[someFuturePoint]
				// **************
				else {
					bool useNewerMethod(true);
					bool useNewMethod(false);
					unsigned long int _notionalIx = (unsigned long int)floor( /*((double)rand() / RAND_MAX)*/ ArtsRan()*(npPos - 1));
					int thisBootstrapStride       = bootstrapStride; 
					int thisReturnIndex;					

					if (useNewerMethod){
						// just uses balanced sampling - we can't do true antithetic sampling
						for (j = startPoint + 1; j <= startPoint + productDays; j++){
							// ************
							// bootstrap resampling of underlyings
							// ************
							
							// bootstrapStride
							if (doBootstrapStride){
								// just crawl along the returns vector, unless it ends
								if (thisBootstrapStride > 0 && _notionalIx < (maxNpPos - 2) && returnsSeq[_notionalIx + 1] < (unsigned int)totalNumReturns){
									thisBootstrapStride -= 1;
									_notionalIx         += 1;
								}
								else {
									// either finished striding, or came to the last return
									thisBootstrapStride = bootstrapStride;
									_notionalIx = (unsigned long int)floor( /*((double)rand() / RAND_MAX)*/ ArtsRan() *(npPos - 1));
								}
							}
							else {
								// pick element from returns vector
								_notionalIx = (unsigned long int)floor( /*((double)rand() / RAND_MAX)*/ ArtsRan()*(npPos - 1));
							}

							// SLOW: thisReturnIndex = _notionalIx % totalNumReturns;
							thisReturnIndex = returnsSeq[_notionalIx];
							for (i = 0; i < numUl + hasCompoIntoCcy; i++) {
								double thisReturn; thisReturn = ulReturns[i][thisReturnIndex];
								//
								// create path point
								//
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
							for (j = startPoint + 1; j <= maxProductDays; j++){
								int thisIndx = useAntithetic ? totalNumReturns - thisTrace[j - 1] - 1 : thisTrace[j - 1];
								for (i = 0; i < numUl; i++) {
									double thisReturn; thisReturn = ulReturns[i][thisIndx];
									//
									// create path point
									//
									ulPrices[i].price[j] = ulPrices[i].price[j - 1] * thisReturn;
								}
							}
						}
						else{   // OLD method KEEP
							for (j = 1; j < totalNumReturns; j++){
								int thisIndx; thisIndx = (int)floor( /*((double)rand() / RAND_MAX)*/ ArtsRan()*(totalNumReturns - 1));
								for (i = 0; i < numUl; i++) {
									double thisReturn; thisReturn = ulReturns[i][thisIndx];
									//
									// create path point
									//
									ulPrices[i].price[j] = ulPrices[i].price[j - 1] * thisReturn;
								}
							}
						}
					}
				}
				// possible shift prices
				for (i = 0; i < numUl; i++) {
					if (doShiftPrices[i]){
						double thisShift = shiftPrices[i];
						for (j = startPoint + 1; j <= startPoint + productDays; j++){
							//
							// create path point
							//
							ulPrices[i].price[j] -= thisShift;
						}
					}
				}
			}
			// track simulated underlying levels at maturity
			for (i = 0; i < numUl; i++) {
				double thisReturn = ulPrices[i].price[startPoint + productDays] / ulPrices[i].price[startPoint];
				simulatedLogReturnsToMaxYears[i].push_back(log(thisReturn)); 
				simulatedReturnsToMaxYears[i].push_back(thisReturn);
				simulatedLevelsToMaxYears[i].push_back(ulPrices[i].price[startPoint + productDays]);
			}

			// START LOOP wind 'thisPoint' forwards to next TRADING date, so as to start a new product
			for (int thisPoint = startPoint; thisPoint < lastPoint; thisPoint += historyStep) {
				if (numMcIterations == 1){  // only need to start on a trading day for HistoricBacktest
					while (allNonTradingDays.at(thisPoint) && thisPoint < lastPoint) {
						thisPoint += 1;
					}
					if (thisPoint >= lastPoint){ continue; }
				}
				
				// possiblcy track timepoints ulIds
				if (doTimepoints){
					for (i=0; i < numTimepoints; i++){
						int thisTpDays = timepointDays[i];
						for (j=0; j < numUls; j++){
							double thisLevel = ulPrices[j].price[thisTpDays + thisPoint] / ulPrices[j].price[thisPoint];
							timepointLevels[i][j].push_back(thisLevel);
						}
					}

					// save path
					if (doPaths && !usingProto && !doPriips){
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
				// init budget things: postStrike deals will initialise .budgetUsed and .initialBudget
				double budget     = thisPostStrikeState.initialBudget - thisPostStrikeState.budgetUsed;
				double lockedIn   = thisPostStrikeState.lockedIn;

				for (i=0; i < numUls; i++){ useUl[i] = true; }
				matured                                = false;
				maturityBarrier                        = -1;
				couponValue                            = 0.0;
				double                 thisPayoff;
				double                 finalAssetReturn  = 1.0e9;
				int                    finalAssetIndx;
				double                 benchmarkReturn   = 1.0e9;
				std::vector<double>    thesePrices(numUl + hasCompoIntoCcy), startLevels(numUl + hasCompoIntoCcy), lookbackLevel(numUl + hasCompoIntoCcy), overrideThesePrices(numUl + hasCompoIntoCcy);

				// set up barriers
				/*
				* BEWARE ... startLevels[] has same underlyings-order as ulPrices[], so make sure any comparison with them is in the same order
				*        ... and watch out for useUlMap argument which tells callee function that array arguments are already in the correct synchronised order
				*/
				for (i = 0; i < numUl+hasCompoIntoCcy; i++) { startLevels.at(i) = ulPrices.at(i).price.at(thisPoint); }
				for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
					// if (barrierDisabled[thisBarrier]){ std::cerr << "BarrierDisabled:" << thisBarrier << std::endl;  }
					barrierDisabled[thisBarrier]= false;
					SpBarrier& b(barrier.at(thisBarrier));
					int numBrel = (int)b.brel.size();
					std::vector<double>	theseExtrema; theseExtrema.reserve(10);
					for (unsigned int uI = 0; uI < (unsigned int)numBrel; uI++){
						SpBarrierRelation& thisBrel(b.brel.at(uI));
						int thisName = ulIdNameMap.at(thisBrel.underlying);
						thisBrel.doAveragingIn(startLevels.at(thisName), thisPoint, lastPoint + (!doAccruals ? monDateIndx[numMonDates-1]: 0), ulPrices.at(thisName),useUl);
						
						
						if (b.isStrikeReset && (thisBrel.startDays>0 || b.isStopLoss)){
							int brelStartPoint = thisPoint + thisBrel.startDays;
							if (brelStartPoint < 0){ brelStartPoint  = 0; }
							// remove next line: we now ensure there are enough prices for the full product term
							// if (brelStartPoint >= totalNumDays){ brelStartPoint  = totalNumDays-1; }
							thisBrel.setLevels(ulPrices.at(thisName).price.at(brelStartPoint), doAccruals);
						}
						else { thisBrel.setLevels(startLevels.at(thisName), doAccruals); }
						// cater for extremum barriers, where typically averaging does not apply to barrier hit test
						// ...so set barrierWasHit[thisBarrier] if the extremum condition is met
						// check to see if extremumBarriers hit
						if (b.isExtremum) {
							double thisExtremum;
							int firstPoint = thisPoint + thisBrel.startDays; if (firstPoint < 0){ firstPoint  = 0; }
							int lastPoint  = thisPoint + thisBrel.endDays;
							// accruals only look as far as real data (cannot look further into simulated time
							if (doAccruals && (lastPoint  > totalNumDays - 1)){ lastPoint   = totalNumDays - 1; }
							const std::vector<double>&  thisTimeseries = ulPrices.at(thisName).price;
							// 'Continuous'    means ONE-touch  so Above needs MAX  and !Above needs MIN
							// 'ContinuousALL' means  NO-touch  so Above needs MIN  and !Above needs MAX
							// ... so we need MAX in these cases
							if ((thisBrel.above && !thisBrel.isContinuousALL) || (!thisBrel.above && thisBrel.isContinuousALL)) {
								for (k = firstPoint, thisExtremum = -1.0e20; k <= lastPoint; k++) {
									if (thisTimeseries[k] > thisExtremum){ thisExtremum = thisTimeseries[k]; }
								}
							}
							else {
								for (k = firstPoint, thisExtremum = 1.0e20; k <= lastPoint; k++) {
									if (thisTimeseries[k] < thisExtremum){ thisExtremum = thisTimeseries[k]; }
								}
							}
							theseExtrema.push_back(thisExtremum);
						}
					} // END for (unsigned int uI = 0; uI < numBrel; uI++){
					if (b.isExtremum && (!doAccruals || b.endDays<0)) {
						if (b.isContinuousGroup && numBrel>1) {  // eg product 536, all underlyings must be above their barriers on some common date
							int firstPoint = thisPoint + b.brel[0].startDays; if (firstPoint < 0){ firstPoint  = 0; }
							int lastPoint  = thisPoint + b.brel[0].endDays;
							// accruals only look as far as real data (cannot look further into simulated time
							if (doAccruals && (lastPoint  > totalNumDays - 1)){ lastPoint   = totalNumDays - 1; }
							std::vector<int> ulNames;
							for (unsigned int uI = 0; uI < (unsigned int)numBrel; uI++){
								ulNames.push_back(ulIdNameMap.at(b.brel.at(uI).underlying));
							}
							for (k=firstPoint; !barrierWasHit.at(thisBarrier) && k <= lastPoint; k++) {
								std::vector<double>    tempPrices;
								for (j=0; j < numBrel; j++) { tempPrices.push_back(ulPrices.at(ulNames[j]).price[k]); }
								barrierWasHit.at(thisBarrier) = b.hasBeenHit || (b .* (b.isHit))(k, ulPrices, tempPrices, false, startLevels,useUl);
							}
						}
						else { barrierWasHit.at(thisBarrier) = b.hasBeenHit || (b .* (b.isHit))(k, ulPrices, theseExtrema, false, startLevels,useUl); }
						// for post-strike deals, record if barriers have already been hit
						if (doAccruals && b.yearsToBarrier <= 0.0){ 
							b.hasBeenHit = barrierWasHit[thisBarrier]; 
						}
					}
				} // END set up barriers

				// START LOOP through each monitoring date to trigger events
				for (int thisMonIndx = 0; !matured && thisMonIndx < (int)monDateIndx.size(); thisMonIndx++){
					int thisMonDays  = monDateIndx[thisMonIndx];
					int thisMonPoint = thisPoint + thisMonDays;
					const std::string   thisDateString(allDates.at(thisMonPoint));
					for (i = 0; i < numUl + addCompoIntoCcy; i++) {
						thesePrices[i] = ulPrices[i].price.at(thisMonPoint);
						if (false) {
							std::cerr << "Iteration:" << thisIteration << "thisMonDays:" << thisMonDays << "UL#" << i << "level:" << thesePrices[i] << std::endl;								
						}
					}
					int lastTradingIndx = thisMonPoint - (thisMonPoint>0 ? 1:0);
					while (lastTradingIndx && ulPrices[0].nonTradingDay[lastTradingIndx]){ lastTradingIndx -= 1; }
					// START LOOP test each barrier
					for (int thisBarrier = 0; !matured && thisBarrier<numBarriers; thisBarrier++){
						SpBarrier &b(barrier[thisBarrier]);
						// START is barrier alive
						bool notDisabled = barrierDisabled[thisBarrier] == false;
						if ((b.endDays == thisMonDays || (b.isContinuous && thisMonDays <= b.endDays && thisMonDays >= b.startDays)) && notDisabled) {
							// strikeReset AND stopLoss means we want YESTERDAY's close
							// ...ge a hack to accomodate #2774 where investor is short a strip of daily-reset KIPs struck at YESTERDAY's close
							if (b.isStrikeReset && b.isStopLoss){
								int numBrel = (int)b.brel.size();
								for (unsigned int uI = 0; uI < (unsigned int)numBrel; uI++){
									SpBarrierRelation& thisBrel(b.brel.at(uI));
									int thisName = ulIdNameMap.at(thisBrel.underlying);
									thisBrel.setLevels(ulPrices.at(thisName).price.at(lastTradingIndx), doAccruals);
								}
							}

							// averaging/lookback - will replace thesePrices with their averages							
							b.doAveraging(startLevels,thesePrices, lookbackLevel, ulPrices, thisPoint, thisMonPoint,numUls,useUl);
							
							//
							// ******* START is barrier hit
							//

							// if issuerCallable, need forwardValue of paidOutCoupons for earlyExercise decision
							//  ... so during burnIn b.couponValues.push_back() ALWAYS called
							//  ... so after burnIn  b.couponValues.size() should be >= (numBurnInIterations)
							//  ... where 'greaterThan' numBurnInIterations can happen if doTurkey and the previous getMarketValue iteratinos recorder some hits
							if (issuerCallable && couponPaidOut && !doAccruals && b.capitalOrIncome) {
								double thisCouponValue = 0.0;
								for (int paidOutBarrier = 0; paidOutBarrier < thisBarrier; paidOutBarrier++) {
									if (!barrier[paidOutBarrier].capitalOrIncome		
										&&  barrierWasHit[paidOutBarrier]
										// && !(issuerCallable && barrier[paidOutBarrier].endDays == b.endDays)
										&& (barrier[paidOutBarrier].endDays >= -settleDays || (barrier[paidOutBarrier].isMemory && !barrier[paidOutBarrier].hasBeenHit))) {
										SpBarrier &ib(barrier[paidOutBarrier]);
										thisCouponValue   += ((ib.payoffTypeId == fixedPayoff ? 1.0 : 0.0)*(ib.isCountAvg ? ib.participation*min(ib.cap, ib.proportionHits*ib.payoff) : ib.proportionHits*ib.payoff) + ib.variableCoupon)*pow(b.forwardRate, b.yearsToBarrier - ib.yearsToBarrier);
									}
								}
								if (thisIteration >= startBurnInIteration && thisIteration < stopBurnInIteration) {
									b.couponValues.push_back(thisCouponValue);   // does what storePayoff() would have done, were we not in burnIn period
								}
								else {
									b.thisCouponValue = thisCouponValue;
								}
							}

							if (b.hasBeenHit || barrierWasHit[thisBarrier] || b.proportionalAveraging || b.countAveraging || (!b.isExtremum && (b .* (b.isHit))(thisMonPoint, ulPrices, thesePrices, true, startLevels,useUl))){
								barrierWasHit[thisBarrier] = true;

								// for post-strike deals, record if barriers have already been hit
								if (doAccruals){ 
									b.hasBeenHit= true; 
								}  
								double thisOptionPayoff;
								bool   barrierPayoffContainsVariableCoupons(false);

								thisPayoff = b.getPayoff(startLevels, lookbackLevel, thesePrices, AMC, productShape, doFinalAssetReturn, 
									finalAssetReturn, thisOptionPayoff, finalAssetIndx, ulIds, useUl,thisPoint,ulPrices,lockedIn);
								if (hasCompoIntoCcy){
									double thisFxLevel   = ulPrices[numUl].price.at(thisMonPoint);
									double startFxLevel  = ulPrices[numUl].price.at(thisPoint);
									double fxReturn      = thisFxLevel / startFxLevel;
									thisPayoff          *=  fxReturn;
								}
								// process barrier commands
								if (b.barrierCommands != ""){
									std::vector<std::string> barrierCommands = split(b.barrierCommands, ';');
									for (int i=0; i<(int)barrierCommands.size(); i++){
										std::string bCommand  = barrierCommands[i];
										std::vector<std::string> bCommandBits;
										splitBarrierCommand(bCommandBits, bCommand);
										std::vector<std::string> bCommandArgs;
										std::string thisBcommand = bCommandBits[0];
										if (bCommandBits.size() > 1){ bCommandArgs = split(bCommandBits[1], ','); }

										if (thisBcommand == "disableBarrier"){
											int targetBarrierId = atoi(bCommandArgs[0].c_str()) - 1;
											barrierDisabled[targetBarrierId] = true;
											numDisables++;
										}
										else if (thisBcommand == "lockIn"){
											// lockin some level
											double lock = atof(bCommandArgs[0].c_str()) ;
											if (lock > lockedIn){ lockedIn = lock; }
											if (doAccruals){
												thisPostStrikeState.lockedIn    = lockedIn;
											}
										}
										else if (thisBcommand == "decreaseBudget"){
											// initialise budget if there is an argument
											if (bCommandBits.size() > 1){
												budget = atof(bCommandArgs[0].c_str()); 
												thisPostStrikeState.initialBudget = budget;
											}
											if (doAccruals){
												thisPostStrikeState.budgetUsed += thisOptionPayoff;
											}
											budget -= thisOptionPayoff;
											if (budget < 0.0){
												// zero barrier levels of next CapitalBarrier
												bool done = false;
												for (int nextBarrier = thisBarrier + 1; !done && nextBarrier < numBarriers; nextBarrier++){
													SpBarrier &b(barrier[nextBarrier]);
													if (b.capitalOrIncome) {
														done        = true;
														int numBrel = (int)b.brel.size();
														for (unsigned int uI = 0; uI < (unsigned int)numBrel; uI++){
															SpBarrierRelation& thisBrel(b.brel.at(uI));
															thisBrel.barrierLevel  = 0.0;
														}
													}
												}
											}
										}
									}
								}




								// ***********
								// capitalBarrier hit ... so product terminates
								// ***********
								if (b.capitalOrIncome){
									if (thisMonDays >= 0 || doAccruals){
										// DOME: just because a KIP barrier is hit does not mean the put option is ITM
										// ...currently all payoffs for this barrier are measured...so we currently do not report when KIP is hit AND option is ITM
										// ...could just use this predicate around the next block: 
										// if(!(thisBarrier.payoffType === 'put' && thisBarrier.Participation<0 && optionPayoff === 0) ){									
										matured         = true;
										maturityBarrier = thisBarrier;
										// add forwardValue of paidOutCoupons
										if (couponPaidOut && !doAccruals) {
											for (int paidOutBarrier = 0; paidOutBarrier < thisBarrier; paidOutBarrier++){
												// paidOutBarrier could be in the past, if barrier[paidOutBarrier].isMemory
												if (   !barrier[paidOutBarrier].capitalOrIncome										// only couponBarriers
													&&  barrierWasHit[paidOutBarrier]												// ... that were hit (possibly memory-triggered)
													// &&  !(issuerCallable && barrier[paidOutBarrier].endDays == barrier[thisBarrier].endDays)
													&& (barrier[paidOutBarrier].endDays >= -settleDays                              // exclude paidOut in the past, so already 'gone'
														|| 
														(barrier[paidOutBarrier].isMemory && !barrier[paidOutBarrier].hasBeenHit))  // ... but include past memory-triggered coupons
													){
													SpBarrier &ib(barrier[paidOutBarrier]);
													double fwdFactor = doForwardValueCoupons ? pow(b.forwardRate, b.yearsToBarrier - ib.yearsToBarrier) : 1.0 ;
													if (ib.variableCoupon != 0.0) { 
														barrierPayoffContainsVariableCoupons = true; 
													}
													couponValue   += ((ib.payoffTypeId == fixedPayoff ? 1.0 : 0.0)*(ib.isCountAvg ? ib.participation*min(ib.cap, ib.proportionHits*ib.payoff) : ib.proportionHits*ib.payoff) + ib.variableCoupon)*fwdFactor;
												}
											}
										}
										if (couponFrequency.size()) {  // add fixed coupon
											bFixedCouponsDate   = allBdates.at(thisMonPoint);
											int    freqLen      = (int)couponFrequency.length();
											char   freqChar     = toupper(couponFrequency[freqLen - 1]);
											std::string freqNumber = couponFrequency.substr(0, freqLen - 1);
											sprintf(charBuffer, "%s", freqNumber.c_str());
											double couponEvery  = atof(charBuffer);
											double daysPerEvery = freqChar == 'D' ? 1 : freqChar == 'M' ? 30 : daysPerYear;
											double daysElapsed  = (bFixedCouponsDate - bStartDate).days() + daysExtant;
											double couponPeriod = daysPerEvery*couponEvery;
											if (couponPaidOut){
												daysElapsed  -= floor(daysExtant / couponPeriod)*couponPeriod; // floor() so as to include accrued stub
											}
											double numFixedCoupons = max(0.0, /*floor*/(daysElapsed / couponPeriod)); // allow fractional coupons
											double periodicRate    = exp(log(b.forwardRate) * (couponPeriod / daysPerYear));
											double effectiveNumCoupons = (pow(periodicRate, numFixedCoupons) - 1) / (periodicRate - 1);
											couponValue += fixedCoupon*(couponPaidOut && doForwardValueCoupons ? effectiveNumCoupons : numFixedCoupons);
										}
										// add accumulated couponValue, unless b.forfeitCoupons is set
										if (!b.isForfeitCoupons){ 
											thisPayoff += (fullyProtected && (couponValue + accruedCoupon) < 0.0) ? 0.0 : couponValue + accruedCoupon;
										}
										// ** maybe record benchmark performance
										if (benchmarkId){
											n      = ulIdNameMap[benchmarkId];
											double thisRefLevel = startLevels[n] / benchmarkMoneyness;
											double thisShift    = doShiftPrices[n] ? shiftPrices[n] : 0.0;
											benchmarkReturn = (thesePrices[n] + thisShift) / (thisRefLevel + thisShift);
											if (benchmarkStrike > 0.0){   // assume investor short a PUT
												double bmPutReturn = thesePrices[n] / benchmarkStrike;
												thisPayoff *= bmPutReturn < 1.0 ? bmPutReturn : 1.0;
											}
										}
									}
									// possibly save optimisation results (productIndx zero is the dummy product that generates the underlyings'
									if (forOptimisation && productIndx != 0){
										thisAmount        = b.proportionHits*thisPayoff*baseCcyReturn;
										double thisYears  = b.yearsToBarrier;
										int    dayMatured = (int)floor(b.yearsToBarrier*daysPerYear);
										double thisAnnRet = thisYears <= 0.0 ? 0.0 : min(0.4, exp(log((thisAmount < unwindPayoff ? unwindPayoff : thisAmount) / midPrice) / thisYears) - 1.0); // assume once investor has lost 90% it is unwound...
										// MUST recalc discountRate as b.yearsToBarrier may have changed when bumpTheta  b.bumpSomeDays()
										double thisDiscountRate   = b.forwardRate + fundingFraction*interpCurve(cdsTenors[issuerIndx], cdsSpreads[issuerIndx], b.yearsToBarrier);
										double thisDiscountFactor = pow(thisDiscountRate, -(b.yearsToBarrier - forwardStartT));
										double thisPvPayoff = thisAmount*thisDiscountFactor;

										optimiseProductPayoff.push_back(thisAmount);
										optimiseProductResult.push_back(optOptimiseAnnualisedReturn ? thisAnnRet : thisPvPayoff);
										optimiseProductDuration.push_back(thisYears);
										// not using now
										// optimiseDayMatured.push_back(dayMatured - 1);
									}
								} // END of capital barrier processing
								else { // income barrier processing
									if (thisPayoff == 0.0 && b.barrierCommands ==""){ // Rainbow option coupons for example, where a 'hit' is only known after the option payoff is calculated 
										barrierWasHit[thisBarrier] = false;
									}
									else {
										barrierWasHit[thisBarrier] = true;
									}
									
									if (!couponPaidOut || b.endDays >= 0 || (doAccruals && b.endDays >= -settleDays)) {
										if (!couponPaidOut || doAccruals){  // barrier coupons only accrued, so no need to forwardCouponValue
											couponValue += b.isCountAvg ? b.participation*min(b.cap, b.proportionHits*thisPayoff) : b.proportionHits*thisPayoff;
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
													// if (!couponPaidOut)  ... already know this barrier was not hit, so couponPaidOut not relevant to memory-barriers
													// memory coupons are marked barrierWasHit[k] = true, which get forward-valued if !couponPaidOut
													if (!couponPaidOut) {
														couponValue += payoffOther;
													}
													
													// only store a hit if this barrier is in the future
													//if (thisMonDays>0){
													bOther.storePayoff(thisDateString, payoffOther*baseCcyReturn, payoffOther*baseCcyReturn, 1.0, 
														finalAssetReturn, finalAssetIndx, thisBarrier, doFinalAssetReturn, 0, false, doAccruals,false, thisIteration);
													//}
												}
											}
										}
									}
								} // END income barrier processing
								// only store a hit if this barrier is in the future
								//if (thisMonDays>0){
								thisAmount = (b.isCountAvg ? b.participation*min(b.cap, b.proportionHits*thisPayoff) : b.proportionHits*thisPayoff)*baseCcyReturn;
								

								// ***********************
								//  ******* issuerCallable
								// ***********************
								//   ... initial burnInIterations:
								//        ... store 'thisAmount' payoffs in callableCashflows[] during burnIn
								//        ... DO NOT store them in hit[] 'cos storePayoff() does NOT get called
								//   ... on the final burnInIteration we use these payoffs to estimate a regressionFunction for each earlier capitalBarrier
								//   ... subsequent iterations then use b.iscallableHit() to exercise-or-continue based on the expected-vcontinuation-value from these regressionFunctions
								//         ... MAYBE: Re-estimate for those burn-in iterations, then continue with the remaining iterations
								if (issuerCallable  
									&&  b.capitalOrIncome 
									&& numMcIterations>1 
									&& thisIteration >= startBurnInIteration
									&& thisIteration <  stopBurnInIteration) {
									//  store burn-in terminal cashflows, for use in GMM/LS regression
									callableCashflows.push_back(thisAmount);
									//
									// once burned-in, estimate the GMM/LS regressions
									//
									if (thisIteration == (stopBurnInIteration - 1)){
										if (stopBurnInIteration - startBurnInIteration != (int)callableCashflows.size()){
											std::cerr << " callable: incorrect size of callableCashflows" << std::endl;  exit(1);
										}
										double laterDiscountFactor = b.discountFactor;
										//
										// ******* Working backwards, estimate conditional expectation at each earlier CAPITAL
										//
										for (int thatBarrier = thisBarrier - 1; thatBarrier >= 0 && barrier[thatBarrier].endDays>0; thatBarrier--){
											SpBarrier &thatB(barrier[thatBarrier]);
											if (thatB.endDays < b.endDays && thatB.capitalOrIncome){
												Mat_IO_DP            conditionalExpectation(numBurnInIterations, 1);
												Mat_IO_DP            rhs(numBurnInIterations, numLRMrhs); // for basis function regression 
												Mat_IO_DP            lhs(numBurnInIterations, 1);         // for basis function regression 
												std::vector<double>  &x(thatB.worstUlRegressionPrices);   // for GMM regression
												std::vector<double>  &y(callableCashflows);               // for GMM regression 
												Vec_IO_DP            eK(numBurnInIterations);             // for GMM debug only - nice to see which cluster associates most closely with each point

												// discount callableCashflows back to this obsDate
												double thisDiscountFactor = laterDiscountFactor / thatB.discountFactor;
												laterDiscountFactor = thatB.discountFactor;
												double dT           = b.yearsToBarrier - thatB.yearsToBarrier;
												if (dT != 0.0) { dT = pow(dT,0.5); }
												double issuerCdsVol = cdsVols[issuerIndx];
												double ccMean, ccStdev, ccStdErr;
												MeanAndStdev(callableCashflows, ccMean, ccStdev, ccStdErr);
												for (int i=0; i < numBurnInIterations; i++) {
													// ... omit wobble for now, which adds ~25bp to FVs
													// ... not sure the EDG desk cares about better rates/cds to as to benefit from better funding conditions
													// ...  ... the EDG desk is surely swapped-up at inception
													// but if there is  no variability in callableCashflows,assume product is fixed-income only, so allow 'wobble'
													if (ccStdev<1.0e-5) {
														double  oncurveWobble  =  oncurveVol * NormSInv(ArtsRan());
														double  cdsWobble      =  issuerCdsVol * NormSInv(ArtsRan());
														double  thisWobble = 1.0 - (oncurveVol * NormSInv(ArtsRan()) + issuerCdsVol * NormSInv(ArtsRan())) * dT;
														if (doDebug) {
															thisWobble = 1.0;
														}
														callableCashflows[i]  *= thisDiscountFactor * thisWobble;
													}
													callableCashflows[i]  *= thisDiscountFactor;
												}
												// check data
												if (numBurnInIterations != (int)thatB.worstUlRegressionPrices.size()) {
													std::cerr << " callable: incorrect size of worstUlRegressionPrices" << std::endl;  exit(1);
												}
												if (numUls > 1 && numBurnInIterations != (int)thatB.nextWorstUlRegressionPrices.size()) {
													std::cerr << " callable: incorrect size of nextWorstUlRegressionPrices" << std::endl;  exit(1);
												}
												if (doDebug  && debugLevel == 4) {
													sprintf(charBuffer, "%s%d%s%d", "delete from regressiondata where productid=", productId, " and barrierid=", thatBarrier);
													mydb.prepare((SQLCHAR *)charBuffer, 1);
													sprintf(charBuffer, "%s%d%s%d", "delete from conditionalexpectation where productid=", productId, " and barrierid=", thatBarrier);
													mydb.prepare((SQLCHAR *)charBuffer, 1);
												}
												//
												//  Gaussian Mixture Model  condExp(y|x)  =  sumOverClusters( clusterMix * (muY  + covXY/covXX * (x - muX) )
												//
												if (USE_GMM_CLUSTERS) {
													int          numClusters(MAX_GMM_CLUSTERS);      // R-based tests show this seems a reasonable max# to start with  
																			          //   NOTE: this may decrease as we try and simplify the model
													bool         done(false);         // done once EM shows minimal BIC improvement
													const int    gmmIterations(20);   // R-based tests show this should be enough
													double       BIC(0.0);
													Vec_IO_DP    a(numClusters);      // the probability that the (unknown) dataGeneratingFunction selects a cluster's normalDistribution to generate an x,y
																				      //  ... they should sum to 1.0, although there seems to be rounding errors so 0.999 - 1.001 should be OK
													Vec_IO_DP    covX(numClusters), covY(numClusters), covXY(numClusters);
													Mat_IO_DP    mu(numClusters, 2);  // meanX,meanY of each cluster
													const double minX = *std::min_element(std::begin(x), std::end(x));
													const double minY = *std::min_element(std::begin(y), std::end(y));
													const double maxX = *std::max_element(std::begin(x), std::end(x));
													const double maxY = *std::max_element(std::begin(y), std::end(y));
													const double dataCovX  = MyCorrelation(x, x, false) ;
													const double dataCovY  = MyCorrelation(y, y, false) ;
													const double dataCovXY = MyCorrelation(x, y, false) ;
													const int    minClusterSize = (int)max(3.0, numBurnInIterations*gmmMinClusterFraction);
													if (doDebug && debugLevel == 4) {
														sprintf(charBuffer, "%s%d%s%d", "delete from gmmcoeff where productid=", productId, " and barrierid=", thatBarrier);
														mydb.prepare((SQLCHAR *)charBuffer, 1);
													}
													if (doDebug && debugLevel == 4) {
														// GMM.R needs x,y datapoints
														for (int j=0; j < numBurnInIterations; j++) {
															sprintf(charBuffer, "%s%d%s%d%s%lf%s%lf%s",
																"insert into regressiondata (ProductId,BarrierId,X1,Y) values (",
																productId,   ",",
																thatBarrier, ",",
																x[j],        ",",
																y[j],       ");");
																mydb.prepare((SQLCHAR *)charBuffer, 1);
														}
													}
													double sumMix;
													//
													// iterate until done, or there is only 2 clusters
													//
													while (!done && numClusters > MIN_GMM_CLUSTERS) {
														//
														// init cluster assignment uniformly
														//
														Mat_IO_DP initialk(numBurnInIterations, 1);
														for (int i=0,thisCluster=0; i < numBurnInIterations;i++) {
															initialk[i][0] = thisCluster++;
															if (thisCluster == numClusters) { thisCluster = 0; }
														}
														//
														// init mu uniformly
														//
														const double xBucketSize = (maxX - minX) / (numClusters-1);
														const double yBucketSize = (maxY - minY) / (numClusters-1);
														std::vector<double> clusterBreaks;
														double thisClusterBreak(0.0); // reserve one for the minimum point ... KIP/minimum payoff attractor
														for (int i=0; i < numClusters;i++) {
															clusterBreaks.push_back(thisClusterBreak);
															thisClusterBreak += 1.0;
														}
														for (int i=0; i < numClusters; i++) {
															mu[i][0] = minX + xBucketSize * clusterBreaks[i];
															mu[i][1] = minY + yBucketSize * clusterBreaks[i];
														}
														//
														// init cov - spread data-covariance across all clusters
														//
														const double thisCovX  =  dataCovX  / numClusters;
														const double thisCovY  =  dataCovY  / numClusters;
														const double thisCovXY =  dataCovXY / numClusters;
														for (int i=0; i < numClusters; i++) {
															covX [i] = thisCovX  ;
															covY [i] = thisCovY  ;
															covXY[i] = thisCovXY ;
														}
														//
														// init cluster mix - the probability that the (unknown) dataGeneratingFunction selects a cluster's normalDistribution to generate an x,y
														//
														if (GMM_INIT_MIX_RANDOMLY) {
															// random mix
															double ranSum(0.0);
															for (int i=0; i < numClusters; i++) {
																double thisRan = ArtsRan();
																a[i]    = thisRan;
																ranSum += thisRan;
															}
															for (int i=0; i < numClusters; i++) {
																a[i] /= ranSum;
															}
														}
														else {
															// uniform mix
															const double initMix(1.0/numClusters);
															for (int i=0; i < numClusters; i++) {
																a[i] = initMix;
															}
														}

														
														// EM loop 
														Mat_IO_DP z(numBurnInIterations, numClusters);  // PDFs of clusters for x,y datapoints
														Mat_IO_DP r(numBurnInIterations, numClusters);  // responsibilities of clusters for x,y datapoints
														double  previousBIC(0.0);                       // wany to track BIC improvements in EM loop
														bool    emDone(false);                          // EM is done once minimal BIC improvement, or if we need to reduce #clusters
														int     thisIter;
														for(thisIter=0; !emDone && thisIter < gmmIterations; thisIter++) {
															// fixup clusters with small determinants - usually constant y payoffs
															for (int i=0; i < numClusters; i++) {
																int	   thisCount = 0;
																while (my2dDet(covX[i], covXY[i], covXY[i], covY[i]) < 1.0e-08) {
																	covY[i]   = covY[i] == 0.0 ? covX[i] * 0.0001 : covY[i] * 10.0;    // increase y variability a bit - won't impact the conditional regression which does not need covY
																	thisCount += 1;
																	if (thisCount > 1000) {
																		// something wrong if we get here
																		std::cerr << "IssuerCallable: cannot make determinant large enough for cluster:" << i << std::endl;
																		evalResult.errorCode = 11123;
																		return(evalResult);
																	}
																}
															}
															// *************
															// ***** Calculate into z the PDF for each x,y point under each cluster mean and covariances
															// *************
															Mat_IO_DP  oldZ(z);
															evalResult = mvpdf(z, x, y, mu, covX, covY, covXY, numClusters);
															if (evalResult.errorCode != 0) {
																	return(evalResult);
															}
															if (thisIter > 0) {
																// one stopping rule is to stop if all z elements are within some epsilon of oldZ
																double maxDz(0.0);
																for (int j=0; j < numBurnInIterations; j++) {
																	for (int i=0; i < numClusters; i++) {
																		double thisDz = abs(z[j][i] - oldZ[j][i]);
																		if (thisDz > maxDz) { maxDz = thisDz; }
																	}
																}
																if (maxDz < 0.01) {
																	emDone = true;
																	done   = true;
																	continue;
																}																
															}
															// **************
															// ******* Expectation Step
															//          - compute       r[j = burnInIteration][i = cluster] = relative likelihood of each cluster i to have been "responsible" for each j(x,y) datapoint															
															//          - then compute mc[i = cluster]                      = relative likelihood of each cluster i to have been "responsible" for ALL datapoints
															// **************
															for (int j=0; j < numBurnInIterations; j++) {
																double sumModelResponsabilities = 0.0;
																for (int i=0; i < numClusters; i++) {
																	// likelihood that cluster i was "responsible " for x,y = normalPDF * cluster's current mix a[i] 
																	double modelResponsibility =  z[j][i] * a[i];
																	r[j][i]                   = modelResponsibility;
																	sumModelResponsabilities += modelResponsibility;
																}
																// rescale responsabilities to sum to 1.0 for each x,y datapoint
																//  ... but not if sumModelResponsabilities is 0.0, which can happen if all responsibilities for this burnIn are 0.0 
																//  ... ie it is an outlier so that no cluster is near it, especially if we pruned numClusters too hard
																if (sumModelResponsabilities != 0.0) {
																	for (int i=0; i < numClusters; i++) {
																		r[j][i]     /= sumModelResponsabilities;
																	}
																}
																
															}
															// recalc cluster mix mc[] to their current expected probability of having generated all x,y datapoints
															//   = the relative proportion of each cluster's total responsabilities
															Vec_IO_DP mc(numClusters);          // mc[] is sum of responsabilities for each cluster i - will update mix a[] below
															for (int i=0; i < numClusters; i++) {
																mc[i] = 0.0;
															}
															for (int j=0; j < numBurnInIterations; j++) {
																int    thisCluster = 0;
																double highestR  = 0.0;
																for (int i=0; i < numClusters; i++) {
																	double thisR = r[j][i];
																	mc[i] += thisR;
																	if (thisR > highestR) { highestR = thisR;  thisCluster = i; }
																}
																eK[j] = thisCluster;
															}
															// *************
															// ******* Maximisation likelihood step 
															// *************
															//  - MLEs for each cluster's parameters (by virtue of being normalDists) are just the sample means/covars weighted by the cluster responsabilities
															//  - update a = mix and normal means/covars using mc[i] = sum(r[j][i]) for each cluster i
															sumMix = 0.0;
															for (int i=0; i < numClusters; i++) {
																double muX  = 0.0;
																double muY  = 0.0;
																// update mix a[] to each cluster's relative aggregate responsibility
																a[i]    = mc[i] / numBurnInIterations;  
																sumMix  += a[i];
																// update cluster x,y means to responsibilityWeighed
																for (int j=0; j < numBurnInIterations; j++) {
																	muX += x[j] * r[j][i];
																	muY += y[j] * r[j][i];
																}
																mu[i][0] = muX / mc[i];
																mu[i][1] = muY / mc[i];
															}
															//
															// check mix a[] sum to near 1.0
															//
															if (sumMix < 0.999 || sumMix > 1.001) {
																int jj = 1;
															}

															// similarly update covariance matrices, also weighted by responsibilities
															double totalCovarX(0.0);  // just debug info
															for (int i=0; i < numClusters; i++) {
																double thisCovX  = 0.0;
																double thisCovY  = 0.0;
																double thisCovXY = 0.0;
																for (int j=0; j < numBurnInIterations; j++) {
																	double thisDx             = x[j] - mu[i][0];
																	// if we have just 1 point in this cluster_i, then thisDy will be zero, making covY zero
																	// ... we try and add some covY jiggle above to avoid zero det of the covariance matrix
																	double thisDy             = y[j] - mu[i][1];
																	double thisResponsibility = r[j][i];
																	thisCovX  += thisDx * thisDx * thisResponsibility;
																	thisCovY  += thisDy * thisDy * thisResponsibility;
																	thisCovXY += thisDx * thisDy * thisResponsibility;
																}
																covX [i] = thisCovX  / mc[i];
																covY [i] = thisCovY  / mc[i];
																covXY[i] = thisCovXY / mc[i];
																totalCovarX += covX[i];
															}
															
															// compute logLikelihood - sum the PDF's weighted by mix
															double llik = 0.0;
															for (int j=0; j < numBurnInIterations; j++) {
																double thisPointLik = 0.0;
																for (int i=0; i < numClusters; i++) {
																	thisPointLik += z[j][i] * a[i];
																}				
																llik += log(thisPointLik);
															}
															// compute BIC
															const int numVariables(2);
															previousBIC = BIC; 
															BIC = 2 * llik - numClusters * (1 + 2 * numVariables + (numVariables*numVariables - numVariables) / 2) * log(numBurnInIterations);
													
															// exit loop if BIC improvement small
															if (thisIter > 10 && (BIC / previousBIC < 1.01)) {
																// std::cerr << "Iter:" << thisIter << " small BIC improvement" << std::endl;
																emDone = true;
																done   = true;
															}

															// ********
															// FINALLY check for small clusters - reduce the #clusters and loop again
															// ********
															bool isSmallCluster[MAX_GMM_CLUSTERS];
															int currentNumClusters = numClusters;
															double sumMix(0.0);
															for (int i=0; i < currentNumClusters; i++) {
																if (mc[i] < minClusterSize  
																	// comment out next line
																	// ... 'cos may cause clusters with just 1 burnIn, leading to zero covY, and zero det, for that cluster
																	// && numClusters > MIN_GMM_CLUSTERS 
																	) {
																	numClusters      -= 1;
																	isSmallCluster[i] = true;
																}
																else { 
																	isSmallCluster[i] = false; 
																	sumMix += a[i];
																}
															}
															// if we want fewer clusters, copy good cluster params into pole position
															if (numClusters < currentNumClusters) {
																for (int i=0,j=0; i < currentNumClusters; i++) {
																	// keep this cluster
																	if (!isSmallCluster[i]) {
																		if (i > j) {
																			// shuffle z left
																			for (int m=0; m < numBurnInIterations; m++) {
																				z[m][j] = z[m][i];
																			}
																			// shuffle cluster params left 
																			// ... shuffle mu,etc  left
																			mu   [j][0] = mu   [i][0];
																			mu   [j][1] = mu   [i][1];
																			covX [j]    = covX [i];
																			covY [j]    = covY [i];
																			covXY[j]    = covXY[i];
																			// shuffle a[] left
																			a[j] = a[i];
																			// click KEEP index
																			j += 1;
																		}																		
																		else { j += 1; }
																	}
																}
																// finally renormalise a[] to sum to 1.0
																if (numClusters == 1){
																	a[0] = 1.0;
																}
																else {
																	for (int i=0; i < numClusters; i++) {
																		a[i] /= sumMix;
																	}
																}
															}
															int jj = 1;
														} // for EM loop
														// finally renormalise a[] to sum to 1.0
														if (numClusters == 1) {
															a[0] = 1.0;
														}
														else {
															sumMix = 0.0;
															for (int i=0; i < numClusters; i++) {
																sumMix  += a[i];
															}
															for (int i=0; i < numClusters; i++) {
																a[i] /= sumMix;
															}
														}
														if (thisIter == gmmIterations) {
															done = true;
														}
													}  // while !done
													// final check mix a[] sums to 1.0
													sumMix = 0.0;
													for (int i=0; i < numClusters; i++) {
														sumMix  += a[i];
													}
													if (sumMix < 0.999 || sumMix > 1.001) {
														int jj = 1;
													}

													// install mix coefficients for this barrier
													for (int i=0; i < numClusters; i++) {
														thatB.a.push_back(     a[i]      );
														thatB.muX.push_back(   mu[i][0]  );
														thatB.muY.push_back(   mu[i][1]  );
														thatB.covX.push_back(  covX[i]   );
														thatB.covXY.push_back( covXY[i]  );
													}
													thatB.gmmNumClusters  =  numClusters;
													// now we have a GMM, form conditional expectation   condExp(y|x)  =  sumOverClusters( clusterMix * (muY  + covXY/covXX * (x - muX) )
													// ... lower down we will use these to test whether issuer would have called
													for (int j=0; j < numBurnInIterations; j++) {
														conditionalExpectation[j][0] = thatB.gmmConditionalExpectation(x[j]);
													}
													// debug info
													if (doDebug  && debugLevel == 4) {
														for (int i=0; i < numClusters; i++) {
															sprintf(charBuffer, "%s %d%s %d%s %d%s %lf%s %lf%s %lf%s %lf%s %lf%s %lf%s ", 
																"insert into gmmcoeff (ProductId,BarrierId,ClusterId,mix,muX,muY,covX,covY,covXY) values (", 
																productId,   ",", 
																thatBarrier, ",", 
																i,           ",", 
																a[i],        ",", 
																mu[i][0],    ",", 
																mu[i][1],    ",", 
																covX[i],     ",", 
																covY[i],     ",", 
																covXY[i],    ");");
															mydb.prepare((SQLCHAR *)charBuffer, 1);
														}
														int jj = 1;
													}
												}
												else {   // global basis function regression
													if (doDebug  && debugLevel == 4) {
														sprintf(charBuffer, "%s%d%s%d", "delete from regressioncoeff where productid=", productId, " and barrierid=", thatBarrier);
														mydb.prepare((SQLCHAR *)charBuffer, 1);
													}
													
													//  regress callableCashflows(possibly changed by later exercise(s)) vs (1.0, WorstUL, WorstUL ^ 2, NextWorstUL, NextWorstUL ^ 2)
													Mat_O_DP  XX(numLRMrhs, numLRMrhs), XXinv(numLRMrhs, numLRMrhs), XY(numLRMrhs, 1), lsB(numLRMrhs, 1);
													for (int j=0; j < numBurnInIterations; j++) {
														lhs[j][0] = callableCashflows[j];
													}

													for (int j=0; j < numBurnInIterations; j++) {
														double thisWorst      = thatB.worstUlRegressionPrices[j];
														rhs[j][0] = 1.0;
														rhs[j][1] = thisWorst;
														rhs[j][2] = thisWorst * thisWorst;
														if (numUls > 1) {
															double thisNextWorst  = thatB.nextWorstUlRegressionPrices[j];
															rhs[j][3] = thisNextWorst;
															rhs[j][4] = thisNextWorst * thisNextWorst;
														}
													}
													// XX = rhsT ** rhs
													MMult(rhs, rhs, XX, true, false);
													// PrintMatrix(XX, "XX");
													MatInverse(XX, XXinv);
													// XY = Xt ** lhs
													MMult(rhs, lhs, XY, true, false);
													// b = XX ** XY
													MMult(XXinv, XY, lsB, false, false);
													// install regression coefficients for this barrier
													thatB.lsConstant        = lsB[0][0];
													thatB.lsWorstB          = lsB[1][0];
													thatB.lsWorstSquaredB   = lsB[2][0];
													if (doDebug  && debugLevel == 4) {
														sprintf(charBuffer, "%s%d%s%d%s%lf%s%lf%s%lf%s", "insert into regressioncoeff (ProductId,BarrierId,constant,b1,b2) values (", productId, ",", thatBarrier, ",", lsB[0][0], ",", lsB[1][0], ",", lsB[2][0], ");");
														mydb.prepare((SQLCHAR *)charBuffer, 1);
													}
													if (numUls > 1) {
														thatB.lsNextWorstB          = lsB[3][0];
														thatB.lsNextWorstSquaredB   = lsB[4][0];
													}
													// form conditional expectation
													MMult(rhs, lsB, conditionalExpectation, false, false);

												} // END global basis function regression

												//
												// using the conditionalExpectations, if this barrier would have exercised early, revise cashflows accordingly
												//
												// delete regression data again, as GMM R code needed regressiondata x,y
												if (doDebug  && debugLevel == 4 && USE_GMM_CLUSTERS) {
													sprintf(charBuffer, "%s%d%s%d", "delete from regressiondata where productid=", productId, " and barrierid=", thatBarrier);
													mydb.prepare((SQLCHAR *)charBuffer, 1);
												}

												double thisFundingUnwindCost = thatB.annualFundingUnwindCost * (maxProductDays - daysExtant - thatB.endDays) / daysPerYear;
												int    firstBurnInIndx       = (int)thatB.couponValues.size() - numBurnInIterations;
												for (int j=0; j < numBurnInIterations; j++) {
													double thisPayoff            = thatB.payoff + thatB.fixedCouponValue + (firstBurnInIndx >= 0 ? thatB.couponValues[firstBurnInIndx + j] : 0.0);
													double continuationValue     = conditionalExpectation[j][0];
													double oldCashflow           = callableCashflows[j];
													if (continuationValue > (thisPayoff + thisFundingUnwindCost)) {
														// issuer would call,  opting for the cheaper (expected) payoff
														callableCashflows[j] = thisPayoff;
													}
													if (doDebug  && debugLevel == 4) {
														if (USE_GMM_CLUSTERS) {
															sprintf(charBuffer, "%s%d%s%d%s%lf%s%lf%s%lf%s%lf%s%lf%s%lf%s%lf%s",
																"insert into regressiondata (ProductId,BarrierId,X1,Y,ClusterId,OldCashflow,Payoff,ContinuationValue,NewCashflow) values (",
																productId, ",",
																thatBarrier, ",",
																x[j], ",",
																oldCashflow, ",",
																eK[j], ",",
																oldCashflow, ",",
																thisPayoff, ",",
																continuationValue, ",",
																callableCashflows[j], ");");
															mydb.prepare((SQLCHAR *)charBuffer, 1);
														}
														else {
															sprintf(charBuffer, "%s%d%s%d%s%lf%s%lf%s%lf%s", "insert into regressiondata (ProductId,BarrierId,X1,X2,Y) values (", 
																productId, ",", thatBarrier, ",", rhs[j][1], ",", rhs[j][2], ",", lhs[j][0], ");");
															mydb.prepare((SQLCHAR *)charBuffer, 1);
														}
													}												
												}
												int jj = 1;
											}  // if (thatB.endDays < b.endDays && thatB.capitalOrIncome){
											int jj = 1;
										} // for (int thatBarrier 
										// ******* END: Working backwards, estimate conditional expectation at each earlier CAPITAL barrier

										// reset barriers
										// ... no need for issuerCallable to resetBarriers during burnIn
										// ... 'cos storePayoff() never gets called during burnIn, so no need to reverse it with resetBarriers()
										// ... HOWEVER burnIn DOES use couponValues.pushBack(), so need to remove those added in burnIn
										// if (!doTurkey) { resetBarriers(); }
										for (j=0; j < numBarriers; j++) {
											SpBarrier& b(barrier.at(j));
											// install callableIsHit
											if (b.capitalOrIncome && b.endDays < maxEndDays){
												// after burnIn, want barriers to be tested with .isCallableHit()
												b.setIsCallableHit();
												// remove burnIn couponValues() ... CAPITAL barriers always have numBurnInIterations of them pushed at the end
												if ( (int)b.couponValues.size() == numBurnInIterations) {
													b.couponValues.clear();
												}
												else {
													// ... doTurkey adds burnIn onto the initial getMarketData analsis, so just want to remove the burnIns
													// ... or already-happened barriers will not push anything to its .couponValues()
													// ... or doBumps repeatedly calls .evaluate()
													if ( (int)b.couponValues.size() > numBurnInIterations) {
														b.couponValues.erase(b.couponValues.end() - numBurnInIterations, b.couponValues.end());
													}
												}
												int jj = 1;
											}
										}
										if (verbose){ std::cerr << "IssuerCallable: regressions finished ... continue remaining mcIterations " << std::endl; }
									} // if (thisIteration == (numBurnInIterations - 1)
								} // if (issuerCallable &&  ...
								// FINALLY, store this payoff and do hit.push_back(thisAmount)
								//  ... provided **not** in issuerCallable burnIn period, which does its own 'storage' of payoffs and does NOT do hit.push_back(thisAmount)
								//  ... so after issuerCallable burnIn, capitalBarriers HAVE EMPTY hit[] arrays
								else {
										b.storePayoff(thisDateString, thisAmount, couponValue*baseCcyReturn, barrierWasHit[thisBarrier] ? b.proportionHits : 0.0,
										finalAssetReturn, finalAssetIndx, thisBarrier, doFinalAssetReturn, benchmarkReturn, benchmarkId>0 && matured, 
										doAccruals, barrierPayoffContainsVariableCoupons, thisIteration);
									//cerr << thisDateString << "\t" << thisBarrier << endl; cout << "Press a key to continue...";  getline(cin, word);
								}

							} // END is barrier hit
							else {  // in case you want to see why not hit
								// b.isHit(thisMonPoint,ulPrices,thesePrices,true,startLevels);
							}
						} // END is barrier alive
					} // END LOOP test each barrier
				} // END LOOP through each monitoring date to trigger events

				// collect statistics for this product episode
				if (!doAccruals){
					// count NUMBER of couponHits, and update running count for that NUMBER  
					int thisNumCouponHits=0;
					bool someCapitalBarrierWasHit(false);
					for (int thisBarrier = 0; thisBarrier <= maturityBarrier; thisBarrier++){
						SpBarrier &b(barrier[thisBarrier]);
						if (!b.capitalOrIncome && (b.hasBeenHit || barrierWasHit[thisBarrier]) && b.proportionHits == 1.0){ thisNumCouponHits += 1; }
						if (b.capitalOrIncome && b.hasBeenHit){
							someCapitalBarrierWasHit = true;
						}
					}
					if (!someCapitalBarrierWasHit && !doAccruals) {
						int jj = 1;
					}
					numCouponHits.at(thisNumCouponHits) += 1;
				}
			} // END LOOP wind 'thisPoint' forwards to next TRADING date, so as to start a new product

			// debug
			if (!matured && !doAccruals){
				int j = 1;
			}

			//
			// ******* convergence test ... small changes in stdev/mean of payoffs
			//
			if (verbose && ((thisIteration + 1) % 1000) == 0){
				std::cout << ".";
			}
			if ((thisIteration - numBurnInIterations) > 750 && ((thisIteration - numBurnInIterations) + 1) % 10000 == 0){
				double thisMean       = PayoffMean(barrier);
				double thisStdevRatio = PayoffStdev(barrier, thisMean) / thisMean;
				double thisChange     = floor(10000.0*(thisStdevRatio - stdevRatio) / stdevRatio) / 100.0;
				const int numSigChanges(3);
				stdevRatioPctChanges.push_back(fabs(thisChange));
				int numStdevRatioPctChanges = (int)stdevRatioPctChanges.size();
				if (numStdevRatioPctChanges > numSigChanges){
					double sumChanges(0.0);
					for (int j=numStdevRatioPctChanges - 1; j >= numStdevRatioPctChanges - numSigChanges; j--) {
						sumChanges += stdevRatioPctChanges[j];
					}
					stdevRatioPctChange = sumChanges / numSigChanges;
				} 
				if (verbose) { std::cout << std::endl << " MeanPayoff:" << thisMean << " StdevRatio:" << thisStdevRatio << " StdevRatioChange:" << stdevRatioPctChange; }
				stdevRatio = thisStdevRatio;
			}
		}
		// ***********************
		// ******* END: LOOP McIterations
		// ***********************

		




		// *****************
		// ******* handle results
		// *****************
		//
		// ... save optimisation data	
		//
		if (forOptimisation && !doAccruals){
			if (productIndx == 0){
				// save underlying values for each iteration
				if (saveOptimisationPaths) {
					for (optCount=0; optCount < numMcIterations; optCount++) {
						if (optCount % optMaxNumToSend == 0) {
							// send batch
							if (optCount != 0) {
								mydb.prepare((SQLCHAR *)lineBuffer, 1);
							}
							// init for next batch							
							strcpy(lineBuffer, "insert into simulatedunderlyings (UnderlyingId,Iteration,Value) values ");
							optFirstTime = true;
						}
						for (i = 0; i < numUl; i++) {
							int id = ulIds[i];
							int ix = optimiseUlIdNameMap[id];
							double startLevel = ulPrices[i].price[startPoint];
							/* to use underlying return to date product matured
							int thisDay = optimiseDayMatured[optCount];
							*/
							int thisDay = 7; // to calc 1-week deltas
							double thisReturn = optimiseMcLevels[ix][thisDay][optCount] / startLevel;
							sprintf(lineBuffer, "%s%s%d%s%d%s%.4lf%s", lineBuffer, optFirstTime ? "(" : ",(", id, ",", optCount, ",", thisReturn, ")");
							optFirstTime  = false;
						}
					}
					// send final optimisation stub
					if (strlen(lineBuffer)) {
						mydb.prepare((SQLCHAR *)lineBuffer, 1);
					}
				}
			}
			else{
				// save product values for each iteration
				double thisNormalisation = 1.0;
				if (!optOptimiseAnnualisedReturn){ 
					// here we want product return = simulatedPV / averageSimulatedPV   ... so here we calc 1 / averageSimulatedPV
					thisNormalisation =  Mean(optimiseProductResult);
				}
				for (optCount=0; optCount < (int)optimiseProductResult.size(); optCount++){
					if (optCount % optMaxNumToSend == 0){
						// send batch
						if (optCount != 0){
							mydb.prepare((SQLCHAR *)lineBuffer, 1);
						}
						// init for next batch							
						strcpy(lineBuffer, "insert into productreturns (ProductId,Iteration,AnnRet,Duration,Payoff,ProjectedReturn) values ");
						optFirstTime = true;
					}
					sprintf(lineBuffer, "%s%s%d%s%d%s%.4lf%s%.4lf%s%.4lf%s%.4lf%s", lineBuffer, optFirstTime ? "(" : ",(", productId, ",", optCount, ",",
						optimiseProductResult[optCount] * thisNormalisation, ",", optimiseProductDuration[optCount], ",", optimiseProductPayoff[optCount], ",", 1.0, ")");
					optFirstTime  = false;
				}

				//	 debug only
				if (0){
					double geomReturn(0.0);
					double sumDuration(0.0);
					for (i = 0; i < (int)optimiseProductPayoff.size(); i++){
						double thisPayoff = optimiseProductPayoff[i];
						geomReturn += log((thisPayoff<unwindPayoff ? unwindPayoff : thisPayoff) / midPrice);
						sumDuration += optimiseProductDuration[i];
						// if (i<10){ std::cerr << "\n\tIteration:" << i << "Payoff:" << thisPayoff << "Duration:" << optimiseProductDuration[i] << "AnnRet:" << optimiseProductResult[i] << std::endl; }
					}
					geomReturn = exp(geomReturn / sumDuration) - 1;
					std::cerr << "\n\tCAGR:" << geomReturn << "off midPrice:" << midPrice << std::endl;
				}
				// send final optimisation stub
				if (strlen(lineBuffer)){
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
				}
			}
		}  // forOptimisation
		//
		// NORMAL processing ... NOT forOptimisation
		//
		else {
			// if we are doing accruals
			if (doAccruals){
				accruedCoupon = couponValue;  // store accrued coupon
				/*
				* save productcoupons
				*/
				strcpy(lineBuffer, "insert into productcoupons values ");
				int  i;
				bool found(false);
				// get coupon barriers, provided couponPaidOut
				if (couponPaidOut){
					for (i=0; i < numBarriers; i++){
						const SpBarrier&    ib(barrier.at(i));
						if (ib.capitalOrIncome == 0 && (int)ib.hitWithDate.size()>0){
							sprintf(lineBuffer, "%s%s%s%d%s%s%s%lf%s%s%s", lineBuffer, found ? "," : "", "(", productId, ",'", ib.hitWithDate[0].date.c_str(), "',", ib.hitWithDate[0].amount, ",'", productCcy.c_str(), "')");
							found = true;
						}
					}
				}

				// add any fixed coupons
				if ((int)couponFrequency.size()) {  // add fixed coupon
					int    freqLen      = (int)couponFrequency.length();
					char   freqChar     = toupper(couponFrequency[freqLen - 1]);
					std::string freqNumber = couponFrequency.substr(0, freqLen - 1);
					sprintf(charBuffer, "%s", freqNumber.c_str());
					double couponEvery  = atof(charBuffer);
					double daysPerEvery = freqChar == 'D' ? 1 : freqChar == 'M' ? 30 : daysPerYear;
					double couponPeriod = daysPerEvery*couponEvery;

					// create array of each coupon 
					int intCouponPeriod    = (int)floor(couponPeriod);
					boost::posix_time::ptime pTime(bProductStartDate), p1Time(bFixedCouponsDate);
					boost::gregorian::days  pDays(intCouponPeriod);

					for (pTime += pDays; pTime < p1Time; pTime += pDays){
						std::string aDate = to_iso_extended_string(pTime).substr(0, 10);
						storeFixedCoupons.push_back(SpPayoffAndDate(aDate.c_str(), fixedCoupon));
					}
					for (i=0; i < (int)storeFixedCoupons.size(); i++){
						sprintf(lineBuffer, "%s%s%s%d%s%s%s%lf%s%s%s", lineBuffer, found ? "," : "", "(", productId, ",'", storeFixedCoupons[i].date.c_str(), "',", storeFixedCoupons[i].amount, ",'", productCcy.c_str(), "')");
						found = true;
					}
				}
				if (found){
					sprintf(charBuffer, "%s%d", "delete from productcoupons where productid=", productId);
					mydb.prepare((SQLCHAR *)charBuffer, 1);
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
				}

				if (matured){
					const SpBarrier&    b(barrier.at(maturityBarrier));
					double thisAmount    = issuePrice * (b.hitWithDate[0].amount); // -(couponPaidOut ? b.couponValues[0] : 0.0));
					productHasMatured    = true;
					// save capital payoff
					std::string rider(extendingPrices ? "possibly" : "");
					sprintf(lineBuffer, "%s%s%s%s%s%lf%s%s%s%s%s%d", "update product set ", rider.c_str(), "Matured=1,", rider.c_str(), "MaturityPayoff=", thisAmount, ",", rider.c_str(), "DateMatured='", b.settlementDate.c_str(), "' where productid=", productId);
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
				}
			} // END doAccruals
			//
			// not doing accruals
			//
			else {
				if (0 && (int)debugCorrelatedRandNos.size() > 2) {
					double thisMean, thisStd, thisStderr;
					MeanAndStdev(debugCorrelatedRandNos, thisMean, thisStd, thisStderr);
					sprintf(lineBuffer, "%s%.4lf", "Simulated average randnos:", thisMean);
					std::cout << lineBuffer << std::endl;
				}

				// check simulated drifts
				double thisMean, thisStd, thisStderr;
				if (!silent) {
					for (i = 0; i < numUl; i++) {
						MeanAndStdev(simulatedReturnsToMaxYears[i], thisMean, thisStd, thisStderr);
						std::cout << "Simulated annualised drift rate (inc. quanto) to:" << ulPrices[i].date[startPoint + productDays] << " :" << exp(365.0*log(thisMean) / productDays) << " for:" << ulNames[i] << " spot:" << spotLevels[i] << " strikeDateLevel:" << strikeDateLevels[i] << " moneyness:" << spotLevels[i]/strikeDateLevels[i] << std::endl;
						MeanAndStdev(simulatedLogReturnsToMaxYears[i], thisMean, thisStd, thisStderr);
						std::cout << "Simulated vol to:" << ulPrices[i].date[startPoint + productDays] << " :" << thisStd / sqrt(productDays / 365) << " for:" << ulNames[i] << std::endl;
						MeanAndStdev(simulatedLevelsToMaxYears[i], thisMean, thisStd, thisStderr);
						std::cout << "Simulated final level (inc. quanto) at:" << ulPrices[i].date[startPoint + productDays] << ":" << thisMean << "(" << thisMean / spotLevels[i] << ") for:" << ulNames[i] << " (" << productDays / 365.0 << "y):" << std::endl;
					}
				}


				//
				//  count allEpisodes CAPITAL episodes
				//
				int    numAllEpisodes(0);
				bool hasProportionalAvg(false), hasCountAvg(false);   // no couponHistogram, which only shows coupon-counts
				for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
					const SpBarrier&    b(barrier.at(thisBarrier));
					if (b.capitalOrIncome) {
						numAllEpisodes += (int)b.hit.size();
					}
					// hasProportionalAvg = hasProportionalAvg || barrier.at(thisBarrier).proportionalAveraging;
					// hasCountAvg        = hasCountAvg        || barrier.at(thisBarrier).countAveraging;
				}
				//
				// log unusual #episodes
				//
				if (numMcIterations>1 && 
					  (
					    (issuerCallable && (numAllEpisodes + (numBurnInIterations * (doTurkey ? 2.0 : 1.0)) != thisIteration)) 
						|| 
						(!issuerCallable && numAllEpisodes != thisIteration)
					  )
					){
					numAllEpisodes = thisIteration;
					sprintf(lineBuffer, "%s%d%s%d%s", "insert into cpluspluslog (DateTime,ProductId,UserId,Msg) values(now(),", productId, ",", userId, ",' **** BEWARE - STRONGLY SUGGEST YOU INVESTIGATE  ****  seems like Capital events do not cover 100% of possible events')");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);					
				}
				// couponHistogram
				if (!usingProto && !getMarketData && !doPriips && ukspaCase == ""){
					// ** delete old
					sprintf(lineBuffer, "%s%d%s%d%s", "delete from couponhistogram where ProductId='", productId, "' and IsBootstrapped='", numMcIterations == 1 ? 0 : 1, "'");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
					if (/* !hasProportionalAvg && !hasCountAvg && */ numIncomeBarriers){
						// ** insert new
						for (int thisNumHits=0; thisNumHits < (int)numCouponHits.size(); thisNumHits++){
							sprintf(lineBuffer, "%s%d%s%d%s%.5lf%s%d%s",
								"insert into couponhistogram (ProductId,NumCoupons,Prob,IsBootstrapped) values (", productId, ",",
								thisNumHits, ",", ((double)numCouponHits[thisNumHits]) / numAllEpisodes, ",", numMcIterations == 1 ? 0 : 1, ")");
							mydb.prepare((SQLCHAR *)lineBuffer, 1);
						}
					}
				}

				// doFinalAssetReturn
				char farBuffer[100000], farBuffer1[100000];
				int  farCounter(0), totalFarCounter(0);
				if (doFinalAssetReturn && !usingProto  && !getMarketData && !doPriips){
					strcpy(farBuffer, "insert into finalassetreturns values ");
					sprintf(lineBuffer, "%s%d%s", "delete from finalassetreturns where productid='", productId, "'");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
					strcpy(lineBuffer, "delete from finalassetinfo");
					mydb.prepare((SQLCHAR *)lineBuffer, 1);
					strcpy(farBuffer1, "insert into finalassetinfo values");
				}


				// process results
				const double tol = 1.0e-6;
				std::string      lastSettlementDate = barrier.at(numBarriers - 1).settlementDate;
				double   actualRecoveryRate = depositGteed ? 0.9 : (collateralised ? 0.9 : recoveryRate);
				double maxYears(0.0); for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){ double ytb=barrier.at(thisBarrier).yearsToBarrier; if (ytb>maxYears){ maxYears = ytb; } }
				// credit-analyse for possibly multiIssuer
				for (int thisIssuerIndx = issuerIndx; 
					!(multiIssuer && issuerCallable && thisIssuerIndx > issuerIndx) && thisIssuerIndx < (int)hazardCurves.size();
					thisIssuerIndx++) {
					const std::vector<double> &hazardCurve(hazardCurves[thisIssuerIndx]);
					const std::vector<double> &cdsTenor(cdsTenors[thisIssuerIndx]);
					const std::vector<double> &cdsSpread(cdsSpreads[thisIssuerIndx]);
					if (thisIssuerIndx > 0) { updateCashflows = false; }
					// analyse each results case
					for (int analyseCase = 0; analyseCase < (doPriips || getMarketData || useUserParams ? 1 : 2); analyseCase++) {
						if (doDebug  && debugLevel >= 2) { std::cerr << "Starting analyseResults  for case \n" << analyseCase << std::endl; }
						bool     applyCredit = analyseCase == 1;
						std::map<int, double> cashflowMap;
						double   projectedReturn = (numMcIterations == 1 ? (applyCredit ? 0.05 : 0.0) : (doPriips ? 0.08 : (applyCredit ? 0.02 : 1.0)));
						if      (ukspaCase == "Bear"        ) { projectedReturn = 0.1; }
						else if (ukspaCase == "Neutral"     ) { projectedReturn = 0.2; }
						else if (ukspaCase == "Bull"        ) { projectedReturn = 0.3; }
						if (getMarketData && ukspaCase == "") { projectedReturn = 0.4; }
						if (useUserParams                   ) { projectedReturn = 0.5; }
						if (doTurkey                        ) { projectedReturn = (applyCredit ? 0.2 : 0.1 ); }   // ARROW has no current interest in the UKSPA analysis
						if (applyCredit) { ArtsRanInit(); }  // so multiIssuer analysis always sees the same ran sequence
						bool     foundEarliest = false;
						double   probEarly(0.0), probEarliest(0.0);
						std::vector<double> allPayoffs, allT,allFVpayoffs, allAnnRets, bmAnnRets, bmRelLogRets, pvInstances;
						std::vector<PriipsStruct> priipsInstances;
						std::vector<AnnRet> priipsAnnRetInstances;
						if (doPriips) {
							priipsInstances.reserve(numMcIterations);
							priipsAnnRetInstances.reserve(numMcIterations);
						}

						int    numPosPayoffs(0), numStrPosPayoffs(0), numNegPayoffs(0);
						double sumPosPayoffs(0), sumStrPosPayoffs(0), sumNegPayoffs(0);
						double sumPosDurations(0), sumStrPosDurations(0), sumNegDurations(0), sumYearsToBarrier(0);
						// most likely barrier
						double maxBarrierProb(0.0), maxBarrierProbMoneyness(0.0), maxFirstKoMoneyness(0.0), maxFirstKoReturn(0.0);
						bool doMostLikelyBarrier(analyseCase == 0 || analyseCase == 1);

						// DEBUG: how many capital hits
						if (false) {
							int checkNumHits(0);
							for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++) {
								const SpBarrier&    b(barrier.at(thisBarrier));
								if (b.capitalOrIncome && b.yearsToBarrier >= 0.0) {
									int numHits = (int)b.hit.size();
									checkNumHits += numHits;
								}
							}
						}
						
						// ** process barrier results
						double eStrPosPayoff(0.0), ePosPayoff(0.0), eNegPayoff(0.0), sumPayoffs(0.0), sumAnnRets(0.0), sumCouponRets(0.0), sumParAnnRets(0.0), sumDuration(0.0), sumPossiblyCreditAdjPayoffs(0.0);
						int    numCapitalInstances(0), numStrPosInstances(0), numPosInstances(0), numNegInstances(0), numParInstances(0);
						for (int thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++) {
							if (doDebug  && debugLevel >= 2) { std::cerr << "Starting analyseResults  for barrier \n" << thisBarrier << std::endl; }
							const SpBarrier&    b(barrier.at(thisBarrier));
							int                 numHits         = (int)b.hit.size();
							double              thisBarrierSumPayoffs(0.0), thisAmount;
							std::vector<double> thisBarrierPayoffs; thisBarrierPayoffs.reserve(100000);
							// thisBarrierCouponValues[] seems unnecessary: just stores b.couponValues[i] ... only to be used a few lines later where b.couponValues[i] would have been fine
							//  ... DOME: maybe remove when time allows ... just use b.couponValues[i] a few lines down
							std::vector<double> thisBarrierCouponValues; thisBarrierCouponValues.reserve(100000);
							int                 numInstances    = (int)b.hit.size();
							double              sumProportion   = b.sumProportion;
							double              thisYears       = b.yearsToBarrier;
							double              prob;
							double              thisProbDefault = probDefault(hazardCurve, thisYears);
							int                 yearsAsMapKey   = (int)floor(b.yearsToBarrier * YEARS_TO_INT_MULTIPLIER);

							prob            = b.hasBeenHit ? 1.0 : (b.isCountAvg ? (b.totalNumPossibleHits <=0 ? 0.0 : numHits * sumProportion / b.totalNumPossibleHits / numAllEpisodes) : sumProportion / numAllEpisodes); // REMOVED: eg Memory coupons as in #586 (b.endDays < 0 ? 1 : numAllEpisodes); expired barriers have only 1 episode ... the doAccruals.evaluate()

							//
							// look at each 'hit'
							//
							for (i = 0; i < numHits; i++) {
								thisAmount = b.hit[i].amount;
								// possibly apply credit adjustment
								if (applyCredit) {
									double thisRecoveryRate;
									thisRecoveryRate = /*((double)rand() / RAND_MAX)*/ ArtsRan() < thisProbDefault ? actualRecoveryRate : 1.0;
									thisAmount      *= thisRecoveryRate;
									if (b.capitalOrIncome && b.yearsToBarrier >= 0.0) {
										if (thisAmount > midPrice)  { eStrPosPayoff  += thisAmount; numStrPosInstances++; }
										if (thisAmount >= midPrice) { ePosPayoff     += thisAmount; numPosInstances++; }
										else { eNegPayoff     += thisAmount; numNegInstances++; }
									}
								}
								thisBarrierCouponValues.push_back(b.couponValues[i]);  // pointless? see comment above
								thisBarrierPayoffs.push_back(thisAmount);
								thisBarrierSumPayoffs += thisAmount;   // but not if couponPaidOut
							}

							// MUST recalc discountRate as b.yearsToBarrier may have changed when bumpTheta  b.bumpSomeDays()
							double thisDiscountRate   = b.forwardRate + fundingFraction * interpCurve(cdsTenor, cdsSpread, b.yearsToBarrier);
							double thisDiscountFactor = pow(thisDiscountRate, -(b.yearsToBarrier - forwardStartT));

							if (b.capitalOrIncome && b.yearsToBarrier >= 0.0) {
								// first-hit capital barrier
								if (!foundEarliest) {
									foundEarliest = true;
									probEarliest  = prob;
									// calc distance to first non-negative-participation barrier for each underlying
									// ... MAX Barrier as %spot if Nature=AND, to see how far the laggard has to go
									// ... MIN Barrier as %spot if Nature=OR,  to see how far the leader has to go
									if (updateCashflows && doMostLikelyBarrier && b.participation >= 0.0) {
										double direction = b.isAnd ? -1.0 : 1.0;
										maxFirstKoMoneyness = 100.0*direction;
										for (int j = 0, len=(int)b.brel.size(); j < len; j++) {
											const SpBarrierRelation&    thisBrel(b.brel.at(j));
											double thisMoneyness = thisBrel.barrier / thisBrel.moneyness;
											if (direction*(thisMoneyness - maxFirstKoMoneyness) < 0.0) {
												maxFirstKoMoneyness = thisMoneyness;
												maxFirstKoReturn    = b.payoff / midPrice - 1.0;
											}
										}
										if (fabs(maxFirstKoMoneyness) > 100000.0) { maxFirstKoMoneyness  = (maxFirstKoMoneyness > 0.0 ? 1.0 : -1.0) * 100000.0 ; }
										sprintf(lineBuffer, "%s%s%s%.5lf%s%.5lf%s%d%s%.5lf", "update ", useProto, "cashflows set MaxFirstKoMoneyness='", maxFirstKoMoneyness - 1.0,
											"',MaxFirstKoReturn='", maxFirstKoReturn,
											"' where ProductId='", productId, "' and ProjectedReturn=", projectedReturn);
										mydb.prepare((SQLCHAR *)lineBuffer, 1);
									}
								}
								if (b.settlementDate < lastSettlementDate) { probEarly   += prob; }
								sumDuration                 += numInstances * thisYears;
								numCapitalInstances         += numInstances;
								sumPayoffs                  += b.sumPayoffs;
								sumPossiblyCreditAdjPayoffs += thisBarrierSumPayoffs;
								if (!applyCredit) {
									eStrPosPayoff    += b.sumStrPosPayoffs; numStrPosInstances += b.numStrPosPayoffs;
									ePosPayoff       += b.sumPosPayoffs;    numPosInstances    += b.numPosPayoffs;
									eNegPayoff       += b.sumNegPayoffs;    numNegInstances    += b.numNegPayoffs;
								}
								if (doMostLikelyBarrier && b.participation < 0.0 && !b.isAnd) {
									if (prob >= maxBarrierProb) {
										maxBarrierProb          = prob;
										maxBarrierProbMoneyness = -100.0;
										// want the HIGHEST barrier as %spot
										for (int j = 0, len=(int)b.brel.size(); j < len; j++) {
											const SpBarrierRelation&    thisBrel(b.brel.at(j));
											double thisMoneyness = thisBrel.barrier / thisBrel.moneyness;
											if (thisMoneyness > maxBarrierProbMoneyness) { maxBarrierProbMoneyness = thisMoneyness; }
										}
									}
								}
								for (i = 0; i < (int)b.hit.size(); i++) {
									double thisAmount      = thisBarrierPayoffs[i];
									double thisAnnRet      = thisYears <= 0.0 ? 0.0 : 	// assume once investor has lost 90% it is unwound...									
										min(LARGE_RETURN, 
											b.aimForHeadlineAnnRet && daysExtant <= 0 && b.payoffType == "fixed" && !b.payoffContainsVariableCoupons[i] ?
											(thisAmount / midPrice - 1.0) / thisYears  // no compounding - for folks that like headline rates
											:
											exp(log((thisAmount < unwindPayoff ? unwindPayoff : thisAmount) / midPrice) / thisYears) - 1.0
										);
									if (thisAnnRet < -0.9999) { thisAnnRet = -0.9999; } // avoid later problems with log(1.0+annRets)
									double thisCouponValue = thisBarrierCouponValues[i];
									double thisCouponRet   = thisYears <= 0.0 || ((int)couponFrequency.size() == 0 && numIncomeBarriers == 0) ? 0.0 : (thisCouponValue < -1.0 ? -1.0 : exp(log((1.0 + thisCouponValue) / midPrice) / thisYears) - 1.0);

									// maybe save finalAssetReturns
									if (doFinalAssetReturn && !usingProto && !getMarketData && !applyCredit && totalFarCounter < 400000 && !doPriips) {  // DOME: this is 100 iterations, with around 4000obs per iteration ... in many years time this limit needs to be increased!
										if (farCounter) { strcat(farBuffer, ","); strcat(farBuffer1, ","); }
										sprintf(farBuffer, "%s%s%d%s%.3lf%s%.3lf%s%.3lf%s", farBuffer, "(", productId, ",", thisAmount, ",", b.fars[i].ret, ",", thisYears, ")");
										sprintf(farBuffer1, "%s%s%d%s%.3lf%s%.3lf%s%d%s%d%s", farBuffer1, "(", productId, ",", thisAmount, ",", b.fars[i].ret,
											",", b.fars[i].assetIndx, ",", b.fars[i].barrierIndx, ")");
										farCounter += 1;
										if (farCounter == 100) {
											totalFarCounter += farCounter;
											strcat(farBuffer, ";"); strcat(farBuffer1, ";");
											mydb.prepare((SQLCHAR *)farBuffer, 1);
											mydb.prepare((SQLCHAR *)farBuffer1, 1);
											strcpy(farBuffer, "insert into finalassetreturns values ");
											strcpy(farBuffer1, "insert into finalassetinfo values");
											farCounter = 0;
										}
									}

									allPayoffs.push_back(thisAmount);
									allT.push_back(thisYears);
									allFVpayoffs.push_back(thisAmount*pow(b.forwardRate, maxYears - b.yearsToBarrier));
									allAnnRets.push_back(thisAnnRet);

									/*
									* update cashflow map -
									*/
									cashflowMap[yearsAsMapKey] += thisAmount;


									// pv payoffs
									if (getMarketData) {
										pvInstances.push_back((thisAmount)*thisDiscountFactor);
									}
									if (doPriips  && analyseCase == 0) {
										double thisT      = b.yearsToBarrier;
										double thisReturn = thisAmount / midPrice;
										priipsInstances.push_back(PriipsStruct(thisReturn*pow(1.0 + priipsRfr, -thisT), thisT));
										priipsAnnRetInstances.push_back(AnnRet(thisAnnRet, thisT));
									}
									double bmRet = min(LARGE_RETURN, thisYears <= 0.0 ? 0.0 : (benchmarkId > 0 ? exp(log(b.bmrs[i]) / thisYears - contBenchmarkTER) - 1.0 : hurdleReturn));
									if (isnan(bmRet)) {
										int jj = 1;
									}

									if (bmRet < (unwindPayoff - 1.0)) { bmRet = (unwindPayoff - 1.0); }
									bmAnnRets.push_back(bmRet);
									sumYearsToBarrier += thisYears;
									double tempBmRelLogRet = log((thisAmount < unwindPayoff ? unwindPayoff : thisAmount) / midPrice) - log(1 + bmRet)*thisYears;
									if (isnan(tempBmRelLogRet)) {
										int jj = 1;
									}
									bmRelLogRets.push_back(tempBmRelLogRet);
									sumAnnRets += thisAnnRet;
									sumCouponRets += thisCouponRet;

									if (thisAnnRet > -tol && thisAnnRet < tol) { sumParAnnRets    += thisAnnRet; numParInstances++; }
									if (thisAnnRet > 0.0)                      { sumStrPosPayoffs += thisAmount; numStrPosPayoffs++; sumStrPosDurations += thisYears; }
									if (thisAnnRet > -tol)                     { sumPosPayoffs    += thisAmount; numPosPayoffs++;    sumPosDurations    += thisYears; }
									else                                       { sumNegPayoffs    += thisAmount; numNegPayoffs++;    sumNegDurations    += thisYears; }
								}
							}

							//
							// having examined/logged each 'hit', calc some metrics
							//
							double mean      = numInstances ? thisBarrierSumPayoffs / numInstances : 0.0;
							// watch out: b.yearsToBarrier might be zero, or returnToAnnualise might be negative (if a product capital barrier was entered that way...
							double returnToAnnualise = ((b.capitalOrIncome ? 0.0 : 1.0) + mean) / midPrice;
							double annReturn         = returnToAnnualise > 0.0 && numInstances && b.yearsToBarrier > 0 && midPrice > 0 ? (exp(log(returnToAnnualise) / b.yearsToBarrier) - 1.0) : 0.0;
							// if you get 1.#INF or inf, look for overflow or division by zero. If you get 1.#IND or nan, look for illegal operations
							if (!silent && thisYears >= 0.0 && prob > 0.0) {
								sprintf(lineBuffer, "%s%-20s%s%.5lf%s%.5lf%s%.5lf%s%.5lf%s%.5lf%s%.5lf%s%.2lf%s%.2lf%s%.2lf",
									"EventProbabilityAndPayoff: ",
									b.description.c_str(), ": Prob:",
									prob, ": ConditionalPayoff:",
									mean, ": ExpPayoff:",
									mean*prob, ": DiscFact:",
									thisDiscountFactor, ": PV(%):",
									mean*prob*thisDiscountFactor, ": DiscRate:",
									thisDiscountRate, ": fwdRate:",
									b.forwardRate, ": ffract:",
									fundingFraction, ": y:",
									b.yearsToBarrier
								);
								std::cout << lineBuffer << std::endl;
							}
							// ** SQL 
							// ** WARNING: keep the "'" to delimit SQL values, in case a #INF or #IND sneaks in - it prevents the # char being seem as a comment, with disastrous consequences


							// if (updateCashflows && ((!getMarketData && !useUserParams) || analyseCase == 0) && (!doPriips || (priipsUsingRNdrifts && !doPriipsStress))) 
							// just save to barrierprob table regardless, esp as we are now also interested in the credit-adjusted numbers for Arrow
							{
								sprintf(lineBuffer, "%s%s%s%.5lf%s%.5lf%s%.5lf%s%d", "update ", useProto, "barrierprob set Prob='", prob,
									"',AnnReturn='", annReturn > 10.0 ? 10.0 : annReturn,
									"',CondPayoff='", mean,
									"',NumInstances='", numInstances);
								if (getMarketData && ukspaCase == "") {
									sprintf(lineBuffer, "%s%s%.5lf%s%.5lf%s%.5lf", lineBuffer, "',NonCreditPayoff='", b.yearsToBarrier, "',Reason1Prob='", thisDiscountRate, "',Reason2Prob='", thisDiscountFactor);
								}
								sprintf(lineBuffer, "%s%s%d%s%.2lf%s", lineBuffer, "' where ProductBarrierId='", barrier.at(thisBarrier).barrierId, "' and ProjectedReturn='", projectedReturn, "'");
								if (doDebug  && debugLevel > 4) {
									FILE * pFile;
									pFile = fopen("debug.txt", "a");
									fprintf(pFile, "%s\n", lineBuffer);
									fclose(pFile);
								}
								if (doDebug && debugLevel > 4) {
									std::cerr << updateCashflows << ":" << getMarketData << ":" << useUserParams << ":" << analyseCase << ":" << doPriips << ":" << priipsUsingRNdrifts << ":" << doPriipsStress << std::endl;
									std::cerr << lineBuffer << std::endl;
								}
								mydb.prepare((SQLCHAR *)lineBuffer, 1);
							}
							// save PRIIPS-riskNeutral-drift barrierProb somewhere
							if (priipsUsingRNdrifts && !doPriipsStress && analyseCase == 0) {
								sprintf(lineBuffer, "%s%s%s%.5lf", "update ", useProto, "barrierprob set Reason1Prob='", prob);
								sprintf(lineBuffer, "%s%s%d%s%.2lf%s", lineBuffer, "' where ProductBarrierId='", barrier.at(thisBarrier).barrierId, "' and ProjectedReturn='", projectedReturn, "'");
								mydb.prepare((SQLCHAR *)lineBuffer, 1);
							}


						}
						if (updateCashflows && doMostLikelyBarrier && maxBarrierProb >= 0.0) {
							sprintf(lineBuffer, "%s%s%s%.5lf%s%.5lf%s%d%s%.5lf", "update ", useProto, "cashflows set MaxBarrierProb='", maxBarrierProb,
								"',MaxBarrierProbMoneyness='", 1.0 - maxBarrierProbMoneyness,
								"' where ProductId='", productId, "' and ProjectedReturn=", projectedReturn);
							mydb.prepare((SQLCHAR *)lineBuffer, 1);
						}

						if (numPosInstances > 0) { ePosPayoff    /= numPosInstances; }
						if (numStrPosInstances > 0) { eStrPosPayoff /= numStrPosInstances; }
						if (numNegInstances > 0) { eNegPayoff    /= numNegInstances; }

						int numAnnRets((int)allAnnRets.size());
						if (numAnnRets == 0) {
							std::cerr << "Product seems not to have hit ANY barriers " << "\n";
							evalResult.errorCode = 10001;
							return(evalResult);
						}
						double duration  = sumDuration / numAnnRets;
						// RJ CAGR risk
						// ... sqrt of weightedAverage of weightedAnnRetDeviations
						//
						double cagrMean, cagrStdev, cagrStderr;
						MeanAndStdev(allAnnRets, cagrMean, cagrStdev, cagrStderr);
						double cagrRisk(0.0),sumCagrWeights(0.0);
						for (i=0; i < numAnnRets;i++) {
							double thisWeight          = sqrt(allT[i]);
							double thisAnnRetDeviation = allAnnRets[i] - cagrMean;
							cagrRisk        += thisAnnRetDeviation * thisAnnRetDeviation * thisWeight;
							sumCagrWeights  += thisWeight;
						}
						cagrRisk = sqrt(cagrRisk / sumCagrWeights);

						//
						// ******* winlose ratios for different cutoff returns
						//
						// ...two ways to do it
						// ...first recognises the fact that a 6y annuity is worth more than a 1y annuity
						// ...second assumes annualised returns have equal duration
						bool doWinLoseAnnualised = true; // as you want
						if (analyseCase == 0 && !usingProto && !getMarketData && !doPriips) {
							if (doDebug  && debugLevel > 4) { std::cerr << "Starting analyseResults WinLose for case \n" << analyseCase << std::endl; }
							sprintf(lineBuffer, "%s%d%s", "delete from winlose where productid='", productId, "';");
							mydb.prepare((SQLCHAR *)lineBuffer, 1);
							double winLoseMinRet       =  doWinLoseAnnualised ? -0.20 : 0.9;
							const double winLoseMaxRet =  doWinLoseAnnualised ? 0.21 : 2.0;
							const std::vector<double> &theseWinLoseMeasures = doWinLoseAnnualised ? allAnnRets : allFVpayoffs;
							const double thisWinLoseDivisor  = doWinLoseAnnualised ? 1.0 : midPrice;
							const double thisWinLoseClick    = doWinLoseAnnualised ? 0.01 : 0.05;
							const double thisWinLoseVolScale = doWinLoseAnnualised ? sqrt(duration) : 1;
							while (winLoseMinRet < winLoseMaxRet) {
								double sumWinLosePosPayoffs = 0.0;
								double sumWinLoseNegPayoffs = 0.0;
								int    numWinLosePosPayoffs = 0;
								int    numWinLoseNegPayoffs = 0;
								for (j = 0, len=(int)allAnnRets.size(); j < len; j++) {
									double payoff   = theseWinLoseMeasures[j] / thisWinLoseDivisor - winLoseMinRet;
									if (payoff > 0) { sumWinLosePosPayoffs += payoff; numWinLosePosPayoffs += 1; }
									else { sumWinLoseNegPayoffs -= payoff; numWinLoseNegPayoffs += 1; }
								}
								double winLose        = numWinLoseNegPayoffs ? (thisWinLoseVolScale*sumWinLosePosPayoffs / sumWinLoseNegPayoffs) : 1000.0;
								if (winLose > 1000.0) { winLose = 1000.0; }

								sprintf(lineBuffer, "%s%d%s%.4lf%s%.6lf%s",
									"insert into winlose values (", productId, ",", 100.0*winLoseMinRet, ",", winLose, ");");
								mydb.prepare((SQLCHAR *)lineBuffer, 1);
								winLoseMinRet += thisWinLoseClick;
							}

						}
						//
						// ******* END: winlose ratios for different cutoff returns
						//


						//
						// possibly track timepoints
						//
						if (doTimepoints && analyseCase == 0 && !usingProto && !getMarketData && !doPriips) {
							for (i=0; i < numTimepoints; i++) {
								int thisTpDays   = timepointDays[i];
								std::string name = timepointNames[i];
								for (j=0; j < numUls; j++) {
									bool firstTime      = true;
									sprintf(lineBuffer, "%s", "insert into timepoints (UserId,ProductId,UnderlyingId,TimePointDays,Name,Percentile,SimValue) values ");
									int ulId = ulIds[j];
									sort(timepointLevels[i][j].begin(), timepointLevels[i][j].end());
									int numTpLevels = (int)timepointLevels[i][j].size();
									// save simPercentiles
									for (k=0, len=(int)simPercentiles.size(); k < len; k++) {
										double pctile = simPercentiles[k];
										if (firstTime) { firstTime = false; }
										else { strcat(lineBuffer, ","); }
										int thisIndx = (int)floor(numTpLevels*simPercentiles[k]);
										double value = timepointLevels[i][j][thisIndx];
										sprintf(lineBuffer, "%s%s%d%s%d%s%d%s%s%s%lf%s%lf%s", lineBuffer, "(3,", productId, ",", ulId, ",", thisTpDays, ",'", name.c_str(), "',", pctile, ",", value, ")");
									}
									mydb.prepare((SQLCHAR *)lineBuffer, 1);
								}
							}
						}


						//
						// benchmark underperformance   NOT STRICT
						//
						// NOTE: these are ARITHMETIC averages, whereas ecGain and ecLoss are essentially CAGRs
						double benchmarkProbUnderperf(0.0), benchmarkCondUnderperf(0.0), benchmarkProbOutperf(0.0), benchmarkCondOutperf(0.0);
						double bmRelUnderperfPV(0.0), bmRelOutperfPV(0.0), bmRelCAGR(0.0), cumUnderperfPV(0.0), cumOutperfPV(0.0);
						double cumValue = 0.0, cumValue1 = 0.0;
						int    cumCount = 0, cumCount1 = 0;
						cumUnderperfPV = 0.0;
						cumOutperfPV   = 0.0;
						bmRelCAGR      = 0.0;
						for (i=0; i < numAnnRets; i++) {
							double anyDouble = allAnnRets[i] - bmAnnRets[i];
							if (anyDouble < 0.0) {
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
							bmRelUnderperfPV       = MyMinAbs(1000.0, cumUnderperfPV / cumCount);
						}
						if (cumCount1) {
							benchmarkProbOutperf = ((double)cumCount1) / numAnnRets;
							benchmarkCondOutperf = cumValue1 / cumCount1;
							bmRelOutperfPV       = MyMinAbs(1000.0, cumOutperfPV / cumCount1);
						}
						bmRelCAGR       = sumYearsToBarrier > 0.0 ? exp(bmRelCAGR / sumYearsToBarrier) - 1.0 : 0.0;
						bmRelCAGR       = MyMinAbs(1000.0, bmRelCAGR);


						// *******
						// maybe save productreturns
						// ... BEFORE SORTING, as allT is not sorted
						// *******
						if (doDebug  && debugLevel == 3) {
							sprintf(lineBuffer, "%s%d%s%lf", "delete from productreturns where ProductId=", productId, " and ProjectedReturn=", projectedReturn);
							mydb.prepare((SQLCHAR *)lineBuffer, 1);
							bool firstTime;
							int thisCount;
							for (thisCount=0; thisCount < numAnnRets; thisCount++) {
								if (thisCount % optMaxNumToSend == 0) {
									// send batch
									if (thisCount != 0) {
										mydb.prepare((SQLCHAR *)lineBuffer, 1);
									}
									// init for next batch							
									strcpy(lineBuffer, "insert into productreturns (ProductId,Iteration,AnnRet,Duration,Payoff,ProjectedReturn) values ");
									firstTime = true;
								}
								sprintf(lineBuffer, "%s%s%d%s%d%s%.4lf%s%.4lf%s%.4lf%s%.4lf%s", lineBuffer, firstTime ? "(" : ",(", productId, ",", thisCount, ",",
									allAnnRets[thisCount], ",", allT[thisCount], ",", allPayoffs[thisCount], ",", projectedReturn, ")");
								firstTime  = false;
							}
							// send batch
							if (thisCount != 0) {
								mydb.prepare((SQLCHAR *)lineBuffer, 1);
							}
						}

						// *******************
						// ******* process overall product results
						// *******************
						sort(allPayoffs.begin(),      allPayoffs.end());
						sort(allAnnRets.begin(),      allAnnRets.end());
						if (doPriips) {
							sort(priipsInstances.begin(), priipsInstances.end());
							sort(priipsAnnRetInstances.begin(), priipsAnnRetInstances.end());
						}

						double averageReturn        = sumAnnRets / numAnnRets;
						double averageCouponReturn  = sumCouponRets / numAnnRets;
						double vaR99                = 100.0*allAnnRets[(unsigned int)floor((double)numAnnRets*(0.01))];
						double vaR90                = 100.0*allAnnRets[(unsigned int)floor((double)numAnnRets*(0.1))];
						double vaR50                = 100.0*allAnnRets[(unsigned int)floor((double)numAnnRets*(0.5))];
						double vaR10                = 100.0*allAnnRets[(unsigned int)floor((double)numAnnRets*(0.9))];
						double priipsStressVar(-1.0), priipsStressYears(-1.0), varYears(0.0), var1Years(0.0), var2Years(0.0);
						if (doPriips) {
							double thisStressT = maxProductDays > 365 ? 0.05 : 0.01;
							if (doPriipsStress) {
								PriipsStruct &thisPriip(priipsInstances[(unsigned int)floor(numAnnRets*thisStressT)]);
								priipsStressVar   = thisPriip.pvReturn;
								priipsStressYears = thisPriip.yearsToPayoff;
							}
							varYears  = priipsAnnRetInstances[(unsigned int)floor(numAnnRets*(0.1))].yearsToPayoff;
							var1Years = priipsAnnRetInstances[(unsigned int)floor(numAnnRets*(0.5))].yearsToPayoff;
							var2Years = priipsAnnRetInstances[(unsigned int)floor(numAnnRets*(0.9))].yearsToPayoff;
						}
						// SRRI vol
						struct srriParams { double conf, normStds, normES; };

						srriParams shortfallParams[] ={ // in R,normalExpectedShorfall = vol*dnorm(qnorm(prob))/prob with prob=0.05 for 95%conf
							{ 0.975, 1.96, 2.3378 },
							{ 0.99, 2.326, 2.665 },
							{ 0.999, 3.09, 3.367 },
						};
						i=0;
						double srriConf, srriStds, srriConfRet, normES, cesrStrictVol;
						// some products have an averageReturn somewhat to the left of the 2.5% VaR, so we need to use a more extreme percentile
						do {
							srriConf      = shortfallParams[i].conf;
							srriStds      = shortfallParams[i].normStds;
							normES        = shortfallParams[i].normES;
							srriConfRet   = allAnnRets[(unsigned int)floor(numAnnRets*(1 - srriConf))];
							if (i == 0) { cesrStrictVol = -(srriStds - sqrt(srriStds*srriStds + 2 * (log(1 + averageReturn) - log(1 + srriConfRet)))); }
							i += 1;
						} while (srriConfRet > averageReturn && i < 3);


						//if(srriConfRet>historicalReturn) {srriConf = 1-1.0/numAnnRets;srriStds = -NormSInv(1.0/numAnnRets);  srriConfRet = annRetInstances[0];}
						// replaced the following line with the next one as 'averageReturn' rather than 'historicalReturn' is how we do ESvol
						// BUT DO NOT DELETE as 'historicalReturn' may be the better way: DOME
						//var srriVol        = -100*(srriStds - Math.sqrt(srriStds*srriStds+4*0.5*(Math.log(1+historicalReturn/100) - Math.log(1+srriConfRet/100))));
						double srriVol       = (1 + srriConfRet) > 0.0 ? -(srriStds - sqrt(srriStds*srriStds + 2 * (log(1 + averageReturn) - log(1 + srriConfRet)))) : 1000.0;





						// pctiles and other calcs
						if (numMcIterations > 1 && analyseCase == 0 && !usingProto && !getMarketData) {
							if (doDebug  && debugLevel > 4) { std::cerr << "Starting analyseResults PcTile for case \n" << analyseCase << std::endl; }

							// pctiles
							double bucketSize = productShape == "Supertracker" ? 0.05 : 0.01;
							double minReturn  = 0.05*floor(allAnnRets[0] / 0.05);
							std::vector<double>    returnBucket;
							std::vector<double>    bucketProb;
							for (i = j = 0; i < numAnnRets;) {
								double thisRet = allAnnRets[i];
								if (thisRet < minReturn || thisRet > 0.4) {
									j += 1; i += 1;
								}  // final bucket for any return above 40%pa
								else {
									returnBucket.push_back(minReturn);
									bucketProb.push_back(((double)j) / numAnnRets);
									j = 0;
									minReturn += bucketSize;
									// make sure minReturn of zero is exactly zero ... machine rounding problem
									if (minReturn < tol && minReturn > -tol) {
										minReturn = 0.0;
									}
								}
							}
							returnBucket.push_back(minReturn); bucketProb.push_back(((double)j) / numAnnRets);
							sprintf(lineBuffer, "%s%d%s%lf%s", "delete from pctiles where productid='", productId, "' and ProjectedReturn=", projectedReturn, ";");
							mydb.prepare((SQLCHAR *)lineBuffer, 1);

							sprintf(lineBuffer, "%s", "insert into pctiles values ");
							for (i=0; i < (int)returnBucket.size(); i++) {
								if (i != 0) { sprintf(lineBuffer, "%s%s", lineBuffer, ","); }
								sprintf(lineBuffer, "%s%s%d%s%.2lf%s%.4lf%s%d%s%d%s%lf%s", lineBuffer, "(", productId, ",", 100.0*returnBucket[i], ",", bucketProb[i], ",", numMcIterations, ",", analyseCase == 0 ? 0 : 1, ",", projectedReturn, ")");
							}
							sprintf(lineBuffer, "%s%s", lineBuffer, ";");
							mydb.prepare((SQLCHAR *)lineBuffer, 1);
						}


						// eShortfall, esVol
						// if (doDebug  && debugLevel >= 2){ std::cerr << "Starting analyseResults SavingToDatabase for case \n" << analyseCase << std::endl; }

						const double depoRate = 0.01;  // in decimal...DOME: could maybe interpolate curve for each instance
						const double confLevel(0.1), confLevelTest(0.05);  // confLevelTest is for what-if analysis, for different levels of conf
						int numShortfall(     (int)floor(confLevel     * (double)numAnnRets));
						int numShortfallTest( (int)floor(confLevelTest * (double)numAnnRets));
						// NOTE: eShortfall (which an average of annRets) will be HIGHER than the AnnRet of a KIP barrier, as shown in barrierprob table, which annualises the averagePayoff
						// ... due to Jensen's inequality: f(average) != average(f)
						// ... for example try 2 6y payoffs of 0.1 and 0.6
						double eBestRet(0.0); 	    for (i = 0; i < numShortfall;     i++) { eBestRet       += allAnnRets[numAnnRets -i -1]; }	if (numShortfall) { eBestRet     /= numShortfall; }
						double eShortfall(0.0);	    for (i = 0; i < numShortfall;     i++) { eShortfall     += allAnnRets[i]; }	if (numShortfall)     { eShortfall     /= numShortfall; }
						double eShortfallTest(0.0);	for (i = 0; i < numShortfallTest; i++) { eShortfallTest += allPayoffs[i]; }	if (numShortfallTest) { eShortfallTest /= numShortfallTest; }
						double esVol     = (1 + averageReturn) > 0.0 && (1 + eShortfall) > 0.0 ? (log(1 + averageReturn) - log(1 + eShortfall)) / ESnorm(confLevel) : 0.0;
						double priipsImpliedCost, priipsVaR, priipsDuration;
						double cVar95PctLoss = -100.0*(1.0 - eShortfallTest / midPrice); // eShortfallTest is (confLevelTest-percentile of decimal PAYOFF distribution) so this is the %moneyLoss at that percentile
						// RJ additional metrics
						double stdevStrPosAnnRet(0.0), stdevNegAnnRet(0.0), stdevWorstAnnRet(0.0);
						std::vector<double> stdevStrPosAnnRets, stdevNegAnnRets, stdevWorstAnnRets;
						for (i=0; i < numAnnRets;i++) {
							double thisAnnRet = allAnnRets[i];
							if (thisAnnRet > 0.0) {
								stdevStrPosAnnRets.push_back(thisAnnRet);
							}
							else if (thisAnnRet < 0.0) {
								stdevNegAnnRets.push_back(thisAnnRet);
								if (i < numShortfall) {
									stdevWorstAnnRets.push_back(thisAnnRet);
								}
							}
						}
						
						double rjMean, rjStdev, rjStderr;
						// ... AverageGainVariation
						if (stdevStrPosAnnRets.size() > 1) {
							MeanAndStdev(stdevStrPosAnnRets, rjMean, rjStdev, rjStderr);
							stdevStrPosAnnRet = rjStdev;
						}
						// ... AverageLossVariation
						if (stdevNegAnnRets.size() > 1) {
							MeanAndStdev(stdevNegAnnRets, rjMean, rjStdev, rjStderr);
							stdevNegAnnRet    = rjStdev;
						}
						// ... AverageWorstVariation
						if (stdevWorstAnnRets.size() > 1) {
							MeanAndStdev(stdevWorstAnnRets, rjMean, rjStdev, rjStderr);
							stdevWorstAnnRet  = rjStdev;
						}
						// END: RJ additional metrics


						// downsideVol
						// targetReturn shortfalls
						double downsideVolZeroed(0.0), downsideVol(0.0);
						if (numNegInstances > 1) {
							bool done(false);
							size_t n = 0;
							/* OLD STUFF
							
							std::vector<double> tmpZeroed(numAnnRets), tmp(numNegInstances);  // size: numNegInstances or numAnnRets (non-loss returns will use initialized zeroes)
							double thisMean,thisMean1,dummy1, dummy2;
							for (int i = 0; i < numNegInstances; i++) {
								double thisRet = allAnnRets[i];
								tmpZeroed[ i ] = thisRet;
								tmp      [ i ] = thisRet;
							}
							MeanAndStdev(tmpZeroed,  thisMean,  downsideVolZeroed,  dummy2);
							MeanAndStdev(tmp,        thisMean1, downsideVol,        dummy2);
							// std::cerr << "DownsideVolZeroed:" << numAnnRets << " mean:" << thisMean << " vol:" << downsideVolZeroed << " DownsideVol:" << numNegInstances << " mean:" << thisMean1 << " vol:" << downsideVol << std::endl;

							// possibly better, using targetReturn shortfalls
							std::vector<double> shortfallsZeroed(numAnnRets);
							done = false ;
							for (int i = 0; !done && i < numAnnRets; i++) {
								double thisRet = allAnnRets[i] - targetReturn;
								if (thisRet < 0.0) {
									shortfallsZeroed[n++] = thisRet;
								}
								else {
									done = true;
								}
							}
							MeanAndStdev(shortfallsZeroed, thisMean, downsideVolZeroed, dummy2);
							std::vector<double> shortfalls(n);
							for (int i = 0; i < n; i++) {
								shortfalls[i] = shortfallsZeroed[i];								
							}
							MeanAndStdev(shortfalls, thisMean1, downsideVol, dummy2);
							// std::cerr << "NEWER: " << "DownsideVolZeroed:" << numAnnRets << " mean:" << thisMean << " vol:" << downsideVolZeroed << " DownsideVol:" << n << " mean:" << thisMean1 << " vol:" << downsideVol << std::endl;

							// END: possibly better

							// EVEN better: TargetDownsideDeviation = sqrt( average((annRet - targetReturn)^2) )
							*/
							
							double downsideDeviation(0.0);
							n    = 0;
							done = false;
							for (int i = 0; !done && i < numAnnRets; i++) {
								double thisRet = allAnnRets[i] - targetReturn;
								if (thisRet < 0.0) {
									downsideDeviation += thisRet*thisRet;
									n++;
								}
								else {
									done = true;
								}
							}
							downsideVolZeroed   = sqrt(downsideDeviation / numAnnRets);
							if (n > 0) {
								downsideVol         = sqrt(downsideDeviation / n);
							}
							if (duration > 0.0) {
								downsideVolZeroed  *= sqrt(duration);
								downsideVol        *= sqrt(duration);
							}
							
						}
						
						if (doPriips) {
							PriipsStruct &thisPriip(priipsInstances[(unsigned int)floor((double)priipsInstances.size()*0.025)]);
							priipsVaR          = thisPriip.pvReturn;
							priipsDuration     = thisPriip.yearsToPayoff;
							if (3.842 < 2.0*log(thisPriip.pvReturn)) {
								sprintf(lineBuffer, "%s%lf%s", "Too high a percentile return ", thisPriip.pvReturn, "\n");
								anyString =  lineBuffer;
								std::cerr << anyString << "\n";
								logAnError(anyString);
								evalResult.errorCode = 10002;
								return(evalResult);
							}
							esVol = thisPriip.pvReturn > 0.0 && priipsDuration > 0 ? (sqrt(3.842 - 2.0*log(thisPriip.pvReturn)) - 1.96) / sqrt(priipsDuration) / sqrt(duration) : 0.0;
							// calc PRIIPs PV
							double sumPriipsPvs(0.0);
							int    numPriipsPvs = (int)priipsInstances.size();
							for (int i=0; i < numPriipsPvs; i++) { sumPriipsPvs += priipsInstances[i].pvReturn; }
							double priipsPV = sumPriipsPvs / numPriipsPvs;
							if (priipsUsingRNdrifts && !doPriipsStress) {
								sprintf(lineBuffer, "%s%lf%s%d%s", "update product set PRIIPsPV=", priipsPV, " where productid='", productId, "';");
								mydb.prepare((SQLCHAR *)lineBuffer, 1);
							}
							priipsImpliedCost  =  1.0 - (validFairValue ? fairValue : priipsPV) / askPrice;
						}
						double esVolTest = (1 + averageReturn) > 0.0 && (1 + eShortfallTest) > 0.0 ? (log(1 + averageReturn) - log(1 + eShortfallTest)) / ESnorm(confLevelTest) : 0.0;
						if (averageReturn < -0.99) { esVol = 1000.0; esVolTest = 1000.0; }  // eg a product guaranteed to lose 100%
						double scaledVol = esVol * sqrt(duration);
						double geomReturn(0.0);
						for (i = 0; i < numAnnRets; i++) {
							double thisPayoff = allPayoffs[i];
							geomReturn += log((thisPayoff < unwindPayoff ? unwindPayoff : thisPayoff) / midPrice);
						}
						geomReturn = exp(geomReturn / sumDuration) - 1;
						double sharpeRatio = scaledVol > 0.0 ? (geomReturn / scaledVol > 1000.0 ? 1000.0 : geomReturn / scaledVol) : 1000.0;
						std::vector<double> cesrBuckets   ={ 0.0, 0.005, .02, .05, .1, .15, .25, .4 };
						std::vector<double> priipsBuckets ={ 0.0, 0.005, .05, .12, .2, .30, .80 };
						std::vector<double> cubeBuckets ={ 0.0, 0.026, 0.052, 0.078, 0.104, 0.130, 0.156, 0.182, 0.208, 0.234, 0.260, 0.40 };
						double riskCategory    = calcRiskCategory(cesrBuckets, scaledVol, 1.0);
						double riskScorePriips = calcRiskCategory(priipsBuckets, scaledVol, 1.0);
						double riskScore1to10  = calcRiskCategory(cubeBuckets, scaledVol, 0.0);

						/*
						* irr
						*/

						std::vector<double> c; c.push_back(-midPrice * numCapitalInstances);
						std::vector<double> t; t.push_back(0.0);
						for (std::map<int, double>::iterator it=cashflowMap.begin(); it != cashflowMap.end(); ++it) {
							t.push_back(it->first / YEARS_TO_INT_MULTIPLIER);
							c.push_back(it->second);
						}
						double thisIrr = exp(irr(c, t)) - 1.0;

						// don't forget #annRets may be less than numMcIterations due to early stopping (use forceIterations on commandLine to run the full numMcIterations)
						// std::cerr << "#annRets:" << numAnnRets << "#maxIterations:" << numMcIterations << std::endl;
						// WinLose
						double sumNegRet(0.0), sumPosRet(0.0), sumBelowDepo(0.0);
						int    numNegRet(0), numPosRet(0), numBelowDepo(0);
						for (j = 0; j < numAnnRets; j++) {
							double ret = allAnnRets[j];
							if (ret < 0) { sumNegRet    += ret;  numNegRet++; }
							else { sumPosRet    += ret;  numPosRet++; }
							if (ret < depoRate) { sumBelowDepo += ret;  numBelowDepo++; }
						}
						double probBelowDepo  = (double)numBelowDepo / (double)numAnnRets;
						double eShortfallDepo = numBelowDepo ? sumBelowDepo / (double)numBelowDepo : 0.0;
						double esVolBelowDepo = (1 + averageReturn) > 0.0 && (1 + eShortfallDepo) > 0.0 ? (log(1 + averageReturn) - log(1 + eShortfallDepo)) / ESnorm(probBelowDepo) : 0.0;
						double eNegRet        = numNegRet ? sumNegRet / (double)numNegRet : 0.0;
						double probNegRet     = (double)numNegRet / (double)numAnnRets;
						double esVolNegRet    = (1 + averageReturn) > 0.0 && (1 + eNegRet) > 0.0 ? (log(1 + averageReturn) - log(1 + eNegRet)) / ESnorm(probNegRet) : 0.0;
						double strPosDuration(sumStrPosDurations / numStrPosPayoffs), posDuration(sumPosDurations / numPosPayoffs), negDuration(sumNegDurations / numNegPayoffs);
						double ecGain         = 100.0*(numPosPayoffs ? exp(log(sumPosPayoffs / midPrice / numPosPayoffs) / posDuration) - 1.0 : 0.0);

						double ecStrictGain   = 100.0*(numStrPosPayoffs ? exp(log(sumStrPosPayoffs / midPrice / numStrPosPayoffs) / strPosDuration) - 1.0 : 0.0);
						double ecLoss         = -100.0*(numNegPayoffs && sumNegPayoffs > 0.0 ? exp(log(sumNegPayoffs / midPrice / numNegPayoffs) / negDuration) - 1.0 : 0.0);
						double probGain       = numPosRet ? ((double)numPosRet) / numAnnRets : 0;
						double probStrictGain = numStrPosPayoffs ? ((double)numStrPosPayoffs) / numCapitalInstances : 0;
						double probLoss       = 1 - probGain;
						double eGainRet       = ecGain * probGain;
						double eLossRet       = ecLoss * probLoss;
						// on balance, prefer to use annualised returns, rather than payoffs
						double winLose;
						if (doWinLoseAnnualised) { winLose = sumNegRet ? (sumPosRet / -sumNegRet)*sqrt(duration) : 1000.0; }
						else { winLose        = numNegPayoffs ? -(sumPosPayoffs / midPrice - numPosPayoffs * 1.0) / (sumNegPayoffs / midPrice - numNegPayoffs * 1.0) : 1000.0; }
						if (winLose > 1000.0) { winLose = 1000.0; }
						double expectedPayoff = (applyCredit ? sumPossiblyCreditAdjPayoffs : sumPayoffs) / numAnnRets;
						double earithReturn   = sumPossiblyCreditAdjPayoffs <= 0.0 ? -1.0 : pow(sumPossiblyCreditAdjPayoffs / midPrice / numAnnRets, 1.0 / duration) - 1.0;
						double bmRelAverage        = MyMinAbs(1000.0, bmRelUnderperfPV*benchmarkProbUnderperf + bmRelOutperfPV * benchmarkProbOutperf);
						double productBmReturn     = bmSwapRate + cds5y / 2.0 + (esVol*pow(duration, 0.5) / bmVol)*(bmEarithReturn - bmSwapRate);
						double productExcessReturn = MyMinAbs(1000.0, earithReturn - productBmReturn);

						int    secsTaken      = (int)difftime(time(0), startTime);

						// if (!getMarketData || (ukspaCase != "" && analyseCase == 0)){
						// if (updateCashflows && ((!getMarketData && !useUserParams) || analyseCase == 0)) 
						// update cashflows anyway, esp as ARROW using
						{
							sprintf(lineBuffer, "%s%s%s", "update ", useProto, "cashflows set ");
							if (doPriipsStress) {
								sort(priipsStressVols.begin(), priipsStressVols.end());
								sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "PriipsStressInflationMin='", priipsStressVols[0]);
								sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',PriipsStressInflationMax='", priipsStressVols[(unsigned int)priipsStressVols.size() - 1]);
								sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',PriipsStressYears='", priipsStressYears);
								sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',PriipsStressVar='", priipsStressVar);
							}
							else {
								sprintf(lineBuffer, "%s%s%.5lf%s", lineBuffer, "ESvol='", priipsUsingRNdrifts ? scaledVol : esVol, priipsUsingRNdrifts ? "'/sqrt(duration) +'0" : "");   // pesky way to pick up previously saved realWorld duration						
								sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BenchmarkReturn='", productBmReturn);  // NOTE this is the FORCED comparision against the capital-market-line and NOT the bm-relative return
								sprintf(lineBuffer, "%s%s%s%.5lf%s", lineBuffer, "',ProductExcessReturn=", priipsUsingRNdrifts ? "EArithReturn - " : "", priipsUsingRNdrifts ? productBmReturn : productExcessReturn, "+'0");
								if (priipsUsingRNdrifts) {
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',RiskCategory='", riskCategory);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',RiskScorePriips='", riskScorePriips);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',RiskScore1to10='", riskScore1to10);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',PriipsImpliedCost='", priipsImpliedCost);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',PriipsVaR='", priipsVaR);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',PriipsVarYears='", priipsDuration);
								}
								else {
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',PriipsRealWorldVol='", esVol);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',PriipsRealWorldDuration='", duration);
									// PRIIPS now calculates scenarios using riskfree drift rates ?? following assumes REAL-WORLD drifts
									// ... although the 2017-03-08 regs seem to have a typo at para 12a): where "6" shouldbe presumably "18"...
									// the expected return for each asset or assets shall be the return observed over the period as determined under point 6 of Annex II
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VaRyears='", varYears);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VaR1years='", var1Years);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VaR2years='", var2Years);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VaR99='", vaR99);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VaR='", vaR90);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VaR1='", vaR50);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VaR2='", vaR10);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ExpectedPayoff='", expectedPayoff);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ExpectedGainPayoff='", ePosPayoff);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ExpectedStrictGainPayoff='", eStrPosPayoff);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ExpectedLossPayoff='", eNegPayoff);
									sprintf(lineBuffer, "%s%s%s", lineBuffer, "',FirstDataDate='", allDates[0].c_str());
									sprintf(lineBuffer, "%s%s%s", lineBuffer, "',LastDataDate='", allDates[totalNumDays - 1].c_str());
									sprintf(lineBuffer, "%s%s%d", lineBuffer, "',NumResamples='", thisIteration);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',Duration='", duration);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VolStds='", srriStds);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VolConf='", srriConf);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',AverageAnnRet='", averageReturn);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',CouponReturn='", couponPaidOut ? (averageCouponReturn > 1000.0 ? 1000.0 : averageCouponReturn) : 0.0); // can have large coupon with say 1day to go
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',CESRvol='", srriVol);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',CESRstrictVol='", cesrStrictVol);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ESvolTest='", esVolTest);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ESvolBelowDepo='", esVolBelowDepo);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ESvolNegRet='", esVolNegRet);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ExpectedReturn='", MyMinAbs(1000.0, geomReturn));
									// sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',EArithReturn='",   averageReturn);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',EArithReturn='", MyMinAbs(1000.0, earithReturn));
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',SharpeRatio='", sharpeRatio < -1000.0 ? -1000.0 : (sharpeRatio > 1000.0 ? 1000.0 : sharpeRatio));
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',RiskCategory='", riskCategory);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',RiskScorePriips='", riskScorePriips);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',RiskScore1to10='", riskScore1to10);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',WinLose='", winLose);
									std::time_t rawtime;	struct std::tm * timeinfo;  time(&rawtime);	timeinfo = localtime(&rawtime);
									strftime(charBuffer, 100L, "%Y-%m-%d %H:%M:%S", timeinfo);
									sprintf(lineBuffer, "%s%s%s", lineBuffer, "',WhenEvaluated='", charBuffer);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ProbEarliest='", probEarliest);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ProbEarly='", probEarly);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecGain='", ecGain > 1000.0 ? 1000.0 : ecGain);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecStrictGain='", ecStrictGain > 1000.0 ? 1000.0 : ecStrictGain);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecLoss='", ecLoss);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probGain='", probGain);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probStrictGain='", probStrictGain);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probLoss='", probLoss);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecPar='", numParInstances ? sumParAnnRets / (double)numParInstances : 0.0);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probPar='", (double)numParInstances / (double)numAnnRets);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ErightTailReturn='", eBestRet            *100.0);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',DownsideVol='",      downsideVol         *100.0);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',DownsideVolZeroed='",downsideVolZeroed   *100.0);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',eShortfall='",       eShortfall          *100.0);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',EShortfallTest='",   eShortfallTest      *100.0);  // eShortfallTest is (confLevelTest-percentile of decimal PAYOFF distribution)
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',eShortfallDepo='",   eShortfallDepo      *100.0);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',cVar95PctLoss='",    cVar95PctLoss);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ProbBelowDepo='",    probBelowDepo);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BenchmarkProbShortfall='", benchmarkProbUnderperf);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BenchmarkCondShortfall='", benchmarkCondUnderperf*100.0);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BenchmarkProbOutperf='", benchmarkProbOutperf);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BenchmarkCondOutperf='", benchmarkCondOutperf*100.0);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BmRelCAGR='", bmRelCAGR);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BmRelOutperfPV='", bmRelOutperfPV);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BmRelUnderperfPV='", bmRelUnderperfPV);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',BmRelAverage='", bmRelAverage);
									sprintf(lineBuffer, "%s%s%d", lineBuffer, "',NumEpisodes='", numAllEpisodes);
									sprintf(lineBuffer, "%s%s%d", lineBuffer, "',SecsTaken='", secsTaken);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',IRR='", thisIrr);
									sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',MidPriceUsed='", midPrice);
									if (useUserParams) {
										sprintf(lineBuffer, "%s%s%d", lineBuffer, "',UserParameId='", userId);
									}
								}
							}
							sprintf(lineBuffer, "%s%s%d%s%.2lf%s", lineBuffer, "' where ProductId='", productId, "' and ProjectedReturn='", projectedReturn, "'");
							if (!silent && doDebug && debugLevel > 0) { std::cout << lineBuffer << std::endl; }
							if (mydb.prepare((SQLCHAR *)lineBuffer, 1)) { evalResult.errorCode = 10003; return(evalResult); }
							if (!doBumps && analyseCase == 0) {
								sprintf(lineBuffer, "%s%s%s%.5lf%s%d", "update ", useProto, "product set MidPriceUsed=", midPrice, " where ProductId=", productId);
								mydb.prepare((SQLCHAR *)lineBuffer, 1);
							}
						}
						// ************
						// ******* report fair value things
						// ************
						if ((getMarketData && analyseCase == 0)) {
							double thisMean, thisStdev, thisStderr;
							std::string   thisDateString(allDates.at(startPoint));
							if (!silent) {
								sprintf(charBuffer, "%s", "Spot,Forward,DiscountFactor\nUIDs: ");
								for (i = 0; i < numUl; i++) {
									sprintf(charBuffer, "%s\t%s", charBuffer, ulNames[i].c_str());
								}
								sprintf(charBuffer, "%s%s%.3lf%s", charBuffer, "\tDiscountFactor(incl_FundingFraction=", fundingFraction, ")");
								std::cout << charBuffer << std::endl;
								sprintf(charBuffer, "%s%s", "Spots on: ", thisDateString.c_str());
								for (i = 0; i < numUl; i++) {
									sprintf(charBuffer, "%s\t%.2lf", charBuffer, spotLevels[i]);
								}
								std::cout << charBuffer << std::endl;
								for (int thisMonIndx = 0; thisMonIndx < (int)monDateIndx.size(); thisMonIndx++) {
									int thisMonValue = monDateIndx[thisMonIndx];
									if (std::find(reportableMonDateIndx.begin(), reportableMonDateIndx.end(), thisMonValue) != reportableMonDateIndx.end()) {
										int thisMonPoint = startPoint + thisMonValue;
										thisDateString = allDates.at(thisMonPoint);
										double yearsToBarrier     = monDateIndx[thisMonIndx] / daysPerYear;
										double rootYearsToBarrier = pow(yearsToBarrier,0.5);
										sprintf(charBuffer, "%s%s%s%.2lf", "Fwds(stdev)[%ofSpot][%vol] and discountFactor on: ", thisDateString.c_str(), " T:", yearsToBarrier);
										for (i = 0; i < numUl; i++) {
											double thisSpot = spotLevels[i];
											double meanLogRets, stdevLogRets, stderrLogRets;
											MeanAndStdev(mcForwards[i][thisMonIndx], thisMean, thisStdev, thisStderr);
											// vols ... helps check localVol
											std::vector<double> logRets;
											for (int j=0, len=(int)mcForwards[i][thisMonIndx].size(); j < len; j++) {
												logRets.push_back(log(mcForwards[i][thisMonIndx][j]/ thisSpot));
											}
											MeanAndStdev(logRets, meanLogRets, stdevLogRets, stderrLogRets);
											sprintf(charBuffer, "%s\t%.2lf%s%.2lf%s%.2lf%s%.2lf%s", charBuffer, thisMean, "(", thisStderr, ")[", 100.0*thisMean / spotLevels[i],"][", 100.0*stdevLogRets / rootYearsToBarrier,"]");
										}
										double forwardRate      = 1 + interpCurve(baseCurveTenor, baseCurveSpread, yearsToBarrier); // DOME: very crude for now
										forwardRate            += fundingFraction * interpCurve(cdsTenor, cdsSpread, yearsToBarrier);
										double discountT        = yearsToBarrier - forwardStartT;
										double discountFactor   = pow(forwardRate, -discountT);
										sprintf(charBuffer, "%s\t%.5lf", charBuffer, discountFactor);
										std::cout << charBuffer << std::endl;
									}
								}
							}
							// fair value
							MeanAndStdev(pvInstances, thisMean, thisStdev, thisStderr);
							double issuerCallableComplexityMargin = issuerCallable ? -0.005 : 0.0;  // to be investigated - is there a coupon missed?
							thisFairValue      = (thisMean + issuerCallableComplexityMargin) * issuePrice;
							simulatedFairValue = thisMean;
							sprintf(charBuffer, "%s\t%.2lf%s%.2lf%s%.2lf", "FairValueResults(stdev):",
								thisFairValue, ":", thisStderr*issuePrice, ":", duration);
							std::cout << charBuffer << std::endl;
							if (volShift != 0.0) {
								std::cout << "VOLS HAVE BEEN SHIFTED:" << volShift*100 << std::endl;
							}
							if (doDebug && debugLevel > 4) {
								for (i = 0; i < numUl; i++) {
									for (j = i+1; j < numUl; j++) {
										std::cout << "CORR: " << i << " vs " << j << ":" << MyCorrelation(simulatedShocks[i], simulatedShocks[j], false) << std::endl;										
									}
								}
							}

							evalResult.value  = thisFairValue;
							evalResult.stdErr = thisStderr * issuePrice;

							// update db
							if (updateCashflows || updateProduct) {
								sprintf(lineBuffer, "%s%s%s%.5lf", "update ", useProto, "product set FairValue='", thisFairValue);
								sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',FairValueStdev='",        thisStderr*issuePrice);
								sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',FundingFractionUsed='",   fundingFraction);
								sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',RNduration='",            duration);
								sprintf(lineBuffer, "%s%s%s",    lineBuffer, "',FairValueDate='",         allDates.at(startPoint).c_str());
								sprintf(lineBuffer, "%s%s%d%s",  lineBuffer, "' where ProductId='",       productId, "'");
								// std::cout << lineBuffer << std::endl;
								if (!consumeRands && ukspaCase == "") {
									mydb.prepare((SQLCHAR *)lineBuffer, 1);
								}
							}
							// *****************
							// update fvsnapshot
							// *****************
							int numTenors      = (int)md.oisRatesTenor [0].size();
							int numCdsTenors   = (int)cdsTenors        [0].size();
							int numDivsTenors  = (int)md.divYieldsTenor[0].size();
							int numVolsTenors  = (int)md.ulVolsTenor   [0].size();
							int numVolsStrikes = (int)md.ulVolsStrike  [0].size();
							std::string aDate = to_iso_extended_string(bLastDataDate).substr(0, 10);
							std::string ulList; for (int i = 0; i < numUl; i++) { ulList += (i == 0 ? "":",") + ulNames[i]; }
							double ratesChecksum(0.0); for (int i=0; i < numTenors;i++)      { 
								ratesChecksum  += md.oisRatesTenor [0][i]  * md.oisRatesRate [0][i]; 
							}
							std::ostringstream ratesStream; ratesStream << "<table><tr><th>Tenor</th><th>Rate</th></tr>";
							for (int i=0; i < numTenors; i++) {
								ratesStream << "<tr><td>" << std::showpoint << std::setprecision(4) << md.oisRatesTenor[0][i] << "</td><td>" << std::setprecision(4) << md.oisRatesRate[0][i] << "</td></tr>";
							}
							ratesStream << "</table>";
							std::string ratesHtml = ratesStream.str();

							double cdsChecksum(0.0);   for (int i=0; i < numCdsTenors; i++)  { cdsChecksum    += cdsTenors        [0][i]  * cdsSpreads      [0][i]; }
							double divsChecksum(0.0);  for (int i=0; i < numDivsTenors; i++) { divsChecksum   += md.divYieldsTenor[0][i]  * md.divYieldsRate[0][i]; }
							double volsChecksum(0.0);  for (int i=0; i < numVolsTenors; i++) { 
								for (int j=0; j < numVolsStrikes; j++) {
									volsChecksum   += md.ulVolsTenor[0][i] * md.ulVolsStrike[0][i][j] * md.ulVolsImpVol[0][i][j];
								}
							}
							
							mydb.prepare((SQLCHAR *)"BEGIN TRANSACTION", 1);
							sprintf(lineBuffer, "%s%d", "insert into fvsnapshot (ProductId,UserId,FV,LastDataDate,RatesChecksum,CdsChecksum,DivsChecksum,LocalVolChecksum,BarrierBend,FundingFraction,VolShift,UlList,RatesHtml) values (",productId);
							sprintf(lineBuffer, "%s%s%d",  lineBuffer, ",",   userId);
							sprintf(lineBuffer, "%s%s%lf", lineBuffer, ",",   thisFairValue);
							sprintf(lineBuffer, "%s%s%s", lineBuffer, ",'",   aDate.c_str());
							sprintf(lineBuffer, "%s%s%lf", lineBuffer, "',",  ratesChecksum);
							sprintf(lineBuffer, "%s%s%lf", lineBuffer, ",",   cdsChecksum);
							sprintf(lineBuffer, "%s%s%lf", lineBuffer, ",",   divsChecksum);
							sprintf(lineBuffer, "%s%s%lf", lineBuffer, ",",   volsChecksum);
							sprintf(lineBuffer, "%s%s%lf", lineBuffer, ",",   barrier.at(0).thisBarrierBend);
							sprintf(lineBuffer, "%s%s%lf", lineBuffer, ",",   fundingFraction);
							sprintf(lineBuffer, "%s%s%lf", lineBuffer, ",",   volShift);
							sprintf(lineBuffer, "%s%s%s",  lineBuffer, ",'",  ulList.c_str());
							sprintf(lineBuffer, "%s%s%s",  lineBuffer, "','", ratesHtml.c_str());
							sprintf(lineBuffer, "%s%s",    lineBuffer, "')");
							std::cout << ratesStream.str().c_str() << std::endl;
							mydb.prepare((SQLCHAR *)lineBuffer, 1);
							mydb.prepare((SQLCHAR *)"select max(FvSnapshotId) from FvSnapshot", 1);
							RETCODE retcode = mydb.fetch(true, lineBuffer); 
							if (retcode == MY_SQL_GENERAL_ERROR) { std::cerr << "IPRerror:" << lineBuffer << std::endl; 
								mydb.prepare((SQLCHAR *)"ROLLBACK", 1); 
							}
							else {
								sprintf(lineBuffer, "%s%s","FvSnapshotId:",mydb.bindBuffer[0]);
								std::cout << lineBuffer << std::endl;
							}
							mydb.prepare((SQLCHAR *)"COMMIT", 1);

						}



						// text output
						if (!silent) {
							sprintf(charBuffer, "%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf%s%.2lf",
								analyseCase == 0 ? "MarketRiskResults:" : "MarketAndCreditRiskResults:",
								100.0*geomReturn, ":",
								100.0*earithReturn, ":",
								100.0*esVol*pow(duration, 0.5), ":",
								100.0*productExcessReturn, ":",
								100.0*thisIrr, ":",
								100.0*probStrictGain, ":",
								100.0*probGain, ":",
								100.0*probLoss, ":",
								ecStrictGain, ":",
								ecGain, ":",
								ecLoss, ":",
								ecStrictGain*probStrictGain, ":",
								ecLoss*probLoss, ":",
								duration, ":",
								priipsUsingRNdrifts && !doPriipsStress ? riskScorePriips : riskCategory, ":",
								riskScore1to10, ":",
								winLose, ":",
								100.0*eShortfall, ":",
								vaR90, ":",
								vaR50, ":",
								vaR10, ":",
								cVar95PctLoss, ":",
								cVar95PctLoss + 100.0*benchmarkProbOutperf, ":",
								100.0*averageCouponReturn, ":",
								100.0*maxBarrierProb, ":",
								100.0*maxBarrierProbMoneyness, ":",
								100.0*benchmarkProbOutperf, ":",
								100.0*benchmarkCondOutperf, ":",
								100.0*benchmarkProbUnderperf, ":",
								100.0*benchmarkCondUnderperf, ":",
								100.0*bmRelCAGR, ":",
								100.0*bmRelOutperfPV, ":",
								100.0*bmRelUnderperfPV, ":",
								100.0*bmRelAverage, ":",
								100.0*eBestRet, ":",
								100.0*downsideVolZeroed, ":",
								ecLoss*probLoss == 0.0 ? 0.0 : ecStrictGain*probStrictGain/(ecLoss*probLoss), ":",  // RJ ReturnRatio
								eShortfall      == 0.0 ? 0.0 : fabs(eBestRet/eShortfall), ":",                      // RJ TailRatio
								eBestRet        == 0.0 ? 0.0 : geomReturn/eBestRet, ":",                            // RJ EfficiencyRatio
								100.0*stdevStrPosAnnRet, ":",                                                       // RJ AverageGainVariation
								100.0*stdevNegAnnRet, ":",                                                          // RJ AverageLossVariation
								100.0*stdevWorstAnnRet, ":",                                                        // RJ Worst10PctVariation
								100.0*cagrRisk                                                                      // RJ ExpectedReturnVariation
							);
							std::cout << charBuffer << std::endl;
						} // !silent
					} // for analyseCase
				} // for thisIssuerIndx
			}  // not doAccruals
		} // handleResults
		return(evalResult);
	}  // evaluate()
};
// END class SProduct








// ***********************
// ******* sundry funcs
// ***********************

//
// get index of needle in vec
//
int getIndexInVector(std::vector<int> vec, const int needle){
	std::vector<int>::iterator it = std::find(vec.begin(), vec.end(), needle);
	if (it != vec.end()) {
		return (int) (it - vec.begin());
	}
	return -1;
}

//
// bumpSpots
//
void bumpSpots(SProduct                                  &spr,
	const int                                            i,
	const std::vector<int>                               &ulIds,
	const std::vector<double>                            &spots,
	std::vector<UlTimeseries>                            &ulPrices,
	const bool                                           doStickySmile,
	MarketData                                           &thisMarketData,
	const std::vector<std::vector<std::vector<double>>>  &holdUlVolsStrike,
	const double                                   deltaBumpAmount,
	const int                                      totalNumDays,
	const int                                      daysExtant,
	const bool                                     reinstateOthers){
	int j, k;
	int    numBarriers = (int)spr.barrier.size();
	double bumpFactor  = 1.0 / (1.0 + deltaBumpAmount);
	int    ulId        = ulIds[i];
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
			else if(reinstateOthers) {
				thisBrel.calcMoneyness(thisBrel.originalMoneyness);
			}
		}
	}
}


	