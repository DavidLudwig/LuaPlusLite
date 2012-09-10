//
//  LuaPlusLite.h
//  LuaPlusLite
//
//  Created by David Ludwig on 9/9/12.
//  Copyright (c) 2012 David Ludwig. All rights reserved.
//

#ifndef LuaPlusLite_LuaPlusLite_h
#define LuaPlusLite_LuaPlusLite_h

// C++ Standard Library Includes:
#include <string>

// Lua Includes:
#include <lua.hpp>

// LuaPlusLite Compile-Time Options:
#define LuaPlusLite__ToXYZ_methods_convert_internal_value_types 0
#define LuaPlusLite__IsString_and_IsNumber_only_match_explicitly 0

namespace LuaPlusLite {
	class LuaException : public std::exception {
	public:
		LuaException() {
		}
		
		LuaException(const std::string & message) : message_(message) {
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
	#define luapluslite_assert(expression) if (!(expression)) { throw LuaException(std::string("assertion failed in ") + __PRETTY_FUNCTION__ + ": (" + #expression + ")"); }
	#define luapluslite_assert_ex(expression, message) if (!(expression)) { throw LuaException(std::string("assertion failed in ") + __PRETTY_FUNCTION__ + ": (" + #expression + "); " + message); }
#elif defined(_MSC_VER)
	#define luapluslite_assert(expression) if (!(expression)) { throw LuaException(std::string("assertion failed in ") + __FUNCSIG__ + ": (" + #expression + ")"); }
	#define luapluslite_assert_ex(expression, message) if (!(expression)) { throw LuaException(std::string("assertion failed in ") + __FUNCSIG__ + ": (" + #expression + "); " + message); }
#else
	#define luapluslite_assert(expression) if (!(expression)) { throw LuaException(std::string("assertion failed in ") + __FUNCTION__ + ": (" + #expression + ")"); }
	#define luapluslite_assert_ex(expression, message) if (!(expression)) { throw LuaException(std::string("assertion failed in ") + __FUNCTION__ + ": (" + #expression + "); " + message); }
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
			// TODO: allow Set* operation on userdata objects with appropriate metatables
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key != NULL);
			Push();
			lua_pushinteger(lua_state_->GetCState(), value);
			lua_setfield(lua_state_->GetCState(), -2, key);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetInteger(int key, lua_Integer value) {
			// TODO: allow Set* operation on userdata objects with appropriate metatables
			luapluslite_assert(IsTable() == true);
			Push();
			lua_pushinteger(lua_state_->GetCState(), key);
			lua_pushinteger(lua_state_->GetCState(), value);
			lua_settable(lua_state_->GetCState(), -3);
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
		
		LuaObject GetByIndex(int key) {
			// TODO: check validity of object state, and that it is a table, and that key is non-NULL
			Push();
			lua_pushinteger(lua_state_->GetCState(), key);
			lua_gettable(lua_state_->GetCState(), -2);
			LuaObject value(lua_state_, -1);
			lua_pop(lua_state_->GetCState(), 2);
			return value;
		}
		
		LuaObject operator[](const char * key) {
			return GetByName(key);
		}
		
		LuaObject operator[](int key) {
			return GetByIndex(key);
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


#endif
