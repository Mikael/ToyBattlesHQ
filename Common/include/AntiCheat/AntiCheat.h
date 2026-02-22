#ifndef AC_AC_BASE_H
#define AC_AC_BASE_H

#include "AcEventQueue.h"
#include "Interfaces.h"
#include "Interfaces.h"
#include <iostream>
#include "Checks/PacketFloodingCheck.h"
#include "Checks/PacketReplicaCheck.h"

#include <mariadb/conncpp.hpp>
#include <mariadb/conncpp/Connection.hpp>
#include "../Utils/SetupParser.h"

namespace Ac
{
    class AntiCheatManager
    {
    private:
        sql::Connection* m_con;
        ACEventQueue m_eventQueue;
        std::vector<std::unique_ptr<IACCheckerBase>> m_checkers;
        std::thread m_thread;
        std::atomic<bool> m_isRunning{ false };

        AntiCheatManager(const AntiCheatManager&) = delete;
        AntiCheatManager& operator=(const AntiCheatManager&) = delete;

        void worker()
        {
            while (m_isRunning)
            {
                auto event = m_eventQueue.popEvent();
                if (!event) continue;

                for (auto& checker : m_checkers)
                {
                    if (auto flag = checker->processEventBase(*event))
                    {
                        handleFlag(*flag);
                    }
                }
            }
        }

        void handleFlag(const ACFlag& flag)
        {
            try
            {
                if (!m_con)
                    connectToDb();

                const std::string createTableQuery =
                    "CREATE TABLE IF NOT EXISTS CheatFlags ("
                    "ID INT AUTO_INCREMENT PRIMARY KEY, "
                    "Description TEXT NOT NULL"
                    ")";
                std::unique_ptr<sql::Statement> createStmt(m_con->createStatement());
                createStmt->execute(createTableQuery);

                const std::string insertQuery = "INSERT INTO CheatFlags (Description) VALUES (?)";
                std::unique_ptr<sql::PreparedStatement> insertStmt(m_con->prepareStatement(insertQuery));

                std::ostringstream descriptionStream;
                descriptionStream << "CheatType: " << flag.cheatType
                    << ", AccountID: " << flag.sessionId
                    << ", Details: " << flag.details;

                insertStmt->setString(1, descriptionStream.str());
                insertStmt->executeUpdate();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log(std::string("MariaDB exception while logging cheat flag: ") + e.what(),
                    Utils::LogType::Error, "AntiCheat::handleFlag");
            }
        }

        void connectToDb()
        {
            const auto& dbSetup = Common::Utils::SetupParser::getInstance().getDatabaseSetup();
            try
            {
                m_con = sql::mariadb::get_driver_instance()->connect("tcp://" + dbSetup.ip + ":" + std::to_string(dbSetup.port),
                    dbSetup.username, dbSetup.password);
                m_con->setSchema(dbSetup.databaseName);
                m_con->setAutoCommit(true);
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("Error connecting to MariaDB: " + std::string(e.what()),
                    Utils::LogType::Error, "AntiCheat::connectToDb");
            }
        }

    public:
        AntiCheatManager()
        {
            //registerChecker<PacketFloodChecker>();
            registerChecker<PacketReplicationChecker>();

            m_isRunning = true;
           // m_thread = std::thread(&AntiCheatManager::worker, this);
        }


        ~AntiCheatManager()
        {
            m_isRunning = false;
            m_eventQueue.shutdown();
            if (m_thread.joinable()) m_thread.join();
        }

        template<typename CheckerT>
        void registerChecker() 
        {
            m_checkers.push_back(std::make_unique<CheckerT>());
        }

        void submitEvent(std::unique_ptr<ACEvent> event) 
        {
            m_eventQueue.pushEvent(std::move(event));
        }
    };
}


#endif