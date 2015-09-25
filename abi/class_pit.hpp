#pragma once
#include <delegate>
#include <list>


/**
   Programmable Interval Timer class. A singleton.
   
   @TODO
   ...It has timer-functionality, which should probably be super-classed, 
   so that i.e. the HPET could be used with the same interface.
*/
class PIT{     
public:
  
  typedef delegate<void()> timeout_handler;
  
  
  void onTimeout_ms(uint64_t ms, timeout_handler);
  void onTimeout_sec(uint32_t sec, timeout_handler);
  void onRepeatedTimeout_sec(uint64_t ms, timeout_handler);
  void onRepeatedTimeout_ms(uint32_t sec, timeout_handler);
  
  // No copy or move. The OS owns one instance forever.
  PIT(PIT&) = delete;
  PIT(PIT&&) = delete;
    
  // Get the (single) instance
  static PIT& instance() { 
    if (! pit_) pit_ = new PIT();
    return *pit_;
  };
  
  /** Initialize the hardware. */
  static void init();    

  /** Estimate cpu frequency based on the fixed PIT frequency and rdtsc. 
      @Note This is an asynchronous function. Once finished the result can be 
            fetched by CPUFrequency() (below)
   */
  static void estimateCPUFrequency();
  
  /** Get the last estimated CPU frequency. */
  static double CPUFrequency();
  
  
private: 
  enum Mode { ONE_SHOT = 0, 
	      HW_ONESHOT = 1 << 1, 
	      RATE_GEN = 2 << 1, 
	      SQ_WAVE = 3 << 1, 
	      SW_STROBE = 4 << 1, 
	      HW_STROBE = 5 << 1, 
	      NONE = 256};
  
  static Mode temp_mode_;
  static uint16_t temp_freq_divider_;
  static uint64_t prev_timestamp_;
  static uint16_t sampling_freq_divider_;
  
  /** Disable regular timer interrupts- which are turned on at boot-time. */
  static void disable_regular_interrupts();

  // The default handler for timer interrupts
  static void irq_handler();
  
  // The PIT-chip runs at this fixed frequency (in MHz) , according to OSDev.org
  static constexpr double freq_mhz_ = 14.31818 / 12;   // ~ 1.1931816666666666 MHz
  static constexpr double ticks_pr_sec_ = freq_mhz_ * 1'000'000;   // ~ 1.1931816666666666 MHz
  
  static PIT* pit_;
  
  PIT();
  ~PIT();

  /** A timer is a handler and a timeout. 
      The timeout represents the pre-computed rdtsc-value when we time out.
  */
  struct Timer {
    timeout_handler handler;
    uint64_t timeout;
  };
  
  // This is now extern, since a proper IRQ-handler needs to write to it.
  //static double CPUFreq_;
  
  static uint8_t status_byte_;
  static uint16_t current_freq_divider_;
  static Mode current_mode_;
  static uint64_t IRQ_counter_;
  
  /** A sorted list of timers.
      @note This is why we want to instantiate, and why it's a singleton: 
            If you don't use PIT-timers, you won't pay for them. */
  std::list<Timer> timers_;
  
  // Access mode bits are bits 4- and 5 in the Mode register
  enum AccessMode { LATCH_COUNT = 0x0, LO_ONLY=0x10, HI_ONLY=0x20, LO_HI=0x30 };
  
  static void set_mode(Mode);
  static void set_freq(uint16_t);
  static void oneshot(uint16_t t);
  
  
};

