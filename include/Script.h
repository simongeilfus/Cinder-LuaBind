#pragma once

#include <luabind/luabind.hpp>

class lua_State;

namespace lua {
	
	class State {
	public:
		static void create( bool bindAll = true );
		static lua_State* get();
		static void clear();
		~State();
        
	private:
		State();
		static int panic( lua_State *L );
        
		lua_State* mState;
		static State* mInstance;
		bool mBindAll;
	};
    
	class Script {
	public:
		Script( bool bindAll = true, bool useLuaThread = true );
		~Script();
        
		void loadString( const std::string& script );
		void loadFile( ci::DataSourceRef source );
        
		std::string getLastErrors();
        
		bool functionExists( const std::string& function );
        
        template<typename T, typename... Args>
        T call( const std::string& function, Args... args )
        {
            T result;
            
            if( !functionExists( function ) || ( mStopOnErrors && mErrors ) )
                return result;
            
            try{
                result = luabind::call_function<T>( mState, function.c_str(), args... );
            }
            catch( luabind::error error ){
                mErrors = true;
                mLastErrorString = getErrorMessage();
                std::cout << "Lua Error trying to call " << function << " : " << std::endl << mLastErrorString << std::endl;
            }
            return result;
        }
        
        template<typename... Args>
        void call( const std::string& function, Args... args )
        {
            if( !functionExists( function ) || ( mStopOnErrors && mErrors ) )
                return;
            
            try{
                luabind::call_function<void>( mState, function.c_str(), args... );
            }
            catch( luabind::error error ){
                mErrors = true;
                mLastErrorString = getErrorMessage();
                std::cout << "Lua Error trying to call " << function << " : " << std::endl << mLastErrorString << std::endl;
            }
        }
		
		void setup();
		void update();
		void draw();

		void mouseDown( ci::app::MouseEvent event );
		void mouseUp( ci::app::MouseEvent event );
		void mouseMove( ci::app::MouseEvent event );
		void mouseDrag( ci::app::MouseEvent event );
		void mouseWheel( ci::app::MouseEvent event );
		void keyDown( ci::app::KeyEvent event );
		void keyUp( ci::app::KeyEvent event );

	private:
        static int panic( lua_State* L );
		void addClassSupport();
        
        std::string getErrorMessage();
        
		lua_State*                  mState;

		std::string                 mLastErrorString;
		bool                        mStopOnErrors;
		bool                        mErrors;
	};
};