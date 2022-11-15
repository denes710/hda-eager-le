#include "CNode.h"

using namespace RING;

using namespace std;
using namespace std::placeholders;

CNode::CNode(unsigned p_nodeId,
        const shared_ptr<CLogger>& p_logger,
        const std::string& p_rightSkeletonAddress,
        const std::string& p_leftSkeletonAddress,
        const CUnit::SNeighbour& p_rightNeighbour,
        const CUnit::SNeighbour& p_leftNeighbour)
    : m_nodeId(p_nodeId)
    , m_rightUnit(m_nodeId,
            p_logger,
            p_rightSkeletonAddress,
            p_rightNeighbour,
            EDirection::Right,
            bind(&CNode::ReceiveMessage, this, _1, _2))
    , m_leftUnit(m_nodeId,
            p_logger,
            p_leftSkeletonAddress,
            p_leftNeighbour,
            EDirection::Left,
            bind(&CNode::ReceiveMessage, this, _1, _2))
{}

void CNode::StartSkeleton()
{
    m_rightUnit.StartSkeleton();
    m_leftUnit.StartSkeleton();
}

void CNode::StartStub()
{
    m_rightUnit.StartStub();
    m_leftUnit.StartStub();
}

void CNode::ReceiveMessage(CUnit::EMessageType p_type, unsigned p_nodeId)
{
    {
        lock_guard<mutex> lock(m_mutex);
        m_queue.push({p_type, p_nodeId});
    }

    m_conVariable.notify_one();
}

void CNode::RunHDAEagerLE()
{
    m_thread = thread([&](){ Run(); });
}

void CNode::Run()
{
    SendNodeIdMessage(m_nodeId);

    while (true)
    {
        unique_lock<mutex> lock(m_mutex);
        m_conVariable.wait(lock, [&]{return !m_queue.empty();});

        const auto message = m_queue.front();
        m_queue.pop();

        switch (message.m_type)
        {
            case CUnit::EMessageType::ShareNodeId:
                break;
            case CUnit::EMessageType::LeaderElected:
                break;
        }
    }
}

CUnit& CNode::GetUnit(EDirection p_direction)
{
    switch (p_direction)
    {
        case EDirection::Right:
            return m_rightUnit;
        case EDirection::Left:
            return m_leftUnit;
    }
    
    return m_rightUnit;
}

void CNode::SendNodeIdMessage(unsigned p_nodeId)
{
    thread([&]()
    { GetUnit(m_direction).SendNodeIdMessage(p_nodeId); });
    m_direction = m_direction == EDirection::Right ? EDirection::Left : EDirection::Right;
}
