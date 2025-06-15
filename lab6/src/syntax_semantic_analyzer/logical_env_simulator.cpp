#include "syntax_semantic_analyzer/logical_env_simulator.h"
#include "interm_code/interm_code_model.h"
#include "spdlog/spdlog.h"
#include "symbol_table.h"
#include "syntax_semantic_model.h"
#include <unordered_map>
#include <map>
#include <vector>

LogicalEnvSimulator::LogicalEnvSimulator(const SymbolTable& symbol_table)
    : symbol_table(symbol_table) {}

void LogicalEnvSimulator::init() {
    scope_ids.clear();
    scope_treg_usage.clear();
    scope_var_reg_map.clear();
    scope_arr_reg_map.clear();
    scope_label_map.clear();
    label_to_func_map.clear();
    total_labels.clear();
    stack_usage = 0;
    memory_usage = 0;
    interm_code_list.clear();

    // Initialize scope information
    std::set<int> initial_scope_ids;
    for (const auto& symbol : symbol_table.symbols) {
        initial_scope_ids.insert(symbol.scope_id);
        // if function, also add sub scope ID
        if (symbol.symbol_type == syntax_semantic_model::SymbolType::Function && symbol.direct_child_scope.has_value()) {
            initial_scope_ids.insert(symbol.direct_child_scope.value());
        }
    }
    // also include the global scope ID -1
    initial_scope_ids.insert(-1); // Global scope ID
    scope_ids = std::unordered_set<int>(initial_scope_ids.begin(), initial_scope_ids.end());
    for (const auto& scope_id : scope_ids) {
        scope_treg_usage[scope_id] = 0; // Initialize T register usage for each scope
        scope_var_reg_map[scope_id] = std::unordered_map<std::string, interm_code_model::Register>();
        scope_arr_reg_map[scope_id] = std::unordered_map<std::string, interm_code_model::Register>(); // Initialize label map for each scope
        spdlog::debug("Initialized scope ID {} with T register usage 0.", scope_id);
    }


    // 1. Initialize all functions & arguments in the symbol table
    for (const auto& symbol : symbol_table.symbols) {
        if (symbol.symbol_type == syntax_semantic_model::SymbolType::Function) {
            // Register the function label
            std::string func_label = "L" + symbol.symbol_name; // Example label format
            register_func_label(symbol.symbol_name, func_label);
            if (!symbol.direct_child_scope.has_value() || !symbol.arg_list.has_value()) {
                spdlog::error("Function {} does not have a valid child scope or argument list.", symbol.symbol_name);
                throw std::runtime_error("Function must have a valid child scope and argument list.");
            }
            int sub_scope_id = symbol.direct_child_scope.value();
            auto args = symbol.arg_list.value();
            for (const auto& arg : args) {
                // Register each argument to the corresponding scope and allocate a T register for it
                interm_code_model::Register arg_reg = get_new_treg(sub_scope_id);
                // find arg entry in symbol table
                auto arg_entry = symbol_table.findSymbolInScope(arg, sub_scope_id);
                if (!arg_entry.has_value()) {
                    spdlog::error("Argument {} not found in symbol table for function {}", arg, symbol.symbol_name);
                    throw std::runtime_error("Argument not found in symbol table.");
                }
                if (arg_entry->symbol_type == syntax_semantic_model::SymbolType::Variable) {
                    register_var(arg, sub_scope_id, arg_reg);
                } else if (arg_entry->symbol_type == syntax_semantic_model::SymbolType::Array) {
                    register_arr(arg, sub_scope_id, arg_reg);
                } else {
                    spdlog::error("Argument {} is not a valid variable or array.", arg);
                    throw std::runtime_error("Invalid argument type.");
                }
            }
        }
    }

    // 2. Resgister all normal variables
    for (const auto& symbol : symbol_table.symbols) {
        if (symbol.symbol_type == syntax_semantic_model::SymbolType::Variable) {
            int scope_id = symbol.scope_id;
            // check if it has been registered in the symbol table, if not, get a new T register for it
            if (scope_var_reg_map[scope_id].find(symbol.symbol_name) == scope_var_reg_map[scope_id].end()) {
                interm_code_model::Register var_reg = get_new_treg(scope_id);
                register_var(symbol.symbol_name, scope_id, var_reg);
            } else {
                spdlog::debug("Variable {} already registered in scope ID {}", symbol.symbol_name, scope_id);
            }
        }
        else if (symbol.symbol_type == syntax_semantic_model::SymbolType::Array) {
            int scope_id = symbol.scope_id;
            // check if it has been registered in the symbol table, if not, get a new T register for it
            if (scope_arr_reg_map[scope_id].find(symbol.symbol_name) == scope_arr_reg_map[scope_id].end()) {
                interm_code_model::Register arr_reg = get_new_treg(scope_id);
                register_arr(symbol.symbol_name, scope_id, arr_reg);
            } else {
                spdlog::debug("Array {} already registered in scope ID {}", symbol.symbol_name, scope_id);
            }
        }
    }
    // initialization complete
    spdlog::info("Logical environment simulator initialized with {} scopes.", scope_ids.size());
}

