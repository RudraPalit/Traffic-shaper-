// TrafficShapingTopology.ned
import ned.DatarateChannel;

simple StationA
{
    parameters:
        double meanInterArrivalTime; // Mean inter-arrival time for Poisson distribution
    gates:
        output out;
}

simple TrafficShaper
{
    parameters:
        int queueSize; // Size of the FIFO queue
        double tokenRate; // Rate at which tokens are added to the bucket (tokens per second)
        int bucketSize; // Maximum number of tokens in the bucket
        double packetSendingRate; // Rate to send packets if tokens are available
    gates:
        input in;
        output out;
}

simple StationB
{
    gates:
        input in;
}

network TrafficShapingTopology
{
    parameters:
        @display("bgb=650,200");
        
    submodules:
        stationA: StationA {
            @display("p=100,100");
        }
        trafficShaper: TrafficShaper {
            @display("p=300,100");
        }
        stationB: StationB {
            @display("p=500,100");
        }
        
    connections:
        stationA.out --> trafficShaper.in;
        trafficShaper.out --> stationB.in;
}
