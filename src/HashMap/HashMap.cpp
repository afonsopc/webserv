#include "HashMap.hpp"
#include <sstream>
#include <stdexcept>

bool HashMap::has(const std::string &key) const { return (data_.find(key) != data_.end()); }
bool HashMap::remove(const std::string &key) { return (data_.erase(key) > 0); }

std::vector<std::string> HashMap::keys(void) const
{
    std::vector<std::string> result;
    for (std::map<std::string, HashMapValue>::const_iterator it = data_.begin(); it != data_.end(); ++it)
        result.push_back(it->first);
    return (result);
}

bool HashMap::empty(void) const { return (data_.empty()); }
size_t HashMap::size(void) const { return (data_.size()); }
void HashMap::clear(void) { data_.clear(); }

std::string HashMap::stringify(void) const
{
    std::string result = "{";
    bool first = true;

    for (std::map<std::string, HashMapValue>::const_iterator it = data_.begin(); it != data_.end(); ++it)
    {
        if (!first)
            result += ",";
        first = false;

        result += "\"" + it->first + "\":" + stringifyValue(it->second);
    }

    result += "}";
    return (result);
}

void HashMap::parseJson(const std::string &content)
{
    if (content.empty())
        return;
    std::string trimmed = content;
    size_t start = trimmed.find_first_not_of(" \t\n\r");
    size_t end = trimmed.find_last_not_of(" \t\n\r");
    if (start == std::string::npos)
        return;
    trimmed = trimmed.substr(start, end - start + 1);
    if (trimmed[0] != '{' || trimmed[trimmed.length() - 1] != '}')
        throw(std::runtime_error("Invalid JSON format"));
    trimmed = trimmed.substr(1, trimmed.length() - 2);

    size_t pos = 0;
    while (pos < trimmed.length())
    {
        size_t keyStart = trimmed.find('"', pos);
        if (keyStart == std::string::npos)
            break;
        size_t keyEnd = trimmed.find('"', keyStart + 1);
        if (keyEnd == std::string::npos)
            throw(std::runtime_error("Invalid JSON format: unterminated key"));

        std::string key = trimmed.substr(keyStart + 1, keyEnd - keyStart - 1);

        size_t colonPos = trimmed.find(':', keyEnd);
        if (colonPos == std::string::npos)
            throw(std::runtime_error("Invalid JSON format: missing colon"));

        size_t valueStart = colonPos + 1;
        while (valueStart < trimmed.length() && (trimmed[valueStart] == ' ' || trimmed[valueStart] == '\t'))
            valueStart++;

        HashMapValue value;
        size_t valueEnd = valueStart;

        if (trimmed[valueStart] == '"')
        {
            valueEnd = trimmed.find('"', valueStart + 1);
            if (valueEnd == std::string::npos)
                throw(std::runtime_error("Invalid JSON format: unterminated string"));
            value = HashMapValue(trimmed.substr(valueStart + 1, valueEnd - valueStart - 1));
            valueEnd++;
        }
        else if (trimmed.substr(valueStart, 4) == "true")
        {
            value = HashMapValue(true);
            valueEnd = valueStart + 4;
        }
        else if (trimmed.substr(valueStart, 5) == "false")
        {
            value = HashMapValue(false);
            valueEnd = valueStart + 5;
        }
        else if (trimmed.substr(valueStart, 4) == "null")
        {
            value = HashMapValue();
            valueEnd = valueStart + 4;
        }
        else
        {
            size_t numEnd = valueStart;
            while (numEnd < trimmed.length() &&
                   (std::isdigit(trimmed[numEnd]) || trimmed[numEnd] == '.' ||
                    trimmed[numEnd] == '-' || trimmed[numEnd] == '+'))
                numEnd++;

            std::string numStr = trimmed.substr(valueStart, numEnd - valueStart);
            if (numStr.find('.') != std::string::npos)
                value = HashMapValue(std::atof(numStr.c_str()));
            else
                value = HashMapValue(std::atoi(numStr.c_str()));
            valueEnd = numEnd;
        }

        data_[key] = value;

        pos = trimmed.find(',', valueEnd);
        if (pos == std::string::npos)
            break;
        pos++;
    }
}

