#include "lr_parsing_model.h"
#include "spdlog/spdlog.h"
#include <stdexcept>

namespace lr_parsing_model
{

    // Item member functions
    Item::operator std::string() const
    {
        std::string result = "[" + left_side_symbol.name + " -> ";
        for (const auto &s : sequence_already_parsed)
        {
            result += std::string(s) + " ";
        }
        result += " · ";
        for (const auto &s : sequence_to_parse)
        {
            result += " " + std::string(s);
        }
        result += "]";
        return result;
    }

    // LRParsingTable member functions
    bool LRParsingTable::add_action(const std::string &state, const cfg_model::symbol &symbol, const Action &action)
    {
        try
        {
            // check if the cell already exists
            if (check_cell_empty(state, symbol))
            {
                // add the state and symbol to the action table and return
                action_table[state][symbol].insert(action);
            }
            else
            {
                // check if the action already exists
                if (action_table[state][symbol].find(action) != action_table[state][symbol].end())
                {
                    spdlog::debug("Action already exists in action table for state {} and symbol {}: {}", state, std::string(symbol), action.action_type);
                    spdlog::debug("No need to add it again");
                    return false; // action already exists, do nothing
                }

                // if the action set is not empty, we have a conflict
                spdlog::warn("Conflict detected in action table for state {} and symbol {}: {}", state, std::string(symbol), action.action_type);
                action_table[state][symbol].insert(action);
            }
            spdlog::debug("Adding action to action table for state {} and symbol {}: {}", state, std::string(symbol), action_table[state][symbol].find(action) != action_table[state][symbol].end() ? action.action_type : "not found");
            return true;
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error adding action to action table: {}", e.what());
            throw;
        }
    }

    bool LRParsingTable::add_goto(const std::string &state, const cfg_model::symbol &symbol, const std::string &next_state)
    {
        try
        {
            // check if the cell already exists
            if (check_cell_empty(state, symbol))
            {
                // add the state and symbol to the goto table and return
                goto_table[state][symbol].insert(next_state);
            }
            else
            {
                // check if the action already exists
                if (goto_table[state][symbol].find(next_state) != goto_table[state][symbol].end())
                {
                    spdlog::debug("Goto already exists in goto table for state {} and symbol {}: {}", state, std::string(symbol), next_state);
                    spdlog::debug("No need to add it again");
                    return false; // action already exists, do nothing
                }

                // check if the goto set is not empty, we have a conflict
                spdlog::warn("Conflict detected in goto table for state {} and symbol {}: {}", state, std::string(symbol), next_state);
                goto_table[state][symbol].insert(next_state);
            }
            spdlog::debug("Adding goto to goto table for state {} and symbol {}: {}", state, std::string(symbol), goto_table[state][symbol].find(next_state) != goto_table[state][symbol].end() ? next_state : "not found");
            return true;
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error adding goto to goto table: {}", e.what());
            throw;
        }
    }

    bool LRParsingTable::check_cell_empty(const std::string &state, const cfg_model::symbol &symbol) const
    {
        try{
        // IMPORTANT: EMPTY ACTION is NOT an empty CELL in action table, and empty string is NOT an empty CELL in goto table
        // if the cell has an empty action, it is FILLED with an action, even if the action TYPE is empty, same for goto table
        // check if at least one of the table has the state
        bool cell_in_action_table = !check_action_table_cell_empty(state, symbol);
        bool cell_in_goto_table = !check_goto_table_cell_empty(state, symbol);
        if (cell_in_action_table || cell_in_goto_table)
        {
            if (cell_in_action_table && cell_in_goto_table)
            {
                spdlog::warn("Cell is not empty for state {} and symbol {}: both action and goto table have the cell", state, std::string(symbol));
            }
            spdlog::debug("Cell is not empty for state {} and symbol {}", state, std::string(symbol));
            return false; // cell is not empty
        }
        else
        {
            spdlog::debug("Cell is empty for state {} and symbol {}", state, std::string(symbol));
            return true; // cell is empty
        }
    }
        catch (const std::exception &e)
        {
            spdlog::error("Error checking cell empty: {}", e.what());
            throw;
        }
    }

    bool LRParsingTable::check_action_table_cell_empty(const std::string &state, const cfg_model::symbol &symbol) const
    {
        try
        {
            // check if the action table is empty for the given state
            bool state_in_action_table = action_table.find(state) != action_table.end();
            if (!state_in_action_table)
            {
                spdlog::debug("State {} not found in action table", state);
                return true; // cell is empty
            }
            // check if the symbol is in the action table for the given state
            bool symbol_in_action_table = action_table.at(state).find(symbol) != action_table.at(state).end();
            if (!symbol_in_action_table)
            {
                spdlog::debug("Symbol {} not found in action table for state {}", std::string(symbol), state);
                return true; // cell is empty
            }
            // check if the action table is empty for the given state and symbol
            if (action_table.at(state).at(symbol).empty())
            {
                spdlog::debug("Action table cell is empty for state {} and symbol {}", state, std::string(symbol));
                return true; // cell is empty
            }
            
            return false; // cell is not empty
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error checking action table cell empty: {}", e.what());
            throw;
        }
    }

