/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_EXPRESS_HTTP
#define NODEPP_EXPRESS_HTTP

/*────────────────────────────────────────────────────────────────────────────*/

#define MIDDL function_t<void,express_http_t&,function_t<void>>
#define CALBK function_t<void,express_http_t&>
#define MIMES express_tcp_t

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>

#include <nodepp/optional.h>
#include <nodepp/cookie.h>
#include <nodepp/stream.h>
#include <nodepp/https.h>
#include <nodepp/http.h>
#include <nodepp/path.h>
#include <nodepp/json.h>
#include <nodepp/zlib.h>
#include <nodepp/url.h>
#include <nodepp/fs.h>

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_EXPRESS_GENERATOR
#define NODEPP_EXPRESS_GENERATOR
namespace nodepp { namespace _express_ {

GENERATOR( ssr ) {
protected:

    _file_::until rdd; _file_::write wrt;
    ptr_t<bool> child=new bool(0);
    ptr_t<bool> state=new bool(0);
    array_t<ptr_t<ulong>> match;
    string_t      raw, dir;
    ulong         pos, sop;
    file_t        file;
    ptr_t<ulong>  reg;
    ptr_t<ssr>    cb;

protected:

    void next() {
    auto war=raw.slice( reg[0], reg[1] );
         dir=regex::match( war,"[^<°> \n\t]+" );
    }

    void set( string_t data ){
        raw  =data; pos=0; sop=0;
        match=regex::search_all(raw,"<°[^°]+°>");
    }

public:

    ssr()       : child(new bool(0)) {}
    ssr( bool ) : child(new bool(1)) {}

    template< class T >
    coEmit( T& str, string_t path ){
        
        if( !str.is_available() ){ return -1; }
        if( *state == 1 )        { return  1; }

        if( str.get_borrow().size()>CHUNK_SIZE ){ 
            while( wrt(&str,str.get_borrow())==1 );
            str.set_borrow(nullptr); return 1; 
        }

    gnStart

        if( path.size()<=MAX_PATH ){

            if( !url::is_valid(path) ){
            if( str.params.has(path) ){
                set( str.params[path] ); while( sop!=match.size() ){
    
                reg=match[sop]; cb=new ssr(1); next(); 
                str.get_borrow()+=raw.slice( pos, reg[0] );
                pos=match[sop][1];sop++;coWait((*cb)(str,dir)==1 );
    
            } coGoto(2);
            } elif( !fs::exists_file( path ) ){ coGoto(1); }
                
                file=fs::readable(path); cb=new ssr(1); 
                rdd =_file_::until(); set( nullptr ); 
                
                while( file.is_available() ){

                    do { if( pos%2==0 ){ dir="<°"; }
                         if( pos%2!=0 ){ dir="°>"; }
                         coWait( rdd( &file, dir )==1 ); 
                    } while(0);

                    if( rdd.state<=MAX_PATH && regex::test(rdd.data,dir) ){
                        pos++; continue; 
                    } elif( rdd.state<=MAX_PATH && pos%2!=0 ) {
                        dir=regex::match( rdd.data,"[^<°> \n\t]+" );
                        if( dir.empty() ){ continue; }
                        coWait((*cb)( str, dir )==1 );
                    } else {
                        str.get_borrow()+=rdd.data;
                    }

            } coGoto(4);
            } else {

                if( url::protocol( path )=="http" ){ do {
                    
                    auto self = type::bind( this );
                    auto strm = type::bind( str );
                    fetch_t args; *state=1;
                        
                    args.url     = path;
                    args.method  = "GET";
                    args.query   = str.query;
                    args.headers = header_t({
                        { "Params", query::format( str.params ) },
                        { "Host"  , url::hostname( path ) }
                    });
        
                    http::fetch( args ).fail([=](...){ *self->state=0; })
                                       .then([=]( http_t cli ){
                        if( !str.is_available() ){ return; }
                        cli.onData([=]( string_t data ){ strm->get_borrow()+=data; });
                        cli.onDrain.once([=](){ *self->state=0; });
                        stream::pipe( cli );
                    });
        
                } while(0); coNext; 
                } elif( url::protocol( path )=="https" ) { do {

                    ssl_t ssl; fetch_t args; *state=1;
                    auto self = type::bind( this );
                    auto strm = type::bind( str );
         
                    args.url     = path;
                    args.method  = "GET";
                    args.query   = str.query;
                    args.headers = header_t({
                        { "Params", query::format( str.params ) },
                        { "Host"  , url::hostname( path ) }
                    });
        
                    https::fetch( args, &ssl ).fail([=](...){ *self->state=0; })
                                              .then([=]( https_t cli ){
                        if( !str.is_available() ){ return; }
                        cli.onData([=]( string_t data ){ strm->get_borrow()+=data; });
                        cli.onDrain.once([=](){ *self->state=0; });
                        stream::pipe( cli );
                    });
        
                } while(0); coNext; } coGoto(4);

            }

        } else { coYield(1);
            set( path ); while( sop!=match.size() ){

            reg=match[sop]; cb=new ssr(1); next();
            str.get_borrow()+=raw.slice( pos, reg[0] );
            pos=match[sop][1];sop++;coWait((*cb)(str,dir)==1 );

        } coGoto(2); } coYield(2); 
        
        str.get_borrow()+=raw.slice(pos); coYield(4); 
        if( !(*child) && !str.get_borrow().empty() ){
            coWait( wrt(&str,str.get_borrow())==1 );
            str.set_borrow( nullptr ); coEnd;
        }

    gnStop }

};

/*────────────────────────────────────────────────────────────────────────────*/

GENERATOR( inp ){
public:

    template< class T >
    coEmit( const T& file, const ptr_t<object_t>& done, string_t raw ) {
        static uint _state_=0; if( raw.empty() ){ _state_=0; return -1; }
    gnStart

        try { 
            
            auto data = regex::search( raw, "\r\n\r\n" ); if( data.empty() ) { coEnd; }
            auto hdr  = raw.splice(0,data[0]); header_t obj;
                        raw.splice(0,4); file->close();

            ptr_t<regex_t> regs ({
                  regex_t( "filename=\"([^\"]+)\"", true ),
                  regex_t( "content-type: (.+)", true ),
                  regex_t( "name=\"([^\"]+)\"", true )
            });

            regs[0].search(hdr); if( !regs[0].get_memory().empty() ){ obj["filename"]=regs[0].get_memory()[0]; }
            regs[1].search(hdr); if( !regs[1].get_memory().empty() ){ obj["mimetype"]=regs[1].get_memory()[1]; }
            regs[2].search(hdr); if( !regs[2].get_memory().empty() ){ obj["name"]    =regs[2].get_memory()[2]; }

            for( auto x: obj.keys() ){ if( regex::test( obj[x], "[<\"\'>]|\\N" ) ) { coEnd; } }

            if( !obj.has("filename") ){ (*done)[obj["name"]] = raw; coEnd; } else {
                auto sha = crypto::hash::SHA256();  sha.update( obj["mimetype"] );
                     sha.update( encoder::key::generate("0123456789abcdef",32) );
                     sha.update( obj["filename"] ); sha.update( obj["name"] );
                     sha.update( string::to_string( process::now() ) );
                obj["path"] = path::join( "/tmp", sha.get() + ".tmp" );
                *file = fs::writable( obj["path"] ); 
            }

            if( !(*done)[obj["name"]].has_value() ){ (*done)[obj["name"]] = array_t<object_t>(); }
            auto list=(*done)[obj["name"]].as<array_t<object_t>>(); auto name = obj["name"];
            obj.erase("name"); list.push( json::parse( obj ) ); (*done)[name] = list;

        } catch(...) { coEnd; } 
        
        while( !file->is_closed() ) { file->write( raw ); coNext; }

    gnStop };

};

}}
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class express_http_t : public http_t {
protected:

    struct NODE {
        header_t _headers;
        cookie_t _cookies;
        uint  status= 200;
        int    state= 0;
    };  ptr_t<NODE> exp;

public: query_t params;

    express_http_t ( http_t& cli ) noexcept : http_t( cli ), exp( new NODE() ) { exp->state = 1; }
   ~express_http_t () noexcept { if( exp.count() > 1 ){ return; } exp->state=0; free(); }
    express_http_t () noexcept : exp( new NODE() ) { exp->state = 0; }

    /*.........................................................................*/

    bool is_express_available() const noexcept { return exp->state >  0; }
    bool is_express_closed()    const noexcept { return exp->state <= 0; }

    /*.........................................................................*/

    promise_t<object_t,except_t> parse_stream() const noexcept {

        auto tsk  = type::bind( _express_::inp() );
        auto read = type::bind( _file_::until() );
        auto done = type::bind( object_t() );
        auto prv  = type::bind( string_t() );
        auto file = type::bind( file_t() );
        auto self = type::bind( this );

    return promise_t<object_t,except_t>( [=]( function_t<void,object_t> res, function_t<void,except_t> rej ){
        if( !self->headers.has("Content-Length") ){ rej( except_t( "content length mismatch" ) ); return; }

        auto len = type::bind( string::to_long( self->headers["Content-Length"] ) );
        auto bon = regex::match( self->headers["Content-Type"], "boundary=[^ ]+" ).slice(9);
        if ( bon.empty() ){ res(json::parse(query::parse(url::normalize("?"+self->read())))); return; }

        process::poll::add([=](){
            if( self->is_closed() ){ rej(except_t( "something went wrong" )); return -1; }
        coStart

        while( *len>0 && self->is_available() ) {
           while((*read)( &self, bon )==1){ coNext;    }
              if( read->data == "--" )    { coGoto(1); }
              if( read->data == "--\r\n" ){ break; }
              if( read->data == bon ){
                //*tsk = type::bind( _express_::inp() );
                  (*tsk)( file, done, prv->slice(0,-4) );
                   *prv = nullptr; coGoto(1);
                } else { (*tsk)( file, done, *prv ); }
                   *prv = read->data; coYield(1);
                   *len-= read->state;
            } res( *done );

        coStop });

    }); }

    /*.........................................................................*/

     const express_http_t& send( string_t msg ) const noexcept {
        if( exp->state == 0 ){ return (*this); }
        header( "Content-Length", string::to_string(msg.size()) );
        if( regex::test( headers["Accept-Encoding"], "gzip" ) && msg.size()>UNBFF_SIZE ){
            header( "Content-Encoding", "gzip" ); send();
            write( zlib::gzip::get( msg ) ); close();
        } else {
            send(); write( msg ); close();
        }   exp->state =0; return (*this);
    }

    const express_http_t& sendFile( string_t dir ) const noexcept {
        if( exp->state == 0 ){ return (*this); } if( fs::exists_file( dir ) == false )
          { status(404).send("file does not exist"); } file_t file ( dir, "r" );
            header( "Content-Length", string::to_string(file.size()) );
            header( "Content-Type", path::mimetype(dir) );
        if( regex::test( headers["Accept-Encoding"], "gzip" ) ){
            header( "Content-Encoding", "gzip" ); send();
            zlib::gzip::pipe( file, *this );
        } else {
            send(); stream::pipe( file, *this );
        }   exp->state = 0; return (*this);
    }

    const express_http_t& sendJSON( object_t json ) const noexcept {
        if( exp->state == 0 ){ return (*this); } auto data = json::stringify(json);
        header( "content-length", string::to_string(data.size()) );
        header( "content-type", path::mimetype(".json") );
        send( data ); exp->state = 0; return (*this);
    }

    const express_http_t& cache( ulong time ) const noexcept {
        if( exp->state == 0 ){ return (*this); }
        header( "Cache-Control",string::format( "public, max-age=%lu",time) );
        return (*this);
    }

    const express_http_t& cookie( string_t name, string_t value ) const noexcept {
        if( exp->state == 0 ){ return (*this); } exp->_cookies[ name ] = value;
        header( "Set-Cookie", cookie::format( exp->_cookies ) );
        return (*this);
    }

    const express_http_t& header( string_t name, string_t value ) const noexcept {
        if( exp->state == 0 )    { return (*this); }
        exp->_headers[name]=value; return (*this);
    }

    const express_http_t& redirect( uint value, string_t url ) const noexcept {
        if( exp->state == 0 ){ return (*this); }
        header( "location",url ); status( value );
        send(); exp->state = 0; return (*this);
    }

    template< class T >
    const express_http_t& sendStream( T readableStream ) const noexcept {
       if( exp->state == 0 ){ return (*this); }
       if( regex::test( headers["Accept-Encoding"], "gzip" ) ){
            header( "Content-Encoding", "gzip" ); send();
            zlib::gzip::pipe( readableStream, *this );
        } else { send();
            stream::pipe( readableStream, *this );
        }   exp->state = 0; return (*this);
    }

    const express_http_t& header( header_t headers ) const noexcept {
        if( exp->state == 0 ){ return (*this); }
        forEach( item, headers.data() ){
            header( item.first, item.second );
        }   return (*this);
    }

    const express_http_t& redirect( string_t url ) const noexcept {
        if( exp->state == 0 ){ return (*this); }
        return redirect( 302, url );
    }

    const express_http_t& render( string_t path ) const noexcept {
        if( exp->state == 0 ){ return (*this); }
        send(); auto cb = _express_::ssr();
        process::poll::add( cb, *this, path );
        return (*this);
    }

    const express_http_t& status( uint value ) const noexcept {
        if( exp->state == 0 ){ return (*this); }
            exp->status=value; return (*this);
    }

    const express_http_t& clear_cookies() const noexcept {
        if( exp->state == 0 ){ return (*this); }
        header( "Clear-Site-Data", "\"cookies\"" );
        return (*this);
    }

    const express_http_t& send() const noexcept {
        if( exp->state == 0 ){ return (*this); }
        write_header(exp->status,exp->_headers);
        exp->state = 0; return (*this);
    }

    const express_http_t& done() const noexcept {
        if( exp->state == 0 ){ return (*this); }
        exp->state = 0; return (*this);
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class express_tcp_t {
protected:

    struct express_item_t {
        optional_t<MIDDL> middleware;
        optional_t<CALBK> callback;
        optional_t<MIMES> router;
        string_t          method;
        string_t          path;
    };

    struct NODE {
        queue_t<express_item_t> list;
        agent_t* agent= nullptr;
        string_t path = nullptr;
        tcp_t    fd;
    };  ptr_t<NODE> obj;

    void execute( string_t path, express_item_t& data, express_http_t& cli, function_t<void>& next ) const noexcept {
          if( data.middleware.has_value() ){ data.middleware.value()( cli, next ); }
        elif( data.callback.has_value()   ){ data.callback.value()( cli ); next(); }
        elif( data.router.has_value()     ){ data.router.value().run( path, cli ); next(); }
    }

    bool path_match( express_http_t& cli, string_t base, string_t path ) const noexcept {
        string_t pathname = normalize( base, path );

        array_t<string_t> _path[2] = {
            string::split( cli.path, '/' ),
            string::split( pathname, '/' )
        };

        if( regex::test( cli.path, "^"+pathname ) ){ return true;  }
        if( _path[0].size() != _path[1].size() )   { return false; }

        for ( ulong x=0; x<_path[0].size(); x++ ){ if( _path[1][x]==nullptr ){ return false; }
        elif( _path[1][x][0] == ':' ){ if( _path[0][x].empty() ){ return false; }
              cli.params[_path[1][x].slice(1)] = url::normalize( _path[0][x] ); }
        elif( _path[1][x].empty()        ){ continue;     }
        elif( _path[1][x] == "*"         ){ continue;     }
        elif( _path[1][x] != _path[0][x] ){ return false; }}

        return true;
    }

    void run( string_t path, express_http_t& cli ) const noexcept {

        auto n     = obj->list.first();
        auto _base = normalize( path, obj->path );
        function_t<void> next = [&](){ n = n->next; };

        while ( n!=nullptr ) {
            if( !cli.is_available() || cli.is_express_closed() ){ break; }
            if(( n->data.path=="*" && regex::test( cli.path, "^"+_base ))
            || ( n->data.path=="*" && obj->path.empty() )
            || ( path_match( cli, _base, n->data.path ) ) ){
            if ( n->data.method.empty() || n->data.method==cli.method ){
                 execute( _base, n->data, cli, next );
            } else { next(); }
            } else { next(); }
        }

    }

    string_t normalize( string_t base, string_t path ) const noexcept {
    auto new_path =  base.empty() ? ("/"+path) : path.empty() ?
                                    ("/"+base) : path::join( base, path );
    return path::normalize( new_path );
    }

public:

    express_tcp_t( agent_t* agent ) noexcept : obj( new NODE() ){ obj->agent = agent; }
    express_tcp_t() noexcept : obj( new NODE() ) {}

    /*.........................................................................*/

    void     set_path( string_t path ) const noexcept { obj->path = path; }
    string_t get_path()                const noexcept { return obj->path; }

    /*.........................................................................*/

    bool is_closed() const noexcept { return obj->fd.is_closed(); }
    void     close() const noexcept { obj->fd.close(); }
    tcp_t   get_fd() const noexcept { return obj->fd; }

    /*.........................................................................*/

    const express_tcp_t& RAW( string_t _method, string_t _path, CALBK cb ) const noexcept {
        express_item_t item; // memset( (void*) &item, 0, sizeof(item) );
        item.path     = _path.empty() ? "*" : _path;
        item.method   = _method;
        item.callback = cb;
        obj->list.push( item ); return (*this);
    }

    const express_tcp_t& RAW( string_t _method, CALBK cb ) const noexcept {
        return RAW( _method, nullptr, cb );
    }

    const express_tcp_t& RAW( CALBK cb ) const noexcept {
        return RAW( nullptr, nullptr, cb );
    }

    /*.........................................................................*/

    const express_tcp_t& USE( string_t _path, express_tcp_t cb ) const noexcept {
        express_item_t item; // memset( (void*) &item, 0, sizeof(item) );
        cb.set_path( normalize( obj->path, _path ) );
        item.method     = nullptr;
        item.path       = "*";
        item.router     = optional_t<MIMES>(cb);
        obj->list.push( item ); return (*this);
    }

    const express_tcp_t& USE( express_tcp_t cb ) const noexcept {
        return USE( nullptr, cb );
    }

    /*.........................................................................*/

    const express_tcp_t& USE( string_t _path, MIDDL cb ) const noexcept {
        express_item_t item; // memset( (void*) &item, 0, sizeof(item) );
        item.path       = _path.empty() ? "*" : _path;
        item.middleware = optional_t<MIDDL>(cb);
        item.method     = nullptr;
        obj->list.push( item ); return (*this);
    }

    const express_tcp_t& USE( MIDDL cb ) const noexcept {
        return USE( nullptr, cb );
    }

    /*.........................................................................*/

    const express_tcp_t& ALL( string_t _path, CALBK cb ) const noexcept {
        return RAW( nullptr, _path, cb );
    }

    const express_tcp_t& ALL( CALBK cb ) const noexcept {
        return RAW( cb );
    }

    /*.........................................................................*/

    const express_tcp_t& GET( string_t _path, CALBK cb ) const noexcept {
        return RAW( "GET", _path, cb );
    }

    const express_tcp_t& GET( CALBK cb ) const noexcept {
        return RAW( "GET", cb );
    }

    /*.........................................................................*/

    const express_tcp_t& POST( string_t _path, CALBK cb ) const noexcept {
        return RAW( "POST", _path, cb );
    }

    const express_tcp_t& POST( CALBK cb ) const noexcept {
        return RAW( "POST", cb );
    }

    /*.........................................................................*/

    const express_tcp_t& QUERY( string_t _path, CALBK cb ) const noexcept {
        return RAW( "QUERY", _path, cb );
    }

    const express_tcp_t& QUERY( CALBK cb ) const noexcept {
        return RAW( "QUERY", cb );
    }

    /*.........................................................................*/

    const express_tcp_t& REMOVE( string_t _path, CALBK cb ) const noexcept {
        return RAW( "DELETE", _path, cb );
    }

    const express_tcp_t& REMOVE( CALBK cb ) const noexcept {
        return RAW( "DELETE", cb );
    }

    /*.........................................................................*/

    const express_tcp_t& PUT( string_t _path, CALBK cb ) const noexcept {
        return RAW( "PUT", _path, cb );
    }

    const express_tcp_t& PUT( CALBK cb ) const noexcept {
        return RAW( "PUT", cb );
    }

    /*.........................................................................*/

    const express_tcp_t& HEAD( string_t _path, CALBK cb ) const noexcept {
        return RAW( "HEAD", _path, cb );
    }

    const express_tcp_t& HEAD( CALBK cb ) const noexcept {
        return RAW( "HEAD", cb );
    }

    /*.........................................................................*/

    const express_tcp_t& TRACE( string_t _path, CALBK cb ) const noexcept {
        return RAW( "TRACE", _path, cb );
    }

    const express_tcp_t& TRACE( CALBK cb ) const noexcept {
        return RAW( "TRACE", cb );
    }

    /*.........................................................................*/

    const express_tcp_t& PATCH( string_t _path, CALBK cb ) const noexcept {
        return RAW( "PATCH", _path, cb );
    }

    const express_tcp_t& PATCH( CALBK cb ) const noexcept {
        return RAW( "PATCH", cb );
    }

    /*.........................................................................*/

    const express_tcp_t& OPTIONS( string_t _path, CALBK cb ) const noexcept {
        return RAW( "OPTION", _path, cb );
    }

    const express_tcp_t& OPTIONS( CALBK cb ) const noexcept {
        return RAW( "OPTION", cb );
    }

    /*.........................................................................*/

    const express_tcp_t& CONNECT( string_t _path, CALBK cb ) const noexcept {
        return RAW( "CONNECT", _path, cb );
    }

    const express_tcp_t& CONNECT( CALBK cb ) const noexcept {
        return RAW( "CONNECT", cb );
    }

    /*.........................................................................*/

    template<class... T>
    tcp_t& listen( const T&... args ) const noexcept {
        auto self = type::bind( this );

        function_t<void,http_t> cb = [=]( http_t cli ){
            express_http_t res(cli); if( cli.headers.has("Params") ){
                res.params= query::parse( cli.headers["Params"] );
            }   self->run( nullptr, res );
        };

        obj->fd=http::server( cb, obj->agent );
        obj->fd.listen( args... ); return obj->fd;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace express { namespace http {

    template< class... T > express_tcp_t add( T... args ) { return express_tcp_t(args...); }

    express_tcp_t file( string_t base ) { express_tcp_t app;

        app.ALL([=]( express_http_t& cli ){

            auto pth = regex::replace( cli.path, app.get_path().slice(1), "/" );
                 pth = regex::replace_all( pth, "\\.[.]+/", "" );

            auto dir = pth.empty()? path::join( base, "" ) :
                                    path::join( base,pth ) ;

            if( dir.empty() ){ dir = path::join( base, "index.html" ); }
            if( dir[dir.last()] == '/' ){ dir += "index.html"; }

            if( fs::exists_file(dir+".html")== true ){ dir += ".html"; }
            if(!fs::exists_file(dir) || dir == base ){
            if(!path  ::extname(dir).empty() ){ cli.status(404).send("not_found"); return; }
            if( fs::exists_file( path::join( base, "404.html" ) )){
                dir = path::join( base, "404.html" ); cli.status(404);
            } else { cli.status(404).send("Oops 404 Error"); return; } }

            if( !cli.headers.has("Range") ){

                if( regex::test(path::mimetype(dir),"audio|video",true) ){ cli.send(); return; }
                if( regex::test(path::mimetype(dir),"html",true) ){ cli.render(dir); } else {
                    cli.header( "Cache-Control", "public, max-age=604800" );
			        cli.sendFile( dir );
                }

            } else { auto str = fs::readable( dir );

                array_t<string_t> range = regex::match_all(cli.headers["Range"],"\\d+",true);
                   ulong rang[3]; rang[0] = string::to_ulong( range[0] );
                         rang[1] =min(rang[0]+CHUNK_MB(10),str.size()-1);
                         rang[2] =min(rang[0]+CHUNK_MB(10),str.size()  );

                cli.header( "Content-Range", string::format("bytes %lu-%lu/%lu",rang[0],rang[1],str.size()) );
                cli.header( "Content-Type",  path::mimetype(dir) ); cli.header( "Accept-Range", "bytes" );
                cli.header( "Cache-Control", "public, max-age=604800" );

                str.set_range( rang[0], rang[2] );
                cli.status(206).sendStream( str );

            }
        });

        return app;
    }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

#undef MIMES
#undef CALBK
#undef MIDDL
#endif