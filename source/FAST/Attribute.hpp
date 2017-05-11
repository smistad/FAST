#ifndef FAST_ATTRIBUTE_HPP_
#define FAST_ATTRIBUTE_HPP_

#include "FASTExport.hpp"
#include <string>
#include <memory>
#include <vector>

namespace fast {

enum AttributeType { ATTRIBUTE_TYPE_STRING, ATTRIBUTE_TYPE_FLOAT, ATTRIBUTE_TYPE_INTEGER, ATTRIBUTE_TYPE_BOOLEAN};

class FAST_EXPORT  AttributeValue {
    public:
        virtual ~AttributeValue() {};
};

#define CREATE_ATTRIBUTE_VALUE_OBJECT(NAME, TYPE)                   \
class AttributeValue##NAME : public AttributeValue {                \
    public:                                                         \
        AttributeValue##NAME(TYPE value) : mValue(value) {};        \
        TYPE get() {                                                \
            return mValue;                                          \
        }                                                           \
    private:                                                        \
        TYPE mValue;                                                \
};                                                                  \

CREATE_ATTRIBUTE_VALUE_OBJECT(String, std::string)
CREATE_ATTRIBUTE_VALUE_OBJECT(Float, float)
CREATE_ATTRIBUTE_VALUE_OBJECT(Integer, int)
CREATE_ATTRIBUTE_VALUE_OBJECT(Boolean, bool)


class FAST_EXPORT  Attribute {
    public:
        Attribute(std::string id, std::string name, std::string description, AttributeType type) :
                mID(id), mName(name), mDescription(description), mType(type) {
        }

        void setValue(std::shared_ptr<AttributeValue> value) {
            if(mValues.size() == 0) {
                mValues.push_back(value);
            } else {
                mValues[0] = value;
            }
        }

        std::shared_ptr<AttributeValue> getValue() const {
            return mValues.at(0);
        }

        void setValues(std::vector<std::shared_ptr<AttributeValue>> values) {
            mValues = values;
        }

        std::vector<std::shared_ptr<AttributeValue>> getValues() const {
            return mValues;
        }

        std::string getName() const {
            return mName;
        }

        std::string getID() const {
            return mID;
        }

        AttributeType getType() const {
            return mType;
        }

        void parseInput(std::string input);
    private:
        void parseBooleanInput(std::string input);
        void parseStringInput(std::string input);
        void parseFloatInput(std::string input);
        void parseIntegerInput(std::string input);

        std::string mID;
        std::string mName;
        std::string mDescription;
        AttributeType mType;
        std::vector<std::shared_ptr<AttributeValue> > mValues;
};

} // end namespace fast

#endif
