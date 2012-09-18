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
#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Error Reporting
#endif
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


#if defined(__clang__) || defined(__GNUC__)
#pragma mark - LuaState
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
		
		LuaObject GetGlobal(const char * key);
		LuaObject GetGlobals();


#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Code Loading and Execution
#endif

		int DoFile(const char * filename) {
			return luaL_dofile(c_state_, filename);
		}
		
		int DoString(const char * str) {
			return luaL_dostring(c_state_, str);
		}
		
		int Load(lua_Reader reader, void * data, const char * chunk_name, const char * mode) {
			return lua_load(c_state_, reader, data, chunk_name, mode);
		}
		
		int LoadBuffer(const char * buff, size_t sz, const char * name) {
			return luaL_loadbuffer(c_state_, buff, sz, name);
		}
		
		int LoadBufferX(const char * buff, size_t sz, const char * name, const char * mode) {
			return luaL_loadbufferx(c_state_, buff, sz, name, mode);
		}

		int LoadFile(const char * filename) {
			return luaL_loadfile(c_state_, filename);
		}
		
		int LoadFileX(const char * filename, const char * mode) {
			return luaL_loadfilex(c_state_, filename, mode);
		}
		
		int LoadString(const char * str) {
			return luaL_loadstring(c_state_, str);
		}


#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Error Management
#endif

		lua_CFunction AtPanic(lua_CFunction panicf) {
			return lua_atpanic(c_state_, panicf);
		}


#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Stack Manipulation + Value Retrieval
#endif

		int GetTop() const {
			return lua_gettop(c_state_);
		}
		
		void Pop(int n) {
			lua_pop(c_state_, n);
		}
		
		void PushBoolean(int b) {
			lua_pushboolean(c_state_, b);
		}
		
		void PushCClosure(lua_CFunction fn, int n) {
			lua_pushcclosure(c_state_, fn, n);
		}
		
		void PushCFunction(lua_CFunction fn) {
			lua_pushcfunction(c_state_, fn);
		}
		
		void PushInteger(lua_Integer n) {
			lua_pushinteger(c_state_, n);
		}
		
		void PushLightUserData(void * p) {
			lua_pushlightuserdata(c_state_, p);
		}
		
		void PushNil() {
			lua_pushnil(c_state_);
		}
		
		void PushNumber(lua_Number n) {
			lua_pushnumber(c_state_, n);
		}
		
		const char * PushString(const char * s) {
			return lua_pushstring(c_state_, s);
		}
		
		int PushThread() {
			return lua_pushthread(c_state_);
		}
		
		void SetTop(int n) {
			lua_settop(c_state_, n);
		}
		
		LuaObject Stack(int index);


#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Stack Value Type Checking + Value Retrieval
#endif

		void CheckAny(int stack_index) {
			luaL_checkany(c_state_, stack_index);
		}

		int CheckInt(int stack_index) {
			return luaL_checkint(c_state_, stack_index);
		}

		lua_Integer CheckInteger(int stack_index) {
			return luaL_checkinteger(c_state_, stack_index);
		}
		
		long CheckLong(int stack_index) {
			return luaL_checklong(c_state_, stack_index);
		}

		lua_Number CheckNumber(int stack_index) {
			return luaL_checknumber(c_state_, stack_index);
		}
		
		const char * CheckStack(int stack_index) {
			return luaL_checkstring(c_state_, stack_index);
		}
		
		lua_Unsigned CheckUnsigned(int stack_index) {
			return luaL_checkunsigned(c_state_, stack_index);
		}
		
		void CheckType(int stack_index, int type) {
			luaL_checktype(c_state_, stack_index, type);
		}
		

#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Lua Function Calling
#endif

		int PCall(int nargs, int nresults, int msgh) {
			return lua_pcall(c_state_, nargs, nresults, msgh);
		}

		
	private:
		lua_State * c_state_;
	};
	

