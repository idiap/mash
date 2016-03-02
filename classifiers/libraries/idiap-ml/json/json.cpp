/*******************************************************************************
* The MASH Framework contains the source code of all the servers in the
* "computation farm" of the MASH project (http://www.mash-project.eu),
* developed at the Idiap Research Institute (http://www.idiap.ch).
*
* Copyright (c) 2016 Idiap Research Institute, http://www.idiap.ch/
* Written by Charles Dubout (charles.dubout@idiap.ch)
*
* This file is part of the MASH Framework.
*
* The MASH Framework is free software: you can redistribute it and/or modify
* it under the terms of either the GNU General Public License version 2 or
* the GNU General Public License version 3 as published by the Free
* Software Foundation, whichever suits the most your needs.
*
* The MASH Framework is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public Licenses
* along with the MASH Framework. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/


/// \file json.cpp
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date 2010.03.31

#include "json.h"

#include <iostream>
#include <stdexcept>

using namespace json;

Value::Value() : type_(JSON_NULL) {}

Value::Value(const char* str) : type_(JSON_STRING) {
	string_ = new String(str);
}

Value::Value(const String& str) : type_(JSON_STRING) {
	string_ = new String(str);
}

Value::Value(bool b) : type_(b ? JSON_TRUE : JSON_FALSE) {}

Value::Value(const Value& value) : type_(value.type_) {
	switch (value.type_) {
		case JSON_STRING: string_ = new String(*value.string_); break;
		case JSON_NUMBER: number_ = value.number_; break;
		case JSON_OBJECT: object_ = new Object(*value.object_); break;
		case JSON_ARRAY: array_ = new Array(*value.array_); break;
		default: break;
	}
}

Value::~Value() {
	clear();
}

Value& Value::operator=(const Value& value) {
	clear();

	switch (value.type_) {
		case JSON_STRING: string_ = new String(*value.string_); break;
		case JSON_NUMBER: number_ = value.number_; break;
		case JSON_OBJECT: object_ = new Object(*value.object_); break;
		case JSON_ARRAY: array_ = new Array(*value.array_); break;
		default: break;
	}

	type_ = value.type_;

	return *this;
}

void Value::clear() {
	switch (type_) {
		case JSON_STRING: delete string_; break;
		case JSON_OBJECT: delete object_; break;
		case JSON_ARRAY: delete array_; break;
		default: break;
	}

	type_ = JSON_NULL;
}

Value::String& Value::string() {
	if (type_ != JSON_STRING) {
		clear();
		string_ = new String;
		type_ = JSON_STRING;
	}

	return *string_;
}

const Value::String& Value::string() const {
	if (type_ != JSON_STRING) {
		throw std::runtime_error("invalid cast to string");
	}

	return *string_;
}

Value::Number& Value::number() {
	if (type_ != JSON_NUMBER) {
		clear();
		type_ = JSON_NUMBER;
	}

	return number_;
}

Value::Number Value::number() const {
	if (type_ != JSON_NUMBER) {
		// Automatic conversion from boolean
		if (type_ == JSON_TRUE) {
			return 1;
		}
		else if (type_ == JSON_FALSE) {
			return 0;
		}

		throw std::runtime_error("invalid cast to number");
	}

	return number_;
}

Value::Object& Value::object() {
	if (type_ != JSON_OBJECT) {
		clear();
		object_ = new Object;
		type_ = JSON_OBJECT;
	}

	return *object_;
}

const Value::Object& Value::object() const {
	if (type_ != JSON_OBJECT) {
		throw std::runtime_error("invalid cast to object");
	}

	return *object_;
}

Value::Array& Value::array() {
	if (type_ != JSON_ARRAY) {
		clear();
		array_ = new Array;
		type_ = JSON_ARRAY;
	}

	return *array_;
}

const Value::Array& Value::array() const {
	if (type_ != JSON_ARRAY) {
		throw std::runtime_error("invalid cast to array");
	}

	return *array_;
}

bool Value::boolean() const {
	return type_ < JSON_FALSE;
}

bool Value::parse(std::istream& in) {
	// Advance to the first non space character
	while (in && std::isspace(in.peek())) {
		in.get();
	}

	if (!in) {
		throw std::ios::failure("parse error in Value::parse");
	}

	return parseString(in) || parseNumber(in) || parseObject(in) ||
		   parseArray(in) || parseTrue(in) || parseFalse(in) || parseNull(in);
}

void Value::write(std::ostream& out) const {
	switch (type_) {
		case JSON_STRING: writeString(out); break;
		case JSON_NUMBER: out << number_; break;
		case JSON_OBJECT: writeObject(out); break;
		case JSON_ARRAY: writeArray(out); break;
		case JSON_TRUE: out.write("true", 4); break;
		case JSON_FALSE: out.write("false", 5); break;
		default: out.write("null", 4);
	}
}

bool Value::parseString(std::istream& in) {
	// If the first character is not a quote it is not a string
	if (in.peek() != '\"') {
		return false;
	}

	// Get the quote
	in.get();

	// Get the string
	std::string str;
	str.reserve(16);

	while (in && in.peek() != '\"') {
		int c = in.get();

		if (c != '\\') {
			str += c;
		}
		else {
			c = in.get();

			switch (c) {
				case '\"': str += '\"'; break;
				case '\\': str += '\\'; break;
				case '/': str += '/'; break;
				case 'b': str += '\b'; break;
				case 'f': str += '\f'; break;
				case 'n': str += '\n'; break;
				case 'r': str += '\r'; break;
				case 't': str += '\t'; break;
				default: throw std::runtime_error("invalid escaped sequence");
			}
		}
	}

	// Get the quote
	in.get();

	if (!in) {
		throw std::ios::failure("parse error in Value::parseString");
	}

	clear();
	string_ = new String;
	string_->swap(str);
	type_ = JSON_STRING;

	return true;
}

bool Value::parseNumber(std::istream& in) {
	// The first non space character
	int c = in.peek();

	// If that character is not a digit or a '-' it is not a number
	if (!(c >= '0' && c <= '9') && c != '-') {
		return false;
	}

	// Get the number
	double num;

	if (!(in >> num)) {
		throw std::ios::failure("parse error in Value::parseNumber");
	}

	(*this) = num;

	return true;
}

bool Value::parseObject(std::istream& in) {
	// If the first character is not a brace it is not an object
	if (in.peek() != '{') {
		return false;
	}

	// Get the bracket
	in.get();

	// Create an empty object
	Object obj;

	// Advance to the first non space character
	while (in && std::isspace(in.peek())) {
		in.get();
	}

	if (!in) {
		throw std::ios::failure("parse error in Value::parseObject");
	}

	if (in.peek() != '}') {
		for (;;) {
			// Parse the name
			Value name;

			if (!name.parseString(in)) {
				throw std::ios::failure("parse error in Value::parseObject");
			}

			// Parse the colon
			char colon;

			if (!(in >> colon) || colon != ':') {
				throw std::ios::failure("parse error in Value::parseObject");
			}

			// Parse the value
			Value val;

			if (!val.parse(in)) {
				throw std::ios::failure("parse error in Value::parseObject");
			}

			obj[name.string()] = val;

			// Parse the delimiter
			char delim;

			if (!(in >> delim) || (delim != ',' && delim != '}')) {
				throw std::ios::failure("parse error in Value::parseObject");
			}

			if (delim == '}') {
				break;
			}

			// Advance to the first non space character
			while (in && std::isspace(in.peek())) {
				in.get();
			}

			if (!in) {
				throw std::ios::failure("parse error in Value::parseObject");
			}
		}
	}

	clear();
	object_ = new Object;
	object_->swap(obj);
	type_ = JSON_OBJECT;

	return true;
}

bool Value::parseArray(std::istream& in) {
	// If the first character is not a bracket it is not an object
	if (in.peek() != '[') {
		return false;
	}

	// Get the bracket
	in.get();

	// Create an empty array
	Array arr;

	// Advance to the first non space character
	while (in && std::isspace(in.peek())) {
		in.get();
	}

	if (!in) {
		throw std::ios::failure("parse error in Value::parseArray");
	}

	if (in.peek() != ']') {
		for (;;) {
			// Parse the value
			Value value;

			if (!value.parse(in)) {
				throw std::ios::failure("parse error in Value::parseArray");
			}

			arr.push_back(value);

			// Parse the delimiter
			char delim;

			if (!(in >> delim) || (delim != ',' && delim != ']')) {
				throw std::ios::failure("parse error in Value::parseArray");
			}

			if (delim == ']') {
				break;
			}

			// Advance to the first non space character
			while (in && std::isspace(in.peek())) {
				in.get();
			}

			if (!in) {
				throw std::ios::failure("parse error in Value::parseArray");
			}
		}
	}

	clear();
	array_ = new Array;
	array_->swap(arr);
	type_ = JSON_ARRAY;

	return true;
}

bool Value::parseTrue(std::istream& in) {
	// If the first character is not a 't' it is not true
	if (in.peek() != 't') {
		return false;
	}

	in.get(); // Get the 't'

	if (in.get() != 'r' || in.get() != 'u' || in.get() != 'e') {
		throw std::ios::failure("parse error in Value::parseTrue");
	}

	clear();
	type_ = JSON_TRUE;

	return true;
}

bool Value::parseFalse(std::istream& in) {
	// If the first character is not a 'f' it is not false
	if (in.peek() != 'f') {
		return false;
	}

	in.get(); // Get the 'f'

	if (in.get() != 'a' || in.get() != 'l' || in.get() != 's' ||
		in.get() != 'e') {
		throw std::ios::failure("parse error in Value::parseFalse");
	}

	clear();
	type_ = JSON_FALSE;

	return true;
}

bool Value::parseNull(std::istream& in) {
	// If the first character is not a 'n' it is not null
	if (in.peek() != 'n') {
		return false;
	}

	in.get(); // Get the 'n'

	if (in.get() != 'u' || in.get() != 'l' || in.get() != 'l') {
		throw std::ios::failure("parse error in Value::parseNull");
	}

	clear();

	return true;
}

void Value::writeString(std::ostream& out) const {
	out.put('\"');

	for (std::size_t i = 0; i < string_->length(); ++i) {
		char c = (*string_)[i];

		if (c >= '#' || c == '!') {
			out.put(c);
		}
		else {
			switch (c) {
				case '\"': out.write("\\\"", 2); break;
				case '\\': out.write("\\\\", 2); break;
				case '/': out.write("\\/", 2); break;
				case '\b': out.write("\\b", 2); break;
				case '\f': out.write("\\f", 2); break;
				case '\n': out.write("\\n", 2); break;
				case '\r': out.write("\\r", 2); break;
				case '\t': out.write("\\t", 2); break;
				default: throw std::runtime_error("invalid escaped sequence");
			}
		}
	}

	out.put('\"');
}

void Value::writeObject(std::ostream& out) const {
	static int depth = 0;

	out.write("{\n", 2);

	++depth;

	std::size_t longest = 0;

	for (Object::const_iterator it = object_->begin(), end = object_->end();
		 it != end; ++it) {
		longest = std::max(longest, it->first.length());
	}

	for (Object::const_iterator it = object_->begin(), end = object_->end();
		 it != end;) {
		for (int d = 0; d < depth; ++d) {
			out.put('\t');
		}

		Value name(it->first);
		name.writeString(out);

		out.put(':');

		for (std::size_t l = it->first.length(); l <= longest; ++l) {
			out.put(' ');
		}

		it->second.write(out);

		if (++it != end) {
			out.write(",\n", 2);
		}
		else {
			out.put('\n');
		}
	}

	--depth;

	for (int d = 0; d < depth; ++d) {
		out.put('\t');
	}

	out.put('}');
}

void Value::writeArray(std::ostream& out) const {
	out.put('[');

	for (Array::const_iterator it = array_->begin(), end = array_->end();
		 it != end;) {
		it->write(out);

		if (++it != end) {
			out.write(", ", 2);
		}
	}

	out.put(']');
}

std::istream& json::operator>>(std::istream& in, Value& value) {
	if (!value.parse(in)) {
		throw std::runtime_error("parse error");
	}

	return in;
}

std::ostream& json::operator<<(std::ostream& out, const Value& value) {
	value.write(out);
	return out;
}
