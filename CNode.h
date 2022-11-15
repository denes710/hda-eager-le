#ifndef RING_CNODE_H
#define RING_CNODE_H

#include "CUnit.h"
#include "CLogger.h"

#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>
#include <queue>

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

            void RunHDAEagerLE();

            void ReceiveMessage(CUnit::EMessageType p_type, unsigned m_nodeId);

        private:
            enum class EState
            {
                Candidate,
                Defeated,
                Leader,
                Terminated
            };

            struct SMessage
            {
                CUnit::EMessageType m_type;
                unsigned m_nodeId;
            };

            void Run();
            void SendNodeIdMessage(unsigned p_nodeId);
            void SendLeaderElectedMessage(unsigned p_nodeId);

            CUnit& GetUnit(EDirection p_direction);

            const unsigned m_nodeId;
            CUnit m_rightUnit;
            CUnit m_leftUnit;

            EState m_state = EState::Candidate;
            EDirection m_direction = EDirection::Right;
            std::queue<SMessage> m_queue;
            std::mutex m_mutex;
            std::condition_variable m_conVariable;
            std::thread m_thread;
    };
}

#endif