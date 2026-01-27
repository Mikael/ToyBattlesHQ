
#include <mutex>
#include <vector>
#include <iostream>
#include <cstdlib> 
#include "../../include/Enums/MiscellaneousEnums.h"
#include "IniParser/inicpp.h"
#include "../../include/Utils/Utils.h"
#include "../../include/Utils/Logger.h"
#include "../../include/Utils/SetupParser.h"
#include <regex>

namespace Common
{
    namespace Utils
    {
        SetupParser::SetupParser()
        {
            m_iniFile.load("../Setup/config.ini");
            if (!sanityCheck())
            {
                handleError("Sanity check failed");
            }

            check_assign(Common::Utils::getLocalIp(), m_localIp, "Could not retrieve local IP!");
            check_assign(getSelfMainServerInfoImpl(), m_selfMainInfo, "Could not retrieve self main info!");
            check_assign(getSelfCastServerInfoImpl(), m_selfCastInfo, "Could not retrieve self cast info!");
            check_assign(getMainServersInfoImpl(), m_mainInfos, "Could not retrieve main servers information!");
            check_assign(getCastServersInfoImpl(), m_castInfos, "Could not retrieve cast servers information!");
            m_authSetup = getAuthSetupImpl();
            m_dbSetup = getDatabaseSetupImpl();
            m_websiteSetup = getWebsiteSetupImpl();
            m_clientSetup = getClientSetupImpl();
        }

        void SetupParser::handleError(const std::string& message)
        {
            ::Utils::Logger::log(message, ::Utils::LogType::Error, "SetupParser::SetupParser");
            std::cin.get(); 
            std::terminate(); 
        }

        bool SetupParser::checkMainCastSession()
        {
            const std::optional<std::string> localIp = Common::Utils::getLocalIp(); 
            if (!localIp)
            {
                ::Utils::Logger::log("Common::Utils::getLocalIp was std::nullopt!", ::Utils::LogType::Error, "SetupParser::checkMainCastSession");
                return false;
            }

            bool hasValidMainCastPair = false;
            for (std::uint32_t i = 1; i <= 9; ++i)
            {
                const std::string mainServerSection = "MainServer_" + std::to_string(i);
                const std::string castServerSection = "CastServer_" + std::to_string(i);

                if (m_iniFile.contains(mainServerSection) && m_iniFile.contains(castServerSection))
                {
                    ini::IniSection& mainServer = m_iniFile[mainServerSection];
                    std::string mainIp = mainServer["Ip"].as<std::string>();
                    std::uint32_t mainPort = mainServer["Port"].as<std::uint32_t>();
                    std::uint32_t mainIpcPort = mainServer["IpcPort"].as<std::uint32_t>();
                    std::string mainLocalIp = mainServer["LocalIp"].as<std::string>(); 
                    bool isPublic = mainServer["IsPublic"].as<bool>();

                    if (mainIp.empty() || mainPort == 0 || mainIpcPort == 0)
                    {
                        ::Utils::Logger::log(mainServerSection + " has wrong data in config.ini", ::Utils::LogType::Error, "SetupParser::checkMainCastSession");
                        return false;
                    }

                    ini::IniSection& castServer = m_iniFile[castServerSection];
                    std::string castIp = castServer["Ip"].as<std::string>();
                    std::uint32_t castPort = castServer["Port"].as<std::uint32_t>();
                    std::uint32_t castIpcPort = castServer["IpcPort"].as<std::uint32_t>();
                    std::string castLocalIp = castServer["LocalIp"].as<std::string>();

                    if (castIp.empty() || castPort == 0 || castIpcPort == 0)
                    {
                        ::Utils::Logger::log(castServerSection + " has wrong data in config.ini", ::Utils::LogType::Error, "SetupParser::checkMainCastSession");
                        return false;
                    }

                    bool isMainLocalIpMatching = (mainLocalIp == *localIp);
                    bool isCastLocalIpMatching = (castLocalIp == *localIp);

                    if ((isMainLocalIpMatching && !isCastLocalIpMatching) || (!isMainLocalIpMatching && isCastLocalIpMatching))
                    {
                        ::Utils::Logger::log("MainServer and CastServer LocalIps don't match the current local IP", ::Utils::LogType::Error, 
                            "SetupParser::checkMainCastSession");
                        return false;
                    }

                    hasValidMainCastPair = true;
                }
                else if ((m_iniFile.contains(mainServerSection) && !m_iniFile.contains(castServerSection)) ||
                    (!m_iniFile.contains(mainServerSection) && m_iniFile.contains(castServerSection)))
                {
                    ::Utils::Logger::log("No corresponding MainServer/CastServer pair inside config.ini", ::Utils::LogType::Error, "SetupParser::checkMainCastSession");
                    return false;
                }
            }

            return hasValidMainCastPair;
        }

