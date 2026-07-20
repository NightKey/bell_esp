#include <Arduino.h>
#include <Log.h>
#include <WiFi.h>
#include <list>

enum Status {
    CREATED,
    READY,
    CONNECTED,
    FAILED
};

inline String toString(const Status v) {
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
        std::list<WiFiClient> clients;
        WiFiServer server;
        Status status;
        int maxClients;
        WebServer(const int port, const int maxClientsConfig) {
            server = WiFiServer(port);
            status = Status::CREATED;
            maxClients = maxClientsConfig;
        }

        int loop() {
            if (!server) status = Status::FAILED;
            switch (status)
            {
                case FAILED:
                    return 1;
                    break;
                case CREATED:
                    debugln("WebServer not yet started");
                    break;
                case READY:
                case CONNECTED:
                    if (server.hasClient()) {
                        debugln("Server has client");
                        WiFiClient newClient = server.accept();
                        if (clients.size() >= maxClients) {
                            debugln("Client refused due to having max clients already!");
                            newClient.stop();
                        } else if (newClient) {
                            debugln("New connection from " + String(newClient.remoteIP().toString()) + ":" + String(newClient.remotePort()));
                            clients.push_back(newClient);
                            status = Status::CONNECTED;
                        }
                    }
                    auto iterator = clients.begin();
                    while (iterator != clients.end()) {
                        auto client = iterator;
                        ++iterator;
                        if (!handleClient(*client)) {
                            debugln("Client disconnected!");
                            client->stop();
                            clients.erase(client);
                            if (clients.empty()) {
                                status = Status::READY;
                            }
                        }
                    }
                    break;
            }
            return 0;
        }

        bool send(WiFiClient& client, const String& command) {
            timer.startNewTimer("Sending data");
            debugln("Sending: " + command);
            client.write(command.c_str());
            client.write(static_cast<char>(0x0));
            client.flush();
            timer.stopAndLog("Sending data");
            return true;
        }

        bool sendAll(const String& command) {
            auto iterator = clients.begin();
            bool returnValue = true;
            while (iterator != clients.end()) {
                returnValue &= send(*iterator, command);
                ++iterator;
            }
            return returnValue;
        }

        bool isHealthy() const {
            return status == Status::READY || status == Status::CONNECTED;
        }

        bool begin() {
            server.begin();
            if (!server) status = Status::FAILED;
            else status = Status::READY;
            debugln("WebServer " + toString(status));
            return status == Status::READY;
        }

        void commandRetrieved(WiFiClient &sender, const String &command);
    private:
        Timer timer = Timer();

        bool handleClient(WiFiClient& client) {
            debugln("Handling client " + client.remoteIP().toString());
            timer.startNewTimer("Server handle client");
            if (!client.connected()) {
                debugln("Client disconnected!");
                return false;
            }
            if (client.available()) {
                const String command = client.readStringUntil(0x00);
                debugln("Retrieved: " + command);
                commandRetrieved(client, command);
            }
            debugln("Client handle finished");
            timer.stopAndLog("Server handle client");
            return true;
        }
};
