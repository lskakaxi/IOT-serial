CC	= gcc
LINKER	= gcc -o
OUT_DIR	= out
INC_DIR	= include
CFLAGS	= -I$(INC_DIR) -Wall
DEBUGFLAGS	= -O0 -DDEBUG
RELEASEFLAGS	= -O2
LDFLAGS =

target_client	= client
src_client	= $(wildcard client/*.c)
headers_client	= $(wildcard $(INC_DIR)/*.h)
obj_client	= $(src_client:client/%.c=$(OUT_DIR)/%.o)

$(OUT_DIR)/$(target_client): mkout $(obj_client)
	$(LINKER) $@ $(LDFLAGS) $(obj_client)
	@echo "Generate client!"

$(obj_client) : $(OUT_DIR)/%.o : client/%.c
	$(CC) $(CFLAGS) -c $< -o $@

mkout:
	mkdir -p $(OUT_DIR) $(dir $(obj_client))

.PHONEY: clean
clean:
	rm -rf out
