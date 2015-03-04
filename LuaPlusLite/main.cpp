//
//  main.cpp
//  LuaPlusLite
//
//  Created by David Ludwig on 9/5/12.
//  Copyright (c) 2012 David Ludwig. All rights reserved.
//

#include <assert.h>
#include <stdlib.h>

#include <iostream>

#include "LuaPlusLite.h"

using namespace std;
using namespace LuaPlusLite;


// #includes for testing purposes only!
extern "C" {
#include "lobject.h"
}

static int fail_count = 0;

static void _check(bool condition, const char * conditionAsString, int line)
{
    if ( ! condition) {
        printf("ASSERTION FAILED: (%s) at line %d\n", conditionAsString, line);
        ++fail_count;
    }
}

#define CHECK(CONDITION) _check((bool)(CONDITION), #CONDITION, __LINE__)

static string get_random_string(int num_chars = 15) {
	string random_string;
	random_string.resize(num_chars);
	for (int i = 0; i < num_chars; i++) {
		int ch = rand() % 26;
		ch += 'A';
		random_string[i] = ch;
	}
	return random_string;
}

static bool is_string_convertible_to_number(const char * str) {
	if (str == NULL) {
		return false;
	}
	size_t len = strlen(str);
	lua_Number result = 0;
	return (luaO_str2d(str, len, &result) != 0);
}

void log_and_check_types_via_Is_methods(LuaObject & obj, int actual) {
	cout << "... checking type via Is methods:\n";
	cout << "....... IsBoolean?: " << obj.IsBoolean() << endl;
	CHECK(obj.IsBoolean() == (actual == LUA_TBOOLEAN));
	cout << "....... IsCFunction?: " << obj.IsCFunction() << endl;
	//CHECK(obj.IsCFunction() == (actual == LUA_TFUNCTION));	// Not applied as the value might be a non-C function.
	cout << "....... IsFunction?: " << obj.IsFunction() << endl;
	CHECK(obj.IsFunction() == (actual == LUA_TFUNCTION));
	cout << "....... IsInteger?: " << obj.IsInteger() << endl;
#if LuaPlusLite__IsString_and_IsNumber_only_match_explicitly == 1
	CHECK(obj.IsInteger() == (actual == LUA_TNUMBER));
#else
	CHECK(obj.IsInteger() == (actual == LUA_TNUMBER || (actual == LUA_TSTRING && is_string_convertible_to_number(obj.ToString()))));
#endif
	cout << "....... IsLightUserData?: " << obj.IsLightUserData() << endl;
	CHECK(obj.IsLightUserData() == (actual == LUA_TLIGHTUSERDATA));
	cout << "....... IsNil?: " << obj.IsNil() << endl;
	CHECK(obj.IsNil() == (actual == LUA_TNIL));
	cout << "....... IsNone?: " << obj.IsNone() << endl;
	CHECK(obj.IsNone() == (actual == LUA_TNONE));
	cout << "....... IsNoneOrNil?: " << obj.IsNoneOrNil() << endl;
	CHECK(obj.IsNoneOrNil() == (actual == LUA_TNONE || actual == LUA_TNIL));
	cout << "....... IsNumber?: " << obj.IsNumber() << endl;
#if LuaPlusLite__IsString_and_IsNumber_only_match_explicitly == 1
	CHECK(obj.IsNumber() == (actual == LUA_TNUMBER));
#else
	CHECK(obj.IsNumber() == (actual == LUA_TNUMBER || (actual == LUA_TSTRING && is_string_convertible_to_number(obj.ToString()))));
#endif
	cout << "....... IsString?: " << obj.IsString() << endl;
#if LuaPlusLite__IsString_and_IsNumber_only_match_explicitly == 1
	CHECK(obj.IsString() == (actual == LUA_TSTRING));
#else
	CHECK(obj.IsString() == (actual == LUA_TSTRING || actual == LUA_TNUMBER));
#endif
	cout << "....... IsTable?: " << obj.IsTable() << endl;
	CHECK(obj.IsTable() == (actual == LUA_TTABLE));
	cout << "....... IsThread?: " << obj.IsThread() << endl;
	CHECK(obj.IsThread() == (actual == LUA_TTHREAD));
	cout << "....... IsUserData?: " << obj.IsUserData() << endl;
	CHECK(obj.IsUserData() == (actual == LUA_TUSERDATA || actual == LUA_TLIGHTUSERDATA));
}

