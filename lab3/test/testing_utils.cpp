#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "testing_utils.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <memory>
#include <string>

// register the global environment
std::shared_ptr<spdlog::logger> LoggingEnvironment::logger = nullptr;
::testing::Environment *const env = ::testing::AddGlobalTestEnvironment(new LoggingEnvironment());

std::shared_ptr<spdlog::logger> init_fixture_logger(const std::string &log_filename) {
    // remove the file if it already exists
    std::remove(log_filename.c_str());
    // create a basic file logger, for now just store the logs in the current directory
    
    auto logger = spdlog::get(GTEST_LOGGER_NAME);
    if (!logger) {
        logger = std::make_shared<spdlog::logger>(GTEST_LOGGER_NAME, std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_filename, true));
        // set the log level to debug
        logger->set_level(spdlog::level::debug);
        // set the log pattern: level + message
        logger->set_pattern("[%^%l%$] %v");
        // set the logger to flush after each log
        logger->flush_on(spdlog::level::debug);
        // register the logger
        spdlog::register_logger(logger);
        // !!! set the logger as the default logger
        spdlog::set_default_logger(logger);
    }

    return logger;
}

void release_fixture_logger() {
    // flush and drop the logger at the tear down of each fixture
    // for the future test fixtures
    auto logger = spdlog::get(GTEST_LOGGER_NAME);
    // flush to the file
    logger->flush();
    // drop the logger
    spdlog::drop(GTEST_LOGGER_NAME);
    // reset the logger
    logger.reset();
}

void add_test_start_log(const std::string &test_name) {
    // log the test name
    LoggingEnvironment::logger->info("--------------------------------------------------");
    LoggingEnvironment::logger->info("Running test: {}", test_name);
    LoggingEnvironment::logger->info("--------------------------------------------------");
}

void add_test_end_log(const std::string &test_name) {
    // log the test name
    LoggingEnvironment::logger->info("--------------------------------------------------");
    LoggingEnvironment::logger->info("Finished test: {}", test_name);
    LoggingEnvironment::logger->info("--------------------------------------------------");
}