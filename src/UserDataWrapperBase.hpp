//
//  UserDataWrapperBase.hpp
//  integral
//
//  Copyright (C) 2013, 2014, 2016  André Pereira Henriques
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

#ifndef integral_UserDataWrapperBase_hpp
#define integral_UserDataWrapperBase_hpp

namespace integral {
    namespace detail {
        class UserDataWrapperBase {
        public:
            // non-copyable
            UserDataWrapperBase(const UserDataWrapperBase &) = delete;
            UserDataWrapperBase & operator=(const UserDataWrapperBase &) = delete;

            inline virtual ~UserDataWrapperBase() = 0;

        protected:
            UserDataWrapperBase() = default;

        };

        //--

        inline UserDataWrapperBase::~UserDataWrapperBase() {}
    }
}

#endif
