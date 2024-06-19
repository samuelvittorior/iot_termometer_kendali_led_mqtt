const WebSocket = require("ws");
const mqtt = require("mqtt");

// Create a WebSocket server
const wss = new WebSocket.Server({ port: 8777 });

// Connect to the MQTT broker
const mqttClient = mqtt.connect("mqtt://usm.revolusi-it.com", {
  username: "usm",
  password: "usmjaya24",
});

// Subscribe to the MQTT topic
mqttClient.on("connect", () => {
  mqttClient.subscribe("test/test", (err) => {
    if (!err) {
      console.log("Subscribed to test/test");
    } else {
      console.error("Failed to subscribe:", err);
    }
  });
});

// Handle incoming MQTT messages
mqttClient.on("message", (topic, message) => {
  if (topic === "test/test") {
    const data = message.toString();
    console.log(`Received data: ${data}`);

    // Forward the data to all connected WebSocket clients
    wss.clients.forEach((client) => {
      if (client.readyState === WebSocket.OPEN) {
        client.send(data);
      }
    });
  }
});

// Handle WebSocket server events
wss.on("connection", (ws) => {
  console.log("New WebSocket connection established");

  ws.on("message", (message) => {
    console.log(`Received message from WebSocket client: ${message}`);

    // Parse the message and publish it to the MQTT broker
    const data = JSON.parse(message);
    if (data.nim === "G.231.21.0187" && data.messages) {
      mqttClient.publish("test/test", JSON.stringify(data));
    }
  });

  ws.on("close", () => {
    console.log("WebSocket connection closed");
  });
});

console.log("WebSocket server is running on ws://localhost:8777");
