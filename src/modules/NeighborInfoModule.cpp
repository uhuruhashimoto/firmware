#include "NeighborInfoModule.h"
#include "NodeDB.h"

#define MAX_NEIGHBOR_AGE 10 * 60 * 1000 // 10 minutes
#define MAX_NUM_NEIGHBORS 10            // also defined in NeighborInfo protobuf options
NeighborInfoModule *neighborInfoModule;

/* Send our initial owner announcement 30 seconds after we start (to give network time to setup) */
NeighborInfoModule::NeighborInfoModule()
    : ProtobufModule("neighborinfo", meshtastic_PortNum_PRIVATE_APP, &meshtastic_NeighborInfo_msg), concurrency::OSThread(
                                                                                                        "NeighborInfoModule")
{
    ourPortNum = meshtastic_PortNum_PRIVATE_APP;
    setIntervalFromNow(30 * 1000);
}

/*
Collect a recieved neighbor info packet from another node
Pass it to an upper client; do not persist this data on the mesh
*/
bool handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_NeighborInfo *np)
{

    printNeighborInfo("RECIEVED", np);
}

/* Collect neighbor info from the nodeDB's history, capping at a maximum number of entries and max time */
uint32_t NeighborInfoModule::collectNeighborInfo(meshtastic_NeighborInfo *neighborInfo) {}

/* Send neighbor info to the mesh */
void NeighborInfoModule::sendNeighborInfo(NodeNum dest, bool wantReplies) {}

/*
Encompasses the full construction and sending packet to mesh
Will be used for broadcast.
*/
int32_t NeighborInfoModule::runOnce() {}

/*
Prints a single neighbor info packet and associated neighbors
NOTE: For debugging only
*/
void printNeighborInfo(const char *header, const meshtastic_NeighborInfo *np);

/*
Prints the nodeDB with selectors for the neighbors we've chosed
NOTE: For debugging only
*/
void printNodeDBSelection(const char *header, const int level);