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

void CNode::ReceiveMessage(const string& p_content, unsigned p_result)
{
    // FIXME deals with phases
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