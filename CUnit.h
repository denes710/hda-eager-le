#ifndef RING_CUNIT_H
#define RING_CUNIT_H

#include "EDirection.h"
#include "CLogger.h"

#include <string>
#include <thread>
#include <mutex>

#include <grpcpp/grpcpp.h>
#include "unit.grpc.pb.h"

namespace RING
{
    class CUnit : public ring::Unit::Service
    {
        public:
            enum class EMessageType
            {
                ShareNodeId,
                LeaderElected
            };

            struct SNeighbour
            {
                unsigned m_nodeId;
                std::string m_addr;
            };
        
            using TCallback = std::function<void(EMessageType, unsigned)>;
            CUnit(unsigned p_nodeId,
                    const std::shared_ptr<CLogger>& p_logger,
                    const std::string& p_skeletonAddress,
                    const SNeighbour& p_neighbour,
                    EDirection p_direction,
                    const TCallback& p_callback)
                : m_nodeId(p_nodeId)
                , m_skeletonAddress(p_skeletonAddress)
                , m_neighbour(p_neighbour)
                , m_direction(p_direction)
                , m_callback(p_callback)
                , m_logger(p_logger)
            {}

            ~CUnit();

            // send messag to neighbour stub
            void SendNodeId(unsigned p_nodeId);

            // receives message from neighbour stub
            grpc::Status sendNodeId(grpc::ServerContext* p_context,
                    const ring::sendNodeIdRequest* p_request,
                    ring::sendNodeIdResponse* p_reply) override;

            // send messag to neighbour stub
            void SendLeaderElected(unsigned p_leaderId);

            // receives message from neighbour stub
            grpc::Status sendLeaderElected(grpc::ServerContext* p_context,
                    const ring::sendLeaderElectedRequest* p_request,
                    ring::sendLeaderElectedResponse* p_reply) override;

            void CreateSkeleton();

            // creating a single thread serving as skeleton
            void StartSkeleton();

            void StartStub();

        private:
            std::shared_ptr<CLogger> m_logger;

            const unsigned m_nodeId;
            const std::string m_skeletonAddress;
            const SNeighbour m_neighbour;

            bool m_initialized = false;
            const EDirection m_direction;
            const TCallback m_callback;

            std::shared_ptr<grpc::Channel> m_channel;
            std::unique_ptr<ring::Unit::Stub> m_stub;

            std::unique_ptr<grpc::Server> m_skeleton;
            std::unique_ptr<std::thread> m_thread;
    };
}

#endif