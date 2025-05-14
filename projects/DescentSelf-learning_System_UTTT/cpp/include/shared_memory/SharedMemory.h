// SharedMemory.h
#pragma once

#include <cstddef>
#include <cstdint>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

class SharedMemory {
public:
    // -------------------------------------------------------
    // ПОЛЯ
    // -------------------------------------------------------
    const std::size_t sampleLength;

    // Основные массивы
    uint8_t *sampleMainChannels;    // размер: sampleLength * 9 * 9 * 6
    uint8_t *sampleMacroChannels;   // размер: sampleLength * 3 * 3 * 2
    float *sampleValues;          // размер: sampleLength
    int *intVars;               // размер: 11
    float *floatVars;             // размер: 8

private:
    // Числа элементов в intVars / floatVars
    static constexpr std::size_t intVarsCount = 11;
    static constexpr std::size_t floatVarsCount = 10;

    // Python-модуль (shared_memory_script)
    py::object shared_memory_script_;

    // Ссылки на Python-функции, чтобы вызывать их быстро
    py::object do_func_;
    py::object evaluate_func_;
    py::object learn_func_;

public:
    // -------------------------------------------------------
    // КОНСТРУКТОР / ДЕСТРУКТОР
    // -------------------------------------------------------
    explicit SharedMemory(std::size_t paramSampleLength);

    ~SharedMemory();

    // -------------------------------------------------------
    // Методы, вызывающие Python-функции
    // -------------------------------------------------------
    inline void Do() {
        do_func_();
    }

    inline void Evaluate() {
        evaluate_func_();
    }

    inline void Learn() {
        learn_func_();
    }

    // -------------------------------------------------------
    // Геттеры массивов (возвращают NumPy-массивы без копий)
    // -------------------------------------------------------
    py::array_t<uint8_t> get_sample_main_channels();

    py::array_t<uint8_t> get_sample_macro_chennels();

    py::array_t<float> get_sample_values();

    py::array_t<int> get_int_vars();

    py::array_t<float> get_float_vars();

private:
    // -------------------------------------------------------
    // Статические (общие для всех объектов) вещи
    // -------------------------------------------------------
    static bool pythonInitialized_;
    static bool classRegistered_;
    static py::module_ moduleCppBindings_;

    // Инициализация Python (однократно)
    static void ensurePythonInitialized();

    // Регистрация класса SharedMemory (однократно)
    static void ensureClassRegistered();
};
