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

// A share which holds whether the external program needs to initialize
extern Share<bool> initializeVFLG;
// A share which holds whether the external program should read voltages
extern Share<bool> readVFLG;
// A share which holds whether the external program has communicated how the connections are defined
extern Share<bool> reMapCompleteFLG;
// A share which holds how the connections are defined for adc1
extern Share<uint8_t> adc1PinMap[8];
// A share which holds how the connections are defined for adc2
extern Share<uint8_t> adc2PinMap[8];
// A share which holds how the current connections are defined
extern Share<uint8_t> currPinMap[16];
// A share which holds whether the external program has communicated how the connections are defined
extern Share<double> publishDeltaV[208];

#endif // _SHARES_H_