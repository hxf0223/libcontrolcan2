#pragma once

#include <boost/next_prior.hpp>
#include <chrono>
#include <cstdint>
#include <stdint.h>

#include "ticks/cycle.h"

namespace tick
{

using namespace std::chrono;

inline uint64_t my_get_tick()
{
#if _MSC_VER >= 1200
	auto li = ::getticks();
	return (uint64_t)(li.QuadPart);
#else
	return ::getticks();
#endif
}

struct tickUpdate
{
	enum class stage
	{
		init,
		tick0,
		tick1,
		finish
	};
	uint64_t tick0_{0}, tick1_{0};
	uint64_t tm0_{0}, tm1_{0};
	double delta_ratio_{0.0};
	stage stat_{stage::init};

public:
	bool update()
	{
		if(stat_ == stage::init)
		{
			auto dur = system_clock::now().time_since_epoch();
			tm1_ = tm0_ = duration_cast<microseconds>(dur).count();
			tick1_ = tick0_ = my_get_tick();
			stat_ = stage::tick0;
			return false;
		}

		if(stat_ == stage::tick0)
		{
			stat_ = stage::tick1;
			return false;
		}

		const uint64_t now_tick = my_get_tick();
		if(stat_ == stage::tick1 && is_ellapsed(now_tick))
		{
			auto dur = system_clock::now().time_since_epoch();
			tm1_ = duration_cast<microseconds>(dur).count();
			tick1_ = my_get_tick();

			double temp = tm1_ - tm0_;
			delta_ratio_ = temp / (tick1_ - tick0_);
			stat_ = stage::finish;
			return true;
		}

		return false;
	}

	void reset_state()
	{
		stat_ = stage::tick0;
		tick1_ = tick0_;
		tm0_ = tm1_;
	}

private:
	bool is_ellapsed(const uint64_t& nowTick) const
	{
		if(delta_ratio_ > 0.0)
		{
			auto now_tm = nowTick * delta_ratio_;
			return ((now_tm - tm0_) > 1000); // longer than 1 second
		}

		return ((nowTick - tick0_) > 1000 * 1000 * 100);
	}
};

struct tickExt
{
private:
	uint64_t mile_stone_tick_{0}, mile_stone_tm64_{0};
	double delta_ratio_{0.0};
	bool valid_tick_{false};
	tickUpdate tu_;

public:
	tickExt()
	{
		beginInitTick();
	}

	uint64_t getTick() const
	{
		if(!valid_tick_)
			return 0;

		uint64_t tick_diff = my_get_tick() - mile_stone_tick_;
		return (mile_stone_tm64_ + tick_diff * delta_ratio_);
	}

	void updateTick()
	{
		if(tu_.update())
		{
			mile_stone_tick_ = tu_.tick0_;
			mile_stone_tm64_ = tu_.tm0_;
			tu_.reset_state();
		}
	}

	void beginInitTick()
	{
		auto dur = system_clock::now().time_since_epoch();
		mile_stone_tm64_ = duration_cast<microseconds>(dur).count();
		mile_stone_tick_ = my_get_tick();
	}

	int endInitTick()
	{
		auto dur = system_clock::now().time_since_epoch();
		uint64_t tm64 = duration_cast<microseconds>(dur).count();
		uint64_t tick2 = my_get_tick();

		if(tm64 < mile_stone_tm64_ || tick2 < mile_stone_tick_)
		{
			valid_tick_ = false;
			return -1;
		}

		if((tm64 - mile_stone_tm64_) < 10 || (tick2 - mile_stone_tick_) < 1000)
		{
			valid_tick_ = false;
			return false;
		}

		double temp = tm64 - mile_stone_tm64_;
		delta_ratio_ = temp / (tick2 - mile_stone_tick_);
		valid_tick_ = true;

		return 0;
	}
};

} // namespace tick