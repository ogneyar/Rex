
#include <fstream> // работа с файлами
#include <iostream> // работа вводом/выводом данных
#include <sstream> // работа со стримами
#include <string> // работа со строками
#include <cstring> // работа с символами (char)
#include <typeinfo> // работа с типами данных
#include <dir.h> // работа с директориями

#include "rex.functions.h"

// Для корректной работы freeaddrinfo в MinGW
// Подробнее: http://stackoverflow.com/a/20306451
// #define _WIN32_WINNT 0x501 

#include <WinSock2.h>
#include <WS2tcpip.h>

// Необходимо, чтобы линковка происходила с DLL-библиотекой 
// Для работы с сокетом
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

class Rex { 
private:
    const char *host;
    const char *port;    
    string view;

public: 
    Rex() : host("127.0.0.1"), port("8000"), view("html") {
    
        if (fileExist("config/rex.config.json")) {
            
            int nLength;
            string fieldString;

            fieldString = Rex::parserConfig("host");
            if (fieldString == "Error") fieldString = "127.0.0.1";
            char szBuf[fieldString.length()];    
            nLength = fieldString.copy(szBuf, fieldString.length());
            szBuf[nLength] = '\0';            
            host = szBuf;
            
            fieldString = Rex::parserConfig("port");
            if (fieldString == "Error") fieldString = "8000";
            char szBuf_2[fieldString.length()];
            nLength = fieldString.copy(szBuf_2, fieldString.length());
            szBuf_2[nLength] = '\0';
            port = szBuf_2;

            view = Rex::parserConfig("view");
            if (view == "Error") view = "html";

        }else {
            Rex::createConfigFiles();            
        }
        
    }
    // Rex(const char *newHost) : host(newHost) { }
    // Rex(const char *newHost, const char *newPort) : host(newHost), port(newPort) { }
    Rex(
        const char *newHost, 
        const char *newPort, 
        string newView
        ) : host(newHost), port(newPort), view(newView) { }
   

    // int run();
    // string getRequest(char);

    int run() {
        // служебная структура для хранение информации
        // о реализации Windows Sockets
        WSADATA wsaData;

        // старт использования библиотеки сокетов процессом
        // (подгружается Ws2_32.dll)
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

        // Если произошла ошибка подгрузки библиотеки
        if (result != 0) {
            cerr << "WSAStartup failed: " << result << "\n";
            return result;
        }

        struct addrinfo* addr = NULL; // структура, хранящая информацию
        // об IP-адресе  слущающего сокета

        // Шаблон для инициализации структуры адреса
        struct addrinfo hints;
        ZeroMemory(&hints, sizeof(hints));

        // AF_INET определяет, что используется сеть для работы с сокетом
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM; // Задаем потоковый тип сокета
        hints.ai_protocol = IPPROTO_TCP; // Используем протокол TCP
        // Сокет биндится на адрес, чтобы принимать входящие соединения
        hints.ai_flags = AI_PASSIVE;

        // Инициализируем структуру, хранящую адрес сокета - addr.
        // HTTP-сервер будет висеть на заданном порту заданного хоста
        result = getaddrinfo(host, port, &hints, &addr);
        // result = getaddrinfo("0.0.0.0", port, &hints, &addr);
        // result = getaddrinfo("127.0.0.1", "8000", &hints, &addr);

        // Если инициализация структуры адреса завершилась с ошибкой,
        // выведем сообщением об этом и завершим выполнение программы 
        if (result != 0) {
            cerr << "getaddrinfo failed: " << result << "\n";
            WSACleanup(); // выгрузка библиотеки Ws2_32.dll
			system("pause");
            return 1;
        }

        // Создание сокета
        int listen_socket = socket(addr->ai_family, addr->ai_socktype,
            addr->ai_protocol);
        // Если создание сокета завершилось с ошибкой, выводим сообщение,
        // освобождаем память, выделенную под структуру addr,
        // выгружаем dll-библиотеку и закрываем программу
        if (listen_socket == INVALID_SOCKET) {
            cerr << "Error at socket: " << WSAGetLastError() << "\n";
            freeaddrinfo(addr);
            WSACleanup();
			system("pause");
            return 1;
        }


        // Привязываем сокет к IP-адресу
        result = bind(listen_socket, addr->ai_addr, (int)addr->ai_addrlen);

        // Если привязать адрес к сокету не удалось, то выводим сообщение
        // об ошибке, освобождаем память, выделенную под структуру addr.
        // и закрываем открытый сокет.
        // Выгружаем DLL-библиотеку из памяти и закрываем программу.
        if (result == SOCKET_ERROR) {
            cerr << "bind failed with error: " << WSAGetLastError() << "\n";
            freeaddrinfo(addr);
            closesocket(listen_socket);
            WSACleanup();
			system("pause");
            return 1;
        }


        // Инициализируем слушающий сокет
        if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
            cerr << "listen failed with error: " << WSAGetLastError() << "\n";
            closesocket(listen_socket);
            WSACleanup();
			system("pause");
            return 1;
        }

    
        cout << "Server Rex has been started on " << host << ".\n";
        cout << "Listen port " << port << "... ";

