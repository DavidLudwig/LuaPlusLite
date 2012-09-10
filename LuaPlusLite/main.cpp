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
#include <string>
using namespace std;

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#define LuaPlusLite__ToXYZ_methods_convert_internal_value_types 0
#define LuaPlusLite__IsString_and_IsNumber_only_match_explicitly 0

namespace LuaPlusLite {
	class LuaException : public std::exception {
	public:
		LuaException() {
		}
		
		LuaException(const string & message) : message_(message) {
		}
		
		virtual ~LuaException() {
		}
		
		virtual const char * what() const throw() {
			return message_.c_str();
		}
		
	private:
		std::string message_;
	};

#if defined(__clang__) || defined(__GNUC__)
	#define luapluslite_assert(expression) if (!(expression)) { throw LuaException(string("assertion failed in ") + __PRETTY_FUNCTION__ + ": (" + #expression + ")"); }
	#define luapluslite_assert_ex(expression, message) if (!(expression)) { throw LuaException(string("assertion failed in ") + __PRETTY_FUNCTION__ + ": (" + #expression + "); " + message); }
#elif defined(_MSC_VER)
	#define luapluslite_assert(expression) if (!(expression)) { throw LuaException(string("assertion failed in ") + __FUNCSIG__ + ": (" + #expression + ")"); }
	#define luapluslite_assert_ex(expression, message) if (!(expression)) { throw LuaException(string("assertion failed in ") + __FUNCSIG__ + ": (" + #expression + "); " + message); }
#else
	#define luapluslite_assert(expression) if (!(expression)) { throw LuaException(string("assertion failed in ") + __FUNCTION__ + ": (" + #expression + ")"); }
	#define luapluslite_assert_ex(expression, message) if (!(expression)) { throw LuaException(string("assertion failed in ") + __FUNCTION__ + ": (" + #expression + "); " + message); }
#endif

	static const char * LUAPLUSLITE_LUASTATE_REGISTRYSTRING = "LuaPlusLite_LuaState";
	
	class LuaObject;

	class LuaState {
	public:
		LuaState() : c_state_(NULL) {
			c_state_ = luaL_newstate();
			lua_pushstring(c_state_, LUAPLUSLITE_LUASTATE_REGISTRYSTRING);
			lua_pushlightuserdata(c_state_, this);
			lua_settable(c_state_, LUA_REGISTRYINDEX);
		}
		
		~LuaState() {
			if (c_state_) {
				lua_close(c_state_);
				c_state_ = NULL;
			}
		}
		
		static LuaState * CastState(lua_State * wrapped_c_state) {
			if (wrapped_c_state == NULL) {
				return NULL;
			}
			lua_pushstring(wrapped_c_state, LUAPLUSLITE_LUASTATE_REGISTRYSTRING);
			lua_gettable(wrapped_c_state, LUA_REGISTRYINDEX);
			if ( ! lua_islightuserdata(wrapped_c_state, -1)) {
				lua_pop(wrapped_c_state, 1);
				return NULL;
			} else {
				LuaState * cpp_state = static_cast<LuaState*>(lua_touserdata(wrapped_c_state, -1));
				lua_pop(wrapped_c_state, 1);
				return cpp_state;
			}
		}
		
		lua_State * GetCState() const {
			return c_state_;
		}
		
		int GetTop() const {
			return lua_gettop(c_state_);
		}
		
		void Pop(int n) {
			lua_pop(c_state_, n);
		}
		
		LuaObject Stack(int index);
		
		LuaObject GetGlobals();

	private:
		lua_State * c_state_;
	};

	class LuaObject {
	public:
		LuaObject() : lua_state_(NULL), ref_(LUA_NOREF)
		{
		}
		
		LuaObject(LuaState * state, int stack_position) : lua_state_(state), ref_(LUA_NOREF)
		{
			if (lua_state_->GetCState()) {
				// TODO: check for valid stack position (and throw LuaException on failure)
				lua_pushvalue(lua_state_->GetCState(), stack_position);
				ref_ = luaL_ref(lua_state_->GetCState(), LUA_REGISTRYINDEX);
			}
		}
		
