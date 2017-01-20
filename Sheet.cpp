#include "stdafx.h"
#include "Sheet.h"
#include <ctype.h>
#include <stack>
#include <sstream>
#include <math.h>

// безопасное сложение
bool safeAdd(int & a, int b)
{
    // результат запомним в 64-битном типе
    long long r = a + b;

    // вычисялем выражение
    a += b;

    // вернем результат проверки на переполнение
    return r >= INT_MIN && r <= INT_MAX;
}

bool safeMul(int & a, int b)
{
    // результат вычислений запомним в 64-битном типе
    long long r = a * b;

    // проводим вычисления
    a *= b;

    // вернем результат проверки на переполнение
    return r >= INT_MIN && r <= INT_MAX;
}

bool safeDiv(int & a, int b)
{
    // если просят делить на ноль
    if (b == 0)
        return false; // то вернем ошибку

    a /= b; // иначе проводим вычисления

    return true; // и возвращаем истину
}

bool CSheet::computeCell(int row, int col, std::set<uint> & undefCells)
{
    // считаем идентификатор ячейки
    uint cellId = row * cols + col;

    // если эта ячейка уже ждет результата
    if (undefCells.count(cellId) != 0)
        return false; // то присутствует рекурсивное использование ячеек, т.е. ошибка

    // получаем ссылку на вычисляемую ячейку
    Cell & cell = cells[row][col];

    // если ячейка уже посчитана и содержит ошибку
    if (cell.type == Cell::ErrorCell)
        return false;

    // если ячейка посчитана и не содержит ошибку
    if (cell.type != Cell::ExpressionCell)
        return true;

    // берем ссылку на формулу в ячейке
    std::string & expr = cell.symbols;

    // если ячейка содержит N, то она должна быть помечена пустой
    if (expr[0] == 'N' && expr.length() == 1)
    {
        cell.type = Cell::ErrorCell;
        cell.symbols = "#EMPTYCEL";

        return false;

    }else if (expr[0] == '\'') // если первый символ - одинарная кавычка
    {
        // отрезаем этот символ
        expr = expr.substr(1);

        // помечаем ячейку символьной
        cell.type = Cell::SymbolicCell;

        // возвращаем истину
        return true;
    }
    else if (expr[0] == '=' || isdigit(expr[0])) // если первый символ - это знак "равно" или цифра
    {
        enum ResultType
        {
            NumericResult,
            SymbolicResult,
            EmptyResult
        } resultType = EmptyResult; // сначала делаем результат "пустым"

        // переменные для хранения промежуточных результатов
        std::string  symData;
        int num = 0, numData;
        std::stringstream ss;

        enum ActionType
        {
            ParseNumber,
            ParseIdentifier,
            ParseAll
        } act = ParseAll; // текущее действие - "парсинг всего"

        enum SignType
        {
            SignMinus,
            SignPlus,
            SignDiv,
            SignMul
        } lastSign = SignPlus; // последний оператор - плюс

        // если первой в выражении была цифра, то добавляем пробел в начало строки
        if (isdigit(expr[0]))
            expr = " " + expr;

        // пробегаемся по символам выражения
        for (auto it = expr.begin() + 1; ; it++)
        {
            // если наткнулись на конец строки или оператор
            if (it == expr.end() || *it == '+' || *it == '-' || *it == '*' || *it == '/') {
                
                bool changeActFlag = true; // флаг, говорящий, стоит ли в конце ветки менять действие на "парсинг всего"

                // смотрим, что за действие является текущим
                switch (act)
                {
                case ParseNumber: // если парсинг числа
                    if (resultType == EmptyResult) // если результат пустой
                        resultType = NumericResult; // то обозначаем, что теперь численный

                    if (resultType == NumericResult) // если результат численный
                    {
                        switch (lastSign) // смотрим последний знак
                        {
                        case SignPlus: // если плюс
                            if (!safeAdd(num, numData)) // если не удалось безопасно сложить
                            {
                                cell.type = Cell::ErrorCell; // ячейка становится ошибочной
                                cell.symbols = "#OVERFLOW"; // ошибка - переполнение
                                return false; // вернем ошибку
                            }
                            break;
                        case SignMinus: // если минус
                            if (!safeAdd(num, -numData)) // если не удалось безопасно вычесть
                            {
                                cell.type = Cell::ErrorCell; // ошибка
                                cell.symbols = "#OVERFLOW"; // переполнение
                                return false; // выход
                            }
                            break;
                        case SignMul: // умножение
                            if (!safeMul(num, numData)) // если не удалось
                            {
                                cell.type = Cell::ErrorCell; // ошибка
                                cell.symbols = "#OVERFLOW"; // переполнение
                                return false; // выход
                            }
                            break;
                        case SignDiv: // деление
                            if (!safeDiv(num, numData)) // если не удалось
                            {
                                cell.type = Cell::ErrorCell; // ошибка
                                cell.symbols = "#OVERFLOW"; // переполнение
                                return false; // выход
                            }
                            break;
                        }
                    }
                    else { // если значение символьное
                        switch (lastSign)
                        {
                        case SignPlus: // последний знак - плюс
                            ss << numData; // добавляем к строке последнее число
                            break;
                        default: // если последний знак какой-то другой
                            cell.type = Cell::ErrorCell; // ошибка
                            cell.symbols = "#WRONGOPR"; // неверный оператор
                            return false; // выход
                        }
                    }
                    break;
                case ParseIdentifier: // если парсили идентификатор
                {
                    // вычисляем целевые строку и столбец
                    int tC = tolower(symData[0]) - 'a', tR = numData - 1; 

                    // если столбец вышел за пределы
                    if (tC < 0 || tC >= cols)
                    {
                        cell.type = Cell::ErrorCell; // ошибка
                        cell.symbols = "#WRONGCOL"; // неверный столбец
                        return false; // выход
                    }
                    // если строка вышла за пределы
                    if (tR < 0 || tR >= rows)
                    {
                        cell.type = Cell::ErrorCell; // ошибка
                        cell.symbols = "#WRONGROW"; // неверная строка
                        return false; // выход
                    }
                    // если порядок
                    undefCells.insert(cellId); // пихаем в "неопределенные ячейки" идентификатор текущей
                    if (!computeCell(tR, tC, undefCells)) // если не удалось посчитать целевую ячейку
                    {
                        cell.type = Cell::ErrorCell; // ошибка
                        cell.symbols = "#UNDEFIND"; // неопределенный результат
                        undefCells.erase(cellId); // удаляем идентификатор
                        return false; // выход
                    }
                    undefCells.erase(cellId); // удаляем идентификатор
                    Cell & tCell = cells[tR][tC]; // получаем ссылку на целевую ячейку

                    switch (tCell.type) // определяем тип целевой ячейки
                    {
                    case Cell::NumericCell: // если числовой
                        changeActFlag = false; // не менять действие на "парсить всё"
                        act = ParseNumber; // меняем действие на "парсить число"
                        numData = tCell.number; // запоминаем число из целевой ячейки
                        it -= 1; // откатываемся на один символ назад
                        break;
                    case Cell::SymbolicCell: // если символьтный тип
                        if (resultType == EmptyResult) // если результат был пустым
                            resultType == SymbolicResult; // теперь он станет символьным
                        else if (resultType == NumericResult) // если результат был числовым
                        {
                            resultType = SymbolicResult; // делаем символьным

                            ss << num; // пихаем число в строку
                        }

                        ss << tCell.symbols; // прибавляем к строке символы из целевой ячейки

                    }
                    break;
                }
                }
                if (it == expr.end()) // если это конец строки
                {
                    if (resultType == NumericResult) // если результат численный
                    {
                        cell.type = Cell::NumericCell; // помечаем ячейку численной
                        cell.number = num; // результат - число
                    }
                    else if (resultType == SymbolicResult) { // если результат символьный
                        cell.type = Cell::SymbolicCell; // ячейка символьная
                        cell.symbols = ss.str(); // результат - строка
                    }
                    else { // если пустой результат
                        cell.type = Cell::ErrorCell; // ошибка
                        cell.symbols = "#EMPTYCEL"; // пустая ячейка
                        return false; // выход с ошибкой
                    }
                    return true; // выход с положительным результатом
                }
                switch (*it) // если не конец строки, то определям текущий знак
                {
                case '+': // плюс
                    lastSign = SignPlus; // запоминаем
                    break;
                case '-': // минус
                    lastSign = SignMinus; // запоминаем
                    break;
                case '*': // умножение
                    lastSign = SignMul; // запоминаем
                    break;
                case '/': // деление
                    lastSign = SignDiv; // запоминаем
                    break;
                }

                if(changeActFlag) // если надо сбросить действие
                    act = ParseAll; // сбрасываем на "парсить всё"
            }
            else if (isdigit(*it)) // если встретилась цифра
            {
                switch(act) // смотрим действие
                {
                    case ParseAll: // парсить всё
                        numData = *it - '0'; // промежуточное число равно данной цифре
                        act = ParseNumber; // парсить число
                        break;
                    case ParseNumber: // парсить число
                    case ParseIdentifier: // или идентификатор
                        if (INT_MAX / 10 <= numData) // если пришли к пределу инта
                        {
                            cell.type = Cell::ErrorCell; // ошибка
                            cell.symbols = "#OVERFLOW"; // переполнение
                            return false; // выход
                        }
                        numData = numData * 10 + (*it - '0'); // к промежуточному добавляем цифру
                        break;
                }
            } else if (isalpha(*it)) // если встретили символ алфавита
            {
                switch (act) // смотрим действие
                {
                case ParseAll: // парсить всё
                    symData = *it; // промежуточная строка равна этому символу
                    act = ParseIdentifier; // парсить идентификатор
                    numData = 0; // промежуточное число равно нулю
                    break;
                case ParseNumber: // парсить число
                    cell.type = Cell::ErrorCell; // ошибка
                    cell.symbols = "#WRONGCHR"; // неверный символ
                    return false; // выход
                case ParseIdentifier: // парсить идентификатор
                    cell.type = Cell::ErrorCell; // ошибка
                    cell.symbols = "#WRONGCOL"; // неверный столбец
                    return false; // выход
                }
            } else { // если встретился неподдерживаемый символ
                cell.type = Cell::ErrorCell; // ошибка
                cell.symbols = "#WRONGCHR"; // неверный символ
                return false; // выход
            }
        }
    }
        
    return false; // выход с ошибкой
}


