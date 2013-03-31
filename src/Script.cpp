#include "Script.h"
#include "LuaBindings.h"

#include "cinder/Utilities.h"

extern "C" {
#include <lua/lua.h>
#include <lua/lualib.h>
}

#include <luabind/class_info.hpp>


using namespace ci;
using namespace ci::app;
using namespace std;
using namespace luabind;

namespace lua {
    
	void State::create( bool bindAll ){
		mInstance = new State();
		mInstance->mBindAll = bindAll;
		mInstance->mState = luaL_newstate();
		if( mInstance->mState == NULL ){
			app::console() << "Error(Lua::Create) : Problem creating Global State" << endl;
			return;
		}
        
		luaL_openlibs( mInstance->mState );
        
		lua_atpanic( mInstance->mState, &State::panic );
        
		luabind::open( mInstance->mState );
        
		if( mInstance->mBindAll ){
			Bindings::bindStd( mInstance->mState );
            Bindings::bindCinder( mInstance->mState );
			luabind::bind_class_info( State::get() );
		}
        
	}
    
	lua_State* State::get(){
		if( mInstance == NULL ) create();
		return mInstance->mState;
	}
	void State::clear(){
		delete mInstance;
		mInstance = NULL;
	}
    
	State::~State(){
		lua_close( mState );
	}
    
	State::State(){
	};
    
	int State::panic( lua_State *L ) {
		app::console() << "LuaPanic: " <<  lua_tostring( get(), -1 ) << endl;
		mInstance = NULL;
		return 0;
	}
    
    
	State* State::mInstance = NULL;
    
    int Script::panic( lua_State *L )
    {
        cout << "Lua panic ocurred! : " << lua_tostring(L, -1) << endl;
        cout << "Closing state" << endl;
        return 0;
    }
    
	Script::Script( bool bindAll, bool useLuaThread ){
		mStopOnErrors	= false;
		mErrors			= false;
        
        if( useLuaThread ) {
            mState = lua_newthread( State::get() );
            lua_newtable( mState );
            lua_newtable( mState );
            lua_pushliteral( mState, "__index" );
            lua_getglobal( mState,"_G" );
            lua_settable( mState, -3 );
            lua_setmetatable( mState, -2 );
            lua_setglobal( mState, "_G" );
        }
        else {
            mState = luaL_newstate();
            
            luaL_openlibs( mState );            
            luabind::open( mState );
            
            if( bindAll ){
                Bindings::bindCinder( mState );
                Bindings::bindStd( mState );
            }
        }
        
        lua_atpanic( mState, &panic);
	}
	Script::~Script(){
	}
    
    std::string Script::getErrorMessage()
    {
        luabind::object msg( luabind::from_stack( mState, -1 ) );
        std::ostringstream errorMessage;
        errorMessage << "Lua run-time error: " << msg;
        return errorMessage.str();
    }
		
	std::string Script::getLastErrors(){
		return mLastErrorString;
	}

	void Script::loadFile( DataSourceRef source ){
        loadString( ci::loadString( source ) );
    }
	
	void Script::loadString( const std::string& script ){
		addClassSupport();
        
        mLastErrorString = "";
        
        // load the string
        if( luaL_loadstring( mState, script.c_str() ) != 0 ) {
            mErrors = true;
            mLastErrorString = getErrorMessage();
            cout << mLastErrorString << endl;
            return;
        }
        else mErrors = false;
        
        
        // run the string
        if( lua_pcall( mState, 0, LUA_MULTRET, 0 ) != 0 ) {
            mErrors = true;
            mLastErrorString = getErrorMessage();
            cout << mLastErrorString << endl;
            return;
        }
        else mErrors = false;
        
        
		setup();
	}
    
	void Script::addClassSupport(){
		luaL_dostring( mState,
                      "function class()"
                      "    local cls = {}"
                      "    cls.__index = cls"
                      "    return setmetatable(cls, {__call = function (c, ...)"
                      "        instance = setmetatable({}, cls)"
                      "        if cls.__init then"
                      "            cls.__init(instance, ...)"
                      "        end"
                      "        return instance"
                      "    end})"
                      "end"
                      );
	}


	bool Script::functionExists( const std::string& function ) {

		luabind::object g = luabind::globals( mState );
		luabind::object func = g[ function ];
        
		if( func ) {
			if( luabind::type( func ) == LUA_TFUNCTION )
				return true;
		}
		return false;
	}

	void Script::setup(){
		call( "setup" );
	}
	void Script::update(){
		call( "update" );
	}
	void Script::draw(){
		call( "draw" );
	}
	void Script::mouseDown( MouseEvent event ){
		call( "mouseDown", event );
	}
	void Script::mouseUp( MouseEvent event ){
		call( "mouseUp", event );
	}
	void Script::mouseMove( MouseEvent event ){
		call( "mouseMove", event );
	}
	void Script::mouseDrag( MouseEvent event ){
		call( "mouseDrag", event );
	}
	void Script::mouseWheel( MouseEvent event ){
		call( "mouseWheel", event );
	}
	void Script::keyDown( KeyEvent event ) {
		call( "keyDown", event );
	}
	void Script::keyUp( KeyEvent event ) {
		call( "keyUp", event );
	}
};