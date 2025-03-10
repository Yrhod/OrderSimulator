# Order Simulator / Симулятор Заказов

Добро пожаловать в **Order Simulator** — мощный проект для симуляции обработки заказов в реальном времени. Проект объединяет C++ (с поддержкой многопоточности и мьютексов для эффективной обработки), Go (для API) и Kafka (для асинхронного обмена данными), а также использует PostgreSQL для хранения. Это отличный пример микросервисной архитектуры с акцентом на асинхронность и параллелизм.

---

## О проекте 

Этот проект разработан для демонстрации работы системы обработки заказов в реальном времени. Он включает три основных компонента:
- **Order Receiver** (C++) — принимает заказы через HTTP, использует многопоточность и мьютексы для параллельной обработки, отправляет их в Kafka.
- **Order Processor** (C++) — подписывается на топик `orders` в Kafka, обрабатывает заказы с использованием потоков и мьютексов, обновляет статус в базе данных.
- **Restaurant API** (Go) — предоставляет REST API для взаимодействия с системой.

Проект использует `vcpkg` для управления зависимостями C++ и `CMake` для сборки, а также `Makefile` для упрощения работы.

---

## Установка

1. **Установите зависимости**:
   - Docker (для Kafka).
   - Python 3.x и `pytest` (`pip install pytest`).
   - Go (последняя версия).
   - CMake (последняя версия).
   - vcpkg (установите с [официального сайта](https://vcpkg.io/en/getting-started.html) и выполните `vcpkg install crow cppkafka`).
   - MSVC или MinGW (для компиляции C++).

2. **Клонируйте репозиторий**:
git clone https://github.com/Yrhod/OrderSimulator.git
cd OrderSimulator

3. **Соберите проект**:
make build

4. **Запустите проект**:
make run

---

## Использование 

- **`make build`**: Скомпилировать все компоненты проекта.
- **`make run`**: Запустить `Order Receiver`, `Order Processor` и `Restaurant API` с Kafka.
- **`make test`**: Выполнить Python-тесты.
- **`make clean`**: Очистить сгенерированные файлы.
- **`make help`**: Показать доступные команды.

---

## Зависимости 

- **C++**:
- Crow (веб-фреймворк, через `vcpkg`).
- cppkafka (клиент Kafka, через `vcpkg`).
- Boost (через `vcpkg`, для поддержки потоков и мьютексов).
- **Go**: Стандартная библиотека.
- **Python**: `pytest` для тестов.
- **Kafka**: Для запуска Kafka.

---

## Документация 

#### Архитектура
- **Order Receiver**: Принимает заказы через HTTP, отправляет их в Kafka.
- **Order Processor**: Подписывается на топик `orders` в Kafka, обрабатывает заказы с использованием потоков и мьютексов для синхронизации доступа к данным, обновляет PostgreSQL.
- **Restaurant API**: Обеспечивает REST API для создания заказов.

#### Конфигурация
- Kafka запускается на порту 9093.
- PostgreSQL используется для хранения статусов заказов (настройка через `testcontainers` или вручную).
- Многопоточность в C++ реализована с использованием std::thread для эффективного управления потоками и мьютексами.

#### Пример использования
1. Отправь заказ через API:
curl -X POST http://localhost:8081/order -H "Content-Type: application/json" -d '{"item": "Pizza", "quantity": 1}'
2. Проверь статус в базе данных через `order_processor`.

#### Технические детали (C++)
- Используются `std::thread` и `std::mutex` для управления потоками и синхронизации.
