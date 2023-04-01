TARGET = orbs
INCS = -Iinclude
LIBS = -lX11 -lGL -lGLEW -lm -lpthread -ljpeg -lpng
SHADERFS = 													\
	core/shader/cluster/cluster.vert 						\
	core/shader/cluster/cluster.frag 						\
	core/shader/cluster/evolve_cluster.comp 				\
	core/shader/cluster/gen_elliptic_cluster_pos.comp 		\
	core/shader/cluster/gen_elliptic_cluster_colors.comp 	\
	core/shader/firmament/firmament.vert 					\
	core/shader/firmament/firmament.frag 					\
	core/shader/rng/gen_uniform_samples.comp 				\
	core/shader/rng/gen_gaussian_samples.comp
SHADER_SRC_HEADER = include/shadersrc.h

first: target

target: objdir shader main.o visual.o load_textures.o orbs.o shader_compilation.o
	gcc -o $(TARGET) 				\
		obj/main.o 					\
		obj/visual.o 				\
		obj/load_textures.o 		\
		obj/orbs.o 					\
		obj/shader_compilation.o 	\
		$(LIBS)
 
objdir:
	mkdir -p obj

shader:
	rm -f $(SHADER_SRC_HEADER)
	for shaderf in $(SHADERFS); do \
		awk -f genshadersrc.awk $$shaderf >> $(SHADER_SRC_HEADER); \
	done

main.o: gui/main.c
	gcc -g -c -o obj/main.o gui/main.c $(INCS)

visual.o: gui/visual.c
	gcc -g -c -o obj/visual.o gui/visual.c $(INCS)

load_textures.o: gui/load_textures.c
	gcc -g -c -o obj/load_textures.o gui/load_textures.c

orbs.o: core/orbs.c
	gcc -g -c -o obj/orbs.o core/orbs.c $(INCS)

shader_compilation.o: core/shader_compilation.c
	gcc -g -c -o obj/shader_compilation.o core/shader_compilation.c $(INCS)

clean:
	rm -f orbs
	rm -rf obj/
	rm -f include/shadersrc.h
