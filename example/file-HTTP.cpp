#include <nodepp/nodepp.h>
#include <express/http.h>

using namespace nodepp;

void onMain() {

    auto app = express::http::add();

    app.POST( "/file", []( express_http_t cli ){

        cli.parse_stream().then([=]( object_t obj ){
            console::log( ">>", json::stringify( obj ) );
            cli.header( "Content-Type", "text/html" );
            cli.send  (" done ");
        }).fail([=]( except_t err ){
            console::log( ">>", err );
        });

    } );

    app.GET( []( express_http_t cli ){

        cli.header( "Content-Type", "text/html" );
        cli.send  ( NODEPP_STRINGIFY (

            <form action="/file" enctype="multipart/form-data" method="POST">
                <h1> Send File </h1>
                <input type="file" name="file" />
                <input type="submit" />
            </form>

        ) );

    } );

    app.listen( "localhost", 8000, []( socket_t ){
        console::log( "server started at http://localhost:8000" );
    });

}
