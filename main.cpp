#include <nodepp/nodepp.h>
#include <express/http.h>
#include <nodepp/cluster.h>

using namespace nodepp;

void onMain() {

    /*
    for( auto x=os::cpus(); x-->0; ){
    if ( process::is_child() ){ break; }
         cluster::add();
    }

    if ( process::is_parent() ){ 
         console::log( "task spawned" );    
    return; }
    */

    auto app = express::http::add();

    app.GET( "/test/:uid", [=]( express_http_t cli ){
        
        cli.send( regex::format( _STRING_(
            <h1> ${0} </h1>
            <h2> ${1} </h2>
        ), cli.params["uid"], cli.search ));

    });

    app.GET([=]( express_http_t cli ){
        cli.send( "hello world!" );
    });

//  app.USE( express::http::file( "./www" ) );

    app.listen( "localhost", 8000, []( ... ){
        console::log( "server started at:" );
        console::log( "http://localhost:8000" );
    });

}
