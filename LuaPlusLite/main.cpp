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

#define logprintf printf

#define TEST(STRING_NAME) \
    { \
        const char * testName = STRING_NAME; \
        auto testFunction = [] () { \
            LuaState myLuaState; \
            lua_State * myLuaState_CState = myLuaState.GetCState(); \
            if (0) { myLuaState_CState = NULL; }    /* prevent unused-variable warnings */

#define TEST_END \
        }; \
        logprintf("%s\n", testName); \
        testFunction(); \
    }

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
	logprintf("... checking type via Is methods:\n");
	logprintf("....... IsBoolean?: %d\n", (int)obj.IsBoolean());
	CHECK(obj.IsBoolean() == (actual == LUA_TBOOLEAN));
	logprintf("....... IsCFunction?: %d\n", (int)obj.IsCFunction());
	//CHECK(obj.IsCFunction() == (actual == LUA_TFUNCTION));	// Not applied as the value might be a non-C function.
	logprintf("....... IsFunction?: %d\n", (int)obj.IsFunction());
	CHECK(obj.IsFunction() == (actual == LUA_TFUNCTION));
	logprintf("....... IsInteger?: %d\n", (int)obj.IsInteger());
#if LuaPlusLite__IsString_and_IsNumber_only_match_explicitly == 1
	CHECK(obj.IsInteger() == (actual == LUA_TNUMBER));
#else
	CHECK(obj.IsInteger() == (actual == LUA_TNUMBER || (actual == LUA_TSTRING && is_string_convertible_to_number(obj.ToString()))));
#endif
	logprintf("....... IsLightUserData?: %d\n", (int)obj.IsLightUserData());
	CHECK(obj.IsLightUserData() == (actual == LUA_TLIGHTUSERDATA));
	logprintf("....... IsNil?: %d\n", (int)obj.IsNil());
	CHECK(obj.IsNil() == (actual == LUA_TNIL));
	logprintf("....... IsNone?: %d\n", (int)obj.IsNone());
	CHECK(obj.IsNone() == (actual == LUA_TNONE));
	logprintf("....... IsNoneOrNil?: %d\n", (int)obj.IsNoneOrNil());
	CHECK(obj.IsNoneOrNil() == (actual == LUA_TNONE || actual == LUA_TNIL));
	logprintf("....... IsNumber?: %d\n", (int)obj.IsNumber());
#if LuaPlusLite__IsString_and_IsNumber_only_match_explicitly == 1
	CHECK(obj.IsNumber() == (actual == LUA_TNUMBER));
#else
	CHECK(obj.IsNumber() == (actual == LUA_TNUMBER || (actual == LUA_TSTRING && is_string_convertible_to_number(obj.ToString()))));
#endif
	logprintf("....... IsString?: %d\n", (int)obj.IsString());
#if LuaPlusLite__IsString_and_IsNumber_only_match_explicitly == 1
	CHECK(obj.IsString() == (actual == LUA_TSTRING));
#else
	CHECK(obj.IsString() == (actual == LUA_TSTRING || actual == LUA_TNUMBER));
#endif
	logprintf("....... IsTable?: %d\n", (int)obj.IsTable());
	CHECK(obj.IsTable() == (actual == LUA_TTABLE));
	logprintf("....... IsThread?: %d\n", (int)obj.IsThread());
	CHECK(obj.IsThread() == (actual == LUA_TTHREAD));
	logprintf("....... IsUserData?: %d\n", (int)obj.IsUserData());
	CHECK(obj.IsUserData() == (actual == LUA_TUSERDATA || actual == LUA_TLIGHTUSERDATA));
}

