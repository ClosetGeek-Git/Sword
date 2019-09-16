/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <cstring>
#include <chrono>

using namespace std;

class Message
{
public:
  int command;
  int id;
  int src;
  int dest;
  int finger_idx;

  Message() {}

  Message(int command, int id, int src, int dest, int finger_idx) {
    this->command = command;
    this->id = id;
    this->src = src;
    this->dest = dest;
    this->finger_idx = finger_idx;
  }

  void setContent(int command, int id, int src, int dest, int finger_idx) {
    this->command = command;
    this->id = id;
    this->src = src;
    this->dest = dest;
    this->finger_idx = finger_idx;
  }
};
