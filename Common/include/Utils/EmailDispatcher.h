#ifndef EMAIL_SCHEDULER_H
#define EMAIL_SCHEDULER_H

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <string>
#include <functional>
#include <atomic>
#include <iostream>
#include <Utils.h>

namespace Common
{
    namespace Utils
    {
        enum class EmailType
        {
            Regular,
            Alert
        };

        struct EmailJob
        {
            EmailType type = EmailType::Regular;  
            std::vector<std::string> recipients;  
            std::string subject;
            std::string body;
            std::function<void()> onSuccess;
            std::function<void()> onFailure;
        };

        class EmailDispatcher
        {
        private:
            std::queue<EmailJob> jobQueue;
            std::mutex queueMutex;
            std::condition_variable cv;
            std::thread workerThread;
            std::atomic<bool> stopFlag = false;

            void workerLoop()
            {
                while (!stopFlag)
                {
                    EmailJob job;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        cv.wait(lock, [this]() { return stopFlag || !jobQueue.empty(); });

                        if (stopFlag && jobQueue.empty())
                            break;

                        job = std::move(jobQueue.front());
                        jobQueue.pop();
                    }

                    bool success = false;
                    try
                    {
                        if (job.type == EmailType::Alert)
                        {
                            success = sendEmailAlert(job.subject, job.body);
                        }
                        else
                        {
                            success = sendEmails(job.recipients, job.subject, job.body);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        ::Utils::Logger::log("[EmailDispatcher] Exception: " + std::string(e.what()), ::Utils::LogType::Error, "EmailDispatcher");
                        success = false;
                    }
                    catch (...)
                    {
                        ::Utils::Logger::log("[EmailDispatcher] Unknown exception", ::Utils::LogType::Error, "EmailDispatcher");
                        success = false;
                    }

                    if (success && job.onSuccess) job.onSuccess();
                    if (!success && job.onFailure) job.onFailure();
                }
            }

            void enqueueEmail(EmailType type, const std::vector<std::string>& recipients, const std::string& subject, const std::string& body,
                std::function<void()> onSuccess, std::function<void()> onFailure)
            {
                if (type == EmailType::Regular && recipients.empty())
                {
                    ::Utils::Logger::log("[EmailDispatcher] Empty recipients list for Regular email", ::Utils::LogType::Warning, "EmailDispatcher");
                    return;
                }

                EmailJob job{ type, recipients, subject, body, onSuccess, onFailure };
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    jobQueue.push(std::move(job));
                }
                cv.notify_one();
            }

        public:
            EmailDispatcher()
            {
                workerThread = std::thread(&EmailDispatcher::workerLoop, this);
            }

            ~EmailDispatcher()
            {
                shutdown();
            }

            void sendEmailAsync(const std::vector<std::string>& recipients, const std::string& subject, const std::string& body,
                std::function<void()> onFailure, std::function<void()> onSuccess = nullptr)
            {
                enqueueEmail(EmailType::Regular, recipients, subject, body, onSuccess, onFailure);
            }

            void sendAlertAsync(const std::string& subject, const std::string& body, std::function<void()> onFailure,
                std::function<void()> onSuccess = nullptr)
            {
                enqueueEmail(EmailType::Alert, {}, subject, body, onSuccess, onFailure);
            }

            void shutdown()
            {
                stopFlag = true;
                cv.notify_all();
                if (workerThread.joinable())
                    workerThread.join();
            }
        };

    } // namespace Utils
} // namespace Common

#endif