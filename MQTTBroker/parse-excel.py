import pandas as pd
import subprocess
import json

# Read Excel file
data_frame = pd.read_excel('/home/tuyenld/mqtt-python/Silly graduation cap (Responses).xlsx')

# Access a specific column by name
# column_data = data_frame['What is your first name/nickname? (less than 10 characters)']

# Access a specific column by index
# column_index = 1  # What is your first name/nickname? (less than 10 characters)
# your_names = data_frame.iloc[:, column_index]

# column_index = 2  # What is your message? (less than 30 characters)
# your_messages = data_frame.iloc[:, column_index]

# column_index = 3  # Choose your emoji :))
# your_emojis = data_frame.iloc[:, column_index]

# Print the column data
# print(type(your_names))
# print(your_names)
# print(your_messages)
# print(your_emojis)

command_pub = "mosquitto_pub -u piZero -P pihat -h 104.248.243.162 -t 'hat' -m "


# Iterate over rows
for row in data_frame.values:
    # Access row data
    your_names = row[1] # What is your first name/nickname? (less than 10 characters)
    your_messages = row[2] # What is your message? (less than 30 characters)
    
    json_object = {}
    # Remove non-ASCII characters
    if not pd.isnull (row[3]):
        your_emojis = row[3] # Choose your emoji :))
        your_emojis = your_emojis.encode("ascii", "ignore").decode("ascii")
        json_object["image_path"] = your_emojis
    
    json_object["first_line"] = your_names
    json_object["second_line"] = your_messages
    json_string = json.dumps(json_object)
    json_string = json_string.replace('"', '\\"')
    command = command_pub + '"' + json_string + '"'

    # escaped_command = command.replace('"', '\\"')
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    # print(result.stdout)
    exit_code = result.returncode
    print(f"Exit code should be zero : {exit_code}")
    print(command)