void LogicalEnvSimulator::check_scope_id(int scope_id) {
    if (scope_ids.find(scope_id) == scope_ids.end()) {
        spdlog::error("Scope ID {} does not exist.", scope_id);
        throw std::runtime_error("Scope ID does not exist.");
    }
}

void LogicalEnvSimulator::check_symbol_table_var(const std::string& var_name, int scope_id) {
    if (!symbol_table.symbolExists(var_name, scope_id)) {
        spdlog::error("Variable {} does not exist in scope ID {}", var_name, scope_id);
        throw std::runtime_error("Variable does not exist in scope.");
    }
}

void LogicalEnvSimulator::check_symbol_table_arr(const std::string& arr_name, int scope_id) {
    if (!symbol_table.symbolExists(arr_name, scope_id)) {
        spdlog::error("Array {} does not exist in scope ID {}", arr_name, scope_id);
        throw std::runtime_error("Array does not exist in scope.");
    }
}

void LogicalEnvSimulator::check_symbol_table_func(const std::string& func_name) {
    if (!symbol_table.symbolExists(func_name, 0)) { // -1 indicates global scope
        spdlog::error("Function {} does not exist in any scope.", func_name);
        throw std::runtime_error("Function does not exist.");
    }
}

void LogicalEnvSimulator::check_reg_mapping_var(const std::string& var_name, int scope_id) {
    if (scope_var_reg_map.find(scope_id) == scope_var_reg_map.end() ||
        scope_var_reg_map[scope_id].find(var_name) == scope_var_reg_map[scope_id].end()) {
        spdlog::error("Variable {} is not mapped to a register in scope ID {}", var_name, scope_id);
        throw std::runtime_error("Variable is not mapped to a register.");
    }
}

void LogicalEnvSimulator::check_reg_mapping_arr(const std::string& arr_name, int scope_id) {
    if (scope_arr_reg_map.find(scope_id) == scope_arr_reg_map.end() ||
        scope_arr_reg_map[scope_id].find(arr_name) == scope_arr_reg_map[scope_id].end()) {
        spdlog::error("Array {} is not mapped to a register in scope ID {}", arr_name, scope_id);
        throw std::runtime_error("Array is not mapped to a register.");
    }
}
void LogicalEnvSimulator::check_func_mapped_to_label(const std::string& func_name) {
    if (label_to_func_map.find(func_name) == label_to_func_map.end()) {
        spdlog::error("Function {} is not mapped to any label.", func_name);
        throw std::runtime_error("Function is not mapped to a label.");
    }
}

