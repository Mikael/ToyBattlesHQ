#include <memory>
#include <string>

#include "../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "../../include/Structures/Item/MainItem.h"
#include "../../include/Structures/Item/MainBoughtItem.h"
#include "../../include/Structures/Item/MainEquippedItem.h"
#include "../../include/Structures/MainEventsList.h"
#include "../../include/Structures/PlayerLists/Friend.h"
#include "../../include/Structures/PlayerLists/BlockedPlayer.h"
#include "../../include/Structures/Mailbox.h"
#include "../../include/Structures/AccountInfo/MuteInfo.h"
#include "../../include/Persistence/TransactionGuard.h"
#include "../../include/Persistence/MainDatabaseManager.h"
#include "../../include/MainEnums.h"
#include "Utils/Constants.h"
#include <mariadb/conncpp.hpp>
#include <mariadb/conncpp/Driver.hpp>
#include <mariadb/conncpp/Connection.hpp>
#include "Utils/SetupParser.h"
#include <cstring> 

namespace Main
{
    namespace Persistence
    {
        PersistentDatabase::PersistentDatabase()
        {
            connectWithRetry();
            initialize();
        }

        void PersistentDatabase::initialize()
        {
            std::unique_ptr<sql::Statement> stmt(m_transactionalCon->createStatement());

            stmt->execute("CREATE TABLE IF NOT EXISTS EventMissionsInfo (StartDate DATETIME NOT NULL, EndDate DATETIME NOT NULL)");
            stmt->execute("CREATE TABLE IF NOT EXISTS TradeEvents (StartDate DATETIME NOT NULL, EndDate DATETIME NOT NULL)");
            stmt->execute("CREATE TABLE IF NOT EXISTS CapsuleEvents (StartDate DATETIME NOT NULL, EndDate DATETIME NOT NULL, NewMpPrice INT NOT NULL, NewRtPrice INT NOT NULL)");
            stmt->execute("CREATE TABLE IF NOT EXISTS ExpMpBonusEvents (StartDate DATETIME NOT NULL, EndDate DATETIME NOT NULL, ExpBonusPercent INT NOT NULL, MpBonusPercent INT NOT NULL)");
            stmt->execute(R"(CREATE TABLE IF NOT EXISTS GameLogs (ID INT AUTO_INCREMENT PRIMARY KEY,LogType VARCHAR(255) NOT NULL,Message TEXT NOT NULL,
                Severity VARCHAR(20) NOT NULL,CreatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP))");
            stmt->execute("ALTER TABLE Users ADD COLUMN IF NOT EXISTS VotekickDisabledUntil DATETIME NULL DEFAULT NULL");
        }


        void PersistentDatabase::connectWithRetry()
        {
            const auto& dbSetup = Common::Utils::SetupParser::getInstance().getDatabaseSetup();

            constexpr int maxRetries = 5;

            for (int attempt = 1; attempt <= maxRetries; ++attempt)
            {
                try
                {
                    ::Utils::Logger::log("Connecting to MariaDB (attempt " + std::to_string(attempt) + ")", Utils::LogType::Info, "PersistentDatabase");

                    m_con = sql::mariadb::get_driver_instance()->connect("tcp://" + dbSetup.ip + ":" + std::to_string(dbSetup.port), dbSetup.username, dbSetup.password);
                    m_con->setSchema(dbSetup.databaseName);
                    m_con->setAutoCommit(true);

                    m_transactionalCon = sql::mariadb::get_driver_instance()->connect("tcp://" + dbSetup.ip + ":" + std::to_string(dbSetup.port), dbSetup.username, dbSetup.password);
                    m_transactionalCon->setSchema(dbSetup.databaseName);
                    m_transactionalCon->setAutoCommit(false);

                    ::Utils::Logger::log("Connected to MariaDB successfully", Utils::LogType::Info, "PersistentDatabase");

                    return;
                }
                catch (const sql::SQLException& e)
                {
                    ::Utils::Logger::log("Connection failed: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase");

                    if (attempt == maxRetries)
                    {
                        ::Utils::Logger::log("Max connection retries reached!", Utils::LogType::Error, "PersistentDatabase");
                        throw;
                    }

                    std::this_thread::sleep_for(std::chrono::seconds(1 * attempt));
                }
            }
        }

