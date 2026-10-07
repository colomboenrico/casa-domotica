g++ -std=c++17 sensore.cpp -o sensore -I$(brew --prefix mosquitto)/include -I$(brew --prefix cjson)/include -L$(brew --prefix mosquitto)/lib -lmosquitto
