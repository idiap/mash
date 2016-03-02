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


/// \file json.h
/// \author Charles Dubout (charles.dubout@idiap.ch)
/// \date 2010.03.31

#ifndef JSON_H
#define JSON_H

#include <string>
#include <map>
#include <vector>
#include <iosfwd>
#include <limits>
#include <stdexcept>

namespace json {
	/// \brief	The different possible types of a JSON value
	/// It can be a string in double quotes, or a number, or true or false or
	/// null, or an object or an array.
	enum Type {
		JSON_STRING = 0,
		JSON_NUMBER = 1,
		JSON_OBJECT = 2,
		JSON_ARRAY = 3,
		JSON_TRUE = 4,
		JSON_FALSE = 5,
		JSON_NULL = 6
	};

	// The enable_if templates are tools for controlled creation of the SFINAE
	// (Substitution Failure Is Not An Error) conditions
	namespace detail {
		template <bool, typename T> struct enable_if {};
		template <typename T> struct enable_if<true, T> { typedef T type; };
		template <typename T, typename U = T> struct IsFundamental :
		public enable_if<std::numeric_limits<T>::is_specialized, U> {};
		template <typename T, typename U = T> struct IsInteger :
		public enable_if<std::numeric_limits<T>::is_integer, U> {};
	} // namespace detail

	/// \brief	Class representing all the possible JSON values
	/// \remark	Because of C++ conversion rules it is impossible to have a
	///			boolean constructor, as otherwise all types not matching
	///			exactly the other constructors would get converted to bool
	class Value {
	public:
		typedef std::string String; ///< The type of a JSON string
		typedef double Number; ///< The type of a JSON number
		typedef std::map<String, Value> Object; ///< The type of a JSON object
		typedef std::vector<Value> Array; ///< The type of a JSON array

		/// \brief	Constructs a null JSON value
		Value();

		/// \brief	Constructs a string JSON value
		Value(const char* str);

		/// \brief	Constructs a string JSON value
		Value(const String& str);

		/// \brief	Constructs a number JSON value from any fundamental type
		template <typename T>
		Value(T num, typename detail::IsFundamental<T>::type* dummy = 0);

		/// \brief	Constructs a true or false JSON value
		Value(bool b);

		/// \brief	Copy constructor
		Value(const Value& val);

		/// \brief	Destructor
		~Value();

		/// \brief	Assignment operator
		Value& operator=(const Value& val);

		/// \brief	Clears the value and sets it to null
		void clear();

		/// \brief	Returns the type of the value
		inline Type type() const;

		/// \brief	Returns the value as a string
		String& string();

		/// \brief	Returns the value as a constant string
		const String& string() const;

		/// \brief	Returns the value as a number
		Number& number();

		/// \brief	Returns the value as a constant number
		Number number() const;

		/// \brief	Returns the value as an object
		Object& object();

		/// \brief	Returns the value as a constant object
		const Object& object() const;

		/// \brief	Returns the value as an array
		Array& array();

		/// \brief	Returns the value as a constant array
		const Array& array() const;

		/// \brief	Returns the value as a constant boolean
		bool boolean() const;

		/// \brief	Parses a value from an input stream
		bool parse(std::istream& in);

		/// \brief	Writes a value to an output stream
		void write(std::ostream& out) const;

		// Various overload and conversion operator provided as syntactic sugar
		inline operator Number& ();
		inline operator Number () const;
		inline operator String& ();
		inline operator const String& () const;
		inline Value& operator[](const char* str);
		inline const Value& operator[](const char* str) const;
		inline Value& operator[](const String& str);
		inline const Value& operator[](const String& str) const;

		// Use a template type for the index in order to make sure to never use
		// the operators of the conversions
		template <typename T> inline typename
		detail::IsInteger<T, Value&>::type operator[](T idx);

		template <typename T> inline typename
		detail::IsInteger<T, const Value&>::type operator[](T idx) const;

	private:
		// Parses a particular JSON type
		bool parseString(std::istream& in);
		bool parseNumber(std::istream& in);
		bool parseObject(std::istream& in);
		bool parseArray(std::istream& in);
		bool parseTrue(std::istream& in);
		bool parseFalse(std::istream& in);
		bool parseNull(std::istream& in);

		// Writes a particular JSON type
		void writeString(std::ostream& out) const;
		void writeObject(std::ostream& out) const;
		void writeArray(std::ostream& out) const;

#pragma pack(push, 1)
		union {
			String* string_;
			Number number_;
			Object* object_;
			Array* array_;
		};

		char type_;
#pragma pack(pop)
	};

	/// \brief	Reads a value from an input stream
	std::istream& operator>>(std::istream& in,
							 Value& value);

	/// \brief	Writes a value to an output stream
	std::ostream& operator<<(std::ostream& out,
							 const Value& value);

	// Inline methods
	template <typename T>
	Value::Value(T num, typename detail::IsFundamental<T>::type* dummy) :
	number_(num), type_(JSON_NUMBER) {}

	Type Value::type() const {
		return static_cast<Type>(type_);
	}

	Value::operator Number& () {
		return number();
	}

	Value::operator Number () const {
		return number();
	}

	Value::operator String& () {
		return string();
	}

	Value::operator const String& () const {
		return string();
	}

	Value& Value::operator[](const char* str) {
		return object()[str];
	}

	const Value& Value::operator[](const char* str) const {
		return operator[](String(str));
	}

	Value& Value::operator[](const String& str) {
		return object()[str];
	}

	const Value& Value::operator[](const String& str) const {
		Object::const_iterator it = object().find(str);

		if (it == object().end()) {
			throw std::out_of_range(str);
		}

		return it->second;
	}

	template <typename T> typename
	detail::IsInteger<T, Value&>::type Value::operator[](T idx) {
		return array()[idx];
	}

	template <typename T> typename
	detail::IsInteger<T, const Value&>::type Value::operator[](T idx) const {
		return array()[idx];
	}
} // namespace json

#endif // JSON_H