CSheet::CSheet(int r, int c)
{
    rows = r; // запоминаем количество строк
    cols = c; // столбцов

    cells.resize(r); // растягиваем хранилище ячеек до количества строк

    for (auto it = cells.begin(); it != cells.end(); it++) // потом пробегаем по каждой строке
        it->resize(c); // растягиваем до количества столбцов
}

CSheet::~CSheet()
{
}

void CSheet::addSource(const std::string & source) // добавить исходник
{
    if (sources.size() < rows * cols) // если ячеек не хватает
        sources.push_back(source); // добавляем
}

void CSheet::computeSheet()
{

    int r = 0, c = 0; // значения для текущей строки и столбца

    for (auto s : sources) // пробегаемся по исходникам
    {
        cells[r][c].set(Cell::ExpressionCell, s); // задаем для текущей ячейки текущее выражение

        c += 1; // переходим к следующему столбцу
        r += c / cols; // если надо, переходим к следующей строке
        c %= cols; // номер столбца всегда меньше количеств столбцов
    }

    for (r = 0; r < rows; r++) // пробегаемся по строкам
    {
        for (c = 0; c < cols; c++) // пробегаемся по столбцам
        {
            if (cells[r][c].type == Cell::ExpressionCell) // если в данной ячейке выражение
                computeCell(r, c); // вычислим его
        }
    }

}

std::string CSheet::getResult()
{
    std::stringstream ss; // строковый поток для формирования результата

    for (int r = 0; r < rows; r++) // пробегаемся по строкам
    {
        for (int c = 0; c < cols; c++) // по столбцам
        {
            ss.width(10); // ширина - 10
            if (cells[r][c].type == Cell::NumericCell) // если ячейка численная
                ss << cells[r][c].number; // выводим число
            else // иначе
                ss << cells[r][c].symbols; // символы
        }
        ss << std::endl; // в конце строки переход на следующую строку
    }
    
    return ss.str(); // вернем результирующую строку
}
