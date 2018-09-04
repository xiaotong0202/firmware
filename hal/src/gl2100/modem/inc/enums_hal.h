/**
 ******************************************************************************
  Copyright (c) 2013-2014 IntoRobot Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
*/

#ifndef ENUMS_HAL_H
#define ENUMS_HAL_H

#include <string.h>

// ----------------------------------------------------------------
// Types
// ----------------------------------------------------------------
//! MT Device Types
typedef enum { DEV_UNKNOWN, DEV_SIM800 } Dev;
//! SIM Status
typedef enum { SIM_UNKNOWN, SIM_MISSING, SIM_PIN, SIM_READY } Sim;
//! SIM Status
typedef enum { LPM_DISABLED, LPM_ENABLED, LPM_ACTIVE } Lpm;
//! Device status
typedef struct {
    Dev dev;            //!< Device Type
    Lpm lpm;            //!< Power Saving
    Sim sim;            //!< SIM Card Status
    char ccid[20+1];    //!< Integrated Circuit Card ID
    char imsi[15+1];    //!< International Mobile Station Identity
    char imei[15+1];    //!< International Mobile Equipment Identity
    char meid[18+1];    //!< Mobile Equipment IDentifier
    char manu[16];      //!< Manufacturer (u-blox)
    char model[16];     //!< Model Name (LISA-U200, LISA-C200 or SARA-G350)
    char ver[16];       //!< Software Version
} DevStatus;
//! Registration Status
typedef enum { REG_UNKNOWN, REG_DENIED, REG_NONE, REG_HOME, REG_ROAMING } Reg;
//! Access Technology
typedef enum { ACT_UNKNOWN, ACT_GSM, ACT_EDGE, ACT_UTRAN, ACT_CDMA } AcT;
//! Network Status
typedef struct {
    Reg csd;        //!< CSD Registration Status (Circuit Switched Data)
    Reg psd;        //!< PSD Registration status (Packet Switched Data)
    AcT act;        //!< Access Technology
    int rssi;       //!< Received Signal Strength Indication (in dBm, range -113..-53)
    int qual;       //!< In UMTS RAT indicates the Energy per Chip/Noise ratio in dB levels
                    //!< of the current cell (see <ecn0_ lev> in +CGED command description),
                    //!< see 3GPP TS 45.008 [20] subclause 8.2.4
    char opr[16+1]; //!< Operator Name
    char num[32];   //!< Mobile Directory Number
    unsigned short lac;  //!< location area code in hexadecimal format (2 bytes in hex)
    unsigned int ci;     //!< Cell ID in hexadecimal format (2 to 4 bytes in hex)
} NetStatus;

typedef enum {
    IPSTATUS_ATERROR       = 0,
    IPSTATUS_INITIAL       = 1,
    IPSTATUS_START         = 2,
    IPSTATUS_CONFIG        = 3,
    IPSTATUS_GPRSACT       = 4,
    IPSTATUS_STATUS        = 5,
    IPSTATUS_PROCESSING    = 6,
    IPSTATUS_DEACT         = 7
} ip_status_t;

typedef enum {
    CELLULAR_STATE_INVALID       = 0,
    CELLULAR_STATE_READY         = 1,
    CELLULAR_STATE_CONNECTING    = 2,
    CELLULAR_STATE_CONNECTED     = 3,
    CELLULAR_STATE_DISCONNECTED  = 4,
} cellular_state_t;

//! An IP v4 address
typedef uint32_t MDM_IP;
#define NOIP ((MDM_IP)0) //!< No IP address
// IP number formating and conversion
#define IPSTR           "%d.%d.%d.%d"
#define IPNUM(ip)       ((ip)>>24)&0xff, \
                        ((ip)>>16)&0xff, \
                        ((ip)>> 8)&0xff, \
                        ((ip)>> 0)&0xff
#define IPADR(a,b,c,d) ((((uint32_t)(a))<<24) | \
                        (((uint32_t)(b))<<16) | \
                        (((uint32_t)(c))<< 8) | \
                        (((uint32_t)(d))<< 0))


// ----------------------------------------------------------------
// Device
// ----------------------------------------------------------------

typedef enum { AUTH_NONE, AUTH_PAP, AUTH_CHAP, AUTH_DETECT } Auth;

// ----------------------------------------------------------------
// Sockets
// ----------------------------------------------------------------

//! Type of IP protocol
typedef enum { MDM_IPPROTO_TCP = 0, MDM_IPPROTO_UDP = 1 } IpProtocol;

//! Socket error return codes
#define MDM_SOCKET_ERROR    (-1)

// ----------------------------------------------------------------
// Parsing
// ----------------------------------------------------------------

enum {
    // waitFinalResp Responses
    NOT_FOUND     =  0,
    WAIT          = -1, // TIMEOUT
    RESP_OK       = -2,
    RESP_ERROR    = -3,
    RESP_PROMPT   = -4,
    RESP_ABORTED  = -5,

    // getLine Responses
    #define LENGTH(x)  (x & 0x00FFFF) //!< extract/mask the length
    #define TYPE(x)    (x & 0xFF0000) //!< extract/mask the type

    TYPE_UNKNOWN    = 0x000000,
    TYPE_OK         = 0x110000,
    TYPE_ERROR      = 0x120000,
    TYPE_RING       = 0x210000,
    TYPE_CONNECT    = 0x220000,
    TYPE_DISCONNECT = 0x230000,
    TYPE_CONNECTCLOSTED = 0x240000,
    TYPE_SENDOK     = 0x250000,
    TYPE_SENDERROR  = 0x260000,
    TYPE_NOCARRIER  = 0x270000,
    TYPE_NODIALTONE = 0x280000,
    TYPE_BUSY       = 0x290000,
    TYPE_NOANSWER   = 0x2a0000,
    TYPE_IPSHUT     = 0x2b0000,
    TYPE_STATUS     = 0x2c0000,
    TYPE_PROMPT     = 0x300000,
    TYPE_PLUS       = 0x400000,
    TYPE_TEXT       = 0x500000,
    TYPE_ABORTED    = 0x600000,
    TYPE_DBLNEWLINE = 0x700000,

    // special timout constant
    TIMEOUT_BLOCKING = 0xffffffff
};
#endif

