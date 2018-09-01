
make clean
echo "export LD_LIBRARY_PATH=${PWD}/lib" >> ~/.bashrc
. ~/.bashrc
make 
