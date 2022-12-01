#include "CNode.h"
#include "loggers.h"

#include <string>
#include <thread>
#include <vector>
#include <sstream>
#include <numeric>

using namespace RING;

using namespace std;

struct SCommandLineArgs
{
    enum class ELoggerType
    {
        MessageLogger,
        MessageCountLogger
    };

    enum class EInputType
    {
        FromFile,
        FromCommandLine
    };

    SCommandLineArgs(int p_argc, char* p_argv[])
        : m_loggerType(GetLoggerType(p_argv[1]))
        , m_inputType(GetInputType(p_argv[2]))
    {
        switch (m_inputType)
        {
            case EInputType::FromFile:
                ReadTopologiesFromFile(p_argv[3]);
                break;
            case EInputType::FromCommandLine:
                vector<unsigned> ids;
                for (auto i = 3u; i < p_argc; ++i)
                    ids.push_back(stoul(p_argv[i]));
                m_topologies.push_back(ids);
                break;
        }
    }

    static ELoggerType GetLoggerType(const string& p_loggerType)
    {
        if (p_loggerType == "message_count")
            return ELoggerType::MessageCountLogger;
        else if (p_loggerType == "message_logger")
            return ELoggerType::MessageLogger;
        throw exception();
    }

    static EInputType GetInputType(const string& p_inputType)
    {
        if (p_inputType == "from_file")
            return EInputType::FromFile;
        else if (p_inputType == "command_line")
            return EInputType::FromCommandLine;
        throw exception();
    }

    static unsigned GetArgsNum(int p_argc)
    { return p_argc - s_numOfArgsBeforeInput; }

    static bool CheckArgsNum(int p_argc)
    {
        if (p_argc >= s_minArgs)
            return true;

        cout << "There are no enough args for running!" << endl;
        cout << "[message_logger/message_count] [from_file/command_line] [filename/topology]" << endl;
        return false;
    }

    void ReadTopologiesFromFile(const string& p_filename)
    {
        // FIXME error if something not good
        ifstream infile(p_filename);
        auto numberOfTests = 0u;
        string line;

        while (getline(infile, line))
        {
            stringstream ss(line);
            vector<unsigned> ids;
            string tmp;

            while(getline(ss, tmp, ','))
                ids.push_back(stoul(tmp));

            m_topologies.push_back(ids);
        }
    }

    static const unsigned s_numOfArgsBeforeInput = 2;
    static const unsigned s_minArgs = 4;

    const ELoggerType m_loggerType;
    const EInputType m_inputType;

    vector<vector<unsigned>> m_topologies;
};

int main(int argc, char* argv[])
{
    if (!SCommandLineArgs::CheckArgsNum(argc))
        return 0;

    SCommandLineArgs args(argc, argv);

	const auto ip = "127.0.0.1";
	const auto defaultPortNum = 30000u;

	const auto getAddress = [&](unsigned p_port)
	{
		stringstream ss;
		ss << ip << ":" << p_port;
		return ss.str();
	};

    const auto getLogger = [&](unsigned p_nodeCount) -> shared_ptr<CLoggerBase>
    {
        if (args.m_loggerType == SCommandLineArgs::ELoggerType::MessageCountLogger)
            return make_shared<CMessageCountLogger>(p_nodeCount);

        static const auto getLogFilename = []()
        {
            static auto num = 0u;

            stringstream ss;
            ss << "log-" << num;
            return ss.str();
        };

        return make_shared<CLogger>(getLogFilename());
    };

	for (const auto topology : args.m_topologies)
    {
        cout << accumulate(topology.begin(), topology.end(), string{},
            [](string& p_result, const unsigned& p_elem) -> decltype(auto)
                { return p_result += to_string(p_elem) + " "; }) << endl;

        const auto nodeCount = topology.size();

        auto logger = getLogger(nodeCount);

        vector<unique_ptr<CNode> > ring;

        for (auto i = 0u; i < nodeCount; ++i)
        {
            const auto rightOrderId = (i + 1) % nodeCount;
            const auto leftOrderId = (i + nodeCount - 1) % nodeCount;
            ring.emplace_back(make_unique<CNode>(topology[i],
                        logger,
                        getAddress(defaultPortNum + i), // right skeleton addr
                        getAddress(defaultPortNum + i + nodeCount), // left skeleton addr
                        CUnit::SNeighbour{topology[rightOrderId], getAddress(defaultPortNum + rightOrderId)}, // right neighbor
                        CUnit::SNeighbour{topology[leftOrderId], getAddress(defaultPortNum + leftOrderId + nodeCount)})); // left neighbor
        }

        // starting skeletons
        for (auto& node : ring)
            node->StartSkeleton();
        // starting stubs
        for (auto& node : ring)
            node->StartStub();
        // starting stubs
        for (auto& node : ring)
            node->RunHDAEagerLE();
    }

	return 0;
}
