#include <UnitTest++.h>
#include <mash-utils/stringutils.h>

using namespace Mash;
using namespace std;


SUITE(StringUtilsSuite)
{
	TEST(testPositiveIntToStringConversion)
	{
		int val = 5;
		string str = StringUtils::toString(val);
		
		CHECK_EQUAL("5", str);
	}


	TEST(testNegativeIntToStringConversion)
	{
		int val = -5;
		string str = StringUtils::toString(val);
		
		CHECK_EQUAL("-5", str);
	}


	TEST(testUnsignedIntToStringConversion)
	{
		unsigned int val = 5;
		string str = StringUtils::toString(val);
		
		CHECK_EQUAL("5", str);
	}


	TEST(testPositiveFloatToStringConversion)
	{
		float val = 5.4f;
		string str = StringUtils::toString(val);
		
		CHECK_EQUAL("5.4", str);
	}


	TEST(testNegativeFloatToStringConversion)
	{
		float val = -5.4f;
		string str = StringUtils::toString(val);
		
		CHECK_EQUAL("-5.4", str);
	}


	TEST(testPositiveTimevalToStringConversion)
	{
		struct timeval val = { 5, 1234 };
		string str = StringUtils::toString(val);
		
		CHECK_EQUAL("5.001234", str);
	}


	TEST(testNegativeTimevalToStringConversion)
	{
		struct timeval val = { -5, 1234 };
		string str = StringUtils::toString(val);
		
		CHECK_EQUAL("-5.001234", str);
	}


	TEST(testStringtoPositiveIntConversion)
	{
		string str = "5";
		int val = StringUtils::parseInt(str);
		
		CHECK_EQUAL(5, val);
	}


	TEST(testStringtoNegativeIntConversion)
	{
		string str = "-5";
		int val = StringUtils::parseInt(str);
		
		CHECK_EQUAL(-5, val);
	}


	TEST(testStringtoUnsignedIntConversion)
	{
		string str = "5";
		unsigned int val = StringUtils::parseUnsignedInt(str);
		
		CHECK_EQUAL(5, val);
	}


	TEST(testStringtoPositiveFloatConversion)
	{
		string str = "5.4";
		float val = StringUtils::parseFloat(str);
		
		CHECK_CLOSE(5.4f, val, 1e-6f);
	}


	TEST(testStringtoNegativeFloatConversion)
	{
		string str = "-5.4";
		float val = StringUtils::parseFloat(str);
		
		CHECK_CLOSE(-5.4f, val, 1e-6f);
	}


	TEST(testStringtoPositiveTimevalConversion)
	{
		string str = "5.001234";
		struct timeval val = StringUtils::parseTimeval(str);
		
		CHECK_EQUAL(5, val.tv_sec);
		CHECK_EQUAL(1234, val.tv_usec);
	}


	TEST(testStringtoNegativeTimevalConversion)
	{
		string str = "-5.001234";
		struct timeval val = StringUtils::parseTimeval(str);
		
		CHECK_EQUAL(-5, val.tv_sec);
		CHECK_EQUAL(1234, val.tv_usec);
	}


	TEST(testIncompleteStringtoTimevalConversion)
	{
		string str = ".001234";
		struct timeval val = StringUtils::parseTimeval(str);
		
		CHECK_EQUAL(0, val.tv_sec);
		CHECK_EQUAL(1234, val.tv_usec);
	}


	TEST(testStringtoSecondsOnlyTimevalConversion)
	{
		string str = "10";
		struct timeval val = StringUtils::parseTimeval(str);
		
		CHECK_EQUAL(10, val.tv_sec);
		CHECK_EQUAL(0, val.tv_usec);
	}


	TEST(testStringSplitWithOneDelimiter)
	{
		string str = "ABC DEF GHI";
		tStringList result = StringUtils::split(str, " ");
		
		CHECK_EQUAL(3, result.size());
		CHECK_EQUAL("ABC", result[0]);
		CHECK_EQUAL("DEF", result[1]);
		CHECK_EQUAL("GHI", result[2]);
	}


	TEST(testStringSplitWithTwoDelimiters)
	{
		string str = "ABC DEF_GHI";
		tStringList result = StringUtils::split(str, " _");
		
		CHECK_EQUAL(3, result.size());
		CHECK_EQUAL("ABC", result[0]);
		CHECK_EQUAL("DEF", result[1]);
		CHECK_EQUAL("GHI", result[2]);
	}


	TEST(testStringSplitWithOneDelimiterAndALimit)
	{
		string str = "ABC DEF GHI";
		tStringList result = StringUtils::split(str, " ", 1);
		
		CHECK_EQUAL(2, result.size());
		CHECK_EQUAL("ABC", result[0]);
		CHECK_EQUAL("DEF GHI", result[1]);
	}


	TEST(testStringSplitNoDelimiterFound)
	{
		string str = "ABC DEF GHI";
		tStringList result = StringUtils::split(str, "_");
		
		CHECK_EQUAL(1, result.size());
		CHECK_EQUAL("ABC DEF GHI", result[0]);
	}


	TEST(testStringSplitNoDelimiterProvided)
	{
		string str = "ABC DEF GHI";
		tStringList result = StringUtils::split(str, "");
		
		CHECK_EQUAL(1, result.size());
		CHECK_EQUAL("ABC DEF GHI", result[0]);
	}


	TEST(testStringSplitMultipleDelimitersInARow)
	{
		string str = "ABC  DEF  GHI";
		tStringList result = StringUtils::split(str, " ");
		
		CHECK_EQUAL(3, result.size());
		CHECK_EQUAL("ABC", result[0]);
		CHECK_EQUAL("DEF", result[1]);
		CHECK_EQUAL("GHI", result[2]);
	}


	TEST(testReplaceAll)
	{
		string str = "ABC'DEF' 'GHI";
		string result = StringUtils::replaceAll(str, "'", "\\'");
		
		CHECK_EQUAL("ABC\\'DEF\\' \\'GHI", result);
	}


	TEST(testRightTrim)
	{
		string str = " \t ABC \t ";
		string result = StringUtils::rtrim(str);
		
		CHECK_EQUAL(" \t ABC", result);
	}


	TEST(testLeftTrim)
	{
		string str = " \t ABC \t ";
		string result = StringUtils::ltrim(str);
		
		CHECK_EQUAL("ABC \t ", result);
	}


	TEST(testTrim)
	{
		string str = " \t ABC \t ";
		string result = StringUtils::trim(str);
		
		CHECK_EQUAL("ABC", result);
	}
}