        bool SetupParser::checkAuthSection()
        {
            if (!m_iniFile.contains("AuthServer"))
            {
                ::Utils::Logger::log("Missing 'AuthServer' section in config.ini", ::Utils::LogType::Error, "SetupParser::checkAuthSection");
                return false;
            }

            ini::IniSection& authServer = m_iniFile["AuthServer"];
            if (!authServer.contains("Ip") || authServer["Ip"].as<std::string>().empty())
            {
                ::Utils::Logger::log("Missing/empty Ip in 'AuthServer' section", ::Utils::LogType::Error, "SetupParser::checkAuthSection");
                return false;
            }

            if (!authServer.contains("Port") || authServer["Port"].as<std::uint32_t>() == 0)
            {
                ::Utils::Logger::log("Missing or invalid Port in 'AuthServer' section", ::Utils::LogType::Error, "SetupParser::checkAuthSection");
                return false;
            }
            return true;
        }

        bool SetupParser::checkDatabaseConfig()
        {
            const std::string databaseSection = "Database";

            if (!m_iniFile.contains(databaseSection))
            {
                ::Utils::Logger::log("[Database] section missing in config.ini", ::Utils::LogType::Error, "SetupParser::checkDatabaseConfig");
                return false;
            }

            ini::IniSection& database = m_iniFile[databaseSection];
            const std::string passwordEnv = database["PasswordEnvironmentName"].as<std::string>();

            if (database["LocalIp"].as<std::string>().empty() || database["Ip"].as<std::string>().empty()
                || database["Port"].as<std::uint32_t>() == 0 || database["DatabaseName"].as<std::string>().empty()
                || database["Username"].as<std::string>().empty() || passwordEnv.empty())
            {
                ::Utils::Logger::log("Invalid database configuration in config.ini", ::Utils::LogType::Error, "SetupParser::checkDatabaseConfig");
                return false;
            }

            if (!std::getenv(passwordEnv.c_str()))
            {
                ::Utils::Logger::log("Environment variable " + passwordEnv + " has not been set!", ::Utils::LogType::Error, "SetupParser::checkDatabaseConfig");
                return false;
            }
            return true;
        }

        bool SetupParser::checkWebsiteConfig()
        {
            if (!m_iniFile.contains("Website"))
            {
                ::Utils::Logger::log("[Website] section missing in config.ini", ::Utils::LogType::Error, "SetupParser::checkWebsiteConfig");
                return false;
            }

            ini::IniSection& website = m_iniFile["Website"];

            if (website["Ip"].as<std::string>().empty() || website["Port"].as<std::uint32_t>() == 0)
            {
                ::Utils::Logger::log("Invalid website configuration in config.ini", ::Utils::LogType::Error, "SetupParser::checkWebsiteConfig");
                return false;
            }

            return true;
        }

        bool SetupParser::checkClientConfig()
        {
            if (!m_iniFile.contains("Client"))
            {
                ::Utils::Logger::log("[Client] section missing in config.ini", ::Utils::LogType::Error, "SetupParser::checkClientConfig");
                return false;
            }

            ini::IniSection& client = m_iniFile["Client"];

            const std::string& versionStr = client["ClientVersion"].as<std::string>();

            const std::regex versionRegex(R"(^\d\.\d\.\d$)");
            if (!std::regex_match(versionStr, versionRegex))
            {
                ::Utils::Logger::log("Invalid ClientVersion format - expected X.X.X with one digit per part (e.g., 1.2.3)",
                    ::Utils::LogType::Error, "SetupParser::checkClientConfig");
                return false;
            }

            return true;
        }


        bool SetupParser::sanityCheck()
        {
            return checkAuthSection() && checkMainCastSession() && checkDatabaseConfig() && checkWebsiteConfig() && checkClientConfig();
        }


        AuthSetup SetupParser::getAuthSetupImpl()
        {
            AuthSetup auth;
            auth.ip = m_iniFile["AuthServer"]["Ip"].as<std::string>();
            auth.port = m_iniFile["AuthServer"]["Port"].as<std::uint32_t>();
            return auth;
        }

