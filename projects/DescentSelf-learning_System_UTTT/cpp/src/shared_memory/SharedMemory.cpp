// SharedMemory.cpp
#include "shared_memory/SharedMemory.h"
#include "shared_memory/python_config.h"
#include <pybind11/embed.h>  // для py::scoped_interpreter
#include <cstring>           // memset
#include <iostream>


// -----------------------------------------------------
// Статические поля
// -----------------------------------------------------
bool SharedMemory::pythonInitialized_ = false;
bool SharedMemory::classRegistered_ = false;
py::module_ SharedMemory::moduleCppBindings_;

// -----------------------------------------------------
// Конструктор
// -----------------------------------------------------
SharedMemory::SharedMemory(std::size_t paramSampleLength)
    : sampleLength(paramSampleLength) {
    // 1) Инициализируем Python (однократно)
    ensurePythonInitialized();

    // 2) Регистрируем класс SharedMemory (однократно)
    ensureClassRegistered();

    // 3) Выделяем память под наши массивы
    sampleMainChannels = new uint8_t[sampleLength * 9 * 9 * 6];
    sampleMacroChannels = new uint8_t[sampleLength * 3 * 3 * 2];
    sampleValues = new float[sampleLength];
    intVars = new int[intVarsCount];
    floatVars = new float[floatVarsCount];

    // 4) Обнулим всё для наглядности
    std::memset(sampleMainChannels, 0, sampleLength * 9 * 9 * 6 * sizeof(uint8_t));
    std::memset(sampleMacroChannels, 0, sampleLength * 3 * 3 * 2 * sizeof(uint8_t));
    std::memset(sampleValues, 0, sampleLength * sizeof(float));
    std::memset(intVars, 0, intVarsCount * sizeof(int));
    std::memset(floatVars, 0, floatVarsCount * sizeof(float));

    // 5) Импортируем Python-скрипт
    {
        py::module_ sys = py::module_::import("sys");
        // Пример для Windows: подставьте нужные пути
        sys.attr("path").attr("append")(std::string(python_config::PYTHON_PATH) + ".venv\\Lib\\site-packages");
        sys.attr("path").attr("append")(std::string(python_config::PYTHON_PATH) + "descent");

        shared_memory_script_ = py::module_::import("shared_memory_script");
    }

    // 6) Вызываем init_arrays(self), чтобы Python сохранил ссылки на массивы
    shared_memory_script_.attr("init_arrays")(py::cast(this));

    // 7) Сохраняем ссылки на функции Do, Evaluate, Learn
    do_func_ = shared_memory_script_.attr("Do");
    evaluate_func_ = shared_memory_script_.attr("Evaluate");
    learn_func_ = shared_memory_script_.attr("Learn");
}

// -----------------------------------------------------
// Деструктор
// -----------------------------------------------------
SharedMemory::~SharedMemory() {
    delete[] sampleMainChannels;
    delete[] sampleMacroChannels;
    delete[] sampleValues;
    delete[] intVars;
    delete[] floatVars;
    // Python-интерпретатор не останавливаем
}


// -----------------------------------------------------
// Геттеры для NumPy (без копий)
// -----------------------------------------------------
py::array_t<uint8_t> SharedMemory::get_sample_main_channels() {
    // 4D: [sampleLength, 9, 9, 6]
    std::vector<ssize_t> shape{
        (ssize_t) sampleLength, 9, 9, 6
    };
    // strides в байтах
    std::vector<ssize_t> strides{
        9 * 9 * 6 * (ssize_t) sizeof(uint8_t),
        9 * 6 * (ssize_t) sizeof(uint8_t),
        6 * (ssize_t) sizeof(uint8_t),
        1 * (ssize_t) sizeof(uint8_t)
    };

    py::capsule cap(sampleMainChannels, [](void *) {
    });
    return py::array_t<uint8_t>(shape, strides, sampleMainChannels, cap);
}

py::array_t<uint8_t> SharedMemory::get_sample_macro_chennels() {
    // 4D: [sampleLength, 3, 3, 2]
    std::vector<ssize_t> shape{
        (ssize_t) sampleLength, 3, 3, 2
    };
    std::vector<ssize_t> strides{
        3 * 3 * 2 * (ssize_t) sizeof(uint8_t),
        3 * 2 * (ssize_t) sizeof(uint8_t),
        2 * (ssize_t) sizeof(uint8_t),
        1 * (ssize_t) sizeof(uint8_t)
    };

    py::capsule cap(sampleMacroChannels, [](void *) {
    });
    return py::array_t<uint8_t>(shape, strides, sampleMacroChannels, cap);
}

py::array_t<float> SharedMemory::get_sample_values() {
    std::vector<ssize_t> shape{(ssize_t) sampleLength};
    std::vector<ssize_t> strides{(ssize_t) sizeof(float)};

    py::capsule cap(sampleValues, [](void *) {
    });
    return py::array_t<float>(shape, strides, sampleValues, cap);
}

py::array_t<int> SharedMemory::get_int_vars() {
    std::vector<ssize_t> shape{(ssize_t) intVarsCount};
    std::vector<ssize_t> strides{(ssize_t) sizeof(int)};

    py::capsule cap(intVars, [](void *) {
    });
    return py::array_t<int>(shape, strides, intVars, cap);
}

py::array_t<float> SharedMemory::get_float_vars() {
    std::vector<ssize_t> shape{(ssize_t) floatVarsCount};
    std::vector<ssize_t> strides{(ssize_t) sizeof(float)};

    py::capsule cap(floatVars, [](void *) {
    });
    return py::array_t<float>(shape, strides, floatVars, cap);
}

// -----------------------------------------------------
// ensurePythonInitialized()
// -----------------------------------------------------
void SharedMemory::ensurePythonInitialized() {
    if (!pythonInitialized_) {
        static py::scoped_interpreter guard{}; // живёт до конца процесса
        pythonInitialized_ = true;
        std::cout << "[SharedMemory] Python interpreter initialized.\n";
    }
}

// -----------------------------------------------------
// ensureClassRegistered()
// -----------------------------------------------------
void SharedMemory::ensureClassRegistered() {
    if (!classRegistered_) {
        // 1) Создаём виртуальный модуль "cpp_bindings"
        moduleCppBindings_ = py::module_::create_extension_module(
            "cpp_bindings",
            "doc: internal module for SharedMemory",
            new PyModuleDef()
        );

        // 2) Регистрируем класс SharedMemory
        py::class_<SharedMemory>(moduleCppBindings_, "SharedMemory")
                .def(py::init<std::size_t>(), py::arg("sample_length"))

                // Три публичных метода (связаны с питон-функциями)
                .def("Do", &SharedMemory::Do)
                .def("Evaluate", &SharedMemory::Evaluate)
                .def("Learn", &SharedMemory::Learn)

                // Геттеры (массивы)
                .def("get_sample_main_channels", &SharedMemory::get_sample_main_channels)
                .def("get_sample_macro_chennels", &SharedMemory::get_sample_macro_chennels)
                .def("get_sample_values", &SharedMemory::get_sample_values)
                .def("get_int_vars", &SharedMemory::get_int_vars)
                .def("get_float_vars", &SharedMemory::get_float_vars)

                // Поле
                .def_readonly("sample_length", &SharedMemory::sampleLength);

        // 3) Добавляем модуль в sys.modules
        py::module_ sys = py::module_::import("sys");
        sys.attr("modules")["cpp_bindings"] = moduleCppBindings_;

        classRegistered_ = true;
        std::cout << "[SharedMemory] Class Registered in 'cpp_bindings'.\n";
    }
}
