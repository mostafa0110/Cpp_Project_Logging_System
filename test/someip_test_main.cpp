#include "SomeIPTestServer.hpp"
#include <iostream>
#include <csignal>
#include <atomic>

std::atomic<bool> g_running{true};

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down..." << std::endl;
    g_running = false;
}

int main() {
    // Register signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "=== SomeIP Telemetry Test Server ===" << std::endl;
    std::cout << "Service ID: 0x1234" << std::endl;
    std::cout << "Instance ID: 0x5678" << std::endl;
    std::cout << "Method ID: 0x0001" << std::endl;
    std::cout << std::endl;

    SomeIPTestServer server;
    
    // Use fixed load value for predictable testing
    server.setFixedLoad(75.5f);
    
    std::cout << "Starting server..." << std::endl;
    
    if (!server.start()) {
        std::cerr << "Failed to start SomeIP test server!" << std::endl;
        return 1;
    }
    
    std::cout << "Server is running. Press Ctrl+C to stop." << std::endl;
    std::cout << "Responding with fixed load value: 75.5%" << std::endl;
    std::cout << std::endl;
    
    // Wait for shutdown signal
    while (g_running && server.isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Stopping server..." << std::endl;
    server.stop();
    
    std::cout << "Server stopped. Goodbye!" << std::endl;
    
    return 0;
}