        ClientSetup SetupParser::getClientSetupImpl()
        {
            ClientSetup client;

            const std::string& versionStr = m_iniFile["Client"]["ClientVersion"].as<std::string>();
            const std::regex versionRegex(R"(^\d\.\d\.\d$)");

            if (!std::regex_match(versionStr, versionRegex))
            {
                ::Utils::Logger::log("Invalid ClientVersion format. Expected X.X.X with one digit per part (e.g., 1.2.3)",
                    ::Utils::LogType::Error, "SetupParser::getClientSetupImpl");
                return client; 
            }

            client.version1 = versionStr[0] - '0'; 
            client.version2 = versionStr[2] - '0'; 
            client.version3 = versionStr[4] - '0'; 

            return client;
        }


        std::optional<MainSetup> SetupParser::getSelfMainServerInfoImpl()
        {
            auto mainServersInfoOpt = getMainServersInfoImpl();
            if (!mainServersInfoOpt) return std::nullopt;

            for (const auto mainServerInfo : *mainServersInfoOpt)
            {
                if (mainServerInfo.localIp == m_localIp)
                {
                    return mainServerInfo;
                }
            }
            return std::nullopt;
        }

        std::optional<CastSetup> SetupParser::getSelfCastServerInfoImpl()
        {
            auto castServersInfoOpt = getCastServersInfoImpl();
            if (!castServersInfoOpt)
                return std::nullopt;

            std::uint32_t selfServerNumber = m_selfMainInfo.serverNumber;

            for (const auto& castServerInfo : *castServersInfoOpt)
            {
                if (castServerInfo.serverNumber == selfServerNumber)
                {
                    return castServerInfo;
                }
            }

            return std::nullopt;
        }


        std::optional<std::vector<MainSetup>> SetupParser::getMainServersInfoImpl()
        {
            std::vector<MainSetup> mainServers;
            for (auto& sectionPair : m_iniFile)
            {
                const std::string& sectionName = sectionPair.first;
                ini::IniSection& section = sectionPair.second;

                if (sectionName.find("MainServer_") == 0)
                {
                    MainSetup main;
                    main.ip = section["Ip"].as<std::string>();
                    main.port = section["Port"].as<std::uint32_t>();
                    main.ipcPort = section["IpcPort"].as<std::uint32_t>();
                    main.serverNumber = std::stoi(sectionName.substr(11));
                    main.localIp = section["LocalIp"].as<std::string>();
                    main.isPublic = section["IsPublic"].as<bool>();
                  
                    mainServers.push_back(main);
                }
            }
            if (mainServers.empty()) return std::nullopt;
            return mainServers;
        }

        std::optional<std::vector<CastSetup>> SetupParser::getCastServersInfoImpl()
        {
            std::vector<CastSetup> castServers;
            for (auto& sectionPair : m_iniFile)
            {
                const std::string& sectionName = sectionPair.first;
                ini::IniSection& section = sectionPair.second;

                if (sectionName.find("CastServer_") == 0)
                {
                    CastSetup cast;
                    cast.ip = section["Ip"].as<std::string>();
                    cast.port = section["Port"].as<std::uint32_t>();
                    cast.ipcPort = section["IpcPort"].as<std::uint32_t>();
                    cast.serverNumber = std::stoi(sectionName.substr(11));
                    cast.localIp = section["LocalIp"].as<std::string>();
                    castServers.push_back(cast);
                }
            }
            if (castServers.empty()) return std::nullopt;
            return castServers;
        }

        DatabaseSetup SetupParser::getDatabaseSetupImpl()
        {
            DatabaseSetup db;
            db.ip = m_iniFile["Database"]["Ip"].as<std::string>();
            db.port = m_iniFile["Database"]["Port"].as<std::uint32_t>();
            db.databaseName = m_iniFile["Database"]["DatabaseName"].as<std::string>();
            db.username = m_iniFile["Database"]["Username"].as<std::string>();

            std::string envVarName = m_iniFile["Database"]["PasswordEnvironmentName"].as<std::string>();
            const char* envPassword = std::getenv(envVarName.c_str());
            db.password = envPassword ? envPassword : "";

            return db;
        }

        WebsiteSetup SetupParser::getWebsiteSetupImpl()
        {
            WebsiteSetup website;
            website.ip = m_iniFile["Website"]["Ip"].as<std::string>();
            website.port = m_iniFile["Website"]["Port"].as<std::uint32_t>();

            return website;
        }
    };
}

