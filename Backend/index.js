// Import necessary modules
import express from 'express';
import { connect } from 'mongoose';
import dotenv from 'dotenv';
import cors from 'cors';
import Weight from './weight.model.js';
import Temperature from './temperature.model.js';
import Humidity from './humidity.model.js';

// Load environment variables from .env
dotenv.config();

// Initialize express app
const app = express();

// Middleware
app.use(express.json());
app.use(express.urlencoded({ extended: true }));
app.use(cors());

// Environment variables
const PORT = process.env.PORT || 3000;
const MONGO_URI = process.env.MONGO_URI;

// Connect to MongoDB
connect(MONGO_URI, { useNewUrlParser: true, useUnifiedTopology: true })
    .then(() => console.log('Connected to MongoDB'))
    .catch(err => console.error('MongoDB Connection Error:', err));


// Route to fetch the latest 20 records from each collection
app.get('/data', async (req, res) => {
    try {
        const weightData = await Weight.find().sort({ timestamp: -1 }).limit(20);
        const tempData = await Temperature.find().sort({ timestamp: -1 }).limit(20);
        const humidityData = await Humidity.find().sort({ timestamp: -1 }).limit(20);

        const combinedData = weightData.map((weightRecord, index) => ({
            timestamp: weightRecord.timestamp,
            weight: weightRecord.weight,
            temperature: tempData[index]?.temperature || null,
            humidity: humidityData[index]?.humidity || null
        }));

        res.status(200).json(combinedData);
    } catch (error) {
        console.error('Error fetching data:', error);
        res.status(500).json({ error: 'Failed to fetch data' });
    }
});

// POST route to handle data from ESP32 with authentication
app.post('/data', async (req, res) => {
    const { weight, temperature, humidity } = req.body;

    if (weight === undefined || temperature === undefined || humidity === undefined) {
        return res.status(400).json({ error: 'Weight, temperature, and humidity data are required' });
    }

    try {
        await Promise.all([
            new Weight({ weight }).save(),
            new Temperature({ temperature }).save(),
            new Humidity({ humidity }).save()
        ]);

        res.status(200).json({ message: 'Data inserted successfully' });
    } catch (error) {
        console.error('Error inserting data:', error);
        res.status(500).json({ error: 'Failed to insert data' });
    }
});

// Start the server
app.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
});
