#include "sources/SomeIPTelemetryAdapter.hpp"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<bool> g_running{true};

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down..." << std::endl;
    g_running = false;
}

int main() {
    // Register signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "=== SomeIP Telemetry Test Client ===" << std::endl;
    std::cout << "Connecting to Service ID: 0x1234, Instance: 0x5678" << std::endl;
    std::cout << std::endl;

    SomeIPTelemetryAdapter adapter;
    
    std::cout << "Initializing SomeIP client..." << std::endl;
    
    if (!adapter.openSource()) {
        std::cerr << "Failed to initialize SomeIP client!" << std::endl;
        return 1;
    }
    
    std::cout << "Client initialized. Waiting for server..." << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;
    std::cout << std::endl;
    
    // Poll for data every 2 seconds
    int requestCount = 0;
    while (g_running) {
        std::string data;
        if (adapter.readSource(data)) {
            requestCount++;
            std::cout << "[Request #" << requestCount << "] Received load: " << data << "%" << std::endl;
        } else {
            std::cout << "Waiting for server to become available..." << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    std::cout << "\nClient stopped. Total requests: " << requestCount << std::endl;
    std::cout << "Goodbye!" << std::endl;
    
    return 0;
}
