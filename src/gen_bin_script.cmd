echo '#!/bin/bash'
echo 'LD_LIBRARY_PATH=$LD_LIBRARY_PATH:'$1
echo 'export LD_LIBRARY_PATH'
echo $1'/'$2 '"$@"'
