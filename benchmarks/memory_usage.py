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
print(end_time - start_time)
entries = list(map(lambda entry: entry.replace("\n", "").split(), entries))
#following lists are in megabytes
allocations, sweeps, deallocations = [[], []], [[], []], [[], []] #first array element has x values, second has y values
for entry in entries:
    megabytes = int(entry[4]) / 1e6
    timestamp = int(entry[5]) - start_time
    if entry[0] == "Allocated":
        allocations[0].append(timestamp)
        allocations[1].append(megabytes)
    elif entry[0] == "Sweeped":
        sweeps[0].append(timestamp)
        sweeps[1].append(megabytes)
    elif entry[0] == "Deallocated":
        deallocations[0].append(timestamp)
        deallocations[1].append(megabytes)

plt.style.use('ggplot')

fig, ax = plt.subplots()

#ax.plot(allocations[0] + sweeps[0] + deallocations[0], allocations[1] + sweeps[1] + deallocations[1], "--", color="grey")

ax.scatter(allocations[0], allocations[1], label="Allocation", color="blue", s=75)
ax.scatter(sweeps[0], sweeps[1], label="GC Deletion", color="green", s=75)
ax.scatter(deallocations[0], deallocations[1], label="Deallocation (Program Terminates)", color="red", s=75)

ax.legend()
ax.set_xlabel("Time")
ax.set_ylabel("Memory (Megabytes)")
plt.show()

