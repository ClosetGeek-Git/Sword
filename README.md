# Sword
Distributed P2P message routing and node discovery system using Chord protocol and Swoole coroutine library.

A work in progress. I'm working on last two tasks
1. Currently only works over hardcoded `127.0.0.1` address. Refactoring to connect to any ip address.
2. Need to replace system sockets with swoole coro sockets.

Requires Swoole PHP extension installed from source.

```
# to build
phpize
./configure
make
make instal
```

Only tested on Ubuntu  but should workbon any *nix like OS. Currenlty creates a ring with max size of 16 nodes (0-15) to illustrate that it works correctly. This can be seen by executing `example.php` resulting with the following:
 ```
 Creating node '0' (0 total calls over Sword network)
Creating node '1' (2 total calls over Sword network)
Creating node '2' (14 total calls over Sword network)
Creating node '3' (40 total calls over Sword network)
Creating node '4' (72 total calls over Sword network)
Creating node '5' (116 total calls over Sword network)
Creating node '6' (162 total calls over Sword network)
Creating node '7' (212 total calls over Sword network)
Creating node '8' (270 total calls over Sword network)
Creating node '9' (350 total calls over Sword network)
Creating node '10' (412 total calls over Sword network)
Creating node '11' (482 total calls over Sword network)
Creating node '12' (552 total calls over Sword network)
Creating node '13' (644 total calls over Sword network)
Creating node '14' (716 total calls over Sword network)
Creating node '15' (814 total calls over Sword network)

=======================================================
Node:	0
-------------------------------------------------------
Predecessor:	15
Successor:	1
-------------------------------------------------------
finger[0]	1
finger[1]	2
finger[2]	4
finger[3]	8
=======================================================


=======================================================
Node:	1
-------------------------------------------------------
Predecessor:	0
Successor:	2
-------------------------------------------------------
finger[0]	2
finger[1]	3
finger[2]	5
finger[3]	9
=======================================================


=======================================================
Node:	2
-------------------------------------------------------
Predecessor:	1
Successor:	3
-------------------------------------------------------
finger[0]	3
finger[1]	4
finger[2]	6
finger[3]	10
=======================================================


=======================================================
Node:	3
-------------------------------------------------------
Predecessor:	2
Successor:	4
-------------------------------------------------------
finger[0]	4
finger[1]	5
finger[2]	7
finger[3]	11
=======================================================


=======================================================
Node:	4
-------------------------------------------------------
Predecessor:	3
Successor:	5
-------------------------------------------------------
finger[0]	5
finger[1]	6
finger[2]	8
finger[3]	12
=======================================================


=======================================================
Node:	5
-------------------------------------------------------
Predecessor:	4
Successor:	6
-------------------------------------------------------
finger[0]	6
finger[1]	7
finger[2]	9
finger[3]	13
=======================================================


=======================================================
Node:	6
-------------------------------------------------------
Predecessor:	5
Successor:	7
-------------------------------------------------------
finger[0]	7
finger[1]	8
finger[2]	10
finger[3]	14
=======================================================


=======================================================
Node:	7
-------------------------------------------------------
Predecessor:	6
Successor:	8
-------------------------------------------------------
finger[0]	8
finger[1]	9
finger[2]	11
finger[3]	15
=======================================================


=======================================================
Node:	8
-------------------------------------------------------
Predecessor:	7
Successor:	9
-------------------------------------------------------
finger[0]	9
finger[1]	10
finger[2]	12
finger[3]	0
=======================================================


=======================================================
Node:	9
-------------------------------------------------------
Predecessor:	8
Successor:	10
-------------------------------------------------------
finger[0]	10
finger[1]	11
finger[2]	13
finger[3]	1
=======================================================


=======================================================
Node:	10
-------------------------------------------------------
Predecessor:	9
Successor:	11
-------------------------------------------------------
finger[0]	11
finger[1]	12
finger[2]	14
finger[3]	2
=======================================================


=======================================================
Node:	11
-------------------------------------------------------
Predecessor:	10
Successor:	12
-------------------------------------------------------
finger[0]	12
finger[1]	13
finger[2]	15
finger[3]	3
=======================================================


=======================================================
Node:	12
-------------------------------------------------------
Predecessor:	11
Successor:	13
-------------------------------------------------------
finger[0]	13
finger[1]	14
finger[2]	0
finger[3]	4
=======================================================


=======================================================
Node:	13
-------------------------------------------------------
Predecessor:	12
Successor:	14
-------------------------------------------------------
finger[0]	14
finger[1]	15
finger[2]	1
finger[3]	5
=======================================================


=======================================================
Node:	14
-------------------------------------------------------
Predecessor:	13
Successor:	15
-------------------------------------------------------
finger[0]	15
finger[1]	0
finger[2]	2
finger[3]	6
=======================================================


=======================================================
Node:	15
-------------------------------------------------------
Predecessor:	14
Successor:	0
-------------------------------------------------------
finger[0]	0
finger[1]	1
finger[2]	3
finger[3]	7
=======================================================

------------------
(program exited with code: 0)
Press return to continue
```
