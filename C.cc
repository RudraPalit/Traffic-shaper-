#include <string.h>
#include <omnetpp.h>
#include <queue>

using namespace omnetpp;

class StationA : public cSimpleModule
{
private:
    double meanInterArrivalTime;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(StationA);

void StationA::initialize()
{
    meanInterArrivalTime = par("meanInterArrivalTime");
    // Schedule the first packet
    cMessage *msg = new cMessage("sendPacket");
    scheduleAt(simTime() + exponential(meanInterArrivalTime), msg);
}

void StationA::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // Create and send a new packet
        cPacket *pkt = new cPacket("packet");
        EV << "Sending packet: " << pkt->getName() << endl;
        send(pkt, "out");
        // Schedule next packet
        scheduleAt(simTime() + exponential(meanInterArrivalTime), msg);
    }
}

class TrafficShaper : public cSimpleModule
{
private:
    std::queue<cPacket *> packetQueue;

    int queueSize;

    // Rate at which tokens are added to the token bucket
    double tokenRate;

    // Maximum capacity of the token bucket
    int bucketSize;

    // Number of tokens currently available in the token bucket
    int currentTokens;

    // Self-message used to trigger the addition of tokens to the token bucket
    cMessage *tokenAdditionEvent = nullptr;

protected:
    // Method to initialize the module
    virtual void initialize() override;

    // Method to handle incoming messages
    virtual void handleMessage(cMessage *msg) override;

    // Method called when the simulation finishes
    virtual void finish() override;

    // Method to send packets from the queue when tokens are available
    void sendPacketFromQueue();
};


Define_Module(TrafficShaper);

void TrafficShaper::initialize()
{
    queueSize = par("queueSize");
    tokenRate = par("tokenRate");
    bucketSize = par("bucketSize");
    currentTokens = bucketSize; // Start with a full bucket
    tokenAdditionEvent = new cMessage("addToken");
    scheduleAt(simTime() + (1 / tokenRate), tokenAdditionEvent);
}

void TrafficShaper::handleMessage(cMessage *msg)
{
    if (msg == tokenAdditionEvent)
    {
        // Add tokens to the bucket
        if (currentTokens < bucketSize)
        {
            currentTokens++;
            EV << "Token added. Tokens in bucket: " << currentTokens << "/" << bucketSize << endl;
        }
        scheduleAt(simTime() + (1 / tokenRate), tokenAdditionEvent);
        sendPacketFromQueue();
    }
    else
    {
        // Incoming packet
        cPacket *pkt = check_and_cast<cPacket *>(msg);
        if (currentTokens > 0 && packetQueue.empty())
        {
            // Send packet immediately if tokens are available and queue is empty
            send(pkt, "out");
            currentTokens--;
            EV << "Sending packet: " << pkt->getName() << endl;
        }
        else
        {
            // Queue packet if the queue is not full
            if ((int)packetQueue.size() < queueSize)
            {
                packetQueue.push(pkt);
                EV << "Packet queued. Queue size: " << packetQueue.size() << "/" << queueSize << endl;
            }
            else
            {
                // Drop the packet if the queue is full
                EV << "Packet dropped. Queue full." << endl;
                delete pkt;
            }
        }
    }
}

void TrafficShaper::sendPacketFromQueue()
{
    // Send packets from the queue if tokens are available
    while (!packetQueue.empty() && currentTokens > 0)
    {
        cPacket *queuedPacket = packetQueue.front();
        packetQueue.pop();
        send(queuedPacket, "out");
        currentTokens--;
        EV << "Sending queued packet: " << queuedPacket->getName() << endl;
    }
}

void TrafficShaper::finish()
{
    // Clean up
    cancelAndDelete(tokenAdditionEvent);
    while (!packetQueue.empty())
    {
        delete packetQueue.front();
        packetQueue.pop();
    }
}

class StationB : public cSimpleModule
{
protected:
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(StationB);

void StationB::handleMessage(cMessage *msg)
{
    // Process received packet
    EV << "Received packet: " << msg->getName() << endl;
    delete msg;
}
