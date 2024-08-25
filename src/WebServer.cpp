#include <Arduino.h>
#include <Log.h>
#include <WiFi.h>
#include <queue>
#include <list>

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
        std::list<WiFiClient> clients;
        WiFiServer server;
        int timeout;
        Status status;
        int maxClients;
        WebServer(int port, int maxClientsConfig) {
            server = WiFiServer(port);
            status = Status::CREATED;
            maxClients = maxClientsConfig;
        }

        void loop() {
            switch (status)
            {
                case FAILED: break;
                case READY:
                case CONNECTED:
                    auto iterator = clients.begin();
                    while (iterator != clients.end()) {
                        auto client = iterator;
                        iterator++;
                        if (!handleClient(*client)) {
                            debugln("Client disconnected!");
                            (*client).stop();
                            clients.erase(client);
                            if (clients.size() == 0) {
                                status = Status::READY;
                            }
                        }
                    }
                    break;
            }
            if (status != Status::FAILED && server.hasClient()) {
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
            if (!server) status = Status::FAILED;
            hearthbeat();
        }

        bool send(WiFiClient client, String command) {
            timer.startNewTimer("Sending data");
            debugln("Sending: " + command);
            client.write(command.c_str());
            client.write((char)0x0);
            client.flush();
            timer.stopAndLog("Sending data");
            return true;
        }

        bool sendAll(String command) {
            auto iterator = clients.begin();
            bool returnValue = true;
            while (iterator != clients.end()) {
                returnValue &= send(*iterator, command);
                iterator++;
            }
            return returnValue;
        }

        void begin() {
            server.begin();
            if (!server) status = Status::FAILED;
            else status = Status::READY;
            debugln("WebServer " + ToString(status));
        }

        void commandRetrived(WiFiClient sender, String command);
    private:
        Timer timer = Timer();
        int counter = 0;
        void hearthbeat() {
            if (status != Status::FAILED && (counter++ % 100000) == 0) {
                debug(".");
            }
        }

        bool handleClient(WiFiClient client) {
            timer.startNewTimer("Server handle client");
            if (!client.connected()) {
                return false;
            }
            if (client.available()) {
                String command = client.readStringUntil(0x00);
                debugln("Retrived: " + command);
                commandRetrived(client, command);
            }
            timer.stopAndLog("Server handle client");
            return true;
        }
};
