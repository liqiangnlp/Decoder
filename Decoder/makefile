CXXFLAGS = -O2 -std=c++11

OBJS = alignment_difference.o basic_method.o configuration.o context_sensitive_wd.o debug.o detokenizer.o dispatcher.o ibm_bleu_score.o language_model.o language_model_score.o main.o me_reordering_table.o mert.o model.o msd_reordering_table.o multi_thread.o NiuLM.o online_service.o OurTrainer.o OurTree.o parameter_controller.o phrase_table.o phrasebased_decoder.o recasing.o rescoring.o search_engine.o split_sentences.o test_set.o tokenizer.o translation_memory.o tree.o tree2tree_decoder.o wde_metric.o

# smt platform
NiuTransServer.Decoder: $(OBJS)
	g++ -o ../bin/$@ $^ -lpthread

NiuTransServer.Decoder.i386: $(OBJS)
	g++ -D__MACHINE_TYPE_32__ -o ../bin/NiuTransServer.Decoder $^ -m32 -lpthread

$(OBJS): %.o:%.cpp
	g++ $(CXXFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	-rm $(OBJS) ../bin/NiuTransServer.Decoder
