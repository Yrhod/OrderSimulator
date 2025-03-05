import pytest
from testcontainers.kafka import KafkaContainer
from testcontainers.postgres import PostgresContainer
import subprocess
import time

@pytest.fixture(scope="session")
def kafka_container():
    """Запускает контейнер Kafka для тестов."""
    with KafkaContainer("confluentinc/cp-kafka:latest") as kafka:
        kafka.start()
        time.sleep(5)  
        yield kafka


@pytest.fixture(scope="session")
def postgres_container():
    """Запускает контейнер PostgreSQL для тестов."""
    with PostgresContainer("postgres:13") as postgres:
        yield postgres
