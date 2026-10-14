#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <conio.h>
#include <windows.h>
#include <locale>
#include <cstdio>
#include <fstream>
#include <cstring>
#include <ctime>

struct Node
{
    int x;
    int y;
    Node* prev;

    Node(int _x, int _y)
    {
        x = _x;
        y = _y;
        prev = nullptr;
    }

    Node(int _x, int _y, Node* _prev)
    {
        x = _x;
        y = _y;
        prev = _prev;
    }
};

struct NodeData
{
    int x;
    int y;
};

struct Food
{
    int x;
    int y;
    bool presence;

    Food(int _x, int _y)
    {
        x = _x;
        y = _y;
        presence = true;
    }
};

struct SaveData
{
    int score;
    short direction;
    int food_x;
    int food_y;
    bool food_presence;
    int snake_length;
};

struct Record
{
    char name[50];
    int score;
};

class Snake
{
public:
    Node* beg = nullptr;
    Node* end = nullptr;
    int len;
    short direction;
    int score;

    Snake(int x_beg, int y_beg, int x_end, int y_end, int _direction)
    {
        beg = new Node(x_beg, y_beg);
        end = new Node(x_end, y_end, beg);

        len = 2;
        direction = _direction;
        score = 0;
    }

    ~Snake()
    {
        while (end != nullptr)
        {
            Node* temp = end;
            end = end->prev;
            delete temp;
        }
    }

    void Move(Food* food)
    {
        Node* new_beg = new Node(beg->x, beg->y);

        switch (direction)
        {
            case 0:
                new_beg->y -= 1;
                break;

            case 1:
                new_beg->x += 1;
                break;

            case 2:
                new_beg->y += 1;
                break;

            case 3:
                new_beg->x -= 1;
                break;
        }

        beg->prev = new_beg;
        beg = new_beg;
        if (food->presence && food->x == beg->x && food->y == beg->y)
        {
            food->presence = false;
            score += 1;
            len++;
        }
        else
        {
            Node* oper = end;
            end = end->prev;

            delete oper;
        }
    }

    void SetDirection(char key)
    {
        short new_dir;
        switch (key)
        {
            case 'w':
                new_dir = 0;
                break;

            case 'd':
                new_dir = 1;
                break;

            case 's':
                new_dir = 2;
                break;

            case 'a':
                new_dir = 3;
                break;

            default:
                return;
        }

        if (std::abs(direction - new_dir) == 2) return;
        direction = new_dir;
    }

    bool CheckCollision(int width, int height)
    {
        if (beg->x < 0 || beg->x >= width || beg->y < 0 || beg->y >= height)
        {
            return true;
        }

        Node* temp = beg->prev;
        while (temp != nullptr)
        {
            if (beg->x == temp->x && beg->y == temp->y)
            {
                return true;
            }
            temp = temp->prev;
        }

        return false;
    }
};

bool Rasstanovka(char(&area)[10][10], Snake* snake, Food* food)
{
    for (int y = 0; y < 10; y++)
    {
        for (int x = 0; x < 10; x++)
        {
            area[y][x] = '.';
        }
    }

    Node* slider = snake->end;
    while (slider != nullptr)
    {
        if (slider->y < 0 || slider->y > 9 || slider->x < 0 || slider->x > 9) {
            std::cout << slider->y << " " << slider->x;
            return false;
        }
        if (area[slider->y][slider->x] == '@') {
            return false;
        }

        area[slider->y][slider->x] = '@';
        slider = slider->prev;
    }

    if (food->presence)
    {
        area[food->y][food->x] = '*';
        return true;
    }

    while (!(food->presence))
    {
        for (int y = rand() % 10; y < 10; y++)
        {
            for (int x = 0; x < 10; x++)
            {
                if (area[y][x] == '.' && rand() % 2)
                {
                    area[y][x] = '*';
                    food->x = x;
                    food->y = y;
                    food->presence = true;
                    return true;
                }
            }
        }
    }
    return true;
}

void Otobrazhenie(char(&area)[10][10])
{
    for (int y = 0; y < 10; y++)
    {
        for (int x = 0; x < 10; x++)
        {
            std::cout << area[y][x];
        }
        std::cout << "\n";
    }
}

void sohranitSostoyanieZmei(Snake* snake, Food* food, const char* filename)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "ошибка при открытии сохранений\n";
        return;
    }

    SaveData data;
    data.score = snake->score;
    data.direction = snake->direction;
    data.food_x = food->x;
    data.food_y = food->y;
    data.food_presence = food->presence;
    data.snake_length = snake->len;

    file.write(reinterpret_cast<char*>(&data), sizeof(SaveData));

    std::cout << "длина " << snake->len << "\n";

    NodeData* nodes = new NodeData[snake->len];
    int index = 0;

    Node* temp = snake->end;
    while (temp != nullptr)
    {
        nodes[index].x = temp->x;
        nodes[index].y = temp->y;
        std::cout << index << "x=" << nodes[index].x << "y=" << nodes[index].y << "\n";
        index++;
        temp = temp->prev;
    }

    for (int i = 0; i < snake->len / 2; i++)
    {
        NodeData temp_node = nodes[i];
        nodes[i] = nodes[snake->len - 1 - i];
        nodes[snake->len - 1 - i] = temp_node;
    }

    std::cout << "узлы " << index << "\n";

    file.write(reinterpret_cast<char*>(nodes), sizeof(NodeData) * snake->len);
    delete[] nodes;

    file.close();
    std::cout << "игра сохранена\n";
}

