#include <iostream>
#include <math.h>
#include <iomanip>
#include <fstream>

/**
* \mainpage
* \brief ПИД-регулятор
* \author Дмитаница Влад
*/
using namespace std;

/**
* \class model
* \brief Абстрактный класс, который будет использоваться для построения линейных и нелинейных моделей
*/
class model
{
public:
    /**
    * \details абстрактная функция для переопределения в дочерних классах
    */
    virtual float equation(float y_t, float u_t) = 0;
    virtual ~model() = default;
};

/**
* \class LinearMod
* \brief Класс, представляющий линейную модель контролируемого объекта
* \details Дочерний класс, который расширяет класс model
*/
class LinearMod : public model
{
private:
    float a;
    float b;
    float y_t1;

public:
    /**
    * \details конструктор для LinearMod
    * \param a, b - просто коэффициенты
    * \param y_t1 - температура на выходе
    */
    LinearMod(float a, float b, float y_t1)
        : a(a), b(b), y_t1(y_t1)
    {
    }

    /**
    * \details функция для вычисления температуры по линейной модели
    */
    float equation(float y_t, float u_t) override
    {
        y_t1 = a * y_t + b * u_t;
        return y_t1;
    }

    ~LinearMod() override = default;
};

/**
* \class NonLinearMod
* \brief Класс, представляющий нелинейную модель контролируемого объекта
* \details Дочерний класс, который расширяет класс model
*/
class NonLinearMod : public model
{
private:
    float a; ///< a, b, c, d - просто коэффициенты
    float b;
    float c;
    float d;
    float y_t0 = 0; ///< y_t0 - предыдущее(начальное) значение температуры
    float y_t1; ///< текущее значение температуры на выходе
    float u_t0 = 0; ///< u_t0 - переменная для предыдущего значения тепла
public:
    /**
   * \details конструктор для NonLinearMod
   * \param a, b, c, d просто коэффициенты
   */
    NonLinearMod(float a, float b, float c, float d, float y_t1)
    : a(a), b(b), c(c), d(d), y_t1(y_t1)
    {
    }
    /**
    * \details функция для вычисления температуры по нелинейной модели
    */
    float equation(float y_t, float u_t) override
    {
        y_t1 = a * y_t - b * static_cast<float>(pow(y_t0, 2)) + c * u_t + d * sin(u_t0);
        u_t0 = u_t;
        y_t0 = y_t;
        return y_t1;
    }
    
    ~NonLinearMod() override = default;
};

/**
* \class regulator
* \brief Класс для реализации регулятора
*/
class regulator
{
private:
    float T;
    float T0;
    float TD;
    float K;
    float u = 0;

public:
    /**
    * \details конструктор для regulator
    * \param K,T0,TD,T слева-направо: коэффициент передачи, шаг, постоянная диференцирования, постоянная интегрирования
    */
    regulator(float T, float T0, float TD, float K)
    : T(T), T0(T0), TD(TD), K(K)
    {
    }
    /**
    * \details функция для подсчёта управляющей переменной
    * \param e, em1, em2 значения текущей, прошлой и позапрошлой ошибок
    */
    float temperature(float e, float em1, float em2) {
        float q0 = K * (1 + TD / T0);
        float q1 = -K * (1 + 2 * TD / T0 - T0 / T);
        float q2 = K * TD / T0;
        u += q0 * e + q1 * em1 + q2 * em2;
        return u;
    }
};

/**
* \brief Функция, которая моделирует ПИД-регулятор
* \details функция имитирует работу ПИД-регулятора
* \param w желаемое значение
* \param *reg указатель на экземпляр regulator
* \param *md указатель на экземпляр model
* \param y0 начальное значение y
*/
void PIDregulator(float w, float y0, regulator& reg, model& md) {
    ofstream fout;
    fout.open("E:\\PID.txt", ios_base::out | ios_base::app);
    if (fout.is_open()) {
        float em1 = 0;
        float em2 = 0;
        float y = y0;
        for (int i = 0; i < 100; i++) {
            float e;
            float u;
            e = w - y;
            u = reg.temperature(e, em1, em2);
            y = md.equation(y0, u);
            fout << "E=" << e << " Y=" << y << " U=" << u << endl;
            em2 = em1;
            em1 = e;
        }
    }
    fout.close();
}

/**
* \brief Функция main, создаём экземпляры всех классов и вызываем функцию PIDregulator
*/
int main() {
    setlocale(0, "");
    ofstream fout;
    fout.open("E:\\PID.txt", ios_base::out | ios_base::app);
    if (fout.is_open()) {
        fout << "Linear Model:" << endl;
        LinearMod l(0.333f, 0.667f, 1);
        regulator regl(10, 10, 50, 0.1f);
        PIDregulator(5, 2, regl, l);
        fout << "NonLinear Model:" << endl;
        NonLinearMod nl(1.0f, 0.0033f, 0.525f, 0.525f, 1.0f);
        regulator regnl(10, 10, 50, 0.1f);
        PIDregulator(5, 2, regnl, nl);
    }
    cout << "Данные были сохранены в файл PID.txt" << endl;
    return 0;
}