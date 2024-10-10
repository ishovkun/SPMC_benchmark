#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
from io import StringIO

def make_frame(data_str: str):
    # Split data into lines
    lines = data_str.strip().split('\n')

    # Initialize data list
    data_list = []

    # Parse each line
    for line in lines[:]:
        queue_type, values = line.split(':')
        data_list.append([queue_type.strip()] + [float(x) for x in values.strip().split()])

    # Create DataFrame
    df = pd.DataFrame(data_list)
    df.set_index(0, inplace=True)
    df = df.T
    return df

intel_str = """
Num Consumers       : 1.00       2.00       3.00       4.00       5.00       6.00       7.00       8.00       9.00       10.00      11.00      12.00      13.00      14.00      15.00      16.00      17.00      18.00      19.00      20.00      21.00      22.00      23.00      24.00      25.00      26.00      27.00      28.00      29.00      30.00      31.00      32.00      33.00      34.00      35.00      36.00      37.00      38.00      39.00      40.00      41.00      42.00      43.00      44.00      45.00      46.00
Blocking Queue      : 2.48       2.67       1.42       0.90       0.53       0.40       0.37       0.34       0.29       0.28       0.26       0.23       0.23       0.23       0.24       0.22       0.24       0.23       0.27       0.27       0.25       0.23       0.21       0.22       0.21       0.22       0.20       0.20       0.19       0.17       0.16       0.14       0.13       0.11       0.10       0.09       0.08       0.08       0.07       0.07       0.06       0.06       0.06       0.06       0.06       0.05
Custom SPMC Queue   : 4.92       5.50       5.64       5.98       6.12       6.30       6.20       6.29       6.23       6.16       6.17       6.10       6.07       5.94       5.83       5.84       5.61       5.59       4.97       4.98       5.10       5.00       5.05       5.02       4.93       4.88       4.90       4.83       4.78       1.44       4.62       4.64       4.20       4.49       1.51       4.27       4.23       1.56       1.48       1.49       1.47       1.48       1.54       1.38       1.45       1.62
Boost Lock-Free     : 3.53       2.83       2.33       1.81       1.58       1.49       1.30       1.22       1.14       1.09       1.03       0.96       0.98       0.89       0.84       0.83       0.80       0.78       0.78       0.75       0.72       0.71       0.70       0.70       0.70       0.68       0.68       0.67       0.66       0.66       0.65       0.64       0.64       0.65       0.63       0.64       0.64       0.63       0.63       0.63       0.61       0.62       0.61       0.61       0.61       0.61
Ring Buffer v1      : 15.08      16.58      18.04      21.05      23.24      22.57      23.93      24.03      23.99      24.43      23.22      22.58      23.49      23.05      22.65      21.19      20.48      21.29      20.82      20.61      20.83      20.58      20.90      20.05      20.03      21.33      21.70      21.51      20.95      21.77      21.75      21.35      21.20      22.09      21.99      22.07      22.20      22.42      22.23      22.39      22.26      22.33      22.20      22.67      22.82      22.77
Ring Buffer v2      : 20.13      25.04      28.89      28.98      30.86      33.87      32.11      34.06      32.30      29.35      31.06      29.60      29.40      29.03      29.67      28.55      28.96      27.81      26.97      26.27      26.30      26.45      26.14      25.93      28.45      28.59      28.70      28.88      29.20      28.84      27.20      29.09      29.37      29.07      29.74      29.49      29.50      29.57      29.67      29.93      29.79      29.95      29.89      28.92      30.29      30.30
"""

# Apple M2
apple_str = """
Num Consumers       : 1.00       2.00       3.00       4.00       5.00       6.00       7.00       8.00
Blocking Queue      : 7.69       2.26       2.57       2.09       1.80       1.83       1.82       1.85
Custom SPMC Queue   : 8.63       15.92      4.57       4.07       3.64       3.34       2.76       2.35
Boost Lock-Free     : 8.86       6.21       2.64       2.43       2.18       2.27       2.25       2.12
Ring Buffer v1      : 61.77      48.36      55.62      41.18      43.20      27.91      25.16      19.80
Ring Buffer v2      : 320.93     96.63      41.35      43.25      35.61      24.22      25.33      21.94
"""

df = make_frame(apple_str)
# df = make_frame(intel_str)

print(df)
df.plot(x="Num Consumers", marker='o')
plt.ylabel("Reads / μs")
plt.xlabel("Num Consumers")
# plt.xscale('log')
plt.yscale('log')

x = df["Num Consumers"]
labels = [f'{int(i)}' for i in x]
ax = plt.gca()
# ax.set_xticks(x)
# ax.set_xticklabels(labels)

plt.show()
