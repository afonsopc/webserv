#include "Json.hpp"
#include <sstream>
#include <stdexcept>

bool Json::has(const std::string &key) const { return data_.find(key) != data_.end(); }
bool Json::remove(const std::string &key) { return data_.erase(key) > 0; }

std::vector<std::string> Json::keys(void) const
{
    std::vector<std::string> result;
    for (std::map<std::string, JsonValue>::const_iterator it = data_.begin(); it != data_.end(); ++it)
        result.push_back(it->first);
    return result;
}

bool Json::empty(void) const { return data_.empty(); }
size_t Json::size(void) const { return data_.size(); }
void Json::clear(void) { data_.clear(); }

std::string Json::stringify(void) const
{
    std::string result = "{";
    bool first = true;

    for (std::map<std::string, JsonValue>::const_iterator it = data_.begin(); it != data_.end(); ++it)
    {
        if (!first)
            result += ",";
        first = false;

        result += "\"" + it->first + "\":" + stringifyValue(it->second);
    }

    result += "}";
    return result;
}

void Json::parseJson(const std::string &content)
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
        throw std::runtime_error("Invalid JSON format");
    trimmed = trimmed.substr(1, trimmed.length() - 2);
    size_t pos = 0;
    while (pos < trimmed.length())
    {
        size_t keyStart = trimmed.find('"', pos);
        if (keyStart == std::string::npos)
            break;
        size_t keyEnd = trimmed.find('"', keyStart + 1);
        if (keyEnd == std::string::npos)
            throw std::runtime_error("Invalid JSON key");
        std::string key = trimmed.substr(keyStart + 1, keyEnd - keyStart - 1);
        size_t colonPos = trimmed.find(':', keyEnd);
        if (colonPos == std::string::npos)
            throw std::runtime_error("Missing colon in JSON");
        size_t valueStart = trimmed.find_first_not_of(" \t", colonPos + 1);
        if (valueStart == std::string::npos)
            throw std::runtime_error("Missing value in JSON");
        JsonValue value;
        size_t valueEnd = valueStart;
        if (trimmed[valueStart] == '"')
        {
            size_t stringEnd = trimmed.find('"', valueStart + 1);
            if (stringEnd == std::string::npos)
                throw std::runtime_error("Unterminated string in JSON");
            std::string strValue = trimmed.substr(valueStart + 1, stringEnd - valueStart - 1);
            value = JsonValue(strValue);
            valueEnd = stringEnd + 1;
        }
        else if (trimmed.substr(valueStart, 4) == "true")
        {
            value = JsonValue(true);
            valueEnd = valueStart + 4;
        }
        else if (trimmed.substr(valueStart, 5) == "false")
        {
            value = JsonValue(false);
            valueEnd = valueStart + 5;
        }
        else if (trimmed.substr(valueStart, 4) == "null")
        {
            value = JsonValue();
            valueEnd = valueStart + 4;
        }
        else
        {
            size_t numberEnd = trimmed.find_first_of(",}", valueStart);
            if (numberEnd == std::string::npos)
                numberEnd = trimmed.length();
            std::string numberStr = trimmed.substr(valueStart, numberEnd - valueStart);
            size_t numEnd = numberStr.find_last_not_of(" \t");
            if (numEnd != std::string::npos)
                numberStr = numberStr.substr(0, numEnd + 1);
            if (numberStr.find('.') != std::string::npos)
            {
                std::istringstream iss(numberStr);
                double doubleValue;
                iss >> doubleValue;
                value = JsonValue(doubleValue);
            }
            else
            {
                std::istringstream iss(numberStr);
                int intValue;
                iss >> intValue;
                value = JsonValue(intValue);
            }
            valueEnd = numberEnd;
        }
        data_[key] = value;
        pos = trimmed.find(',', valueEnd);
        if (pos == std::string::npos)
            break;
        pos++;
    }
}

std::string Json::stringifyValue(const JsonValue &value) const
{
    switch (value.getType())
    {
    case JsonValue::JSON_NULL:
        return "null";
    case JsonValue::JSON_BOOL:
        return value.asBool() ? "true" : "false";
    case JsonValue::JSON_INT:
    {
        std::ostringstream oss;
        oss << value.asInt();
        return oss.str();
    }
    case JsonValue::JSON_DOUBLE:
    {
        std::ostringstream oss;
        oss << value.asDouble();
        return oss.str();
    }
    case JsonValue::JSON_STRING:
        return "\"" + value.asString() + "\"";
    case JsonValue::JSON_ARRAY:
    {
        return "[]";
    }
    case JsonValue::JSON_OBJECT:
    {
        return "{}";
    }
    default:
        return "null";
    }
}
