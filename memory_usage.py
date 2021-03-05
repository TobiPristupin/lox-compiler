from itertools import accumulate
from operator import add
import matplotlib.pyplot as plt

log_file = open("heap_log.txt")
entries = log_file.readlines()
log_file.close()

entries, microseconds = entries[:-1], entries[-1]
entries = list(map(lambda entry: entry.replace("\n", "").split(), entries))

times = [entry[3] for entry in entries]
formatted_times = [time for time in times]
memory = [int(entry[2]) / 1000 if entry[0] == "Allocated" else -int(entry[2]) / 1000 for entry in entries]
cumulative_memory = list(accumulate(memory, add))

fig, ax = plt.subplots()
ax.plot(formatted_times, cumulative_memory)
ax.set_xlabel("Time")
ax.set_ylabel("Memory (Kilobytes)")
ax.xaxis.set_major_locator(plt.MaxNLocator(3))
plt.show()

