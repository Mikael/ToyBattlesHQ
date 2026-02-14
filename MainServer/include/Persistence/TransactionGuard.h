
#include <mariadb/conncpp.hpp>
#include <mariadb/conncpp/Connection.hpp>
#include <Logger.h>

namespace Main
{
    namespace Persistence
    {
        class TransactionGuard
        {
            sql::Connection* conn;
            bool committed{false};

        public:
            TransactionGuard(sql::Connection* c) : conn(c)
            {
                conn->setAutoCommit(false);
            }

            void commit()
            {
                conn->commit();
                committed = true;
            }

            ~TransactionGuard()
            {
                try
                {
                    if (!committed)
                    {
                        conn->rollback();
                    }
                }
                catch (const sql::SQLException& e)
                {
                    Utils::Logger::log("Rollback failed: " + std::string(e.what()), Utils::LogType::Error, "TransactionGuard");
                }
            }
        };

    }
}