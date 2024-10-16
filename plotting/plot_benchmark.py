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
Blocking Queue      : 1.95       2.51       1.48       0.85       0.57       0.38       0.36       0.32       0.32       0.27       0.26       0.23       0.22       0.22       0.20       0.26       0.25       0.22       0.25       0.25       0.23       0.24       0.23       0.22       0.23       0.22       0.21       0.20       0.18       0.17       0.15       0.14       0.12       0.11       0.10       0.09       0.08       0.07       0.07       0.06       0.06       0.06       0.06       0.05       0.05       0.05
Custom SPMC Queue   : 7.90       8.19       10.41      10.57      11.80      11.50      12.08      11.58      11.78      10.48      10.63      9.81       9.12       9.17       8.73       8.45       7.86       7.68       7.02       6.09       5.94       5.92       6.01       5.81       5.63       5.78       5.31       5.74       2.02       5.60       1.72       5.04       5.23       5.16       1.63       1.77       1.75       1.61       1.79       1.96       1.86       1.69       1.71       1.91       1.71       1.66
Boost Lock-Free     : 3.61       2.32       2.13       1.93       1.57       1.49       1.37       1.35       1.19       1.12       1.14       0.98       0.98       0.99       0.93       0.91       0.88       0.83       0.87       0.76       0.79       0.77       0.72       0.73       0.73       0.73       0.75       0.72       0.72       0.69       0.71       0.69       0.68       0.69       0.67       0.68       0.68       0.67       0.66       0.66       0.66       0.65       0.65       0.65       0.64       0.64
Ring Buffer v1      : 189.75     366.26     487.60     506.77     759.21     720.22     892.07     810.31     938.57     992.04     938.26     1018.19    1115.87    1059.84    1454.02    1139.30    1150.47    1265.57    1268.94    1170.53    1236.12    1220.15    1529.97    1507.61    1574.45    378.25     530.45     409.41     452.73     437.90     1614.92    1894.24    466.68     468.04     609.05     538.57     501.07     400.94     412.83     571.99     624.28     601.65     587.82     597.39     626.74     630.97
Ring Buffer v2      : 182.59     473.82     785.79     1006.08    1083.04    1224.62    1398.22    1826.83    1977.13    2274.40    2556.72    2726.15    3348.61    3607.38    3817.23    4347.07    4442.73    4440.05    4944.46    5201.84    5460.59    6036.36    6922.65    6143.70    6956.28    6854.69    5737.33    5791.14    5618.43    6316.35    5704.12    5898.80    5443.82    5941.10    4952.00    5950.59    5740.73    4941.52    5219.26    5616.76    5217.06    5321.23    5456.70    5265.68    5010.75    3459.20
"""

# Apple M2
apple_str = """
Num Consumers       : 1.00       2.00       3.00       4.00       5.00       6.00       7.00       8.00
Blocking Queue      : 6.71       3.09       2.61       2.13       1.87       1.82       1.80       1.75
Custom SPMC Queue   : 22.60      15.42      4.94       5.46       4.37       3.64       2.88       2.79
Boost Lock-Free     : 9.08       7.55       2.96       2.55       2.27       2.26       1.99       1.84
Ring Buffer v1      : 63.50      104.09     68.27      74.44      98.80      86.87      94.37      98.14
Ring Buffer v2      : 680.07     1093.78    1622.70    2206.94    3159.47    3593.11    4133.09    4361.88
"""

# df = make_frame(apple_str)
# df = make_frame(intel_str1)
df = make_frame(intel_str)

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
