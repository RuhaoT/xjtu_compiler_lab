#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "testing_utils.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "yaml_cfg_loader.h"
#include "lr_parsing_model.h"
#include "itemset_generator.h"
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

cfg_model::CFG load_test_cfg(const std::string &cfg_file_path)
{
    try
    {
        // Load the CFG from the YAML file
        YAML_CFG_Loader cfg_loader;
        cfg_model::CFG cfg = cfg_loader.LoadCFG(cfg_file_path);
        return cfg;
    }
    catch(const std::exception& e)
    {
        std::string error_message = "Error loading test CFG: " + std::string(e.what());
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
}

lr_parsing_model::ItemSet generate_test_itemset(const std::string &cfg_file_path)
{
    try
    {
        // Load the CFG from the YAML file
        cfg_model::CFG cfg = load_test_cfg(cfg_file_path);
        // generate the itemset
        ItemSetGenerator itemset_generator;
        lr_parsing_model::ItemSet itemset = itemset_generator.generate_item_set(cfg);
        return itemset;
    }
    catch(const std::exception& e)
    {
        std::string error_message = "Error generating test itemset: " + std::string(e.what());
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
    
}