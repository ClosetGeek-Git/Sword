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
#include <vector>
#include <queue>
#include <unordered_map>
#include <map>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <string>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define FIND_SUCCESSOR 1
#define JOIN 2
#define LEAVE 3
#define SHOW 4
#define GET_SUCCESSOR 5
#define GET_PREDECESSOR 6
#define SET_PREDECESSOR 7
#define CLOSEST_PRECEDING_FINGER 8
#define ACK 9
#define RETURN_SUCCESSOR 10
#define RETURN_PREDECESSOR 11
#define RETURN_CPF 12
#define UPDATE_FINGER_TABLE 13

const int M = 4;

typedef struct _chord_node_data {
  int id;
  int ip;
  int port;
} chord_data;

typedef struct _chord_finder_data {
  chord_data node;
  int start;
} finger_data;

#include "message.h"
#include "chord_node.h"

#include "swoole_chord_coro.h"

const int max_id = (int)pow(2, M);

using namespace std;
using namespace swoole;

int msg_count = 0;
int type_count[14] = {0};

mutex count_mtx;
FILE * output = stdout;
std::vector<thread*> all_threads;

map<int, int> nodes;

void node_thread(int node_id);

string get_port(int id) {
  return to_string(id + 10000);
}

void do_close(int out_fd) {
  close(out_fd);
}

void increment_count(int type) {
  count_mtx.lock();
  msg_count++;
  type_count[type]++;
  count_mtx.unlock();
}

void reply_message(int fd, Message & msg) {
  write(fd, &msg, sizeof(Message));
  increment_count(msg.command);
}

int send_message(Message & msg) {

  int s;
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET; /* IPv4 only */
  hints.ai_socktype = SOCK_STREAM; /* TCP */

  s = getaddrinfo(NULL, get_port(msg.dest).c_str(), &hints, &result);

  if(s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(1);
  }

  connect(sock_fd, result->ai_addr, result->ai_addrlen);

  write(sock_fd, &msg, sizeof(Message));
  increment_count(msg.command);

  freeaddrinfo(result);
  return sock_fd;
}

void send_join(int node_id, int arb_node_id) {
  thread* t = new thread(node_thread, node_id);
  all_threads.push_back(t);

  ::sleep(1);

  Message msg(JOIN, arb_node_id, -1, node_id, -1);
  int out_fd = send_message(msg);
  read(out_fd, &msg, sizeof(Message));
  if(msg.command != ACK) cout << "error" <<endl;
  do_close(out_fd);
  return;
}

void send_leave(int node_id) {
  Message msg(LEAVE, -1, -1, node_id, -1);
  int out_fd = send_message(msg);
  read(out_fd, &msg, sizeof(Message));
  if(msg.command != ACK) cout << "error" <<endl;
  do_close(out_fd);
  return;
}

void send_show(int node_id) {
  Message msg(SHOW, -1, -1, node_id, -1);
  int out_fd = send_message(msg);
  read(out_fd, &msg, sizeof(Message));
  if(msg.command != ACK) cout << "error" <<endl;
  do_close(out_fd);
  return;
}

int send_find_successor(int node_id, int id) {
  Message msg(FIND_SUCCESSOR, id, -1, node_id, -1);
  int out_fd = send_message(msg);
  read(out_fd, &msg, sizeof(Message));
  if(msg.command != RETURN_SUCCESSOR) cout << "error" <<endl;
  do_close(out_fd);
  return msg.id;
}

int send_closest_preceding_finger(int node_id, int id) {
  Message msg(CLOSEST_PRECEDING_FINGER, id, -1, node_id, -1);
  int out_fd = send_message(msg);
  read(out_fd, &msg, sizeof(Message));
  if(msg.command != RETURN_CPF) cout << "error" <<endl;
  do_close(out_fd);
  return msg.id;
}

int send_get_successor(int node_id) {
  Message msg(GET_SUCCESSOR, -1, -1, node_id, -1);
  int out_fd = send_message(msg);
  read(out_fd, &msg, sizeof(Message));
  if(msg.command != RETURN_SUCCESSOR) cout << "error" <<endl;
  do_close(out_fd);
  return msg.id;
}

int send_get_predecessor(int node_id) {
  Message msg(GET_PREDECESSOR, -1, -1, node_id, -1);
  int out_fd = send_message(msg);
  read(out_fd, &msg, sizeof(Message));
  if(msg.command != RETURN_PREDECESSOR) cout << "error" <<endl;
  do_close(out_fd);
  return msg.id;
}