#if defined(__clang__) || defined(__GNUC__)
#pragma mark - LuaObject
#endif

	class LuaObject {
	public:
	
#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Initialization and Destruction
#endif

		LuaObject() : lua_state_(NULL)
		{
		}
		
		LuaObject(LuaState * state, int stack_position) : lua_state_(state)
		{
			if (lua_state_->GetCState()) {
				// TODO: check for valid stack position (and throw LuaException on failure)
				lua_pushvalue(lua_state_->GetCState(), stack_position);
				lua_rawsetp(lua_state_->GetCState(), LUA_REGISTRYINDEX, this);
			}
		}
		
		LuaObject(const LuaObject & src) : lua_state_(src.lua_state_)
		{
			if (lua_state_->GetCState()) {
				src.Push();
				lua_rawsetp(lua_state_->GetCState(), LUA_REGISTRYINDEX, this);
			}
		}
		
		~LuaObject() {
			if (lua_state_ && lua_state_->GetCState()) {
				lua_pushnil(lua_state_->GetCState());
				lua_rawsetp(lua_state_->GetCState(), LUA_REGISTRYINDEX, this);
			}
		}
		
		void Reset() {
			if (lua_state_ != NULL) {
				lua_pushnil(lua_state_->GetCState());
				lua_rawsetp(lua_state_->GetCState(), LUA_REGISTRYINDEX, this);
				lua_state_ = NULL;
			}
		}
		
#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Stack Management
#endif
		
		void Push() const {
			luapluslite_assert(lua_state_ != NULL);
			lua_rawgetp(lua_state_->GetCState(), LUA_REGISTRYINDEX, this);
		}


#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Value Assignment
#endif
		
		void AssignBoolean(LuaState * state, bool value) {
			luapluslite_assert(state != NULL);
			lua_pushboolean(state->GetCState(), value);
			AssignValueToState(state);
		}
	
		void AssignInteger(LuaState * state, lua_Integer value) {
			luapluslite_assert(state != NULL);
			lua_pushinteger(state->GetCState(), value);
			AssignValueToState(state);
		}
		
		void AssignLightUserData(LuaState * state, void * value) {
			luapluslite_assert(state != NULL);
			lua_pushlightuserdata(state->GetCState(), value);
			AssignValueToState(state);
		}
		
		void AssignNumber(LuaState * state, lua_Number value) {
			luapluslite_assert(state != NULL);
			lua_pushnumber(state->GetCState(), value);
			AssignValueToState(state);
		}
		
		void AssignString(LuaState * state, const char * value) {
			luapluslite_assert(state != NULL);
			lua_pushstring(state->GetCState(), value);
			AssignValueToState(state);
		}
		
		void AssignNil(LuaState * state) {
			luapluslite_assert(state != NULL);
			lua_pushnil(state->GetCState());
			AssignValueToState(state);
		}
		
		void AssignNewTable(LuaState * state, int narr = 0, int nrec = 0) {
			luapluslite_assert(state != NULL);
			lua_createtable(state->GetCState(), narr, nrec);
			AssignValueToState(state);
		}


#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Value Retrieval
#endif

		bool GetBoolean() {
			return ToBoolean();
		}
		
		double GetDouble() {
			return GetDouble();
		}
		
		float GetFloat() {
			return GetNumber();
		}
		
		lua_Integer GetInteger() {
			return ToInteger();
		}
		
		void * GetLightUserData() {
			return ToUserData();
		}
		
		lua_Number GetNumber() {
			return ToNumber();
		}
		
		const char * GetString() {
			return ToString();
		}
		
		void * GetUserData() {
			return GetUserData();
		}

		bool ToBoolean() {
			if ( ! lua_state_) {
				return false;
			}
			Push();
			int value = lua_toboolean(lua_state_->GetCState(), -1);
#if LuaPlusLite__ToXYZ_methods_convert_internal_value_types == 1
			if (Type() != lua_state_->Stack(-1).Type()) {
				lua_rawsetp(lua_state_->GetCState(), LUA_REGISTRYINDEX, this);
				return value;
			}
#endif
			lua_pop(lua_state_->GetCState(), 1);
			return value;
		}
		
		lua_Integer ToInteger(int * isnum = NULL) {
			luapluslite_assert(IsInteger() == true);
			Push();
			lua_Integer value = lua_tointegerx(lua_state_->GetCState(), -1, isnum);
#if LuaPlusLite__ToXYZ_methods_convert_internal_value_types == 1
			if (Type() != lua_state_->Stack(-1).Type()) {
				lua_rawsetp(lua_state_->GetCState(), LUA_REGISTRYINDEX, this);
				return value;
			}
#endif
			lua_pop(lua_state_->GetCState(), 1);
			return value;
		}
		
		lua_Number ToNumber(int * isnum = NULL) {
			luapluslite_assert(IsNumber() == true);
			Push();
			lua_Number value = lua_tonumberx(lua_state_->GetCState(), -1, isnum);
#if LuaPlusLite__ToXYZ_methods_convert_internal_value_types == 1
			if (Type() != lua_state_->Stack(-1).Type()) {
				lua_rawsetp(lua_state_->GetCState(), LUA_REGISTRYINDEX, this);
				return value;
			}
#endif
			lua_pop(lua_state_->GetCState(), 1);
			return value;
		}
		
		const char * ToString(size_t * len = NULL) {
			luapluslite_assert(IsString() == true);
			Push();
			const char * value = lua_tolstring(lua_state_->GetCState(), -1, len);
#if LuaPlusLite__ToXYZ_methods_convert_internal_value_types == 1
			if (Type() != lua_state_->Stack(-1).Type()) {
				lua_rawsetp(lua_state_->GetCState(), LUA_REGISTRYINDEX, this);
				return value;
			}
#endif
			lua_pop(lua_state_->GetCState(), 1);
			return value;
		}
		
		void * ToUserData() {
			luapluslite_assert(IsUserData() == true);
			Push();
			switch (Type()) {
				case LUA_TUSERDATA:
				{
					// TODO: prevent certain, maybe all, userdata objects from being dereferenced in ToUserData
					void ** inner_value = (void **)lua_touserdata(lua_state_->GetCState(), -1);
					lua_pop(lua_state_->GetCState(), 1);
					return *inner_value;
				}
				
				case LUA_TLIGHTUSERDATA:
				{
					void * value = lua_touserdata(lua_state_->GetCState(), -1);
					lua_pop(lua_state_->GetCState(), 1);
					return value;
				}
				
				default:
					luapluslite_assert(Type() == LUA_TUSERDATA || Type() == LUA_TLIGHTUSERDATA);
					return NULL;
			}
		}

#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Type Checking
#endif

		int Type() const {
			if ( ! lua_state_) {
				return LUA_TNONE;
			}
			Push();
			int type = lua_type(lua_state_->GetCState(), -1);
			lua_pop(lua_state_->GetCState(), 1);
			return type;
		}
		
		const char * TypeName() {
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
		
		bool IsConvertibleToString() const {
			if ( ! lua_state_) {
				return false;
			}
			Push();
			const bool is_match = lua_isstring(lua_state_->GetCState(), -1);
			lua_state_->Pop(1);
			return is_match;
		}
		
		bool IsInteger() const {
			return IsNumber();
		}
		
		bool IsString() const {
#if LuaPlusLite__IsString_and_IsNumber_only_match_explicitly == 1
			return Type() == LUA_TSTRING;
#else
			return IsConvertibleToString();
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
		
		
#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Table Value Assignment
#endif

		// TODO: allow Set operation on userdata objects with appropriate metatables
		// TODO: make Set operations with LuaObject-based keys check for compatible LuaStates
		
		LuaObject CreateTable(const char * key, int narr = 0, int nrec = 0) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key != NULL);
			Push();
			lua_createtable(lua_state_->GetCState(), narr, nrec);
			LuaObject value(lua_state_, -1);
			lua_setfield(lua_state_->GetCState(), -2, key);
			lua_pop(lua_state_->GetCState(), 1);
			return value;
		}

		LuaObject CreateTable(int key, int narr = 0, int nrec = 0) {
			luapluslite_assert(IsTable() == true);
			Push();
			lua_pushinteger(lua_state_->GetCState(), key);
			lua_createtable(lua_state_->GetCState(), narr, nrec);
			LuaObject value(lua_state_, -1);
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
			return value;
		}
		
		LuaObject CreateTable(LuaObject key, int narr = 0, int nrec = 0)
		{
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key.IsNone() == false);
			Push();
			key.Push();
			lua_createtable(lua_state_->GetCState(), narr, nrec);
			LuaObject value(lua_state_, -1);
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
			return value;
		}
		
		// TODO: Implement Insert(LuaObject)

		void SetBoolean(const char * key, bool value) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key != NULL);
			Push();
			lua_pushboolean(lua_state_->GetCState(), value);
			lua_setfield(lua_state_->GetCState(), -2, key);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetBoolean(int key, bool value) {
			luapluslite_assert(IsTable() == true);
			Push();
			lua_pushinteger(lua_state_->GetCState(), key);
			lua_pushboolean(lua_state_->GetCState(), value);
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetBoolean(LuaObject key, bool value) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key.IsNone() == false);
			Push();
			key.Push();
			lua_pushboolean(lua_state_->GetCState(), value);
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetInteger(const char * key, lua_Integer value) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key != NULL);
			Push();
			lua_pushinteger(lua_state_->GetCState(), value);
			lua_setfield(lua_state_->GetCState(), -2, key);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetInteger(int key, lua_Integer value) {
			luapluslite_assert(IsTable() == true);
			Push();
			lua_pushinteger(lua_state_->GetCState(), key);
			lua_pushinteger(lua_state_->GetCState(), value);
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetInteger(LuaObject key, lua_Integer value) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key.IsNone() == false);
			Push();
			key.Push();
			lua_pushinteger(lua_state_->GetCState(), value);
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetLightUserData(const char * key, void * value) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key != NULL);
			Push();
			lua_pushlightuserdata(lua_state_->GetCState(), value);
			lua_setfield(lua_state_->GetCState(), -2, key);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetLightUserData(int key, void * value) {
			luapluslite_assert(IsTable() == true);
			Push();
			lua_pushinteger(lua_state_->GetCState(), key);
			lua_pushlightuserdata(lua_state_->GetCState(), value);
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetLightUserData(LuaObject key, void * value) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key.IsNone() == false);
			Push();
			key.Push();
			lua_pushlightuserdata(lua_state_->GetCState(), value);
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetNil(const char * key) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key != NULL);
			Push();
			lua_pushnil(lua_state_->GetCState());
			lua_setfield(lua_state_->GetCState(), -2, key);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetNil(int key) {
			luapluslite_assert(IsTable() == true);
			Push();
			lua_pushinteger(lua_state_->GetCState(), key);
			lua_pushnil(lua_state_->GetCState());
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetNil(LuaObject key) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key.IsNone() == false);
			Push();
			key.Push();
			lua_pushnil(lua_state_->GetCState());
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetNumber(const char * key, lua_Number value) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key != NULL);
			Push();
			lua_pushnumber(lua_state_->GetCState(), value);
			lua_setfield(lua_state_->GetCState(), -2, key);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetNumber(int key, lua_Number value) {
			luapluslite_assert(IsTable() == true);
			Push();
			lua_pushinteger(lua_state_->GetCState(), key);
			lua_pushnumber(lua_state_->GetCState(), value);
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetNumber(LuaObject key, lua_Number value) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key.IsNone() == false);
			Push();
			key.Push();
			lua_pushnumber(lua_state_->GetCState(), value);
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetString(const char * key, const char * value) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key != NULL);
			luapluslite_assert(value != NULL);
			Push();
			lua_pushstring(lua_state_->GetCState(), value);
			lua_setfield(lua_state_->GetCState(), -2, key);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetString(int key, const char * value) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(value != NULL);
			Push();
			lua_pushinteger(lua_state_->GetCState(), key);
			lua_pushstring(lua_state_->GetCState(), value);
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetString(LuaObject key, const char * value) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key.IsNone() == false);
			luapluslite_assert(value != NULL);
			Push();
			key.Push();
			lua_pushstring(lua_state_->GetCState(), value);
			lua_settable(lua_state_->GetCState(), -3);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
		void SetUserData(const char * key, void * value) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key != NULL);
			Push();
			lua_newuserdata(lua_state_->GetCState(), sizeof(void *));
			void ** inner_value = (void **)lua_touserdata(lua_state_->GetCState(), -1);
			*inner_value = value;
			lua_setfield(lua_state_->GetCState(), -2, key);
			lua_pop(lua_state_->GetCState(), 1);
		}
		
