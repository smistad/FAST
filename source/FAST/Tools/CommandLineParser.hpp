#pragma once

#include <unordered_map>
#include <FAST/Data/DataTypes.hpp>

namespace fast {


class FAST_EXPORT CommandLineParser : public Object {
    public:
        CommandLineParser(std::string title, std::string description = "", bool allowUnknownVariables = false);
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

        std::map<std::string, std::string> getVariables();
        ~CommandLineParser() {};
    private:
        void printHelpMessage() const;

        std::string m_title, m_description, m_command;
        bool m_allowUnknownVariables;

        class Variable {
        public:
            explicit Variable(const std::string& name, const std::string& helpText, bool required) : name(name), helpText(helpText), required(required) {};
            uint position = 0; // 0, means it has no position
            bool required;
            std::string helpText;
            std::string name;
            virtual void setValue(const std::string& value) = 0;
            virtual std::string getValue() = 0;
            virtual void printHelp(int length) = 0;
        };

        class StringVariable : public Variable {
        public:
            using Variable::Variable;
            std::string defaultValue;
            std::string value = "";
            virtual void setValue(const std::string& value) override;
            std::string getValue() override;
            virtual void printHelp(int length) override;
        };

        class Choice : public StringVariable {
        public:
            using StringVariable::StringVariable;
            std::vector<std::string> choices;
            void setValue(const std::string& value) override;
            void printHelp(int length) override;
        };

        class Option : public Variable {
        public:
            using Variable::Variable;
            bool value = false;
            void setValue(const std::string& value) override;
            std::string getValue() override;
            void printHelp(int length) override;
        };

        std::map<std::string, SharedPointer<Variable>> m_variables;
        std::map<uint, SharedPointer<Variable>> m_positionVariables;

        void processToken(SharedPointer<Variable>&, uint position, const std::string &token);
};

// Template specializations
template <>
FAST_EXPORT int CommandLineParser::get(const std::string &name) const;

template <>
FAST_EXPORT float CommandLineParser::get(const std::string &name) const;

template <>
FAST_EXPORT Vector3i CommandLineParser::get(const std::string &name) const;

template <>
FAST_EXPORT Vector3f CommandLineParser::get(const std::string &name) const;

}