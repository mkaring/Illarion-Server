//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#ifndef MAIN_HELP_HPP
#define MAIN_HELP_HPP

#include <atomic>
#include <string>
#include <vector>

// break out of the main loop if false
extern std::atomic_bool running;

class Player;

void logout_save(Player *who, bool forced, unsigned long int thistime);
void login_save(Player *who);

// process commandline arguments
auto checkArguments(const std::vector<std::string> &args) -> bool;

// load item definitions
void loadData();

void init_sighandlers();
void reset_sighandlers();

#endif
