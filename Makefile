all : benchmark

# FLAGS = -O3 -Wall -Wextra -Werror -std=c++2a
# FLAGS = -O0 -g -Wall -Wextra -Werror -std=c++2a -fsanitize=address
# COMPILER = clang++
COMPILER = g++

BOOST_INCLUDE = -I/opt/homebrew/Cellar/boost/1.86.0/include
BOOST_LIBS = -L/usr/local/lib -lboost_system -lboost_filesystem

FLAGS = -O3 -Wall -Wextra -Werror -std=c++2a $(BOOST_INCLUDE)

benchmark : benchmark_message_queue.cpp RingBuffer_v1.hpp RingBuffer_v2.hpp BlockingQueue.hpp Makefile defs.hpp
	$(COMPILER) $(FLAGS)  benchmark_message_queue.cpp -o benchmark
