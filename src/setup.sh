
make clean
echo "export GUROBI_HOME=${PWD}/gurobi801/linux64" >> ~/.bashrc
echo "export PATH=${PATH}:${GUROBI_HOME}/bin" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=${PWD}/lib" >> ~/.bashrc
. ~/.bashrc
make 
