rm -rf /mnt/data/Document/DSnP/hw4/log/*

./memTest -fil tests/do1 >& log/mydo1
./memTest -fil tests/do2 >& log/mydo2
./memTest -fil tests/do3 >& log/mydo3
./memTest -fil tests/do4 >& log/mydo4
./memTest -fil tests/do5 >& log/mydo5

ref/memTest -fil tests/do1 >& log/refdo1
ref/memTest -fil tests/do2 >& log/refdo2
ref/memTest -fil tests/do3 >& log/refdo3
ref/memTest -fil tests/do4 >& log/refdo4
ref/memTest -fil tests/do5 >& log/refdo5
