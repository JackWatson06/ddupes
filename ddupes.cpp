#include "extract.h"

/*
** Functionality and Design **
- Take in the directories that we want to use to find the ddupes for.
- Run the fdupes command over the set of the directories. I wonder how I can
exec in c++
- Move and remove the cache created by fdupes into a temporary directory.
- Get a list of all the directories from the SQLLite
- Simple layer structure. Domain layer has hashing logic, and structs.
- I think I actually need a tree structure for this. Basically I create the tree
structure then I
- Have an array of three directory trees in memory.
- Loop through the leaf nodes. If parent has a hash somewhere in the system then
continue up the tree until I find a unique directroy.
- Depth First Search and Hashmap for Hashes to nodes.
- When I find a unique directory then don't navigate up to parent and clip this
branch off the tree and store the full path of the duplicate directory or file.
- Store the duplicate directories in a grouped result set.
- Since a function could call delete on an object the desctuctors are part of
the objects interface A function does not construct an object of the passed in
type. The object will already be constructed. When it's passed in.
*/

/**
** TODO **
- Load from the SQLLite db.
- Remove the SQLLite cache at the end.
- Having a filter to return the folder I am specifically looking at would be
wonderful.
- Change our DirectoryTableRow to use a -1 to represent a null instead of
pointer for the parent_id.
- How can we get the home directory of the current user when finding the hash.db
file.
- Integration tests with the SQLLite query classes.
- Improve error handling for sqlite
*/

/**
** Example Data **
fdupes --cache testing_dirs/dir1/ testing_dirs/dir1/
cat ~/.cache/fdupes/hash.db

Directories
1|/|/|
2|home|/home|1
3|jack|/home/jack|2
4|Downloads|/home/jack/Downloads|3
8|WorkingDirectory|/home/jack/WorkingDirectory|3
9|ddupes|/home/jack/WorkingDirectory/ddupes|8
10|testing_dirs|/home/jack/WorkingDirectory/ddupes/testing_dirs|9
11|dir1|/home/jack/WorkingDirectory/ddupes/testing_dirs/dir1|10
12|dir2|/home/jack/WorkingDirectory/ddupes/testing_dirs/dir2|10
13|dir3|/home/jack/WorkingDirectory/ddupes/testing_dirs/dir3|10
14|sub_dir|/home/jack/WorkingDirectory/ddupes/testing_dirs/dir2/sub_dir|12
15|sub_dir_two|/home/jack/WorkingDirectory/ddupes/testing_dirs/dir2/sub_dir_two|12
16|sub_dir|/home/jack/WorkingDirectory/ddupes/testing_dirs/dir1/sub_dir|11
17|sub_dir_two|/home/jack/WorkingDirectory/ddupes/testing_dirs/dir1/sub_dir_two|11


Hashes
13|example1.txt|�H|13|�_g|�_g|825233896|825233896|�݋�y�)�����K�X|4096|�݋�y�)�����K�X|1
12|example1.txt|K|13|�_g|�_g|696247435|696247435|�݋�y�)�����K�X|4096|�݋�y�)�����K�X|1
14|example4.txt|�H|13|�_g|�_g|591343502|591343502|�E��#,G.�;����P|4096|�E��#,G.�;����P|1
12|example5.txt|L|13| _g|
_g|460096331|460096331|�E��#,G.�;����P|4096|�E��#,G.�;����P|1
11|example1.txt|F|13|?_g|?_g|361453834|361453834|�݋�y�)�����K�X|4096|�݋�y�)�����K�X|1
16|example4.txt||H|13|�_g|�_g|356892402|356892402|�E��#,G.�;����P|4096|�E��#,G.�;����P|1
13|example3.txt|H|12|g|g|48054447|48054447|�^�.�T�Y��A����<|4096|�^�.�T�Y��A����<|1
11|example3.txt|I|12|d_g|d_g|69219403|69219403|�^�.�T�Y��A����<|4096|�^�.�T�Y��A����<|1
11|example5.txt|jH|13|�_g|�_g|875691672|803794009|�E��#,G.�;����P|4096|�E��#,G.�;����P|1
15|example7.txt|q|9|*wg|*wg|618860719|618860719|���u�ۮL��5�ўp|4096|���u�ۮL��5�ўp|1
17|example7.txt|^|9|wg|wg|889871227|889871227|���u�ۮL��5�ўp|4096|���u�ۮL��5�ўp|1
15|example8.txt|�|13|*wg|*wg|619860718|619860718|zGg+��%S��˯$tB|4096|zGg+��%S��˯$tB|1
17|example8.txt|_|13|wg|wg|929869554|929869554|zGg+��%S��˯$tB|4096|zGg+��%S��˯$tB|1
 */

int main() {
  extract({"testing_dirs/dir1", "testing_dirs/dir2"},
          "/home/jack/.cache/fdupes/hash.db");
  return 0;
}
