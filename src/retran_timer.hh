#include "tcp_config.hh"
#include "tcp_sender.hh"
#include <cstddef>
#include <sys/types.h>

class ReTimer {
private:
    uint64_t timeout_{TCPConfig::TIMEOUT_DFLT}; 
    uint64_t initial_RTO_ms_{TCPConfig::TIMEOUT_DFLT};
    bool running_{false};      // 定时器是否在运行
    size_t time_ms_{0};
public:
    ReTimer() = default;
    explicit ReTimer(uint64_t timeout) : timeout_(timeout), initial_RTO_ms_(timeout){}

    // 定时器开始
    void start(){
        running_ = true;
        time_ms_ = 0;
    }

    // 定时器运行
    void tick (const size_t ms_since_last_tick) {
        if (running_) {
            time_ms_ += ms_since_last_tick;
        }
    }
    // 运行
    bool is_running() const {   return running_;    }
    // 超时
    bool is_timeout() const {   return running_ && (time_ms_ >= timeout_);  }
    // 指数退避
    void RTO_double() {timeout_ *= 2;}
    // 复原
    void RTO_reset()  {timeout_ = initial_RTO_ms_;}
    // 定时器运行结束
    void stop() {   running_ = false;   }
};