interm_code_model::Register LogicalEnvSimulator::get_new_treg(int scope_id) {
    try {
        // check if the scope exists
        check_scope_id(scope_id);
        // get the current T register usage for the scope
        scope_treg_usage[scope_id]++;
        int treg_usage = scope_treg_usage[scope_id];
        // create a new T register
        interm_code_model::Register treg(interm_code_model::RegisterType::TYPE_T_GENERAL, treg_usage);
        spdlog::debug("Allocated new T register {} for scope ID {}", treg.toString(), scope_id);
        return treg;
    } catch (const std::exception& e) {
        spdlog::error("Error allocating new T register for scope ID {}: {}", scope_id, e.what());
        throw;
    }
}

std::string LogicalEnvSimulator::get_new_temp_label(int scope_id) {
    try {
        // check if the scope exists
        check_scope_id(scope_id);
        // generate a new temporary label: L<scope_id>_<label_count>
        int label_count = scope_label_map.count(scope_id);
        std::string new_label = "L" + std::to_string(scope_id) + "_" + std::to_string(label_count);
        // check if the label already exists
        if (total_labels.find(new_label) != total_labels.end()) {
            spdlog::error("Temporary label {} already exists in scope ID {}", new_label, scope_id);
            throw std::runtime_error("Temporary label already exists.");
        }
        // register the new label in the scope as well as globally
        scope_label_map.emplace(scope_id, new_label);
        total_labels.insert(new_label);
        spdlog::debug("Generated new temporary label {} for scope ID {}", new_label, scope_id);
        return new_label;
    } catch (const std::exception& e) {
        spdlog::error("Error generating new temporary label for scope ID {}: {}", scope_id, e.what());
        throw;
    }
}

interm_code_model::Register LogicalEnvSimulator::get_var_reg(const std::string& var_name, int scope_id) {
    try {
        // check if the scope exists
        check_scope_id(scope_id);
        // check if the variable exists in the symbol table
        check_symbol_table_var(var_name, scope_id);
        // get the register for the variable
        check_reg_mapping_var(var_name, scope_id);
        auto& reg_map = scope_var_reg_map.at(scope_id);
        return reg_map.at(var_name);
    } catch (const std::exception& e) {
        spdlog::error("Error getting register for variable {} in scope ID {}: {}", var_name, scope_id, e.what());
        throw;
    }
}

interm_code_model::Register LogicalEnvSimulator::get_arr_reg(const std::string& arr_name, int scope_id) {
    try {
        // check if the scope exists
        check_scope_id(scope_id);
        // check if the array exists in the symbol table
        check_symbol_table_arr(arr_name, scope_id);
        // get the register for the array
        check_reg_mapping_arr(arr_name, scope_id);
        auto& reg_map = scope_arr_reg_map[scope_id];
        return reg_map[arr_name];
    } catch (const std::exception& e) {
        spdlog::error("Error getting register for array {} in scope ID {}: {}", arr_name, scope_id, e.what());
        throw;
    }
}

std::string LogicalEnvSimulator::get_func_label(const std::string& func_name) {
    try {
        // check if the function exists in the symbol table
        check_symbol_table_func(func_name);
        // check if the function is mapped to a label
        check_func_mapped_to_label(func_name);
        return label_to_func_map[func_name];
    } catch (const std::exception& e) {
        spdlog::error("Error getting label for function {}: {}", func_name, e.what());
        throw;
    }
}

void LogicalEnvSimulator::register_var(const std::string& var_name, int scope_id, interm_code_model::Register reg) {
    try {
        // check if the scope exists
        check_scope_id(scope_id);
        // check if the variable already mapped to a register
        try {
            check_reg_mapping_var(var_name, scope_id);
        }
        catch (const std::runtime_error&) {
            // If the variable is not mapped, we can proceed to register it
            scope_var_reg_map[scope_id].emplace(var_name, reg);
            spdlog::debug("Registered variable {} with register {} in scope ID {}", var_name, reg.toString(), scope_id);
            return;
        }
        // If the variable is already mapped, we log an error
        spdlog::error("Variable {} is already registered in scope ID {}", var_name, scope_id);
        throw std::runtime_error("Variable is already registered in this scope.");
    } catch (const std::exception& e) {
        spdlog::error("Error registering variable {} in scope ID {}: {}", var_name, scope_id, e.what());
        throw;
    }
}