        const int max_client_buffer_size = 2048;
        char buf[max_client_buffer_size];

        long long unsigned int client_socket = INVALID_SOCKET;

        for (;;) {

            // Принимаем входящие соединения
            client_socket = accept(listen_socket, NULL, NULL);
            if (client_socket == INVALID_SOCKET) {
                cerr << "accept failed: " << WSAGetLastError() << "\n";
                closesocket(listen_socket);
                WSACleanup();
				system("pause");
                return 1;
            }
        
			// Принимаем входящее сообщение в буфер (buf)
            result = recv(client_socket, buf, max_client_buffer_size, 0);

            stringstream response; // сюда будет записываться ответ клиенту
            stringstream response_body; // тело ответа

            if (result == SOCKET_ERROR) {
                // ошибка получения данных
                cerr << "recv failed: " << result << "\n";
                closesocket(client_socket);
            } else if (result == 0) {
                // соединение закрыто клиентом
                cerr << "connection closed...\n";
            } else if (result > 0) {
                // Мы знаем фактический размер полученных данных, поэтому ставим метку конца строки
                // В буфере запроса.
                // (-4) поставил так как при считывании данных дублировалась строка 'cookies' 
                buf[result-4] = '\0';


                // Переданные данные от клиента записываем в строку JSON
                string json = Rex::getRequest(buf);

// на время убрал
                // mkdir("temp");
                string temp = "temp/rex.request.json";

                // и сохраняем в файл (почему-то пока только с файлом работает rapidjson)
// на время убрал
                // writeFile(temp, json);

                // запрашиваем поле path (путь в адресной строке браузера)
                string path = parserFile(temp, "path");    


                // буфер для перевода string в массив char
                char pathBuf[path.length()];                
                int pathLength = path.copy(pathBuf, path.length());
                // если в конце адреса нет слеша, то добавляем
                if (pathBuf[pathLength-1] != '/') path = path + "/";
               
                
                // запрашиваем поле method (GET PUT POST...)
                string method = parserFile(temp, "method");
                
                
                if (path != "/favicon.ico/") {
                    // в консоли выводим сообщение, кроме когда браузером запрашивается иконка
                    cout << "Request method: " << method << " " << path << "\n";
                                    

                    // Данные успешно получены
                    // формируем тело ответа (HTML)
                    string route = Rex::getRoutes(path);
                    // string route = parserFile("routes/route.json", path);
                    // если есть такая запись в файле route.json
                    if (route != "Error") {
                        string html;
                        // если view в корневом каталоге, то возвращаем только название html файла без '/'
                        if (view == "/") html = route;
                        // иначе полный путь к html файлу
                        else html = view + "/" + route;

                        // если такой файл существует
                        if (fileExist(html)) {
                            // считываем файл
                            response_body << readFile(html);
                        }else { // иначе создаём 'базовый' html
                            response_body << Rex::createIndexHtml(view, html);
                        }
                    }else { // вывод информации 'страница не найдена'
                        response_body << "<br><br><center>";
                        response_body << "<h1>Error 404. Page not found.</h1>";
                        response_body << "</center><br><smal>Server rex.</smal>";
                        response_body << "<br><hr>";
                    }                        

                    cout << "Listen port " << port << "... ";               

                    // Формируем весь ответ вместе с заголовками
                    response << "HTTP/1.1 200 OK\r\n"
                        << "Version: HTTP/1.1\r\n"
                        << "Content-Type: text/html; charset=utf-8\r\n"
                        << "Content-Length: " << response_body.str().length()
                        << "\r\n\r\n"
                        << response_body.str(); 

                    // Отправляем ответ клиенту с помощью функции send
                    result = send(client_socket, response.str().c_str(),
                        response.str().length(), 0);

                    if (result == SOCKET_ERROR) {
                        // произошла ошибка при отправле данных
                        cerr << "send failed: " << WSAGetLastError() << "\n";
                    }
                    
                }

                // Закрываем соединение к клиентом
                closesocket(client_socket);
            }


        }

