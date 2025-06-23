#include "gtest/gtest.h"    
#include "spdlog/spdlog.h"
#include "testing_utils.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "yaml_cfg_loader.h"
#include "lr_parsing_model.h"
#include "itemset_generator.h"
#include <memory>
#include <string>
#include <stdexcept>
#include <filesystem>

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
    else {
        // the last logger is not dropped properly, throw an error
        std::string error_message = "Logger with name '" + std::string(GTEST_LOGGER_NAME) + "' already exists. Please release the logger before initializing a new one.";
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
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

std::shared_ptr<spdlog::logger> get_fixture_logger() {
    // get the logger
    auto logger = spdlog::get(GTEST_LOGGER_NAME);
    if (!logger) {
        std::string error_message = "Logger with name '" + std::string(GTEST_LOGGER_NAME) + "' does not exist. Please initialize the logger before using it.";
        spdlog::error(error_message);
        throw std::runtime_error(error_message);
    }
    return logger;
}

void set_fixture_logger_level(spdlog::level::level_enum level) {
    // get the logger
    auto logger = get_fixture_logger();
    // set the log level
    logger->set_level(level);
}

void add_test_start_log(const std::string &test_name) {
    // get the logger
    auto logger = get_fixture_logger();
    // log the test name
    logger->info("--------------------------------------------------");
    logger->info("Running test: {}", test_name);
    logger->info("--------------------------------------------------");
}

void add_test_end_log(const std::string &test_name) {
    // get the logger
    auto logger = get_fixture_logger();
    // log the test name
    logger->info("--------------------------------------------------");
    logger->info("Finished test: {}", test_name);
    logger->info("--------------------------------------------------");
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

std::vector<std::string> get_dir_file_with_prefix(const std::string &dir, const std::string &prefix)
{
    std::vector<std::string> files;
    for (const auto &entry : std::filesystem::directory_iterator(dir))
    {
        if (entry.is_regular_file() && entry.path().filename().string().find(prefix) == 0)
        {
            files.push_back(entry.path().string());
        }
    }
    return files;
}

// get the base name of a file from its path
std::string get_file_name(const std::string &file_path)
{
    return std::filesystem::path(file_path).filename().string();
}

// get the base name without extension of a file from its path
std::string get_base_name_without_extension(const std::string &file_path)
{
    return std::filesystem::path(file_path).stem().string();
}