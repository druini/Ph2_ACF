source setup.sh
rm -r build/*
cd build 
cmake ../
cd ..
make -C build/ -j 4 


