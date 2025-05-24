# NODEPP-EXPRESS

This project provides a basic and clear implementation of the popular ExpressJS framework running within the [NodePP](https://github.com/NodeppOficial/nodepp) environment. It aims to showcase the fundamental aspects of integrating these two technologies for web development, offering a simple starting point for developers interested in exploring this combination. You'll find the core elements needed to get a basic ExpressJS application up and running within NodePP.

🔗: [Building Web Applications in C++ with ExpressPP: A Comprehensive Guide](https://medium.com/@EDBCBlog/building-web-applications-in-c-with-expresspp-a-comprehensive-guide-895f88314173)

# Features
- **ExpressPP Functionality:**
    - **Minimalist Framework:** Leverages the core, unopinionated nature of ExpressJS.
    - **Powerful Routing:** Utilizes ExpressPP's robust routing system to map URLs to handlers.
    - **Middleware Support:** Enables the use of ExpressPP middleware for tasks like logging, authentication, etc.
    - **HTTP Utility Methods:** Provides ExpressPP's convenient methods for handling HTTP requests and responses.
    - **Static File Serving:** Facilitates serving static assets like CSS, JavaScript, and images.
    - **RESTful API Development:** Simplifies the creation of RESTful APIs.

- **NodePP Integration:**
    - **Asynchronous and Event-Driven:** Runs ExpressPP within NodePP's asynchronous, event-driven architecture, built with C++ for high performance.
    - **Node.js-like API:** Offers a familiar Node.js-style API for writing C++ code.
    - **High Performance:** Benefits from NodePP's C++ foundation for potentially faster execution.
    - **Scalability:** Aims to build scalable web applications leveraging NodePP's capabilities.
    - **Networking Support:** Could potentially utilize NodePP's TCP, TLS, UDP, HTTP, and WebSocket support.

## Dependencies
```bash
# Openssl
🪟: pacman -S mingw-w64-ucrt-x86_64-openssl
🐧: sudo apt install libssl-dev

# Zlib
🪟: pacman -S mingw-w64-ucrt-x86_64-zlib
🐧: sudo apt install zlib1g-dev

# Nodepp
💻: https://github.com/NodeppOficial/nodepp
```

## Build & Run
```bash
🪟: g++ -o main main.cpp -I ./include -lz -lws2_32 -lssl -lcrypto ; ./main
🐧: g++ -o main main.cpp -I ./include -lz -lssl -lcrypto ; ./main
```

## Usage

```cpp
#include <nodepp/nodepp.h>
#include <express/http.h>

using namespace nodepp;

void onMain() {

    auto app = express::http::add();

    app.USE([]( express_http_t cli, function_t<void> next ){
        console::log( "this is a middleware" );
        next();
    });

    app.GET("/test",[]( express_http_t cli ){
        cli.status(200)
           .header( "content-type", "text/plain" )
           .send("this is an ExpressPP test");
    });

    app.GET([]( express_http_t cli ){
        cli.status(200)
           .header( "content-type", "text/html" )
           .render( R"(
                <h1> 
                    <° http://localhost:8000/test °> 
                </h1>
            )" );
    });

    app.listen( "localhost", 8000, []( ... ){
        console::log( "server started at:" );
        console::log( "http://localhost:8000" );
    });

}
```

## License

**Nodepp** is distributed under the MIT License. See the LICENSE file for more details.
