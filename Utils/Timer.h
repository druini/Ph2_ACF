#ifndef __HWInterface_Timer_h
#define __HWInterface_Timer_h

#include <iostream>
#include <chrono>
#include <ctime>

using namespace std::chrono;
class Timer
{

  public:
    Timer() : start_(), end_() {}
    virtual ~Timer() {}
    void start()
    {
        // start_ = system_clock::now();
        std::cout<< __PRETTY_FUNCTION__ << " Disabled due to a bug!!!" << std::endl;
    }
    void stop()
    {
    // std::cout<<__PRETTY_FUNCTION__<<__LINE__<<std::endl;
    //     end_ = system_clock::now();
    // std::cout<<__PRETTY_FUNCTION__<<__LINE__<<std::endl;
        std::cout<< __PRETTY_FUNCTION__ << " Disabled due to a bug!!!" << std::endl;

    }
    void show (const std::string& label)
    {
    // std::cout<<__PRETTY_FUNCTION__<<__LINE__<<std::endl;
    //     duration<double> time_span = duration_cast<duration<double>> (end_ - start_);
    // std::cout<<__PRETTY_FUNCTION__<<__LINE__<<std::endl;
    //     std::time_t end_time = system_clock::to_time_t (end_);

    // std::cout<<__PRETTY_FUNCTION__<<__LINE__<<std::endl;
    // std::cout<<__PRETTY_FUNCTION__<<__LINE__ << " " << end_time <<std::endl;
    //     const std::string& tnow = std::ctime (&end_time);
    // std::cout<<__PRETTY_FUNCTION__<<__LINE__<<std::endl;
    //     std::cout << label
    //               << " finished at: " << tnow
    //               << "\telapsed time: " << time_span.count() << " seconds" << std::endl;
    // std::cout<<__PRETTY_FUNCTION__<<__LINE__<<std::endl;
        std::cout<< __PRETTY_FUNCTION__ << " Disabled due to a bug!!!" << std::endl;

    }
    double getElapsedTime()
    {
        // duration<double> time_span = duration_cast<duration<double>> (end_ - start_);
        // return time_span.count();
        std::cout<< __PRETTY_FUNCTION__ << " Disabled due to a bug!!!" << std::endl;
        return 0.;
    }
    double getCurrentTime()
    {
        // system_clock::time_point now_ = system_clock::now();
        // duration<double> time_span = duration_cast<duration<double>> (now_ - start_);
        // return time_span.count();
        std::cout<< __PRETTY_FUNCTION__ << " Disabled due to a bug!!!" << std::endl;
        return 0.;
    }
    void reset()
    {
        // start_ = end_;
        std::cout<< __PRETTY_FUNCTION__ << " Disabled due to a bug!!!" << std::endl;
    }

  private:
    system_clock::time_point start_, end_;
};
#endif
