/*
 * File name  : ZFilter.h
 * Description:  IIR FIR general filter
 * Author     : zell
 * Version    : V0.1
 * Create Time: 2019
 * Change Log :
*/

#ifndef _KALMANFILTER_H
#define _KALMANFILTER_H
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <Arduino.h>
#include <inttypes.h>
/****************************************************************************/
/***        Local variables                                               ***/
/****************************************************************************/


/****************************************************************************/
/***        Class Definitions                                             ***/
/****************************************************************************/
class ZFilter
{
    public:
	  ZFilter();
      //KalmanFilter();	  
	  float Filter(float);	 
	  //float Filter_k(float);
	private:
	/* variables */
	unsigned int idx;
	float  X_post,X_pre1,X_pre2, X_cur,p2,p1,p;
	//float X_pre, X_post, P_pre, P_post, K_cur;
	//float Gaussian_Noise_Cov(void);
	
};


class KalmanFilter
{
    public:
	  KalmanFilter();	 
	  float Filter(float);
	private:
	/* variables */
	float X_pre, X_post, P_pre, P_post, K_cur; 
	float Gaussian_Noise_Cov(void);
	
};
#endif