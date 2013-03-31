#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "Script.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicApp : public AppNative {
  public:
	void setup();
};

void BasicApp::setup()
{
    lua::Script script;
    script.loadString( "print( toString( randVec3f() * Vec3f( 10.0, 3.0, 2.0 ) ) )" );
    
    getWindow()->getSignalDraw().connect( [this]{ gl::clear(); } );
}

CINDER_APP_NATIVE( BasicApp, RendererGl )
