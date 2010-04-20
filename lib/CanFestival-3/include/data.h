/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT and Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __data_h__
#define __data_h__

#ifdef __cplusplus
extern "C" {
#endif

/* declaration of CO_Data type let us include all necessary headers
 struct struct_CO_Data can then be defined later
 */
typedef struct struct_CO_Data CO_Data;

#include "applicfg.h"
#include "def.h"
#include "can.h"
#include "objdictdef.h"
#include "objacces.h"
#include "sdo.h"
#include "pdo.h"
#include "states.h"
#include "lifegrd.h"
#include "sync.h"
#include "nmtSlave.h"
#include "nmtMaster.h"
#include "emcy.h"
#ifdef CO_ENABLE_LSS
#include "lss.h"
#endif

/**
 * @ingroup od
 * @brief This structure contains all necessary informations to define a CANOpen node 
 */
struct struct_CO_Data {
	/* Object dictionary */
	UNS8 *bDeviceNodeId;
	const indextable *objdict;
	s_PDO_status *PDO_status;
	const quick_index *firstIndex;
	const quick_index *lastIndex;
	const UNS16 *ObjdictSize;
	const UNS8 *iam_a_slave;
	valueRangeTest_t valueRangeTest;
	
	/* SDO */
	s_transfer transfers[SDO_MAX_SIMULTANEOUS_TRANSFERTS];
	/* s_sdo_parameter *sdo_parameters; */

	/* State machine */
	e_nodeState nodeState;
	s_state_communication CurrentCommunicationState;
	initialisation_t initialisation;
	preOperational_t preOperational;
	operational_t operational;
	stopped_t stopped;
     void (*NMT_Slave_Node_Reset_Callback)(CO_Data*);
     void (*NMT_Slave_Communications_Reset_Callback)(CO_Data*);
     
	/* NMT-heartbeat */
	UNS8 *ConsumerHeartbeatCount;
	UNS32 *ConsumerHeartbeatEntries;
	TIMER_HANDLE *ConsumerHeartBeatTimers;
	UNS16 *ProducerHeartBeatTime;
	TIMER_HANDLE ProducerHeartBeatTimer;
	heartbeatError_t heartbeatError;
	e_nodeState NMTable[NMT_MAX_NODE_ID]; 

	/* SYNC */
	TIMER_HANDLE syncTimer;
	UNS32 *COB_ID_Sync;
	UNS32 *Sync_Cycle_Period;
	/*UNS32 *Sync_window_length;;*/
	post_sync_t post_sync;
	post_TPDO_t post_TPDO;
	post_SlaveBootup_t post_SlaveBootup;
	
	/* General */
	UNS8 toggle;
	CAN_PORT canHandle;	
	scanIndexOD_t scanIndexOD;
	storeODSubIndex_t storeODSubIndex; 
	
	/* DCF concise */
    const indextable* dcf_odentry;
	UNS8* dcf_cursor;
	UNS32 dcf_entries_count;
	UNS8 dcf_request;
	
	/* EMCY */
	e_errorState error_state;
	UNS8 error_history_size;
	UNS8* error_number;
	UNS32* error_first_element;
	UNS8* error_register;
    UNS32* error_cobid;
	s_errors error_data[EMCY_MAX_ERRORS];
	post_emcy_t post_emcy;
	
#ifdef CO_ENABLE_LSS
	/* LSS */
	lss_transfer_t lss_transfer;
	lss_StoreConfiguration_t lss_StoreConfiguration;
#endif	
};

#define NMTable_Initializer Unknown_state,

#define s_transfer_Initializer {\
		0,          /* nodeId */\
		0,          /* wohami */\
		SDO_RESET,  /* state */\
		0,          /* toggle */\
		0,          /* abortCode */\
		0,          /* index */\
		0,          /* subIndex */\
		0,          /* count */\
		0,          /* offset */\
		{0},        /* data (static use, so that all the table is initialize at 0)*/\
		0,          /* dataType */\
		-1,         /* timer */\
		NULL        /* Callback */\
	  },

#define ERROR_DATA_INITIALIZER \
	{\
	0, /* errCode */\
	0, /* errRegMask */\
	0 /* active */\
	},
	
#ifdef CO_ENABLE_LSS

#ifdef CO_ENABLE_LSS_FS	
#define lss_fs_Initializer \
		,0,						/* IDNumber */\
  		128, 					/* BitChecked */\
  		0,						/* LSSSub */\
  		0, 						/* LSSNext */\
  		0, 						/* LSSPos */\
  		LSS_FS_RESET,			/* FastScan_SM */\
  		-1,						/* timerFS */\
  		{{0,0,0,0},{0,0,0,0}}   /* lss_fs_transfer */
#else
#define lss_fs_Initializer
#endif		

