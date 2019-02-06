#include "FAST/Tools/CommandLineParser.hpp"
#include <iostream>
#include <FAST/Utility.hpp>

namespace fast {

CommandLineParser::CommandLineParser(std::string title, std::string description) {
    m_title = title;
    m_description = description;
}

void CommandLineParser::parse(const int argc, char ** const argv) {
    // TODO make sure parse has been run before, calling get, gotValue etc.
    // TODO make sure there are no spaces. Add support for "asdasd asdasd"
    m_command = argv[0];
    SharedPointer<Variable> currentVariable;
    uint currentPosition = 1;
    for(int i = 1; i < argc; ++i) {
        std::string token = argv[i];
        Reporter::info() << "Processing token " << token << Reporter::end();

        if(token == "--help") {
            printHelpMessage();
            std::exit(0);
        }

        processToken(currentVariable, currentPosition, token);
        currentPosition++;
    }

    // Check if all required variables got a value
    bool error = false;
    for(auto&& var : m_variables) {
        if(var.second->required) {
            if(var.second->getValue().empty()) {
                if(!error)
                    std::cerr << "Command line errors encountered:\n";
                std::cerr << "* The required variable ´--" << var.first << "´ was not given in the command line" << "\n";
                error = true;
            }
        }
    }
    if(error) {
        std::cerr << "Run `" << m_command << " --help` for more information.\n";
        std::cerr << std::flush;
        std::exit(-1);
    }
}

void CommandLineParser::processToken(SharedPointer<Variable>& currentVariable, uint currentPosition, const std::string &token) {

    // If no current variable, then the first token has to start with --, or it is a position variable
    if(!currentVariable) {
        if(token.size() > 2 && token.substr(0, 2) == "--") {
            if(m_variables.count(token.substr(2)) == 0)
                throw Exception("Unknown program parameter " + token.substr(2));
            currentVariable = m_variables.at(token.substr(2));
            currentVariable->setValue(""); // This will set options to true
        } else {
            // Positional variables get token right away
            m_positionVariables.at(currentPosition)->setValue(token);
            Reporter::info() << "CommandLineParser: Assigning variable " << m_positionVariables.at(currentPosition)->name << " with value " << token << Reporter::end();
        }
    } else {
        if(token.size() > 2 && token.substr(0, 2) == "--") {
            // Last variable must have been an option
            // Get new variable
            if(m_variables.count(token.substr(2)) == 0)
                throw Exception("Unknown program parameter " + token.substr(2));
            currentVariable = m_variables.at(token.substr(2));
            currentVariable->setValue(""); // This will set options to true
            return;
        }
        // If we have a variable atm, the token is a value
        Reporter::info() << "CommandLineParser: Assigning variable " << currentVariable->name << " with value " << token << Reporter::end();
        currentVariable->setValue(token);
        // Finish this variable
        currentVariable.reset();
    }
}

void CommandLineParser::addOption(std::string name, std::string helpText) {
    SharedPointer<Option> option = std::make_shared<Option>(name, helpText, false);
    m_variables[name] = option;
}

void CommandLineParser::addVariable(const std::string& name, const std::string& defaultValue, const std::string& helpText) {
    SharedPointer<StringVariable> var = std::make_shared<StringVariable>(name, helpText, false);
    var->defaultValue = defaultValue;
    var->value = defaultValue;

    m_variables[name] = var;
}

void CommandLineParser::addVariable(const std::string& name, bool required, const std::string& helpText) {
    SharedPointer<StringVariable> var = std::make_shared<StringVariable>(name, helpText, required);
    // TODO check for space in name
    m_variables[name] = var;
}

void CommandLineParser::addVariable(const std::string &name, const char *defaultValue, const std::string &helpText) {
    addVariable(name, std::string(defaultValue), helpText);
}

void CommandLineParser::addPositionVariable(uint position, const std::string &name, bool required,
                                            const std::string &helpText) {
    if(position == 0)
        throw Exception("CommandLineParser: Position must be > 0");
    SharedPointer<StringVariable> var = std::make_shared<StringVariable>(name, helpText, required);
    var->position = position;
    m_positionVariables[position] = var;
    m_variables[name] = var;
}

void CommandLineParser::addPositionVariable(uint position, const std::string &name, const std::string &defaultValue,
                                            const std::string &helpText) {
    if(position == 0)
        throw Exception("CommandLineParser: Position must be > 0");
    SharedPointer<StringVariable> var = std::make_shared<StringVariable>(name, helpText, false);
    // TODO check for space in name
    var->defaultValue = defaultValue;
    var->value = defaultValue;
    var->position = position;
    m_positionVariables[position] = var;
    m_variables[name] = var;
}
void CommandLineParser::addPositionVariable(uint position, const std::string &name, const char *defaultValue,
                                            const std::string &helpText) {
    if(position == 0)
        throw Exception("CommandLineParser: Position must be > 0");
    addPositionVariable(position, name, std::string(defaultValue), helpText);
}

std::string CommandLineParser::get(const std::string &name) const {
    const std::string value = m_variables.at(name)->getValue();
    if(value == "")
        throw Exception("CommandLineParser: No value received for " + name);
    return value;
}

std::string CommandLineParser::get(uint position) const {
    const std::string value = m_positionVariables.at(position)->getValue();
    if(value == "")
        throw Exception("CommandLineParser: No value received for " + m_positionVariables.at(position)->name);
    return value;
}

bool CommandLineParser::getOption(const std::string &name) const {
    if(m_variables.count(name) == 0)
        throw Exception("CommandLineParser: Option " + name + " not found.");
    return m_variables.at(name)->getValue() == "true";
}

void CommandLineParser::printHelpMessage() const {
    std::cout << m_title << "\n";
    std::cout << "==================\n";
    if(!m_description.empty())
        std::cout << m_description<< "\n\n";
    std::cout << "usage: " << m_command;

    for(auto&& var : m_positionVariables) {
        std::cout << " ";
        if(!var.second->required) {
            std::cout << "[" << var.second->name << "]";
        } else {
            std::cout << var.second->name;
        }
    }
    if(!m_variables.empty() && m_positionVariables.size() < m_variables.size())
        std::cout << " [variables]";
    std::cout << "\n\n";

    // Get length of longest variable name
    int maxLength = 0;
    for(auto &&variable : m_variables) {
        if(maxLength < variable.first.size()) {
            maxLength = variable.first.size();
        }
    }

    for(auto &&variable : m_positionVariables) {
        variable.second->printHelp(maxLength + 10);
        std::cout << "\n";
    }
    std::cout << "\n";

    if(!m_variables.empty() && m_positionVariables.size() < m_variables.size()) {
        std::cout << "Variables:\n";
        for(auto &&variable : m_variables) {
            if(variable.second->position == 0) {
                variable.second->printHelp(maxLength + 10);
                std::cout << "\n";
            }
        }
    }

    std::cout << std::flush;
}

bool CommandLineParser::gotValue(const std::string &name) const {
    if(m_variables.count(name) > 0) {
        return !m_variables.at(name)->getValue().empty();
    } else {
        throw Exception("Command line parser: Variable/option " + name + " does not exist.");
    }
}

void CommandLineParser::addChoice(const std::string &name, bool required, std::vector<std::string> choices,
                                    const std::string &helpText) {
    SharedPointer<Choice> var = std::make_shared<Choice>(name, helpText, required);
    var->choices = choices;
    var->value = "";
    m_variables[name] = var;
}

void CommandLineParser::addChoice(const std::string &name, std::vector<std::string> choices, const char *defaultValue,
                                    const std::string &helpText) {
    addChoice(name, choices, std::string(defaultValue), helpText);
}

void CommandLineParser::addChoice(const std::string &name, std::vector<std::string> choices,
                                    const std::string &defaultValue, const std::string &helpText) {
    SharedPointer<Choice> var = std::make_shared<Choice>(name, helpText, false);
    var->defaultValue = defaultValue;
    var->value = defaultValue;
    var->choices = choices;
    m_variables[name] = var;

}

void CommandLineParser::addPositionChoice(uint position, const std::string &name, std::vector<std::string> choices,
                                            const std::string &defaultValue, const std::string &helpText) {
    if(position == 0)
        throw Exception("CommandLineParser: Position must be > 0");
    SharedPointer<Choice> var = std::make_shared<Choice>(name, helpText, false);
    var->defaultValue = defaultValue;
    var->value = "";
    var->choices = choices;
    m_positionVariables[position] = var;
}

void CommandLineParser::addPositionChoice(uint position, const std::string &name, std::vector<std::string> choices,
                                            const char *defaultValue, const std::string &helpText) {
    if(position == 0)
        throw Exception("CommandLineParser: Position must be > 0");
    addPositionChoice(position, name, choices, std::string(defaultValue), helpText);
}

void CommandLineParser::addPositionChoice(uint position, const std::string &name, std::vector<std::string> choices,
                                            bool required, const std::string &helpText) {
    if(position == 0)
        throw Exception("CommandLineParser: Position must be > 0");
    SharedPointer<Choice> var = std::make_shared<Choice>(name, helpText, required);
    var->choices = choices;
    m_positionVariables[position] = var;
}


template <>
int CommandLineParser::get(const std::string &name) const {
    return std::stoi(get(name));
}

template <>
float CommandLineParser::get(const std::string &name) const {
    return std::stof(get(name));
}

template <>
Vector3i CommandLineParser::get(const std::string &name) const {
    auto tokens = split(get(name), ",");
    if(tokens.size() != 3)
        throw Exception("Require exactly 3 values separated by , for command line variable " + name);

    Vector3i result;
    for(int i = 0; i < 3; ++i)
        result[i] = std::stoi(tokens[i]);

    return result;
}

template <>
Vector3f CommandLineParser::get(const std::string &name) const {
    auto tokens = split(get(name), ",");
    if(tokens.size() != 3)
        throw Exception("Require exactly 3 values separated by , for command line variable " + name);

    Vector3f result;
    for(int i = 0; i < 3; ++i)
        result[i] = std::stof(tokens[i]);

    return result;
}

void CommandLineParser::StringVariable::setValue(const std::string &value) {
    this->value = value;
}

std::string CommandLineParser::StringVariable::getValue() {
    return value;
}

void CommandLineParser::StringVariable::printHelp(int length) {
    std::string start = "";
    if(position == 0)
        start += "--";
    start += name;
    if(position == 0)
        start += " value";
    std::cout << start.append(length - start.size(), ' ');
    if(!helpText.empty())
        std::cout << " - " << helpText;
    if(!defaultValue.empty())
        std::cout << " - Default value: " << defaultValue;
}

void CommandLineParser::Choice::setValue(const std::string &value) {
    if(value.empty())
        return;
    bool found = false;
    for(int i = 0; i < choices.size(); ++i) {
        if(choices[i] == value) {
            this->value = value;
            found = true;
            break;
        }
    }
    if(!found)
        throw Exception("Choice " + value + " was not valid for variable " + name);
}

void CommandLineParser::Choice::printHelp(int length) {
    std::string start = "";
    if(position == 0)
        start += "--";
    start += name + " ";
    for(int i = 0; i < choices.size(); ++i) {
        start += choices[i];
        if(i < choices.size() - 1)
            start += "|";
    }
    if(length > start.size()) {
        std::cout << start.append(length - start.size(), ' ');
    } else {
        std::cout << start;
    }
    if(!helpText.empty())
        std::cout << " - " << helpText << " ";
    std::cout << " - Default value: " << defaultValue;
}

void CommandLineParser::Option::setValue(const std::string &value) {
    if(value != "") {
        throw Exception("Option " + name + " should not receive any value. Got " + value);
    }
    this->value = true;
}

std::string CommandLineParser::Option::getValue() {
    return value ? "true" : "false";
}

void CommandLineParser::Option::printHelp(int length) {
    std::string start = "--" + name;
    std::cout << start.append(length - start.size(), ' ');
    if(!helpText.empty())
        std::cout << " - " << helpText;
}

}
