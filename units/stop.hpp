void stop(int) { stopRequested.store(true, std::memory_order_relaxed); }
