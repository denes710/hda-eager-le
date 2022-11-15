#include "CNode.h"

using namespace RING;

using namespace std;
using namespace std::placeholders;

CNode::CNode(unsigned p_nodeId,
        const std::string& p_rightSkeletonAddress,
        const std::string& p_leftSkeletonAddress,
        const std::string& p_rightNeighborAddress,
        const std::string& p_leftNeighborAddress)
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

void CNode::ResultCallback(const std::string& p_content, unsigned p_result)
{
    const lock_guard<mutex> guard(m_resultMutex);
    m_result.push_back(make_pair(p_content, p_result));
}

void CNode::InjectMessage(EDirection p_direction, unsigned p_receiverId, const std::string& p_content)
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