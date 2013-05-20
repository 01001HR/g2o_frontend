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


#ifndef BOSS_OBJECT_DATA_H
#define BOSS_OBJECT_DATA_H

#include <string>
#include <vector>
#include <map>
#include <limits>

namespace boss {

class Identifiable;
class IdPlaceholder;

enum ValueType {
  BOOL, NUMBER, STRING, ARRAY, OBJECT, POINTER, POINTER_REF, BLOB_REF
};

class ValueData {
public:
  virtual int getInt();
  virtual double getDouble();
  virtual float getFloat();
  virtual bool getBool();
  virtual const std::string& getString();
  virtual std::vector<ValueData*>& getArray();
  virtual std::map<std::string,ValueData*>& getMap();
  virtual Identifiable* getPointer();
  virtual void bindPointer(Identifiable*& pvar);

  virtual ValueType type()=0;
  const std::string& typeName();
  operator bool() {
    return getBool();
  }
  operator int() {
    return getInt();
  }
  operator double() {
    return getDouble();
  }
  operator float() {
    return getFloat();
  }
  operator const std::string&() {
    return getString();
  }

  virtual ~ValueData();
};


class BoolData: public ValueData {
public:
  BoolData(bool value): _value(value) {};
  virtual bool getBool();
  virtual int getInt();
  virtual ValueType type();

protected:
  bool _value;
};

class NumberData: public ValueData {
public:
  NumberData(double value=0.0, int precision=std::numeric_limits<double>::digits10):
    _value(value),
    _precision(precision) {}
  NumberData(float value=0.0, int precision=std::numeric_limits<float>::digits10):
    _value(value),
    _precision(precision) {}
  NumberData(int value=0, int precision=std::numeric_limits<double>::digits10):
    _value(value),
    _precision(precision) {}
  virtual int getInt();
  virtual double getDouble();
  virtual float getFloat();
  virtual bool getBool();
  virtual ValueType type();

  int precision() {
    return _precision;
  }
protected:
  double _value;
  int _precision;
};

class StringData: public ValueData {
public:
  StringData(const std::string& value): _value(value) {}
  virtual const std::string& getString();
  virtual ValueType type();

protected:
  std::string _value;
};

class ArrayData: public ValueData {
public:
  virtual std::vector<ValueData*>& getArray();
  virtual ValueType type();
  virtual ~ArrayData();
  void add(bool value);
  void add(int value);
  void add(double value);
  void add(float value);
  void add(const std::string& value);
  void add(const char* value);
  void add(ValueData* value);
  void addPointer(Identifiable* ptr);

protected:
  std::vector<ValueData*> _value;
};

class ObjectData: public ValueData {
public:
  virtual std::map<std::string,ValueData*>& getMap();
  virtual ValueType type();
  void setField(const std::string& name, ValueData* value);
  void setInt(const std::string& name, int value);
  void setDouble(const std::string& name, double value);
  void setFloat(const std::string& name, float value);
  void setString(const std::string& name, const std::string& value);
  void setString(const std::string& name, const char* value);
  void setBool(const std::string& name, bool value);
  void setPointer(const std::string&name, Identifiable* ptr);
  
  int getInt(const std::string& name) {
    return getField(name)->getInt();
  }
  
  double getDouble(const std::string& name) {
    return getField(name)->getDouble();
  }
  
  float getFloat(const std::string& name) {
    return getField(name)->getFloat();
  }
  
  const std::string& getString(const std::string& name) {
    return getField(name)->getString();
  }
  
  bool getBool(const std::string& name) {
    return getField(name)->getBool();
  }
  
  void getPointer(const std::string&name, Identifiable*& pvar);
  
  ValueData* getField(const std::string& name);
  
  virtual ~ObjectData();

protected:
  std::map<std::string, ValueData*> _value;
};

class PointerData: public ValueData {
public:
  PointerData(Identifiable* pointer): _pointer(pointer) {}
  virtual Identifiable* getPointer();
  virtual ValueType type();
  
protected:
  Identifiable* _pointer;
};

class PointerReference: public ValueData {
public:
  PointerReference(IdPlaceholder* ref): _ref(ref) {}

  virtual void bindPointer(Identifiable*& pvar);
  virtual ValueType type();
protected:
  IdPlaceholder* _ref;
};

std::pair<const std::string&, const int&> field(const std::string& nm, const int& val);
std::pair<const std::string&, int&> field(const std::string& nm, int& val);
ObjectData& operator << (ObjectData& o, std::pair<const std::string&, const int&> f);
ObjectData& operator << (ObjectData& o, std::pair<const std::string&, int&> f);
ObjectData& operator >> (ObjectData& o, std::pair<const std::string&, int&> f);

std::pair<const std::string&, const float&> field(const std::string& nm, const float& val);
std::pair<const std::string&, float&> field(const std::string& nm, float& val);
ObjectData& operator << (ObjectData& o, std::pair<const std::string&, const float&> f);
ObjectData& operator << (ObjectData& o, std::pair<const std::string&, float&> f);
ObjectData& operator >> (ObjectData& o, std::pair<const std::string&, float&> f);

std::pair<const std::string&, const double&> field(const std::string& nm, const double& val);
std::pair<const std::string&, double&> field(const std::string& nm, double& val);
ObjectData& operator << (ObjectData& o, std::pair<const std::string&, const double&> f);
ObjectData& operator << (ObjectData& o, std::pair<const std::string&, double&> f);
ObjectData& operator >> (ObjectData& o, std::pair<const std::string&, double&> f);

std::pair<const std::string&, const bool&> field(const std::string& nm, const bool& val);
std::pair<const std::string&, bool&> field(const std::string& nm, bool& val);
ObjectData& operator << (ObjectData& o, std::pair<const std::string&, const bool&> f);
ObjectData& operator << (ObjectData& o, std::pair<const std::string&, bool&> f);
ObjectData& operator >> (ObjectData& o, std::pair<const std::string&, bool&> f);

std::pair<const std::string&, const std::string&> field(const std::string& nm, const std::string& val);
std::pair<const std::string&, std::string&> field(const std::string& nm, std::string& val);
ObjectData& operator << (ObjectData& o, std::pair<const std::string&, const std::string&> f);
ObjectData& operator << (ObjectData& o, std::pair<const std::string&, std::string&> f);
ObjectData& operator >> (ObjectData& o, std::pair<const std::string&, std::string&> f);

}

#endif // BOSS_OBJECT_DATA_H
