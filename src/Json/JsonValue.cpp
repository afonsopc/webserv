#include "Json.hpp"
#include <sstream>
#include <stdexcept>

JsonValue::Data::Data(void) : int_value(0) {}
JsonValue::Data::~Data(void) {}

void JsonValue::cleanup(void)
{
    switch (type_)
    {
    case JSON_STRING:
        delete data_.string_value;
        break;
    case JSON_ARRAY:
        delete data_.array_value;
        break;
    case JSON_OBJECT:
        delete data_.object_value;
        break;
    default:
        break;
    }
}

void JsonValue::copyFrom(const JsonValue &other)
{
    switch (other.type_)
    {
    case JSON_NULL:
        break;
    case JSON_BOOL:
        data_.bool_value = other.data_.bool_value;
        break;
    case JSON_INT:
        data_.int_value = other.data_.int_value;
        break;
    case JSON_DOUBLE:
        data_.double_value = other.data_.double_value;
        break;
    case JSON_STRING:
        data_.string_value = new std::string(*other.data_.string_value);
        break;
    case JSON_ARRAY:
        data_.array_value = new std::vector<JsonValue>(*other.data_.array_value);
        break;
    case JSON_OBJECT:
        data_.object_value = new std::map<std::string, JsonValue>(*other.data_.object_value);
        break;
    }
}

JsonValue::JsonValue(void) : type_(JSON_NULL) {}
JsonValue::JsonValue(bool value) : type_(JSON_BOOL) { data_.bool_value = value; }
JsonValue::JsonValue(int value) : type_(JSON_INT) { data_.int_value = value; }
JsonValue::JsonValue(double value) : type_(JSON_DOUBLE) { data_.double_value = value; }
JsonValue::JsonValue(const std::string &value) : type_(JSON_STRING) { data_.string_value = new std::string(value); }
JsonValue::JsonValue(const char *value) : type_(JSON_STRING) { data_.string_value = new std::string(value); }
JsonValue::JsonValue(const JsonValue &other) : type_(other.type_) { copyFrom(other); }

JsonValue &JsonValue::operator=(const JsonValue &other)
{
    if (this != &other)
    {
        cleanup();
        type_ = other.type_;
        copyFrom(other);
    }
    return (*this);
}

JsonValue::~JsonValue(void) { cleanup(); }
JsonValue::Type JsonValue::getType(void) const { return (type_); }
bool JsonValue::isNull(void) const { return type_ == JSON_NULL; }
bool JsonValue::isBool(void) const { return type_ == JSON_BOOL; }
bool JsonValue::isInt(void) const { return type_ == JSON_INT; }
bool JsonValue::isDouble(void) const { return type_ == JSON_DOUBLE; }
bool JsonValue::isString(void) const { return type_ == JSON_STRING; }
bool JsonValue::isArray(void) const { return type_ == JSON_ARRAY; }
bool JsonValue::isObject(void) const { return type_ == JSON_OBJECT; }

bool JsonValue::asBool(void) const
{
    if (type_ != JSON_BOOL)
        throw std::runtime_error("Not a boolean");
    return data_.bool_value;
}

int JsonValue::asInt(void) const
{
    if (type_ != JSON_INT)
        throw std::runtime_error("Not an integer");
    return data_.int_value;
}

double JsonValue::asDouble(void) const
{
    if (type_ != JSON_DOUBLE)
        throw std::runtime_error("Not a double");
    return data_.double_value;
}

const std::string &JsonValue::asString(void) const
{
    if (type_ != JSON_STRING)
        throw std::runtime_error("Not a string");
    return *data_.string_value;
}

std::vector<JsonValue> &JsonValue::asArray(void)
{
    if (type_ != JSON_ARRAY)
    {
        cleanup();
        type_ = JSON_ARRAY;
        data_.array_value = new std::vector<JsonValue>();
    }
    return (*data_.array_value);
}

Json JsonValue::asObject(void) const
{
    if (type_ != JSON_OBJECT)
    {
        throw std::runtime_error("JsonValue is not an object type");
    }

    Json result;
    for (std::map<std::string, JsonValue>::const_iterator it = data_.object_value->begin(); it != data_.object_value->end(); ++it)
    {
        result.set(it->first, it->second);
    }
    return result;
}

JsonValue &JsonValue::operator[](size_t index) { return asArray()[index]; }

JsonValue &JsonValue::operator[](const std::string &key)
{
    if (type_ != JSON_OBJECT)
    {
        cleanup();
        type_ = JSON_OBJECT;
        data_.object_value = new std::map<std::string, JsonValue>();
    }
    return (*data_.object_value)[key];
}

JsonValue &JsonValue::operator[](const char *key)
{
    return (*this)[std::string(key)];
}
Json::Json(void) {}
Json::Json(const std::string &content) { parseJson(content); }

JsonValue Json::get(const std::string &key) const
{
    std::map<std::string, JsonValue>::const_iterator it = data_.find(key);
    if (it != data_.end())
        return (it->second);
    return (JsonValue());
}

void Json::set(const std::string &key, const JsonValue &value) { data_[key] = value; }
void Json::set(const std::string &key, bool value) { data_[key] = JsonValue(value); }
void Json::set(const std::string &key, int value) { data_[key] = JsonValue(value); }
void Json::set(const std::string &key, double value) { data_[key] = JsonValue(value); }
void Json::set(const std::string &key, const std::string &value) { data_[key] = JsonValue(value); }
void Json::set(const std::string &key, const char *value) { data_[key] = JsonValue(value); }
JsonValue &Json::operator[](const std::string &key) { return data_[key]; }

const JsonValue &Json::operator[](const std::string &key) const
{
    std::map<std::string, JsonValue>::const_iterator it = data_.find(key);
    if (it != data_.end())
        return (it->second);
    static const JsonValue null_value;
    return (null_value);
}
