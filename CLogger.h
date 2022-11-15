#ifndef RING_CLOGGER_H
#define RING_CLOGGER_H

#include "EDirection.h"

#include <fstream>
#include <string>
#include <mutex>
#include <chrono>
#include <ctime>

namespace RING
{
    class CLogger
    {
        public:                
            CLogger(const std::string& p_filename)
                : m_outfile(p_filename)
            {}

            void AddLog(unsigned p_nodeId, unsigned p_senderId, unsigned p_receiverId)
            {
                const std::lock_guard<std::mutex> guard(m_mutex);

                // FIXME maybe better option
                const auto now = std::chrono::system_clock::now();
                std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

                m_outfile << "<" << p_nodeId << ", " << std::ctime(&nowTime) << ", " << p_senderId << ", " <<
                    p_receiverId << ">" << std::endl; 
            }

        private:
            std::ofstream m_outfile;
            std::mutex m_mutex;
    };
}

#endif