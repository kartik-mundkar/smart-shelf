// Import Mongoose
import mongoose from 'mongoose';

// Create a schema for temperature data
const temperatureSchema = new mongoose.Schema({
    temperature: {
        type: Number,     // Temperature value
        required: true    // This field is required
    },
    timestamp: {
        type: Date,       // Time of measurement
        default: Date.now // Automatically set to the current date and time
    },
    sensorId: {
        type: String,     // Unique ID for the temperature sensor
        default: '0',     // Default sensor ID
        required: true    // Ensuring every document has a sensor ID
    }
});

// Create a model for the schema
const Temperature = mongoose.model('Temperature', temperatureSchema);

// Export the model
export default Temperature;