void send_set_predecessor(int node_id, int id) {
  Message msg(SET_PREDECESSOR, id, -1, node_id, -1);
  int out_fd = send_message(msg);
  read(out_fd, &msg, sizeof(Message));
  if(msg.command != ACK) cout << "error" <<endl;
  do_close(out_fd);
  return;
}

void send_update_finger_table(int node_id, int id, int idx)
{
  Message msg(UPDATE_FINGER_TABLE, id, -1, node_id, idx);
  int out_fd = send_message(msg);
  read(out_fd, &msg, sizeof(Message));
  if(msg.command != ACK) cout << "error" <<endl;
  do_close(out_fd);
  return;
}

void send_update_finger_table(int node_id, ChordNode & node, int idx)
{
  Message msg(UPDATE_FINGER_TABLE, node.fingers[0].node.id, node.node_data.id, node_id, idx);
  int out_fd = send_message(msg);
  read(out_fd, &msg, sizeof(Message));
  if(msg.command != ACK) cout << "error" <<endl;
  do_close(out_fd);
  return;
}

bool range(int low, int lowoffset, int high, int highoffset, int arg) {
  if(arg > max_id || arg < 0) {
    return false;
  }
  if(low == high) {
    if(lowoffset == highoffset && lowoffset == 0) {
      return (arg != low);
    }
    else {
      return true;
    }
  }
  else {
    if(low < high) {
      return ((arg > (low - lowoffset)) && (arg < (high + highoffset)));
    }
    else {
      return (((low - lowoffset) < arg) || (arg < (high + highoffset)));
    }
  }
}

int closest_preceding_finger(int id, ChordNode & node) {
  for(int i = M-1; i > -1; --i) {
    if(range(node.node_data.id, 0, id, 0, node.fingers[i].node.id)) {
      return node.fingers[i].node.id;
    }
  }
  return node.node_data.id;
}

int find_predecessor(int id, ChordNode & node) {
  int n = node.node_data.id;
  int suc = node.fingers[0].node.id;
  while(!range(n, 0, suc, 1, id))
  {
    if(n == node.node_data.id) {
      n = closest_preceding_finger(id, node);
    }
    else {
      n = send_closest_preceding_finger(n, id);
    }
    if(n == node.node_data.id) {
      suc = node.fingers[0].node.id;
    }
    else {
      suc = send_get_successor(n);
    }
  }
  return n;
}

void init_finger_table(int arb_node_id, ChordNode & node) {
  node.fingers[0].node.id = send_find_successor(arb_node_id, node.fingers[0].start);
  node.predecessor = send_get_predecessor(node.fingers[0].node.id);
  send_set_predecessor(node.fingers[0].node.id, node.node_data.id);

  for(int i = 0; i < M-1; ++i) {
    
    if(range(node.node_data.id, 1, node.fingers[i].node.id, 0, node.fingers[i+1].start)) {
      node.fingers[i+1].node.id = node.fingers[i].node.id;
    }
    else {
      node.fingers[i+1].node.id = send_find_successor(arb_node_id, node.fingers[i+1].start);
    }
  }
}

void update_others_join(ChordNode & node) {
  for(int i = 0; i < M; ++i) {
    
    int tmp = node.node_data.id - pow(2,i) + 1;
    int arg = (tmp >= 0) ? tmp : (tmp + max_id + 1) % (int)pow(2, M);
    
    int p = find_predecessor(arg, node);
    
    if(p != node.node_data.id) {
      send_update_finger_table(p, node.node_data.id, i);
    }
  }
}

void update_others_leave(ChordNode & node) {
  for(int i = 0; i < M; ++i) {
    
    int tmp = node.node_data.id - pow(2,i) + 1;
    int arg = (tmp >= 0) ? tmp : (tmp + max_id + 1) % (int)pow(2, M);
    
    int p = find_predecessor(arg, node);
    
    if(p != node.node_data.id) {
      send_update_finger_table(p, node, i);
    }
  }
}

