#ifndef MAIN_SCHEDULER_HEADER
#define MAIN_SCHEDULER_HEADER

#include <unordered_map>
#include <map>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <functional>

#include "../Structures/AccountInfo/MainAccountInfo.h"
#include "../Structures/Item/MainItem.h"
#include "../Structures/Item/MainEquippedItem.h"
#include "../Persistence/MainDatabaseManager.h"
#include <source_location>

// Used to accept overloaded member functions inside Database.h when calling MainScheduler::immediatePersist(...)
#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

#define ARROW(...) \
    noexcept(noexcept((__VA_ARGS__))) -> decltype(auto) requires(requires { (__VA_ARGS__); }) \
    { return (__VA_ARGS__); }

#define LIFT_MEMBER(...) \
    [](auto&& detail_obj, auto&&... detail_args) ARROW(FWD(detail_obj).__VA_ARGS__(FWD(detail_args)...))


namespace Main
{
    namespace Persistence
    {
        class MainScheduler
        {
        private:
            using AccountInfo = Main::Structures::AccountInfo;
            using Item = Main::Structures::Item;
            using EquippedItem = Main::Structures::EquippedItem;
            using BoughtItem = Main::Structures::BoughtItem;

            std::size_t m_wakeupFrequency{};
            std::thread m_schedulerThread{};
            bool m_stopRequested = false;

            Main::Persistence::PersistentDatabase& m_database;
            std::unordered_map<std::uint32_t, std::map<std::size_t, std::function<void()>>> m_databaseCallbacks{};
            std::unordered_map<std::uint32_t, std::map<std::size_t, std::function<void()>>> m_databaseCallbacksIncremental{};
            std::size_t m_incrementalDifferentiationKey = 0;
            std::mutex m_callbacksMutex;

            template <typename Func>
            decltype(auto) withRetry(Func&& f, int maxRetries = 5)
            {
                int attempt = 0;
                while (true)
                {
                    try
                    {
                        m_database.ensureConnections();
                        return std::forward<Func>(f)();
                    }
                    catch (const sql::SQLException& e)
                    {
                        if (++attempt >= maxRetries)
                        {
                            ::Utils::Logger::log("Max retries (" + std::to_string(maxRetries) + ") exceeded: " + std::string(e.what()), Utils::LogType::Error, "MainScheduler::withRetry");
                            break;
                        }

                        if (isConnectionError(e))
                        {
                            ::Utils::Logger::log("Connection error detected, will reconnect. Attempt " + std::to_string(attempt) + "/" + std::to_string(maxRetries),
                                Utils::LogType::Warning, "MainScheduler::withRetry");

                            std::this_thread::sleep_for(std::chrono::milliseconds(500 * attempt));
                        }
                        else
                        {
                            ::Utils::Logger::log("Non-connection SQL error: " + std::string(e.what()), Utils::LogType::Error, "MainScheduler::withRetry");
                        }
                    }
                }
            }

            bool isConnectionError(sql::SQLException e)
            {
                switch (e.getErrorCode()) {
                case 2002: // Can't connect to server
                case 2003: // Can't connect to MySQL server
                case 2005: // Unknown MySQL server host
                case 2006: // MySQL server has gone away
                case 2013: // Lost connection to MySQL server
                case 2055: // Lost connection to MySQL server
                    return true;
                default:
                    return false;
                }
            }

            void schedulerLoop();
            void persist();

        public:
            explicit MainScheduler(std::size_t wakeupFrequency, Main::Persistence::PersistentDatabase& database);

            ~MainScheduler();

            template<typename Function, typename... Args>
            void addCallback(const std::source_location& loc, std::uint32_t accountId, std::size_t differentiationKey, Function databaseMemberFunction, Args&&... args)
            {
                std::unique_lock<std::mutex> lock(m_callbacksMutex);
                m_databaseCallbacks[accountId][differentiationKey] =
                    [this, databaseMemberFunction, ...args = std::forward<Args>(args)]() mutable
                    {
                        return withRetry([&]() {
                            return std::invoke(databaseMemberFunction, m_database, std::forward<decltype(args)>(args)...);
                            });
                    };
            }

            template <typename Function, typename... Args>
            void addRepetitiveCallback(const std::source_location& loc, std::uint32_t accountId, Function databaseMemberFunction, Args&&... args)
            {
                std::unique_lock<std::mutex> lock(m_callbacksMutex);
                m_databaseCallbacksIncremental[accountId][++m_incrementalDifferentiationKey] =
                    [this, databaseMemberFunction, ...args = std::forward<Args>(args)]() mutable
                    {
                        withRetry([&]() {
                            std::invoke(databaseMemberFunction, m_database, std::forward<decltype(args)>(args)...);
                            });
                    };
            }


            template<typename F, typename... Args>
            decltype(auto) immediatePersist(const std::source_location& loc, F databaseMemberFunction, Args&&... args)
            {
                return withRetry([&]() -> decltype(auto) {
                    return std::invoke(databaseMemberFunction, m_database, std::forward<Args>(args)...);
                    });
            }

            void persistFor(std::uint32_t accountId);
        };
    }
}
#endif
