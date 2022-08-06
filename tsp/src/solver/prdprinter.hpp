#pragma once

#include "config.hpp"
#include <cstdint>

// TODO thread printujacy zeby nie zwalniac liczacego
class prd_printer {
    inline static prd_printer* s_instance = nullptr;

public:
    static prd_printer* instance()
    {
        return s_instance;
    }

    static void start(config::value_type const& val)
    {
        if (s_instance == nullptr) {
            s_instance = new prd_printer(val);
        }
    }

    static void stop()
    {
        if (s_instance != nullptr) {
            delete s_instance;
        }
    }

private:
    config::value_type prd_ {};

public:
    prd_printer(config::value_type const& v)
        : prd_ { v }
    {
    }

    ~prd_printer()
    {
    }

    void print(uint64_t iteration, config::value_type value);
};