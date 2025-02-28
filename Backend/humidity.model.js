// Import Mongoose
import mongoose from 'mongoose';

// Create a schema for humidity data
const humiditySchema = new mongoose.Schema({
    humidity: {
        type: Number,     // Humidity value
        required: true    // This field is required
    },
    timestamp: {
        type: Date,       // Time of measurement
        default: Date.now // Automatically set to the current date and time
    },
    sensorId: {
        type: String,     // Unique ID for the humidity sensor
        default: '0',     // Default sensor ID
        required: true    // Ensuring every document has a sensor ID
    }
});

// Create a model for the schema
const Humidity = mongoose.model('Humidity', humiditySchema);

// Export the model
export default Humidity;