		LuaObject(const LuaObject & src) : lua_state_(src.lua_state_), ref_(LUA_NOREF)
		{
			if (lua_state_->GetCState() && src.ref_ != LUA_NOREF) {
				src.Push();
				ref_ = luaL_ref(lua_state_->GetCState(), LUA_REGISTRYINDEX);
				assert(ref_ != src.ref_);
			}
		}
		
		~LuaObject() {
			if (lua_state_ && lua_state_->GetCState() && ref_ != LUA_NOREF) {
				luaL_unref(lua_state_->GetCState(), LUA_REGISTRYINDEX, ref_);
			}
		}
		
		void Reset() {
			if (ref_ != LUA_NOREF && lua_state_ != NULL) {
				luaL_unref(lua_state_->GetCState(), LUA_REGISTRYINDEX, ref_);
				ref_ = LUA_NOREF;
				lua_state_ = NULL;
			}
		}
		
		void Push() const {
			lua_rawgeti(lua_state_->GetCState(), LUA_REGISTRYINDEX, ref_);
		}
		
		void AssignBoolean(LuaState * state, bool value) {
			lua_pushboolean(state->GetCState(), value);
			int new_ref = luaL_ref(state->GetCState(), LUA_REGISTRYINDEX);
			AssignToStateAndRef(state, new_ref);
		}
	
		void AssignInteger(LuaState * state, lua_Integer value) {
			lua_pushinteger(state->GetCState(), value);
			int new_ref = luaL_ref(state->GetCState(), LUA_REGISTRYINDEX);
			AssignToStateAndRef(state, new_ref);
		}
		
		void AssignNumber(LuaState * state, lua_Number value) {
			lua_pushnumber(state->GetCState(), value);
			int new_ref = luaL_ref(state->GetCState(), LUA_REGISTRYINDEX);
			AssignToStateAndRef(state, new_ref);
		}
		
		void AssignString(LuaState * state, const char * value) {
			lua_pushstring(state->GetCState(), value);
			int new_ref = luaL_ref(state->GetCState(), LUA_REGISTRYINDEX);
			AssignToStateAndRef(state, new_ref);
		}
		
		void AssignNil(LuaState * state) {
			lua_pushnil(state->GetCState());
			int new_ref = luaL_ref(state->GetCState(), LUA_REGISTRYINDEX);
			AssignToStateAndRef(state, new_ref);
		}
		
		void AssignNewTable(LuaState * state, int narr = 0, int nrec = 0) {
			lua_createtable(state->GetCState(), narr, nrec);
			int new_ref = luaL_ref(state->GetCState(), LUA_REGISTRYINDEX);
			AssignToStateAndRef(state, new_ref);
		}
		
		void SetInteger(const char * key, lua_Integer value) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key != NULL);
			Push();
			lua_pushinteger(lua_state_->GetCState(), value);
			lua_setfield(lua_state_->GetCState(), -2, key);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		LuaObject GetByName(const char * key) {
			// TODO: check validity of object state, and that it is a table, and that key is non-NULL
			Push();
			lua_getfield(lua_state_->GetCState(), -1, key);
			LuaObject value(lua_state_, -1);
			lua_pop(lua_state_->GetCState(), 2);
			return value;
		}
		
		LuaObject operator[](const char * key) {
			return GetByName(key);
		}

		int Type() const {
			if ( ! lua_state_) {
				return LUA_TNONE;
			}
			// TODO: check validity of object state, and type of value
			Push();
			int type = lua_type(lua_state_->GetCState(), -1);
			lua_pop(lua_state_->GetCState(), 1);
			return type;
		}
		
		const char * TypeName() {
			// TODO: check validity of object state, and type of value
			int type = Type();
			if (lua_state_) {
				return lua_typename(lua_state_->GetCState(), type);
			} else {
				return lua_typename(NULL, type);
			}
		}
		
		bool IsBoolean() const {
			if ( ! lua_state_) {
				return false;
			}
			Push();
			const bool is_match = lua_isboolean(lua_state_->GetCState(), -1);
			lua_state_->Pop(1);
			return is_match;
		}
		
		bool IsInteger() const {
			return IsNumber();
		}
		
