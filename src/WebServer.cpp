#include <Arduino.h>
#include <Log.h>
#include <WiFi.h>
#include <queue>

enum Status {
    READY,
    CONNECTED
};

class WebServer {
    public:
        WiFiClient client;
        WiFiServer server;
        int timeout;
        Status status;
        std::queue<String> commandQueue;
        WebServer() {  }
        WebServer(int port) {
            server.begin(port);
            status = Status::READY;
        }

        void loop() {
            if (status == Status::READY) {
                client = server.available();
                if (client) {
                    debugln("New connection");
                    status = Status::CONNECTED;
                } else return;
            }
            if (!client.connected()) {
                debugln("Client disconnected!");
                status = Status::READY;
                return;
            }
            if (client.available()) {
                String command = client.readStringUntil(0x00);
                debugln("Retrived: " + command);
                commandRetrived(command);
            }
            if(!commandQueue.empty()) {
                debugln("Queue length: " + commandQueue.size());
                sendQueue();
            }
        }

        void send(String command) {
            if (status == Status::READY || !client.availableForWrite()) {
                commandQueue.push(command);
                return;
            }
            debugln("Sending: " + command);
            client.write(command.c_str());
            client.write((byte)0x00);
        }

        void commandRetrived(String command);
    private:
        void sendQueue() {
            do {
                send(commandQueue.front());
                commandQueue.pop();
            } while (!commandQueue.empty());
        }
};
