//
//  UnexpectedStackException.cpp
//  integral
//
//  Copyright (C) 2016  André Pereira Henriques
//  aphenriques (at) outlook (dot) com
//
//  This file is part of integral.
//
//  integral is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  integral is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with integral.  If not, see <http://www.gnu.org/licenses/>.
//

#include "UnexpectedStackException.hpp"
#include "utility.hpp"

namespace integral {
    // the leading "::" in "::exception::LogicException" is necessary to avoid ambiguity between "exception" namespace and "std::exception" base class
    UnexpectedStackException::UnexpectedStackException(lua_State *luaState, const std::string &fileName, int lineNumber, const std::string &functionName, const std::string &errorMessage) : ::exception::LogicException(fileName, lineNumber, functionName, std::string("[integral] invalid Lua stack: ") + errorMessage + ". lua stack: { " + utility::getStackString(luaState) + " }") {}
    
    // the leading "::" in "::exception::LogicException" is necessary to avoid ambiguity between "exception" namespace and "std::exception" base class
    UnexpectedStackException::UnexpectedStackException(lua_State *luaState, const std::string &fileName, int lineNumber, const std::string &functionName) : ::exception::LogicException(fileName, lineNumber, functionName, std::string("[integral] invalid Lua stack. lua stack: { ") + utility::getStackString(luaState) + " }") {}
    
    // If a class is defined in a header file and has a vtable (either it has virtual methods or it derives from classes with virtual methods), it must always have at least one out-of-line virtual method in the class. Without this, the compiler will copy the vtable and RTTI into every .o file that #includes the header, bloating .o file sizes and increasing link times.
    // source: http://llvm.org/docs/CodingStandards.html#provide-a-virtual-method-anchor-for-classes-in-headers
    UnexpectedStackException::~UnexpectedStackException() {}

}