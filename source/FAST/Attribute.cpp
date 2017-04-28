#include "Attribute.hpp"
#include "FAST/Utility.hpp"

namespace fast {

void Attribute::parseInput(std::string input) {
    switch(mType) {
        case ATTRIBUTE_TYPE_BOOLEAN:
            parseBooleanInput(input);
            break;
        case ATTRIBUTE_TYPE_STRING:
            parseStringInput(input);
            break;
        case ATTRIBUTE_TYPE_FLOAT:
            parseFloatInput(input);
            break;
        case ATTRIBUTE_TYPE_INTEGER:
            parseIntegerInput(input);
            break;
   }
}

void Attribute::parseStringInput(std::string input) {
    std::vector<std::string> strings = split(input, "\" \"");
    if(strings.size() > 1) {
        std::vector<std::shared_ptr<AttributeValue>> values;
        for(auto string : strings) {
            string = replace(string, "\"", "");
            std::shared_ptr<AttributeValueString> value = std::make_shared<AttributeValueString>(string);
            values.push_back(value);
        }
        setValues(values);
    } else {
        input = replace(input, "\"", "");
        std::shared_ptr<AttributeValueString> value = std::make_shared<AttributeValueString>(input);
        setValue(value);
    }
}

void Attribute::parseIntegerInput(std::string input) {
    std::vector<std::string> strings = split(input, " ");
    std::vector<std::shared_ptr<AttributeValue>> values;
    for(auto string : strings) {
        int integer = std::stoi(string);
        std::shared_ptr<AttributeValueInteger> value = std::make_shared<AttributeValueInteger>(integer);
        values.push_back(value);
    }
    setValues(values);
}

void Attribute::parseFloatInput(std::string input) {
    std::vector<std::string> strings = split(input, " ");
    std::vector<std::shared_ptr<AttributeValue>> values;
    for(auto string : strings) {
        float floatValue = std::stof(string);
        std::shared_ptr<AttributeValueFloat> value = std::make_shared<AttributeValueFloat>(floatValue);
        values.push_back(value);
    }
    setValues(values);
}

void Attribute::parseBooleanInput(std::string input) {
    std::vector<std::string> strings = split(input, " ");
    std::vector<std::shared_ptr<AttributeValue>> values;
    for(auto string : strings) {
        bool boolean;
        if(string == "true") {
            boolean = true;
        } else if(string == "false") {
            boolean = false;
        } else {
            throw Exception("Error while parsing boolean input attribute: " + input + ", expecting true or false");
        }
        std::shared_ptr<AttributeValueBoolean> value = std::make_shared<AttributeValueBoolean>(boolean);
        values.push_back(value);
    }
    setValues(values);
}

}