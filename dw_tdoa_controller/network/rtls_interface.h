/*! ------------------------------------------------------------------------------------------------------------------
* @file rtls_interface.h
* @brief DecaWave RTLS interface definitions between the CLE and anchor nodes
*
* @attention
* Copyright 2008, 2014 (c) DecaWave Ltd, Dublin, Ireland.
*
* All rights reserved.
*
* @author Billy Verso / Zoran Skrba / Viswanathan Bhojan
*/
#ifndef __RTLS_INTERFACE_H
#define __RTLS_INTERFACE_H

// Protocol structs and enums :
enum RTLS_CMD {
	// Anchor -> LE
	RTLS_CMD_REQ_CFG = 0x42,				// Config Req
	RTLS_CMD_SET_CFG = 0x43,
	RTLS_CMD_REPORT_TOA = 0x3A, 			// TDOA
	RTLS_CMD_REPORT_TOA_EX = 0x3B, 			// TDOA with extra data (e.g. IMU data)
	RTLS_CMD_REPORT_CS = 0x3C, 				// CS
	RTLS_CMD_REPORT_NODE = 0x3D, 			// Report Wireless Node
	RTLS_CMD_DEBUG = 0x35,					// Debug Message
	RTLS_CMD_NETWORK_STATS,					// TCP Statistics

	// LE -> Anchor
	RTLS_CFG_REQ = 0x52,
	RTLS_COMM_TEST_START_REQ,
	RTLS_COMM_TEST_RESULT_REQ,
	RTLS_RANGE_MEAS_REQ,
	RTLS_INIT_REQ,
	RTLS_START_REQ,

	RTLS_CFG_IND = 0x80,
	RTLS_COMM_TEST_DONE_IND,
	RTLS_COMM_TEST_RESULT_IND,
	RTLS_RANGE_MEAS_IND,

	RTLS_CMD_NODE_CMD,
	RTLS_CMD_NODE_ID,
	RTLS_CMD_ACK_IND,

	RTLS_TEMP_VBAT_IND,						// Temperature and VBAT
	RTLS_TOA_CCP_ACCUMULATOR_REQ,           // Request Acculmulator for CCP or TOA
	RTLS_TOA_CCP_ACCUMULATOR_IND            // Accumulator Report for CCP or TOA
};


#define RTLS_CMD_REQ_CFG_LEN    			(64)
#define RTLS_CMD_REPORT_TOA_LEN 			(16)
#define RTLS_CMD_REPORT_TOA_EX_LEN 			(17) //minimum length of longer TDoA report, extra byte, specifying extra data length
#define RTLS_CMD_REPORT_CS_LEN 				(21)
#define RTLS_CMD_REPORT_NODE_LEN 			(64)
#define RTLS_CMD_REPORT_RANGING_TOF_LEN 	(33)
#define RTLS_CMD_REPORT_RANGING_TOF_ERR_LEN (33)
#define RTLS_CMD_DEBUG_LEN 					(3)
#define RTLS_CMD_NETWORK_STATS_LEN          (9)

#define RTLS_COMM_TEST_DONE_IND_LEN			(1)
#define RTLS_COMM_TEST_RESULT_IND_LEN		(5)
#define RTLS_RANGE_MEAS_IND_LEN				(7)
#define RTLS_TEMP_VBAT_IND_LEN				(7)
#define RTLS_TOA_CCP_ACCUMULATOR_IND_LEN	(7+16+4096)


#define DWT_SIZEOFIMUDATA (25) //1 mask, 6 mag, 6 acc, 6 gyr, 4 pressure, 2 temp.

#define DWT_CLOCKPERIOD (17.207401025) //((double)(2^40/(128*499.2e6))

enum RTLS_DATA {
    RTLS_DATA_ANCHOR_REQ = 0x41,

    RTLS_DATA_ANCHOR = 0x61,
    RTLS_DATA_BLINK = 0x62,
    RTLS_DATA_STATS = 0x63,
    RTLS_DATA_IMU = 0x64,
    RTLS_DATA_BLINK_EXT = 0x65
};

#endif //__RTLS_INTERFACE_H
