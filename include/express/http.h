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
#define ROUTR express_tcp_t

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>

#include <nodepp/optional.h>
#include <nodepp/cookie.h>
#include <nodepp/crypto.h>
#include <nodepp/stream.h>
#include <nodepp/http.h>
#include <nodepp/path.h>
#include <nodepp/json.h>
#include <nodepp/url.h>
#include <nodepp/fs.h>
#include <nodepp/os.h>

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef EXPRESS_ALLOW_ZLIB_COMPRESSION
#define EXPRESS_ALLOW_ZLIB_COMPRESSION 1
#endif

#if EXPRESS_ALLOW_ZLIB_COMPRESSION == 1
#include <nodepp/zlib.h>
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef EXPRESS_GENERATOR
#define EXPRESS_GENERATOR

namespace nodepp { namespace express { GENERATOR( post_parser ){
public:

    template< class T >
    coEmit( const T& file, const ptr_t<object_t>& done, string_t raw ) {
        static regex_t reg8 = regex_t( "Content-Disposition", false );
        static regex_t reg7 = regex_t( "\r\n\r\n", true );
        if( raw.empty() ){ _state_=0; return -1; }
    coBegin

        do { 
            
            if( !reg8.test( raw ) ){ coSet(0); return -1; }
            auto pos = reg7.search( raw );
            if( pos.empty() ) /**/ { coSet(0); return -1; }

            auto hdr = raw.slice( 0, pos[0] );
            auto bdy = raw.slice(/**/pos[1] );
            auto sby = bdy.slice( 0, 1024   ); header_t obj;

            thread_local static ptr_t<regex_t> regs ({
                  regex_t( "filename=\"([^\"]+)\"", true ),
                  regex_t( "content-type: (\\n+)" , true ),
                  regex_t( "name=\"([^\"]+)\""    , true ),
                  regex_t( "^([^\r]+)\r\n"        , true )
            });

            regs[0].search(hdr); if( !regs[0].get_memory().empty() ){ obj["filename"]=regs[0].get_memory()[0]; }
            regs[1].search(hdr); if( !regs[1].get_memory().empty() ){ obj["mimetype"]=regs[1].get_memory()[0]; }
            regs[2].search(hdr); if( !regs[2].get_memory().empty() ){ obj["name"]    =regs[2].get_memory()[0]; }
            regs[3].search(sby); if( !regs[3].get_memory().empty() ){ sby /*------*/ =regs[3].get_memory()[0]; }
            /*----------------*/ else /*-------------------------*/ { sby.clear(); }

            regs[0].clear_memory(); regs[1].clear_memory();
            regs[2].clear_memory(); regs[3].clear_memory();

            if( !obj.has("filename") ){ (*done)[obj["name"]] = sby; coEnd; } else {
                auto sha = crypto::hash::SHA256();  sha.update( obj["mimetype"] );
                     sha.update( encoder::key::generate("0123456789abcdef",32) );
                     sha.update( obj["filename"] ); sha.update( obj["name"] );
                     sha.update( string::to_string( process::now() ) );
                obj["path"] = path::join( os::tmp(), sha.get() + ".tmp" );
                *file = fs::writable( obj["path"] );
            }

            if( !(*done)[obj["name"]].has_value() ){ (*done)[obj["name"]] = array_t<object_t>(); }
            auto list=(*done)[obj["name"]].as<array_t<object_t>>(); auto name = obj["name"];
            obj.erase("name"); list.push( json::parse( obj ) ); (*done)[name] = list;
            file->write( bdy );

        } while(0); do { coNext; file->write(raw); } while(1);

    coFinish }

};}}

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

        auto read = type::bind( generator::file::until() );
        auto tsk  = type::bind( express::post_parser() );
        auto done = type::bind( object_t() );
        auto file = type::bind( file_t() );
        auto self = type::bind( this );

    return promise_t<object_t,except_t>( [=]( function_t<void,object_t> res, function_t<void,except_t> rej ){
        if( !self->headers.has("Content-Length") )
          { rej( except_t( "content length mismatch" ) ); return; }
        if( string::to_ulong( self->headers["Content-Length"])==0 )
          { rej( except_t( "content length mismatch" ) ); return; }

        static regex_t reg6 = regex_t( "boundary=[^ ]+", true );
        auto len = type::bind( string::to_ulong( self->headers["Content-Length"] ) );
        auto bon = "--" + reg6.match( self->headers["Content-Type"] ).slice(9);
        if ( bon.empty() ){ res(json::parse(query::parse(url::normalize("?"+self->read())))); return; }

        process::add( coroutine::add( COROUTINE(){
            if( self->is_closed() ){ rej("something went wrong"); return -1; }
        coBegin

            while( *len>0 && self->is_available() ) {
           coWait((*read)( &self, bon )==1 ); *len-=min( (ulong) read->state,*len );
               if( read->state<= 0         ){ coGoto(1); }
               if( read->data != bon       ){
                        (*tsk)(file,done,read->data);
               } else { (*tsk)(file,done,nullptr   ); }
            }

            res(*done); coEnd; coYield(1); do {
            for( auto x: done->keys() ){
            if ((*done)[x].is<array_t<object_t>>() ){
            for( auto y:(*done)[x].as<array_t<object_t>>() ){
                 fs::remove_file( y["path"].as<string_t>() );
            }}}} while(0); rej("something went wrong");

        coFinish
        }));

    }); }

    /*.........................................................................*/

     const express_http_t& send( string_t msg ) const noexcept {
        if( exp->state==0 && !is_closed() ){ write( msg ); return (*this); }
        if( exp->state==0 ) /*----------*/ { /*---------*/ return (*this); }
        header( "Content-Length", string::to_string(msg.size()) );

    #if EXPRESS_ALLOW_ZLIB_COMPRESSION==1
        if( msg.size() > NODEPP_UNBFF_SIZE ){
        if( regex::test( headers["Accept-Encoding"], "gzip" ) ){
            header( "Content-Encoding", "gzip" ); send();
            write( zlib::gzip::get( msg ) );
        return (*this); }}
    #endif

        send(); write( msg ); return (*this); 
    }

    const express_http_t& send_file( string_t dir ) const noexcept {
        if( exp->state==0 ){ return (*this); }
        if( !fs::exists_file( dir ) ){ 
            status(404).send("file does not exist"); 
        return (*this); } 
        
        file_t file ( dir, "r" );
            
        header( "Content-Length", string::to_string(file.size()) );
        header( "Content-Type"  , path::mimetype(dir) );
        return send_stream( file );

    }

    template< class T >
    const express_http_t& send_stream( T readableStream ) const noexcept {
        if( exp->state == 0 ){ return (*this); }

    #if EXPRESS_ALLOW_ZLIB_COMPRESSION == 1
        if( regex::test( headers["Accept-Encoding"], "gzip" ) ){
            header( "Content-Encoding", "gzip" ); send(); 
            zlib::gzip::pipe( readableStream, *this );
        return (*this); }
    #endif

        send(); stream::pipe( readableStream, *this );
        return (*this);
    }

    const express_http_t& send_json( object_t json ) const noexcept {
        if( exp->state == 0 ){ return (*this); } auto data = json::stringify(json);
        header( "content-type", path::mimetype(".json") );
        send( data ); return (*this);
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
        send(); /*----------*/ return (*this);
    }

    const express_http_t& header( header_t headers ) const noexcept {
        if( exp->state == 0 ){ return (*this); }
        auto x=headers.raw().first(); while( x!=nullptr ){
        auto y=x->next; /*-------------------------------*/
             exp->_headers[x->data.first] = x->data.second;
        x=y; } return (*this);
    }

    const express_http_t& redirect( string_t url ) const noexcept {
        if( exp->state == 0 ){ return (*this); }
        return redirect( 302, url );
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
        done(); return (*this);
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
        optional_t<ROUTR> router;
        string_t /*----*/ method;
        string_t /*----*/ path;
    };

    /*.......................................................................*/

    struct NODE {
        queue_t<express_item_t> list; tcp_t fd;
        queue_t<express_item_t> mddl; 
        ptr_t<agent_t> agent=nullptr;
        string_t /*-*/ path =nullptr;
    };  ptr_t<NODE> obj;

    /*.......................................................................*/

    void execute( string_t path, express_item_t& data, express_http_t& cli, function_t<void>& next ) const noexcept {
        if  ( data.middleware.has_value() ){ data.middleware.value()( cli, next ); return; }
        elif( data.callback  .has_value() ){ data.callback  .value() /*----*/ ( cli ); }
        elif( data.router    .has_value() ){ data.router    .value().run( path, cli ); }
    next(); }

    /*.......................................................................*/

    void insert( string_t path, const express_item_t& item ) const noexcept {
        auto x= obj->list.first(); while( x != nullptr ){
        if( path.size() > x->data.path.size() ){ break; }
        x = x->next; } obj->list.insert( x, item );
    }

    /*.......................................................................*/

    int path_match( express_http_t& cli, string_t base, string_t path ) const noexcept {

        string_t pathname = normalize( base, path );

        if  ( regex::test( cli.path, "^"+pathname ) ){ return 1; }
        
            array_t<string_t> _path[2]= { 
            string::split(cli.path,'/'), 
            string::split(pathname,'/') };

        for ( int x=2; x-->0; ){
        if  ( _path[x][_path[x].first()].empty() ){ _path[x].shift(); }
        if  ( _path[x][_path[x].last ()].empty() ){ _path[x].pop  (); }}

        if  ( _path[0].empty()&& _path[1].empty()){ return  1; }
    //  if  ( _path[0].empty()|| _path[1].empty()){ return -1; }
    //  if  ( _path[0][0]     != _path[1][0]     ){ return -1; }
        if  ( _path[0].size() <  _path[1].size() ){ return  0; }

        for ( ulong x=0; x<_path[1].size(); ++x ){
        if  ( _path[1][x] ==nullptr ){ continue; }
        elif( _path[1][x][0] == ':' ){ if( _path[0][x].empty() )/**/{ return 0; }
              cli.params[_path[1][x].slice(1)] = url::normalize( _path[0][x] ); }
        elif( _path[1][x].empty() /*--*/ ){ continue; }
        elif( _path[1][x] != _path[0][x] ){ return 0; }}

    return 1; }

    /*.......................................................................*/

    void run( string_t path, express_http_t& cli ) const noexcept { do {

        auto     n = obj->mddl.first();
        auto _base = normalize( path, obj->path );
        function_t<void> next = [&](){ n = n->next; };

        while( n!=nullptr && !cli.is_express_closed() ) 
             { execute( _base, n->data, cli, next ); }

    } while(0); if( cli.is_express_closed() ){ return; } do {

        auto     n = obj->list.first();
        auto _base = normalize( path, obj->path );
        function_t<void> next = [&](){ n = n->next; };

        while ( n!=nullptr && !cli.is_express_closed() ) { int c=0;
            if(( n->data.path.empty() && obj->path.empty() )
            || ((c=path_match(cli,_base,n->data.path))==1 )){
            if ( n->data.method.empty() 
            || ( n->data.method==cli.method ) )
               { execute( _base, n->data, cli, next ); continue; }}
        if(c==-1) { break; } next(); }

    } while(0); }

    /*.......................................................................*/

    string_t normalize( string_t base, string_t path ) const noexcept {
        auto new_path =  base.empty() ? ("/"+path) : path.empty() ? /*------*/
        /*---------------------------*/ ("/"+base) : path::join( base, path );
        return path::normalize( new_path );
    }

