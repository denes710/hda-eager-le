#ifndef RING_CLOGGER_H
#define RING_CLOGGER_H

#include "EDirection.h"

#include <fstream>
#include <string>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

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

                const auto now = std::chrono::system_clock::now();
                const auto nowAsTimeT = std::chrono::system_clock::to_time_t(now);
                const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

                std::stringstream nowSs;
                nowSs << std::put_time(std::localtime(&nowAsTimeT), "%a %b %d %Y %T") <<
                    '.' << std::setfill('0') << std::setw(3) << nowMs.count();;

                m_outfile << "<" << p_nodeId << ", " << nowSs.str() << ", " << p_senderId << ", " <<
                    p_receiverId << ">" << std::endl; 
            }

        private:
            std::ofstream m_outfile;
            std::mutex m_mutex;
    };
}

#endif