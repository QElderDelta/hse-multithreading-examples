# Проверка работы condition_variable (hw3)

## Сборка

cmake -S . -B build
cmake --build build --target futex_condition_variable

## Запуск

./build/futex/futex_condition_variable

## Вывод программы

Worker woke up, shared_value = 42