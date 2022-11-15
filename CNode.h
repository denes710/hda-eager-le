#ifndef RING_CNODE_H
#define RING_CNODE_H

#include "CUnit.h"

#include <mutex>
#include <string>
#include <vector>

namespace RING
{
    class CNode
    {
        public:
            using TResult = std::vector<std::pair<std::string, unsigned>>;

            CNode(unsigned p_nodeId,
                    const std::string& p_rightSkeletonAddress,
                    const std::string& p_leftSkeletonAddress,
                    const std::string& p_rightNeighborAddress,
                    const std::string& p_leftNeighborAddress);

            void StartSkeleton();

            void StartStub();

            void ResultCallback(const std::string& p_content, unsigned p_result);

            void InjectMessage(EDirection p_direction, unsigned p_receiverId, const std::string& p_content);

            const TResult& GetResult() const
            { return m_result; }

            unsigned GetNodeId() const
            { return m_nodeId; }

        private:
            const unsigned m_nodeId;
            CUnit m_rightUnit;
            CUnit m_leftUnit;

            std::mutex m_resultMutex;
            TResult m_result;
    };
}

#endif