/* Setup prerequisites for the test environment, like spdlog, etc */

#include <gtest/gtest.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <iostream>

// Define the custom Environment class (can also be in a separate header)
class LoggingEnvironment : public ::testing::Environment
{
public:
    ~LoggingEnvironment() override {}

    // Called before any tests run.
    void SetUp() override
    {
        try
        {
            // Configure and initialize your logger (e.g., spdlog)
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("test_run_log.txt", true); // true = overwrite existing log
            auto logger = std::make_shared<spdlog::logger>("gtest_logger", file_sink);
            logger->set_pattern("[%l] %v"); // Set log format
            logger->set_level(spdlog::level::debug); // Set desired log level
            logger->flush_on(spdlog::level::debug);  // Ensure logs are written quickly, good for tests
            spdlog::set_default_logger(logger);      // Make it globally accessible via spdlog::* calls

            spdlog::info("================ Test Run Starting ================");
            spdlog::info("Logging Environment Initialized.");
        }
        catch (const spdlog::spdlog_ex &ex)
        {
            std::cerr << "FATAL: Log initialization failed in LoggingEnvironment::SetUp(): "
                      << ex.what() << std::endl;
            // You might want to abort if logging is critical
            GTEST_FAIL() << "Logging setup failed, cannot continue tests.";
            // exit(1); // Or just exit
        }
        catch (const std::exception &ex)
        {
            std::cerr << "FATAL: Non-spdlog exception during log initialization: " << ex.what() << std::endl;
            GTEST_FAIL() << "Generic exception during logging setup.";
            // exit(1);
        }
    }

    // Called after all tests have run.
    void TearDown() override
    {
        spdlog::info("Logging Environment Tearing Down.");
        spdlog::info("================= Test Run Finished =================");
        spdlog::shutdown(); // Cleanly shut down the logger (flushes buffers, etc.)
    }
};

// Register the environment with Google Test
testing::Environment *const env =
    testing::AddGlobalTestEnvironment(new LoggingEnvironment);