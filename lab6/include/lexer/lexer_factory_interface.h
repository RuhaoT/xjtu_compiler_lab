#ifndef FACTORY_INTERFACE_H
#define FACTORY_INTERFACE_H
#include <string>
#include <memory>
#include "lexer_interface.h"

class LexerFactoryInterface {
public:
    virtual ~LexerFactoryInterface() = default;

    virtual std::unique_ptr<LexerInterface> CreateLexer(const std::string& lexer_type, const std::string& general_config, const std::string& specific_config) = 0;
};

#endif // !FACTORY_INTERFACE_H