public:

    express_tcp_t( agent_t* agent=nullptr ) noexcept : obj( new NODE() ){ 
        obj->agent = type::bind( agent ); 
    }
    
    /*.........................................................................*/

    void     set_path( string_t path ) const noexcept { obj->path = path; }
    string_t get_path() /*----------*/ const noexcept { return obj->path; }

    /*.........................................................................*/

    bool is_closed() const noexcept { return obj->fd.is_closed(); }
    void     close() const noexcept { obj->fd.close(); }
    tcp_t   get_fd() const noexcept { return obj->fd; }

    /*.........................................................................*/

    const express_tcp_t& RAW( string_t _method, string_t _path, CALBK cb ) const noexcept {
        express_item_t item; // memset( (void*) &item, 0, sizeof(item) );
        item.path   = _path.empty() ? nullptr : _path;
        item.method = _method; item.callback = cb;
        insert( _path, item ); return (*this);
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
        item.router = optional_t<ROUTR>(cb);
        item.method = nullptr;
        item.path   = nullptr;
        insert( _path, item ); return (*this);
    }

    const express_tcp_t& USE( express_tcp_t cb ) const noexcept {
        return USE( nullptr, cb );
    }

    /*.........................................................................*/

    const express_tcp_t& USE( string_t _path, MIDDL cb ) const noexcept {
        express_item_t item; // memset( (void*) &item, 0, sizeof(item) );
        item.path       = _path.empty() ? nullptr : _path;
        item.middleware = optional_t<MIDDL>(cb);
        item.method     = nullptr;
        if( _path.empty() ){ obj->mddl.push( item ); }
        else /*---------*/ { insert ( _path, item ); }
    return (*this); }

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
            res.params=query::parse( cli.headers["Params"] );
        }   self->run( nullptr, res ); };

        obj->fd=http::server( cb, &obj->agent );
        obj->fd.listen( args... ); return obj->fd;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace express { namespace http {

    template< class... T > express_tcp_t add( T... args ) { return express_tcp_t(args...); }

    inline express_tcp_t file( string_t base ) {

        static ptr_t<regex_t> reg ({
            regex_t( "audio|video", true ),
            regex_t( "\\.[.]+/" ),
            regex_t( "\\d+"     )
        });

        express_tcp_t app; app.ALL([=]( express_http_t& cli ){

            auto pth = regex::replace( cli.path, app.get_path().slice_view(1), "/" );
                 pth = reg[1].replace_all( pth, "" );

            auto dir = pth.empty()? path::join( base, "" ):
            /*-------------------*/ path::join( base,pth );

            if( dir.empty() ) /*-----*/ { dir = base; }
            if( fs::exists_folder(dir) ){ dir+= "index.html"; }
            if(!fs::exists_file  (dir) ){
                cli.send("file not found 404");
            return; }

            auto str = fs::readable( dir );

            if( !cli.headers.has("Range") ){

                if( reg[0].test(path::mimetype(dir)) )
                  { cli.send(); return; }

                cli.header( "Content-Length", string::to_string( str.size() ) );
                cli.header( "Cache-Control" , "public, max-age=604800" );
                cli.header( "Content-Type"  , path::mimetype( dir ) );
                cli.send_stream( str );

            } else {

                array_t<string_t> range = reg[2].match_all(cli.headers["Range"]);
                   ulong rang[3]; rang[0] = string::to_ulong( range[0] );
                         rang[1] =min(rang[0]+CHUNK_MB(10),str.size()-1);
                         rang[2] =min(rang[0]+CHUNK_MB(10),str.size()  );

                cli.header( "Content-Range", string::format("bytes %lu-%lu/%lu",rang[0],rang[1],str.size()) );
                cli.header( "Content-Type",  path::mimetype(dir) ); cli.header( "Accept-Range", "bytes" );
                cli.header( "Cache-Control", "public, max-age=604800" );

                str.set_range( rang[0], rang[2] ); cli.status(206).send();
                stream::pipe ( str, cli );

            }

    }); return app; }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

#undef ROUTR
#undef CALBK
#undef MIDDL
#endif

/*────────────────────────────────────────────────────────────────────────────*/
