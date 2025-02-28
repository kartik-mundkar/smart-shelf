// Import Mongoose
import mongoose from 'mongoose';

// Create a schema for load cell data
const weightSchema = new mongoose.Schema({
    weight: {
        type: Number,     // The weight measured by the load cell
        required: true    // This field is required
    },
    timestamp: {
        type: Date,       // The time when the weight was measured
        default: Date.now // Automatically set to the current date and time
    },
    unit: {
        type: String,     // Unit of measurement (e.g., kg, grams)
        default: 'g'     // Default unit is grams
    },
    sensorId: {
        type: String,     // Unique ID for the load cell sensor (if you are using multiple sensors)
        default: '0',     // Default unit is grams
        required: true    // Ensuring every document has a sensor ID
    }
});

// Create a model for the schema (this will create a collection named 'Weights' in MongoDB)
const Weight = mongoose.model('Weight', weightSchema);

// Export the model to use it in other parts of your application
export default Weight;