Snake* zagruzitSostoyanieZmei(Food* food, const char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "ошибка при открытии файла загркзки\n";
        return nullptr;
    }

    SaveData data;
    file.read(reinterpret_cast<char*>(&data), sizeof(SaveData));

    food->x = data.food_x;
    food->y = data.food_y;
    food->presence = data.food_presence;

    NodeData* nodes = new NodeData[data.snake_length];
    file.read(reinterpret_cast<char*>(nodes), sizeof(NodeData) * data.snake_length);

    file.close();

    for (int i = 0; i < data.snake_length; i++)
    {
        std::cout << i << "x=" << nodes[i].x << " y=" << nodes[i].y << "\n";
    }

    if (data.snake_length < 2)
    {
        delete[] nodes;
        return nullptr;
    }

    Snake* snake = new Snake(nodes[0].x, nodes[0].y, nodes[1].x, nodes[1].y, data.direction);
    snake->score = data.score;
    snake->len = data.snake_length;

    for (int i = 2; i < data.snake_length; i++)
    {
        Node* new_node = new Node(nodes[i].x, nodes[i].y, snake->end);
        snake->end->prev = new_node;
        snake->end = new_node;
    }

    delete[] nodes;

    std::cout << "игра загружена\n";
    return snake;
}

void sohranitRekord(const char* name, int score, const char* filename)
{
    std::ofstream file(filename, std::ios::binary | std::ios::app);
    if (!file.is_open())
    {
        std::cout << "ошибка при открытии рекордов\n";
        return;
    }

    Record record;
    strncpy(record.name, name, 49);
    record.name[49] = '\0';
    record.score = score;

    file.write(reinterpret_cast<char*>(&record), sizeof(Record));
    file.close();
}

void pokazatRekordy(const char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "нет рекордов\n";
        return;
    }

    file.seekg(0, std::ios::end);
    int file_size = file.tellg();
    int num_records = file_size / sizeof(Record);
    file.seekg(0, std::ios::beg);

    Record record;
    std::cout << "=== Рекорды ===\n";
    for (int i = 0; i < num_records; i++)
    {
        file.read(reinterpret_cast<char*>(&record), sizeof(Record));
        std::cout << record.name << " - " << record.score << "\n";
    }
    file.close();
}

void Menu()
{
    std::cout << "=== Меню ===\n";
    std::cout << "1. Новая игра\n";
    std::cout << "2. Загрузить игру\n";
    std::cout << "3. Таблица рекордов\n";
    std::cout << "4. Выход\n";
    std::cout << "Выберите действие: ";
}

void GameLoop(Snake* snake, Food* food)
{
    char area[10][10];
    bool game_over = false;

    while (!game_over)
    {
        if (!Rasstanovka(area, snake, food)) break;

        std::cout << "счёт: " << snake->score << "\n";
        std::cout << "для сохранения нажмите 'z'\n";
        Otobrazhenie(area);

        Sleep(500);

        if (_kbhit() == 1)
        {
            char key = _getch();
            if (key == 'z')
            {
                sohranitSostoyanieZmei(snake, food, "savegame.bin");
                std::cout << "нажмите любую клавишу\n";
                _getch();
            }
            else
            {
                snake->SetDirection(key);
            }
        }

        snake->Move(food);

        if (snake->CheckCollision(10, 10))
        {
            game_over = true;
        }

        system("cls");
    }

    std::cout << "Конец. Счёт - " << snake->score << "\n";

    char name[50];
    std::cout << "введите имя для рекордов ";
    std::cin >> name;
    sohranitRekord(name, snake->score, "records.bin");
}

int main()
{
    setlocale(LC_ALL, "RUS");
    srand(static_cast<unsigned int>(time(nullptr)));

    while (true)
    {
        system("cls");
        Menu();

        int choice;
        std::cin >> choice;

        if (choice == 1)
        {
            Snake snake = Snake(7, 1, 8, 1, 3);
            Food food = Food(1, 1);
            GameLoop(&snake, &food);
        }
        else if (choice == 2)
        {
            std::ifstream checkFile("savegame.bin");
            if (!checkFile.good())
            {
                std::cout << "нет сохранений\n";
                Sleep(2000);
            }
            else
            {
                checkFile.close();
                Food food = Food(0, 0);
                Snake* snake = zagruzitSostoyanieZmei(&food, "savegame.bin");
                if (snake != nullptr)
                {
                    GameLoop(snake, &food);
                }
            }
        }
        else if (choice == 3)
        {
            system("cls");
            pokazatRekordy("records.bin");
            std::cout << "нажмите любую клавишу чтобы вернуться\n";
            _getch();
        }
        else if (choice == 4)
        {
            break;
        }
        else
        {
            std::cout << "нет\n";
            Sleep(1000);
        }
    }

    return 0;
}