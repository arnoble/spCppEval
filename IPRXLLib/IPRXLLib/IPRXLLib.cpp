#include <math.h>

double __stdcall IPR_optionPrice(double &CallPutFlag, double &S, double &K, double &T, double &r, double &v, double &ExerciseType, double &dN) {
	
	//return 123.45;

	// validate inputs
	if (CallPutFlag != -1.0 && CallPutFlag != 1.0)    return -10000;
	if (ExerciseType != 1.0 && ExerciseType != 2.0)    return -9999;
	if (dN>1000.0) return -9998;
	if (S<0.0)    return -9997;
	if (K<0.0)    return -9996;
	if (T<0.0)    return -9995;
	if (r<0.0)    return -9994;
	if (v<0.0)    return -9993;

	int i, j, N;
	double pv, immediateVal, temp;
	N = (int)dN;
	double S0 = S;
	double dt = T / dN;
	double u  = exp(v * sqrt(dt));               // size of up jump
	double d  = 1.0 / u;                         //size of down jump
	double p1 = (u - exp(r * dt)) / (u - d);     //probability of up jump
	double p2 = 1 - p1;                          // probability of down jump
	double *Smat[1002];                          // stock prices
	double *Cmat[1002];                     // call prices
	for (i=0; i < N + 2; i++){ 
		Smat[i] = new double[N + 2]; 
		Cmat[i] = new double[N + 2];
	}
	Smat[1][1] = S0;
	for (i = 1; i <= N; i++)	{
		Smat[1][i + 1] = Smat[1][i] * u;
		for (j = 2; j <= i + 1; j++) {
			Smat[j][i + 1] = Smat[j - 1][i] * d;
		}
	}


	for (i = 1; i <= N + 1; i++){
		temp = CallPutFlag * (Smat[i][N + 1] - K);
		Cmat[i][N + 1] = temp>0.0 ? temp : 0.0;
	}

	//return (Smat[N+1][N + 1]);
	double dtPV = exp(-r * dt);
	for (i = N; i >= 1; i--) {
		for (j = 1; j <= i; j++) {
			pv = dtPV * (p2 * Cmat[j][i + 1] + p1 * Cmat[j + 1][i + 1]);
			immediateVal = CallPutFlag * (Smat[j][i] - K);
			if (ExerciseType == 2.0) {
				Cmat[j][i] = pv > immediateVal ? pv : immediateVal;
			}
			else {
				Cmat[j][i] = pv > 0.0 ? pv : 0.0;
			}
		}
	}

	// tidy up
	double ans = Cmat[1][1];
	for (i=0; i < N + 2; i++){
		delete Smat[i];
		delete Cmat[i];
	}
	return (ans);


//	return 123.45;

 }
 
//This function returns option price using CRR Binomial tree
//parameters are:
//CallPutFlag - use 1 for call and -1 for put option
//S - spot price
//K - option strike
//T - option maturity
//r - risk free rate
//v - volatility
//ExerciseType - use 2 for American and 1 for european
//N - no of time steps for the binomial tree

