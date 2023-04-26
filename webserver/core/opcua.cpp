#include <open62541.h>
#include "ladder.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

static unsigned char log_msg[1000];
vector<vector<UA_NodeId>> digitalInputs;
vector<vector<UA_NodeId>> digitalOutputs;
vector<UA_NodeId> analogInputs;
vector<UA_NodeId> analogOutputs;
UA_Server *server;

UA_Boolean running = true;

void stopHandler(){
    running = false;
    UA_Server_delete(server);
} 


int buildInformationModel(UA_Server *server)
{
    //TODO add own namespace
    //TODO free allocations
    UA_NodeId openPlcId;
    UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
    oAttr.displayName = UA_LOCALIZEDTEXT("en-US", "openPLC");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, "openPLC"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            oAttr, NULL, &openPlcId);
    //UA_ObjectAttributes_clear(&oAttr);
    UA_NodeId inputsId;//object node for all inputs
    UA_ObjectAttributes oAttrInputs = UA_ObjectAttributes_default;
    oAttrInputs.displayName = UA_LOCALIZEDTEXT("en-US", "Inputs");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            openPlcId,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, "Inputs"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            oAttrInputs, NULL, &inputsId);
    //UA_ObjectAttributes_clear(&oAttrInputs);
    UA_NodeId binputsId;//object node for digital inputs
    UA_ObjectAttributes oAttrBInputs = UA_ObjectAttributes_default;
    oAttrBInputs.displayName = UA_LOCALIZEDTEXT("en-US", "Digital Inputs");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            inputsId,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, "Digital Inputs"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            oAttrBInputs, NULL, &binputsId);
    //UA_ObjectAttributes_clear(&oAttrBInputs);
    
    UA_NodeId ainputsId;//object node for analog inputs
    UA_ObjectAttributes oAttrAInputs = UA_ObjectAttributes_default;
    oAttrAInputs.displayName = UA_LOCALIZEDTEXT("en-US", "Analog Inputs");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            inputsId,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, "Analog Inputs"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            oAttrAInputs, NULL, &ainputsId);
    //UA_ObjectAttributes_clear(&oAttrAInputs);   

    UA_NodeId outputsId;//Object node for all outputs
    UA_ObjectAttributes oAttrOutputs = UA_ObjectAttributes_default;
    oAttrOutputs.displayName = UA_LOCALIZEDTEXT("en-US", "Outputs");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            openPlcId,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, "Outputs"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            oAttrOutputs, NULL, &outputsId);
    //UA_ObjectAttributes_clear(&oAttrOutputs);

    UA_NodeId boutputsId;//Object node for digital outputs
    UA_ObjectAttributes oAttrBOutputs = UA_ObjectAttributes_default;
    oAttrBOutputs.displayName = UA_LOCALIZEDTEXT("en-US", "Digital Outputs");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            outputsId,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, "Digital Outputs"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            oAttrBOutputs, NULL, &boutputsId);
    //UA_ObjectAttributes_clear(&oAttrBOutputs);

    UA_NodeId aoutputsId;//Object node for analog outputs
    UA_ObjectAttributes oAttrAOutputs = UA_ObjectAttributes_default;
    oAttrAOutputs.displayName = UA_LOCALIZEDTEXT("en-US", "Analog Outputs");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            outputsId,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, "Analog Outputs"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            oAttrAOutputs, NULL, &aoutputsId);
    //UA_ObjectAttributes_clear(&oAttrAOutputs);
   for (int i = 0; i < 100; ++i) 
    {
        for (int j = 0; j < 8; ++j) 
        {   /* Digital Inputs are created here */
            if (bool_input[i][j]) {
                UA_VariableAttributes boolInputAttr = UA_VariableAttributes_default;
                UA_Boolean status;
                UA_NodeId tmpNodeId;
                if (*bool_input[i][j]) 
                    status = true;
                else 
                    status = false;
                auto name = "%%IX"+std::to_string(i)+"." + std::to_string(j);
                UA_Variant_setScalar(&boolInputAttr.value, &status, &UA_TYPES[UA_TYPES_BOOLEAN]);
                boolInputAttr.displayName = UA_LOCALIZEDTEXT("en-US", (char*)name.c_str());
                boolInputAttr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
                boolInputAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
                UA_Server_addVariableNode(server, UA_NODEID_NULL, binputsId,
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                        UA_QUALIFIEDNAME(1, (char*)name.c_str()),
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                        boolInputAttr, NULL, &tmpNodeId);
                digitalInputs[i][j]=tmpNodeId; // adding node id to node store
            }
            /* Digital Outputs are created here */
            if (bool_output[i][j]) {
                UA_VariableAttributes boolOutputAttr = UA_VariableAttributes_default;
                UA_Boolean status;
                UA_NodeId tmpNodeId;
                if (*bool_input[i][j]) 
                    status = true;
                else 
                    status = false;
                auto name = "%%QX"+std::to_string(i)+"." + std::to_string(j);
                UA_Variant_setScalar(&boolOutputAttr.value, &status, &UA_TYPES[UA_TYPES_BOOLEAN]);
                boolOutputAttr.displayName = UA_LOCALIZEDTEXT("en-US", (char*)name.c_str());
                boolOutputAttr.dataType = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
                boolOutputAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
                UA_Server_addVariableNode(server, UA_NODEID_NULL, boutputsId,
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                        UA_QUALIFIEDNAME(1, (char*)name.c_str()),
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                        boolOutputAttr, NULL, &tmpNodeId);
                digitalOutputs[i][j]=tmpNodeId;  // add node id to digital output node store
            }
        }
        /* Analog inputs are created here */
       if (int_input[i] != NULL) {
            UA_NodeId tmpNodeId;
            UA_VariableAttributes intInputAttr = UA_VariableAttributes_default;
            auto name = "%%IW"+std::to_string(i);
            UA_UInt16 value = *int_input[i];
            UA_Variant_setScalar(&intInputAttr.value, &value, &UA_TYPES[UA_TYPES_UINT16]);
            intInputAttr.displayName = UA_LOCALIZEDTEXT("en-US", (char*)name.c_str());
            intInputAttr.dataType = UA_TYPES[UA_TYPES_UINT16].typeId;
            intInputAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
            UA_Server_addVariableNode(server, UA_NODEID_NULL, ainputsId,
                                    UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                    UA_QUALIFIEDNAME(1, (char*)name.c_str()),
                                    UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                    intInputAttr, NULL, &tmpNodeId);
            analogInputs.push_back(tmpNodeId); // add node id to analog input node store
        }
        /* Analog outputs are created here */
        if (int_output[i] != NULL) {
            UA_NodeId tmpNodeId;
            UA_VariableAttributes outInputAttr = UA_VariableAttributes_default;
            auto name = "%%QW"+std::to_string(i);
            UA_UInt16 value = *int_output[i];
            UA_Variant_setScalar(&outInputAttr.value, &value, &UA_TYPES[UA_TYPES_UINT16]);
            outInputAttr.displayName = UA_LOCALIZEDTEXT("en-US", (char*)name.c_str());
            outInputAttr.dataType = UA_TYPES[UA_TYPES_UINT16].typeId;
            outInputAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
            UA_Server_addVariableNode(server, UA_NODEID_NULL, aoutputsId,
                                    UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                    UA_QUALIFIEDNAME(1, (char*)name.c_str()),
                                    UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                    outInputAttr, NULL, &tmpNodeId);
            analogOutputs.push_back(tmpNodeId);
        }
    }
    return 0;
}

void opcuaStartServer(int port)
{
    digitalInputs = {};
    digitalOutputs = {};
    analogInputs = {};
    analogOutputs = {};
    server = UA_Server_new();
    unsigned char log_msg[1000];
    for (int i = 0; i < 100; ++i) 
    {/* just for initialization of the multidimensional node store */
        vector<UA_NodeId> d = {};
        vector<UA_NodeId> d0 = {};
        digitalInputs.push_back(d);
        digitalOutputs.push_back(d0);
        for (int j = 0; j < 8; ++j) {
            UA_NodeId tmp;
            digitalInputs[i].push_back(tmp);
            digitalOutputs[i].push_back(tmp);
        }
    }
    if (buildInformationModel(server))
    {
        sprintf((char*)log_msg, "OPC UA Server: error creating OPC UA information model\n");
        log(log_msg);
    }
    else {
        sprintf((char*)log_msg, "OPC UA Server: created OPC UA information model successfully\n");
        log(log_msg);
    }

    UA_StatusCode retval = UA_Server_run(server, &running);
}