bool message_handler(int fd, ChordNode & node)
{
  Message msg;
  Message reply;
  read(fd, &msg, sizeof(Message));

  switch (msg.command) {
    case FIND_SUCCESSOR:
    {
      int pre = find_predecessor(msg.id, node);
      int suc;
      if(pre == node.node_data.id) {
        suc = node.fingers[0].node.id;
      }
      else {
        suc = send_get_successor(pre);
      }
      reply.setContent(RETURN_SUCCESSOR, suc, node.node_data.id, msg.src, -1);
      reply_message(fd, reply);
      break;
    }
    case JOIN:
    {
      if(msg.id == -1) {
        node.predecessor = node.node_data.id;
        for(int i = 0; i < M; ++i) {
          node.fingers[i].node.id = node.node_data.id;
        }
      }
      else {
        init_finger_table(msg.id, node);
        update_others_join(node);
      }
      reply.setContent(ACK, node.node_data.id, -1, msg.src, -1);
      reply_message(fd, reply);
      break;
    }
    case LEAVE:
    {
      if(node.predecessor != node.node_data.id) {
        update_others_leave(node);
        send_set_predecessor(node.fingers[0].node.id, node.predecessor);
      }
      reply.setContent(ACK, node.node_data.id, -1, msg.src, -1);
      reply_message(fd, reply);
      close(fd);
      return true;
    }
    case SHOW:
    {
      string out = "=======================================================\n";
      out = out + "FingerTable:\t" + to_string(node.node_data.id) + "\n";
      out = out + "-------------------------------------------------------\n";
      out = out + "Predecessor:\t" + to_string(node.predecessor) + "\n";
      out = out + "Successor:\t" + to_string(node.fingers[0].node.id) + "\n";
      out = out + "-------------------------------------------------------\n";            
      for(int i = 0; i < M; i++) {
        out = out + "finger[" + to_string(i) + "]\t" + to_string(node.fingers[i].start) + "\n";
      }
      out = out + "=======================================================\n\n";
      fprintf(output, "%s\n", out.c_str());
      reply.setContent(ACK, -1, -1, msg.src, -1);
      reply_message(fd, reply);
            
      break;
    }
    case GET_SUCCESSOR:
    {
      reply.setContent(RETURN_SUCCESSOR, node.fingers[0].node.id, -1, msg.src, -1);
      reply_message(fd, reply);
      break;
    }
    case GET_PREDECESSOR:
    {
      reply.setContent(RETURN_PREDECESSOR, node.predecessor, -1, msg.src, -1);
      reply_message(fd, reply);
      break;
    }
    case SET_PREDECESSOR:
    {
      node.predecessor = msg.id;
      reply.setContent(ACK, msg.id, -1, msg.src, -1);
      reply_message(fd, reply);
      break;
    }
    case CLOSEST_PRECEDING_FINGER:
    {
      int n = closest_preceding_finger(msg.id, node);
      reply.setContent(RETURN_CPF, n, -1, msg.src, -1);
      reply_message(fd, reply);
      break;
    }
    case UPDATE_FINGER_TABLE:
    {
      int i =  msg.finger_idx;
      if(msg.src == -1) {
        if(range(node.node_data.id, 1, node.fingers[i].node.id, 0, msg.id)) {
          node.fingers[i].node.id = msg.id;
          if(node.predecessor != msg.id) {
            send_update_finger_table(node.predecessor, msg.id, i);
          }
        }
      }
      else {
        if(node.fingers[i].node.id == msg.src) {
          node.fingers[i].node.id = msg.id;
          if(node.predecessor != msg.src){
            ChordNode tmp = ChordNode(msg.src, msg.id);
            send_update_finger_table(node.predecessor, tmp, i);
          }
        }
      } 
      reply.setContent(ACK, -1, -1, msg.src, -1);
      reply_message(fd, reply);
      break;
    }
  }

  close(fd);
  return false;
}

void node_thread(int node_id) {
  int s;
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  s = getaddrinfo(NULL, get_port(node_id).c_str(), &hints, &result);
  if(s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(1);
  }

  int optval = 1;
  setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

  if( ::bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0 ) {
    perror("bind()");
    exit(1);
  }

  if( listen(sock_fd, 10) != 0 ) {
    perror("listen()");
    exit(1);
  }

  freeaddrinfo(result);

  ChordNode node(node_id);
  bool leave = false;

  while(!leave) {
    int fd = accept(sock_fd, NULL, NULL);
    leave = message_handler(fd, node);
  }

  close(sock_fd);
  return;
}


// END OF main.cpp

