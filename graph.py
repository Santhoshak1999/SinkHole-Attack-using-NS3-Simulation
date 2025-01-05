import pandas as pd
import matplotlib.pyplot as plt

# Load the energy log CSV file into a DataFrame
df = pd.read_csv("/home/santhoshak/NS-3/ns-allinone-3.36.1/ns-3.36.1/energy_log.csv")

# Plot the energy consumption over time for each node
plt.figure(figsize=(12, 6))

# Iterate over each unique node ID
for node_id in df['Node ID'].unique():
    node_data = df[df['Node ID'] == node_id]
    plt.plot(node_data['Time (s)'], node_data['Remaining Energy (J)'], label=f'Node {node_id}')

# Customize the plot
plt.xlabel('Time (s)')
plt.ylabel('Remaining Energy (J)')
plt.title('Remaining Energy Over Time for Each Node')
plt.legend()
plt.grid(True)

# Save the plot as a PNG file
plt.savefig("energy_plot.png")

# Display the plot
plt.show()
