#include <nodepp/nodepp.h>
#include <express/https.h>

using namespace nodepp;

void onMain() {

    auto ssl = ssl_t();
    auto app = express::https::add( &ssl );

    app.GET( "/test/:uid", [=]( express_https_t cli ){
        
        cli.send( regex::format( _STRING_(
            <h1> ${0} </h1>
            <h2> ${1} </h2>
        ), cli.params["uid"], json::stringify( cli.query ) ));

    });

    app.USE( express::https::file( "./www" ) );

    app.listen( "localhost", 8000, []( ... ){
        console::log( "server started at:" );
        console::log( "https://localhost:8000" );
    });

}