PHP_MINIT_FUNCTION(swoole_chord);
PHP_MINFO_FUNCTION(swoole_chord);
void swoole_chord_init(int module_number);

static const zend_module_dep swoole_chord_deps[] = {
  ZEND_MOD_REQUIRED("swoole")
  ZEND_MOD_END
};

zend_module_entry swoole_chord_module_entry = {
  STANDARD_MODULE_HEADER_EX, NULL,
  swoole_chord_deps,
  "swoole_chord",
  NULL,
  PHP_MINIT(swoole_chord),
  NULL,
  NULL,
  NULL,
  PHP_MINFO(swoole_chord),
  PHP_SWOOLE_VERSION,
  STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SWOOLE_CHORD
ZEND_GET_MODULE(swoole_chord)
#endif

static PHP_METHOD(swoole_chord_coro, __construct);
static PHP_METHOD(swoole_chord_coro, __destruct);
static PHP_METHOD(swoole_chord_coro, join);
static PHP_METHOD(swoole_chord_coro, find);
static PHP_METHOD(swoole_chord_coro, leave);
static PHP_METHOD(swoole_chord_coro, show);
static PHP_METHOD(swoole_chord_coro, quit);

static void _free_result(zend_resource *rsrc);

static void _free_result(zend_resource *rsrc) {
  chdht_object *chdht_result = (chdht_object *)rsrc->ptr;
  efree(chdht_result);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_chord_join, 0, 0, 1)
  ZEND_ARG_INFO(0, node)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_chord_find, 0, 0, 2)
  ZEND_ARG_INFO(0, node)
  ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_chord_leave, 0, 0, 1)
  ZEND_ARG_INFO(0, node)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_chord_show, 0, 0, 0)
  ZEND_ARG_INFO(0, node)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_chord_quit, 0, 0, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry swoole_chord_coro_methods[] =
{
  PHP_ME(swoole_chord_coro, __construct, arginfo_swoole_void, ZEND_ACC_PUBLIC)
  PHP_ME(swoole_chord_coro, join, arginfo_chord_join, ZEND_ACC_PUBLIC)
  PHP_ME(swoole_chord_coro, find, arginfo_chord_find, ZEND_ACC_PUBLIC )
  PHP_ME(swoole_chord_coro, leave, arginfo_chord_leave, ZEND_ACC_PUBLIC )
  PHP_ME(swoole_chord_coro, show, arginfo_chord_show, ZEND_ACC_PUBLIC )
  PHP_ME(swoole_chord_coro, quit, arginfo_chord_quit, ZEND_ACC_PUBLIC)
  PHP_ME(swoole_chord_coro, __destruct, arginfo_swoole_void, ZEND_ACC_PUBLIC)
  PHP_FE_END
};

static zend_class_entry *swoole_chord_coro_ce;
static zend_object_handlers swoole_chord_coro_handlers;
static int le_result;

void swoole_chord_init(int module_number)
{
  SW_INIT_CLASS_ENTRY(swoole_chord_coro, "Swoole\\Coroutine\\ChordDHT", NULL, "Co\\ChordDHT", swoole_chord_coro_methods);
  SW_SET_CLASS_SERIALIZABLE(swoole_chord_coro, zend_class_serialize_deny, zend_class_unserialize_deny);
  SW_SET_CLASS_CLONEABLE(swoole_chord_coro, sw_zend_class_clone_deny);
  SW_SET_CLASS_UNSET_PROPERTY_HANDLER(swoole_chord_coro, sw_zend_class_unset_property_deny);
  le_result = zend_register_list_destructors_ex(_free_result, NULL, "chord result", module_number);
  zend_declare_property_null(swoole_chord_coro_ce, ZEND_STRL("error"), ZEND_ACC_PUBLIC);

  SW_REGISTER_LONG_CONSTANT("SW_CHORD_ASSOC", CHORD_ASSOC);
  SW_REGISTER_LONG_CONSTANT("SW_CHORD_NUM", CHORD_NUM);
  SW_REGISTER_LONG_CONSTANT("SW_CHORD_BOTH", CHORD_BOTH);
}

static PHP_METHOD(swoole_chord_coro, __construct)
{
  chdht_object *pg = (chdht_object *) emalloc(sizeof(chdht_object));
  bzero(pg, sizeof(chdht_object));
  pg->object = ZEND_THIS;
  swoole_set_object(ZEND_THIS, pg);

  nodes[0] = 1;
  send_join(0, -1);
}

