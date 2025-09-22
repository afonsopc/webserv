#include "HashMap.hpp"
#include <sstream>
#include <stdexcept>

HashMapValue::Data::Data(void) : int_value(0) {}
HashMapValue::Data::~Data(void) {}

void HashMapValue::cleanup(void)
{
	switch (type_)
	{
	case HASHMAP_STRING:
		delete data_.string_value;
		break;
	case HASHMAP_ARRAY:
		delete data_.array_value;
		break;
	case HASHMAP_OBJECT:
		delete data_.object_value;
		break;
	default:
		break;
	}
}

void HashMapValue::copyFrom(const HashMapValue &other)
{
	switch (other.type_)
	{
	case HASHMAP_NULL:
		break;
	case HASHMAP_BOOL:
		data_.bool_value = other.data_.bool_value;
		break;
	case HASHMAP_INT:
		data_.int_value = other.data_.int_value;
		break;
	case HASHMAP_DOUBLE:
		data_.double_value = other.data_.double_value;
		break;
	case HASHMAP_STRING:
		data_.string_value = new std::string(*other.data_.string_value);
		break;
	case HASHMAP_ARRAY:
		data_.array_value = new std::vector<HashMapValue>(*other.data_.array_value);
		break;
	case HASHMAP_OBJECT:
		data_.object_value = new std::map<std::string, HashMapValue>(*other.data_.object_value);
		break;
	}
}

HashMapValue::HashMapValue(void) : type_(HASHMAP_NULL) {}

HashMapValue::HashMapValue(bool value) : type_(HASHMAP_BOOL)
{
	data_.bool_value = value;
}

HashMapValue::HashMapValue(int value) : type_(HASHMAP_INT)
{
	data_.int_value = value;
}

HashMapValue::HashMapValue(double value) : type_(HASHMAP_DOUBLE)
{
	data_.double_value = value;
}

HashMapValue::HashMapValue(const std::string &value) : type_(HASHMAP_STRING)
{
	data_.string_value = new std::string(value);
}

HashMapValue::HashMapValue(const char *value) : type_(HASHMAP_STRING)
{
	data_.string_value = new std::string(value);
}

HashMapValue::HashMapValue(const HashMapValue &other) : type_(other.type_)
{
	copyFrom(other);
}

HashMapValue &HashMapValue::operator=(const HashMapValue &other)
{
	if (this != &other)
	{
		cleanup();
		type_ = other.type_;
		copyFrom(other);
	}
	return *this;
}

HashMapValue::~HashMapValue(void)
{
	cleanup();
}

HashMapValue::Type HashMapValue::getType(void) const { return type_; }
bool HashMapValue::isNull(void) const { return type_ == HASHMAP_NULL; }
bool HashMapValue::isBool(void) const { return type_ == HASHMAP_BOOL; }
bool HashMapValue::isInt(void) const { return type_ == HASHMAP_INT; }
bool HashMapValue::isDouble(void) const { return type_ == HASHMAP_DOUBLE; }
bool HashMapValue::isString(void) const { return type_ == HASHMAP_STRING; }
bool HashMapValue::isArray(void) const { return type_ == HASHMAP_ARRAY; }
bool HashMapValue::isObject(void) const { return type_ == HASHMAP_OBJECT; }

bool HashMapValue::asBool(void) const
{
	if (type_ != HASHMAP_BOOL)
		throw std::runtime_error("HashMapValue is not a boolean");
	return data_.bool_value;
}

int HashMapValue::asInt(void) const
{
	if (type_ != HASHMAP_INT)
		throw std::runtime_error("HashMapValue is not an integer");
	return data_.int_value;
}

double HashMapValue::asDouble(void) const
{
	if (type_ != HASHMAP_DOUBLE)
		throw std::runtime_error("HashMapValue is not a double");
	return data_.double_value;
}

const std::string &HashMapValue::asString(void) const
{
	if (type_ != HASHMAP_STRING)
		throw std::runtime_error("HashMapValue is not a string");
	return *data_.string_value;
}

std::vector<HashMapValue> &HashMapValue::asArray(void)
{
	if (type_ != HASHMAP_ARRAY)
	{
		cleanup();
		type_ = HASHMAP_ARRAY;
		data_.array_value = new std::vector<HashMapValue>();
	}
	return *data_.array_value;
}

const std::vector<HashMapValue> &HashMapValue::asArray(void) const
{
	if (type_ != HASHMAP_ARRAY)
		throw std::runtime_error("HashMapValue is not an array");
	return *data_.array_value;
}

HashMap HashMapValue::asObject(void) const
{
	if (type_ != HASHMAP_OBJECT)
		throw std::runtime_error("HashMapValue is not an object");

	HashMap result;
	for (std::map<std::string, HashMapValue>::const_iterator it = data_.object_value->begin();
		 it != data_.object_value->end(); ++it)
	{
		result.set(it->first, it->second);
	}
	return result;
}

HashMapValue &HashMapValue::operator[](size_t index)
{
	if (type_ != HASHMAP_ARRAY)
	{
		cleanup();
		type_ = HASHMAP_ARRAY;
		data_.array_value = new std::vector<HashMapValue>();
	}

	if (index >= data_.array_value->size())
		data_.array_value->resize(index + 1);

	return (*data_.array_value)[index];
}

HashMapValue &HashMapValue::operator[](const std::string &key)
{
	if (type_ != HASHMAP_OBJECT)
	{
		cleanup();
		type_ = HASHMAP_OBJECT;
		data_.object_value = new std::map<std::string, HashMapValue>();
	}
	return (*data_.object_value)[key];
}

HashMapValue &HashMapValue::operator[](const char *key)
{
	return (*this)[std::string(key)];
}