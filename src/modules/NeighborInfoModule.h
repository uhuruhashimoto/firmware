#pragma once
#include "ProtobufModule.h"
#include "mesh/generated/meshtastic/neighborinfo.pb.h"

/*
 * Neighborinfo module for sending info on each node's 0-hop neighbors to the mesh
 * This partners with neighbor bit fields in the NodeInfo packet/NodeDB to allow nodes to declare their status
 */
class NeighborInfoModule : public ProtobufModule<meshtastic_NeighborInfo>, private concurrency::OSThread
{
  public:
    /*
     * Expose the constructor
     */
    NeighborInfoModule();

    /*
     * Send info on our node's neighbors into the mesh
     */
    void sendNeighborInfo(NodeNum dest = NODENUM_BROADCAST, bool wantReplies = false);

  protected:
    /*
     * Called to handle a particular incoming message
     * @return true if you've guaranteed you've handled this message and no other handlers should be considered for it
     */
    virtual bool handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_NeighborInfo *nb) override;

    /*
     * Collect neighbor info from the nodeDB's history, capping at a maximum number of entries and max time
     * @return the number of entries collected
     */
    uint32_t collectNeighborInfo(meshtastic_NeighborInfo *neighborInfo);

    /* Allocate a new NeighborInfo packet */
    meshtastic_NeighborInfo *allocateNeighborInfoPacket();

    /* Does our periodic broadcast */
    int32_t runOnce() override;
};
extern NeighborInfoModule *neighborInfoModule;