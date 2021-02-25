/*
 * Authored by Alex Hultman, 2018-2020.
 * Intellectual property of third-party.

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 *     http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UWS_WEBSOCKETDATA_H
#define UWS_WEBSOCKETDATA_H

#include "WebSocketProtocol.h"
#include "AsyncSocketData.h"
#include "PerMessageDeflate.h"
#include "TopicTree.h"

#include <string>

namespace uWS {

struct WebSocketData : AsyncSocketData<false>, WebSocketState<true> {
    /* This guy has a lot of friends - why? */
    template <bool, bool> friend struct WebSocketContext;
    template <bool> friend struct WebSocketContextData;
    template <bool, bool> friend struct WebSocket;
    template <bool> friend struct HttpContext;
private:
    std::string fragmentBuffer;
    unsigned int controlTipLength = 0;
    bool isShuttingDown = 0;
    enum CompressionStatus : char {
        DISABLED,
        ENABLED,
        COMPRESSED_FRAME
    } compressionStatus;

    /* We might have a dedicated compressor */
    DeflationStream *deflationStream = nullptr;

    /* We could be a subscriber */
    Subscriber *subscriber = nullptr;
public:
    WebSocketData(bool perMessageDeflate, CompressOptions compressOptions, std::string &&backpressure) : AsyncSocketData<false>(std::move(backpressure)), WebSocketState<true>() {
        compressionStatus = perMessageDeflate ? ENABLED : DISABLED;

        /* Initialize the dedicated sliding window */
        if (perMessageDeflate && (compressOptions != CompressOptions::SHARED_COMPRESSOR)) {
            deflationStream = new DeflationStream(compressOptions);
        }
    }

    ~WebSocketData() {
        if (deflationStream) {
            delete deflationStream;
        }

        if (subscriber) {
            delete subscriber;
        }
    }
};

}

#endif // UWS_WEBSOCKETDATA_H
