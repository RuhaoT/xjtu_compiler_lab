#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <memory>
#include <string>

#define GTEST_LOGGER_NAME "gtest_logger"

std::shared_ptr<spdlog::logger> init_fixture_logger(const std::string &log_filename);

void release_fixture_logger();

void add_test_start_log(const std::string &test_name);
void add_test_end_log(const std::string &test_name);

// a global environment for storing the logger pointer
class LoggingEnvironment : public ::testing::Environment
{
public:
    static std::shared_ptr<spdlog::logger> logger;
};