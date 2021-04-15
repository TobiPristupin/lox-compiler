import matplotlib.pyplot as plt
import sys


'''format of log file: 
first line: unix timestamp of program's start in nanoseconds
(De)allocated [object_type] [object_name] [object_byte_size] [bytes_currently_allocated] [unix_timestamp_of_allocation_nanoseconds]
last line: unix timestamp of program's end in nanoseconds
'''
log_file = open(sys.argv[1])
lines = log_file.readlines()
log_file.close()

start_time, entries, end_time = int(lines[0]), lines[1:-1], int(lines[-1])
entries = list(map(lambda entry: entry.replace("\n", "").split(), entries))
bytes_allocated_mb = [int(entry[4]) / 1e6 for entry in entries]
allocation_timestamps = [(int(entry[5]) - start_time) for entry in entries]
print(bytes_allocated_mb)

fig, ax = plt.subplots()
ax.plot(allocation_timestamps, bytes_allocated_mb, linestyle='--', marker='o', color='b')
ax.set_xlabel("Time")
ax.set_ylabel("Memory (Megabytes)")
plt.show()

