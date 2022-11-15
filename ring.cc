#include "CNode.h"

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
	const auto leftNodeId = readInputNumber("Insert left reciever node id: ");
	const auto rightNodeId = readInputNumber("Insert right reciever node id: ");

	const auto portNum = 60000u;
	const auto ip = "127.0.0.1";

	vector<shared_ptr<CNode> > ring;

	const auto getAddress = [ip](unsigned p_port)
	{
		stringstream ss;
		ss << ip << ":" << p_port;
		return ss.str();
	};

	// creating nodes
	for (auto i = 0u; i < nodeCount; ++i)
	{
		ring.emplace_back(make_shared<CNode>(i,
					getAddress(portNum + i), // right skeleton addr
					getAddress(portNum + i + nodeCount), // left skeleton addr
					getAddress(portNum + (i + 1) % nodeCount), // right neighbor
					getAddress(portNum + (i + nodeCount - 1) % nodeCount + nodeCount))); // left neighbor
	}

	// starting skeletons
	for (auto node : ring)
		node->StartSkeleton();
	// starting stubs
	for (auto node : ring)
		node->StartStub();

	auto left = thread([&](){ring[0]->InjectMessage(EDirection::Left, leftNodeId, "Nothing left interesting");});
	auto right = thread([&](){ring[0]->InjectMessage(EDirection::Right, rightNodeId, "Radical right");});

	right.join();
	left.join();

	for (const auto& node : ring)
	{
		cout << "Node " << node->GetNodeId() << " values: " << endl;
		const auto result = node->GetResult();
		if (result.empty())
		{
			cout << "nothing" << endl;
			continue;
		}
		
		for (const auto& pair : result)
			cout << "content: " << pair.first << " response: " << pair.second << endl;
	}

	// wait for keypress
	int x;
	cin >> x;

	return 0;
}
