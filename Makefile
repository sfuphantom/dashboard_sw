CC = gcc
CFLAGS = -Wall -g
LDLIBS = -lwiringPi -pthread

OBJS = main.o vehicleSpeed.o batteryVoltage.o decode.o

TEST_SPEED_BIN = test_speed
TEST_BATTERY_BIN = test_battery
TEST_DECODE_BIN = test_decode
INSTALL_SCRIPT = scripts/install_dashboard.sh

all: dashboard

dashboard: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

# compiling
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

valgrind: dashboard
	valgrind --leak-check=full --show-reachable=yes --track-origins=yes ./dashboard

# CAN helpers 
.PHONY: can-up can-down can-test
can-up:
	./scripts/setup_can.sh

can-down:
	sudo ip link set can0 down || true

can-test:
	cansend can0 002#E803000000000000
	cansend can0 080#FFFF000000000000
	cansend can0 080#0000000000000000

# tests 
.PHONY: test test-speed test-battery test-decode
test: test-decode

test-speed: $(TEST_SPEED_BIN)
	./$(TEST_SPEED_BIN)

test-battery: $(TEST_BATTERY_BIN)
	./$(TEST_BATTERY_BIN)

test-decode: $(TEST_DECODE_BIN)
	./$(TEST_DECODE_BIN)

$(TEST_SPEED_BIN): test_speed.o vehicleSpeed.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(TEST_BATTERY_BIN): test_battery.o batteryVoltage.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(TEST_DECODE_BIN): test_decode.o decode.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o dashboard $(TEST_SPEED_BIN) $(TEST_BATTERY_BIN) $(TEST_DECODE_BIN)

.PHONY: run
run: dashboard
	./dashboard

.PHONY: install
install:
	sudo ./$(INSTALL_SCRIPT)

.PHONY: all valgrind clean
