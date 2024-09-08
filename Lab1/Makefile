MAIN := inout.asm
MAIN_OUT := $(MAIN:.asm=.out)  ## 使用变量替换来简化文件后缀的转换
OUT := way/out

.PHONY: clean all

all: $(MAIN_OUT)
	./$(MAIN_OUT)

clean:
	rm -f $(MAIN_OUT) $(MAIN:.asm=.o)  ## 使用变量替换来简化清理目标文件的操作

$(MAIN_OUT): $(MAIN:.asm=.o)
	ld $< -o $@  ## 使用变量引用来代替具体文件名

%.o: %.asm  ## 利用默认规则处理 .asm 到 .o 的转换
	nasm -felf64 $<