static PHP_METHOD(swoole_chord_coro, join) {

  zend_long node_l = 0;
  
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(node_l);
  ZEND_PARSE_PARAMETERS_END();
    
  chdht_object *object = (chdht_object *) swoole_get_object(ZEND_THIS);
  
  int node= (int)node_l;
  if(node > max_id || nodes.find(node) != nodes.end()) {
    cout << "Invalid node id " << node << endl;
  } else {
    nodes[node] = 1;
    send_join(node, nodes.begin()->first);
  } 
}

static PHP_METHOD(swoole_chord_coro, find) {

  zend_long node_l = 0;
  zend_long key_l = 0;
  
  ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_LONG(node_l);
    Z_PARAM_LONG(key_l);
  ZEND_PARSE_PARAMETERS_END();  
  
  chdht_object *object = (chdht_object *) swoole_get_object(ZEND_THIS);

  int node = (int)node_l;
  int key = (int)key_l;
  if(key > max_id || nodes.find(node) == nodes.end()) {
    cout << "Invalid input " << endl;
  } else {
    int location = send_find_successor(node, key);
    cout << "Found key " << key << " at node " << location << endl;
  } 
}

static PHP_METHOD(swoole_chord_coro, leave) {

  zend_long node_l = 0;
  
  ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(node_l);
  ZEND_PARSE_PARAMETERS_END();

  chdht_object *object = (chdht_object *) swoole_get_object(ZEND_THIS);

  int node = (int)node_l;
  if(nodes.find(node) == nodes.end()) {
    cout << "Invalid node id " << node << endl;
  } else if(nodes.size() == 1) {
    cout << "Only one node in the network" << endl;
  } else {
    nodes.erase(node);
    send_leave(node);
  } 
}

static PHP_METHOD(swoole_chord_coro, show) {
  
  zend_long node_l = -10;
  
  ZEND_PARSE_PARAMETERS_START(0, 1)
    Z_PARAM_OPTIONAL
    Z_PARAM_LONG(node_l)
  ZEND_PARSE_PARAMETERS_END();
    
  chdht_object *object = (chdht_object *) swoole_get_object(ZEND_THIS);
  
  if(node_l == -10) {
    for(auto it = nodes.begin(); it != nodes.end(); ++it) {
      if(it->second == 1) send_show(it->first);
    }
  }
  else {
    int node = (int)node_l;
    if(nodes.find(node) == nodes.end()) {
      cout << "Invalid node id " << node << endl;
    } else {
      send_show(node);
    }
  }
}

static PHP_METHOD(swoole_chord_coro, quit) {
  chdht_object *object = (chdht_object *) swoole_get_object(ZEND_THIS);

  while(nodes.size()!=0) {
    send_leave(nodes.begin()->first);
    nodes.erase(nodes.begin()->first);
  }
  for(size_t i = 0; i < all_threads.size(); ++i) {
    all_threads[i]->join();
    delete all_threads[i];
  }
}

static PHP_METHOD(swoole_chord_coro, __destruct) {
  SW_PREVENT_USER_DESTRUCT();

  chdht_object *object = (chdht_object *) swoole_get_object(ZEND_THIS);
  if(object) {
    efree(object);
    swoole_set_object(ZEND_THIS, NULL);
  }
}
  
PHP_MINIT_FUNCTION(swoole_chord)
{
  swoole_chord_init(module_number);
  return SUCCESS;
}

PHP_MINFO_FUNCTION(swoole_chord)
{
  char buf[64];
  php_info_print_table_start();
  php_info_print_table_header(2, "Swoole Chord DHT", "enabled");
  php_info_print_table_row(2, "Author", "Jason Lester <jasonrlester@yahoo.com>");
  php_info_print_table_row(2, "Version", SWOOLE_VERSION);
  snprintf(buf, sizeof(buf), "%s %s", __DATE__, __TIME__);
  php_info_print_table_row(2, "Built", buf);

#ifdef SW_DEBUG
  php_info_print_table_row(2, "debug", "enabled");
#endif
#ifdef SW_LOG_TRACE_OPEN
  php_info_print_table_row(2, "trace_log", "enabled");
#endif

  php_info_print_table_end();

  DISPLAY_INI_ENTRIES();
}
