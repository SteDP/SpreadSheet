#pragma once

#include <string>
#include <vector>
#include <utility>
#include <set>

typedef unsigned int uint;

class CSheet
{
private:
    class Cell // тип, описывающий каждую €чейку
    {
    public:
        // тип, описывающий тип конкретной €чейки
        enum CellType
        {
            NumericCell,
            SymbolicCell,
            ErrorCell,
            ExpressionCell
        };

        // переменна€, хран€ща€ тип €чейки
        CellType type;

        std::string symbols;
        int number;

        // дефолтный конструктор
        Cell()
        {
            type = ErrorCell;
            symbols = "#EMPTYCEL";
        }

        ~Cell()
        {

        }

        Cell(const Cell &o)
        {
            type = o.type;
            if (type == NumericCell)
                number = o.number;
            else
                symbols = o.symbols;
        }

        void set(int num)
        {
            type = NumericCell;
            number = num;
        }

        void set(CellType t, const std::string &v)
        {
            type = t;
            symbols = v;
        }
    };

    // все €чейки данной таблицы
    std::vector<std::vector<Cell>> cells;

    // исходники данных таблицы
    std::vector<std::string> sources;

    int rows, cols;

    /* 
        обсчитать заданную €чейку
        undefCells - массив, хран€щий идентификаторы всех €чеек, которые ждут результата
    */
    bool computeCell(int row, int col, std::set<uint> & undefCells = std::set<uint>());

public:
    CSheet(int rows, int cols);
    ~CSheet();

    // добавить исходный код очередной €чейки
    void addSource(const std::string &source);

    // вычислить конечный рехультат дл€ таблицы
    void computeSheet();

    // получить текстовое представление дл€ таблицы
    std::string getResult();
};

