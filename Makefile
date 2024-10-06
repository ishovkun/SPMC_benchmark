all : benchmark

FLAGS = -O3 -Wall -Wextra -Werror -std=c++2a
# FLAGS = -O0 -g -Wall -Wextra -Werror -std=c++2a -fsanitize=address
COMPILER = clang++

benchmark : benchmark_message_queue.cpp RingBuffer_v1.hpp RingBuffer_v2.hpp BlockingQueue.hpp Makefile
	$(COMPILER) $(FLAGS)  benchmark_message_queue.cpp -o benchmark
