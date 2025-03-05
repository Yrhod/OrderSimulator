import pytest
from unittest.mock import Mock, patch
import requests
import json

def test_order_receiver_functional():
    with patch('requests.post') as mock_receiver:
        mock_receiver_response = Mock()
        mock_receiver_response.status_code = 200
        mock_receiver.return_value = mock_receiver_response

        order_data = {"item": "Laptop", "quantity": 1}
        response = requests.post("http://localhost:8081/order", json=order_data, timeout=10)
        assert response.status_code == 200
        mock_receiver.assert_called_once()

def test_order_processor_functional():
    with patch('requests.post') as mock_restaurant:
        mock_restaurant_response = Mock()
        mock_restaurant_response.status_code = 200
        mock_restaurant_response.text = '{"status": "processing"}'
        mock_restaurant.return_value = mock_restaurant_response

        order_data = {"item": "Laptop", "quantity": 1, "id": 1}
        response = requests.post("http://localhost:8083/process", json=order_data, timeout=10)
        assert response.status_code == 200
        assert json.loads(response.text)["status"] == "processing"
        mock_restaurant.assert_called_once()

    with patch('requests.post') as mock_complete:
        mock_complete_response = Mock()
        mock_complete_response.status_code = 200
        mock_complete.return_value = mock_complete_response

        complete_data = {"order_id": 1, "status": "completed"}
        response = requests.post("http://localhost:8082/complete", json=complete_data, timeout=10)
        assert response.status_code == 200
        mock_complete.assert_called_once()