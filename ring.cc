#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <sstream>
#include <functional>

#include <grpcpp/grpcpp.h>
#include "unit.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

using ring::sendMessageRequest;
using ring::sendMessageResponse;
using ring::Unit;

using namespace std;
using namespace std::placeholders;

enum class EDirection
{
	Right,
	Left,
};

inline string GetStrFromDirection(EDirection p_direction)
{
	switch (p_direction)
	{
		case EDirection::Right:
			return "right";
		case EDirection::Left:
			return "left";
	}

	return "unknown direction";
}

class CUnit : public Unit::Service
{
	public:
		CUnit(unsigned p_nodeId,
				const string& p_skeletonAddress,
				const string& p_neighborAddress,
				EDirection p_direction,
				const function<void(string, unsigned)>& p_callback)
			: m_nodeId(p_nodeId)
			, m_skeletonAddress(p_skeletonAddress)
			, m_neighborAddress(p_neighborAddress)
			, m_direction(p_direction)
			, m_callback(p_callback)
		{}

		// send messag to neighbor stub
		unsigned SendMessage(unsigned p_receiverId, unsigned p_senderId, const string& p_content)
		{
			// assemble request
			sendMessageRequest request;
			request.set_receiverid(p_receiverId);
			request.set_senderid(p_senderId);
			request.set_content(p_content);

			// container for server response
			sendMessageResponse reply;
			// Context can be used to send meta data to server or modify RPC behaviour
			ClientContext context;
			// Actual Remote Procedure Call
			const auto status = m_stub->sendMessage(&context, request, &reply);

			// Returns results based on RPC status
			if (!status.ok())
			{
				cout << status.error_code() << ": " << status.error_message() << endl;
				throw exception();
			}

			return reply.result();
		}

		// receives message from neighbor stub
		Status sendMessage(ServerContext* p_context,
				const sendMessageRequest* p_request,
				sendMessageResponse* p_reply) override
		{
			// message is for this node
			if (p_request->receiverid() == m_nodeId)
			{
				cout << m_nodeId << " (" << GetStrFromDirection(m_direction) << ") got message from " <<
					p_request->senderid() << "\"" << p_request->content() <<
					"\"" << endl;
				// delivered

				p_reply->set_result(p_request->content().length());
			}
			else
			{
				cout << m_nodeId << " (" << GetStrFromDirection(m_direction) << "): sender " <<
					p_request->senderid() << " sends mesage to " <<
					p_request->receiverid() << " \"" << p_request->content() << "\"" << endl;

				auto result = 0u;

				{
					const lock_guard<mutex> guard(m_mutex);
					result = SendMessage(p_request->receiverid(),
							p_request->senderid(),
							p_request->content());
				}

				p_reply->set_result(result);
			}
			// message processed
			return Status::OK;
		}

		void InjectMessage(unsigned p_receiverId, const string& p_content)
		{
			const lock_guard<mutex> guard(m_mutex);
			m_callback(p_content, SendMessage(p_receiverId, m_nodeId, p_content));
		}

		void CreateSkeleton()
		{
			ServerBuilder builder;
			// Listen on the given address without any authentication mechanism
			builder.AddListeningPort(m_skeletonAddress, grpc::InsecureServerCredentials());
			// Register "service" as the instance through which
			// communication with client takes place
			builder.RegisterService(this);
			// Assembling the skeleton interface
			m_skeleton = builder.BuildAndStart();
			cout << "Server listening on port: " << m_skeletonAddress << endl;
			// server started;
			m_initialized = true;
			m_skeleton->Wait();
		}

		// creating a single thread serving as skeleton
		void StartSkeleton()
		{
			m_thread = make_unique<thread>(&CUnit::CreateSkeleton, this);
			// wait until the thread is ready
			while (m_initialized != true) {}
			cout << "Skeleton " << m_nodeId << " (" << GetStrFromDirection(m_direction) <<
				") is ready. Forwarding messages to " << m_neighborAddress << endl;
		}

		void StartStub()
		{
			// Instantiates the client
			m_channel =  grpc::CreateChannel(m_neighborAddress, grpc::InsecureChannelCredentials());
			m_stub = Unit::NewStub(m_channel);
		}

	private:
		const unsigned m_nodeId;
		mutex m_mutex;
		const string m_neighborAddress;
		unique_ptr<Unit::Stub> m_stub;
		shared_ptr<Channel> m_channel;
		const string m_skeletonAddress;
		unique_ptr<thread> m_thread;
		unique_ptr<Server> m_skeleton;
		bool m_initialized = false;
		const EDirection m_direction;
		const function<void(string, unsigned)> m_callback;
};

class CNode
{
	public:
		using TResult = vector<pair<string, unsigned>>;

		CNode(unsigned p_nodeId,
				const string& p_rightSkeletonAddress,
				const string& p_leftSkeletonAddress,
				const string& p_rightNeighborAddress,
				const string& p_leftNeighborAddress)
			: m_nodeId(p_nodeId)
			, m_rightUnit(m_nodeId,
					p_rightSkeletonAddress,
					p_rightNeighborAddress,
					EDirection::Right,
					bind(&CNode::ResultCallback, this, _1, _2))
			, m_leftUnit(m_nodeId,
					p_leftSkeletonAddress,
					p_leftNeighborAddress,
					EDirection::Left,
					bind(&CNode::ResultCallback, this, _1, _2))
		{}

		void StartSkeleton()
		{
			m_rightUnit.StartSkeleton();
			m_leftUnit.StartSkeleton();
		}

		void StartStub()
		{
			m_rightUnit.StartStub();
			m_leftUnit.StartStub();
		}

		void ResultCallback(const string& p_content, unsigned p_result)
		{
			const lock_guard<mutex> guard(m_resultMutex);
			m_result.push_back(make_pair(p_content, p_result));
		}

		void InjectMessage(EDirection p_direction, unsigned p_receiverId, const string& p_content)
		{
			switch (p_direction)
			{
				case EDirection::Right:
					m_rightUnit.InjectMessage(p_receiverId, p_content);
					break;
				case EDirection::Left:
					m_leftUnit.InjectMessage(p_receiverId, p_content);
					break;
			}
		}

		const TResult& GetResult() const
		{ return m_result; }

		unsigned GetNodeId() const
		{ return m_nodeId; }

	private:
		const unsigned m_nodeId;
		CUnit m_rightUnit;
		CUnit m_leftUnit;

		mutex m_resultMutex;
		TResult m_result;
};

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
