#include <Arduino.h>
#include <Log.h>
#include <WiFi.h>
#include <queue>

enum Status {
    CREATED,
    READY,
    CONNECTED,
    FAILED
};

inline const String ToString(Status v) {
    switch (v)
    {
        case READY: return "Ready";
        case CONNECTED: return "Connected";
        case FAILED: return "Failed";
        case CREATED: return "Created";
        default: return "UNKNOWN Status!";
    }
}

class WebServer {
    public:
        WiFiClient client;
        WiFiServer server;
        int timeout;
        Status status;
        std::queue<String> commandQueue;
        WebServer(int port) {
            server = WiFiServer(port);
            status = Status::CREATED;
        }

        void loop() {
            switch(status) {
                case FAILED: break;
                case READY:
                    if (server.hasClient()) {
                        debugln("Server has client");
                        client = server.accept();
                        if (client) {
                            debugln("New connection from " + String(client.remoteIP().toString()) + ":" + String(client.remotePort()));
                            status = Status::CONNECTED;
                        }
                    }
                    break;
                case CONNECTED:
                    handleClient();
                    break;
                default: break;
            }
            if (!server) status = Status::FAILED;
            hearthbeat();
        }

        bool send(String command, bool addToQueIfNotAvailable = true) {
            if (status == Status::READY) {
                if (addToQueIfNotAvailable) {
                    debugln("Saving command: " + command);
                    commandQueue.push(command);
                }
                return false;
            }
            debugln("Sending: " + command);
            client.write(command.c_str());
            client.write((char)0x0);
            return true;
        }

        void begin() {
            server.begin();
            if (!server) status = Status::FAILED;
            else status = Status::READY;
            debugln("WebServer " + ToString(status));
        }

        void commandRetrived(String command);
    private:
        int counter = 0;
        void hearthbeat() {
            if (status != Status::FAILED && (counter++ % 100000) == 0) {
                debug(".");
            }
        }
        
        void sendQueue() {
            do {
                if (send(commandQueue.front(), false)) commandQueue.pop();
            } while (!commandQueue.empty());
        }

        void handleClient() {
            if (!client.connected()) {
                debugln("Client disconnected!");
                status = Status::READY;
                client.stop();
                return;
            }
            if (client.available()) {
                String command = client.readStringUntil(0x00);
                debugln("Retrived: " + command);
                commandRetrived(command);
                if(!commandQueue.empty()) {
                    debugln("Queue length: " + String(commandQueue.size()));
                    sendQueue();
                }
            }
        }
};
