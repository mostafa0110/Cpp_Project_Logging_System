#pragma once

#include <vsomeip/vsomeip.hpp>
#include <memory>
#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <chrono>
#include <cstring>

class SomeIPTelemetrySourceImpl {
public:
    // No copy/move operations (Singleton)
    SomeIPTelemetrySourceImpl(const SomeIPTelemetrySourceImpl&) = delete;
    SomeIPTelemetrySourceImpl& operator=(const SomeIPTelemetrySourceImpl&) = delete;
    SomeIPTelemetrySourceImpl(SomeIPTelemetrySourceImpl&&) = delete;
    SomeIPTelemetrySourceImpl& operator=(SomeIPTelemetrySourceImpl&&) = delete;

    static SomeIPTelemetrySourceImpl& getInstance() {
        static SomeIPTelemetrySourceImpl instance;
        return instance;
    }

    bool init() {
        if (isRunning_) {
            return true;
        }

        // Initialize vsomeip application
        if (!app_->init()) {
            return false;
        }

        // Register state handler
        app_->register_state_handler(
            std::bind(&SomeIPTelemetrySourceImpl::onState, this, std::placeholders::_1));

        // Register availability handler
        app_->register_availability_handler(
            SERVICE_ID, INSTANCE_ID,
            std::bind(&SomeIPTelemetrySourceImpl::onAvailability, this,
                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        // Register message handler for responses
        app_->register_message_handler(
            SERVICE_ID, INSTANCE_ID, METHOD_ID,
            std::bind(&SomeIPTelemetrySourceImpl::onMessage, this, std::placeholders::_1));

        // Start application in separate thread
        isRunning_ = true;
        runnerThread_ = std::thread([this]() { app_->start(); });

        return true;
    }

    void shutdown() {
        if (!isRunning_) {
            return;
        }

        isRunning_ = false;
        isAvailable_ = false;

        // Release service request
        app_->release_service(SERVICE_ID, INSTANCE_ID);

        // Unregister handlers
        app_->unregister_message_handler(SERVICE_ID, INSTANCE_ID, METHOD_ID);
        app_->unregister_availability_handler(SERVICE_ID, INSTANCE_ID);
        app_->unregister_state_handler();

        // Stop application
        app_->stop();

        // Join runner thread
        if (runnerThread_.joinable()) {
            runnerThread_.join();
        }
    }

    bool requestLoadData(float& loadPercentage) {
        if (!isAvailable_) {
            return false;
        }

        std::unique_lock<std::mutex> lock(responseMutex_);
        responseReceived_ = false;

        // Create and send request message
        auto request = vsomeip::runtime::get()->create_request();
        request->set_service(SERVICE_ID);
        request->set_instance(INSTANCE_ID);
        request->set_method(METHOD_ID);
        request->set_interface_version(MAJOR_VERSION);

        app_->send(request);

        // Wait for response with timeout
        if (responseCV_.wait_for(lock, std::chrono::seconds(5), 
                [this]() { return responseReceived_; })) {
            loadPercentage = lastLoadValue_;
            return true;
        }
        
        return false;
    }

    bool isAvailable() const {
        return isAvailable_;
    }

private:
    SomeIPTelemetrySourceImpl() 
        : app_(vsomeip::runtime::get()->create_application("TelemetryClient")) {
    }
    
    ~SomeIPTelemetrySourceImpl() {
        shutdown();
    }

    void onState(vsomeip::state_type_e state) {
        if (state == vsomeip::state_type_e::ST_REGISTERED) {
            // Request service availability
            app_->request_service(SERVICE_ID, INSTANCE_ID, MAJOR_VERSION, MINOR_VERSION);
        }
    }

    void onAvailability(vsomeip::service_t service, 
                        vsomeip::instance_t instance, 
                        bool available) {
        if (service == SERVICE_ID && instance == INSTANCE_ID) {
            isAvailable_ = available;
        }
    }

    void onMessage(const std::shared_ptr<vsomeip::message>& response) {
        if (response->get_service() != SERVICE_ID || 
            response->get_method() != METHOD_ID) {
            return;
        }

        // Check for successful response
        if (response->get_return_code() != vsomeip::return_code_e::E_OK) {
            return;
        }

        // Parse response payload
        auto payload = response->get_payload();
        if (payload && payload->get_length() >= sizeof(float)) {
            // Deserialize float from payload
            std::memcpy(&lastLoadValue_, payload->get_data(), sizeof(float));
        }

        std::lock_guard<std::mutex> lock(responseMutex_);
        responseReceived_ = true;
        responseCV_.notify_one();
    }

    // vsomeip application
    std::shared_ptr<vsomeip::application> app_;
    std::thread runnerThread_;
    
    // State tracking
    std::atomic<bool> isAvailable_{false};
    std::atomic<bool> isRunning_{false};
    
    // Response synchronization
    std::mutex responseMutex_;
    std::condition_variable responseCV_;
    bool responseReceived_{false};
    float lastLoadValue_{0.0f};

    // Service IDs - configurable via vsomeip.json
    static constexpr vsomeip::service_t SERVICE_ID = 0x1234;
    static constexpr vsomeip::instance_t INSTANCE_ID = 0x5678;
    static constexpr vsomeip::method_t METHOD_ID = 0x0001;
    static constexpr vsomeip::major_version_t MAJOR_VERSION = 1;
    static constexpr vsomeip::minor_version_t MINOR_VERSION = 0;
};
