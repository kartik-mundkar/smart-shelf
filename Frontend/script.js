const dataUrl = 'http://localhost:3000/data'; // Backend URL to fetch the data


// Function to fetch data and update the charts
async function updateCharts() {
  try {
    const response = await fetch(dataUrl);
    if (!response.ok) {
      console.error('Error fetching data:', response.status);
      throw new Error(`Error fetching data: ${response.status}`);
    }
    
    const data = await response.json();

    // Process the data
    const timestamps = data.map(item => new Date(item.timestamp).toLocaleTimeString());
    const weights = data.map(item => item.weight);
    const temperatures = data.map(item => item.temperature);
    const humidities = data.map(item => item.humidity);

    // Create the plotly data structure for Weight chart
    const weightData = {
      x: timestamps,
      y: weights,
      type: 'scatter',
      mode: 'lines+markers',
      name: 'Weight (g)',
      line: {color: 'blue'}
    };

    // Create the plotly data structure for Temperature chart
    const temperatureData = {
      x: timestamps,
      y: temperatures,
      type: 'scatter',
      mode: 'lines+markers',
      name: 'Temperature (°C)',
      line: {color: 'red'}
    };

    // Create the plotly data structure for Humidity chart
    const humidityData = {
      x: timestamps,
      y: humidities,
      type: 'scatter',
      mode: 'lines+markers',
      name: 'Humidity (%)',
      line: {color: 'green'}
    };

    // Dark theme layout configuration
    const darkLayout = {
      plot_bgcolor: '#2e2e2e', // Background color of the plot
      paper_bgcolor: '#1f1f1f', // Background color of the paper
      font: {
        color: 'white' // Font color for all the text
      },
      xaxis: {
        title: 'Time',
        color: 'white', // Axis line color
        tickcolor: 'white' // Tick marks color
      },
      yaxis: {
        title: 'Value',
        color: 'white', // Axis line color
        tickcolor: 'white' // Tick marks color
      },
      title: {
        font: {
          color: 'white' // Title font color
        }
      }
    };

    // Update the charts with the new data and dark theme layout
    Plotly.newPlot('weight-chart', [weightData], {
      ...darkLayout,
      title: 'Weight Over Time',
      yaxis: {title: 'Weight (g)'}
    });

    Plotly.newPlot('temperature-chart', [temperatureData], {
      ...darkLayout,
      title: 'Temperature Over Time',
      yaxis: {title: 'Temperature (°C)'}
    });

    Plotly.newPlot('humidity-chart', [humidityData], {
      ...darkLayout,
      title: 'Humidity Over Time',
      yaxis: {title: 'Humidity (%)'}
    });

  } catch (error) {
    console.error('Error fetching data:', error);
  }
}

// Update the charts every 10 seconds
setInterval(updateCharts, 1000);
updateCharts(); // Initial call to display the charts
