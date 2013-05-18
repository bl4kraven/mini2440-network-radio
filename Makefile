HOST=arm-linux-
SRC_DIR=.

CPPFLAGS+=-Wall -O2
CPPFILES=$(shell find $(SRC_DIR)  -maxdepth 1 -name "*.cpp")
CPPOBJS=$(CPPFILES:.cpp=.o)
TARGET=net_radio
LIBS+=

all:$(TARGET) 

-include $(addsuffix /*.d, $(SRC_DIR))

$(TARGET):$(CPPOBJS)
	$(HOST)g++ -o $@ $^
	$(HOST)strip $(@)

$(CPPOBJS):%.o:%.cpp
	$(HOST)g++ -c $(CPPFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -o $@ $<

clean:
	-rm -f $(addsuffix /*.d, $(SRC_DIR)) $(addsuffix /*.o, $(SRC_DIR)) $(TARGET)