        // Убираем за собой
        closesocket(listen_socket);
        freeaddrinfo(addr);
        WSACleanup();
		
		system("pause");

        return 0;
    }


    string parserConfig(string field) {    
        return parserFile("config/rex.config.json", field);
    }


    void createConfigFiles(void) {
        mkdir("config");
        string text = "{\n    \"view\": \"html\",\n    \"content-type\": \"text/html\",\n    \"host\": \"127.0.0.1\",\n    \"port\": \"8000\"\n}";
        writeFile("config/rex.config.json", text);

        return;
    }



    string getRequest(char *buf) {

        stringstream request;
        stringstream fin;
        fin << buf;        

        string METHOD;
        string PATH;
        string HTTP;
        string NULL_STRING;
        string temp;
        fin >> METHOD;
        fin >> PATH;
        fin >> HTTP;
        // fin >> NULL_STRING;

        request << "{\n    \"method\": \"" << METHOD << "\"";
        request << ",\n    \"path\": \"" << PATH << "\"";
        request << ",\n    \"http\": \"" << HTTP << "\"";
     
        int nLength;

        while(!fin.eof()) {
            fin >> temp;
            
            // .copy('куда копировать', 'до какого номера символа', ['от какого номера символа'])
            nLength = temp.copy(buf, temp.length()-1);
            buf[nLength] = '\0'; // завершаем строку в буфере

            request << ",\n    \"" << buf << "\": ";

            fin.getline(buf, 2048);
            buf[0] = '\"';

            if (buf[strlen(buf)-1] == '\r') buf[strlen(buf)-1] = '\0';

            request << buf << "\"";
        }
        request << "\n}";

        // writeFile("server_request.json", request.str());
        return request.str();
     
    }



    string getRoutes(string path) {

        if (fileExist("routes/route.json")) {

            return parserFile("routes/route.json", path);

        }else {
            string text = "{\n    \"/\": \"index.html\"\n}";
            mkdir("routes");
            writeFile("routes/route.json", text);

            if (path == "/") return "index.html";
        }    

        return "Error";        
    }


    string createIndexHtml(string dir, string html) {
        if (dir != "/") {
            char szBuf[dir.length()];    
            int nLength = dir.copy(szBuf, dir.length());
            szBuf[nLength] = '\0';
            mkdir(szBuf);
        }

        string text = "<html><head><title>Rex</title><meta content=\"text/html; charset=UTF-8\" http-equiv=\"Content-Type\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><style>body {display: flex;justify-content: center;align-items: center;min-height: 100vh;background: #f1f1f1;}</style></head><body><div class=\'h1\'><h1>Мой сайт на сервере Rex!</h1><small>Create yore file index.html!</small></div></body></html>";
        writeFile(html, text);

        return text;
    }



};


//=======================================================================