int main(int argc, const char * argv[])
{
	logprintf("Welcome to LuaPlusLite!\n");
	srand(0);
	
    TEST("Retrieving C-State") {
        //lua_State * myLuaState_CState = myLuaState.GetCState();
        logprintf("... Inner lua_State is %p\n", myLuaState_CState);
        logprintf("... Wrapper (a LuaState) is at %p\n", &myLuaState);
        CHECK(myLuaState_CState != NULL);
    } TEST_END;
	
    TEST("Retrieving the C++ LuaState via the wrapped, C-based, lua_State") {
        LuaState * luaStateFromInnerState = LuaState::CastState(myLuaState_CState);
        logprintf("... LuaState from inner, C-based, lua_State: %p\n", luaStateFromInnerState);
        CHECK(luaStateFromInnerState == &myLuaState);
        CHECK(lua_gettop(myLuaState_CState) == 0);
    } TEST_END;
	
    TEST("Assigning and retrieving a random integer") {
        lua_Integer random_number = rand();
        logprintf("... random number: %td\n", random_number);
        LuaObject myEncodedInteger;
        myEncodedInteger.AssignInteger(&myLuaState, random_number);
        logprintf("... encoded type number is %d\n", myEncodedInteger.Type());
        CHECK(myEncodedInteger.Type() == LUA_TNUMBER);
        logprintf("... encoded type name is %s\n", myEncodedInteger.TypeName());
        CHECK(strcmp(myEncodedInteger.TypeName(), "number") == 0);
        lua_Integer decoded_number = myEncodedInteger.ToInteger();
        logprintf("... decoded number is %td\n", decoded_number);
        CHECK(random_number == decoded_number);
        log_and_check_types_via_Is_methods(myEncodedInteger, LUA_TNUMBER);
        CHECK(lua_gettop(myLuaState_CState) == 0);
    } TEST_END;
	
    TEST("Assigning and retrieving a random number") {
		lua_Number random_number = ((lua_Number)rand() / (lua_Number)rand());
		logprintf("... random number: %f\n", random_number);
		LuaObject myEncodedNumber;
		myEncodedNumber.AssignNumber(&myLuaState, random_number);
		logprintf("... encoded type number is %d\b", myEncodedNumber.Type());
		CHECK(myEncodedNumber.Type() == LUA_TNUMBER);
		logprintf("... encoded type name is %s\n", myEncodedNumber.TypeName());
		CHECK(strcmp(myEncodedNumber.TypeName(), "number") == 0);
		lua_Number decoded_number = myEncodedNumber.ToNumber();
		logprintf("... decoded number is %f", decoded_number);
		CHECK(random_number == decoded_number);
		log_and_check_types_via_Is_methods(myEncodedNumber, LUA_TNUMBER);
		CHECK(lua_gettop(myLuaState_CState) == 0);
    } TEST_END;
	
    TEST("Copying LuaObject containing a previously-generated, random integer") {
        lua_Integer random_number = rand();
        logprintf("... random number: %td\n", random_number);
        LuaObject myEncodedInteger;
        myEncodedInteger.AssignInteger(&myLuaState, random_number);
        LuaObject copyOfEncodedInteger(myEncodedInteger);
        logprintf("... encoded type number is %d\n", copyOfEncodedInteger.Type());
        CHECK(copyOfEncodedInteger.Type() == LUA_TNUMBER);
        logprintf("... encoded type name is %s\n", copyOfEncodedInteger.TypeName());
        CHECK(strcmp(copyOfEncodedInteger.TypeName(), "number") == 0);
        lua_Integer copy_of_decoded_number = copyOfEncodedInteger.ToInteger();
        logprintf("... decoded number is %td\n", copy_of_decoded_number);
        CHECK(random_number == copy_of_decoded_number);
        log_and_check_types_via_Is_methods(copyOfEncodedInteger, LUA_TNUMBER);
        CHECK(lua_gettop(myLuaState_CState) == 0);
    } TEST_END;
	
	TEST("Assigning and retrieving the previously-generated random integer via use of the Lua stack") {
        lua_Integer random_number = rand();
        logprintf("... random number: %td\n", random_number);
        lua_pushinteger(myLuaState.GetCState(), random_number);
        LuaObject stackMadeEncodedInteger(&myLuaState, -1);
        lua_pop(myLuaState.GetCState(), 1);
        logprintf("... encoded type number is %d\n", stackMadeEncodedInteger.Type());
        CHECK(stackMadeEncodedInteger.Type() == LUA_TNUMBER);
        logprintf("... encoded type name is %s\n", stackMadeEncodedInteger.TypeName());
        CHECK(strcmp(stackMadeEncodedInteger.TypeName(), "number") == 0);
        lua_Integer stack_made_decoded_number = stackMadeEncodedInteger.ToInteger();
        logprintf("... decoded number is %td\n", stack_made_decoded_number);
        CHECK(random_number == stack_made_decoded_number);
        log_and_check_types_via_Is_methods(stackMadeEncodedInteger, LUA_TNUMBER);
        CHECK(lua_gettop(myLuaState_CState) == 0);
    } TEST_END;
	
    TEST("Assigning and retrieving a random string") {
        char random_string[16];
        const size_t num_chars_in_random_string = sizeof(random_string) / sizeof(char);
        for (int i = 0; i < (num_chars_in_random_string - 1); i++) {
            int ch = rand() % 26;
            ch += 'A';
            random_string[i] = ch;
        }
        random_string[num_chars_in_random_string - 1] = 0;
        logprintf("%s\n", random_string);
        LuaObject myEncodedString;
        myEncodedString.AssignString(&myLuaState, random_string);
        logprintf("... encoded type number is %d\n", myEncodedString.Type());
        CHECK(myEncodedString.Type() == LUA_TSTRING);
        logprintf("... encoded type name is %s\n", myEncodedString.TypeName());
        CHECK(strcmp(myEncodedString.TypeName(), "string") == 0);
        const char * decoded_string = myEncodedString.ToString();
        logprintf("... decoded string is %s\n", decoded_string);
        CHECK(decoded_string != NULL);
        CHECK(strcmp(random_string, decoded_string) == 0);
        log_and_check_types_via_Is_methods(myEncodedString, LUA_TSTRING);
        CHECK(lua_gettop(myLuaState_CState) == 0);
    } TEST_END;
	
    TEST("Assigning and retrieving a nil value") {
        LuaObject myNilObject;
        myNilObject.AssignNil(&myLuaState);
        logprintf("... encoded type number is %d\n", myNilObject.Type());
        CHECK(myNilObject.Type() == LUA_TNIL);
        logprintf("... encoded type name is %s\n", myNilObject.TypeName());
        CHECK(strcmp(myNilObject.TypeName(), "nil") == 0);
        log_and_check_types_via_Is_methods(myNilObject, LUA_TNIL);
        CHECK(lua_gettop(myLuaState_CState) == 0);
    } TEST_END;
	
    TEST("Inspecting an uninitialized LuaObject") {
		LuaObject uninitializedObject;
		logprintf("... encoded type number is %d\n", uninitializedObject.Type());
		CHECK(uninitializedObject.Type() == LUA_TNONE);
		logprintf("... encoded type name: %s\n", uninitializedObject.TypeName());
		CHECK(strcmp(uninitializedObject.TypeName(), "no value") == 0);
		log_and_check_types_via_Is_methods(uninitializedObject, LUA_TNONE);
    } TEST_END;
	
	TEST("Assigning and retrieving a boolean true value") {
		LuaObject encoded_boolean;
		encoded_boolean.AssignBoolean(&myLuaState, true);
		logprintf("... encoded type number is %d\n", encoded_boolean.Type());
		CHECK(encoded_boolean.Type() == LUA_TBOOLEAN);
		logprintf("... encoded type name: %s\n", encoded_boolean.TypeName());
		CHECK(strcmp(encoded_boolean.TypeName(), "boolean") == 0);
		bool decoded_boolean = encoded_boolean.ToBoolean();
		logprintf("... decoded value: %d\n", (int)decoded_boolean);
		log_and_check_types_via_Is_methods(encoded_boolean, LUA_TBOOLEAN);
	} TEST_END;

	TEST("Assigning and retrieving a boolean false value") {
		LuaObject encoded_boolean;
		encoded_boolean.AssignBoolean(&myLuaState, false);
		logprintf("... encoded type number is %d\n", encoded_boolean.Type());
		CHECK(encoded_boolean.Type() == LUA_TBOOLEAN);
		logprintf("... encoded type name: %s\n", encoded_boolean.TypeName());
		CHECK(strcmp(encoded_boolean.TypeName(), "boolean") == 0);
		bool decoded_boolean = encoded_boolean.ToBoolean();
		logprintf("... decoded value: %d\n", (int)decoded_boolean);
		log_and_check_types_via_Is_methods(encoded_boolean, LUA_TBOOLEAN);
	} TEST_END;
	
    TEST("Assigning a new table") {
        LuaObject myTable;
        myTable.AssignNewTable(&myLuaState);
        logprintf("... encoded type number is %d\n", myTable.Type());
        CHECK(myTable.Type() == LUA_TTABLE);
        logprintf("... encoded type name is %s\n", myTable.TypeName());
        CHECK(strcmp(myTable.TypeName(), "table") == 0);
        log_and_check_types_via_Is_methods(myTable, LUA_TTABLE);
        CHECK(lua_gettop(myLuaState_CState) == 0);
    } TEST_END;
	
    TEST("Creating and retriving a table entry using a random string as a key and a random number as a value") {
		int value = rand();
		string key = get_random_string();
        LuaObject myTable;
        myTable.AssignNewTable(&myLuaState);
        logprintf("... key: %s\n", key.c_str());
        logprintf("... value: %d\n", value);
		myTable.SetInteger(key.c_str(), value);
		LuaObject encoded_value_1 = myTable.GetByName(key.c_str());
		logprintf("... encoded value type via GetByName: %d\n", encoded_value_1.Type());
		CHECK(encoded_value_1.Type() == LUA_TNUMBER);
		logprintf("... encoded value type name via GetByName: %s\n", encoded_value_1.TypeName());
		CHECK(strcmp(encoded_value_1.TypeName(), "number") == 0);
		logprintf("... decoded value via GetByName: %td\n", encoded_value_1.ToInteger());
		CHECK(encoded_value_1.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_1, LUA_TNUMBER);
		LuaObject encoded_value_2 = myTable[key.c_str()];
		logprintf("... encoded value type via operator[]: %d\n", encoded_value_2.Type());
		CHECK(encoded_value_2.Type() == LUA_TNUMBER);
		logprintf("... encoded value type name via operator[]: %s\n", encoded_value_2.TypeName());
		CHECK(strcmp(encoded_value_2.TypeName(), "number") == 0);
		logprintf("... decoded value via operator[]: %td\n", encoded_value_2.ToInteger());
		CHECK(encoded_value_2.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_2, LUA_TNUMBER);
	} TEST_END;
	
    TEST("Creating and retriving a table entry using a random number as a key and another random number as a value") {
		int key = rand();
		int value = rand();
        logprintf("... key: %d\n", key);
        logprintf("... value: %d\n", value);
		LuaObject myTable;
		myTable.AssignNewTable(&myLuaState);
		myTable.SetInteger(key, value);

		LuaObject encoded_value_1 = myTable.GetByIndex(key);
		logprintf("... encoded value type via GetByIndex: %d\n", encoded_value_1.Type());
		CHECK(encoded_value_1.Type() == LUA_TNUMBER);
		logprintf("... encoded value type name via GetByIndex: %s\n", encoded_value_1.TypeName());
		CHECK(strcmp(encoded_value_1.TypeName(), "number") == 0);
		logprintf("... decoded value via GetByIndex: %td\n", encoded_value_1.ToInteger());
		CHECK(encoded_value_1.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_1, LUA_TNUMBER);
		LuaObject encoded_value_2 = myTable[key];
		logprintf("... encoded value type via operator[]: %d\n", encoded_value_2.Type());
		CHECK(encoded_value_2.Type() == LUA_TNUMBER);
		logprintf("... encoded value type name via operator[]: %s\n", encoded_value_2.TypeName());
		CHECK(strcmp(encoded_value_2.TypeName(), "number") == 0);
		logprintf("... decoded value via operator[]: %td\n", encoded_value_2.ToInteger());
		CHECK(encoded_value_2.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_2, LUA_TNUMBER);
		CHECK(myLuaState.GetTop() == 0);
	} TEST_END;
	
    TEST("Creating and retriving a table entry using a pointer as a key and a random number as a value") {
		int dummy = 0;
		void * raw_key = (void *)&dummy;
		int value = rand();
        logprintf("... key: %p\n", raw_key);
        logprintf("... value: %d\n", value);
		LuaObject myTable;
		myTable.AssignNewTable(&myLuaState);
		LuaObject key;
		key.AssignLightUserData(&myLuaState, raw_key);
		myTable.SetInteger(key, value);
		
		LuaObject encoded_value_1 = myTable.GetByObject(key);
		logprintf("... encoded value type via GetByObject: %d\n", encoded_value_1.Type());
		CHECK(encoded_value_1.Type() == LUA_TNUMBER);
		logprintf("... encoded value type name via GetByObject: %s\n", encoded_value_1.TypeName());
		CHECK(strcmp(encoded_value_1.TypeName(), "number") == 0);
		logprintf("... decoded value via GetByObject: %td\n", encoded_value_1.ToInteger());
		CHECK(encoded_value_1.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_1, LUA_TNUMBER);
		LuaObject encoded_value_2 = myTable[key];
		logprintf("... encoded value type via operator[]: %d\n", encoded_value_2.Type());
		CHECK(encoded_value_2.Type() == LUA_TNUMBER);
		logprintf("... encoded value type name via operator[]: %s\n", encoded_value_2.TypeName());
		CHECK(strcmp(encoded_value_2.TypeName(), "number") == 0);
		logprintf("... decoded value via operator[]: %td\n", encoded_value_2.ToInteger());
		CHECK(encoded_value_2.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_2, LUA_TNUMBER);
		CHECK(myLuaState.GetTop() == 0);
	} TEST_END;
	
	TEST("Assigning a full userdata value to a table:") {
		string key = get_random_string();
		int dummy = 0;
		void * value = (void *)&dummy;
		logprintf("... key: %s\n", key.c_str());
		logprintf("... value: %p\n", value);
		LuaObject myTable;
		myTable.AssignNewTable(&myLuaState);
		myTable.SetUserData(key.c_str(), value);
		
		LuaObject encoded_value_1 = myTable.GetByName(key.c_str());
		logprintf("... encoded value type via GetByName: %d\n", encoded_value_1.Type());
		CHECK(encoded_value_1.Type() == LUA_TUSERDATA);
		logprintf("... encoded value type name via GetByName: %s\n", encoded_value_1.TypeName());
		CHECK(strcmp(encoded_value_1.TypeName(), "userdata") == 0);
		logprintf("... decoded value via GetByName: %p\n", encoded_value_1.ToUserData());
		CHECK(encoded_value_1.ToUserData() == value);
		log_and_check_types_via_Is_methods(encoded_value_1, LUA_TUSERDATA);
		LuaObject encoded_value_2 = myTable[key.c_str()];
		logprintf("... encoded value type via operator[]: %d\n", encoded_value_2.Type());
		CHECK(encoded_value_2.Type() == LUA_TUSERDATA);
		logprintf("... encoded value type name via operator[]: %s\n", encoded_value_2.TypeName());
		CHECK(strcmp(encoded_value_2.TypeName(), "userdata") == 0);
		logprintf("... decoded value via operator[]: %p\n", encoded_value_2.ToUserData());
		CHECK(encoded_value_2.ToUserData() == value);
		log_and_check_types_via_Is_methods(encoded_value_2, LUA_TUSERDATA);
		CHECK(myLuaState.GetTop() == 0);
	} TEST_END;
	
	TEST("Assigning a light userdata value to a table") {
		string key = get_random_string();
		int dummy = 0;
		void * value = (void *)&dummy;
		logprintf("... key: %s\n", key.c_str());
		logprintf("... value: %p\n", value);
		LuaObject myTable;
		myTable.AssignNewTable(&myLuaState);
		myTable.SetLightUserData(key.c_str(), value);
		
		LuaObject encoded_value_1 = myTable.GetByName(key.c_str());
		logprintf("... encoded value type via GetByName: %d\n", encoded_value_1.Type());
		CHECK(encoded_value_1.Type() == LUA_TLIGHTUSERDATA);
		logprintf("... encoded value type name via GetByName: %s\n", encoded_value_1.TypeName());
		CHECK(strcmp(encoded_value_1.TypeName(), "userdata") == 0);
		logprintf("... decoded value via GetByName: %p\n", encoded_value_1.ToUserData());
		CHECK(encoded_value_1.ToUserData() == value);
		log_and_check_types_via_Is_methods(encoded_value_1, LUA_TLIGHTUSERDATA);
		LuaObject encoded_value_2 = myTable[key.c_str()];
		logprintf("... encoded value type via operator[]: %d\n", encoded_value_2.Type());
		CHECK(encoded_value_2.Type() == LUA_TLIGHTUSERDATA);
		logprintf("... encoded value type name via operator[]: %s\n", encoded_value_2.TypeName());
		CHECK(strcmp(encoded_value_2.TypeName(), "userdata") == 0);
		logprintf("... decoded value via operator[]: %p\n", encoded_value_2.ToUserData());
		CHECK(encoded_value_2.ToUserData() == value);
		log_and_check_types_via_Is_methods(encoded_value_2, LUA_TLIGHTUSERDATA);
		CHECK(myLuaState.GetTop() == 0);
	} TEST_END;

    TEST("Retrieving a new random number from the stack, as accessed by LuaState::Stack()") {
        int random_number_2 = rand();
        logprintf("... random number: %d\n", random_number_2);
		int original_stack_top = myLuaState.GetTop();
		logprintf("... original stack top: %d\n", original_stack_top);
		lua_pushinteger(myLuaState_CState, random_number_2);
		int new_stack_top = myLuaState.GetTop();
		logprintf("... new stack top: %d\n", new_stack_top);
		CHECK(new_stack_top = original_stack_top + 1);
		int type_from_Stack_position_1 = myLuaState.Stack(1).Type();
		logprintf("... type from Stack(1): %d\n", type_from_Stack_position_1);
		const char * type_name_from_stack_position_1 = myLuaState.Stack(1).TypeName();
		logprintf("... type name from Stack(1): %s\n", type_name_from_stack_position_1);
		CHECK(strcmp(type_name_from_stack_position_1, "number") == 0);
		int decoded_integer_from_Stack_method = myLuaState.Stack(1).ToInteger();
		logprintf("... decoded integer from Stack(1): %d\n", decoded_integer_from_Stack_method);
		CHECK(decoded_integer_from_Stack_method == random_number_2);
//		log_and_check_types_via_Is_methods<LuaObject>(myLuaState.Stack(1), LUA_TNUMBER);
		myLuaState.Pop(1);
	} TEST_END;
	
	TEST("Retrieving global table") {
		LuaObject allGlobals = myLuaState.GetGlobals();
		logprintf("... type: %d\n", allGlobals.Type());
		CHECK(allGlobals.Type() == LUA_TTABLE);
		logprintf("... type name: %s\n", allGlobals.TypeName());
		CHECK(strcmp(allGlobals.TypeName(), "table") == 0);
		log_and_check_types_via_Is_methods(allGlobals, LUA_TTABLE);
	} TEST_END;
	
	TEST("Assigning, retrieving, and clearing a global value") {
		int value = rand();
		string key = get_random_string();
		logprintf("... key (a random string): %s\n", key.c_str());
		logprintf("... value (a random integer): %d\n", value);
		myLuaState.GetGlobals().SetInteger(key.c_str(), value);
		int decoded_value_1 = myLuaState.GetGlobals().GetByName(key.c_str()).ToInteger();
		logprintf("... decoded value 1 (single-line retrieval): %d\n", decoded_value_1);
		CHECK(decoded_value_1 == value);
		LuaObject encoded_value = myLuaState.GetGlobals().GetByName(key.c_str());
		int decoded_value_2 = encoded_value.ToInteger();
		logprintf("... decoded value 2 (value from named LuaObject): %d\n", decoded_value_2);
		CHECK(decoded_value_2 == value);
	} TEST_END;
	
	TEST("Type Conversion Test") {
		LuaObject myTable;
		myTable.AssignNewTable(&myLuaState);
		myTable.SetInteger("hello", 123);
		LuaObject helloObj = myTable["hello"];
		logprintf("... initial type: %d\n", helloObj.Type());
		logprintf("... initial type name: %s\n", helloObj.TypeName());
		const char * value_as_string = helloObj.ToString();
		logprintf("... value as string: %s\n", value_as_string);
		logprintf("... new type: %d\n", helloObj.Type());
		logprintf("... new type name: %s\n", helloObj.TypeName());
#if LuaPlusLite__ToXYZ_methods_convert_internal_value_types == 1
		CHECK(helloObj.Type() == LUA_TSTRING);
		log_and_check_types_via_Is_methods(helloObj, LUA_TSTRING);
#else
		CHECK(helloObj.Type() == LUA_TNUMBER);
		log_and_check_types_via_Is_methods(helloObj, LUA_TNUMBER);
#endif
	} TEST_END;
	
	TEST("SetInteger exception test (on a non-table)") {
		LuaObject myNonTable;

		bool wasExceptionCaught = false;
		string exceptionMessage;
		try {
			myNonTable.SetInteger("foo", 456);
		} catch (LuaException & e) {
			wasExceptionCaught = true;
			exceptionMessage = e.what();
		}
		logprintf("... was exception caught on SetInteger called on uninitialized LuaObject?: %d\n", wasExceptionCaught);
		CHECK(wasExceptionCaught == true);
		logprintf("... exception message: \"%s\"\n", exceptionMessage.c_str());

		wasExceptionCaught = false;
		exceptionMessage = "";
		myNonTable.AssignInteger(&myLuaState, 123);
		try {
			myNonTable.SetInteger("foo", 456);
		} catch (LuaException & e) {
			wasExceptionCaught = true;
			exceptionMessage = e.what();
		}
		logprintf("... was exception caught on SetInteger called on integer-representing LuaObject?: %d\n", wasExceptionCaught);
		CHECK(wasExceptionCaught == true);
        logprintf("... exception message: \"%s\"\n", exceptionMessage.c_str());
	} TEST_END;
    
    if (fail_count > 0) {
        logprintf("FAIL COUNT: %d\n", fail_count);
    } else {
        logprintf("ALL CHECKS PASSED!\n");
    }
	
    return 0;
}

