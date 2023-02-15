#include "NeighborInfoModule.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "RTC.h"

#define MAX_NEIGHBOR_AGE 10 * 60 * 1000 // 10 minutes
#define MAX_NUM_NEIGHBORS 10            // also defined in NeighborInfo protobuf options
NeighborInfoModule *neighborInfoModule;

/*
Prints a single neighbor info packet and associated neighbors
Uses LOG_DEBUG, which equates to Console.log
NOTE: For debugging only
*/
void printNeighborInfo(const char *header, const meshtastic_NeighborInfo *np)
{
    LOG_DEBUG("%s NEIGHBORINFO PACKET\n----------------\n", header);
    LOG_DEBUG("Packet contains %d neighbors\n", np->neighbors_count);
    for (int i = 0; i < np->neighbors_count; i++) {
        LOG_DEBUG("Neighbor %d: node_id=%d, rx_time=%d, snr=%d\n", i, np->neighbors[i].node_id, np->neighbors[i].rx_time,
                  np->neighbors[i].snr);
    }
    LOG_DEBUG("----------------\n");
}

/*
Prints the nodeDB with selectors for the neighbors we've chosen to send (inefficiently)
Uses LOG_DEBUG, which equates to Console.log
NOTE: For debugging only
*/
void printNodeDBSelection(const char *header, const meshtastic_NeighborInfo *np)
{
    int num_nodes = nodeDB.getNumNodes();
    LOG_DEBUG("%s NODEDB SELECTION:\n----------------\n", header);
    LOG_DEBUG("Selected %d neighbors of %d DB nodes\n", np->neighbors_count, num_nodes);
    for (int i = 0; i < num_nodes; i++) {
        meshtastic_NodeInfo *dbEntry = nodeDB.getNodeByIndex(i);
        bool chosen = false;
        for (int j = 0; j < np->neighbors_count; j++) {
            if (np->neighbors[j].node_id == dbEntry->num) {
                chosen = true;
            }
        }
        if (!chosen) {
            LOG_DEBUG("     Node %d: node_id=%d, rx_time=%d, snr=%d\n", i, dbEntry->num, dbEntry->last_heard, dbEntry->snr);
        } else {
            LOG_DEBUG("---> Node %d: node_id=%d, rx_time=%d, snr=%d\n", i, dbEntry->num, dbEntry->last_heard, dbEntry->snr);
        }
    }
    LOG_DEBUG("----------------\n");
}

/* Send our initial owner announcement 30 seconds after we start (to give network time to setup) */
// TODO: meshtastic_PortNum_NEIGHBORINFO_APP
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
    printNodeDBSelection("COLLECTED", neighborInfo);
    return neighborInfo->neighbors_count;
}

/* Send neighbor info to the mesh */
void NeighborInfoModule::sendNeighborInfo(NodeNum dest, bool wantReplies)
{
    meshtastic_NeighborInfo *neighborInfo = allocateNeighborInfoPacket();
    collectNeighborInfo(neighborInfo);
    meshtastic_MeshPacket *p = allocDataProtobuf(*neighborInfo);
    p->to = dest;
    p->decoded.want_response = wantReplies;
    printNeighborInfo("SENDING", neighborInfo);
    service.sendToMesh(p);
}

/*
Encompasses the full construction and sending packet to mesh
Will be used for broadcast.
*/
int32_t NeighborInfoModule::runOnce()
{
    bool requestReplies = false;
    sendNeighborInfo(NODENUM_BROADCAST, requestReplies);
    return default_broadcast_interval_secs * 1000;
}

/*
Collect a recieved neighbor info packet from another node
Pass it to an upper client; do not persist this data on the mesh
*/
bool NeighborInfoModule::handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_NeighborInfo *np)
{
    // TODO: check if we want to handle this packet
    printNeighborInfo("RECIEVED", np);
    // No need for others to handle this packet
    return true;
}
