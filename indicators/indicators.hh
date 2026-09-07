#pragma once

#include <indicators/block_progress_bar.hpp>
#include <indicators/color.hpp>
#include <indicators/cursor_control.hpp>
#include <indicators/cursor_movement.hpp>
#include <indicators/display_width.hpp>
#include <indicators/dynamic_progress.hpp>
#include <indicators/font_style.hpp>
#include <indicators/indeterminate_progress_bar.hpp>
#include <indicators/multi_progress.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/progress_spinner.hpp>
#include <indicators/progress_type.hpp>
#include <indicators/setting.hpp>
#include <indicators/termcolor.hpp>
#include <indicators/terminal_size.hpp>

#include <cstdlib>

namespace indicators {
inline std::atomic<std::size_t> n_col_curr {};

inline Color next_col() noexcept {
	switch(n_col_curr++ % 6) {
		case 0:  return Color::red;
		case 1:  return Color::green;
		case 2:  return Color::yellow;
		case 3:  return Color::blue;
		case 4:  return Color::magenta;
		case 5:  return Color::cyan;
		default: __builtin_unreachable();
	}
}
inline Color rand_col() noexcept {
	int x = rand();
	switch(x % 6) {
		case 0:  return Color::red;
		case 1:  return Color::green;
		case 2:  return Color::yellow;
		case 3:  return Color::blue;
		case 4:  return Color::magenta;
		case 5:  return Color::cyan;
		default: __builtin_unreachable();
	}
}

}
