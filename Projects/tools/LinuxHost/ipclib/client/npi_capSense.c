 /**************************************************************************************************
  Filename:       npi_capSense.c
  Revised:        $Date: 2012-03-21 17:37:33 -0700 (Wed, 21 Mar 2012) $
  Revision:       $Revision: 246 $

  Description:    This file contains Linux platform specific SYS CAPSENSE
                  Surrogate implementation

  Copyright (C) {2012} Texas Instruments Incorporated - http://www.ti.com/


   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

     Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the
     distribution.

     Neither the name of Texas Instruments Incorporated nor the names of
     its contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

/**************************************************************************************************
 *                                           Includes
 **************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "npi_ipc_client.h"
#include "npi_capSense.h"
#include "rti_lnx.h" // for rStatus_t

//TO DO, To Rename or suppress
#define RTI_ERROR_SYNCHRONOUS_NPI_TIMEOUT                0xFF



#define msg_memcpy(src, dst, len)	memcpy(src, dst, len)


#define NAME_ELEMENT(element) [element&0x1F] = #element

/**************************************************************************************************
 * @fn          CAP_AsynchMsgCback
 *
 * @brief       This function is a NPI callback to the client that indicates an
 *              asynchronous message from CapSense modules has been received. The
 *              client software is expected to complete this call.
 *
 *              Note: The client must copy this message if it requires it
 *                    beyond the context of this call.
 *
 * input parameters
 *
 * @param       *pMsg - A pointer to an asychronously received message.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
int CAP_AsynchMsgCback( npiMsgData_t *pMsg )
{
	if ((pMsg->subSys == RPC_SYS_PERIPHERALS) && (pMsg->cmdId == RPC_CMD_ID_CAPSENSE_SUB_SUBSYSTEM_ID))
	{
		switch (pMsg->pData[0])
		{
		case RPC_CMD_ID_CAPSENSE_BUTTON_IND:
		{
			CapSenseButtonPressedInd(*(uint16*)(&pMsg->pData[1]));
			break;
		}
		case RPC_CMD_ID_CAPSENSE_BUTTON_STATS:
		{
			CapSenseRawDataInd(pMsg->pData[1],
					*(uint16*)(&pMsg->pData[2]),
					*(uint16*)(&pMsg->pData[4]),
					*(uint16*)(&pMsg->pData[6]));
			break;
		}
		case RPC_CMD_ID_CAPSENSE_BUTTON_STATS_32:
		{
			CapSenseRawDataInd32(pMsg->pData[1],
					*(int32*)(&pMsg->pData[2]),
					*(int32*)(&pMsg->pData[6]),
					*(int32*)(&pMsg->pData[10]));
			break;
		}
		default:
			break;
		}
	}
	return 0;
}


/**************************************************************************************************
 *
 * @fn          HalCapSenseControlStat
 *
 * @brief       This API is used to control CapSense stats printing
 *
 * input parameters
 *
 * @param    ON/OFF     Turn printing ON/OFF
 *
 * @return   Status
 *
 **************************************************************************************************/
void  HalCapSenseControlStat(uint8 statOnOff)
{
	npiMsgData_t pMsg;
	pMsg.len = 2;
	pMsg.subSys = RPC_SYS_PERIPHERALS | RPC_CMD_AREQ;
	pMsg.cmdId = RPC_CMD_ID_CAPSENSE_SUB_SUBSYSTEM_ID;
	pMsg.pData[0] = RPC_CMD_ID_CAPSENSE_STATS_CONTROL_REQ;
	pMsg.pData[1] = statOnOff;

	// send Program Buffer request to NP RTIS asynchronously as a confirm is due back
	NPI_SendAsynchData( &pMsg );
}

/**************************************************************************************************
 *
 * @fn          HalCapSenseSetThresholds
 *
 * @brief       This API is used to set CapSense thresholds
 *
 * input parameters
 *
 * @param    thresholds		pointer to buffer containing new thresholds
 *
 * @return   Status
 *
 **************************************************************************************************/
void  HalCapSenseSetThresholds(uint8 length, uint8 *thresholds)
{
	npiMsgData_t pMsg;
	pMsg.len = 2 + length;
	pMsg.subSys = RPC_SYS_PERIPHERALS | RPC_CMD_AREQ;
	pMsg.cmdId = RPC_CMD_ID_CAPSENSE_SUB_SUBSYSTEM_ID;
	pMsg.pData[0] = RPC_CMD_ID_CAPSENSE_SET_THRESHOLDS;
	pMsg.pData[1] = length;
	memcpy(&pMsg.pData[2], thresholds, length);

	// send Program Buffer request to NP RTIS asynchronously as a confirm is due back
	NPI_SendAsynchData( &pMsg );
}

/**************************************************************************************************
 *
 * @fn          HalCapSenseSetStuckKeyDetectionThresholds
 *
 * @brief       This API is used to set CapSense Stuck Key Detection thresholds
 *
 *				For each key set the number of samples to count above the statistics threshold
 *				before suspecting that the key is stuck.
 *
 * input parameters
 *
 * @param    thresholds		pointer to buffer containing new thresholds
 *
 * @return   Status
 *
 **************************************************************************************************/
void  HalCapSenseSetStuckKeyDetectionThresholds(uint8 length, uint8 *thresholds)
{
	npiMsgData_t pMsg;
	pMsg.len = 2 + length;
	pMsg.subSys = RPC_SYS_PERIPHERALS | RPC_CMD_AREQ;
	pMsg.cmdId = RPC_CMD_ID_CAPSENSE_SUB_SUBSYSTEM_ID;
	pMsg.pData[0] = RPC_CMD_ID_CAPSENSE_SET_STUCK_KEY_DETECTION;
	pMsg.pData[1] = length;
	memcpy(&pMsg.pData[2], thresholds, length);

	// send Program Buffer request to NP RTIS asynchronously as a confirm is due back
	NPI_SendAsynchData( &pMsg );
}



/**************************************************************************************************
 **************************************************************************************************/
