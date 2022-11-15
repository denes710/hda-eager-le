#ifndef RING_CNODE_H
#define RING_CNODE_H

#include "CUnit.h"
#include "CLogger.h"

#include <mutex>
#include <string>
#include <vector>

namespace RING
{
    class CNode
    {
        public:
            CNode(unsigned p_nodeId,
                    const std::shared_ptr<CLogger>& p_logger,
                    const std::string& p_rightSkeletonAddress,
                    const std::string& p_leftSkeletonAddress,
                    const CUnit::SNeighbour& p_rightNeighbour,
                    const CUnit::SNeighbour& p_leftNeighbour);

            void StartSkeleton();

            void StartStub();

            void ResultCallback(const std::string& p_content, unsigned p_result);

            void StartHDAEagerLE();

            void ReceiveMessage(const std::string& p_content, unsigned p_result);

        private:
            const unsigned m_nodeId;
            CUnit m_rightUnit;
            CUnit m_leftUnit;
    };
}

#endif