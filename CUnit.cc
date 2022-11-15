#include "CUnit.h"

using namespace RING;

using namespace grpc;
using namespace std;

void CUnit::SendNodeIdMessage(unsigned p_nodeId)
{
    m_logger->AddLog(m_nodeId, m_nodeId, m_neighbour.m_nodeId);

    // assemble request
    ring::sendNodeIdMessageRequest request;
    request.set_sender_id(m_nodeId);
    request.set_node_id(p_nodeId);

    // container for server response
    // FIXME perhaps it is not necessary
    ring::sendNodeIdMessageResponse reply;
    // Context can be used to send meta data to server or modify RPC behaviour
    ClientContext context;
    // Actual Remote Procedure Call
    const auto status = m_stub->sendNodeIdMessage(&context, request, &reply);

    // Returns results based on RPC status
    if (!status.ok())
    {
        throw exception();
    }
}

Status CUnit::sendNodeIdMessage(ServerContext* p_context,
        const ring::sendNodeIdMessageRequest* p_request,
        ring::sendNodeIdMessageResponse* p_reply)
{
    m_logger->AddLog(m_nodeId, p_request->sender_id(), m_nodeId);

    m_callback(EMessageType::ShareNodeId, p_request->node_id());

    // message processed
    return Status::OK;
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
    m_channel =  grpc::CreateChannel(m_neighbour.m_addr, grpc::InsecureChannelCredentials());
    m_stub = ring::Unit::NewStub(m_channel);
}