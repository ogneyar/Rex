
#include <fstream> // работа с файлами
#include <iostream> // работа вводом/выводом данных
#include <sstream> // работа со стримами
#include <string> // работа со строками

#include <cstring> // работа со строками (strlen)

#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"

#include <exception> // исключения
// #include <stdexcept> // исключения

using namespace std;
using namespace rapidjson;


int writeFile(string, string);
string readFile(string);
string parserFile(string, string);
string parserJSON(string, string);
bool fileExist(string);
// const char *strToChar(string);

using namespace std;


int writeFile(string file, string text) {
    // создаём файл и привязываем его к объекту класс ofstream
    ofstream fout(file);
    if (!fout.is_open()) {// если файл не открыт
        cout << "File " << file << " not open for write!\n"; // сообщить об этом
        return -1;
    }
    // делаем запись в файл
    fout << text;
    // закрываем файл
    fout.close();
    return 0;
}


string readFile(string fileName) {
    int lenBuff = 1024;
    // буфер промежуточного хранения считываемого из файла текста
    char buff[lenBuff];
    stringstream response;
    // открыли файл для чтения
    ifstream fin(fileName);
    if (!fin.is_open()) {// если файл не открыт
        response << "File " << fileName << " not open for read!\n"; // сообщить об этом
        return response.str();
    }
    while (!fin.eof()) {
        // считали строку из файла
        fin.getline(buff, lenBuff);
        response << buff << endl;
    }
    // закрываем файл
    fin.close();
    return response.str();
}


string parserFile(string file, string field) {    
    if (fileExist(file)) {
        return parserJSON(readFile(file), field);
    }    
    return "Error";
}


string parserJSON(string json_str, string field) {
    char szBuf[json_str.length()];
    // .copy('куда копировать', 'до какого номера символа', ['от какого номера символа'])
    int nLength = json_str.copy(szBuf, json_str.length()-1);
    szBuf[nLength] = '\0'; // завершаем строку в буфере
    
    const char* json = szBuf;
    try {
        Document document;
        document.Parse(json);
        if (document.HasMember(field.c_str())) {

            return document[field.c_str()].GetString();

        }else throw runtime_error("Error, not key in JSON");

    }catch (runtime_error e) {
        cout << "Exception! " << e.what() << endl;
    }
    return "Error";
}


bool fileExist(string filePath) {
    bool isExist = false;
    ifstream fin(filePath.c_str());
 
    if(fin.is_open())
        isExist = true;
 
    fin.close();
    return isExist;
}