		bool IsString() const {
			if ( ! lua_state_) {
				return false;
			}
#if LuaPlusLite__IsString_and_IsNumber_only_match_explicitly == 1
			return Type() == LUA_TSTRING;
#else
			Push();
			const bool is_match = lua_isstring(lua_state_->GetCState(), -1);
			lua_state_->Pop(1);
			return is_match;
#endif
		}
		
		bool IsNumber() const {
			if ( ! lua_state_) {
				return false;
			}
#if LuaPlusLite__IsString_and_IsNumber_only_match_explicitly == 1
			return Type() == LUA_TNUMBER;
#else
			Push();
			const bool is_match = lua_isnumber(lua_state_->GetCState(), -1);
			lua_state_->Pop(1);
			return is_match;
#endif
		}
		
		bool IsTable() const {
			if ( ! lua_state_) {
				return false;
			}
			Push();
			const bool is_match = lua_istable(lua_state_->GetCState(), -1);
			lua_state_->Pop(1);
			return is_match;
		}
		
		bool IsNil() const {
			if ( ! lua_state_) {
				return false;
			}
			Push();
			const bool is_match = lua_isnil(lua_state_->GetCState(), -1);
			lua_state_->Pop(1);
			return is_match;
		}
		
		bool IsNone() const {
			// API DIFFERENCE: LuaPlus will return 'false' if IsNone() is called
			// on an uninitialized LuaObject.  LuaPlusLite will return 'true'
			// instead.
			if ( ! lua_state_) {
				return true;
			}
			Push();
			const bool is_match = lua_isnone(lua_state_->GetCState(), -1);
			lua_state_->Pop(1);
			return is_match;
		}
		
		bool IsNoneOrNil() const {
			if ( ! lua_state_) {
				return true;
			}
			Push();
			const bool is_match = lua_isnoneornil(lua_state_->GetCState(), -1);
			lua_state_->Pop(1);
			return is_match;
		}
		
		bool IsUserData() const {
			if ( ! lua_state_) {
				return false;
			}
			Push();
			const bool is_match = lua_isuserdata(lua_state_->GetCState(), -1);
			lua_state_->Pop(1);
			return is_match;
		}
		
		bool IsFunction() const {
			if ( ! lua_state_) {
				return false;
			}
			Push();
			const bool is_match = lua_isfunction(lua_state_->GetCState(), -1);
			lua_state_->Pop(1);
			return is_match;
		}
		
		bool IsCFunction() const {
			if ( ! lua_state_) {
				return false;
			}
			Push();
			const bool is_match = lua_iscfunction(lua_state_->GetCState(), -1);
			lua_state_->Pop(1);
			return is_match;
		}
		
		bool IsLightUserData() const {
			if ( ! lua_state_) {
				return false;
			}
			Push();
			const bool is_match = lua_islightuserdata(lua_state_->GetCState(), -1);
			lua_state_->Pop(1);
			return is_match;
		}
		
		bool IsThread() const {
			if ( ! lua_state_) {
				return false;
			}
			Push();
			const bool is_match = lua_isthread(lua_state_->GetCState(), -1);
			lua_state_->Pop(1);
			return is_match;
		}
		
		bool ToBoolean() {
			Push();
			int value = lua_toboolean(lua_state_->GetCState(), -1);
#if LuaPlusLite__ToXYZ_methods_convert_internal_value_types == 1
			if (Type() != lua_state_->Stack(-1).Type()) {
				int new_ref = luaL_ref(lua_state_->GetCState(), LUA_REGISTRYINDEX);
				AssignToStateAndRef(lua_state_, new_ref);
				return value;
			}
#endif
			lua_pop(lua_state_->GetCState(), 1);
			return value;
		}
		
		lua_Integer ToInteger(int * isnum = NULL) {
			// TODO: check validity of object state, and type of value
			Push();
			lua_Integer value = lua_tointegerx(lua_state_->GetCState(), -1, isnum);
#if LuaPlusLite__ToXYZ_methods_convert_internal_value_types == 1
			if (Type() != lua_state_->Stack(-1).Type()) {
				int new_ref = luaL_ref(lua_state_->GetCState(), LUA_REGISTRYINDEX);
				AssignToStateAndRef(lua_state_, new_ref);
				return value;
			}
#endif
			lua_pop(lua_state_->GetCState(), 1);
			return value;
		}
		