static std::string stringifyArray(const HashMapValue &array, const HashMap &context)
{
    std::string result = "[";
    bool first = true;
    HashMapValue non_const_array = array;
    const std::vector<HashMapValue> &arr = non_const_array.asArray();
    for (std::vector<HashMapValue>::const_iterator it = arr.begin(); it != arr.end(); ++it)
    {
        if (!first)
            result += ",";
        first = false;
        result += context.stringifyValue(*it);
    }
    result += "]";
    return (result);
}

static std::string stringifyObject(const HashMapValue &object, const HashMap &context)
{
    std::string result = "{";
    bool first = true;
    HashMap non_const_object = object.asObject();
    std::vector<std::string> keys = non_const_object.keys();
    for (std::vector<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it)
    {
        if (!first)
            result += ",";
        first = false;
        result += "\"" + *it + "\":" + context.stringifyValue(non_const_object.get(*it));
    }
    result += "}";
    return (result);
}

std::string HashMap::stringifyValue(const HashMapValue &value) const
{
    switch (value.getType())
    {
    case HashMapValue::HASHMAP_NULL:
        return ("null");
    case HashMapValue::HASHMAP_BOOL:
        return (value.asBool() ? "true" : "false");
    case HashMapValue::HASHMAP_INT:
    {
        std::ostringstream oss;
        oss << value.asInt();
        return (oss.str());
    }
    case HashMapValue::HASHMAP_DOUBLE:
    {
        std::ostringstream oss;
        oss << value.asDouble();
        return (oss.str());
    }
    case HashMapValue::HASHMAP_STRING:
        return ("\"" + value.asString() + "\"");
    case HashMapValue::HASHMAP_ARRAY:
        return (stringifyArray(value, *this));
    case HashMapValue::HASHMAP_OBJECT:
        return (stringifyObject(value, *this));
    default:
        return ("null");
    }
}

HashMap::HashMap(void) {}
HashMap::HashMap(const std::string &json) { parseJson(json); }

HashMapValue HashMap::get(const std::string &key) const
{
    std::map<std::string, HashMapValue>::const_iterator it = data_.find(key);
    if (it != data_.end())
        return (it->second);
    return (HashMapValue());
}

void HashMap::set(const std::string &key, const HashMapValue &value) { data_[key] = value; }
void HashMap::set(const std::string &key, bool value) { data_[key] = HashMapValue(value); }
void HashMap::set(const std::string &key, int value) { data_[key] = HashMapValue(value); }
void HashMap::set(const std::string &key, double value) { data_[key] = HashMapValue(value); }
void HashMap::set(const std::string &key, const std::string &value) { data_[key] = HashMapValue(value); }
void HashMap::set(const std::string &key, const char *value) { data_[key] = HashMapValue(value); }

HashMapValue &HashMap::operator[](const std::string &key) { return data_[key]; }
const HashMapValue &HashMap::operator[](const std::string &key) const
{
    std::map<std::string, HashMapValue>::const_iterator it = data_.find(key);
    if (it != data_.end())
        return (it->second);

    static HashMapValue null_value;
    return (null_value);
}

std::string HashMap::headerifyValue(const HashMapValue &value) const
{
    switch (value.getType())
    {
    case HashMapValue::HASHMAP_NULL:
        return ("");
    case HashMapValue::HASHMAP_BOOL:
        return (value.asBool() ? "true" : "false");
    case HashMapValue::HASHMAP_INT:
    {
        std::ostringstream oss;
        oss << value.asInt();
        return (oss.str());
    }
    case HashMapValue::HASHMAP_DOUBLE:
    {
        std::ostringstream oss;
        oss << value.asDouble();
        return (oss.str());
    }
    case HashMapValue::HASHMAP_STRING:
        return (value.asString());
    case HashMapValue::HASHMAP_ARRAY:
    case HashMapValue::HASHMAP_OBJECT:
        return ("");
    default:
        return ("");
    }
}

std::string HashMap::headerify(void) const
{
    std::string result;
    for (std::map<std::string, HashMapValue>::const_iterator it = data_.begin(); it != data_.end(); ++it)
    {
        if (!result.empty())
            result += "\r\n";
        result += it->first + ": " + headerifyValue(it->second);
    }
    return (result);
}
