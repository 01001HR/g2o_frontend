/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef BOSS_SERIALIZATION_CONTEXT_H
#define BOSS_SERIALIZATION_CONTEXT_H

#include <string>
#include <iostream>

#include "id_context.h"
#include "blob.h"

namespace boss {

class Serializer;

class SerializationContext: public IdContext {
public:
  virtual std::string createBinaryFilePath(BaseBLOBReference& instance);
  virtual std::ostream* getBinaryOutputStream(const std::string& fname)=0;
  virtual std::istream* getBinaryInputStream(const std::string& fname)=0;
};

}

#endif // BOSS_SERIALIZATION_CONTEXT_H