        void PersistentDatabase::updatePlayerCurrencyByType(std::uint32_t accountID, std::uint32_t newAmount, Main::Enums::ItemCurrencyType currencyType)
        {
            try
            {
                std::string sql;
                if (currencyType == Main::Enums::ITEM_MP) sql = "UPDATE Users SET MicroPoints = ? WHERE AccountID = ?";
                else if (currencyType == Main::Enums::ITEM_RT) sql = "UPDATE Users SET RockTotens = ? WHERE AccountID = ?";
                else if (currencyType == Main::Enums::ITEM_COUPON) sql = "UPDATE Users SET Coupons = ? WHERE AccountID = ?";
                else if (currencyType == Main::Enums::ITEM_COIN) sql = "UPDATE Users SET Coins = ? WHERE AccountID = ?";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(sql));
                stmt->setUInt(1, newAmount);
                stmt->setUInt(2, accountID);

                stmt->executeUpdate();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::updatePlayerCurrencyByType");
            }
        }

        void PersistentDatabase::addPlayerAchievement(std::uint32_t accountID, std::uint32_t achievementIndex)
        {
            try
            {
                std::string sql = "INSERT INTO UserAchievements (AccountID, AchievementIndex) VALUES (?, ?)";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(sql));
                stmt->setUInt(1, accountID);
                stmt->setUInt(2, achievementIndex);

                stmt->executeUpdate();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::addPlayerAchievement");
            }
        }

        void PersistentDatabase::logMessage(std::uint32_t accountID, const std::string& message)
        {
            try
            {
                std::string truncatedMessage = message.substr(0, 300);
                std::string sql = "INSERT INTO ChatLogs (AccountID, Message) VALUES (?, ?)";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(sql));
                stmt->setUInt(1, accountID);
                stmt->setString(2, truncatedMessage);

                stmt->executeUpdate();
            }
            catch (const sql::SQLException& e)
            {
                return;
            }
        }

        std::vector<std::pair<std::uint32_t, std::uint32_t>> PersistentDatabase::getPlayerAchievements(std::uint32_t accountID)
        {
            std::vector<std::pair<std::uint32_t, std::uint32_t>> achievements;

            try
            {
                std::string sql = "SELECT AchievementIndex, AchievementType FROM UserAchievements WHERE AccountID = ?";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(sql));
                stmt->setUInt(1, accountID);

                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                while (res->next())
                {
                    std::uint32_t achievementIndex = res->getUInt("AchievementIndex");
                    std::uint32_t achievementType = res->getUInt("AchievementType");
                    achievements.emplace_back(achievementIndex, achievementType);
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::getPlayerAchievements");
            }

            return achievements;
        }

        std::unordered_map<std::uint32_t, std::uint32_t> PersistentDatabase::getPlayerMissions(std::uint32_t accountID)
        {
            std::unordered_map<std::uint32_t, std::uint32_t> missions;

            try
            {
                TransactionGuard guard(m_transactionalCon);

                std::string selectSql = "SELECT TotalMission1, TotalMission2, TotalMission3, TotalMission4, TotalMission5 "
                    "FROM EventMissions WHERE AccountID = ?";

                std::unique_ptr<sql::PreparedStatement> selectStmt(m_transactionalCon->prepareStatement(selectSql));
                selectStmt->setUInt(1, accountID);

                std::unique_ptr<sql::ResultSet> res(selectStmt->executeQuery());

                if (res->next())
                {
                    for (int i = 1; i <= Common::Constants::totalEventMissions; ++i)
                    {
                        std::uint32_t missionProgress = res->isNull("TotalMission" + std::to_string(i))
                            ? 0
                            : res->getUInt("TotalMission" + std::to_string(i));

                        if (missionProgress < Common::Constants::eventMissionTotal)
                        {
                            missions[i] = missionProgress;
                        }
                    }
                }
                else
                {
                    std::string insertSql = "INSERT INTO EventMissions (AccountID, TotalMission1, TotalMission2, TotalMission3, "
                        "TotalMission4, TotalMission5) VALUES (?, 0, 0, 0, 0, 0)";
                    std::unique_ptr<sql::PreparedStatement> insertStmt(m_transactionalCon->prepareStatement(insertSql));
                    insertStmt->setUInt(1, accountID);
                    insertStmt->executeUpdate();

                    for (int i = 1; i <= Common::Constants::totalEventMissions; ++i)
                    {
                        missions[i] = 0;
                    }
                }

                guard.commit();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::getPlayerMissions");
            }

            return missions;
        }

        std::optional<Main::Structures::EventMissionInfo> PersistentDatabase::getEventInfo(const std::string& tableName)
        {
            try
            {
                TransactionGuard guard(m_transactionalCon);

                const std::string query = "SELECT UNIX_TIMESTAMP(StartDate) AS StartTimestamp, "
                    "UNIX_TIMESTAMP(EndDate) AS EndTimestamp FROM " + tableName + " LIMIT 1";
                std::unique_ptr<sql::PreparedStatement> stmt(m_transactionalCon->prepareStatement(query));
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (res->next())
                {
                    Main::Structures::EventMissionInfo info;
                    info.startDate = res->getUInt("StartTimestamp");
                    info.endDate = res->getUInt("EndTimestamp");
                    guard.commit();
                    return info;
                }
                else
                {
                    const std::string insertQuery = "INSERT INTO " + tableName + " (StartDate, EndDate) "
                        "VALUES (FROM_UNIXTIME(0), FROM_UNIXTIME(0))";
                    std::unique_ptr<sql::PreparedStatement> insertStmt(m_transactionalCon->prepareStatement(insertQuery));
                    insertStmt->executeUpdate();
                    guard.commit();
                    return Main::Structures::EventMissionInfo{ 0, 0 };
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::getEventInfo (" + tableName + ")");
            }

            return std::nullopt;
        }

        std::optional<Main::Structures::CapsuleListDatabase> PersistentDatabase::getCapsuleEvent()
        {
            try
            {
                TransactionGuard guard(m_transactionalCon);

                const std::string query = R"(SELECT UNIX_TIMESTAMP(StartDate) AS StartTimestamp,UNIX_TIMESTAMP(EndDate) AS EndTimestamp,
                   NewMpPrice,NewRtPrice FROM CapsuleEvents LIMIT 1)";

                std::unique_ptr<sql::PreparedStatement> stmt(m_transactionalCon->prepareStatement(query));
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (res->next())
                {
                    Main::Structures::CapsuleListDatabase capsule;
                    capsule.saleEventStartDate = res->getUInt("StartTimestamp");
                    capsule.saleEventEndDate = res->getUInt("EndTimestamp");
                    capsule.newMpPrice = res->getUInt("NewMpPrice");
                    capsule.newRtPrice = res->getUInt("NewRtPrice");

                    guard.commit();
                    return capsule;
                }
                else
                {
                    const std::string insertQuery = R"(INSERT INTO CapsuleEvents (StartDate, EndDate, NewMpPrice, NewRtPrice)
                      VALUES (FROM_UNIXTIME(0), FROM_UNIXTIME(0), 0, 0))";

                    std::unique_ptr<sql::PreparedStatement> insertStmt(m_transactionalCon->prepareStatement(insertQuery));
                    insertStmt->executeUpdate();

                    guard.commit();
                    return Main::Structures::CapsuleListDatabase{};
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()),Utils::LogType::Error,"PersistentDatabase::getCapsuleEvent");
            }

            return std::nullopt;
        }

        bool PersistentDatabase::updateCapsuleEvent(const Main::Structures::CapsuleListDatabase& capsule)
        {
            try
            {
                TransactionGuard txn(m_transactionalCon);

                const std::string checkQuery = "SELECT COUNT(*) as Count FROM CapsuleEvents";
                std::unique_ptr<sql::PreparedStatement> checkStmt(m_transactionalCon->prepareStatement(checkQuery));
                std::unique_ptr<sql::ResultSet> checkRes(checkStmt->executeQuery());

                bool exists = false;
                if (checkRes->next())
                {
                    exists = checkRes->getUInt("Count") > 0;
                }

                if (exists)
                {
                    const std::string updateQuery = R"(UPDATE CapsuleEvents SET StartDate = FROM_UNIXTIME(?), EndDate = FROM_UNIXTIME(?),
                    NewMpPrice = ?, NewRtPrice = ?)";

                    std::unique_ptr<sql::PreparedStatement> updateStmt(m_transactionalCon->prepareStatement(updateQuery));
                    updateStmt->setUInt(1, capsule.saleEventStartDate);
                    updateStmt->setUInt(2, capsule.saleEventEndDate);
                    updateStmt->setUInt(3, capsule.newMpPrice);
                    updateStmt->setUInt(4, capsule.newRtPrice);
                    updateStmt->executeUpdate();
                }
                else
                {
                    const std::string insertQuery = R"(INSERT INTO CapsuleEvents (StartDate, EndDate, NewMpPrice, NewRtPrice) VALUES (FROM_UNIXTIME(?), FROM_UNIXTIME(?), ?, ?))";

                    std::unique_ptr<sql::PreparedStatement> insertStmt(m_transactionalCon->prepareStatement(insertQuery));
                    insertStmt->setUInt(1, capsule.saleEventStartDate);
                    insertStmt->setUInt(2, capsule.saleEventEndDate);
                    insertStmt->setUInt(3, capsule.newMpPrice);
                    insertStmt->setUInt(4, capsule.newRtPrice);
                    insertStmt->executeUpdate();
                }

                txn.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error,"PersistentDatabase::updateCapsuleEvent");
                return false; 
            }
        }

        bool PersistentDatabase::insertItemLogs(std::uint32_t accountId, const std::vector<Main::Structures::ItemLogInfo>& logs)
        {
            if (logs.empty()) return true;

            try
            {
                TransactionGuard txn(m_transactionalCon);

                const std::string query = R"(INSERT INTO ItemLogs (AccountID, Date, ItemNumber, ItemID, Action, ExpirationDate) VALUES (?, NOW(), ?, ?, ?, ?))";

                std::unique_ptr<sql::PreparedStatement> stmt(m_transactionalCon->prepareStatement(query));

                for (const auto& log : logs)
                {
                    stmt->setUInt64(1, accountId);
                    stmt->setUInt(2, log.itemNumber);
                    stmt->setUInt64(3, log.itemId);
                    stmt->setString(4, log.action);
                    stmt->setUInt(5, log.expirationDate);
                    stmt->executeUpdate();
                }

                txn.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()),Utils::LogType::Error,"PersistentDatabase::insertItemLogs");
                return false; 
            }
        }

        bool PersistentDatabase::insertItemLog(std::uint32_t accountId, const Main::Structures::ItemLogInfo& log)
        {
            try
            {
                const std::string query = R"(INSERT INTO ItemLogs (AccountID, Date, ItemNumber, ItemID, Action, ExpirationDate) VALUES (?, NOW(), ?, ?, ?, ?))";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(query));
                stmt->setUInt64(1, accountId);
                stmt->setUInt(2, log.itemNumber);
                stmt->setUInt64(3, log.itemId);
                stmt->setString(4, log.action);
                stmt->setUInt(5, log.expirationDate);
                stmt->executeUpdate();

                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::insertItemLog");
            }

            return false;
        }


        bool PersistentDatabase::logBoughtItems(std::uint32_t accountId, const std::vector<Main::Structures::BoughtItem>& boughtItems, bool isCouponItems)
        {
            std::vector<Main::Structures::ItemLogInfo> logInfos;
            for (const auto& currentBoughtItem : boughtItems)
            {
                logInfos.emplace_back(Main::Structures::ItemLogInfo{ currentBoughtItem.serialInfo.itemNumber, 
                    currentBoughtItem.itemId.itemId, currentBoughtItem.unknown, 
                    isCouponItems ? "The item was bought from the coupon shop" : "The item was bought from the normal shop" });
            }

            return insertItemLogs(accountId, logInfos);
        }

        bool PersistentDatabase::logBoxItems(std::uint32_t accountId, const std::vector<Main::Structures::BoxItem>& boxItem)
        {
            std::vector<Main::Structures::ItemLogInfo> logInfos;
            for (const auto& currentBoxItem : boxItem)
            {
                logInfos.emplace_back(Main::Structures::ItemLogInfo{ currentBoxItem.serialInfo.itemNumber,
                    currentBoxItem.itemId.itemId, currentBoxItem.expirationDate,
                    "The item was won through a box" });
            }

            return insertItemLogs(accountId, logInfos);
        }

        bool PersistentDatabase::logProlongedItems(std::uint32_t accountId, const std::vector<Main::Structures::BoughtItemToProlong>& boughtItems,
            const std::vector<std::uint64_t>& itemDurations)
        {
            const std::uint32_t timeNow = static_cast<std::uint32_t>(
                std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

            std::vector<Main::Structures::ItemLogInfo> logInfos;
            for (std::uint32_t i = 0; const auto & currentBoughtItem : boughtItems)
            {
                std::uint32_t actualItemDuration = itemDurations[i] <= 3 ? itemDurations[i] : itemDurations[i] + timeNow;
                logInfos.emplace_back(Main::Structures::ItemLogInfo{currentBoughtItem.serialInfo.itemNumber, 0, actualItemDuration, 
                    "The item was prolonged after it expired" });
                ++i;
            }

            return insertItemLogs(accountId, logInfos);
        }

        bool PersistentDatabase::updateEventInfo(const std::string& tableName, const Main::Structures::EventMissionInfo& info)
        {
            try
            {
                TransactionGuard txn(m_transactionalCon);

                const std::string ensureRowQuery = "SELECT COUNT(*) AS RowCount FROM " + tableName;
                std::unique_ptr<sql::PreparedStatement> checkStmt(m_transactionalCon->prepareStatement(ensureRowQuery));
                std::unique_ptr<sql::ResultSet> checkRes(checkStmt->executeQuery());

                if (checkRes->next() && checkRes->getUInt("RowCount") == 0)
                {
                    const std::string insertQuery = "INSERT INTO " + tableName + " (StartDate, EndDate) "
                        "VALUES (FROM_UNIXTIME(0), FROM_UNIXTIME(0))";

                    std::unique_ptr<sql::PreparedStatement> insertStmt(m_transactionalCon->prepareStatement(insertQuery));
                    insertStmt->executeUpdate();
                }

                const std::string updateQuery = "UPDATE " + tableName + " SET StartDate = FROM_UNIXTIME(?), "
                    "EndDate = FROM_UNIXTIME(?) LIMIT 1";

                std::unique_ptr<sql::PreparedStatement> updateStmt(m_transactionalCon->prepareStatement(updateQuery));
                updateStmt->setUInt(1, info.startDate);
                updateStmt->setUInt(2, info.endDate);

                bool success = updateStmt->executeUpdate() > 0;
                txn.commit(); 
                return success;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()),Utils::LogType::Error,"PersistentDatabase::updateEventInfo (" + tableName + ")");
            }

            return false; 
        }


        std::optional<Main::Structures::ExpMpBonusInfo> PersistentDatabase::getExpMpBonusInfo()
        {
            try
            {
                TransactionGuard txn(m_transactionalCon);

                const std::string query = R"(SELECT UNIX_TIMESTAMP(StartDate) AS StartTimestamp, UNIX_TIMESTAMP(EndDate) AS EndTimestamp,
                   ExpBonusPercent, MpBonusPercent FROM ExpMpBonusEvents LIMIT 1)";
                std::unique_ptr<sql::PreparedStatement> stmt(m_transactionalCon->prepareStatement(query));
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (res->next())
                {
                    Main::Structures::ExpMpBonusInfo info;
                    info.startDate = res->getUInt("StartTimestamp");
                    info.endDate = res->getUInt("EndTimestamp");
                    info.expBonusPercent = res->getUInt("ExpBonusPercent");
                    info.mpBonusPercent = res->getUInt("MpBonusPercent");
                    txn.commit();
                    return info;
                }
                else
                {
                    const std::string insertQuery = R"(INSERT INTO ExpMpBonusEvents (StartDate, EndDate, ExpBonusPercent, MpBonusPercent) 
                        VALUES (FROM_UNIXTIME(0), FROM_UNIXTIME(0), 0, 0))";
                    std::unique_ptr<sql::PreparedStatement> insertStmt(m_transactionalCon->prepareStatement(insertQuery));
                    insertStmt->executeUpdate();
                    txn.commit();
                    return Main::Structures::ExpMpBonusInfo{ 0, 0, 0, 0 };
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()),Utils::LogType::Error,"PersistentDatabase::getExpMpBonusInfo");
            }

            return std::nullopt;
        }

        bool PersistentDatabase::updateExpMpBonusInfo(const Main::Structures::ExpMpBonusInfo& info)
        {
            try
            {
                TransactionGuard txn(m_transactionalCon);

                const std::string checkQuery = R"(SELECT COUNT(*) AS RowCount FROM ExpMpBonusEvents)";
                std::unique_ptr<sql::PreparedStatement> checkStmt(m_transactionalCon->prepareStatement(checkQuery));
                std::unique_ptr<sql::ResultSet> checkRes(checkStmt->executeQuery());

                if (checkRes->next() && checkRes->getUInt("RowCount") == 0)
                {
                    const std::string insertQuery = R"(INSERT INTO ExpMpBonusEvents (StartDate, EndDate, ExpBonusPercent, MpBonusPercent)
                        VALUES (FROM_UNIXTIME(0), FROM_UNIXTIME(0), 0, 0))";

                    std::unique_ptr<sql::PreparedStatement> insertStmt(m_transactionalCon->prepareStatement(insertQuery));
                    insertStmt->executeUpdate();
                }

                const std::string updateQuery = R"(UPDATE ExpMpBonusEvents SET StartDate = FROM_UNIXTIME(?), EndDate = FROM_UNIXTIME(?), ExpBonusPercent = ?,
                    MpBonusPercent = ? LIMIT 1)";

                std::unique_ptr<sql::PreparedStatement> updateStmt(m_transactionalCon->prepareStatement(updateQuery));
                updateStmt->setUInt(1, info.startDate);
                updateStmt->setUInt(2, info.endDate);
                updateStmt->setUInt(3, info.expBonusPercent);
                updateStmt->setUInt(4, info.mpBonusPercent);
                updateStmt->executeUpdate();

                const std::string updateModes = R"(UPDATE EventModes SET EndDate = FROM_UNIXTIME(?))";
                {
                    std::unique_ptr<sql::PreparedStatement> updateStmtModes(m_transactionalCon->prepareStatement(updateModes));
                    updateStmtModes->setUInt(1, info.endDate);
                    updateStmtModes->executeUpdate();
                }

                const std::string updateMaps = R"(UPDATE EventMaps SET EndDate = FROM_UNIXTIME(?))";
                {
                    std::unique_ptr<sql::PreparedStatement> updateStmtMaps(m_transactionalCon->prepareStatement(updateMaps));
                    updateStmtMaps->setUInt(1, info.endDate);
                    updateStmtMaps->executeUpdate();
                }

                txn.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()),Utils::LogType::Error,"PersistentDatabase::updateExpMpBonusInfo");
            }

            return false;
        }

        bool PersistentDatabase::updatePlayerMissionProgress(std::uint32_t accountID, std::uint32_t missionID, std::uint32_t newProgress)
        {
            if (missionID == 0 || missionID > Common::Constants::totalEventMissions)
            {
                ::Utils::Logger::log("Invalid mission ID: " + std::to_string(missionID),Utils::LogType::Warning, "PersistentDatabase::updatePlayerMissionProgress");
                return false;
            }

            try
            {
                TransactionGuard txn(m_transactionalCon);

                std::string checkSql = "SELECT COUNT(*) FROM EventMissions WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> checkStmt(m_transactionalCon->prepareStatement(checkSql));
                checkStmt->setUInt(1, accountID);
                std::unique_ptr<sql::ResultSet> checkRes(checkStmt->executeQuery());

                bool exists = false;
                if (checkRes->next())
                {
                    exists = checkRes->getUInt(1) > 0;
                }

                if (exists)
                {
                    std::string column = "TotalMission" + std::to_string(missionID);
                    std::string updateSql = "UPDATE EventMissions SET " + column + " = ? WHERE AccountID = ?";
                    std::unique_ptr<sql::PreparedStatement> updateStmt(m_transactionalCon->prepareStatement(updateSql));
                    updateStmt->setUInt(1, newProgress);
                    updateStmt->setUInt(2, accountID);
                    updateStmt->executeUpdate();
                }
                else
                {
                    std::string insertSql = "INSERT INTO EventMissions (AccountID, TotalMission1, TotalMission2, TotalMission3, TotalMission4, TotalMission5) "
                        "VALUES (?, ?, ?, ?, ?, ?)";
                    std::unique_ptr<sql::PreparedStatement> insertStmt(m_transactionalCon->prepareStatement(insertSql));
                    insertStmt->setUInt(1, accountID);
                    for (int i = 1; i <= Common::Constants::totalEventMissions; ++i)
                    {
                        insertStmt->setUInt(i + 1, (i == static_cast<int>(missionID)) ? newProgress : 0);
                    }
                    insertStmt->executeUpdate();
                }

                txn.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()),Utils::LogType::Error, "PersistentDatabase::updatePlayerMissionProgress");
                return false;
            }
        }

        bool PersistentDatabase::savePlayerMissions(std::uint32_t accountID, const std::unordered_map<std::uint32_t, std::uint32_t>& activeMissions)
        {
            if (activeMissions.size() > Common::Constants::totalEventMissions)
            {
                ::Utils::Logger::log("Error: too many active missions, expected <= 5", Utils::LogType::Error, "PersistentDatabase::savePlayerMissions");
                return false;
            }

            try
            {
                TransactionGuard txn(m_transactionalCon);

                std::string checkSql = "SELECT COUNT(*) FROM EventMissions WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> checkStmt(m_transactionalCon->prepareStatement(checkSql));
                checkStmt->setUInt(1, accountID);
                std::unique_ptr<sql::ResultSet> res(checkStmt->executeQuery());

                bool exists = false;
                if (res->next())
                {
                    exists = res->getUInt(1) > 0;
                }

                std::array<std::uint32_t, Common::Constants::totalEventMissions> totals{};
                if (exists)
                {
                    std::string fetchSql = "SELECT TotalMission1, TotalMission2, TotalMission3, TotalMission4, TotalMission5 FROM EventMissions WHERE AccountID = ?";
                    std::unique_ptr<sql::PreparedStatement> fetchStmt(m_transactionalCon->prepareStatement(fetchSql));
                    fetchStmt->setUInt(1, accountID);
                    std::unique_ptr<sql::ResultSet> fetchRes(fetchStmt->executeQuery());
                    if (fetchRes->next())
                    {
                        for (int i = 0; i < totals.size(); ++i)
                        {
                            totals[i] = fetchRes->getUInt(i + 1);
                        }
                    }
                }
                else
                {
                    totals.fill(0);
                }

                for (const auto& [missionID, total] : activeMissions)
                {
                    if (missionID >= 1 && missionID <= Common::Constants::totalEventMissions)
                    {
                        totals[missionID - 1] = total;
                    }
                }

                if (exists)
                {
                    std::string updateSql = "UPDATE EventMissions SET TotalMission1 = ?, TotalMission2 = ?, TotalMission3 = ?, "
                        "TotalMission4 = ?, TotalMission5 = ? WHERE AccountID = ?";
                    std::unique_ptr<sql::PreparedStatement> stmt(m_transactionalCon->prepareStatement(updateSql));

                    for (int i = 0; i < Common::Constants::totalEventMissions; ++i)
                    {
                        stmt->setUInt(i + 1, totals[i]);
                    }
                    stmt->setUInt(6, accountID);
                    stmt->executeUpdate();
                }
                else
                {
                    std::string insertSql = "INSERT INTO EventMissions (AccountID, TotalMission1, TotalMission2, TotalMission3, "
                        "TotalMission4, TotalMission5) VALUES (?, ?, ?, ?, ?, ?)";
                    std::unique_ptr<sql::PreparedStatement> stmt(m_transactionalCon->prepareStatement(insertSql));

                    stmt->setUInt(1, accountID);
                    for (int i = 0; i < Common::Constants::totalEventMissions; ++i)
                    {
                        stmt->setUInt(i + 2, totals[i]);
                    }
                    stmt->executeUpdate();
                }

                txn.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::savePlayerMissions");
                return false;
            }
        }

        void PersistentDatabase::updateLatestSelectedCharacter(std::uint32_t accountID, std::uint16_t characterId)
        {
            try
            {
                const std::string updateCharacterQuery = "UPDATE Users SET LastCharacterUsed = ? WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(updateCharacterQuery));
                stmt->setUInt(1, characterId);
                stmt->setUInt(2, accountID);
                stmt->executeUpdate();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::updateLatestSelectedCharacter");
            }
        }

        bool PersistentDatabase::logGameEvent(const std::string& logType, const std::string& message, const std::string& severity)
        {
            try
            {
                std::string insertQuery = "INSERT INTO GameLogs (LogType, Message, Severity) VALUES (?, ?, ?)";
                std::unique_ptr<sql::PreparedStatement> stmtInsert(m_con->prepareStatement(insertQuery));

                stmtInsert->setString(1, logType);
                stmtInsert->setString(2, message);
                stmtInsert->setString(3, severity);

                return stmtInsert->executeUpdate() > 0;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), ::Utils::LogType::Error, "PersistentDatabase::logGameEvent");
                return false;
            }
        }

        std::optional<Main::Structures::AccountInfo> PersistentDatabase::getPlayerInfo(std::uint32_t playerID)
        {
            Main::Structures::AccountInfo playerInfoStructure{};
            try
            {
                std::string queryStr = "SELECT Users.*, Clans.Clanname as Clan_Clanname, Clans.ClanFrontIcon as Clan_FrontIcon, Clans.ClanBackIcon as Clan_BackIcon "
                    "FROM Users LEFT JOIN Clans ON Users.ClanID = Clans.ClanId WHERE AccountID = ?";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));
                stmt->setUInt(1, playerID);
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (res->next())
                {
                    playerInfoStructure.accountID = playerID;
                    std::memcpy(playerInfoStructure.nickname, res->getString("Nickname"), Common::Constants::maxNicknameSize);
                    std::memcpy(playerInfoStructure.clanName, res->getString("Clan_Clanname"), Common::Constants::maxNicknameSize);
                    playerInfoStructure.accountKey = static_cast<std::uint32_t>(res->getUInt("AccountKey"));
                    playerInfoStructure.totalKills = static_cast<std::uint32_t>(res->getInt("Kills"));
                    playerInfoStructure.deaths = static_cast<std::uint32_t>(res->getInt("Deaths"));
                    playerInfoStructure.assists = static_cast<std::uint32_t>(res->getInt("Assists"));
                    playerInfoStructure.wins = static_cast<std::uint32_t>(res->getInt("Wins"));
                    playerInfoStructure.losses = static_cast<std::uint32_t>(res->getInt("Loses"));
                    playerInfoStructure.draws = static_cast<std::uint32_t>(res->getInt("Draws"));
                    playerInfoStructure.meleeKills = static_cast<std::uint32_t>(res->getInt("MeleeKills"));
                    playerInfoStructure.rifleKills = static_cast<std::uint32_t>(res->getInt("RifleKills"));
                    playerInfoStructure.shotgunKills = static_cast<std::uint32_t>(res->getInt("ShotgunKills"));
                    playerInfoStructure.sniperKills = static_cast<std::uint32_t>(res->getInt("SniperKills"));
                    playerInfoStructure.microgunKills = static_cast<std::uint32_t>(res->getInt("GatlingKills"));
                    playerInfoStructure.bazookaKills = static_cast<std::uint32_t>(res->getInt("BazookaKills"));
                    playerInfoStructure.grenadeKills = static_cast<std::uint32_t>(res->getInt("GrenadeKills"));
                    playerInfoStructure.killstreak = static_cast<std::uint64_t>(res->getInt("HighestKillstreak"));
                    playerInfoStructure.headshots = static_cast<std::uint64_t>(res->getInt("Headshots"));
                    playerInfoStructure.playtime = static_cast<std::uint32_t>(res->getInt("Playtime"));
                    playerInfoStructure.clanId = static_cast<std::uint32_t>(res->getInt("ClanID"));
                    playerInfoStructure.latestSelectedCharacter = static_cast<std::uint64_t>(res->getInt("LastCharacterUsed"));
                    playerInfoStructure.playerLevel = static_cast<std::uint64_t>(res->getInt("Level")) + 1;
                    playerInfoStructure.battery = static_cast<std::uint64_t>(res->getInt("Battery"));
                    playerInfoStructure.luckyPoints = static_cast<std::uint64_t>(res->getInt("LuckyPoints"));
                    playerInfoStructure.coins = static_cast<std::uint64_t>(res->getInt("Coins"));
                    playerInfoStructure.playerGrade = static_cast<std::uint64_t>(res->getInt("Grade"));
                    playerInfoStructure.experience = static_cast<std::uint32_t>(res->getInt("Experience"));
                    playerInfoStructure.microPoints = static_cast<std::uint64_t>(res->getInt64("MicroPoints"));
                    playerInfoStructure.rockTotens = static_cast<std::uint64_t>(res->getInt64("RockTotens"));
                    playerInfoStructure.inventorySpace = static_cast<std::uint32_t>(res->getInt("MaxInventory"));
                    playerInfoStructure.isTutorialDone = static_cast<std::uint32_t>(res->getInt("HasFinishedTutorial"));
                    playerInfoStructure.maxBattery = static_cast<std::uint32_t>(res->getInt("MaxBattery"));
                    playerInfoStructure.singleWaveAttempts = static_cast<std::uint32_t>(res->getInt("SingleWaveAttempts"));
                    playerInfoStructure.highestSinglewaveStage = static_cast<std::uint32_t>(res->getInt("SingleWaveAttempts"));
                    playerInfoStructure.highestSingleWaveScore = static_cast<std::uint32_t>(res->getInt("HighestSinglewaveScore"));
                    playerInfoStructure.vipExperience = static_cast<std::uint32_t>(res->getInt("VipExperience"));
                    playerInfoStructure.clanContribution = static_cast<std::uint64_t>(res->getInt64("ClanContribution"));
                    playerInfoStructure.clanLogoFrontId = static_cast<std::uint64_t>(res->getInt("Clan_FrontIcon"));
                    playerInfoStructure.clanLogoBackId = static_cast<std::uint64_t>(res->getInt("Clan_BackIcon"));
                    playerInfoStructure.clanWins = static_cast<std::uint64_t>(res->getInt("ClanWins"));
                    playerInfoStructure.clanLosses = static_cast<std::uint64_t>(res->getInt("ClanLoses"));
                    playerInfoStructure.clanDraws = static_cast<std::uint64_t>(res->getInt("ClanDraws"));
                    playerInfoStructure.clanKills = static_cast<std::uint32_t>(res->getInt("ClanKills"));
                    playerInfoStructure.clanDeaths = static_cast<std::uint32_t>(res->getInt("ClanDeaths"));
                    playerInfoStructure.clanAssists = static_cast<std::uint32_t>(res->getInt("ClanAssists"));
                    playerInfoStructure.infected = static_cast<std::uint32_t>(res->getInt("InfectedKills"));
                    playerInfoStructure.zombieKills = res->getInt("ZombieKills");

                    std::vector<Common::Enums::Characters> boughtCharacterTypes{};
                    boughtCharacterTypes.reserve(static_cast<std::size_t>(Common::Enums::Characters::Sophitia));
                    for (std::size_t currentCharacter = 0; currentCharacter <= static_cast<std::size_t>(Common::Enums::Characters::Sophitia); ++currentCharacter)
                    {
                        boughtCharacterTypes.push_back(static_cast<Common::Enums::Characters>(currentCharacter));
                    }
                    playerInfoStructure.setBoughtCharacters(boughtCharacterTypes);

                    // achievements
                    for (const auto& currentAchievement : getPlayerAchievements(playerID))
                    {
                        playerInfoStructure.achievements.setAchievementTier1(currentAchievement.first);
                    }
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::getPlayerInfo");
                return std::nullopt;
            }

            return playerInfoStructure;
        }

        std::optional<std::string> PersistentDatabase::getLastLogged(std::uint32_t accountId)
        {
            try
            {
                std::string queryStr = "SELECT LastLogged FROM Users WHERE AccountID = ?";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));
                stmt->setUInt(1, accountId);

                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (res->next())
                {
                    return res->getString("LastLogged").c_str();
                }
                else
                {
                    return std::nullopt;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()),Utils::LogType::Error,"PersistentDatabase::getLastLoggedByAccountId");
                return std::nullopt;
            }
        }

        bool PersistentDatabase::resetAccountKey(std::uint32_t accountId)
        {
            try
            {
                std::string queryStr = "UPDATE Users SET AccountKey = 0 WHERE AccountID = ?";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));
                stmt->setUInt(1, accountId);

                return stmt->executeUpdate() > 0;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::resetAccountKey");
                return false;
            }
        }

        std::optional<std::pair<Main::Structures::AccountInfo, std::string>> PersistentDatabase::getPlayerInfoByNickname(const std::string& nickname)
        {
            Main::Structures::AccountInfo playerInfoStructure{};
            try
            {
                std::string queryStr = "SELECT AccountID, Grade, Level, MicroPoints, RockTotens, LastLogged "
                    "FROM Users WHERE Nickname = ?";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));
                stmt->setString(1, nickname);
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (res->next())
                {
                    playerInfoStructure.accountID = static_cast<std::uint32_t>(res->getUInt("AccountID"));
                    playerInfoStructure.playerGrade = static_cast<std::uint32_t>(res->getInt("Grade"));
                    playerInfoStructure.playerLevel = static_cast<std::uint64_t>(res->getInt("Level")) + 1;
                    playerInfoStructure.microPoints = static_cast<std::uint64_t>(res->getInt64("MicroPoints"));
                    playerInfoStructure.rockTotens = static_cast<std::uint64_t>(res->getInt64("RockTotens"));

                    const char* lastLoggedCStr = res->getString("LastLogged").c_str();
                    std::string lastLogged = std::string(lastLoggedCStr);
                    return std::make_optional<std::pair<Main::Structures::AccountInfo, std::string>>(
                        playerInfoStructure, lastLogged);
                }
                else
                {
                    return std::nullopt;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::getPlayerInfoByNickname");
                return std::nullopt;
            }
        }

        Main::Structures::MuteInfo PersistentDatabase::isMuted(std::uint32_t playerID)
        {
            try
            {
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement("SELECT * FROM Users WHERE AccountID = ?"));
                stmt->setUInt(1, playerID);
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (res->next())
                {
                    const std::string mutedUntil = res->getString("MutedUntil").c_str();
                    const std::string muteReason = res->getString("MuteReason").c_str();
                    const std::string mutedBy = res->getString("MutedBy").c_str();

                    auto const time = std::chrono::utc_clock::now();
                    std::string currentTimeStr = std::format("{:%Y-%m-%d %X}", time); 

                    bool isMuted = mutedUntil > currentTimeStr;

                    return Main::Structures::MuteInfo{isMuted, muteReason, mutedBy, mutedUntil };
                }
                else
                {
                    ::Utils::Logger::log("No muteinfo found for AccountID: " + std::to_string(playerID), Utils::LogType::Warning, "PersistentDatabase::isMuted");
                    return {};  
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::isMuted");
                return {}; 
            }
        }

        std::optional<Main::Structures::MuteInfo> PersistentDatabase::getMuteInfoByNickname(const std::string& nickname)
        {
            try
            {
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement("SELECT * FROM Users WHERE Nickname = ?"));
                stmt->setString(1, nickname);
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (res->next())
                {
                    const std::string mutedUntil = res->getString("MutedUntil").c_str();
                    const std::string muteReason = res->getString("MuteReason").c_str();
                    const std::string mutedBy = res->getString("MutedBy").c_str();

                    auto const time = std::chrono::utc_clock::now();
                    std::string currentTimeStr = std::format("{:%Y-%m-%d %X}", time);

                    bool isMuted = mutedUntil > currentTimeStr;

                    return Main::Structures::MuteInfo{ isMuted, muteReason, mutedBy, mutedUntil };
                }
                else
                {
                    ::Utils::Logger::log("No muteinfo found for nickname: " + nickname, Utils::LogType::Warning, "PersistentDatabase::getMuteInfoByNickname");
                    return std::nullopt;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::getMuteInfoByNickname");
                return std::nullopt;
            }
        }

        std::optional<Main::Structures::BanInfo> PersistentDatabase::getBanInfoByNickname(const std::string& nickname)
        {
            try
            {
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement("SELECT * FROM Users WHERE Nickname = ?"));
                stmt->setString(1, nickname);
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (res->next())
                {
                    const std::string suspendedUntil = res->getString("SuspendedUntil").c_str();
                    const std::string suspensionReason = res->getString("SuspensionReason").c_str();

                    auto const time = std::chrono::utc_clock::now();
                    std::string currentTimeStr = std::format("{:%Y-%m-%d %X}", time);  

                    bool isBanned = suspendedUntil > currentTimeStr;

                    return Main::Structures::BanInfo{ isBanned, suspensionReason, suspendedUntil };
                }
                else
                {
                    ::Utils::Logger::log("No baninfo found for targetPlayer " + nickname, Utils::LogType::Warning, "PersistentDatabase::getBanInfoByNickname");
                    return std::nullopt;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::getBanInfoByNickname");
                return std::nullopt;
            }
        }

        std::optional<std::string> PersistentDatabase::getRoomCreationDisabledUntil(const std::string& nickname)
        {
            try
            {
                std::string selectQueryStr = "SELECT RoomCreationDisabledUntil FROM Users WHERE Nickname = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(selectQueryStr));
                stmt->setString(1, nickname);  

                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
                if (res->next())
                {
                    const std::string disabledUntil = res->getString("RoomCreationDisabledUntil").c_str();
                    return disabledUntil;
                }

                return std::nullopt;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::getRoomCreationDisabledUntil");
                return std::nullopt;
            }
        }

        bool PersistentDatabase::isRoomCreationDisabled(std::uint32_t playerID)
        {
            try
            {
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement("SELECT RoomCreationDisabledUntil FROM Users WHERE AccountID = ?"));
                stmt->setUInt(1, playerID);
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (res->next())
                {
                    const std::string disabledUntil = res->getString("RoomCreationDisabledUntil").c_str();
                    auto const time = std::chrono::utc_clock::now();
                    return disabledUntil > std::format("{:%Y-%m-%d %X}", time);
                }

                ::Utils::Logger::log("No room creation ban info found for AccountID: " + std::to_string(playerID), Utils::LogType::Warning, "PersistentDatabase::isRoomCreationDisabled");
                return false;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::isRoomCreationDisabled");
                return false;
            }
        }

        std::optional<std::string> PersistentDatabase::getVotekickDisabledUntil(const std::string& nickname)
        {
            try
            {
                std::string selectQueryStr = "SELECT VotekickDisabledUntil FROM Users WHERE Nickname = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(selectQueryStr));
                stmt->setString(1, nickname);

                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
                if (res->next())
                {
                    const std::string disabledUntil = res->getString("VotekickDisabledUntil").c_str();
                    return disabledUntil;
                }
                return std::nullopt;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::getVotekickDisabledUntil");
                return std::nullopt;
            }
        }

        bool PersistentDatabase::isVotekickDisabled(std::uint32_t playerID)
        {
            try
            {
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement("SELECT VotekickDisabledUntil FROM Users WHERE AccountID = ?"));
                stmt->setUInt(1, playerID);

                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
                if (res->next())
                {
                    const std::string disabledUntil = res->getString("VotekickDisabledUntil").c_str();
                    auto const time = std::chrono::utc_clock::now();
                    return disabledUntil > std::format("{:%Y-%m-%d %X}", time);
                }
                return false;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::isVotekickDisabled");
                return false;
            }
        }

        bool PersistentDatabase::unbanPlayer(const std::string& nickname)
        {
            try
            {
                std::string queryStr = "UPDATE Users SET SuspendedUntil = 0, SuspensionReason = '' WHERE Nickname = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));
                stmt->setString(1, nickname);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("No rows changed for player: " + nickname, Utils::LogType::Warning, "PersistentDatabase::unbanPlayer");
                    return false;
                }
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::unbanPlayer");
                return false;
            }
        }

        bool PersistentDatabase::addPlayer(const std::string& username, const std::string& password, const std::string& nickname)
        {
            try
            {
                std::string queryStr = "INSERT INTO Users (Username, Password, Nickname) VALUES (?, ?, ?)";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));
                stmt->setString(1, username);
                stmt->setString(2, password);
                stmt->setString(3, nickname);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Error in query execution: " + queryStr, Utils::LogType::Warning, "PersistentDatabase::addPlayer");
                    return false;
                }
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::addPlayer");
                return false;
            }
        }

        auto PersistentDatabase::getPlayerItems(std::uint32_t playerID)
            -> std::pair<std::vector<Main::Structures::Item>, std::unordered_map<std::uint16_t, std::vector<Main::Structures::EquippedItem>>>
        {
            std::vector<Main::Structures::Item> nonEquippedItems;
            std::unordered_map<std::uint16_t, std::vector<Main::Structures::EquippedItem>> equippedItemsPerCharacter;
            std::vector<std::pair<std::uint64_t, std::uint64_t>> itemNumbersToUpdate;
            std::uint64_t itemNum = 0;

            try
            {
                TransactionGuard tx(m_transactionalCon);

                const std::string queryStr = "SELECT * FROM UserItems WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_transactionalCon->prepareStatement(queryStr));
                stmt->setUInt(1, playerID);
                std::unique_ptr<sql::ResultSet> resultSet(stmt->executeQuery());

                nonEquippedItems.reserve(1000);
                equippedItemsPerCharacter.reserve(500);

                while (resultSet->next())
                {
                    Main::Structures::Item item{ static_cast<std::uint32_t>(resultSet->getInt("ItemID")) };
                    auto rowId = static_cast<std::uint64_t>(resultSet->getUInt64("rowid"));
                    item.serialInfo.itemOrigin = static_cast<std::uint64_t>(resultSet->getInt64("ItemOrigin"));
                    item.serialInfo.m_serverId = static_cast<std::uint64_t>(resultSet->getInt64("acquisitionServerId"));
                    item.serialInfo.itemCreationDate = static_cast<__time32_t>(resultSet->getInt64("creationDate"));

                    item.serialInfo.itemNumber = ++itemNum;
                    itemNumbersToUpdate.emplace_back(std::pair{ rowId, static_cast<unsigned long>(item.serialInfo.itemNumber) });

                    const std::uint64_t itemDuration_s = static_cast<std::uint64_t>(resultSet->getInt64("ItemDuration"));
                    item.expirationDate = (itemDuration_s <= 2)
                        ? static_cast<__time32_t>(itemDuration_s)
                        : static_cast<__time32_t>(item.serialInfo.itemCreationDate + itemDuration_s);

                    item.durability = static_cast<std::uint16_t>(resultSet->getInt("durability"));
                    item.energy = static_cast<std::uint16_t>(resultSet->getInt("energy"));
                    item.isSealed = static_cast<std::uint32_t>(resultSet->getInt("isSealed"));
                    item.sealLevel = static_cast<std::uint32_t>(resultSet->getInt("sealLevel"));
                    item.experienceEnhancement = static_cast<std::uint32_t>(resultSet->getInt("expEnhancement"));
                    item.mpEnhancement = static_cast<std::uint32_t>(resultSet->getInt("mpEnhancement"));
                    item.unknown = static_cast<std::uint32_t>(resultSet->getInt("IsCoupon"));
                    item.itemId.stock = static_cast<std::uint32_t>(resultSet->getInt("Stocks"));

                    if (resultSet->getInt("IsEquipped") == 1)
                    {
                        Main::Structures::EquippedItem equippedItem{ item };
                        const auto characterId = static_cast<std::uint16_t>(resultSet->getInt("CharacterID"));

                        if (equippedItem.type >= 0 && equippedItem.type <= 17)
                        {
                            auto& equippedList = equippedItemsPerCharacter[characterId];
                            auto duplicateIt = std::find_if(equippedList.begin(), equippedList.end(),
                                [&](const Main::Structures::EquippedItem& existing) {
                                    return existing.type == equippedItem.type;
                                });

                            if (duplicateIt == equippedList.end())
                            {
                                equippedList.push_back(equippedItem);
                            }
                            else
                            {
                                Utils::Logger::log("Logic warning: Player with AID " + std::to_string(playerID) +
                                    " has multiple equipped items of type " + std::to_string(equippedItem.type) +
                                    " for CharacterID " + std::to_string(characterId) + ". Auto-fixing by unequipping one.",
                                    Utils::LogType::Warning, "PersistentDatabase::getPlayerItems");

                                nonEquippedItems.push_back(std::move(item));

                                const std::string updateQuery = "UPDATE UserItems SET IsEquipped = 0 WHERE rowid = ?";
                                std::unique_ptr<sql::PreparedStatement> updateStmt(m_transactionalCon->prepareStatement(updateQuery));
                                updateStmt->setUInt64(1, rowId);
                                updateStmt->executeUpdate();
                            }
                        }
                        else
                        {
                            Utils::Logger::log("Logic error: Player with AID " + std::to_string(playerID) +
                                " got an equipped item that is not allowed (< 0 or > 17)!", Utils::LogType::Error, "PersistentDatabase::getPlayerItems");

                            nonEquippedItems.push_back(std::move(item));

                            const std::string updateQuery = "UPDATE UserItems SET IsEquipped = 0 WHERE rowid = ?";
                            std::unique_ptr<sql::PreparedStatement> updateStmt(m_transactionalCon->prepareStatement(updateQuery));
                            updateStmt->setUInt64(1, rowId);
                            updateStmt->executeUpdate();
                        }
                    }
                    else
                    {
                        nonEquippedItems.push_back(std::move(item));
                    }
                }

                if (!itemNumbersToUpdate.empty())
                {
                    std::string updateItemNumbersQuery = "UPDATE UserItems SET ItemNumber = ? WHERE rowid = ?";
                    std::unique_ptr<sql::PreparedStatement> updateStmt(m_transactionalCon->prepareStatement(updateItemNumbersQuery));

                    for (const auto& [rowId, itemNumber] : itemNumbersToUpdate)
                    {
                        updateStmt->setInt64(1, itemNumber);
                        updateStmt->setUInt64(2, rowId);
                        updateStmt->executeUpdate();
                        updateStmt->clearParameters();
                    }
                }

                tx.commit();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::getPlayerItems");
                return {};
            }

            return std::pair{ std::move(nonEquippedItems), std::move(equippedItemsPerCharacter) };
        }

        bool PersistentDatabase::replaceItem(std::uint32_t accountID, std::uint64_t itemNumber, std::uint32_t newItemId)
        {
            try
            {
                const std::string queryStr = "UPDATE UserItems SET ItemID = ?, energy = 0 WHERE AccountID = ? AND ItemNumber = ?";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));
                stmt->setUInt(1, newItemId);
                stmt->setUInt(2, accountID);
                stmt->setUInt64(3, itemNumber); 

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("replaceItem: No rows affected. Possible reasons: item not found or already has the same ItemID. "
                        "AccountID: " + std::to_string(accountID) + ", ItemNumber: " + std::to_string(itemNumber) + ", NewItemID: " + std::to_string(newItemId),
                        Utils::LogType::Warning, "PersistentDatabase::replaceItem"); 
                    return false;
                }
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()) + " | AccountID: " + std::to_string(accountID) +
                    ", ItemNumber: " + std::to_string(itemNumber) + ", NewItemID: " + std::to_string(newItemId), 
                    Utils::LogType::Error, "PersistentDatabase::replaceItem");  
                return false;
            }
        }

        bool PersistentDatabase::replaceItemResetEnergy(std::uint32_t accountID, std::uint64_t itemNumber, std::uint32_t newItemId)
        {
            try
            {
                const std::string queryStr = "UPDATE UserItems SET ItemID = ?, energy = 0 WHERE AccountID = ? AND ItemNumber = ?";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));
                stmt->setUInt(1, newItemId);
                stmt->setUInt(2, accountID);
                stmt->setUInt64(3, itemNumber);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("replaceItemResetEnergy: No rows affected. Possible reasons: item not found or already has the same ItemID. "
                        "AccountID: " + std::to_string(accountID) +
                        ", ItemNumber: " + std::to_string(itemNumber) +
                        ", NewItemID: " + std::to_string(newItemId),
                        Utils::LogType::Warning, "PersistentDatabase::replaceItemResetEnergy");
                    return false;
                }
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()) +
                    " | AccountID: " + std::to_string(accountID) +
                    ", ItemNumber: " + std::to_string(itemNumber) +
                    ", NewItemID: " + std::to_string(newItemId),
                    Utils::LogType::Error, "PersistentDatabase::replaceItemResetEnergy");
                return false;
            }
        }

        bool PersistentDatabase::addPlayerItems(std::uint32_t accountID, const std::vector<Item>& items, std::uint32_t latestCharacterSelected)
        {
            try
            {
                TransactionGuard tx(m_transactionalCon);

                const std::string queryStr =
                    "INSERT INTO UserItems (AccountID, IsEquipped, CharacterID, ItemID, ItemDuration, ItemNumber, ItemOrigin, acquisitionServerId, creationDate,"
                    " durability, energy, isSealed, sealLevel, expEnhancement, mpEnhancement, IsCoupon, Stocks) "
                    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

                std::unique_ptr<sql::PreparedStatement> stmt(m_transactionalCon->prepareStatement(queryStr));
                for (const auto& item : items)
                {
                    stmt->setUInt(1, accountID);
                    stmt->setUInt(2, latestCharacterSelected == -1 ? 0 : 1);
                    stmt->setUInt(3, latestCharacterSelected == -1 ? 0 : latestCharacterSelected);
                    stmt->setUInt(4, item.itemId.itemId);
                    stmt->setInt64(5, static_cast<std::int64_t>(item.expirationDate <= 3 ? item.expirationDate : item.expirationDate - item.serialInfo.itemCreationDate));
                    stmt->setInt64(6, static_cast<std::int64_t>(item.serialInfo.itemNumber));
                    stmt->setInt64(7, static_cast<std::int64_t>(item.serialInfo.itemOrigin));
                    stmt->setInt64(8, static_cast<std::int64_t>(item.serialInfo.m_serverId));
                    stmt->setInt64(9, static_cast<std::int64_t>(item.serialInfo.itemCreationDate));
                    stmt->setUInt(10, static_cast<std::uint32_t>(item.durability));
                    stmt->setUInt(11, static_cast<std::uint32_t>(item.energy));
                    stmt->setUInt(12, 0); // isSealed
                    stmt->setUInt(13, 0); // sealLevel
                    stmt->setUInt(14, 0); // expEnhancement
                    stmt->setUInt(15, 0); // mpEnhancement
                    stmt->setUInt(16, 0); // unknown
                    stmt->setUInt(17, item.itemId.stock);
                    stmt->executeUpdate();
                }
                tx.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::addPlayerItems");
                return false;
            }
        }

        bool PersistentDatabase::updateItemStock(std::uint32_t accountID, std::uint64_t itemNumber, std::uint32_t newStock)
        {
            try
            {
                const std::string queryStr = "UPDATE UserItems SET Stocks = ? WHERE AccountID = ? AND ItemNumber = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));
                stmt->setUInt(1, newStock);     
                stmt->setUInt(2, accountID);      
                stmt->setInt64(3, static_cast<std::int64_t>(itemNumber)); 
                stmt->executeUpdate();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::updateItemStock");
                return false;
            }
        }

        void PersistentDatabase::prolongItems(std::uint32_t accountID, const std::vector<Main::Structures::BoughtItemToProlong>& toProlongItems,
            const std::vector<std::uint64_t>& itemDurations, std::uint32_t timeNow)
        {
            try
            {
                TransactionGuard tg(m_transactionalCon);

                const std::string prolongItemQuery = "UPDATE UserItems SET ItemDuration = ?, ItemOrigin = ?, acquisitionServerId = ? "
                    "WHERE AccountID = ? AND ItemNumber = ?";

                std::unique_ptr<sql::PreparedStatement> stmt(m_transactionalCon->prepareStatement(prolongItemQuery));

                for (std::size_t i = 0; i < toProlongItems.size(); ++i)
                {
                    const auto& itemToProlong = toProlongItems[i];
                    stmt->setInt64(1, static_cast<std::int64_t>(itemDurations[i] <= 3 ? itemDurations[i] : itemDurations[i] + timeNow));
                    stmt->setInt64(2, static_cast<std::int64_t>(itemToProlong.serialInfo.itemOrigin));
                    stmt->setInt64(3, static_cast<std::int64_t>(itemToProlong.serialInfo.m_serverId));
                    stmt->setInt64(4, static_cast<std::int64_t>(accountID));
                    stmt->setInt(5, static_cast<std::int32_t>(itemToProlong.serialInfo.itemNumber));

                    if (stmt->executeUpdate() == 0)
                    {
                        ::Utils::Logger::log("Failed to update item with ItemNumber: " + std::to_string(itemToProlong.serialInfo.itemNumber) +
                            " for AccountID: " + std::to_string(accountID), Utils::LogType::Warning, "PersistentDatabase::prolongItems");

                        return; 
                    }
                }

                tg.commit(); 
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::prolongItems");
            }
        }

        bool PersistentDatabase::removePlayerItem(std::uint32_t accountId, std::uint64_t itemNumber, const std::string& caller)
        {
            try
            {
                const std::string deleteItem = "DELETE FROM UserItems WHERE AccountID = ? AND ItemNumber = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(deleteItem));

                stmt->setUInt(1, accountId);
                stmt->setUInt64(2, itemNumber);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("[CALLER: " + caller + "] Warning: No rows affected by query for AccountID: " + std::to_string(accountId) +
                        " and ItemNumber: " + std::to_string(itemNumber), Utils::LogType::Warning, "PersistentDatabase::removePlayerItem");
                    return false;
                }

                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("[Main::Database::removePlayerItem] MariaDB exception: " + std::string(e.what()) +
                    " | AccountID: " + std::to_string(accountId) +
                    ", ItemNumber: " + std::to_string(itemNumber),
                    Utils::LogType::Error, "PersistentDatabase::removePlayerItem");
                return false;
            }
        }

        void PersistentDatabase::updatePlayerLevel(std::uint32_t accountID, std::uint16_t level)
        {
            try
            {
                std::string updateLevelQuery = "UPDATE Users SET Level = ? WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(updateLevelQuery));

                stmt->setUInt(1, level);
                stmt->setUInt(2, accountID);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("No rows changed with query", Utils::LogType::Warning, "PersistentDatabase::updatePlayerLevel");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log(std::string("MariaDB exception: ") + e.what(), Utils::LogType::Error, "PersistentDatabase::updatePlayerLevel");
            }
        }

        void PersistentDatabase::updatePlayerExperience(std::uint32_t accountID, std::uint32_t exp)
        {
            try
            {
                std::string updateLevelQuery = "UPDATE Users SET Experience = ? WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(updateLevelQuery));

                stmt->setUInt(1, exp);
                stmt->setUInt(2, accountID);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("No rows changed with query", Utils::LogType::Warning, "PersistentDatabase::updatePlayerExperience");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log(std::string("MariaDB exception: ") + e.what(), Utils::LogType::Error, "PersistentDatabase::updatePlayerExperience");
            }
        }

        bool PersistentDatabase::updatePlayerName(std::uint32_t accountID, const char* name)
        {
            try
            {
                std::string updateNameQuery = "UPDATE Users SET Nickname = ? WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(updateNameQuery));

                stmt->setString(1, name);
                stmt->setUInt(2, accountID);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("No rows changed with query", Utils::LogType::Warning, "PersistentDatabase::updatePlayerName");
                    return false;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log(std::string("MariaDB exception: ") + e.what(), Utils::LogType::Error, "PersistentDatabase::updatePlayerName");
                return false;
            }
            return true;
        }

        bool PersistentDatabase::updateSuspension(const std::string& nickname, const std::string& until, const std::string& reason, std::uint32_t executorGrade)
        {
            try
            {
                TransactionGuard tg(m_transactionalCon);

                std::string checkGradeQuery = "SELECT Grade FROM Users WHERE Nickname = ?";
                std::unique_ptr<sql::PreparedStatement> checkStmt(m_transactionalCon->prepareStatement(checkGradeQuery));
                checkStmt->setString(1, nickname);

                std::unique_ptr<sql::ResultSet> res(checkStmt->executeQuery());
                if (!res->next())
                {
                    return false;
                }

                if (res->getInt(1) >= static_cast<int>(executorGrade))
                {
                    return false;
                }

                std::string updateQuery = "UPDATE Users SET SuspendedUntil = ?, SuspensionReason = ? WHERE Nickname = ?";
                std::unique_ptr<sql::PreparedStatement> updateStmt(m_transactionalCon->prepareStatement(updateQuery));
                updateStmt->setString(1, until);
                updateStmt->setString(2, reason);
                updateStmt->setString(3, nickname);

                if (updateStmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Error executing query: " + updateQuery, Utils::LogType::Warning, "PersistentDatabase::updateSuspension");
                    return false;
                }

                tg.commit();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log(std::string("MariaDB exception: ") + e.what(), Utils::LogType::Error, "PersistentDatabase::updateSuspension");
                return false;
            }
            return true;
        }

        void PersistentDatabase::updateLatestRewardDay(const std::string& columnName, std::uint32_t accountId, const std::string& rewardDay)
        {
            try
            {
                std::string queryStr = "UPDATE Users SET " + columnName + " = ? WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));

                stmt->setString(1, rewardDay);
                stmt->setUInt(2, accountId);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Error executing query: " + queryStr, Utils::LogType::Warning, "PersistentDatabase::updateLatestRewardDay");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log(std::string("MariaDB exception: ") + e.what(), Utils::LogType::Error, "PersistentDatabase::updateLatestRewardDay");
            }
        }

        std::string PersistentDatabase::getLatestRewardDayFor(const std::string& columnName, std::uint32_t accountId)
        {
            try
            {
                const std::string queryStr = "SELECT " + columnName + " FROM Users WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));

                stmt->setUInt(1, accountId);
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (res->next())
                {
                    return res->getString(1).c_str();
                }
                else
                {
                    return "";
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log(std::string("MariaDB exception: ") + e.what(), Utils::LogType::Error, "PersistentDatabase::getLatestRewardDayFor");
                return "";
            }
        }

        bool PersistentDatabase::mustRewardsBeUpdated(const std::string& tableName, std::uint32_t daysToCheck)
        {
            try
            {
                std::string queryStr = "SELECT Date FROM " + tableName + " ORDER BY Date DESC LIMIT 1";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (res->next())
                {
                    std::string dateStr = res->getString("Date").c_str();

                    std::istringstream ss(dateStr);
                    int year, month, day;
                    char char1, char2;
                    ss >> year >> char1 >> month >> char2 >> day;

                    if (ss.fail() || char1 != '-' || char2 != '-' || month < 1 || month > 12 || day < 1 || day > 31)
                    {
                        return false;
                    }

                    std::chrono::year_month_day parsedDate = std::chrono::year_month_day(std::chrono::year(year), std::chrono::month(month), std::chrono::day(day));
                    auto todayTimestamp = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
                    return std::chrono::duration_cast<std::chrono::days>(todayTimestamp - std::chrono::sys_days(parsedDate)).count() >= daysToCheck;
                }
                else
                {
                    return true;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::mustRewardsBeUpdated");
                return false;
            }
        }

        void PersistentDatabase::updateRewards(const std::string& tableName, const std::vector<std::uint32_t>& items)
        {
            using std::chrono::system_clock;
            const auto today = std::format("{:%F}", std::chrono::floor<std::chrono::days>(system_clock::now()));

            try
            {
                TransactionGuard tg(m_transactionalCon);

                std::string countQueryStr = "SELECT COUNT(*) FROM " + tableName;
                std::unique_ptr<sql::PreparedStatement> countStmt(m_transactionalCon->prepareStatement(countQueryStr));
                std::unique_ptr<sql::ResultSet> countRes(countStmt->executeQuery());

                if (countRes->next() && countRes->getInt(1) > 0)
                {
                    std::string deleteQueryStr = "DELETE FROM " + tableName;
                    std::unique_ptr<sql::PreparedStatement> deleteStmt(m_transactionalCon->prepareStatement(deleteQueryStr));
                    if (deleteStmt->executeUpdate() == 0)
                    {
                        return;
                    }
                }

                std::string insertQueryStr = "INSERT INTO " + tableName + " (ItemID, Date) VALUES (?, ?)";
                std::unique_ptr<sql::PreparedStatement> insertStmt(m_transactionalCon->prepareStatement(insertQueryStr));

                for (const auto& item : items)
                {
                    insertStmt->setUInt(1, item);
                    insertStmt->setString(2, today);
                    if (insertStmt->executeUpdate() == 0)
                    {
                        return;
                    }
                }

                tg.commit();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::updateRewards");
            }
        }

        void PersistentDatabase::updateBattery(std::uint32_t accountId, std::uint32_t newBattery)
        {
            try
            {
                std::string updateQueryStr = "UPDATE Users SET Battery = ? WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(updateQueryStr));

                stmt->setUInt(1, newBattery);
                stmt->setUInt(2, accountId);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Update failed.", Utils::LogType::Warning, "PersistentDatabase::updateBattery");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::updateBattery");
            }
        }

        void PersistentDatabase::updateClanContribution(std::uint32_t clanId, std::uint32_t newContribution)
        {
            try
            {
                std::string updateQueryStr =
                    "UPDATE Clans SET TotalContribution = TotalContribution + ?  WHERE ClanId = ?";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(updateQueryStr));
                stmt->setUInt(1, newContribution);
                stmt->setUInt(2, clanId);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Update failed.", Utils::LogType::Warning, "PersistentDatabase::updateClanContribution");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::updateClanContribution");
            }
        }

        void PersistentDatabase::updateClanStats(std::uint32_t clanId, Main::Enums::MatchEnd endType)
        {
            try
            {
                std::string updateQueryStr = "UPDATE Clans SET TotalWins = TotalWins + ?, TotalLosses = TotalLosses + ?, "
                    "TotalDraws = TotalDraws + ? WHERE ClanId = ?";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(updateQueryStr));

                stmt->setUInt(1, (endType == Main::Enums::MATCH_WON) ? 1 : 0);  
                stmt->setUInt(2, (endType == Main::Enums::MATCH_LOST) ? 1 : 0);
                stmt->setUInt(3, (endType == Main::Enums::MATCH_DRAW) ? 1 : 0);  
                stmt->setUInt(4, clanId);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Update failed.", Utils::LogType::Warning, "PersistentDatabase::updateClanStats");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::updateClanStats");
            }
        }

        void PersistentDatabase::reduceDurability(std::uint32_t accountId, const std::vector<std::pair<std::uint32_t, std::uint64_t>>& equippedItems)
        {
            try
            {
                TransactionGuard tg(m_transactionalCon);

                std::string updateQueryStr = "UPDATE UserItems SET Durability = ? WHERE AccountID = ? AND ItemNumber = ? AND ItemDuration = 0";
                std::unique_ptr<sql::PreparedStatement> stmt(m_transactionalCon->prepareStatement(updateQueryStr));

                for (const auto& currentEquippedItem : equippedItems)
                {
                    const auto currentItemBaseDurability = Main::CdbUtils::getItemDurability(currentEquippedItem.first);

                    if (!currentItemBaseDurability || *currentItemBaseDurability == 0)
                        continue;

                    const std::uint32_t reduction = (*currentItemBaseDurability / 100) * 1;

                    const std::uint32_t newDurability = (*currentItemBaseDurability > reduction)
                        ? (*currentItemBaseDurability - reduction)
                        : 0;

                    stmt->setUInt(1, newDurability);
                    stmt->setUInt(2, accountId);
                    stmt->setUInt(3, currentEquippedItem.second);
                    stmt->executeUpdate();
                }

                tg.commit();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log(std::string("SQL Error: ") + e.what(), Utils::LogType::Error, "PersistentDatabase::reduceDurability");
            }
        }

        void PersistentDatabase::updateItemDurability(std::uint32_t accountId, std::uint32_t itemNumber, std::uint32_t newDurability)
        {
            try
            {
                const std::string updateQueryStr =
                    "UPDATE UserItems SET Durability = ? WHERE AccountID = ? AND ItemNumber = ?";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(updateQueryStr));

                stmt->setUInt(1, newDurability);
                stmt->setUInt(2, accountId);
                stmt->setUInt(3, itemNumber);
                stmt->executeUpdate();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log(std::string("SQL Error: ") + e.what(), Utils::LogType::Error, "PersistentDatabase::updateItemDurability");
            }
        }

        void PersistentDatabase::updatePlayerStats(std::uint32_t accountId, const Main::Structures::AccountInfo& updatedAccountInfo)
        {
            try
            {
                std::string updateQueryStr = "UPDATE Users SET MeleeKills = ?, RifleKills = ?, ShotgunKills = ?, SniperKills = ?, GatlingKills = ?, "
                    "BazookaKills = ?, GrenadeKills = ?, HighestKillstreak = ?, Kills = ?, Deaths = ?, Headshots = ?, Assists = ?, "
                    "Experience = ?, MicroPoints = ?, Wins = ?, Loses = ?, Draws = ?, Level = ?, ZombieKills = ?, InfectedKills = ?, "
                    "ClanKills = ?, ClanDeaths = ?, ClanAssists = ?, ClanWins = ?, ClanLoses = ?, ClanDraws = ?, ClanContribution = ?, "
                    "HighestSinglewaveScore = ?, HighestSinglewaveStage = ?, HasFinishedTutorial = ?, Playtime = ? WHERE AccountID = ? ";

                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(updateQueryStr));

                stmt->setUInt(1, updatedAccountInfo.meleeKills);
                stmt->setUInt(2, updatedAccountInfo.rifleKills);
                stmt->setUInt(3, updatedAccountInfo.shotgunKills);
                stmt->setUInt(4, updatedAccountInfo.sniperKills);
                stmt->setUInt(5, updatedAccountInfo.microgunKills);
                stmt->setUInt(6, updatedAccountInfo.bazookaKills);
                stmt->setUInt(7, updatedAccountInfo.grenadeKills);
                stmt->setUInt(8, static_cast<std::uint32_t>(updatedAccountInfo.killstreak));
                stmt->setUInt(9, updatedAccountInfo.totalKills);
                stmt->setUInt(10, updatedAccountInfo.deaths);
                stmt->setUInt(11, static_cast<std::uint32_t>(updatedAccountInfo.headshots));
                stmt->setUInt(12, updatedAccountInfo.assists);
                stmt->setUInt(13, updatedAccountInfo.experience);
                stmt->setUInt(14, static_cast<std::uint32_t>(updatedAccountInfo.microPoints));
                stmt->setUInt(15, updatedAccountInfo.wins);
                stmt->setUInt(16, updatedAccountInfo.losses);
                stmt->setUInt(17, updatedAccountInfo.draws);
                stmt->setUInt(18, static_cast<std::uint32_t>(updatedAccountInfo.playerLevel - 1));
                stmt->setUInt(19, updatedAccountInfo.zombieKills); 
                stmt->setUInt(20, updatedAccountInfo.infected);    
                stmt->setUInt(21, updatedAccountInfo.clanKills);
                stmt->setUInt(22, updatedAccountInfo.clanDeaths);
                stmt->setUInt(23, updatedAccountInfo.clanAssists);
                stmt->setUInt(24, updatedAccountInfo.clanWins);
                stmt->setUInt(25, updatedAccountInfo.clanLosses);
                stmt->setUInt(26, updatedAccountInfo.clanDraws);
                stmt->setUInt(27, updatedAccountInfo.clanContribution);
                stmt->setUInt(28, updatedAccountInfo.highestSingleWaveScore);
                stmt->setUInt(29, updatedAccountInfo.highestSinglewaveStage);
                stmt->setUInt(30, updatedAccountInfo.isTutorialDone);
                stmt->setUInt(31, updatedAccountInfo.playtime); 
                stmt->setUInt(32, accountId); 


                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Update failed.", Utils::LogType::Warning, "PersistentDatabase::updatePlayerStats");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::updatePlayerStats");
            }
        }

        bool PersistentDatabase::updateMute(const std::string& nickname, const std::string& until,
            const std::string& reason, const std::string& mutedBy,
            std::uint32_t executorGrade)
        {
            try
            {
                TransactionGuard tg(m_transactionalCon);

                std::string checkGradeQuery = "SELECT Grade FROM Users WHERE Nickname = ?";
                std::unique_ptr<sql::PreparedStatement> checkStmt(m_transactionalCon->prepareStatement(checkGradeQuery));
                checkStmt->setString(1, nickname);

                std::unique_ptr<sql::ResultSet> resultSet(checkStmt->executeQuery());  

                if (!resultSet->next())
                {
                    return false; 
                }

                const int targetPlayerGrade = resultSet->getInt("Grade");
                if (targetPlayerGrade >= executorGrade)
                {
                    return false;  
                }

                std::string updateQuery = "UPDATE Users SET MutedUntil = ?, MuteReason = ?, MutedBy = ? WHERE Nickname = ?";
                std::unique_ptr<sql::PreparedStatement> updateStmt(m_transactionalCon->prepareStatement(updateQuery));
                updateStmt->setString(1, until);
                updateStmt->setString(2, reason);
                updateStmt->setString(3, mutedBy);
                updateStmt->setString(4, nickname);

                if (updateStmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("No rows updated in updateMute", Utils::LogType::Warning, "PersistentDatabase::updateMute");
                    return false;
                }

                tg.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()),
                    Utils::LogType::Error, "PersistentDatabase::updateMute");
                return false;
            }
        }

        bool PersistentDatabase::updateVotekickDisabledUntil(const std::string& nickname, const std::string& until)
        {
            try
            {
                std::string updateQuery = "UPDATE Users SET VotekickDisabledUntil = ? WHERE Nickname = ?";
                std::unique_ptr<sql::PreparedStatement> updateStmt(m_con->prepareStatement(updateQuery));
                updateStmt->setString(1, until);
                updateStmt->setString(2, nickname);

                if (updateStmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Update failed: No rows affected.", Utils::LogType::Warning, "PersistentDatabase::updateVotekickDisabledUntil");
                    return false;
                }

                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::updateVotekickDisabledUntil");
                return false;
            }
        }

        bool PersistentDatabase::resetVotekickDisabledUntil(const std::string& nickname)
        {
            try
            {
                TransactionGuard tg(m_transactionalCon);

                std::string selectQueryStr = "SELECT AccountID FROM Users WHERE Nickname = ?";
                std::unique_ptr<sql::PreparedStatement> selectStmt(m_transactionalCon->prepareStatement(selectQueryStr));
                selectStmt->setString(1, nickname);

                std::unique_ptr<sql::ResultSet> resultSet(selectStmt->executeQuery());
                if (!resultSet->next())
                {
                    return false;
                }

                const std::uint32_t accountID = resultSet->getUInt("AccountID");
                std::string updateQueryStr = "UPDATE Users SET VotekickDisabledUntil = 0 WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> updateStmt(m_transactionalCon->prepareStatement(updateQueryStr));
                updateStmt->setUInt(1, accountID);

                if (updateStmt->executeUpdate() == 0)
                {
                    return false;
                }

                tg.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::resetVotekickDisabledUntil");
                return false;
            }
        }

        bool PersistentDatabase::updateRoomCreationDisabledUntil(const std::string& nickname, const std::string& until)
        {
            try
            {
                std::string updateQuery = "UPDATE Users SET RoomCreationDisabledUntil = ? WHERE Nickname = ?";
                std::unique_ptr<sql::PreparedStatement> updateStmt(m_con->prepareStatement(updateQuery));
                updateStmt->setString(1, until);
                updateStmt->setString(2, nickname);

                if (updateStmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Update failed: No rows affected.", Utils::LogType::Warning, "PersistentDatabase::updateRoomCreationDisabledUntil");
                    return false;
                }

                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::updateRoomCreationDisabledUntil");
                return false;
            }
        }

        bool PersistentDatabase::resetRoomCreationDisabledUntil(const std::string& nickname)
        {
            try
            {
                TransactionGuard tg(m_transactionalCon);

                std::string selectQueryStr = "SELECT AccountID FROM Users WHERE Nickname = ?";
                std::unique_ptr<sql::PreparedStatement> selectStmt(m_transactionalCon->prepareStatement(selectQueryStr));
                selectStmt->setString(1, nickname);

                std::unique_ptr<sql::ResultSet> resultSet(selectStmt->executeQuery());
                if (!resultSet->next())
                {
                    return false;
                }

                const std::uint32_t accountID = resultSet->getUInt("AccountID");
                std::string updateQueryStr = "UPDATE Users SET RoomCreationDisabledUntil = 0 WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> updateStmt(m_transactionalCon->prepareStatement(updateQueryStr));
                updateStmt->setUInt(1, accountID);

                if (updateStmt->executeUpdate() == 0)
                {
                    return false;
                }

                tg.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::resetRoomCreationDisabledUntil");
                return false;
            }
        }

        bool PersistentDatabase::unmuteAccount(const std::string& nickname)
        {
            try
            {
                TransactionGuard tg(m_transactionalCon);

                std::string selectQueryStr = "SELECT AccountID FROM Users WHERE Nickname = ?";
                std::unique_ptr<sql::PreparedStatement> selectStmt(m_transactionalCon->prepareStatement(selectQueryStr));
                selectStmt->setString(1, nickname);

                std::unique_ptr<sql::ResultSet> resultSet(selectStmt->executeQuery());
                if (!resultSet->next())
                {
                    return false;
                }

                const std::uint32_t accountID = resultSet->getUInt("AccountID");
                std::string updateQueryStr = "UPDATE Users SET MutedUntil = '1970-01-01 00:00:00' WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> updateStmt(m_transactionalCon->prepareStatement(updateQueryStr));
                updateStmt->setUInt(1, accountID);

                if (updateStmt->executeUpdate() == 0)
                {
                    return false;
                }

                tg.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::unmuteAccount");
                return false;
            }
        }

        void PersistentDatabase::switchItemEquip(std::uint32_t accountID, std::uint64_t itemNumber, std::uint32_t characterId)
        {
            try
            {
                std::string queryStr = "UPDATE UserItems SET IsEquipped = CASE WHEN IsEquipped = 0 THEN 1 ELSE 0 END, "
                    " CharacterID = ? WHERE AccountID = ? AND ItemNumber = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));

                stmt->setUInt(1, characterId);
                stmt->setUInt(2, accountID);
                stmt->setUInt64(3, itemNumber);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Update failed.", Utils::LogType::Warning, "PersistentDatabase::switchItemEquip");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::switchItemEquip");
            }
        }

        void PersistentDatabase::unequipItem(std::uint32_t accountID, std::uint64_t unequipItemNumber)
        {
            try
            {
                std::string queryStr = "UPDATE UserItems SET IsEquipped = 0, CharacterID = 0 WHERE AccountID = ? AND ItemNumber = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));

                stmt->setUInt(1, accountID);
                stmt->setUInt64(2, unequipItemNumber);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Update failed", Utils::LogType::Warning, "PersistentDatabase::unequipItem");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::unequipItem");
            }
        }

        void PersistentDatabase::equipItem(std::uint32_t accountID, std::uint64_t equipItemNumber, std::uint16_t characterId)
        {
            try
            {
                std::string queryStr = "UPDATE UserItems SET IsEquipped = 1, CharacterID = ? WHERE AccountID = ? AND ItemNumber = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(queryStr));

                stmt->setUInt(1, characterId);
                stmt->setUInt(2, accountID);
                stmt->setUInt64(3, equipItemNumber);

                if (stmt->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Update failed", Utils::LogType::Warning, "PersistentDatabase::equipItem");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::equipItem");
            }
        }

        void PersistentDatabase::swapItems(std::uint32_t accountID, std::uint64_t toUnequipItemNumber, std::uint64_t toEquipItemNumber, std::uint16_t characterId)
        {
            try
            {
                TransactionGuard tg(m_transactionalCon);

                std::string queryStr1 = "UPDATE UserItems SET IsEquipped = 1, CharacterID = ? WHERE AccountID = ? AND ItemNumber = ?";
                std::unique_ptr<sql::PreparedStatement> stmt1(m_transactionalCon->prepareStatement(queryStr1));
                stmt1->setUInt(1, characterId);
                stmt1->setUInt(2, accountID);
                stmt1->setUInt64(3, toEquipItemNumber);

                std::string queryStr2 = "UPDATE UserItems SET IsEquipped = 0, CharacterID = 0 WHERE AccountID = ? AND ItemNumber = ?";
                std::unique_ptr<sql::PreparedStatement> stmt2(m_transactionalCon->prepareStatement(queryStr2));
                stmt2->setUInt(1, accountID);
                stmt2->setUInt64(2, toUnequipItemNumber);

                if (stmt1->executeUpdate() == 0)
                {
                    return;
                }

                if (stmt2->executeUpdate() == 0)
                {
                    return;
                }

                tg.commit();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::swapItems");
            }
        }

        void PersistentDatabase::addFriend(std::uint32_t accountID, std::uint32_t targetAccountId)
        {
            try
            {
                TransactionGuard tg(m_transactionalCon);

                const std::string insertQuery = "INSERT IGNORE INTO Friendlist (AccountID, TargetAccountID) VALUES (?, ?)";
                std::unique_ptr<sql::PreparedStatement> stmt(m_transactionalCon->prepareStatement(insertQuery));

                stmt->setUInt(1, accountID);
                stmt->setUInt(2, targetAccountId);
                if (stmt->executeUpdate() == 0)
                {
                    return;
                }

                stmt->setUInt(1, targetAccountId);
                stmt->setUInt(2, accountID);
                if (stmt->executeUpdate() == 0)
                {
                    return;
                }

                tg.commit();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::addFriend");
            }
        }

        bool PersistentDatabase::resetKillDeath(std::uint32_t accountID)
        {
            try
            {
                const std::string updateQuery = "UPDATE Users SET Kills = 0, Deaths = 0 WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(updateQuery));
                stmt->setUInt(1, accountID);

                bool success = stmt->executeUpdate() > 0;

                if (!success)
                {
                    ::Utils::Logger::log("Failed to reset kills/deaths for account: " + std::to_string(accountID),
                        Utils::LogType::Warning, "PersistentDatabase::resetKillDeath");
                }

                return success;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()),
                    Utils::LogType::Error, "PersistentDatabase::resetKillDeath");
                return false;
            }
        }

        bool PersistentDatabase::resetRecord(std::uint32_t accountID)
        {
            try
            {
                const std::string resetQuery = "UPDATE Users SET Wins = 0, Loses = 0, Draws = 0 WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(resetQuery));
                stmt->setUInt(1, accountID);

                bool success = stmt->executeUpdate() > 0;

                if (!success)
                {
                    ::Utils::Logger::log("Failed to reset record for account: " + std::to_string(accountID),Utils::LogType::Warning, "PersistentDatabase::resetRecord");
                }

                return success;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()),Utils::LogType::Error, "PersistentDatabase::resetRecord");
                return false;
            }
        }

        void PersistentDatabase::addPlayerItem(const Item& item, std::uint32_t accountID, std::uint32_t latestCharacterSelected)
        {
            addPlayerItems(accountID, std::vector<Item>{ item }, latestCharacterSelected);
        }

        bool PersistentDatabase::batteryRecharge(std::uint32_t accountID, std::uint32_t quantity)
        {
            try
            {
                TransactionGuard tg(m_transactionalCon);

                const std::string getBattery = "SELECT Battery FROM Users WHERE AccountID = ?";
                const std::string updateBattery = "UPDATE Users SET Battery = ? WHERE AccountID = ?";

                std::unique_ptr<sql::PreparedStatement> stmtGetBattery(m_transactionalCon->prepareStatement(getBattery));
                std::unique_ptr<sql::PreparedStatement> stmtUpdateBattery(m_transactionalCon->prepareStatement(updateBattery));

                stmtGetBattery->setUInt(1, accountID);
                std::unique_ptr<sql::ResultSet> resGetBattery(stmtGetBattery->executeQuery());

                if (!resGetBattery->next())
                {
                    return false;
                }

                uint32_t currentBattery = resGetBattery->getUInt("Battery");
                stmtUpdateBattery->setUInt(1, currentBattery + quantity);
                stmtUpdateBattery->setUInt(2, accountID);

                if (stmtUpdateBattery->executeUpdate() == 0)
                {
                    return false;
                }

                tg.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::batteryRecharge");
                return false;
            }
        }

        bool PersistentDatabase::batteryExpansion(std::uint32_t accountID)
        {
            try
            {
                TransactionGuard tg(m_transactionalCon);

                const std::string getMaxBattery = "SELECT MaxBattery FROM Users WHERE AccountID = ?";
                const std::string updateMaxBattery = "UPDATE Users SET MaxBattery = ? WHERE AccountID = ?";

                std::unique_ptr<sql::PreparedStatement> stmtGetMaxBattery(m_transactionalCon->prepareStatement(getMaxBattery));
                std::unique_ptr<sql::PreparedStatement> stmtUpdateMaxBattery(m_transactionalCon->prepareStatement(updateMaxBattery));

                stmtGetMaxBattery->setUInt(1, accountID);
                std::unique_ptr<sql::ResultSet> resGetMaxBattery(stmtGetMaxBattery->executeQuery());

                if (!resGetMaxBattery->next())
                {
                    return false;
                }

                uint32_t currentMaxBattery = resGetMaxBattery->getUInt("MaxBattery");
                if (currentMaxBattery > 4000)
                {
                    return false;
                }

                uint32_t newMaxBattery = currentMaxBattery + 1000;
                stmtUpdateMaxBattery->setUInt(1, newMaxBattery);
                stmtUpdateMaxBattery->setUInt(2, accountID);

                if (stmtUpdateMaxBattery->executeUpdate() == 0)
                {
                    return false;
                }

                tg.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::batteryExpansion");
                return false;
            }
        }

        bool PersistentDatabase::inventoryExpansion(std::uint32_t accountID, std::uint32_t spaceToAdd)
        {
            try
            {
                TransactionGuard tg(m_transactionalCon);

                const std::string getMaxInventory = "SELECT MaxInventory FROM Users WHERE AccountID = ?";
                const std::string updateMaxInventory = "UPDATE Users SET MaxInventory = ? WHERE AccountID = ?";

                std::unique_ptr<sql::PreparedStatement> stmtGetMaxInventory(m_transactionalCon->prepareStatement(getMaxInventory));
                std::unique_ptr<sql::PreparedStatement> stmtUpdateMaxInventory(m_transactionalCon->prepareStatement(updateMaxInventory));

                stmtGetMaxInventory->setUInt(1, accountID);
                std::unique_ptr<sql::ResultSet> resGetMaxInventory(stmtGetMaxInventory->executeQuery());

                if (!resGetMaxInventory->next())
                {
                    return false;
                }

                uint32_t currentMaxInventory = resGetMaxInventory->getUInt("MaxInventory");

                if (currentMaxInventory + spaceToAdd > 1000)
                {
                    return false;
                }

                uint32_t newMaxInventory = currentMaxInventory + spaceToAdd;
                stmtUpdateMaxInventory->setUInt(1, newMaxInventory);
                stmtUpdateMaxInventory->setUInt(2, accountID);

                if (stmtUpdateMaxInventory->executeUpdate() == 0)
                {
                    return false;
                }

                tg.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::inventoryExpansion");
                return false;
            }
        }

        void PersistentDatabase::removeFriend(std::uint32_t accountID, std::uint32_t targetAccountId)
        {
            try
            {
                const std::string deleteFriend = "DELETE FROM Friendlist WHERE (AccountID = ? AND TargetAccountID = ?) OR (AccountID = ? AND TargetAccountID = ?)";
                std::unique_ptr<sql::PreparedStatement> stmtDeleteFriend(m_con->prepareStatement(deleteFriend));

                stmtDeleteFriend->setUInt(1, accountID);
                stmtDeleteFriend->setUInt(2, targetAccountId);
                stmtDeleteFriend->setUInt(3, targetAccountId);
                stmtDeleteFriend->setUInt(4, accountID);

                const int deleteResult = stmtDeleteFriend->executeUpdate();
                if (deleteResult == 0)
                {
                    ::Utils::Logger::log("Error executing delete: no rows affected", Utils::LogType::Warning, "PersistentDatabase::removeFriend");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::removeFriend");
            }
        }

        std::vector<Main::Structures::Friend> PersistentDatabase::loadFriends(std::uint32_t accountID)
        {
            std::vector<Main::Structures::Friend> friendList;
            std::size_t count = 0;
            try
            {
                static const char* selectFriendsQuery = "SELECT Friendlist.*, Users.Nickname AS TargetNickname "
                    "FROM Friendlist INNER JOIN Users ON Friendlist.TargetAccountID = Users.AccountID "
                    "WHERE Friendlist.AccountID = ?";

                std::unique_ptr<sql::PreparedStatement> stmtSelectFriends(m_con->prepareStatement(selectFriendsQuery));
                stmtSelectFriends->setUInt(1, accountID);
                std::unique_ptr<sql::ResultSet> res(stmtSelectFriends->executeQuery());

                Main::Structures::Friend ffriend;
                while (res->next())
                {
                    if (count >= Common::Constants::maxFriends)
                        break;

                    std::memcpy(ffriend.targetNickname, res->getString("TargetNickname").c_str(), Common::Constants::maxNicknameSize);
                    ffriend.targetAccountId = res->getInt("TargetAccountID");

                    friendList.push_back(ffriend);
                    ++count;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::loadFriends");
                return {};
            }

            return friendList;
        }

        std::vector<Main::Structures::BlockedPlayer> PersistentDatabase::loadBlockedPlayers(std::uint32_t accountID)
        {
            std::vector<Main::Structures::BlockedPlayer> blockedList;
            std::size_t count = 0;
            try
            {
                static const char* selectBlockedPlayersQuery = "SELECT BlockedPlayers.*, Users.Nickname AS TargetNickname "
                    "FROM BlockedPlayers INNER JOIN Users ON BlockedPlayers.TargetAccountID = Users.AccountID "
                    "WHERE BlockedPlayers.AccountID = ?";

                std::unique_ptr<sql::PreparedStatement> stmtSelectBlockedPlayers(m_con->prepareStatement(selectBlockedPlayersQuery));
                stmtSelectBlockedPlayers->setUInt(1, accountID);

                std::unique_ptr<sql::ResultSet> res(stmtSelectBlockedPlayers->executeQuery());

                Main::Structures::BlockedPlayer blockedPlayer;
                while (res->next())
                {
                    if (count >= Common::Constants::maxFriends)
                        break;

                    std::memcpy(blockedPlayer.targetNickname, res->getString("TargetNickname").c_str(), Common::Constants::maxNicknameSize);
                    blockedPlayer.targetAccountId = res->getInt("TargetAccountID");

                    blockedList.push_back(blockedPlayer);
                    ++count;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::loadBlockedPlayers");
                return {};
            }

            return blockedList;
        }

        void PersistentDatabase::blockPlayer(std::uint32_t accountID, std::uint32_t targetAccountId)
        {
            try
            {
                std::unique_ptr<sql::PreparedStatement> stmtBlockPlayer(m_con->prepareStatement("INSERT INTO BlockedPlayers (AccountID, TargetAccountID) VALUES (?, ?)"));
                stmtBlockPlayer->setUInt(1, accountID);
                stmtBlockPlayer->setUInt(2, targetAccountId);

                if (stmtBlockPlayer->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Error executing query", Utils::LogType::Warning, "PersistentDatabase::blockPlayer");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::blockPlayer");
            }
        }

        std::optional<std::uint32_t> PersistentDatabase::blockPlayerByNickname(std::uint32_t accountID, const std::string& targetNickname)
        {
            try
            {
                TransactionGuard tg(m_transactionalCon);

                std::unique_ptr<sql::PreparedStatement> stmtFind(m_transactionalCon->prepareStatement(
                    "SELECT AccountID, Grade FROM Users WHERE Nickname = ?"));
                stmtFind->setString(1, targetNickname);
                std::unique_ptr<sql::ResultSet> resFind(stmtFind->executeQuery());

                if (!resFind->next())
                    return std::nullopt;

                const std::uint32_t targetAccountId = resFind->getUInt("AccountID");
                const std::uint32_t targetGrade = resFind->getUInt("Grade");

                if (targetGrade >= 3)
                    return std::nullopt;

                std::unique_ptr<sql::PreparedStatement> stmtBlockPlayer(m_transactionalCon->prepareStatement(
                    "INSERT INTO BlockedPlayers (AccountID, TargetAccountID) VALUES (?, ?)"));
                stmtBlockPlayer->setUInt(1, accountID);
                stmtBlockPlayer->setUInt(2, targetAccountId);

                if (stmtBlockPlayer->executeUpdate() == 0)
                    return std::nullopt;

                tg.commit();
                return targetAccountId;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::blockPlayerByNickname");
                return std::nullopt;
            }
        }

        void PersistentDatabase::unblockPlayer(std::uint32_t accountID, std::uint32_t targetAccountId)
        {
            try
            {
                std::unique_ptr<sql::PreparedStatement> stmtUnblockPlayer(m_con->prepareStatement("DELETE FROM BlockedPlayers WHERE AccountID = ? AND TargetAccountID = ?"));
                stmtUnblockPlayer->setUInt(1, accountID);
                stmtUnblockPlayer->setUInt(2, targetAccountId);

                if (stmtUnblockPlayer->executeUpdate() == 0)
                {
                    ::Utils::Logger::log("Error executing query", Utils::LogType::Warning, "PersistentDatabase::unblockPlayer");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::unblockPlayer");
            }
        }

        Main::Enums::AddFriendServerExtra PersistentDatabase::addPendingFriendRequest(
            std::uint32_t aid,
            const char* targetName)
        {
            try
            {
                TransactionGuard tx(m_transactionalCon);

                std::uint32_t targetAid = 0;

                std::unique_ptr<sql::PreparedStatement> stmtFindPlayer(
                    m_transactionalCon->prepareStatement("SELECT AccountID FROM Users WHERE Nickname = ? FOR UPDATE"));
                stmtFindPlayer->setString(1, targetName);

                std::unique_ptr<sql::ResultSet> resFindPlayer(stmtFindPlayer->executeQuery());

                if (!resFindPlayer->next())
                    return Main::Enums::AddFriendServerExtra::TARGET_NOT_FOUND;

                targetAid = static_cast<std::uint32_t>(
                    resFindPlayer->getUInt("AccountID"));

                if (targetAid == aid)
                    return Main::Enums::AddFriendServerExtra::TARGET_NOT_FOUND;

                std::unique_ptr<sql::PreparedStatement> stmtFriendCount(
                    m_transactionalCon->prepareStatement("SELECT COUNT(*) AS FriendCount FROM Friendlist WHERE AccountID = ? FOR UPDATE"));
                stmtFriendCount->setUInt(1, targetAid);

                std::unique_ptr<sql::ResultSet> resFriendCount(stmtFriendCount->executeQuery());

                if (!resFriendCount->next())
                    return Main::Enums::AddFriendServerExtra::DB_ERROR;

                if (resFriendCount->getUInt("FriendCount") >= Common::Constants::maxFriends)
                    return Main::Enums::AddFriendServerExtra::TARGET_OR_SENDER_FRIEND_LIST_FULL;

                std::unique_ptr<sql::PreparedStatement> stmtBlocked(
                    m_transactionalCon->prepareStatement("SELECT 1 FROM BlockedPlayers WHERE AccountID = ? AND TargetAccountID = ? FOR UPDATE"));
                stmtBlocked->setUInt(1, targetAid);
                stmtBlocked->setUInt(2, aid);

                std::unique_ptr<sql::ResultSet> resBlocked(stmtBlocked->executeQuery());

                if (resBlocked->next())
                    return Main::Enums::AddFriendServerExtra::RECEIVER_BLOCKED_SENDER;

                std::unique_ptr<sql::PreparedStatement> stmtInsert(
                    m_transactionalCon->prepareStatement("INSERT INTO PendingFriendRequests (AccountID, TargetAccountID) VALUES (?, ?)"));
                stmtInsert->setUInt(1, targetAid);
                stmtInsert->setUInt(2, aid);

                if (stmtInsert->executeUpdate() == 0)
                    return Main::Enums::AddFriendServerExtra::DB_ERROR;

                tx.commit();
                return Main::Enums::AddFriendServerExtra::REQUEST_SENT;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()),Utils::LogType::Error,"PersistentDatabase::addPendingFriendRequest");

                return Main::Enums::AddFriendServerExtra::DB_ERROR;
            }
        }

        std::vector<Main::Structures::Friend> PersistentDatabase::loadPendingFriendRequests(std::uint32_t accountID)
        {
            std::vector<Main::Structures::Friend> pendingFriendRequests;

            try
            {
                TransactionGuard tx(m_transactionalCon);

                std::unique_ptr<sql::PreparedStatement> stmt(
                    m_transactionalCon->prepareStatement("SELECT PendingFriendRequests.*, Users.Nickname AS TargetNickname "
                        "FROM PendingFriendRequests INNER JOIN Users ON PendingFriendRequests.TargetAccountID = Users.AccountID "
                        "WHERE PendingFriendRequests.AccountID = ? FOR UPDATE"));

                stmt->setUInt(1, accountID);
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
                Main::Structures::Friend ffriend{};

                while (res->next())
                {
                    std::memcpy(ffriend.targetNickname,res->getString("TargetNickname").c_str(),16);

                    ffriend.targetAccountId = res->getUInt("TargetAccountID");

                    pendingFriendRequests.push_back(ffriend);
                }

                if (!pendingFriendRequests.empty())
                {
                    std::unique_ptr<sql::PreparedStatement> stmtRemove(m_transactionalCon->prepareStatement("DELETE FROM PendingFriendRequests WHERE AccountID = ?"));

                    stmtRemove->setUInt(1, accountID);
                    stmtRemove->executeUpdate();
                }

                tx.commit();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()),Utils::LogType::Error,"PersistentDatabase::loadPendingFriendRequests");
            }

            return pendingFriendRequests;
        }

        std::vector<Main::Structures::SingleModeEvent> PersistentDatabase::getEventsModeList()
        {
            std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement("SELECT * FROM EventsModes"));
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

            std::vector<Main::Structures::SingleModeEvent> events;
            Main::Structures::SingleModeEvent singleEvent;

            while (res->next())
            {
                singleEvent.gameMode = static_cast<Common::Enums::GameModes>(res->getInt("GameMode"));

                const std::string startDateStr = res->getString("StartDate").c_str();
                const std::string endDateStr = res->getString("EndDate").c_str();

                std::tm startTm = {};
                std::tm endTm = {};

                std::istringstream(startDateStr) >> std::get_time(&startTm, "%Y-%m-%d %H:%M:%S");
                std::istringstream(endDateStr) >> std::get_time(&endTm, "%Y-%m-%d %H:%M:%S");

                auto startTimePoint = std::chrono::system_clock::from_time_t(std::mktime(&startTm));
                auto endTimePoint = std::chrono::system_clock::from_time_t(std::mktime(&endTm));

                singleEvent.startDate = static_cast<__time32_t>(std::chrono::system_clock::to_time_t(startTimePoint));
                singleEvent.endDate = static_cast<__time32_t>(std::chrono::system_clock::to_time_t(endTimePoint));

                events.push_back(singleEvent);
            }

            return events;
        }

        std::vector<Main::Structures::SingleMapEvent> PersistentDatabase::getEventsMapList()
        {
            std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement("SELECT * FROM EventsMaps"));
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

            std::vector<Main::Structures::SingleMapEvent> events;
            Main::Structures::SingleMapEvent singleEvent;

            while (res->next())
            {
                singleEvent.gameMap = static_cast<Common::Enums::GameMaps>(res->getInt("GameMap"));

                const std::string startDateStr = res->getString("StartDate").c_str();
                const std::string endDateStr = res->getString("EndDate").c_str();

                std::tm startTm = {};
                std::tm endTm = {};

                std::istringstream(startDateStr) >> std::get_time(&startTm, "%Y-%m-%d %H:%M:%S");
                std::istringstream(endDateStr) >> std::get_time(&endTm, "%Y-%m-%d %H:%M:%S");

                auto startTimePoint = std::chrono::system_clock::from_time_t(std::mktime(&startTm));
                auto endTimePoint = std::chrono::system_clock::from_time_t(std::mktime(&endTm));

                singleEvent.startDate = static_cast<__time32_t>(std::chrono::system_clock::to_time_t(startTimePoint));
                singleEvent.endDate = static_cast<__time32_t>(std::chrono::system_clock::to_time_t(endTimePoint));

                events.push_back(singleEvent);
            }

            return events;
        }

        void PersistentDatabase::storeMailbox(const Main::Structures::Mailbox& mailbox, std::uint32_t accountId, bool isSent)
        {
            try
            {
                const std::string storeMailboxQuery = "INSERT INTO Mailbox (accountId, timestamp, nickname, message, sent) VALUES (?, ?, ?, ?, ?)";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(storeMailboxQuery));

                stmt->setUInt(1, accountId);
                stmt->setUInt64(2, mailbox.timestamp);
                stmt->setString(3, mailbox.nickname);
                stmt->setString(4, mailbox.message);
                stmt->setBoolean(5, isSent);

                if (stmt->executeUpdate() != 1)
                {
                    ::Utils::Logger::log("Error executing query: " + storeMailboxQuery, Utils::LogType::Warning, "PersistentDatabase::storeMailbox");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("SQLException: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::storeMailbox");
            }
        }

        bool PersistentDatabase::storeGiftbox(const Main::Structures::Giftbox& giftbox, std::uint32_t accountId)
        {
            try
            {
                const std::string storeGiftboxQuery = "INSERT INTO Giftbox (accountId, itemId, timestamp, sender, message) VALUES (?, ?, ?, ?, ?)";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(storeGiftboxQuery));

                stmt->setUInt(1, accountId);
                stmt->setUInt(2, giftbox.id);
                stmt->setUInt64(3, giftbox.timestamp);
                stmt->setString(4, giftbox.nickname);
                stmt->setString(5, giftbox.message);

                if (stmt->executeUpdate() != 1)
                {
                    ::Utils::Logger::log("Error executing query: " + storeGiftboxQuery, Utils::LogType::Warning, "PersistentDatabase::storeGiftbox");
                    return false;
                }
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("SQLException: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::storeGiftbox");
                return false;
            }
        }

        bool PersistentDatabase::storeGiftbox(const std::string& nickname, const std::string& giftDescription, std::uint32_t itemId)
        {
            try
            {
                TransactionGuard tx(m_transactionalCon);

                const std::string retrieveAccountIdQuery = "SELECT AccountID FROM Users WHERE Nickname = ? FOR UPDATE";
                std::unique_ptr<sql::PreparedStatement> stmt(m_transactionalCon->prepareStatement(retrieveAccountIdQuery));
                stmt->setString(1, nickname);

                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (!res->next())
                {
                    ::Utils::Logger::log("No user found with nickname: " + nickname, Utils::LogType::Warning, "PersistentDatabase::storeGiftbox");
                    return false;
                }
                const std::uint32_t accountId = res->getUInt("AccountID");

                const std::string countGiftboxesQuery = "SELECT COUNT(*) FROM Giftbox WHERE accountId = ? FOR UPDATE";
                std::unique_ptr<sql::PreparedStatement> countStmt(m_transactionalCon->prepareStatement(countGiftboxesQuery));
                countStmt->setUInt(1, accountId);

                std::unique_ptr<sql::ResultSet> countRes(countStmt->executeQuery());

                if (countRes->next())
                {
                    const int giftboxCount = countRes->getInt(1);
                    if (giftboxCount >= Common::Constants::maxMailbox)
                    {
                        return false;
                    }
                }

                Main::Structures::Giftbox giftbox{ accountId, static_cast<__time32_t>(std::time(0)), itemId, itemId, itemId };
                std::memcpy(giftbox.nickname, Common::Constants::teamString.c_str(), Common::Constants::teamString.size());
                std::memcpy(giftbox.message, giftDescription.c_str(), giftDescription.size());

                const std::string storeGiftboxQuery = "INSERT INTO Giftbox (accountId, itemId, timestamp, sender, message) VALUES (?, ?, ?, ?, ?)";
                std::unique_ptr<sql::PreparedStatement> insertStmt(m_transactionalCon->prepareStatement(storeGiftboxQuery));

                insertStmt->setUInt(1, accountId);
                insertStmt->setUInt(2, giftbox.id);
                insertStmt->setUInt64(3, giftbox.timestamp);
                insertStmt->setString(4, giftbox.nickname);
                insertStmt->setString(5, giftbox.message);

                if (insertStmt->executeUpdate() != 1)
                {
                    ::Utils::Logger::log("Error executing query: " + storeGiftboxQuery, Utils::LogType::Warning, "PersistentDatabase::storeGiftbox");
                    return false;
                }

                tx.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("SQLException: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::storeGiftbox");
                return false;
            }
        }
       
        Main::Enums::MailboxExtra PersistentDatabase::storeOfflineMailbox(const Main::Structures::Mailbox& mailbox, const char* senderNickname)
        {
            try
            {
                TransactionGuard tx(m_transactionalCon);

                std::uint32_t accountId = 0;
                const std::string retrieveAccountIdQuery = "SELECT AccountID FROM Users WHERE Nickname = ? FOR UPDATE";
                std::unique_ptr<sql::PreparedStatement> stmt(m_transactionalCon->prepareStatement(retrieveAccountIdQuery));
                stmt->setString(1, mailbox.nickname);

                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
                if (res->next())
                    accountId = res->getUInt("AccountID");
                else
                    return Main::Enums::MailboxExtra::RECEIVER_NOT_FOUND;

                const std::string countMailboxesQuery = "SELECT COUNT(*) FROM Mailbox WHERE accountId = ? AND sent = 0 FOR UPDATE";
                std::unique_ptr<sql::PreparedStatement> countStmt(m_transactionalCon->prepareStatement(countMailboxesQuery));
                countStmt->setUInt(1, accountId);

                std::unique_ptr<sql::ResultSet> countRes(countStmt->executeQuery());
                if (countRes->next() && countRes->getInt(1) >= Common::Constants::maxMailbox)
                    return Main::Enums::MailboxExtra::RECEIVER_NO_SPACE_LEFT;

                std::uint32_t senderAccountId = 0;
                std::unique_ptr<sql::PreparedStatement> senderQuery(m_transactionalCon->prepareStatement(retrieveAccountIdQuery));
                senderQuery->setString(1, senderNickname);

                std::unique_ptr<sql::ResultSet> senderRes(senderQuery->executeQuery());
                if (senderRes->next())
                    senderAccountId = senderRes->getUInt("AccountID");
                else
                    return Main::Enums::MailboxExtra::MAILBOX_DB_ERROR;

                const std::string blockedPlayersQuery = "SELECT 1 FROM BlockedPlayers WHERE AccountID = ? AND TargetAccountID = ? FOR UPDATE";
                std::unique_ptr<sql::PreparedStatement> blockCheckQuery(m_transactionalCon->prepareStatement(blockedPlayersQuery));
                blockCheckQuery->setUInt(1, accountId);
                blockCheckQuery->setUInt(2, senderAccountId);

                std::unique_ptr<sql::ResultSet> blockCheckRes(blockCheckQuery->executeQuery());
                if (blockCheckRes->next())
                    return Main::Enums::MailboxExtra::MAILBOX_RECEIVER_BLOCKED_SENDER;

                const std::string storeMailboxQuery =
                    "INSERT INTO Mailbox (accountId, timestamp, uniqueId, nickname, message, sent, isNew) VALUES (?, ?, ?, ?, ?, ?, ?)";
                std::unique_ptr<sql::PreparedStatement> query(m_transactionalCon->prepareStatement(storeMailboxQuery));

                query->setUInt(1, accountId);
                query->setUInt64(2, mailbox.timestamp);
                query->setUInt64(3, 0);
                query->setString(4, senderNickname);
                query->setString(5, mailbox.message);
                query->setBoolean(6, false);
                query->setBoolean(7, true);

                if (query->executeUpdate() != 1)
                {
                    ::Utils::Logger::log("Error executing query: " + storeMailboxQuery, Utils::LogType::Error, "PersistentDatabase::storeOfflineMailbox");
                    return Main::Enums::MailboxExtra::MAILBOX_DB_ERROR;
                }

                tx.commit();
                return Main::Enums::MailboxExtra::MAILBOX_SENT;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("SQLException: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::storeOfflineMailbox");
                return Main::Enums::MailboxExtra::MAILBOX_DB_ERROR;
            }
        }

        std::vector<Main::Structures::Mailbox> PersistentDatabase::getNewMailboxes(std::uint32_t accountID)
        {
            std::vector<Main::Structures::Mailbox> mailboxes;

            try
            {
                const std::string selectMailboxQuery = "SELECT * FROM Mailbox WHERE accountId = ? AND isNew = 1";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(selectMailboxQuery));
                stmt->setUInt(1, accountID);

                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                Main::Structures::Mailbox mailbox;
                while (res->next())
                {
                    mailbox.accountId = res->getUInt("accountId");
                    mailbox.timestamp = res->getUInt("timestamp");
                    std::memcpy(mailbox.nickname, res->getString("nickname").c_str(), 16);
                    std::memcpy(mailbox.message, res->getString("message").c_str(), 256);
                    mailboxes.push_back(mailbox);
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("SQLException: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::getNewMailboxes");
            }

            return mailboxes;
        }

        void PersistentDatabase::updateReadMailbox(std::uint32_t accountID, std::uint32_t timestamp)
        {
            try
            {
                const std::string updateReadMailboxQuery = "UPDATE Mailbox SET isNew = 0 WHERE accountId = ? AND timestamp = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(updateReadMailboxQuery));
                stmt->setUInt(1, accountID);
                stmt->setUInt(2, timestamp);

                if (stmt->executeUpdate() != 1)
                {
                    ::Utils::Logger::log("Error executing query: " + updateReadMailboxQuery, Utils::LogType::Warning, "PersistentDatabase::updateReadMailbox");
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("SQLException: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::updateReadMailbox");
            }
        }

        void PersistentDatabase::deleteMailbox(std::uint32_t timestamp, std::uint32_t accountId, bool isSent)
        {
            try
            {
                const std::string deleteMailboxQuery = "DELETE FROM Mailbox WHERE timestamp = ? AND accountId = ? AND sent = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(deleteMailboxQuery));

                stmt->setUInt(1, timestamp);
                stmt->setUInt(2, accountId);
                stmt->setBoolean(3, isSent);

                if (stmt->executeUpdate() != 1)
                {
                    ::Utils::Logger::log("Error executing query: " + deleteMailboxQuery, Utils::LogType::Warning, "PersistentDatabase::deleteMailbox");
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("SQLException: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::deleteMailbox");
            }
        }

        void PersistentDatabase::deleteReceivedGiftbox(std::uint32_t accountId, std::uint32_t timestamp)
        {
            try
            {
                const std::string deleteGiftboxQuery = "DELETE FROM Giftbox WHERE timestamp = ? AND accountId = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(deleteGiftboxQuery));

                stmt->setUInt(1, timestamp);
                stmt->setUInt(2, accountId);

                if (stmt->executeUpdate() != 1)
                {
                    ::Utils::Logger::log("Error executing query: " + deleteGiftboxQuery, Utils::LogType::Warning, "PersistentDatabase::deleteReceivedGiftbox");
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("SQLException: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::deleteReceivedGiftbox");
            }
        }

        std::pair<std::vector<Main::Structures::Mailbox>, std::vector<Main::Structures::Mailbox>>
            PersistentDatabase::loadMailboxes(std::uint32_t accountID)
        {
            std::vector<Main::Structures::Mailbox> sentMailboxes;
            std::vector<Main::Structures::Mailbox> receivedMailboxes;

            try
            {
                const std::string selectMailboxQuery = "SELECT * FROM Mailbox WHERE accountId = ? AND (sent = ? OR sent = ?)";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(selectMailboxQuery));

                stmt->setUInt(1, accountID);
                stmt->setBoolean(2, true);
                stmt->setBoolean(3, false);

                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                Main::Structures::Mailbox mailbox;
                while (res->next())
                {
                    mailbox.accountId = res->getInt("accountId");
                    mailbox.timestamp = res->getInt("timestamp");
                    mailbox.hasBeenRead = !res->getBoolean("isNew");
                    const std::string nicknameStr = res->getString("nickname").c_str();
                    std::memset(mailbox.nickname, 0, sizeof(mailbox.nickname));
                    std::memcpy(mailbox.nickname, nicknameStr.c_str(), std::min(nicknameStr.size(), sizeof(mailbox.nickname) - 1));

                    const std::string messageStr = res->getString("message").c_str();
                    std::memset(mailbox.message, 0, sizeof(mailbox.message));
                    std::memcpy(mailbox.message, messageStr.c_str(), std::min(messageStr.size(), sizeof(mailbox.message) - 1));

                    if (res->getBoolean("sent"))
                        sentMailboxes.push_back(mailbox);
                    else
                        receivedMailboxes.push_back(mailbox);
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("SQLException: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::loadMailboxes");
                return {};
            }

            return { sentMailboxes, receivedMailboxes };
        }

        std::vector<Main::Structures::Giftbox> PersistentDatabase::loadReceivedGiftboxes(std::uint32_t accountID)
        {
            std::vector<Main::Structures::Giftbox> giftboxes;
            try
            {
                const std::string selectGiftboxQuery = "SELECT * FROM Giftbox WHERE accountId = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(selectGiftboxQuery));

                stmt->setUInt(1, accountID);

                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                Main::Structures::Giftbox giftbox;
                while (res && res->next())
                {
                    giftbox.accountId = res->getInt("accountId");
                    giftbox.timestamp = res->getInt("timestamp");
                    giftbox.id = giftbox.id1 = giftbox.id2 = res->getInt("itemId");

                    const std::string& senderStr = res->getString("sender").c_str();
                    const std::string& messageStr = res->getString("message").c_str();

                    std::memset(giftbox.nickname, 0, sizeof(giftbox.nickname));
                    std::memset(giftbox.message, 0, sizeof(giftbox.message));
                    std::memcpy(giftbox.nickname, senderStr.c_str(), std::min(senderStr.size(), sizeof(giftbox.nickname) - 1));
                    std::memcpy(giftbox.message, messageStr.c_str(), std::min(messageStr.size(), sizeof(giftbox.message) - 1));

                    giftboxes.push_back(giftbox);
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("SQLException: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::loadReceivedGiftboxes");
                return {};
            }
            return giftboxes;
        }

        void PersistentDatabase::insertEnergyToItem(std::uint32_t accountID, std::uint64_t itemNumber, std::uint32_t newItemEnergy, std::uint32_t newTotalEnergy)
        {
            try
            {
                TransactionGuard tx(m_transactionalCon);

                const std::string setEnergyForItem = "UPDATE UserItems SET energy = ? WHERE AccountID = ? AND ItemNumber = ?";
                std::unique_ptr<sql::PreparedStatement> stmt1(m_transactionalCon->prepareStatement(setEnergyForItem));
                stmt1->setUInt(1, newItemEnergy);
                stmt1->setUInt(2, accountID);
                stmt1->setUInt64(3, itemNumber);

                const std::string removeEnergyFromTotalEnergy = "UPDATE Users SET Battery = ? WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> stmt2(m_transactionalCon->prepareStatement(removeEnergyFromTotalEnergy));
                stmt2->setUInt(1, newTotalEnergy);
                stmt2->setUInt(2, accountID);

                if (stmt1->executeUpdate() != 1 || stmt2->executeUpdate() != 1)
                {
                    ::Utils::Logger::log("Error executing queries for energy-insertion", Utils::LogType::Warning, "PersistentDatabase::insertEnergyToItem");
                    return;
                }

                Main::Structures::ItemLogInfo logInfo{ itemNumber, 0, 0, "The user added energy to this item. New item energy is " + std::to_string(newItemEnergy) };
                insertItemLog(accountID, logInfo);

                tx.commit();
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("SQLException: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::insertEnergyToItem");
            }
        }

        void PersistentDatabase::updatePlayerLuckyPoints(std::uint32_t accountID, std::uint32_t luckyPoints)
        {
            try
            {
                std::string updateLevelQuery = "UPDATE Users SET LuckyPoints = ? WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(updateLevelQuery));

                stmt->setUInt(1, luckyPoints);
                stmt->setUInt(2, accountID);

                if (stmt->executeUpdate() != 1)
                {
                    ::Utils::Logger::log("Error executing query: " + updateLevelQuery, Utils::LogType::Warning, "PersistentDatabase::updatePlayerLuckyPoints");
                    return;
                }
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("SQLException: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::updatePlayerLuckyPoints");
            }
        }

        bool PersistentDatabase::setCommandEventExpirationHours(std::uint32_t hoursFromNow)
        {
            try
            {
                TransactionGuard tx(m_transactionalCon);

                auto now = std::chrono::system_clock::now();
                auto expirationTime = now + std::chrono::hours(hoursFromNow);
                std::time_t expirationT = std::chrono::system_clock::to_time_t(expirationTime);

                std::ostringstream oss;
                oss << std::put_time(std::localtime(&expirationT), "%Y-%m-%d %H:%M:%S");
                const std::string expirationDate = oss.str();

                const std::string updateQuery = "UPDATE EventCommands SET ExpirationDate = ?";
                std::unique_ptr<sql::PreparedStatement> updateStmt(m_transactionalCon->prepareStatement(updateQuery));
                updateStmt->setString(1, expirationDate);
                int affected = updateStmt->executeUpdate();

                if (affected == 0)
                {
                    const std::string insertQuery = "INSERT INTO EventCommands (ExpirationDate) VALUES (?)";
                    std::unique_ptr<sql::PreparedStatement> insertStmt(m_transactionalCon->prepareStatement(insertQuery));
                    insertStmt->setString(1, expirationDate);
                    insertStmt->executeUpdate();
                }

                tx.commit();
                return true;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::setCommandEventExpirationHours");
                return false;
            }
        }

        bool PersistentDatabase::isCommandEventExpired()
        {
            try
            {
                const std::string selectQuery = "SELECT ExpirationDate < NOW() AS Expired FROM EventCommands LIMIT 1";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(selectQuery));
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (res->next())
                    return res->getBoolean("Expired");
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception: " + std::string(e.what()), Utils::LogType::Error, "PersistentDatabase::isEventExpired");
            }

            return true; 
        }

        bool PersistentDatabase::getGradedHwid(std::uint32_t accountId, std::string& outHash, std::string& outSalt) const
        {
            try
            {
                std::string query = "SELECT HWIDGraded, HWIDGradedSalt FROM Users WHERE AccountID = ?";
                std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(query));
                stmt->setUInt(1, accountId);

                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
                if (res->next())
                {
                    outHash = res->getString("HWIDGraded").c_str();
                    outSalt = res->getString("HWIDGradedSalt").c_str();
                    return true;
                }

                return false;
            }
            catch (const sql::SQLException& e)
            {
                ::Utils::Logger::log("MariaDB exception in getGradedHwid: " + std::string(e.what()), ::Utils::LogType::Error);
                return false;
            }
        }

    } // end namespace Main
} // end namespace Persistence