		lua_Number ToNumber(int * isnum = NULL) {
			// TODO: check validity of object state, and type of value
			Push();
			lua_Number value = lua_tonumberx(lua_state_->GetCState(), -1, isnum);
#if LuaPlusLite__ToXYZ_methods_convert_internal_value_types == 1
			if (Type() != lua_state_->Stack(-1).Type()) {
				int new_ref = luaL_ref(lua_state_->GetCState(), LUA_REGISTRYINDEX);
				AssignToStateAndRef(lua_state_, new_ref);
				return value;
			}
#endif
			lua_pop(lua_state_->GetCState(), 1);
			return value;
		}
		
		const char * ToString(size_t * len = NULL) {
			// TODO: check validity of object state, and type of value
			Push();
			const char * value = lua_tolstring(lua_state_->GetCState(), -1, len);
#if LuaPlusLite__ToXYZ_methods_convert_internal_value_types == 1
			if (Type() != lua_state_->Stack(-1).Type()) {
				int new_ref = luaL_ref(lua_state_->GetCState(), LUA_REGISTRYINDEX);
				AssignToStateAndRef(lua_state_, new_ref);
				return value;
			}
#endif
			lua_pop(lua_state_->GetCState(), 1);
			return value;
		}
		
	private:
		void AssignToStateAndRef(LuaState * state, int ref) {
			// TODO: check validity of state and consider throwing a LuaException if it is invalid
			// TODO: consider returning early here if 'lua_state_ == state'
			Reset();
			// TODO: see if lua_state_ is/should-be reference counted
			lua_state_ = state;
			ref_ = ref;
		}
	
		LuaState * lua_state_;
		int ref_;
	};
	
	LuaObject LuaState::Stack(int index) {
		// TODO: check for a valid stack index
		return LuaObject(this, index);
	}
	
	LuaObject LuaState::GetGlobals() {
		lua_rawgeti(c_state_, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
		LuaObject globals(this, -1);
		lua_pop(c_state_, 1);
		return globals;
	}
}
using namespace LuaPlusLite;