    bool LRParsingTable::check_goto_table_cell_empty(const std::string &state, const cfg_model::symbol &symbol) const
    {
        try
        {
            // check if the goto table is empty for the given state
            bool state_in_goto_table = goto_table.find(state) != goto_table.end();
            if (!state_in_goto_table)
            {
                spdlog::debug("State {} not found in goto table", state);
                return true; // cell is empty
            }
            // check if the symbol is in the goto table for the given state
            bool symbol_in_goto_table = goto_table.at(state).find(symbol) != goto_table.at(state).end();
            if (!symbol_in_goto_table)
            {
                spdlog::debug("Symbol {} not found in goto table for state {}", std::string(symbol), state);
                return true; // cell is empty
            }
            // check if the goto table is empty for the given state and symbol
            if (goto_table.at(state).at(symbol).empty())
            {
                spdlog::debug("Goto table cell is empty for state {} and symbol {}", state, std::string(symbol));
                return true; // cell is empty
            }
            
            return false; // cell is not empty
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error checking goto table cell empty: {}", e.what());
            throw;
        }
    }

    std::unordered_set<Action> LRParsingTable::get_actions(const std::string &state, const cfg_model::symbol &symbol) const
    {
        try{
        // check if the action table and goto table are empty for the given state and symbol
        if (!symbol.is_terminal)
        {
            std::string error_message = "Error: Symbol is not terminal: " + std::string(symbol) + ", use get_gotos() instead";
            spdlog::error(error_message);
            throw std::runtime_error(error_message);
        }
        if (check_cell_empty(state, symbol))
        {
            throw std::runtime_error("No actions found for state " + state + " and symbol " + std::string(symbol));
        }
        // return the actions for the given state and symbol
        return action_table.at(state).at(symbol);
        }
        catch (const std::exception &e)
        {
            spdlog::error("Error getting actions: {}", e.what());
            throw;
        }
    }

    std::unordered_set<std::string> LRParsingTable::get_gotos(const std::string &state, const cfg_model::symbol &symbol) const
    {
        try{
        if (symbol.is_terminal)
        {
            std::string error_message = "Error: Symbol is terminal: " + std::string(symbol) + ", use get_actions() instead";
            spdlog::error(error_message);
            throw std::runtime_error(error_message);
        }
        // check if the action table and goto table are empty for the given state and symbol
        if (check_cell_empty(state, symbol))
        {
            throw std::runtime_error("No gotos found for state " + state + " and symbol " + std::string(symbol));
        }
        // return the gotos for the given state and symbol
        return goto_table.at(state).at(symbol);
    }
        catch (const std::exception &e)
        {
            spdlog::error("Error getting gotos: {}", e.what());
            throw;
        }
    }

    bool LRParsingTable::filling_check() const
    {
        // IMPORTANT: EMPTY ACTION is NOT empty CELL
        // if the cell has an empty action, it is FILLED with an action, even if the action TYPE is empty
        // check if the action table and goto table are empty for all states and symbols
        bool valid = true;
        int missing_cells = 0;
        int unrecognized_cells = 0;
        for (const auto &state : all_states)
        {
            for (const auto &symbol : all_symbols)
            {
                if (check_cell_empty(state, symbol))
                {
                    spdlog::warn("Empty cell found in parsing table for state {} and symbol {}", state, std::string(symbol));
                    valid = false;
                    missing_cells++;
                }
            }
        }
        // check if there are any cells out of states and symbols
        // check action table
        for (const auto &state_pair : action_table)
        {
            for (const auto &symbol_pair : state_pair.second)
            {
                if (all_states.find(state_pair.first) == all_states.end() || all_symbols.find(symbol_pair.first) == all_symbols.end())
                {
                    spdlog::warn("Cell found out of states and symbols in action table for state {} and symbol {}", state_pair.first, std::string(symbol_pair.first));
                    valid = false;
                    unrecognized_cells++;
                }
            }
        }
        // check goto table
        for (const auto &state_pair : goto_table)
        {
            for (const auto &symbol_pair : state_pair.second)
            {
                if (all_states.find(state_pair.first) == all_states.end() || all_symbols.find(symbol_pair.first) == all_symbols.end())
                {
                    spdlog::warn("Cell found out of states and symbols in goto table for state {} and symbol {}", state_pair.first, std::string(symbol_pair.first));
                    valid = false;
                    unrecognized_cells++;
                }
            }
        }
        spdlog::debug("Parsing table filling check: {}", valid ? "valid" : "invalid");
        if (!valid)
        {
            spdlog::warn("Parsing table filling check failed: {} missing cells and {} unrecognized cells", missing_cells, unrecognized_cells);
        }
        return valid;
    }

    std::multimap<std::string, cfg_model::symbol> LRParsingTable::find_conflicts() const
    {
        std::multimap<std::string, cfg_model::symbol> conflicts;
        int conflict_count = 0;
        for (const auto &state_pair : action_table)
        {
            const std::string &state = state_pair.first;
            for (const auto &symbol_pair : state_pair.second)
            {
                const cfg_model::symbol &symbol = symbol_pair.first;
                if (symbol_pair.second.size() > 1)
                {
                    spdlog::warn("Conflict found in action table for state {} and symbol {}: {} actions", state, std::string(symbol), symbol_pair.second.size());
                    conflicts.insert({state, symbol});
                    conflict_count++;
                }
            }
        }
        spdlog::debug("Found {} conflicts in action table", conflict_count);
        return conflicts;
    }

} // namespace lr_parsing_model
