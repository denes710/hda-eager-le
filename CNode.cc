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
{
    std::cout << "NodeId: " << p_nodeId << std::endl;
    std::cout << "p_rightSkeletonAddress: " << p_rightSkeletonAddress << std::endl;
    std::cout << "p_leftSkeletonAddress: " << p_leftSkeletonAddress << std::endl;
    std::cout << "p_rightNeighbour: " << p_rightNeighbour.m_nodeId << " p_rightNeighbour.add: " << p_rightNeighbour.m_addr << std::endl;
    std::cout << "p_leftNeighbour: " << p_leftNeighbour.m_nodeId << " p_leftNeighbour.add: " << p_leftNeighbour.m_addr << std::endl;
}

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
    // FIXME a queue and another thread are necessary
}

void CNode::StartHDAEagerLE()
{
    // FIXME just start the first phase
}