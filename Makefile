TARGET = orbs
INCS = -Iinc
LIBS = -lX11 -lGL -lGLEW -lm
SHADERFS = \
	src/shader/cluster/cluster.vert \
	src/shader/cluster/cluster.frag \
	src/shader/cluster/clusterdyn.comp \
	src/shader/rng/rng_gauss.comp \
	src/shader/rng/rng_uniform.comp \
	src/shader/cluster_gen/clustercol.comp \
	src/shader/cluster_gen/clusterpos.comp \
	src/shader/bg/bg.frag \
	src/shader/bg/bg.vert
SHADER_SRC_H = inc/shadersrc.h

first: target

target: objdir shader main.o simulation.o sc.o
	tcc -o $(TARGET) obj/main.o obj/simulation.o obj/sc.o $(LIBS)
 
objdir:
	mkdir -p obj

shader:
	rm -f $(SHADER_SRC_H)
	for shaderf in $(SHADERFS); do \
		awk -f genshadersrc.awk $$shaderf >> $(SHADER_SRC_H); \
	done

main.o: src/main.c
	tcc -c -g -o obj/main.o src/main.c $(INCS)

simulation.o: src/simulation.c
	tcc -c -g -o obj/simulation.o src/simulation.c $(INCS)

sc.o: src/sc.c
	tcc -c -g -o obj/sc.o src/sc.c $(INCS)

clean:
	rm -f ./orbs
	rm -f ./obj/*
	rmdir ./obj
