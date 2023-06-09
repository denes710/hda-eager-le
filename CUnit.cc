#include "CUnit.h"

using namespace RING;

using namespace grpc;
using namespace std;

void CUnit::SendNodeId(unsigned p_nodeId)
{
    m_logger->AddLog(m_nodeId, m_nodeId, m_neighbour.m_nodeId);

    // assemble request
    ring::sendNodeIdRequest request;
    request.set_sender_id(m_nodeId);
    request.set_node_id(p_nodeId);

    // container for server response
    ring::sendNodeIdResponse reply;
    // Context can be used to send meta data to server or modify RPC behaviour
    ClientContext context;
    // Actual Remote Procedure Call
    const auto status = m_stub->sendNodeId(&context, request, &reply);
}

Status CUnit::sendNodeId(ServerContext* p_context,
        const ring::sendNodeIdRequest* p_request,
        ring::sendNodeIdResponse* p_reply)
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

void CUnit::SendLeaderElected(unsigned p_leaderId)
{
    m_logger->AddLog(m_nodeId, m_nodeId, m_neighbour.m_nodeId);

    // assemble request
    ring::sendLeaderElectedRequest request;
    request.set_sender_id(m_nodeId);
    request.set_leader_id(p_leaderId);

    // container for server response
    ring::sendLeaderElectedResponse reply;
    // Context can be used to send meta data to server or modify RPC behaviour
    ClientContext context;
    // Actual Remote Procedure Call
    const auto status = m_stub->sendLeaderElected(&context, request, &reply);
}

Status CUnit::sendLeaderElected(ServerContext* p_context,
        const ring::sendLeaderElectedRequest* p_request,
        ring::sendLeaderElectedResponse* p_reply)
{
    m_logger->AddLog(m_nodeId, p_request->sender_id(), m_nodeId);

    m_callback(EMessageType::LeaderElected, p_request->leader_id());

    // message processed
    return Status::OK;
}

CUnit::~CUnit()
{
    m_skeleton->Shutdown();
    if (m_thread->joinable())
        m_thread->join();
}