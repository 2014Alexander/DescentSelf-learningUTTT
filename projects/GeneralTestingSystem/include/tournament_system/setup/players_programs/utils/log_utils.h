// log_utils.h
#pragma once
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>

namespace log_utils {
    enum class Level { Error = 0, Warning, Info, Debug };

    inline Level current_level = Level::Info; // меняйте в runtime при необходимости

    namespace detail {
        // thread‑safe вывод времени вида HH:MM:SS
        inline void printTimestamp(std::ostream &os) {
            using clock = std::chrono::system_clock;
            auto tt = clock::to_time_t(clock::now());

            std::tm tm;
#ifdef _WIN32
            localtime_s(&tm, &tt);
#else
            localtime_r(&tt, &tm);
#endif
            os << std::put_time(&tm, "%H:%M:%S");
        }

        inline std::mutex &logMutex() {
            static std::mutex m;
            return m;
        }

        template<Level L>
        inline void log(const std::string &msg) {
            if (static_cast<int>(L) > static_cast<int>(current_level))
                return;

            std::lock_guard<std::mutex> lk(logMutex());
            printTimestamp(std::cerr);
            switch (L) {
                case Level::Error: std::cerr << " [ERROR] ";
                    break;
                case Level::Warning: std::cerr << " [WARNING] ";
                    break;
                case Level::Info: std::cerr << " [INFO] ";
                    break;
                case Level::Debug: std::cerr << " [DEBUG] ";
                    break;
            }
            std::cerr << msg << '\n';
        }
    } // namespace detail

    inline void set_level(Level l) { current_level = l; }

    inline void error(const std::string &m) {
        detail::log<Level::Error>(m);
        system("pause");
    }

    inline void warning(const std::string &m) {
        detail::log<Level::Warning>(m);
        system("pause");
    }

    inline void info(const std::string &m) { detail::log<Level::Info>(m); }
    inline void debug(const std::string &m) { detail::log<Level::Debug>(m); }

    [[noreturn]] inline void fatal(const std::string &m) {
        // Выводим сразу без уровня — это всегда фатальная ошибка:
        std::cerr << "[FATAL] " << m << '\n'
                << "Press Enter to exit...";
        // Ждём, чтобы пользователь успел прочитать
        std::cin.get();
        std::exit(1);
    }
} // namespace log_utils