void LogicalEnvSimulator::register_arr(const std::string& arr_name, int scope_id, interm_code_model::Register reg) {
    try {
        // check if the scope exists
        check_scope_id(scope_id);
        // check if the array already mapped to a register
        try {
            check_reg_mapping_arr(arr_name, scope_id);
        }
        catch (const std::runtime_error&) {
            // If the array is not mapped, we can proceed to register it
            scope_arr_reg_map[scope_id].emplace(arr_name, reg);
            spdlog::debug("Registered array {} with register {} in scope ID {}", arr_name, reg.toString(), scope_id);
            return;
        }
        // If the array is already mapped, we log an error
        spdlog::error("Array {} is already registered in scope ID {}", arr_name, scope_id);
        throw std::runtime_error("Array is already registered in this scope.");
    } catch (const std::exception& e) {
        spdlog::error("Error registering array {} in scope ID {}: {}", arr_name, scope_id, e.what());
        throw;
    }
}

void LogicalEnvSimulator::register_func_label(const std::string& func_name, const std::string& label) {
    try {
        // check if the function exists in the symbol table
        check_symbol_table_func(func_name);
        // check if the label is already registered
        if (total_labels.find(label) != total_labels.end()) {
            spdlog::error("Label {} is already registered.", label);
            throw std::runtime_error("Label is already registered.");
        }
        // register the function label
        label_to_func_map[func_name] = label;
        total_labels.insert(label);
        spdlog::debug("Registered function {} with label {}", func_name, label);
    } catch (const std::exception& e) {
        spdlog::error("Error registering function {} with label {}: {}", func_name, label, e.what());
        throw;
    }
}

std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> LogicalEnvSimulator::save_scope_state(int scope_id) {
    try {
        // step 1: Check if the scope ID exists
        check_scope_id(scope_id);
        // step 2: get the current scope's T register usage
        int treg_usage = scope_treg_usage[scope_id];
        // step 3: generate T register store codes
        std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> save_codes;
        for (int i = 1; i <= treg_usage; ++i) {
            int curr_stack_top = stack_usage;
            interm_code_model::LogicalAddress store_addr(interm_code_model::LogicalMemSpaceType::STACK, curr_stack_top);
            interm_code_model::Register treg(interm_code_model::RegisterType::TYPE_T_GENERAL, i);
            auto store_code = std::make_shared<interm_code_model::IntermediateCode>(
                interm_code_model::OperationType::STORE, treg, store_addr);
            save_codes.push_back(store_code);
            stack_usage += 1; // Increment stack usage for each T register stored
            spdlog::debug("Generated store code for T register {} at stack address {}", treg.toString(), store_addr.toString());
        }
        // step 4: store the RA register to the stack
        interm_code_model::LogicalAddress ra_store_addr(interm_code_model::LogicalMemSpaceType::STACK, stack_usage);
        interm_code_model::Register ra_reg(interm_code_model::RegisterType::TYPE_RA, 0); // Only one RA register
        auto ra_store_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::STORE, ra_reg, ra_store_addr);
        save_codes.push_back(ra_store_code);
        stack_usage += 1; // Increment stack usage for RA register stored
        spdlog::debug("Generated store code for RA register at stack address {}", ra_store_addr.toString());
        return save_codes;
    }
    catch (const std::exception& e) {
        spdlog::error("Error saving scope state for scope ID {}: {}", scope_id, e.what());
        throw;
    }
}

