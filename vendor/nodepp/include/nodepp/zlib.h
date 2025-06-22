/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_ZLIB
#define NODEPP_ZLIB

/*────────────────────────────────────────────────────────────────────────────*/

#include <zlib.h>
#include <zconf.h>
#include "stream.h"
#include "generator.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class zlib_t {
protected:

    struct NODE {
        z_stream  fd;
        bool state= 1;
        int  mode = 0;
        int  type = 0;
        ptr_t<char> bff;
    };  ptr_t<NODE> obj;

    void _init_() const noexcept {
        obj->fd.zfree    = Z_NULL;
        obj->fd.zalloc   = Z_NULL;
        obj->fd.opaque   = Z_NULL;
        obj->fd.next_in  = Z_NULL;
        obj->fd.avail_in = Z_NULL;
    }

public:

    event_t<except_t>  onError;
    event_t<>          onDrain;
    event_t<>          onClose;
    event_t<>          onOpen;
    event_t<string_t>  onData;
    
    /*─······································································─*/
    
    virtual ~zlib_t() noexcept { if( obj.count()>1 || obj->state==0 ){ return; } free(); }

    zlib_t( int type=0, ulong size=CHUNK_SIZE ) noexcept : obj( new NODE ) { 
        obj->bff  = ptr_t<char>( size ); 
        obj->type = type; _init_(); 
    }

    /*─······································································─*/
    
    virtual void free() const noexcept {
        if( obj->state == 0 ){ return; } obj->state = 0;
        if( obj->mode  ==-1 ){ inflateEnd( &obj->fd ); }
        if( obj->mode  == 1 ){ deflateEnd( &obj->fd ); }
        onDrain.emit(); onClose.emit();
    }
    
    /*─······································································─*/

    void        close() const noexcept { free(); }
    bool    is_closed() const noexcept { return obj->state == 0; }
    bool is_available() const noexcept { return obj->state == 1; }
    
    /*─······································································─*/

    string_t update_inflate( string_t data, int mode=Z_PARTIAL_FLUSH ) const noexcept {
        if( is_closed() || data.size() == 0 || obj->mode < 0 ){ return nullptr; }

        if( obj->mode == 0 ){ if( inflateInit2( &obj->fd, obj->type ) != Z_OK ){ 
            _EERROR( onError, "Failed to initialize zlib for decompression." ); close(); return nullptr;
        } onOpen.emit(); obj->mode = 1; } string_t output; ulong size =0;
        
            obj->fd.avail_in = data.size();
            obj->fd.avail_out= obj->bff.size();
            obj->fd.next_in  = (Bytef*)data.get();
            obj->fd.next_out = (Bytef*)obj->bff.get();
        
            auto x = ::inflate( &obj->fd, mode );

            if(( size=obj->bff.size()-obj->fd.avail_out )>0 ){
                auto raw = string_t( obj->bff.get(), size );
            if( onData.empty() ){ output += raw; } else {
                onData.emit( raw );
            }}

            if( x == Z_STREAM_END ){ return output; } elif( x < 0 ) {
                _EERROR( onError, "Compression failed:", obj->fd.msg );
                close(); return nullptr;
            }

        return output;
        
    }

    string_t update_deflate( string_t data, int mode=Z_PARTIAL_FLUSH ) const noexcept { 
        if( is_closed() || data.size() == 0 || obj->mode > 0 ){ return nullptr; }
        
        if( obj->mode == 0 ){ if( deflateInit2( &obj->fd, Z_DEFAULT_COMPRESSION, Z_DEFLATED, obj->type, 8, Z_DEFAULT_STRATEGY ) != Z_OK ){ 
            _EERROR( onError, "Failed to initialize zlib for compression." ); close(); return nullptr;
        } onOpen.emit(); obj->mode = -1; } string_t output; ulong size =0;

            obj->fd.avail_in = data.size();
            obj->fd.avail_out= obj->bff.size();
            obj->fd.next_in  = (Bytef*)data.get();
            obj->fd.next_out = (Bytef*)obj->bff.get();
        
            auto x = ::deflate( &obj->fd, mode );

            if(( size=obj->bff.size()-obj->fd.avail_out )>0 ){
                auto raw = string_t( obj->bff.get(), size );
            if( onData.empty() ){ output += raw; } else {
                onData.emit( raw );
            }}

            if( x == Z_STREAM_END ){ return output; } elif( x < 0 ) {
                _EERROR( onError, "Compression failed:", obj->fd.msg );
                close(); return nullptr;
            }

        return output;
        
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace zlib { namespace inflate {

    template< class... T >
    void pipe( const T&... file ){ 
        _zlib_::pipe_inflate task; auto zlib = zlib_t(-15);
        process::poll::add( task, zlib, file... );
    }
    
    zlib_t get(){ return zlib_t(-15); }
    
    string_t get( const string_t& data ){ 
        return zlib_t(-15).update_inflate(data,Z_FINISH); 
    }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace zlib { namespace deflate {

    template< class... T >
    void pipe( const T&... file ){
        _zlib_::pipe_deflate task; auto zlib = zlib_t(-15);
        process::poll::add( task, zlib, file... );
    }
    
    zlib_t get(){ return zlib_t(-15); }

    string_t get( const string_t& data ){ 
        return zlib_t(-15).update_deflate(data,Z_FINISH); 
    } 

}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace zlib { namespace gunzip {

    template< class... T >
    void pipe( const T&... file ){
        _zlib_::pipe_inflate arg; auto zlib = zlib_t(15|32);
        process::poll::add( arg, zlib, file... );
    }
    
    zlib_t get(){ return zlib_t(15|32); }

    string_t get( const string_t& data ){ 
        return zlib_t(15|32).update_inflate(data,Z_FINISH); 
    }
    
}}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace zlib { namespace gzip {

    template< class... T >
    void pipe( const T&... file ){
        _zlib_::pipe_deflate task; auto zlib = zlib_t(15|16);
        process::poll::add( task, zlib, file... );
    }
    
    zlib_t get(){ return zlib_t(15|16); }

    string_t get( const string_t& data ){ 
        return zlib_t(15|16).update_deflate(data,Z_FINISH); 
    }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
