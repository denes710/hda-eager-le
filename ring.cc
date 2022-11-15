#include "CNode.h"
#include "CLogger.h"

#include <string>
#include <thread>
#include <vector>
#include <sstream>

using namespace RING;

using namespace std;

int main(int argc, char* argv[])
{
    ifstream infile("topologies.txt");
    string line;
    vector<vector<unsigned>> topologies;
    
    const unsigned testSize = 1;
    auto numberOfTests = 0u;

    while (getline(infile, line) && numberOfTests < testSize)
    {
        std::cout << line << std::endl;
        stringstream ss(line);
        vector<unsigned> ids;

        string tmp;

        while(getline(ss, tmp, ','))
            ids.push_back(stoul(tmp));

        topologies.push_back(ids);
        ++numberOfTests;
    }
    
    // FIXME randoms shuffle

	const auto defaultPortNum = 40000u;
    auto portNumNow = defaultPortNum;
	const auto ip = "127.0.0.1";

	const auto getAddress = [ip](unsigned p_port)
	{
		stringstream ss;
		ss << ip << ":" << p_port;
		return ss.str();
	};

	for (const auto topology : topologies)
    {
        vector<shared_ptr<CNode> > ring;
        auto logger = make_shared<CLogger>("log");
        const auto nodeCount = topology.size();

        for (auto i = 0u; i < nodeCount; ++i)
        {
            const auto rightOrderId = (i + 1) % nodeCount;
            const auto leftOrderId = (i + nodeCount - 1) % nodeCount;
            ring.emplace_back(make_shared<CNode>(topology[i],
                        logger,
                        getAddress(portNumNow + i), // right skeleton addr
                        getAddress(portNumNow + i + nodeCount), // left skeleton addr
                        CUnit::SNeighbour{topology[rightOrderId], getAddress(portNumNow + rightOrderId)}, // right neighbor
                        CUnit::SNeighbour{topology[leftOrderId], getAddress(portNumNow + leftOrderId + nodeCount)})); // left neighbor
        }

        portNumNow = portNumNow + 2 * nodeCount;

        // starting skeletons
        for (auto node : ring)
            node->StartSkeleton();
        // starting stubs
        for (auto node : ring)
            node->StartStub();
        // starting stubs
        for (auto node : ring)
            node->RunHDAEagerLE();
    }

	return 0;
}