#define lss_Initializer {\
		LSS_RESET,  			/* state */\
		0,						/* command */\
		LSS_WAITING_MODE, 		/* mode */\
		0,						/* dat1 */\
		0,						/* dat2 */\
		0,          			/* NodeID */\
		0,          			/* addr_sel_match */\
		0,          			/* addr_ident_match */\
		"none",         		/* BaudRate */\
		0,          			/* SwitchDelay */\
		SDELAY_OFF,   			/* SwitchDelayState */\
		NULL,					/* canHandle_t */\
		-1,						/* TimerMSG */\
		-1,          			/* TimerSDELAY */\
		NULL,        			/* Callback */\
		0						/* LSSanswer */\
		lss_fs_Initializer		/*FastScan service initialization */\
	  },\
	  NULL 	/* _lss_StoreConfiguration*/
#else
#define lss_Initializer
#endif


/* A macro to initialize the data in client app.*/
/* CO_Data structure */
#define CANOPEN_NODE_DATA_INITIALIZER(NODE_PREFIX) {\
	/* Object dictionary*/\
	& NODE_PREFIX ## _bDeviceNodeId,     /* bDeviceNodeId */\
	NODE_PREFIX ## _objdict,             /* objdict  */\
	NODE_PREFIX ## _PDO_status,          /* PDO_status */\
	& NODE_PREFIX ## _firstIndex,        /* firstIndex */\
	& NODE_PREFIX ## _lastIndex,         /* lastIndex */\
	& NODE_PREFIX ## _ObjdictSize,       /* ObjdictSize */\
	& NODE_PREFIX ## _iam_a_slave,       /* iam_a_slave */\
	NODE_PREFIX ## _valueRangeTest,      /* valueRangeTest */\
	\
	/* SDO, structure s_transfer */\
	{\
          REPEAT_SDO_MAX_SIMULTANEOUS_TRANSFERTS_TIMES(s_transfer_Initializer)\
	},\
	\
	/* State machine*/\
	Unknown_state,      /* nodeState */\
	/* structure s_state_communication */\
	{\
		0,          /* csBoot_Up */\
		0,          /* csSDO */\
		0,          /* csEmergency */\
		0,          /* csSYNC */\
		0,          /* csHeartbeat */\
		0,           /* csPDO */\
		0           /* csLSS */\
	},\
	_initialisation,     /* initialisation */\
	_preOperational,     /* preOperational */\
	_operational,        /* operational */\
	_stopped,            /* stopped */\
	NULL,                /* NMT node reset callback */\
	NULL,                /* NMT communications reset callback */\
	\
	/* NMT-heartbeat */\
	& NODE_PREFIX ## _highestSubIndex_obj1016, /* ConsumerHeartbeatCount */\
	NODE_PREFIX ## _obj1016,                   /* ConsumerHeartbeatEntries */\
	NODE_PREFIX ## _heartBeatTimers,           /* ConsumerHeartBeatTimers  */\
	& NODE_PREFIX ## _obj1017,                 /* ProducerHeartBeatTime */\
	TIMER_NONE,                                /* ProducerHeartBeatTimer */\
	_heartbeatError,           /* heartbeatError */\
	\
	{REPEAT_NMT_MAX_NODE_ID_TIMES(NMTable_Initializer)},\
                                                   /* is  well initialized at "Unknown_state". Is it ok ? (FD)*/\
	\
	/* SYNC */\
	TIMER_NONE,                                /* syncTimer */\
	& NODE_PREFIX ## _obj1005,                 /* COB_ID_Sync */\
	& NODE_PREFIX ## _obj1006,                 /* Sync_Cycle_Period */\
	/*& NODE_PREFIX ## _obj1007, */            /* Sync_window_length */\
	_post_sync,                 /* post_sync */\
	_post_TPDO,                 /* post_TPDO */\
	_post_SlaveBootup,			/* post_SlaveBootup */\
	\
	/* General */\
	0,                                         /* toggle */\
	NULL,                   /* canSend */\
	NODE_PREFIX ## _scanIndexOD,                /* scanIndexOD */\
	_storeODSubIndex,                /* storeODSubIndex */\
    /* DCF concise */\
    NULL,       /*dcf_odentry*/\
	NULL,		/*dcf_cursor*/\
	1,		/*dcf_entries_count*/\
	0,		/* dcf_request*/\
	\
	/* EMCY */\
	Error_free,                      /* error_state */\
	sizeof(NODE_PREFIX ## _obj1003) / sizeof(NODE_PREFIX ## _obj1003[0]),      /* error_history_size */\
	& NODE_PREFIX ## _highestSubIndex_obj1003,    /* error_number */\
	& NODE_PREFIX ## _obj1003[0],    /* error_first_element */\
	& NODE_PREFIX ## _obj1001,       /* error_register */\
    & NODE_PREFIX ## _obj1014,       /* error_cobid */\
	/* error_data: structure s_errors */\
	{\
	REPEAT_EMCY_MAX_ERRORS_TIMES(ERROR_DATA_INITIALIZER)\
	},\
	_post_emcy,              /* post_emcy */\
	/* LSS */\
	lss_Initializer\
}

#ifdef __cplusplus
};
#endif

#endif /* __data_h__ */


