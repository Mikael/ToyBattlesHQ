#ifndef CUSTOM_LOGGER_H
#define CUSTOM_LOGGER_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <string>
#include <sstream>
#include <format>
#include <memory>
#include <mutex>

namespace Utils
{
    enum class LogType
    {
        Info, Error, Warning, Normal
    };

    namespace LogColors
    {
        const inline std::string Reset = "\033[0m";
        const inline std::string Info = "\033[32m";
        const inline std::string Error = "\033[31m";
        const inline std::string Normal = "\033[37m";
        const inline std::string Warning = "\033[33m";
    }

    class Logger 
    {
    private:
        static inline bool m_loggingEnabled = true;
        static inline std::once_flag initFlag;

        static void enableAnsiEscapeCodes();
       
    public:
        static void enableLogging()
        {
            m_loggingEnabled = true;
        }

        static void disableLogging()
        {
            m_loggingEnabled = false;
        }

        static void log(const std::string& message, LogType type = LogType::Normal, const std::string& functionName = "");

        static void newline()
        {
            printToConsole("", LogType::Normal);
        }

    private:
        static void printToConsole(const std::string& message, LogType type);
        static std::string getCurrentDateTime();
        static std::string logTypeToString(LogType type);
    };
}

#endif
