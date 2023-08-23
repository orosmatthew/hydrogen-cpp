#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <chrono>
#include <atomic>
#include <string>
#include <algorithm>
#include <sstream>


namespace utils
{
    std::string capture_stdout(char* cmd, int buffer_size) 
    {
        FILE* pipe = popen(cmd, "r");
        if (!pipe)
        {
            std::cerr << "error opening pipe\n";
            return NULL;
        }

        char buffer[buffer_size];
        std::string result = "";

        while (!feof(pipe))
        {
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) 
            {
                result += buffer;
            }
        }
        
        pclose(pipe);
        // removes newline
        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
        return result;
    }

    using timestamp = std::chrono::_V2::system_clock::time_point;
    class Timer
    {
    private:
        std::string start;
        int buffer_size = 2;
        char const* cmd = "date +%s%N | cut -b1-13";
    public:
        Timer() {
            start = capture_stdout((char*)cmd, buffer_size);
        }

        std::string stop()
        {
            std::string stop = capture_stdout((char*)cmd, buffer_size);
            long _start; long _stop;
            _start = std::stol(start);
            _stop  = std::stol(stop);
            auto duration = _stop - _start;
            double seconds = static_cast<double>(duration) / 1000.0;
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3) << seconds;
            return ss.str();
        }        
    };

    std::string get_now() 
    {
        auto now = std::chrono::system_clock::now();
        auto now_time = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        struct tm timeinfo;
        // TODO: need to add logic to automatically detect the os
        localtime_r(&now_time, &timeinfo); // Use localtime_s on shitty os, localtime_r on linux
        char buffer[50];
        strftime(buffer, sizeof(buffer), "[%Y-%m-%d|%H:%M:%S", &timeinfo);

        std::string res = buffer + std::string(".") + std::to_string(now_ms.count()) + "]";
        return res;
    }

    void log_info(std::string message) 
    {
        std::cout << get_now() << " 🔥 " << message << std::endl;
    }

    void log_success(std::string message) 
    {
        std::cout << get_now() << " ✅ " << message << std::endl;
    }

    void log_error(std::string message) 
    {
        std::cerr << get_now() << " 💥 " << message << std::endl;
    }
}

