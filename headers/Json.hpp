#ifndef JSON_HPP
#define JSON_HPP

#include <string>
#include <map>
#include <vector>

class Json;

class JsonValue
{
public:
    enum Type
    {
        JSON_NULL,
        JSON_BOOL,
        JSON_INT,
        JSON_DOUBLE,
        JSON_STRING,
        JSON_ARRAY,
        JSON_OBJECT
    };

private:
    Type type_;

    union Data
    {
        bool bool_value;
        int int_value;
        double double_value;
        std::string *string_value;
        std::vector<JsonValue> *array_value;
        std::map<std::string, JsonValue> *object_value;

        Data(void);
        ~Data(void);
    } data_;

    void cleanup(void);
    void copyFrom(const JsonValue &other);

public:
    JsonValue(void);
    JsonValue(bool value);
    JsonValue(int value);
    JsonValue(double value);
    JsonValue(const std::string &value);
    JsonValue(const char *value);
    JsonValue(const JsonValue &other);

    JsonValue &operator=(const JsonValue &other);
    ~JsonValue(void);

    Type getType(void) const;
    bool isNull(void) const;
    bool isBool(void) const;
    bool isInt(void) const;
    bool isDouble(void) const;
    bool isString(void) const;
    bool isArray(void) const;
    bool isObject(void) const;

    bool asBool(void) const;
    int asInt(void) const;
    double asDouble(void) const;
    const std::string &asString(void) const;
    std::vector<JsonValue> &asArray(void);
    Json asObject(void) const;

    JsonValue &operator[](size_t index);
    JsonValue &operator[](const std::string &key);
    JsonValue &operator[](const char *key);
};

class Json
{
private:
    std::map<std::string, JsonValue> data_;

    void parseJson(const std::string &content);
    std::string stringifyValue(const JsonValue &value) const;

public:
    Json(void);
    Json(const std::string &content);

    JsonValue get(const std::string &key) const;

    void set(const std::string &key, const JsonValue &value);
    void set(const std::string &key, bool value);
    void set(const std::string &key, int value);
    void set(const std::string &key, double value);
    void set(const std::string &key, const std::string &value);
    void set(const std::string &key, const char *value);

    JsonValue &operator[](const std::string &key);
    const JsonValue &operator[](const std::string &key) const;

    bool has(const std::string &key) const;
    bool remove(const std::string &key);
    std::vector<std::string> keys(void) const;
    bool empty(void) const;
    size_t size(void) const;
    void clear(void);
    std::string stringify(void) const;
};

#endif