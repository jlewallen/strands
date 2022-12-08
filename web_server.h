#pragma once

class WebServer {
private:
    bool have_time_{ false };

public:
    WebServer();

public:
    bool have_time() const {
        return have_time_;
    }

public:
    void begin();
    void service();
};