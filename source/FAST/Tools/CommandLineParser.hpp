#pragma once

#include <unordered_map>
#include <FAST/Data/DataTypes.hpp>

namespace fast {


class CommandLineParser {
    public:
        CommandLineParser(std::string title, std::string description = "");
        void parse(const int argc, char** const argv);
        void addOption(
                std::string name,
                std::string helpText = ""
        );
        void addVariable(
                const std::string& name,
                bool required,
                const std::string& helpText = ""
        );
        void addVariable(
                const std::string& name,
                const char * defaultValue = "",
                const std::string& helpText = ""
        );
        void addVariable(
                const std::string& name,
                const std::string& defaultValue,
                const std::string& helpText = ""
        );
        void addPositionVariable(
                uint position,
                const std::string &name,
                const std::string &defaultValue = "",
                const std::string &helpText = ""
        );
        void addPositionVariable(
                uint position,
                const std::string &name,
                const char *defaultValue = "",
                const std::string &helpText = ""
        );
        void addPositionVariable(
                uint position,
                const std::string &name,
                bool required,
                const std::string &helpText = ""
        );

        // With choices
        void addChoice(
                const std::string& name,
                bool required,
                std::vector<std::string> choices,
                const std::string& helpText = ""
        );
        void addChoice(
                const std::string& name,
                std::vector<std::string> choices,
                const char * defaultValue = "",
                const std::string& helpText = ""
        );
        void addChoice(
                const std::string& name,
                std::vector<std::string> choices,
                const std::string& defaultValue,
                const std::string& helpText = ""
        );
        void addPositionChoice(
                uint position,
                const std::string &name,
                std::vector<std::string> choices,
                const std::string &defaultValue = "",
                const std::string &helpText = ""
        );
        void addPositionChoice(
                uint position,
                const std::string &name,
                std::vector<std::string> choices,
                const char *defaultValue = "",
                const std::string &helpText = ""
        );
        void addPositionChoice(
                uint position,
                const std::string &name,
                std::vector<std::string> choices,
                bool required,
                const std::string &helpText = ""
        );
        bool gotValue(const std::string &name) const;
        std::string get(const std::string &name) const;
        std::string get(uint position) const;
        bool getOption(const std::string &name) const;

        template <class T>
        T get(const std::string &name) const;

        template <class T>
        T get(uint position) const;
    private:
        void printHelpMessage() const;

        std::string m_title, m_description, m_command;

        class Variable {
            public:
                explicit Variable(const std::string& name, const std::string& helpText, bool requied) : name(name), helpText(helpText), required(required) {}
                bool required;
                std::string helpText;
                std::string name;
                virtual void setValue(const std::string& value) = 0;
                virtual std::string getValue() = 0;
                virtual void printHelp() = 0;
        };

        class PositionableVariable : public Variable {
            public:
                using Variable::Variable;
                uint position = 0; // 0, means it has no position
                virtual void setValue(const std::string& value) = 0;
                virtual std::string getValue() = 0;
                virtual void printHelp() = 0;
        };

        class StringVariable : public PositionableVariable{
            public:
                using PositionableVariable::PositionableVariable;
                std::string defaultValue;
                std::string value = "";
                virtual void setValue(const std::string& value) override;
                std::string getValue() override;
                virtual void printHelp() override;
        };

        class Choice : public StringVariable {
            public:
                using StringVariable::StringVariable;
                std::vector<std::string> choices;
                void setValue(const std::string& value) override;
                void printHelp() override;
        };

        class Option : public Variable {
            public:
                using Variable::Variable;
                bool value = false;
                void setValue(const std::string& value) override;
                std::string getValue() override;
                void printHelp() override;
        };

        std::map<std::string, std::shared_ptr<Variable>> m_variables;
        std::map<uint, std::shared_ptr<Variable>> m_positionVariables;

        void processToken(std::shared_ptr<Variable>&, uint position, const std::string &token);
};

// Template specializations
template <>
int CommandLineParser::get(const std::string &name) const;

template <>
float CommandLineParser::get(const std::string &name) const;

template <>
Vector3i CommandLineParser::get(const std::string &name) const;

template <>
Vector3f CommandLineParser::get(const std::string &name) const;

}