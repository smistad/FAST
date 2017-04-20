#ifndef FAST_ATTRIBUTE_HPP_
#define FAST_ATTRIBUTE_HPP_

#include <string>
#include <memory>
#include <vector>

namespace fast {

enum AttributeType { ATTRIBUTE_TYPE_STRING, ATTRIBUTE_TYPE_FLOAT, ATTRIBUTE_TYPE_INTEGER, ATTRIBUTE_TYPE_BOOLEAN};

class AttributeValue {
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


class Attribute {
    public:
        Attribute(std::string name, std::string description, AttributeType type) :
                mName(name), mDescription(description), mType(type) {

        }
    private:
        std::string mName;
        std::string mDescription;
        AttributeType mType;
        std::vector<std::shared_ptr<AttributeValue> > mValues;
};

} // end namespace fast

#endif
