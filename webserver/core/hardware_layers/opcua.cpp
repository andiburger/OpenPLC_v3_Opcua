//-----------------------------------------------------------------------------
// Copyright 2015 Thiago Alves
//
// Based on the LDmicro software by Jonathan Westhues
// This file is part of the OpenPLC Software Stack.
//
// OpenPLC is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenPLC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenPLC.  If not, see <http://www.gnu.org/licenses/>.
//------
//
// This file is the hardware layer for the OpenPLC. If you change the platform
// where it is running, you may only need to change this file. All the I/O
// related stuff is here. Basically it provides functions to read and write
// to the OpenPLC internal buffers in order to update I/O state.
// Thiago Alves, Dec 2015
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>
#include <open62541/server.h>
#include <string>

#include "ladder.h"
#include "custom_layer.h"


extern std::vector<std::vector<UA_NodeId>> digitalInputs;
extern std::vector<std::vector<UA_NodeId>> digitalOutputs;
extern std::vector<UA_NodeId> analogInputs;
extern std::vector<UA_NodeId> analogOutputs;
extern UA_Server* server;


/* Booleans */
static IEC_BOOL last_bool_input[BUFFER_SIZE][8];
static IEC_BOOL last_bool_output[BUFFER_SIZE][8];

/* Bytes */
static IEC_BYTE last_byte_input[BUFFER_SIZE];
static IEC_BYTE last_byte_output[BUFFER_SIZE];

/* Analog I/O */
static IEC_UINT last_int_input[BUFFER_SIZE];
static IEC_UINT last_int_output[BUFFER_SIZE];

/* Memory */
static IEC_UINT last_int_memory[BUFFER_SIZE];
static IEC_DINT last_dint_memory[BUFFER_SIZE];
static IEC_LINT last_lint_memory[BUFFER_SIZE];

static unsigned char log_msg[1000];

#define FLOAT_TO_INT(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))

//-----------------------------------------------------------------------------
// This function is called by the bufferIn and bufferOut methods to access the 
// UA node store programmatically
//-----------------------------------------------------------------------------
int concat(int a, int b)
{
	//TODO check for a more efficient way
    // Convert both the integers to string
    std::string s1 = std::to_string(a);
    std::string s2 = std::to_string(b);
    // Concatenate both strings
    std::string s = s1 + s2;
    // Convert the concatenated string
    // to integer
    int c = stoi(s);
    // return the formed integer
    return c;
}

//-----------------------------------------------------------------------------
// This function is called by the main OpenPLC routine when it is initializing.
// Hardware initialization procedures should be here.
//-----------------------------------------------------------------------------
void initializeHardware()
{
}

//-----------------------------------------------------------------------------
// This function is called by the main OpenPLC routine when it is finalizing.
// Resource clearing procedures should be here.
//-----------------------------------------------------------------------------
void finalizeHardware()
{
}

//-----------------------------------------------------------------------------
// This function is called by the OpenPLC in a loop. Here the internal buffers
// must be updated to reflect the actual Input state. The mutex bufferLock
// must be used to protect access to the buffers on a threaded environment.
//-----------------------------------------------------------------------------
void updateBuffersIn()
{
	UA_StatusCode retval;
	pthread_mutex_lock(&bufferLock); //lock mutex
	/*********READING FROM I/O**************/
	/* Write a different integer value */
	if (server != NULL) {
			for (int i = 0; i < 100; ++i) 
		{
		for (int j = 0; j < 8; ++j) 
			{
				UA_Variant var;
				UA_Variant_init(&var);
				retval = UA_Server_readValue(server, digitalInputs[i][j], &var);
				
					if (retval == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&var,&UA_TYPES[UA_TYPES_BOOLEAN]))
					{
							UA_Boolean rawData = *(UA_Boolean *) var.data;
						if (last_bool_input[i][j] != rawData){
							*bool_input[i][j] = rawData;
							last_bool_input[i][j] = *bool_input[i][j];
							sprintf((char*)log_msg, "OPC UA Server: Digital Input updated\n");
							log(log_msg);
						}
					}
			} 
			
			UA_Variant var;
			UA_Variant_init(&var);
			retval = UA_Server_readValue(server, analogInputs[i], &var);
			if (retval == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&var,&UA_TYPES[UA_TYPES_FLOAT]))
			{
				UA_Float rawData = *(UA_Float *) var.data;
				int value = FLOAT_TO_INT(rawData);
				if (last_int_input[i] != value) {
					*int_input[i] = value;
					last_int_input[i] = *int_input[i]; 
					sprintf((char*)log_msg, "OPC UA Server: Analog Input updated\n");
					log(log_msg);
				}
			}
		}
	}
		/**************************************************/

		pthread_mutex_unlock(&bufferLock); //unlock mutex
	}

	//-----------------------------------------------------------------------------
	// This function is called by the OpenPLC in a loop. Here the internal buffers
	// must be updated to reflect the actual Output state. The mutex bufferLock
	// must be used to protect access to the buffers on a threaded environment.
	//-----------------------------------------------------------------------------
	void updateBuffersOut()
	{
		UA_StatusCode retval;
		pthread_mutex_lock(&bufferLock); //lock mutex
		/*********WRITING TO I/O**************/
		if (server != NULL) {
			for (int i = 0; i < 100; ++i) 
		{
			for (int j = 0; j < 8; ++j) 
			{
				if  (last_bool_output[i][j] != *bool_output[i][j])
				{ 
					UA_Boolean value = *bool_output[i][j];
					UA_Variant var;
					UA_Variant_init(&var);
					UA_Variant_setScalar(&var, &value, &UA_TYPES[UA_TYPES_BOOLEAN]);
					retval = UA_Server_writeValue(server, digitalOutputs[i][j], var);
					if (retval != UA_STATUSCODE_GOOD) {
						sprintf((char*)log_msg, "OPC UA Server: Error updating Digital Output\n");
        				log(log_msg);
					}
					last_bool_output[i][j] = *bool_output[i][j];
					sprintf((char*)log_msg, "OPC UA Server: Digital Output %i updated with value %i \n",concat(i,j),*bool_output[i][j]);
        			log(log_msg);
				}
			}
			if (last_int_output[i] != *int_output[i]) 
			{
				UA_Float value = (float) *int_output[i];
				UA_Variant var;
				UA_Variant_init(&var);
				UA_Variant_setScalar(&var, &value, &UA_TYPES[UA_TYPES_FLOAT]);
				UA_Server_writeValue(server, analogOutputs[i], var);
				last_int_output[i] = *int_output[i];
				sprintf((char*)log_msg, "OPC UA Server: Analog Output %i updated with value %i \n",i,*int_output[i]);
        			log(log_msg);
			}
		}
	}
	
	/**************************************************/

	pthread_mutex_unlock(&bufferLock); //unlock mutex
}