#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Table Value Retrieval
#endif
		
		LuaObject GetByName(const char * key) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key != NULL);
			Push();
			lua_getfield(lua_state_->GetCState(), -1, key);
			LuaObject value(lua_state_, -1);
			lua_pop(lua_state_->GetCState(), 2);
			return value;
		}
		
		LuaObject GetByIndex(int key) {
			luapluslite_assert(IsTable() == true);
			Push();
			lua_pushinteger(lua_state_->GetCState(), key);
			lua_gettable(lua_state_->GetCState(), -2);
			LuaObject value(lua_state_, -1);
			lua_pop(lua_state_->GetCState(), 2);
			return value;
		}
		
		LuaObject GetByObject(LuaObject key) {
			luapluslite_assert(IsTable() == true);
			luapluslite_assert(key.IsNone() == false);
			// TODO: check that key is in the same state as 'this'
			Push();
			key.Push();
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
		
		LuaObject operator[](LuaObject key) {
			return GetByObject(key);
		}
		
#if defined(__clang__) || defined(__GNUC__)
#pragma mark - Private Stuff
#endif
	private:
		void AssignValueToState(LuaState * state) {
			if (lua_state_ != NULL) {
				Reset();
			}
			lua_state_ = state;
			lua_rawsetp(state->GetCState(), LUA_REGISTRYINDEX, this);
		}
	
		LuaState * lua_state_;
	};


#if defined(__clang__) || defined(__GNUC__)
#pragma mark - LuaState Miscellany
#endif
	
	LuaObject LuaState::Stack(int index) {
		// TODO: check for a valid stack index
		return LuaObject(this, index);
	}
	
	LuaObject LuaState::GetGlobal(const char * key) {
		return GetGlobals()[key];
	}
	
	LuaObject LuaState::GetGlobals() {
		lua_rawgeti(c_state_, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
		LuaObject globals(this, -1);
		lua_pop(c_state_, 1);
		return globals;
	}
}


#endif
