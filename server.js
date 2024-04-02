// server.js
import express from "express";
import mqtt from "mqtt";
import axios from "axios";

const app = express();
const port = process.env.PORT || 3000;

// Middleware for parsing JSON data
app.use(express.json());

// Start the server
app.listen(port, () => {
  console.log(`Server is running on port ${port}`);
});

// Define route to serve the webpage
app.get("/", (req, res) => {
  res.render("index");
});

// Function to calculate running average
let runningTemperatureSum = 0;
let runningHumiditySum = 0;
let count = 0;

function calculateRunningAverage(newTemperature, newHumidity) {
  // Increment count
  count++;

  // Update running sum for temperature and humidity
  runningTemperatureSum += newTemperature;
  runningHumiditySum += newHumidity;

  // Calculate running average
  const runningTemperatureAverage = runningTemperatureSum / count;
  const runningHumidityAverage = runningHumiditySum / count;

  console.log("Running Temperature Average:", runningTemperatureAverage);
  console.log("Running Humidity Average:", runningHumidityAverage);
}

// Route to handle MQTT data
app.post("/mqtt", (req, res) => {
  const { temperature, humidity } = req.body;

  // Calculate running average
  calculateRunningAverage(temperature, humidity);

  // Save data to database (implementation needed)

  res.status(200).send("Data received successfully");
});

const mqttClient = mqtt.connect("mqtt://localhost"); // Replace with your MQTT broker URL

// Subscribe to the topic where ESP32 publishes data
mqttClient.on("connect", () => {
  mqttClient.subscribe("esp32/data", (err) => {
    if (err) {
      console.error("Error subscribing to topic:", err);
    } else {
      console.log("Subscribed to topic");
    }
  });
});

// Handle incoming MQTT messages
mqttClient.on("message", (topic, message) => {
  const data = JSON.parse(message.toString());
  const { temperature, humidity } = data;

  // Forward data to route for handling
  axios
    .post("http://localhost:3000/mqtt", { temperature, humidity })
    .then((response) => {
      console.log("Data forwarded successfully:", response.data);
    })
    .catch((error) => {
      console.error("Error forwarding data:", error);
    });
});
