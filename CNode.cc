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
            bind(&CNode::ReceiveMessage, this, _1, _2, EDirection::Right))
    , m_leftUnit(m_nodeId,
            p_logger,
            p_leftSkeletonAddress,
            p_leftNeighbour,
            EDirection::Left,
            bind(&CNode::ReceiveMessage, this, _1, _2, EDirection::Left))
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

void CNode::ReceiveMessage(CUnit::EMessageType p_type, unsigned p_nodeId, EDirection p_direction)
{
    {
        lock_guard<mutex> lock(m_mutex);
        m_queue.push({p_type, p_nodeId, p_direction});
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

    bool running = true;

    while (running)
    {
        unique_lock<mutex> lock(m_mutex);
        m_conVariable.wait(lock, [&]{return !m_queue.empty();});

        const auto message = m_queue.front();
        m_queue.pop();

        switch (message.m_type)
        {
            case CUnit::EMessageType::ShareNodeId:
                switch (m_state)
                {
                    case EState::Candidate:
                        if (message.m_nodeId > m_nodeId)
                        {
                            SendNodeIdMessage(m_nodeId);
                        }
                        else if (message.m_nodeId < m_nodeId)
                        {
                            m_state = EState::Defeated;
                        }
                        else
                        {
                            m_state = EState::Leader;
                            SendLeaderElectedMessage(m_nodeId, EDirection::Both);
                        }
                        break;
                    case EState::Defeated:
                        SendNodeIdMessage(message.m_nodeId);
                        break;
                    default:
                        break; //FIXME exception maybe?
                }
                break;
            case CUnit::EMessageType::LeaderElected:
                SendLeaderElectedMessage(message.m_nodeId, GetInverse(message.m_fromDirection));
                m_state = EState::Terminated;
                running = false;
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
    thread tmp([&, direction = m_direction, nodeId = p_nodeId]()
    { GetUnit(direction).SendNodeId(nodeId); });
    tmp.detach();
    m_direction = GetInverse(m_direction);
}

void CNode::SendLeaderElectedMessage(unsigned p_nodeId, EDirection p_direction)
{
    if (p_direction == EDirection::Both)
    {
        thread tmpRight([&, nodeId = p_nodeId]()
        { m_rightUnit.SendLeaderElected(nodeId); });
        tmpRight.detach();
        thread tmpLeft([&, nodeId = p_nodeId]()
        { m_leftUnit.SendLeaderElected(nodeId); });
        tmpLeft.detach();

        return;
    }

    thread tmp([&, direction = m_direction, nodeId = p_nodeId]()
    { GetUnit(direction).SendLeaderElected(nodeId); });
    tmp.detach();
}