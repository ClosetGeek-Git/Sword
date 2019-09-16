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
#include <map>
#include <cmath>
using namespace std;

class ChordNode
{
public:
  chord_data node_data;
  int predecessor;
  
  finger_data fingers[M];
  
  ChordNode(int id) {
    this->node_data.id = id;
    for(int i = 0; i < M; ++i) {
      this->fingers[i].start = (id + (int)pow(2, i)) % (int)pow(2, M);
    }
  }
  
  ChordNode(int id, int suc)  {
    this->node_data.id = id;
    this->fingers[0].node.id = suc;
  }

};