// #includes for testing purposes only!
extern "C" {
#include "lobject.h"
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
	cout << "... checking type via Is methods:\n";
	cout << "....... IsBoolean?: " << obj.IsBoolean() << endl;
	assert(obj.IsBoolean() == (actual == LUA_TBOOLEAN));
	cout << "....... IsCFunction?: " << obj.IsCFunction() << endl;
	//assert(obj.IsCFunction() == (actual == LUA_TFUNCTION));	// Not applied as the value might be a non-C function.
	cout << "....... IsFunction?: " << obj.IsFunction() << endl;
	assert(obj.IsFunction() == (actual == LUA_TFUNCTION));
	cout << "....... IsInteger?: " << obj.IsInteger() << endl;
#if LuaPlusLite__IsString_and_IsNumber_only_match_explicitly == 1
	assert(obj.IsInteger() == (actual == LUA_TNUMBER));
#else
	assert(obj.IsInteger() == (actual == LUA_TNUMBER || (actual == LUA_TSTRING && is_string_convertible_to_number(obj.ToString()))));
#endif
	cout << "....... IsLightUserData?: " << obj.IsLightUserData() << endl;
	assert(obj.IsLightUserData() == (actual == LUA_TLIGHTUSERDATA));
	cout << "....... IsNil?: " << obj.IsNil() << endl;
	assert(obj.IsNil() == (actual == LUA_TNIL));
	cout << "....... IsNone?: " << obj.IsNone() << endl;
	assert(obj.IsNone() == (actual == LUA_TNONE));
	cout << "....... IsNoneOrNil?: " << obj.IsNoneOrNil() << endl;
	assert(obj.IsNoneOrNil() == (actual == LUA_TNONE || actual == LUA_TNIL));
	cout << "....... IsNumber?: " << obj.IsNumber() << endl;
#if LuaPlusLite__IsString_and_IsNumber_only_match_explicitly == 1
	assert(obj.IsNumber() == (actual == LUA_TNUMBER));
#else
	assert(obj.IsNumber() == (actual == LUA_TNUMBER || (actual == LUA_TSTRING && is_string_convertible_to_number(obj.ToString()))));
#endif
	cout << "....... IsString?: " << obj.IsString() << endl;
#if LuaPlusLite__IsString_and_IsNumber_only_match_explicitly == 1
	assert(obj.IsString() == (actual == LUA_TSTRING));
#else
	assert(obj.IsString() == (actual == LUA_TSTRING || actual == LUA_TNUMBER));
#endif
	cout << "....... IsTable?: " << obj.IsTable() << endl;
	assert(obj.IsTable() == (actual == LUA_TTABLE));
	cout << "....... IsThread?: " << obj.IsThread() << endl;
	assert(obj.IsThread() == (actual == LUA_TTHREAD));
	cout << "....... IsUserData?: " << obj.IsUserData() << endl;
	assert(obj.IsUserData() == (actual == LUA_TUSERDATA || actual == LUA_TLIGHTUSERDATA));
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
	assert(myLuaState_CState != NULL);
	
	cout << "Retrieving the C++ LuaState via the wrapped, C-based, lua_State\n";
	LuaState * luaStateFromInnerState = LuaState::CastState(myLuaState_CState);
	cout << "... LuaState from inner, C-based, lua_State: " << luaStateFromInnerState << endl;
	assert(luaStateFromInnerState == &myLuaState);
	assert(lua_gettop(myLuaState_CState) == 0);
	
	cout << "Assigning and retrieving a random integer: ";
	lua_Integer random_number = rand();
	cout << random_number << endl;
	LuaObject myEncodedInteger;
	myEncodedInteger.AssignInteger(&myLuaState, random_number);
	cout << "... encoded type number is " << myEncodedInteger.Type() << endl;
	assert(myEncodedInteger.Type() == LUA_TNUMBER);
	cout << "... encoded type name " << myEncodedInteger.TypeName() << endl;
	assert(strcmp(myEncodedInteger.TypeName(), "number") == 0);
	lua_Integer decoded_number = myEncodedInteger.ToInteger();
	cout << "... decoded number is " << decoded_number << endl;
	assert(random_number == decoded_number);
	log_and_check_types_via_Is_methods(myEncodedInteger, LUA_TNUMBER);
	assert(lua_gettop(myLuaState_CState) == 0);
	
	{
		cout << "Assigning and retrieving a random number: ";
		lua_Number random_number = ((lua_Number)rand() / (lua_Number)rand());
		cout << random_number << endl;
		LuaObject myEncodedNumber;
		myEncodedNumber.AssignNumber(&myLuaState, random_number);
		cout << "... encoded type number is " << myEncodedNumber.Type() << endl;
		assert(myEncodedNumber.Type() == LUA_TNUMBER);
		cout << "... encoded type name " << myEncodedNumber.TypeName() << endl;
		assert(strcmp(myEncodedNumber.TypeName(), "number") == 0);
		lua_Number decoded_number = myEncodedNumber.ToNumber();
		cout << "... decoded number is " << decoded_number << endl;
		assert(random_number == decoded_number);
		log_and_check_types_via_Is_methods(myEncodedNumber, LUA_TNUMBER);
		assert(lua_gettop(myLuaState_CState) == 0);
	}
	
	cout << "Copying LuaObject containing the previously-generated, random integer: " << random_number << endl;
	LuaObject copyOfEncodedInteger(myEncodedInteger);
	cout << "... encoded type number is " << copyOfEncodedInteger.Type() << endl;
	assert(copyOfEncodedInteger.Type() == LUA_TNUMBER);
	cout << "... encoded type name " << copyOfEncodedInteger.TypeName() << endl;
	assert(strcmp(copyOfEncodedInteger.TypeName(), "number") == 0);
	lua_Integer copy_of_decoded_number = copyOfEncodedInteger.ToInteger();
	cout << "... decoded number is " << copy_of_decoded_number << endl;
	assert(random_number == copy_of_decoded_number);
	log_and_check_types_via_Is_methods(copyOfEncodedInteger, LUA_TNUMBER);
	assert(lua_gettop(myLuaState_CState) == 0);
	
	cout << "Assigning and retrieving the previously-generated random integer via use of the Lua stack.\n";
	lua_pushinteger(myLuaState.GetCState(), random_number);
	LuaObject stackMadeEncodedInteger(&myLuaState, -1);
	lua_pop(myLuaState.GetCState(), 1);
	cout << "... encoded type number is " << stackMadeEncodedInteger.Type() << endl;
	assert(stackMadeEncodedInteger.Type() == LUA_TNUMBER);
	cout << "... encoded type name " << stackMadeEncodedInteger.TypeName() << endl;
	assert(strcmp(stackMadeEncodedInteger.TypeName(), "number") == 0);
	lua_Integer stack_made_decoded_number = stackMadeEncodedInteger.ToInteger();
	cout << "... decoded number is " << stack_made_decoded_number << endl;
	assert(random_number == stack_made_decoded_number);
	log_and_check_types_via_Is_methods(stackMadeEncodedInteger, LUA_TNUMBER);
	assert(lua_gettop(myLuaState_CState) == 0);
	
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
	assert(myEncodedString.Type() == LUA_TSTRING);
	cout << "... encoded type name " << myEncodedString.TypeName() << endl;
	assert(strcmp(myEncodedString.TypeName(), "string") == 0);
	const char * decoded_string = myEncodedString.ToString();
	cout << "... decoded string is " << decoded_string << endl;
	assert(decoded_string != NULL);
	assert(strcmp(random_string, decoded_string) == 0);
	log_and_check_types_via_Is_methods(myEncodedString, LUA_TSTRING);
	assert(lua_gettop(myLuaState_CState) == 0);
	
	cout << "Assigning and retrieving a nil value\n";
	LuaObject myNilObject;
	myNilObject.AssignNil(&myLuaState);
	cout << "... encoded type number is " << myNilObject.Type() << endl;
	assert(myNilObject.Type() == LUA_TNIL);
	cout << "... encoded type name " << myNilObject.TypeName() << endl;
	assert(strcmp(myNilObject.TypeName(), "nil") == 0);
	log_and_check_types_via_Is_methods(myNilObject, LUA_TNIL);
	assert(lua_gettop(myLuaState_CState) == 0);
	
	{
		cout << "Inspecting an uninitialized LuaObject:\n";
		LuaObject uninitializedObject;
		cout << "... encoded type number is " << uninitializedObject.Type() << endl;
		assert(uninitializedObject.Type() == LUA_TNONE);
		cout << "... encoded type name: " << uninitializedObject.TypeName() << endl;
		assert(strcmp(uninitializedObject.TypeName(), "no value") == 0);
		log_and_check_types_via_Is_methods(uninitializedObject, LUA_TNONE);
	}
	
	{
		cout << "Assigning and retrieving a boolean true value:\n";
		LuaObject encoded_boolean;
		encoded_boolean.AssignBoolean(&myLuaState, true);
		cout << "... encoded type number is " << encoded_boolean.Type() << endl;
		assert(encoded_boolean.Type() == LUA_TBOOLEAN);
		cout << "... encoded type name: " << encoded_boolean.TypeName() << endl;
		assert(strcmp(encoded_boolean.TypeName(), "boolean") == 0);
		bool decoded_boolean = encoded_boolean.ToBoolean();
		cout << "... decoded value: " << decoded_boolean << endl;
		log_and_check_types_via_Is_methods(encoded_boolean, LUA_TBOOLEAN);
	}

	{
		cout << "Assigning and retrieving a boolean false value:\n";
		LuaObject encoded_boolean;
		encoded_boolean.AssignBoolean(&myLuaState, false);
		cout << "... encoded type number is " << encoded_boolean.Type() << endl;
		assert(encoded_boolean.Type() == LUA_TBOOLEAN);
		cout << "... encoded type name: " << encoded_boolean.TypeName() << endl;
		assert(strcmp(encoded_boolean.TypeName(), "boolean") == 0);
		bool decoded_boolean = encoded_boolean.ToBoolean();
		cout << "... decoded value: " << decoded_boolean << endl;
		log_and_check_types_via_Is_methods(encoded_boolean, LUA_TBOOLEAN);
	}
	
	cout << "Assigning a new table\n";
	LuaObject myTable;
	myTable.AssignNewTable(&myLuaState);
	cout << "... encoded type number is " << myTable.Type() << endl;
	assert(myTable.Type() == LUA_TTABLE);
	cout << "... encoded type name " << myTable.TypeName() << endl;
	assert(strcmp(myTable.TypeName(), "table") == 0);
	log_and_check_types_via_Is_methods(myTable, LUA_TTABLE);
	assert(lua_gettop(myLuaState_CState) == 0);
	
	{
		int value = rand();
		string key = get_random_string();
		cout << "Creating and retriving a table entry using the random string, \""
			<< key << "\", as a key and the random number, "
			<< value << ", as a value.\n";
		myTable.SetInteger(key.c_str(), value);
		LuaObject encoded_value_1 = myTable.GetByName(key.c_str());
		cout << "... encoded value type via GetByName: " << encoded_value_1.Type() << endl;
		assert(encoded_value_1.Type() == LUA_TNUMBER);
		cout << "... encoded value type name via GetByName: " << encoded_value_1.TypeName() << endl;
		assert(strcmp(encoded_value_1.TypeName(), "number") == 0);
		cout << "... decoded value via GetByName: " << encoded_value_1.ToInteger() << endl;
		assert(encoded_value_1.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_1, LUA_TNUMBER);
		LuaObject encoded_value_2 = myTable[key.c_str()];
		cout << "... encoded value type via operator[]: " << encoded_value_2.Type() << endl;
		assert(encoded_value_2.Type() == LUA_TNUMBER);
		cout << "... encoded value type name via operator[]: " << encoded_value_2.TypeName() << endl;
		assert(strcmp(encoded_value_2.TypeName(), "number") == 0);
		cout << "... decoded value via operator[]: " << encoded_value_2.ToInteger() << endl;
		assert(encoded_value_2.ToInteger() == value);
		log_and_check_types_via_Is_methods(encoded_value_2, LUA_TNUMBER);
	}

	{
		int random_number_2 = rand();
		cout << "Retrieving a new random number from the stack, " << random_number_2 << ", as accessed by LuaState::Stack().\n";
		int original_stack_top = myLuaState.GetTop();
		cout << "... original stack top: " << original_stack_top << endl;
		lua_pushinteger(myLuaState_CState, random_number_2);
		int new_stack_top = myLuaState.GetTop();
		cout << "... new stack top: " << new_stack_top << endl;
		assert(new_stack_top = original_stack_top + 1);
		int type_from_Stack_position_1 = myLuaState.Stack(1).Type();
		cout << "... type from Stack(1): " << type_from_Stack_position_1 << endl;
		const char * type_name_from_stack_position_1 = myLuaState.Stack(1).TypeName();
		cout << "... type name from Stack(1): " << type_name_from_stack_position_1 << endl;
		assert(strcmp(type_name_from_stack_position_1, "number") == 0);
		int decoded_integer_from_Stack_method = myLuaState.Stack(1).ToInteger();
		cout << "... decoded integer from Stack(1): " << decoded_integer_from_Stack_method << endl;
		assert(decoded_integer_from_Stack_method == random_number_2);
//		log_and_check_types_via_Is_methods<LuaObject>(myLuaState.Stack(1), LUA_TNUMBER);
		myLuaState.Pop(1);
	}
	
	{
		cout << "Retrieving global table\n";
		LuaObject allGlobals = myLuaState.GetGlobals();
		cout << "... type: " << allGlobals.Type() << endl;
		assert(allGlobals.Type() == LUA_TTABLE);
		cout << "... type name: " << allGlobals.TypeName() << endl;
		assert(strcmp(allGlobals.TypeName(), "table") == 0);
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
		assert(decoded_value_1 == value);
		LuaObject encoded_value = myLuaState.GetGlobals().GetByName(key.c_str());
		int decoded_value_2 = encoded_value.ToInteger();
		cout << "... decoded value 2 (value from named LuaObject): " << decoded_value_2 << endl;
		assert(decoded_value_2 == value);
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
		assert(helloObj.Type() == LUA_TSTRING);
		log_and_check_types_via_Is_methods(helloObj, LUA_TSTRING);
#else
		assert(helloObj.Type() == LUA_TNUMBER);
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
		assert(wasExceptionCaught == true);
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
		assert(wasExceptionCaught == true);
		cout << "... exception message: \"" << exceptionMessage << "\"\n";
	}
	
    return 0;
}

