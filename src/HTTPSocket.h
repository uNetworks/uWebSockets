#ifndef HTTPSOCKET_UWS_H
#define HTTPSOCKET_UWS_H

#include "Socket.h"
#include <string>
// #include <experimental/string_view>

namespace uWS {

struct Header {
    char *key, *value;
    unsigned int keyLength, valueLength;

    operator bool() {
        return key;
    }

    // slow without string_view!
    std::string toString() {
        return std::string(value, valueLength);
    }
};

enum ContentType {
    TEXT_HTML
};

enum HTTPVerb {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    OPTIONS,
    INVALID
};

struct HttpRequest {
    Header *headers;
    unsigned int requestId;
    Header getHeader(const char *key) {
        return getHeader(key, strlen(key));
    }

    HttpRequest() = default;
    HttpRequest(Header *headers, unsigned int requestId) : headers(headers), requestId(requestId) {}

    Header getHeader(const char *key, size_t length) {
        if (headers) {
            for (Header *h = headers; *++h; ) {
                if (h->keyLength == length && !strncmp(h->key, key, length)) {
                    return *h;
                }
            }
        }
        return {nullptr, nullptr, 0, 0};
    }

    Header getUrl() {
        if (headers->key) {
            return *headers;
        }
        return {nullptr, nullptr, 0, 0};
    }

    HTTPVerb getVerb() {
        if (!headers->key) {
            return INVALID;
        }
        switch (headers->keyLength) {
        case 3:
            if (!strncmp(headers->key, "get", 3)) {
                return GET;
            } else if (!strncmp(headers->key, "put", 3)) {
                return PUT;
            }
            break;
        case 4:
            if (!strncmp(headers->key, "post", 4)) {
                return POST;
            }
        case 5:
            if (!strncmp(headers->key, "patch", 5)) {
                return PATCH;
            }
            break;
        case 6:
            if (!strncmp(headers->key, "delete", 6)) {
                return DELETE;
            }
            break;
        case 7:
            if (!strncmp(headers->key, "options", 7)) {
                return OPTIONS;
            }
            break;
        }
        return INVALID;
    }

    unsigned int getRequestId() {
        return requestId;
    }
};

template <const bool isServer>
struct WIN32_EXPORT HttpSocket : private uS::Socket {
    struct Data : uS::SocketData {
        std::string httpBuffer;
        size_t contentLength = 0;

        // todo: limit these to only client, but who cares for now?
        std::string path;
        std::string host;
        void *httpUser;

        // used to close sockets not sending requests in time
        bool missedDeadline = false;
        bool awaitsResponse = false;

        // used to order responses
        unsigned int currentBlockingRequest = 0;
        unsigned int nextRequestId = 0;

        Data(uS::SocketData *socketData) : uS::SocketData(*socketData) {}
    };

    using uS::Socket::getUserData;
    using uS::Socket::setUserData;
    using uS::Socket::getAddress;
    using uS::Socket::Address;

    uv_poll_t *getPollHandle() const {return p;}

    // use Node.js-like interface here!
    void end(unsigned int requestId, char *data = nullptr, size_t length = 0);
    // void write()

    // respond is a full HTTP response with a call to timeOut
    void respond(char *message, size_t length, ContentType contentType, void(*callback)(void *httpSocket, void *data, bool cancelled, void *reserved) = nullptr, void *callbackData = nullptr);

    // send is a raw TCP send where you can send any set of headers
    void send(char *message, size_t length, void(*callback)(void *httpSocket, void *data, bool cancelled, void *reserved) = nullptr, void *callbackData = nullptr);

    // a socket that is timing out will close if not getting a new request in a while
    bool isTimingOut() {
        return !getData()->awaitsResponse;
    }

    // put socket in timeOut state
    void timeOut() {
        getData()->awaitsResponse = false;
    }

    using uS::Socket::shutdown;
    using uS::Socket::close;

    void terminate() {
        onEnd(*this);
    }

    HttpSocket(uS::Socket s) : uS::Socket(s) {}

    typename HttpSocket::Data *getData() {
        return (HttpSocket::Data *) getSocketData();
    }

    bool upgrade(const char *secKey, const char *extensions,
                 size_t extensionsLength, const char *subprotocol,
                 size_t subprotocolLength, bool *perMessageDeflate);

private:
    friend class uS::Socket;
    friend struct Hub;
    static void onData(uS::Socket s, char *data, int length);
    static void onEnd(uS::Socket s);
};

}

#endif // HTTPSOCKET_UWS_H
