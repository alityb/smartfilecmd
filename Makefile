CXX = g++-11
CXXFLAGS = -std=c++20 -O2 -Wall
LIBS = -lstdc++fs
SOURCES = cpp_backend/main.cpp cpp_backend/actions.cpp cpp_backend/utils.cpp
TARGET = smartfilecmd

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

cpp_performance_test: cpp_performance_test.cpp cpp_backend/actions.cpp cpp_backend/utils.cpp
	$(CXX) $(CXXFLAGS) -Icpp_backend -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET) cpp_performance_test

.PHONY: clean cpp_performance_test
