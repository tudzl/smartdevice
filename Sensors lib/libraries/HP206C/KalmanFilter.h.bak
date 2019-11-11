/*
 * File name  : kalmanFilter.h
 * Description:  
 * Author     : Oliver Wang from Seeed studio
 * Version    : V0.1
 * Create Time: 2014/04
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
extern KalmanFilter kalmanFilter;
#endif