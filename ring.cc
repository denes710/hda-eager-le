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
	const auto readInputNumber = [](const string& p_message)
	{
		unsigned value;
		cout << p_message;
		cin >> value;
		cout << endl;
		return value;
	};

	const auto nodeCount = readInputNumber("Insert number of nodes: ");

	const auto portNum = 60000u;
	const auto ip = "127.0.0.1";

	vector<shared_ptr<CNode> > ring;

	const auto getAddress = [ip](unsigned p_port)
	{
		stringstream ss;
		ss << ip << ":" << p_port;
		return ss.str();
	};

    auto logger = make_shared<CLogger>("log");

	// creating nodes
	for (auto i = 0u; i < nodeCount; ++i)
	{
        const auto rightId = (i + 1) % nodeCount;
        const auto leftId = (i + nodeCount - 1) % nodeCount;
		ring.emplace_back(make_shared<CNode>(i,
                    logger,
					getAddress(portNum + i), // right skeleton addr
					getAddress(portNum + i + nodeCount), // left skeleton addr
					CUnit::SNeighbour{rightId, getAddress(portNum + rightId)}, // right neighbor
					CUnit::SNeighbour{leftId, getAddress(portNum + leftId + nodeCount)})); // left neighbor
	}

	// starting skeletons
	for (auto node : ring)
		node->StartSkeleton();
	// starting stubs
	for (auto node : ring)
		node->StartStub();
	// starting stubs
	for (auto node : ring)
		node->RunHDAEagerLE();

    cout << "Waiting for the result";

	// wait for keypress
	int x;
	cin >> x;

	return 0;
}
