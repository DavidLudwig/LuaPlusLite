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

namespace LuaPlusLite {
	class LuaState {
	public:
		LuaState() : c_state_(NULL) {
			c_state_ = luaL_newstate();
		}
		
		~LuaState() {
			if (c_state_) {
				lua_close(c_state_);
				c_state_ = NULL;
			}
		}
		
		lua_State * GetCState() const {
			return c_state_;
		}

	private:
		lua_State * c_state_;
	};

	class LuaObject {
	public:
		LuaObject() : lua_state_(NULL), ref_(LUA_NOREF) {
		}
		
		void Reset() {
			if (ref_ != LUA_NOREF && lua_state_ != NULL) {
				luaL_unref(lua_state_->GetCState(), LUA_REGISTRYINDEX, ref_);
				ref_ = LUA_NOREF;
				lua_state_ = NULL;
			}
		}
	
		void AssignInteger(LuaState * state, lua_Integer value) {
			// TODO: check validity of state and throw a LuaException if it is invalid
			Reset();
			
			// TODO: see if lua_state_ is/should-be reference counted
			lua_state_ = state;
			lua_pushinteger(state->GetCState(), value);
			ref_ = luaL_ref(state->GetCState(), LUA_REGISTRYINDEX);
		}
		
		void AssignString(LuaState * state, const char * value) {
			// TODO: check validity of state and throw a LuaException if it is invalid
			Reset();
			
			// TODO: see if lua_state_ is/should-be reference counted
			lua_state_ = state;
			lua_pushstring(state->GetCState(), value);
			ref_ = luaL_ref(state->GetCState(), LUA_REGISTRYINDEX);
		}
		
		lua_Integer ToInteger(int * isnum = NULL) {
			// TODO: check validity of object state, and type of value
			lua_rawgeti(lua_state_->GetCState(), LUA_REGISTRYINDEX, ref_);
			lua_Integer value = lua_tointegerx(lua_state_->GetCState(), -1, isnum);
			lua_pop(lua_state_->GetCState(), 1);
			return value;
		}
		
		const char * ToString(size_t * len = NULL) {
			// TODO: check validity of object state, and type of value
			lua_rawgeti(lua_state_->GetCState(), LUA_REGISTRYINDEX, ref_);
			const char * value = lua_tolstring(lua_state_->GetCState(), -1, len);
			lua_pop(lua_state_->GetCState(), 1);
			return value;
		}
		
	private:
		LuaState * lua_state_;
		int ref_;
	};

	class LuaStackObject {
	public:
	};
}
using namespace LuaPlusLite;

int main(int argc, const char * argv[])
{
	cout << "Welcome to LuaPlusLite!\n";
	srand(0);
	
	cout << "Making a LuaState\n";
	LuaState myLuaState;
	
	lua_State * myLuaState_CState = myLuaState.GetCState();
	cout << "... Done!  Inner lua_State is " << myLuaState_CState << endl;
	assert(myLuaState_CState != NULL);
	
	cout << "Assigning and retrieving a random integer: ";
	lua_Integer random_number = rand();
	cout << random_number << endl;
	LuaObject myEncodedInteger;
	myEncodedInteger.AssignInteger(&myLuaState, random_number);
	lua_Integer decoded_number = myEncodedInteger.ToInteger();
	cout << "... decoded number is " << decoded_number << endl;
	assert(random_number == decoded_number);
	
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
	const char * decoded_string = myEncodedString.ToString();
	cout << "... decoded string is " << decoded_string << endl;
	assert(decoded_string != NULL);
	assert(strcmp(random_string, decoded_string) == 0);
	
    return 0;
}