int main(int argc, const char * argv[])
{
	cout << "Welcome to LuaPlusLite!\n";
	srand(0);
	
	cout << "Making a LuaState\n";
	LuaState myLuaState;
	
	lua_State * myLuaState_CState = myLuaState.GetCState();
	cout << "... Done!\n";
	cout << "... Inner lua_State is " << myLuaState_CState << endl;
	cout << "... Wrapper (a LuaState) is at " << &myLuaState << endl;
	CHECK(myLuaState_CState != NULL);
	
	cout << "Retrieving the C++ LuaState via the wrapped, C-based, lua_State\n";
	LuaState * luaStateFromInnerState = LuaState::CastState(myLuaState_CState);
	cout << "... LuaState from inner, C-based, lua_State: " << luaStateFromInnerState << endl;
	CHECK(luaStateFromInnerState == &myLuaState);
	CHECK(lua_gettop(myLuaState_CState) == 0);
	
	cout << "Assigning and retrieving a random integer: ";
	lua_Integer random_number = rand();
	cout << random_number << endl;
	LuaObject myEncodedInteger;
	myEncodedInteger.AssignInteger(&myLuaState, random_number);
	cout << "... encoded type number is " << myEncodedInteger.Type() << endl;
	CHECK(myEncodedInteger.Type() == LUA_TNUMBER);
	cout << "... encoded type name " << myEncodedInteger.TypeName() << endl;
	CHECK(strcmp(myEncodedInteger.TypeName(), "number") == 0);
	lua_Integer decoded_number = myEncodedInteger.ToInteger();
	cout << "... decoded number is " << decoded_number << endl;
	CHECK(random_number == decoded_number);
	log_and_check_types_via_Is_methods(myEncodedInteger, LUA_TNUMBER);
	CHECK(lua_gettop(myLuaState_CState) == 0);
	
	{
		cout << "Assigning and retrieving a random number: ";
		lua_Number random_number = ((lua_Number)rand() / (lua_Number)rand());
		cout << random_number << endl;
		LuaObject myEncodedNumber;
		myEncodedNumber.AssignNumber(&myLuaState, random_number);
		cout << "... encoded type number is " << myEncodedNumber.Type() << endl;
		CHECK(myEncodedNumber.Type() == LUA_TNUMBER);
		cout << "... encoded type name " << myEncodedNumber.TypeName() << endl;
		CHECK(strcmp(myEncodedNumber.TypeName(), "number") == 0);
		lua_Number decoded_number = myEncodedNumber.ToNumber();
		cout << "... decoded number is " << decoded_number << endl;
		CHECK(random_number == decoded_number);
		log_and_check_types_via_Is_methods(myEncodedNumber, LUA_TNUMBER);
		CHECK(lua_gettop(myLuaState_CState) == 0);
	}
	
	cout << "Copying LuaObject containing the previously-generated, random integer: " << random_number << endl;
	LuaObject copyOfEncodedInteger(myEncodedInteger);
	cout << "... encoded type number is " << copyOfEncodedInteger.Type() << endl;
	CHECK(copyOfEncodedInteger.Type() == LUA_TNUMBER);
	cout << "... encoded type name " << copyOfEncodedInteger.TypeName() << endl;
	CHECK(strcmp(copyOfEncodedInteger.TypeName(), "number") == 0);
	lua_Integer copy_of_decoded_number = copyOfEncodedInteger.ToInteger();
	cout << "... decoded number is " << copy_of_decoded_number << endl;
	CHECK(random_number == copy_of_decoded_number);
	log_and_check_types_via_Is_methods(copyOfEncodedInteger, LUA_TNUMBER);
	CHECK(lua_gettop(myLuaState_CState) == 0);
	
	cout << "Assigning and retrieving the previously-generated random integer via use of the Lua stack.\n";
	lua_pushinteger(myLuaState.GetCState(), random_number);
	LuaObject stackMadeEncodedInteger(&myLuaState, -1);
	lua_pop(myLuaState.GetCState(), 1);
	cout << "... encoded type number is " << stackMadeEncodedInteger.Type() << endl;
	CHECK(stackMadeEncodedInteger.Type() == LUA_TNUMBER);
	cout << "... encoded type name " << stackMadeEncodedInteger.TypeName() << endl;
	CHECK(strcmp(stackMadeEncodedInteger.TypeName(), "number") == 0);
	lua_Integer stack_made_decoded_number = stackMadeEncodedInteger.ToInteger();
	cout << "... decoded number is " << stack_made_decoded_number << endl;
	CHECK(random_number == stack_made_decoded_number);
	log_and_check_types_via_Is_methods(stackMadeEncodedInteger, LUA_TNUMBER);
	CHECK(lua_gettop(myLuaState_CState) == 0);
	
	cout << "Assigning and retrieving a random string: ";
	char random_string[16];
	const size_t num_chars_in_random_string = sizeof(random_string) / sizeof(char);
	for (int i = 0; i < (num_chars_in_random_string - 1); i++) {
		int ch = rand() % 26;
		ch += 'A';
		random_string[i] = ch;
	}
	random_string[num_chars_in_random_string - 1] = 0;
	cout << random_string << endl;
	LuaObject myEncodedString;
	myEncodedString.AssignString(&myLuaState, random_string);
	cout << "... encoded type number is " << myEncodedString.Type() << endl;
	CHECK(myEncodedString.Type() == LUA_TSTRING);
	cout << "... encoded type name " << myEncodedString.TypeName() << endl;
	CHECK(strcmp(myEncodedString.TypeName(), "string") == 0);
	const char * decoded_string = myEncodedString.ToString();
	cout << "... decoded string is " << decoded_string << endl;
	CHECK(decoded_string != NULL);
	CHECK(strcmp(random_string, decoded_string) == 0);
	log_and_check_types_via_Is_methods(myEncodedString, LUA_TSTRING);
	CHECK(lua_gettop(myLuaState_CState) == 0);
	
	cout << "Assigning and retrieving a nil value\n";
	LuaObject myNilObject;
	myNilObject.AssignNil(&myLuaState);
	cout << "... encoded type number is " << myNilObject.Type() << endl;
	CHECK(myNilObject.Type() == LUA_TNIL);
	cout << "... encoded type name " << myNilObject.TypeName() << endl;
	CHECK(strcmp(myNilObject.TypeName(), "nil") == 0);
	log_and_check_types_via_Is_methods(myNilObject, LUA_TNIL);
	CHECK(lua_gettop(myLuaState_CState) == 0);
	
	{
		cout << "Inspecting an uninitialized LuaObject:\n";
		LuaObject uninitializedObject;
		cout << "... encoded type number is " << uninitializedObject.Type() << endl;
		CHECK(uninitializedObject.Type() == LUA_TNONE);
		cout << "... encoded type name: " << uninitializedObject.TypeName() << endl;
		CHECK(strcmp(uninitializedObject.TypeName(), "no value") == 0);
		log_and_check_types_via_Is_methods(uninitializedObject, LUA_TNONE);
	}
	
	{
		cout << "Assigning and retrieving a boolean true value:\n";
		LuaObject encoded_boolean;
		encoded_boolean.AssignBoolean(&myLuaState, true);
		cout << "... encoded type number is " << encoded_boolean.Type() << endl;
		CHECK(encoded_boolean.Type() == LUA_TBOOLEAN);
		cout << "... encoded type name: " << encoded_boolean.TypeName() << endl;
		CHECK(strcmp(encoded_boolean.TypeName(), "boolean") == 0);
		bool decoded_boolean = encoded_boolean.ToBoolean();
		cout << "... decoded value: " << decoded_boolean << endl;
		log_and_check_types_via_Is_methods(encoded_boolean, LUA_TBOOLEAN);
	}

	{
		cout << "Assigning and retrieving a boolean false value:\n";
		LuaObject encoded_boolean;
		encoded_boolean.AssignBoolean(&myLuaState, false);
		cout << "... encoded type number is " << encoded_boolean.Type() << endl;
		CHECK(encoded_boolean.Type() == LUA_TBOOLEAN);
		cout << "... encoded type name: " << encoded_boolean.TypeName() << endl;
		CHECK(strcmp(encoded_boolean.TypeName(), "boolean") == 0);
		bool decoded_boolean = encoded_boolean.ToBoolean();
		cout << "... decoded value: " << decoded_boolean << endl;
		log_and_check_types_via_Is_methods(encoded_boolean, LUA_TBOOLEAN);
	}
	
	cout << "Assigning a new table\n";
	LuaObject myTable;
	myTable.AssignNewTable(&myLuaState);
	cout << "... encoded type number is " << myTable.Type() << endl;
	CHECK(myTable.Type() == LUA_TTABLE);
	cout << "... encoded type name " << myTable.TypeName() << endl;
	CHECK(strcmp(myTable.TypeName(), "table") == 0);
	log_and_check_types_via_Is_methods(myTable, LUA_TTABLE);
	CHECK(lua_gettop(myLuaState_CState) == 0);
	
	{
		int value = rand();
		string key = get_random_string();
		cout << "Creating and retriving a table entry using the random string, \""
			<< key << "\", as a key and the random number, "
			<< value << ", as a value.\n";
		myTable.SetInteger(key.c_str(), value);
		LuaObject encoded_value_1 = myTable.GetByName(key.c_str());
		cout << "... encoded value type via GetByName: " << encoded_value_1.Type() << endl;
		CHECK(encoded_value_1.Type() == LUA_TNUMBER);
		cout << "... encoded value type name via GetByName: " << encoded_value_1.TypeName() << endl;
		CHECK(strcmp(encoded_value_1.TypeName(), "number") == 0);
		cout << "... decoded value via GetByName: " << encoded_value_1.ToInteger() << endl;
		CHECK(encoded_value_1.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_1, LUA_TNUMBER);
		LuaObject encoded_value_2 = myTable[key.c_str()];
		cout << "... encoded value type via operator[]: " << encoded_value_2.Type() << endl;
		CHECK(encoded_value_2.Type() == LUA_TNUMBER);
		cout << "... encoded value type name via operator[]: " << encoded_value_2.TypeName() << endl;
		CHECK(strcmp(encoded_value_2.TypeName(), "number") == 0);
		cout << "... decoded value via operator[]: " << encoded_value_2.ToInteger() << endl;
		CHECK(encoded_value_2.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_2, LUA_TNUMBER);
	}
	
	{
		int key = rand();
		int value = rand();
		cout << "Creating and retriving a table entry using the random number, "
			<< key << ", as a key and the random number, "
			<< value << ", as a value.\n";
		LuaObject myTable;
		myTable.AssignNewTable(&myLuaState);
		myTable.SetInteger(key, value);

		LuaObject encoded_value_1 = myTable.GetByIndex(key);
		cout << "... encoded value type via GetByIndex: " << encoded_value_1.Type() << endl;
		CHECK(encoded_value_1.Type() == LUA_TNUMBER);
		cout << "... encoded value type name via GetByIndex: " << encoded_value_1.TypeName() << endl;
		CHECK(strcmp(encoded_value_1.TypeName(), "number") == 0);
		cout << "... decoded value via GetByIndex: " << encoded_value_1.ToInteger() << endl;
		CHECK(encoded_value_1.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_1, LUA_TNUMBER);
		LuaObject encoded_value_2 = myTable[key];
		cout << "... encoded value type via operator[]: " << encoded_value_2.Type() << endl;
		CHECK(encoded_value_2.Type() == LUA_TNUMBER);
		cout << "... encoded value type name via operator[]: " << encoded_value_2.TypeName() << endl;
		CHECK(strcmp(encoded_value_2.TypeName(), "number") == 0);
		cout << "... decoded value via operator[]: " << encoded_value_2.ToInteger() << endl;
		CHECK(encoded_value_2.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_2, LUA_TNUMBER);
		CHECK(myLuaState.GetTop() == 0);
	}
	
	{
		int dummy = 0;
		void * raw_key = (void *)&dummy;
		int value = rand();
		cout << "Creating and retriving a table entry using a pointer, "
			<< raw_key << ", as a key and a random number, "
			<< value << ", as a value.\n";
		LuaObject myTable;
		myTable.AssignNewTable(&myLuaState);
		LuaObject key;
		key.AssignLightUserData(&myLuaState, raw_key);
		myTable.SetInteger(key, value);
		
		LuaObject encoded_value_1 = myTable.GetByObject(key);
		cout << "... encoded value type via GetByObject: " << encoded_value_1.Type() << endl;
		CHECK(encoded_value_1.Type() == LUA_TNUMBER);
		cout << "... encoded value type name via GetByObject: " << encoded_value_1.TypeName() << endl;
		CHECK(strcmp(encoded_value_1.TypeName(), "number") == 0);
		cout << "... decoded value via GetByObject: " << encoded_value_1.ToInteger() << endl;
		CHECK(encoded_value_1.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_1, LUA_TNUMBER);
		LuaObject encoded_value_2 = myTable[key];
		cout << "... encoded value type via operator[]: " << encoded_value_2.Type() << endl;
		CHECK(encoded_value_2.Type() == LUA_TNUMBER);
		cout << "... encoded value type name via operator[]: " << encoded_value_2.TypeName() << endl;
		CHECK(strcmp(encoded_value_2.TypeName(), "number") == 0);
		cout << "... decoded value via operator[]: " << encoded_value_2.ToInteger() << endl;
		CHECK(encoded_value_2.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_2, LUA_TNUMBER);
		CHECK(myLuaState.GetTop() == 0);
	}
	
	{
		cout << "Assigning a full userdata value to a table:\n";
		string key = get_random_string();
		int dummy = 0;
		void * value = (void *)&dummy;
		cout << "... key: " << key << endl;
		cout << "... value: " << value << endl;
		LuaObject myTable;
		myTable.AssignNewTable(&myLuaState);
		myTable.SetUserData(key.c_str(), value);
		
		LuaObject encoded_value_1 = myTable.GetByName(key.c_str());
		cout << "... encoded value type via GetByName: " << encoded_value_1.Type() << endl;
		CHECK(encoded_value_1.Type() == LUA_TUSERDATA);
		cout << "... encoded value type name via GetByName: " << encoded_value_1.TypeName() << endl;
		CHECK(strcmp(encoded_value_1.TypeName(), "userdata") == 0);
		cout << "... decoded value via GetByName: " << encoded_value_1.ToUserData() << endl;
		CHECK(encoded_value_1.ToUserData() == value);
		log_and_check_types_via_Is_methods(encoded_value_1, LUA_TUSERDATA);
		LuaObject encoded_value_2 = myTable[key.c_str()];
		cout << "... encoded value type via operator[]: " << encoded_value_2.Type() << endl;
		CHECK(encoded_value_2.Type() == LUA_TUSERDATA);
		cout << "... encoded value type name via operator[]: " << encoded_value_2.TypeName() << endl;
		CHECK(strcmp(encoded_value_2.TypeName(), "userdata") == 0);
		cout << "... decoded value via operator[]: " << encoded_value_2.ToUserData() << endl;
		CHECK(encoded_value_2.ToUserData() == value);
		log_and_check_types_via_Is_methods(encoded_value_2, LUA_TUSERDATA);
		CHECK(myLuaState.GetTop() == 0);
	}
	
	{
		cout << "Assigning a light userdata value to a table:\n";
		string key = get_random_string();
		int dummy = 0;
		void * value = (void *)&dummy;
		cout << "... key: " << key << endl;
		cout << "... value: " << value << endl;
		LuaObject myTable;
		myTable.AssignNewTable(&myLuaState);
		myTable.SetLightUserData(key.c_str(), value);
		
		LuaObject encoded_value_1 = myTable.GetByName(key.c_str());
		cout << "... encoded value type via GetByName: " << encoded_value_1.Type() << endl;
		CHECK(encoded_value_1.Type() == LUA_TLIGHTUSERDATA);
		cout << "... encoded value type name via GetByName: " << encoded_value_1.TypeName() << endl;
		CHECK(strcmp(encoded_value_1.TypeName(), "userdata") == 0);
		cout << "... decoded value via GetByName: " << encoded_value_1.ToUserData() << endl;
		CHECK(encoded_value_1.ToUserData() == value);
		log_and_check_types_via_Is_methods(encoded_value_1, LUA_TLIGHTUSERDATA);
		LuaObject encoded_value_2 = myTable[key.c_str()];
		cout << "... encoded value type via operator[]: " << encoded_value_2.Type() << endl;
		CHECK(encoded_value_2.Type() == LUA_TLIGHTUSERDATA);
		cout << "... encoded value type name via operator[]: " << encoded_value_2.TypeName() << endl;
		CHECK(strcmp(encoded_value_2.TypeName(), "userdata") == 0);
		cout << "... decoded value via operator[]: " << encoded_value_2.ToUserData() << endl;
		CHECK(encoded_value_2.ToUserData() == value);
		log_and_check_types_via_Is_methods(encoded_value_2, LUA_TLIGHTUSERDATA);
		CHECK(myLuaState.GetTop() == 0);
	}

	{
		int random_number_2 = rand();
		cout << "Retrieving a new random number from the stack, " << random_number_2 << ", as accessed by LuaState::Stack().\n";
		int original_stack_top = myLuaState.GetTop();
		cout << "... original stack top: " << original_stack_top << endl;
		lua_pushinteger(myLuaState_CState, random_number_2);
		int new_stack_top = myLuaState.GetTop();
		cout << "... new stack top: " << new_stack_top << endl;
		CHECK(new_stack_top = original_stack_top + 1);
		int type_from_Stack_position_1 = myLuaState.Stack(1).Type();
		cout << "... type from Stack(1): " << type_from_Stack_position_1 << endl;
		const char * type_name_from_stack_position_1 = myLuaState.Stack(1).TypeName();
		cout << "... type name from Stack(1): " << type_name_from_stack_position_1 << endl;
		CHECK(strcmp(type_name_from_stack_position_1, "number") == 0);
		int decoded_integer_from_Stack_method = myLuaState.Stack(1).ToInteger();
		cout << "... decoded integer from Stack(1): " << decoded_integer_from_Stack_method << endl;
		CHECK(decoded_integer_from_Stack_method == random_number_2);
//		log_and_check_types_via_Is_methods<LuaObject>(myLuaState.Stack(1), LUA_TNUMBER);
		myLuaState.Pop(1);
	}
	
	{
		cout << "Retrieving global table\n";
		LuaObject allGlobals = myLuaState.GetGlobals();
		cout << "... type: " << allGlobals.Type() << endl;
		CHECK(allGlobals.Type() == LUA_TTABLE);
		cout << "... type name: " << allGlobals.TypeName() << endl;
		CHECK(strcmp(allGlobals.TypeName(), "table") == 0);
		log_and_check_types_via_Is_methods(allGlobals, LUA_TTABLE);
	}
	
	{
		int value = rand();
		string key = get_random_string();
		cout << "Assigning, retrieving, and clearing a global value.\n";
		cout << "... key (a random string): " << key << endl;
		cout << "... value (a random integer): " << value << endl;
		myLuaState.GetGlobals().SetInteger(key.c_str(), value);
		int decoded_value_1 = myLuaState.GetGlobals().GetByName(key.c_str()).ToInteger();
		cout << "... decoded value 1 (single-line retrieval): " << decoded_value_1 << endl;
		CHECK(decoded_value_1 == value);
		LuaObject encoded_value = myLuaState.GetGlobals().GetByName(key.c_str());
		int decoded_value_2 = encoded_value.ToInteger();
		cout << "... decoded value 2 (value from named LuaObject): " << decoded_value_2 << endl;
		CHECK(decoded_value_2 == value);
	}
	
	{
		cout << "Type Conversion Test:\n";
		LuaObject myTable;
		myTable.AssignNewTable(&myLuaState);
		myTable.SetInteger("hello", 123);
		LuaObject helloObj = myTable["hello"];
		cout << "... initial type: " << helloObj.Type() << endl;
		cout << "... initial type name: " << helloObj.TypeName() << endl;
		const char * value_as_string = helloObj.ToString();
		cout << "... value as string: " << value_as_string << endl;
		cout << "... new type: " << helloObj.Type() << endl;
		cout << "... new type name: " << helloObj.TypeName() << endl;
#if LuaPlusLite__ToXYZ_methods_convert_internal_value_types == 1
		CHECK(helloObj.Type() == LUA_TSTRING);
		log_and_check_types_via_Is_methods(helloObj, LUA_TSTRING);
#else
		CHECK(helloObj.Type() == LUA_TNUMBER);
		log_and_check_types_via_Is_methods(helloObj, LUA_TNUMBER);
#endif
	}
	
	{
		cout << "SetInteger exception test (on a non-table):\n";
		LuaObject myNonTable;

		bool wasExceptionCaught = false;
		string exceptionMessage;
		try {
			myNonTable.SetInteger("foo", 456);
		} catch (LuaException & e) {
			wasExceptionCaught = true;
			exceptionMessage = e.what();
		}
		cout << "... was exception caught on SetInteger called on uninitialized LuaObject?: " << wasExceptionCaught << endl;
		CHECK(wasExceptionCaught == true);
		cout << "... exception message: \"" << exceptionMessage << "\"\n";

		wasExceptionCaught = false;
		exceptionMessage = "";
		myNonTable.AssignInteger(&myLuaState, 123);
		try {
			myNonTable.SetInteger("foo", 456);
		} catch (LuaException & e) {
			wasExceptionCaught = true;
			exceptionMessage = e.what();
		}
		cout << "... was exception caught on SetInteger called on integer-representing LuaObject?: " << wasExceptionCaught << endl;
		CHECK(wasExceptionCaught == true);
		cout << "... exception message: \"" << exceptionMessage << "\"\n";
	}
    
    if (fail_count > 0) {
        cout << "FAIL COUNT: " << fail_count << "\n";
    } else {
        cout << "ALL CHECKS PASSED!\n";
    }
	
    return 0;
}

