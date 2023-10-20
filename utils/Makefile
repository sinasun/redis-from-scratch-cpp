CXX = g++
CXXFLAGS = -Wall

SRCDIR = .
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(SRCS:.cpp=.o)

LIBRARY = libutils.a

.PHONY: all clean

all: $(LIBRARY)

$(LIBRARY): $(OBJS)
		ar rcs $@ $^

%.o: %.cpp
		$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
		rm -f $(OBJS) $(LIBRARY)

