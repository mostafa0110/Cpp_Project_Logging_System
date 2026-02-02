#pragma once

#include <vsomeip/vsomeip.hpp>
#include <memory>
#include <thread>
#include <atomic>
#include <random>
#include <cstring>
#include <vector>

class SomeIPTestServer {
public:
    SomeIPTestServer() 
        : app_(vsomeip::runtime::get()->create_application("TelemetryServer")) {
    }
    
    ~SomeIPTestServer() {
        stop();
    }

    // Non-copyable, non-movable
    SomeIPTestServer(const SomeIPTestServer&) = delete;
    SomeIPTestServer& operator=(const SomeIPTestServer&) = delete;
    SomeIPTestServer(SomeIPTestServer&&) = delete;
    SomeIPTestServer& operator=(SomeIPTestServer&&) = delete;

    bool start() {
        if (isRunning_) {
            return true;
        }

        // Initialize vsomeip application
        if (!app_->init()) {
            return false;
        }

        // Register state handler
        app_->register_state_handler(
            std::bind(&SomeIPTestServer::onState, this, std::placeholders::_1));

        // Register message handler for METHOD_ID
        app_->register_message_handler(
            SERVICE_ID, INSTANCE_ID, METHOD_ID,
            std::bind(&SomeIPTestServer::onMessage, this, std::placeholders::_1));

        // Start in separate thread
        isRunning_ = true;
        runnerThread_ = std::thread(&SomeIPTestServer::run, this);
        
        return true;
    }

    void stop() {
        if (!isRunning_) {
            return;
        }
        
        isRunning_ = false;
        
        // Stop offering service
        app_->stop_offer_service(SERVICE_ID, INSTANCE_ID);

        // Unregister handlers
        app_->unregister_message_handler(SERVICE_ID, INSTANCE_ID, METHOD_ID);
        app_->unregister_state_handler();

        // Stop application
        app_->stop();
        
        if (runnerThread_.joinable()) {
            runnerThread_.join();
        }
    }

    bool isRunning() const { return isRunning_; }

    void setFixedLoad(float load) { 
        useFixedLoad_ = true; 
        fixedLoad_ = load; 
    }

    void useRandomLoad() { useFixedLoad_ = false; }

private:
    void run() {
        app_->start();
    }

    void onState(vsomeip::state_type_e state) {
        if (state == vsomeip::state_type_e::ST_REGISTERED) {
            // Offer service
            app_->offer_service(SERVICE_ID, INSTANCE_ID, MAJOR_VERSION, MINOR_VERSION);
        }
    }

    void onMessage(const std::shared_ptr<vsomeip::message>& request) {
        if (request->get_service() != SERVICE_ID || 
            request->get_method() != METHOD_ID) {
            return;
        }

        // Generate load value
        float loadValue = useFixedLoad_ ? fixedLoad_ : loadDist_(rng_);
        
        // Create response
        auto response = vsomeip::runtime::get()->create_response(request);
        auto payload = vsomeip::runtime::get()->create_payload();
        
        // Serialize load value into payload
        std::vector<vsomeip::byte_t> data(sizeof(float));
        std::memcpy(data.data(), &loadValue, sizeof(float));
        payload->set_data(data);
        response->set_payload(payload);
        
        // Send response
        app_->send(response);
    }

    std::shared_ptr<vsomeip::application> app_;
    std::thread runnerThread_;
    std::atomic<bool> isRunning_{false};
    
    // Load generation
    bool useFixedLoad_{false};
    float fixedLoad_{50.0f};
    std::mt19937 rng_{std::random_device{}()};
    std::uniform_real_distribution<float> loadDist_{0.0f, 100.0f};

    // Must match client IDs
    static constexpr vsomeip::service_t SERVICE_ID = 0x1234;
    static constexpr vsomeip::instance_t INSTANCE_ID = 0x5678;
    static constexpr vsomeip::method_t METHOD_ID = 0x0001;
    static constexpr vsomeip::major_version_t MAJOR_VERSION = 1;
    static constexpr vsomeip::minor_version_t MINOR_VERSION = 0;
};
