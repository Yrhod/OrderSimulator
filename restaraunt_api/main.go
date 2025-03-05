package main

import (
    "bytes"
    "encoding/json"
    "fmt"
    "math/rand"
    "net/http"
    "time"
)

type OrderRequest struct {
    Item     string `json:"item"`
    Quantity int    `json:"quantity"`
    ID       int    `json:"id"`
}

type OrderResponse struct {
    Status  string `json:"status"`
    OrderID int    `json:"order_id"`
}

type CompleteRequest struct {
    OrderID int    `json:"order_id"`
    Status  string `json:"status"`
}

func processOrder(w http.ResponseWriter, r *http.Request) {
    fmt.Println("Received request at /process for order %d\n", r.Header.Get("X-Order-ID"))
    if r.Method != http.MethodPost {
        http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
        return
    }

    var req OrderRequest
    if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
        http.Error(w, "Invalid request body", http.StatusBadRequest)
        return
    }

    resp := OrderResponse{
        Status:  "processing",
        OrderID: req.ID,
    }

    w.Header().Set("Content-Type", "application/json")
    w.Header().Set("Connection", "keep-alive")
    w.Header().Set("Content-Length", fmt.Sprintf("%d", len(fmt.Sprintf(`{"status":"processing","order_id":%d}`, req.ID))))
    json.NewEncoder(w).Encode(resp)
    fmt.Printf("Sent response for order %d: %+v\n", req.ID, resp)

    go notifyComplete(req.ID)
}

func notifyComplete(orderID int) {
    delay := time.Duration(rand.Intn(5)) * time.Second
    time.Sleep(delay)
    completeReq := CompleteRequest{
        OrderID: orderID,
        Status:  "completed",
    }
    data, _ := json.Marshal(completeReq)
    resp, err := http.Post("http://localhost:8082/complete", "application/json", bytes.NewBuffer(data))
    for i := 0; i < 3 && err != nil; i++ { 
        time.Sleep(time.Second) 
        resp, err = http.Post("http://localhost:8082/complete", "application/json", bytes.NewBuffer(data))
    }
    if err != nil {
        fmt.Printf("Failed to notify Order Processor for order %d after retries: %v\n", orderID, err)
        return
    }
    defer resp.Body.Close()
    fmt.Printf("Order %d completed after %v\n", orderID, delay)
}

func main() {
    http.HandleFunc("/process", processOrder)
    fmt.Println("Restaurant API running on port 8083...")
    if err := http.ListenAndServe(":8083", nil); err != nil {
        fmt.Println("Error:", err)
    }
}