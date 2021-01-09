// https://code-live.ru/post/cpp-http-server-over-sockets/

#include <fstream> // работа с файлами
#include <iostream> // работа вводом/выводом данных
#include <sstream> // работа со стримами
#include <string> // работа со строками

#include "libs/rex/rex.server.h"

using namespace std;

class E { public: void Exception() { cout << "exception" << endl; } };

int main() {
    try{

        Rex server; // rex берёт настройки из config/rex.config.json
        // можно и так подключить
        // Rex server("localhost","8080", "view"); 
        server.run(); // старт сервера

    }catch(E e) {
        e.Exception();
    }



    return 0;
}
//===========================================================================