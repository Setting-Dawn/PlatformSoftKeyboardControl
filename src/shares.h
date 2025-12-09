/** @file shares.h
 *  Shares and Queues used in Soft Keyboard ME507 Project
 *  
 *  @author Setting-Dawn
 *  @author JR Ridgely
 *  @date   2021-Oct-23 Original file by JR Ridgely
 *  @date   2021-Dec-04 Setting-Dawn rewrote example shares and queues to be project specific
 *  @copyright (c) 2021 by JR Ridgely, released under the LGPL 3.0. 
 */

#ifndef _SHARES_H_
#define _SHARES_H_

#include "taskqueue.h"
#include "taskshare.h"

// A share which holds whether the external program needs to initialize V0
extern Share<bool> initializeVFLG;
// A share which holds whether the external program should read current voltages
extern Share<bool> readVFLG;
// Shares to communicate target position between webpage and motor control tasks
extern Share<float> xBar;
extern Share<float> yBar;
// A rudimentary share to publish data from
extern float publish[208];
extern Share<bool> dataAvailable;
// Mutexes to thread protect the twi process
extern SemaphoreHandle_t twiMutex;
#endif // _SHARES_H_