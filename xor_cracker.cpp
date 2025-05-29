#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include <iomanip>
#include <atomic>
#include <sstream>
#include <mutex>

std::mutex print_mutex;
std::atomic<bool> solution_found(false);
std::string target = "373a19293e5213383c3a260a20133a174637392d221c26612f0f3e2d3a1b11";

// Convert hex string to bytes
std::vector<uint8_t> hex_to_bytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

// Convert bytes to hex string
std::string bytes_to_hex(const std::vector<uint8_t>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

std::string generate_random_key() {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);
    
    std::string key;
    key.reserve(5);
    for (int i = 0; i < 5; ++i) {
        key += charset[dis(gen)];
    }
    return key;
}

std::string xor_encrypt(const std::string& input, const std::string& key) {
    std::vector<uint8_t> result;
    result.reserve(input.length());
    
    for (size_t i = 0; i < input.length(); ++i) {
        result.push_back(input[i] ^ key[i % key.length()]);
    }
    
    return bytes_to_hex(result);
}

void crack_attempt(int thread_id, uint64_t attempts, uint64_t progress_interval) {
    const std::string flag = "THM{thisisafakeflag}";
    uint64_t counter = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    while (counter < attempts && !solution_found) {
        std::string key = generate_random_key();
        std::string result = xor_encrypt(flag, key);
        
        if (result == target) {
            solution_found = true;
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
            
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "Success!! Key found by thread " << thread_id << std::endl;
            std::cout << "Key: " << key << std::endl;
            std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;
            return;
        }
        
        if (++counter % progress_interval == 0) {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
            
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "Thread " << thread_id << ": " 
                      << counter << " attempts in " 
                      << duration.count() << " seconds ("
                      << counter / (duration.count() ? duration.count() : 1) 
                      << " attempts/sec)" << std::endl;
        }
    }
}

int main() {
    const uint64_t attempts_per_thread = 100000000;  // 100 million attempts per thread
    const uint64_t progress_interval = 1000000;      // Print progress every 1 million attempts
    const unsigned int thread_count = std::thread::hardware_concurrency() * 8; // 8x the number of CPU cores
    
    std::cout << "Starting brute force with " << thread_count << " threads..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    
    for (unsigned int i = 0; i < thread_count; ++i) {
        threads.emplace_back(crack_attempt, i, attempts_per_thread, progress_interval);
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    
    if (!solution_found) {
        std::cout << "No solution found after " << duration.count() << " seconds" << std::endl;
    }
    
    return 0;
}
