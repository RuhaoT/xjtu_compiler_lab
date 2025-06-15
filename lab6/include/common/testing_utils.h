#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "yaml_cfg_loader.h"
#include "lr_parsing_model.h"
#include <memory>
#include <string>

#define GTEST_LOGGER_NAME "gtest_logger"

std::shared_ptr<spdlog::logger> init_fixture_logger(const std::string &log_filename);

void release_fixture_logger();

void set_fixture_logger_level(spdlog::level::level_enum level);

void add_test_start_log(const std::string &test_name);
void add_test_end_log(const std::string &test_name);

cfg_model::CFG load_test_cfg(const std::string &filename);
lr_parsing_model::ItemSet generate_test_itemset(const std::string &cfg_file_path);

std::shared_ptr<spdlog::logger> get_fixture_logger();

// a global environment for storing the logger pointer
class LoggingEnvironment : public ::testing::Environment
{
public:
    static std::shared_ptr<spdlog::logger> logger;
};