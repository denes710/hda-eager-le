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
            struct SNeighbour
            {
                unsigned m_nodeId;
                std::string m_addr;
            };
        
            using TCallback = std::function<void(std::string, unsigned)>;
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

            // send messag to neighbour stub
            unsigned SendMessage(unsigned p_receiverId, unsigned p_senderId, const std::string& p_content);

            // receives message from neighbour stub
            grpc::Status sendMessage(grpc::ServerContext* p_context,
                    const ring::sendMessageRequest* p_request,
                    ring::sendMessageResponse* p_reply) override;

            void InjectMessage(unsigned p_receiverId, const std::string& p_content);

            void CreateSkeleton();

            // creating a single thread serving as skeleton
            void StartSkeleton();

            void StartStub();

        private:
            //FIXME make it readable
            const unsigned m_nodeId;
            std::mutex m_mutex;
            const SNeighbour m_neighbour;
            std::unique_ptr<ring::Unit::Stub> m_stub;
            std::shared_ptr<grpc::Channel> m_channel;
            const std::string m_skeletonAddress;
            std::unique_ptr<std::thread> m_thread;
            std::unique_ptr<grpc::Server> m_skeleton;
            bool m_initialized = false;
            const EDirection m_direction;
            const TCallback m_callback;
            std::shared_ptr<CLogger> m_logger;
    };
}

#endif