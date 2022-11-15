#include "CUnit.h"

using namespace RING;

using namespace grpc;
using namespace std;

unsigned CUnit::SendMessage(unsigned p_receiverId, unsigned p_senderId, const string& p_content)
{
    // assemble request
    ring::sendMessageRequest request;
    request.set_receiverid(p_receiverId);
    request.set_senderid(p_senderId);
    request.set_content(p_content);

    // container for server response
    ring::sendMessageResponse reply;
    // Context can be used to send meta data to server or modify RPC behaviour
    ClientContext context;
    // Actual Remote Procedure Call
    const auto status = m_stub->sendMessage(&context, request, &reply);

    // Returns results based on RPC status
    if (!status.ok())
    {
        throw exception();
    }

    return reply.result();
}

Status CUnit::sendMessage(ServerContext* p_context,
        const ring::sendMessageRequest* p_request,
        ring::sendMessageResponse* p_reply)
{
    // message is for this node
    if (p_request->receiverid() == m_nodeId)
    {
        // delivered
        p_reply->set_result(p_request->content().length());
    }
    else
    {
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

void CUnit::InjectMessage(unsigned p_receiverId, const string& p_content)
{
    const lock_guard<mutex> guard(m_mutex);
    m_callback(p_content, SendMessage(p_receiverId, m_nodeId, p_content));
}

void CUnit::CreateSkeleton()
{
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism
    builder.AddListeningPort(m_skeletonAddress, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which
    // communication with client takes place
    builder.RegisterService(this);
    // Assembling the skeleton interface
    m_skeleton = builder.BuildAndStart();
    // server started;
    m_initialized = true;
    m_skeleton->Wait();
}

void CUnit::StartSkeleton()
{
    m_thread = make_unique<thread>(&CUnit::CreateSkeleton, this);
    // wait until the thread is ready
    while (m_initialized != true) {}
}

void CUnit::StartStub()
{
    // Instantiates the client
    m_channel =  grpc::CreateChannel(m_neighborAddress, grpc::InsecureChannelCredentials());
    m_stub = ring::Unit::NewStub(m_channel);
}