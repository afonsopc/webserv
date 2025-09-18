#ifndef HASHMAP_HPP
#define HASHMAP_HPP

#include <string>
#include <map>
#include <vector>

class HashMap;

class HashMapValue
{
public:
    enum Type
    {
        HASHMAP_NULL,
        HASHMAP_BOOL,
        HASHMAP_INT,
        HASHMAP_DOUBLE,
        HASHMAP_STRING,
        HASHMAP_ARRAY,
        HASHMAP_OBJECT
    };

private:
    Type type_;

    union Data
    {
        bool bool_value;
        int int_value;
        double double_value;
        std::string *string_value;
        std::vector<HashMapValue> *array_value;
        std::map<std::string, HashMapValue> *object_value;

        Data(void);
        ~Data(void);
    } data_;

    void cleanup(void);
    void copyFrom(const HashMapValue &other);

public:
    HashMapValue(void);
    HashMapValue(bool value);
    HashMapValue(int value);
    HashMapValue(double value);
    HashMapValue(const std::string &value);
    HashMapValue(const char *value);
    HashMapValue(const HashMapValue &other);

    HashMapValue &operator=(const HashMapValue &other);
    ~HashMapValue(void);

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
    std::vector<HashMapValue> &asArray(void);
    HashMap asObject(void) const;

    HashMapValue &operator[](size_t index);
    HashMapValue &operator[](const std::string &key);
    HashMapValue &operator[](const char *key);
};

class HashMap
{
private:
    std::map<std::string, HashMapValue> data_;

    void parseJson(const std::string &content);
    std::string headerifyValue(const HashMapValue &value) const;

public:
    std::string stringifyValue(const HashMapValue &value) const;
    HashMap(void);
    HashMap(const std::string &json);

    HashMapValue get(const std::string &key) const;

    void set(const std::string &key, const HashMapValue &value);
    void set(const std::string &key, bool value);
    void set(const std::string &key, int value);
    void set(const std::string &key, double value);
    void set(const std::string &key, const std::string &value);
    void set(const std::string &key, const char *value);

    HashMapValue &operator[](const std::string &key);
    const HashMapValue &operator[](const std::string &key) const;

    bool has(const std::string &key) const;
    bool remove(const std::string &key);
    std::vector<std::string> keys(void) const;
    bool empty(void) const;
    size_t size(void) const;
    void clear(void);
    std::string stringify(void) const;
    std::string headerify(void) const;
};

#endif
