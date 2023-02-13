#include "NeighborInfoModule.h"
#include "NodeDB.h"
#include "RTC.h"

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
Allocate a zeroed neighbor info packet
*/
meshtastic_NeighborInfo *NeighborInfoModule::allocateNeighborInfoPacket()
{
    meshtastic_NeighborInfo *neighborInfo = (meshtastic_NeighborInfo *)malloc(sizeof(meshtastic_NeighborInfo));
    memset(neighborInfo, 0, sizeof(meshtastic_NeighborInfo));
    return neighborInfo;
}

/*
Collect neighbor info from the nodeDB's history, capping at a maximum number of entries and max time
Assumes that the neighborInfo packet has been allocated
@returns the number of entries collected
*/
uint32_t NeighborInfoModule::collectNeighborInfo(meshtastic_NeighborInfo *neighborInfo)
{
    // TODO: star graph; mask for real neighbors in the nodeDB
    int num_nodes = nodeDB.getNumNodes();
    int current_time = getTime();

    for (int i = 0; i < num_nodes; i++) {
        meshtastic_NodeInfo *dbEntry = nodeDB.getNodeByIndex(i);
        if ((current_time - dbEntry->last_heard) < MAX_NEIGHBOR_AGE && neighborInfo->neighbors_count < MAX_NUM_NEIGHBORS) {
            neighborInfo->neighbors[neighborInfo->neighbors_count].node_id = dbEntry->num;
            neighborInfo->neighbors[neighborInfo->neighbors_count].rx_time = dbEntry->last_heard;
            neighborInfo->neighbors[neighborInfo->neighbors_count].snr = dbEntry->snr;
            neighborInfo->neighbors_count++;
        }
    }
    return neighborInfo->neighbors_count;
}

/* Send neighbor info to the mesh */
void NeighborInfoModule::sendNeighborInfo(NodeNum dest, bool wantReplies)
{
    meshtastic_NeighborInfo *neighborInfo = allocateNeighborInfoPacket();
    collectNeighborInfo(neighborInfo);
}

/*
Encompasses the full construction and sending packet to mesh
Will be used for broadcast.
*/
int32_t NeighborInfoModule::runOnce() {}

/*
Collect a recieved neighbor info packet from another node
Pass it to an upper client; do not persist this data on the mesh
*/
bool handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_NeighborInfo *np)
{
    // TODO: check if we want to handle this packet
    printNeighborInfo("RECIEVED", np);
}

/*
Prints a single neighbor info packet and associated neighbors
NOTE: For debugging only
*/
void printNeighborInfo(const char *header, const meshtastic_NeighborInfo *np) {}

/*
Prints the nodeDB with selectors for the neighbors we've chosed
NOTE: For debugging only
*/
void printNodeDBSelection(const char *header, const meshtastic_NeighborInfo *np) {}