std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> LogicalEnvSimulator::restore_scope_state(int scope_id) {
    try {
        spdlog::debug("Restoring scope state for scope ID {}", scope_id);
        std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> restore_codes;
        // step 1: Check if the scope ID exists
        check_scope_id(scope_id);
        // step 2: get the current scope's T register usage
        int treg_usage = scope_treg_usage[scope_id];
        // step 3: load the RA register from stack top
        stack_usage -= 1; // Decrement stack usage for RA register
        interm_code_model::LogicalAddress ra_load_addr(interm_code_model::LogicalMemSpaceType::STACK, stack_usage);
        interm_code_model::Register ra_reg(interm_code_model::RegisterType::TYPE_RA, 0); // Only one RA register
        auto ra_load_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::LOAD, ra_load_addr, ra_reg);
        restore_codes.push_back(ra_load_code);
        spdlog::debug("Generated load code for RA register from stack address {}", ra_load_addr.toString());
        // step 4: load the T registers from stack
        for (int i = treg_usage; i >= 1; --i) {
            stack_usage -= 1; // Decrement stack usage for each T register
            interm_code_model::LogicalAddress treg_load_addr(interm_code_model::LogicalMemSpaceType::STACK, stack_usage);
            interm_code_model::Register treg(interm_code_model::RegisterType::TYPE_T_GENERAL, i);
            auto treg_load_code = std::make_shared<interm_code_model::IntermediateCode>(
                interm_code_model::OperationType::LOAD, treg_load_addr, treg);
            restore_codes.push_back(treg_load_code);
            spdlog::debug("Generated load code for T register {} from stack address {}", treg.toString(), treg_load_addr.toString());
        }
        // step 5: return the restore codes
        spdlog::debug("Restored {} T registers and RA register for scope ID {}", treg_usage, scope_id);
        return restore_codes;
    }
    catch (const std::exception& e) {
        spdlog::error("Error restoring scope state for scope ID {}: {}", scope_id, e.what());
        throw;
    }
}

std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> LogicalEnvSimulator::generate_func_header(int arg_count, const std::string &func_label) {
    try {
        spdlog::debug("Generating function header for function with label: {} and {} arguments", func_label, arg_count);
        // Generate the function header code
        std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> header_codes;
        // init as empty vector
        header_codes.clear();

        for (int i = 1; i <= arg_count; ++i) {
            // Allocate a new T register for each argument
            interm_code_model::Register arg_reg(interm_code_model::RegisterType::TYPE_T_GENERAL, i);
            interm_code_model::Register rarg_reg(interm_code_model::RegisterType::TYPE_R_GENERAL, i);
            // Create the intermediate code for argument assignment
            auto arg_code = std::make_shared<interm_code_model::IntermediateCode>(
                interm_code_model::OperationType::ASSIGN, arg_reg, rarg_reg);
            header_codes.push_back(arg_code);
            spdlog::debug("Generated function header code for argument {}: {}", i, arg_code->toString());
        }
        // !!! ADD a empty code at the start of the header codes, in case the function has no arguments
        auto empty_code = std::make_shared<interm_code_model::IntermediateCode>(
            interm_code_model::OperationType::EMPTY);
        header_codes.insert(header_codes.begin(), empty_code);
        spdlog::debug("Added empty code at the start of the function header codes.");
        // set the label to the first line of the function
        header_codes[0]->label = func_label;
        spdlog::debug("Generated function header with label: {}", func_label);
        return header_codes;
        
    } catch (const std::exception& e) {
        spdlog::error("Error generating function header: {}", e.what());
        throw;
    }
}

void LogicalEnvSimulator::add_intermediate_code_batch(const std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> &interm_code_batch) {
    try {
        // Add the batch of intermediate codes to the list
        interm_code_list.insert(interm_code_list.end(), interm_code_batch.begin(), interm_code_batch.end());
        spdlog::debug("Added {} intermediate codes to the list.", interm_code_batch.size());
    } catch (const std::exception& e) {
        spdlog::error("Error adding intermediate code batch: {}", e.what());
        throw;
    }
}

std::vector<std::shared_ptr<interm_code_model::IntermediateCode>> LogicalEnvSimulator::get_intermediate_code_list() const {
    return interm_code_list;
}